/*
 *  cg_connect.cpp
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

#include "cg_connect.h"

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h" // TODO implement in terms of nest-API


namespace nest
{
void
cg_connect( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  index source_offset,
  RangeSet& targets,
  index target_offset,
  DictionaryDatum params_map,
  index syn )
{
  cg_set_masks( cg, sources, targets );
  cg->start();

  int source, target, num_parameters = cg->arity();
  if ( num_parameters == 0 )
  {
    // connect source to target
    while ( cg->next( source, target, NULL ) )
    {
      if ( kernel().node_manager.is_local_gid( target + target_offset ) )
      {
        Node* const target_node =
          kernel().node_manager.get_node( target + target_offset );
        const thread target_thread = target_node->get_thread();
        kernel().connection_manager.connect(
          source + source_offset, target_node, target_thread, syn );
      }
    }
  }
  else if ( num_parameters == 2 )
  {
    if ( not params_map->known( names::weight )
      || not params_map->known( names::delay ) )
    {
      throw BadProperty(
        "The parameter map has to contain the indices of weight and delay." );
    }

    long w_idx = ( *params_map )[ names::weight ];
    long d_idx = ( *params_map )[ names::delay ];
    std::vector< double > params( 2 );

    // connect source to target with weight and delay
    while ( cg->next( source, target, &params[ 0 ] ) )
    {
      if ( kernel().node_manager.is_local_gid( target + target_offset ) )
      {
        Node* const target_node =
          kernel().node_manager.get_node( target + target_offset );
        const thread target_thread = target_node->get_thread();
        kernel().connection_manager.connect( source + source_offset,
          target_node,
          target_thread,
          syn,
          params[ d_idx ],
          params[ w_idx ] );
      }
    }
  }
  else
  {
    LOG( M_ERROR,
      "Connect",
      "Either two or no parameters in the Connection Set expected." );
    throw DimensionMismatch();
  }
}

void
cg_connect( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  std::vector< long >& source_gids,
  RangeSet& targets,
  std::vector< long >& target_gids,
  DictionaryDatum params_map,
  index syn )
{
  cg_set_masks( cg, sources, targets );
  cg->start();

  int source, target, num_parameters = cg->arity();
  if ( num_parameters == 0 )
  {
    // connect source to target
    while ( cg->next( source, target, NULL ) )
    {
      if ( kernel().node_manager.is_local_gid( target_gids.at( target ) ) )
      {
        Node* const target_node =
          kernel().node_manager.get_node( target_gids.at( target ) );
        const thread target_thread = target_node->get_thread();
        kernel().connection_manager.connect(
          source_gids.at( source ), target_node, target_thread, syn );
      }
    }
  }
  else if ( num_parameters == 2 )
  {
    if ( not params_map->known( names::weight )
      || not params_map->known( names::delay ) )
    {
      throw BadProperty(
        "The parameter map has to contain the indices of weight and delay." );
    }

    long w_idx = ( *params_map )[ names::weight ];
    long d_idx = ( *params_map )[ names::delay ];
    std::vector< double > params( 2 );

    // connect source to target with weight and delay
    while ( cg->next( source, target, &params[ 0 ] ) )
    {
      if ( kernel().node_manager.is_local_gid( target_gids.at( target ) ) )
      {
        Node* const target_node =
          kernel().node_manager.get_node( target_gids.at( target ) );
        const thread target_thread = target_node->get_thread();
        kernel().connection_manager.connect( source_gids.at( source ),
          target_node,
          target_thread,
          syn,
          params[ d_idx ],
          params[ w_idx ] );
      }
    }
  }
  else
  {
    LOG( M_ERROR,
      "Connect",
      "Either two or no parameters in the Connection Set expected." );
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
cg_set_masks( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  RangeSet& targets )
{
  long np = kernel().mpi_manager.get_num_processes();
  std::vector< ConnectionGenerator::Mask > masks(
    np, ConnectionGenerator::Mask( 1, np ) );

  cg_create_masks( &masks, sources, targets );
  cg->setMask( masks, kernel().mpi_manager.get_rank() );
}

/**
 * Create the masks for sources and targets based on the contiguous
 * ranges given in sources and targets. We need to do some index
 * translation here, as the CG expects indices from 0..n for both
 * source and target populations, while the RangeSets sources and
 * targets contain NEST global indices (gids).
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
cg_create_masks( std::vector< ConnectionGenerator::Mask >* masks,
  RangeSet& sources,
  RangeSet& targets )
{
  // The index of the left border of the currently looked at range
  // (counting from 0). This is used for index translation.
  size_t cg_idx_left = 0;

  // For sources, we only need to translate from NEST to CG indices.
  for ( RangeSet::iterator source = sources.begin(); source != sources.end();
        ++source )
  {
    size_t num_elements = source->last - source->first;
    size_t right = cg_idx_left + num_elements;
    for ( size_t proc = 0; proc
            < static_cast< size_t >( kernel().mpi_manager.get_num_processes() );
          ++proc )
    {
      ( *masks )[ proc ].sources.insert( cg_idx_left, right );
    }
    cg_idx_left += num_elements + 1;
  }

  // Reset the index of the left border of the range for index
  // translation for the targets.
  cg_idx_left = 0;

  for ( RangeSet::iterator target = targets.begin(); target != targets.end();
        ++target )
  {
    size_t num_elements = target->last - target->first;
    for ( size_t proc = 0; proc
            < static_cast< size_t >( kernel().mpi_manager.get_num_processes() );
          ++proc )
    {
      // Make sure that the range is only added on as many ranks as
      // there are elements in the range, or exactly on every rank,
      // if there are more elements in the range.
      if ( proc <= num_elements )
      {
        // For the different ranks, left will take on the CG indices
        // of all first local nodes that are contained in the range.
        // The rank, where this mask is to be used is determined
        // below when inserting the mask.
        size_t left = cg_idx_left + proc;

        // right is set to the CG index of the right border of the
        // range. This is the same for all ranks.
        size_t right = cg_idx_left + num_elements;

        // We index the masks according to the modulo distribution
        // of neurons in NEST. This ensures that the mask is set for
        // the rank where left acutally is the first neuron fromt
        // the currently looked at range.
        ( *masks )[ ( proc + target->first )
          % kernel().mpi_manager.get_num_processes() ].targets.insert( left,
          right );
      }
    }

    // Update the CG index of the left border of the next range to
    // be one after the current range.
    cg_idx_left += num_elements + 1;
  }
}

/**
 * Calculate the right border of the contiguous range of gids
 * starting at left. The element is found using a binary search with
 * stepsize step.
 *
 * \param left The leftmost element of the range
 * \param step The step size for the binary search
 * \param gids The std::vector<long> of gids to search in
 */
