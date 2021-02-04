/*
 *  conn_builder_conngen.cpp
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

#include "conn_builder_conngen.h"

#ifdef HAVE_LIBNEUROSIM

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

ConnectionGeneratorBuilder::ConnectionGeneratorBuilder( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const DictionaryDatum& conn_spec,
  const std::vector< DictionaryDatum >& syn_specs )
  : ConnBuilder( sources, targets, conn_spec, syn_specs )
  , cg_( ConnectionGeneratorDatum() )
  , params_map_()
{
  updateValue< ConnectionGeneratorDatum >( conn_spec, "cg", cg_ );
  if ( cg_->arity() != 0 )
  {
    if ( not conn_spec->known( "params_map" ) )
    {
      throw BadProperty( "A params_map has to be given if the ConnectionGenerator has values." );
    }

    updateValue< DictionaryDatum >( conn_spec, "params_map", params_map_ );

    for ( Dictionary::iterator it = params_map_->begin(); it != params_map_->end(); ++it )
    {
      it->second.set_access_flag();
    }

    if ( syn_specs[ 0 ]->known( names::weight ) or syn_specs[ 0 ]->known( names::delay ) )
    {
      throw BadProperty(
        "Properties weight and delay cannot be specified in syn_spec if the ConnectionGenerator has values." );
    }
  }
  if ( syn_specs.size() > 1 )
  {
    throw BadProperty( "Connection rule conngen cannot be used with collocated synapses." );
  }
}

void
ConnectionGeneratorBuilder::connect_()
{
  cg_set_masks();
  cg_->start();

  librandom::RngPtr rng = kernel().rng_manager.get_grng();

  int source;
  int target;
  const int num_parameters = cg_->arity();
  if ( num_parameters == 0 )
  {
    // connect source to target
    while ( cg_->next( source, target, NULL ) )
    {
      // No need to check for locality of the target, as the mask
      // created by cg_set_masks() only contains local nodes.
      Node* const target_node = kernel().node_manager.get_node_or_proxy( ( *targets_ )[ target ] );
      const thread target_thread = target_node->get_thread();
      single_connect_( ( *sources_ )[ source ], *target_node, target_thread, rng );
    }
  }
  else if ( num_parameters == 2 )
  {
    if ( not params_map_->known( names::weight ) or not params_map_->known( names::delay ) )
    {
      throw BadProperty( "The parameter map has to contain the indices of weight and delay." );
    }

    const size_t d_idx = ( *params_map_ )[ names::delay ];
    const size_t w_idx = ( *params_map_ )[ names::weight ];

    const bool d_idx_is_0_or_1 = ( d_idx == 0 ) or ( d_idx == 1 );
    const bool w_idx_is_0_or_1 = ( w_idx == 0 ) or ( w_idx == 1 );
    const bool indices_differ = ( w_idx != d_idx );
    if ( not( d_idx_is_0_or_1 and w_idx_is_0_or_1 and indices_differ ) )
    {
      throw BadProperty( "The indices for weight and delay have to be either 0 or 1 and cannot be the same." );
    }

    // connect source to target with weight and delay
    std::vector< double > params( 2 );
    while ( cg_->next( source, target, &params[ 0 ] ) )
    {
      // No need to check for locality of the target node, as the mask
      // created by cg_set_masks() only contains local nodes.
      Node* target_node = kernel().node_manager.get_node_or_proxy( ( *targets_ )[ target ] );
      const thread target_thread = target_node->get_thread();

      update_param_dict_( ( *sources_ )[ source ], *target_node, target_thread, rng, 0 );

      // Use the low-level connect() here, as we need to pass a custom weight and delay
      kernel().connection_manager.connect( ( *sources_ )[ source ],
        target_node,
        target_thread,
        synapse_model_id_[ 0 ],
        param_dicts_[ 0 ][ target_thread ],
        params[ d_idx ],
        params[ w_idx ] );
    }
  }
  else
  {
    LOG( M_ERROR, "Connect", "Either two or no parameters in the ConnectionGenerator expected." );
    throw DimensionMismatch();
  }
}

/**
 * Create the masks for sources and targets and set them on the
 * Connection Generator.
 *
 * The masks are based on the contiguous ranges present in the given
 * sources and targets. We need to do some index translation here, as
 * the CG expects indices from 0..n for both source and target
 * populations, while the corresponding RangeSets for sources and
 * targets contain NEST global indices (node IDs).
 *
 * The masks for the sources must contain all nodes (local+remote). To
 * achieve this, the skip of the mask is set to 1 and the same source
 * mask is stored n_proc times on each process.
 *
 * The masks for the targets must only contain local nodes. This is
 * achieved by first setting skip to num_processes upon creation of
 * the mask, and second by the fact that for each contiguous range of
 * nodes in a mask, each of them contains the index-translated id of
 * the first local neuron as the first entry. If this renders the
 * range empty (i.e. because the first local id is beyond the last
 * element of the range), the range is not added to the mask.
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
ConnectionGeneratorBuilder::cg_set_masks()
{
  const size_t np = kernel().mpi_manager.get_num_processes();
  std::vector< ConnectionGenerator::Mask > masks( np, ConnectionGenerator::Mask( 1, np ) );

  // The index of the left border of the currently looked at range
  // (counting from 0). This is used for index translation.
  size_t cg_idx_left = 0;

  // For sources, we only need to translate from NEST to CG indices.
  RangeSet source_ranges;
  cg_get_ranges( source_ranges, sources_ );
  for ( auto source : source_ranges )
  {
    const size_t num_elements = source.last - source.first + 1;
    const size_t right = cg_idx_left + num_elements - 1;
    for ( size_t proc = 0; proc < np; ++proc )
    {
      masks[ proc ].sources.insert( cg_idx_left, right );
    }
    cg_idx_left += num_elements;
  }

  // Reset the index of the left border of the range for index
  // translation for the targets.
  cg_idx_left = 0;

  RangeSet target_ranges;
  cg_get_ranges( target_ranges, targets_ );
  for ( auto target : target_ranges )
  {
    size_t num_elements = target.last - target.first + 1;
    for ( size_t proc = 0; proc < np; ++proc )
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
        masks[ ( proc + target.first ) % np ].targets.insert( left, right );
      }
    }

    // Update the CG index of the left border of the next range to
    // be one behind the current range.
    cg_idx_left += num_elements;
  }

  cg_->setMask( masks, kernel().mpi_manager.get_rank() );
}


/**
 * Calculate the right border of the contiguous range of node IDs
 * starting at left. The element is found using a binary search with
 * stepsize step.
 *
 * \param left The leftmost element of the range
 * \param step The step size for the binary search
 * \param nodes The std::vector<long> of node IDs to search in
 * \returns the right border of the range
 */
