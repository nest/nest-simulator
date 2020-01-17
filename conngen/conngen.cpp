/*
 *  conngen.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "conngen.h"

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "modelrange.h"

// Includes from sli:
#include "token.h"

namespace nest
{

/**
 * Low-level function of the ConnectionGenerator interface for
 * connecting populations of neurons using a connection generator with
 * a value set and custom synapse type.
 *
 * \param cg The ConnectionGenerator describing the connectivity
 * \param source_node_ids A NodeCollection specifying the source population
 * \param target_node_ids A NodeCollection specifying the target population
 * \param params_map A Dictionary mapping the labels "weight" and
 *        "delay" to their indices in the value set
 * \param synmodel_name The name of the synapse model to use for the
 *        connections
 */
void
cg_connect( ConnectionGeneratorDatum& cg,
  const NodeCollectionPTR source_node_ids,
  const NodeCollectionPTR target_node_ids,
  const DictionaryDatum& params_map,
  const Name& synmodel_name )
{
  const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( synmodel_name.toString() );
  }
  const index synmodel_id = static_cast< index >( synmodel );
  DictionaryDatum dummy_params = new Dictionary();

  cg_set_masks( cg, source_node_ids, target_node_ids );
  cg->start();

  int source;
  int target;
  const int num_parameters = cg->arity();
  if ( num_parameters == 0 )
  {
    // connect source to target
    while ( cg->next( source, target, NULL ) )
    {
      // No need to check for locality of the target, as the mask
      // created by cg_set_masks() only contain local nodes.
      kernel().connection_manager.connect(
        ( *source_node_ids )[ source ], ( *target_node_ids )[ target ], dummy_params, synmodel_id );
    }
  }
  else if ( num_parameters == 2 )
  {
    if ( not params_map->known( names::weight ) or not params_map->known( names::delay ) )
    {
      throw BadProperty( "The parameter map has to contain the indices of weight and delay." );
    }

    const size_t w_idx = ( *params_map )[ names::weight ];
    const size_t d_idx = ( *params_map )[ names::delay ];

    const bool w_idx_is_0_or_1 = ( w_idx == 0 ) or ( w_idx == 1 );
    const bool d_idx_is_0_or_1 = ( d_idx == 0 ) or ( d_idx == 1 );
    const bool indices_differ = ( w_idx != d_idx );
    if ( not( w_idx_is_0_or_1 and d_idx_is_0_or_1 and indices_differ ) )
    {
      throw BadProperty( "w_idx and d_idx have to differ and be either 0 or 1." );
    }

    std::vector< double > params( 2 );

    // connect source to target with weight and delay
    while ( cg->next( source, target, &params[ 0 ] ) )
    {
      // No need to check for locality of the target node, as the mask
      // created by cg_set_masks() only contain local nodes.
      Node* const target_node = kernel().node_manager.get_node_or_proxy( ( *target_node_ids )[ target ] );
      const thread target_thread = target_node->get_thread();

      kernel().connection_manager.connect( ( *source_node_ids )[ source ],
        target_node,
        target_thread,
        synmodel_id,
        dummy_params,
        params[ d_idx ],
        params[ w_idx ] );
    }
  }
  else
  {
    LOG( M_ERROR, "CGConnect", "Either two or no parameters in the ConnectionSet expected." );
    throw DimensionMismatch();
  }
}

/**
 * Set the masks on the ConnectionGenerator cg. This function also
 * creates the masks from the given RangeSets sources and targets.
 *
 * \param cg The ConnectionGenerator to set the masks on
 * \param sources The source ranges to create the source masks from
 * \param targets The target ranges to create the target masks from
 */
void
cg_set_masks( ConnectionGeneratorDatum& cg, const NodeCollectionPTR sources, const NodeCollectionPTR targets )
{
  const size_t np = kernel().mpi_manager.get_num_processes();
  std::vector< ConnectionGenerator::Mask > masks( np, ConnectionGenerator::Mask( 1, np ) );

  RangeSet source_ranges;
  cg_get_ranges( source_ranges, sources );

  RangeSet target_ranges;
  cg_get_ranges( target_ranges, targets );

  cg_create_masks( masks, source_ranges, target_ranges );
  cg->setMask( masks, kernel().mpi_manager.get_rank() );
}