index
cg_get_right_border( index left, size_t step, std::vector< long >& gids )
{
  // Check if left is already the index of the last element in
  // gids. If yes, return left as the right border
  if ( left == gids.size() - 1 )
  {
    return left;
  }

  // leftmost_r is the leftmost right border during the search
  long leftmost_r = -1;

  // Initialize the search index i to the last valid index into gids
  // and last_i to i.
  long i = gids.size() - 1, last_i = i;

  while ( true )
  {
    // If i points to the end of gids and the distance between i and
    // left is the same as between the values gids[i] and gids[left]
    // (i.e. gid[i+1] == gid[i]+1 for all i), or if i is pointing at
    // the position of the leftmost right border we found until now
    // (i.e. we're back at an already visited index), we found the
    // right border of the contiguous range (last_i) and return it.
    if ( ( i == static_cast< long >( gids.size() ) - 1
           && gids[ i ] - gids[ left ] == i - static_cast< long >( left ) )
      || i == leftmost_r )
    {
      return last_i;
    }

    // Store the current value of i in last_i. This is the current
    // candidate for the right border of the range.
    last_i = i;

    // If the range between gids[left] and gids[i] is contiguous,
    // set i to the right by step steps, else update the variable
    // for leftmost_r to the current i (i.e. the known leftmost
    // position) and set i to the left by step steps.
    if ( gids[ i ] - gids[ left ] == i - static_cast< long >( left ) )
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
  assert( false && "no right border found during search" );
  return 0;
}

/**
 * Determine all contiguous ranges found in a given vector of gids
 * and add the ranges to the given RangeSet.
 *
 * \param ranges A reference to the RangeSet to add to
 * \param gids The std::vector<long> of gids
 *
 * \note We do not store the indices into the given range, but
 * instead we store the actual gids. This allows us to use CG
 * generated indices as indices into the ranges spanned by the
 * RangeSet. Index translation is done in cg_create_masks().
 */
void
cg_get_ranges( RangeSet& ranges, std::vector< long >& gids )
{
  index right, left = 0;
  while ( true )
  {
    // Determine the right border of the contiguous range starting
    // at left. The initial step is set to half the length of the
    // interval between left and the end of gids.
    right = cg_get_right_border( left, ( gids.size() - left ) / 2, gids );
    ranges.push_back( Range( gids[ left ], gids[ right ] ) );
    if ( right == gids.size() - 1 ) // We're at the end of gids and stop
    {
      break;
    }
    else
    {
      left = right + 1;
    } // The new left border is one after the old right
  }
}

} // namespace nest