index
ConnectionGeneratorBuilder::cg_get_right_border( index left, size_t step, const NodeCollectionPTR nodes )
{
  // Check if left is already the index of the last element in
  // node IDs. If yes, return left as the right border
  if ( left == nodes->size() - 1 )
  {
    return left;
  }

  // leftmost_r is the leftmost right border during the search
  long leftmost_r = -1;

  // Initialize the search index i to the last valid index into node IDs
  // and last_i to i.
  long i = nodes->size() - 1, last_i = i;

  while ( true )
  {
    // If i points to the end of node IDs and the distance between i and
    // left is the same as between the values nodes[i] and nodes[left]
    // (i.e. node_id[i+1] == node_id[i]+1 for all i), or if i is pointing at
    // the position of the leftmost right border we found until now
    // (i.e. we're back at an already visited index), we found the
    // right border of the contiguous range (last_i) and return it.
    const bool end_of_nodes = i == static_cast< long >( nodes->size() ) - 1;
    const auto dist_a = ( *nodes )[ i ] - ( *nodes )[ left ];
    const auto dist_b = i - static_cast< index >( left );
    const bool same_dist = dist_a == dist_b;

    if ( ( end_of_nodes and same_dist ) or i == leftmost_r )
    {
      return last_i;
    }

    // Store the current value of i in last_i. This is the current
    // candidate for the right border of the range.
    last_i = i;

    // If the range between nodes[left] and nodes[i] is contiguous,
    // set i to the right by step steps, else update the variable
    // for leftmost_r to the current i (i.e. the known leftmost
    // position) and set i to the left by step steps.
    if ( ( *nodes )[ i ] - ( *nodes )[ left ] == i - static_cast< index >( left ) )
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
 * \param nodes A reference to a std::vector<long> of node IDs
 *
 * \note We do not store the indices into the given range, but
 * instead we store the actual node IDs. This allows us to use CG
 * generated indices as indices into the ranges spanned by the
 * RangeSet. Index translation is done in cg_create_masks().
 */
void
ConnectionGeneratorBuilder::cg_get_ranges( RangeSet& ranges, const NodeCollectionPTR nodes )
{
  index right, left = 0;
  while ( true )
  {
    // Determine the right border of the contiguous range starting
    // at left. The initial step is set to half the length of the
    // interval between left and the end of node IDs.
    right = cg_get_right_border( left, ( nodes->size() - left ) / 2, nodes );
    ranges.push_back( Range( ( *nodes )[ left ], ( *nodes )[ right ] ) );
    if ( right == nodes->size() - 1 ) // We're at the end of node IDs and stop
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

#endif /* ifdef HAVE_LIBNEUROSIM */