/**
 * Create the masks for sources and targets based on the contiguous
 * ranges given in sources and targets. We need to do some index
 * translation here, as the CG expects indices from 0..n for both
 * source and target populations, while the RangeSets sources and
 * targets contain NEST global indices (node IDs).
 *
 * The masks for the sources must contain all nodes (local+remote).
 * The skip of the mask was set to 1 in cg_set_masks(). The same
 * source mask is stored n_proc times on each process.
 *
 * The masks for the targets must only contain local nodes. This is
 * achieved by first setting skip to num_processes upon creation of
 * the mask in cg_set_masks(), and second by the fact that for each
 * contiguous range of nodes in a mask, each of them contains the
 * index-translated id of the first local neuron as the first
 * entry. If this renders the range empty (i.e. because the first
 * local id is beyond the last element of the range), the range is
 * not added to the mask.
 *
 * \param masks The std::vector of Masks to populate
 * \param sources The source ranges to create the source masks from
 * \param targets The target ranges to create the target masks from
 *
 * \note Each process computes the full set of source and target
 * masks, i.e. one mask per rank will be created on each rank.
 *
 * \note Setting the masks for all processes on each process might
 * become a memory bottleneck when going to very large numbers of
 * processes. Especially so for the source masks, which are all the
 * same. This could be solved by making the ConnectionGenerator
 * interface MPI aware and communicating the masks during connection
 * setup.
*/
void
cg_create_masks( std::vector< ConnectionGenerator::Mask >& masks, RangeSet& sources, RangeSet& targets )
{
  // The index of the left border of the currently looked at range
  // (counting from 0). This is used for index translation.
  size_t cg_idx_left = 0;

  // For sources, we only need to translate from NEST to CG indices.
  for ( RangeSet::iterator source = sources.begin(); source != sources.end(); ++source )
  {
    const size_t num_elements = source->last - source->first + 1;
    const size_t right = cg_idx_left + num_elements - 1;
    for ( size_t proc = 0; proc < static_cast< size_t >( kernel().mpi_manager.get_num_processes() ); ++proc )
    {
      masks[ proc ].sources.insert( cg_idx_left, right );
    }
    cg_idx_left += num_elements;
  }

  // Reset the index of the left border of the range for index
  // translation for the targets.
  cg_idx_left = 0;

  for ( RangeSet::iterator target = targets.begin(); target != targets.end(); ++target )
  {
    size_t num_elements = target->last - target->first + 1;
    for ( size_t proc = 0; proc < static_cast< size_t >( kernel().mpi_manager.get_num_processes() ); ++proc )
    {
      // Make sure that the range is only added on as many ranks as
      // there are elements in the range, or exactly on every rank,
      // if there are more elements in the range.
      if ( proc < num_elements )
      {
        // For the different ranks, left will take on the CG indices
        // of all first local nodes that are contained in the range.
        // The rank, where this mask is to be used is determined
        // below when inserting the mask.
        const size_t left = cg_idx_left + proc;

        // right is set to the CG index of the right border of the
        // range. This is the same for all ranks.
        const size_t right = cg_idx_left + num_elements - 1;

        // We index the masks according to the modulo distribution
        // of neurons in NEST. This ensures that the mask is set for
        // the rank where left acutally is the first neuron fromt
        // the currently looked at range.
        masks[ ( proc + target->first ) % kernel().mpi_manager.get_num_processes() ].targets.insert( left, right );
      }
    }

    // Update the CG index of the left border of the next range to
    // be one behind the current range.
    cg_idx_left += num_elements;
  }
}

/**
 * Calculate the right border of the contiguous range of node IDs
 * starting at left. The element is found using a binary search with
 * stepsize step.
 *
 * \param left The leftmost element of the range
 * \param step The step size for the binary search
 * \param node_ids The std::vector<long> of node IDs to search in
 * \returns the right border of the range
 */
index
cg_get_right_border( index left, size_t step, const NodeCollectionPTR node_ids )
{
  // Check if left is already the index of the last element in
  // node IDs. If yes, return left as the right border
  if ( left == node_ids->size() - 1 )
  {
    return left;
  }

  // leftmost_r is the leftmost right border during the search
  long leftmost_r = -1;

  // Initialize the search index i to the last valid index into node IDs
  // and last_i to i.
  long i = node_ids->size() - 1, last_i = i;

  while ( true )
  {
    // If i points to the end of node IDs and the distance between i and
    // left is the same as between the values node_ids[i] and node_ids[left]
    // (i.e. node_id[i+1] == node_id[i]+1 for all i), or if i is pointing at
    // the position of the leftmost right border we found until now
    // (i.e. we're back at an already visited index), we found the
    // right border of the contiguous range (last_i) and return it.
    if ( ( i == static_cast< long >( node_ids->size() ) - 1
           and ( *node_ids )[ i ] - ( *node_ids )[ left ] == i - static_cast< index >( left ) ) or i == leftmost_r )
    {
      return last_i;
    }

    // Store the current value of i in last_i. This is the current
    // candidate for the right border of the range.
    last_i = i;

    // If the range between node_ids[left] and node_ids[i] is contiguous,
    // set i to the right by step steps, else update the variable
    // for leftmost_r to the current i (i.e. the known leftmost
    // position) and set i to the left by step steps.
    if ( ( *node_ids )[ i ] - ( *node_ids )[ left ] == i - static_cast< index >( left ) )
    {
      i += step;
    }
    else
    {
      leftmost_r = i;
      i -= step;
    }

    // Reduce the search interval by half its size if it is > 1.
    // This adaptation is the basis of the binary search.
    if ( step != 1 )
    {
      step /= 2;
    }
  }

  // The border should always be found and returned during the while
  // loop above. This should never be reached.
  assert( false and "no right border found during search" );
  return 0;
}

/**
 * Determine all contiguous ranges found in a given vector of node IDs
 * and add the ranges to the given RangeSet.
 *
 * \param ranges A reference to the RangeSet to add to
 * \param node_ids A reference to a std::vector<long> of node IDs
 *
 * \note We do not store the indices into the given range, but
 * instead we store the actual node IDs. This allows us to use CG
 * generated indices as indices into the ranges spanned by the
 * RangeSet. Index translation is done in cg_create_masks().
 */
void
cg_get_ranges( RangeSet& ranges, const NodeCollectionPTR node_ids )
{
  index right, left = 0;
  while ( true )
  {
    // Determine the right border of the contiguous range starting
    // at left. The initial step is set to half the length of the
    // interval between left and the end of node IDs.
    right = cg_get_right_border( left, ( node_ids->size() - left ) / 2, node_ids );
    ranges.push_back( Range( ( *node_ids )[ left ], ( *node_ids )[ right ] ) );
    if ( right == node_ids->size() - 1 ) // We're at the end of node IDs and stop
    {
      break;
    }
    else
    {
      left = right + 1; // The new left border is one behind the old right
    }
  }
}

} // namespace nest
