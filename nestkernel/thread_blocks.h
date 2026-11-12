/*
 *  thread_blocks.h
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

#ifndef THREAD_BLOCKS_H
#define THREAD_BLOCKS_H

// C++ includes:
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace nest
{

//! Marks a thread whose NUMA node is not known.
constexpr size_t unknown_numa_node = std::numeric_limits< size_t >::max();

/**
 * Divide the threads into blocks of consecutive threads that are aligned with the NUMA nodes of the threads.
 *
 * Returns the boundaries: block b consists of the threads [ bounds[ b ], bounds[ b + 1 ] ), the last entry is the
 * number of threads. The number of blocks is min( num_blocks, number of threads ).
 *
 * The threads on one NUMA node form a domain if they have consecutive numbers, as with OMP_PROC_BIND=close. If the
 * number of blocks is a multiple of the number of domains, each domain is divided into equal blocks; if the number of
 * domains is a multiple of the number of blocks, neighboring domains are merged. Otherwise, or if a node is unknown,
 * the threads of a node are not consecutive, or a domain has fewer threads than blocks, the threads are divided into
 * blocks of equal size ( ceil( b * T / num_blocks ) ), as without NUMA information.
 *
 * Example: 287 threads on four NUMA nodes with 71, 72, 72, 72 threads (one core left free) and four blocks give the
 * boundaries 0, 71, 143, 215, 287; blocks of equal size would be 0, 72, 144, 216, 287, so that thread 71 would belong
 * to the block of the first node while running on the second.
 */
inline std::vector< size_t >
thread_blocks( const std::vector< size_t >& numa_node_of_thread, const size_t num_blocks )
{
  const size_t num_threads = numa_node_of_thread.size();
  assert( num_threads > 0 and num_blocks > 0 );
  const size_t blocks = std::min( num_blocks, num_threads );

  std::vector< size_t > equal( blocks + 1 );
  for ( size_t b = 0; b <= blocks; ++b )
  {
    equal[ b ] = ( b * num_threads + blocks - 1 ) / blocks;
  }

  // domains: runs of consecutive threads on the same node; every node must form a single run
  std::vector< size_t > domain_begin;
  std::vector< size_t > domain_node;
  for ( size_t t = 0; t < num_threads; ++t )
  {
    const size_t node = numa_node_of_thread[ t ];
    if ( node == unknown_numa_node )
    {
      return equal;
    }
    if ( t == 0 or node != numa_node_of_thread[ t - 1 ] )
    {
      if ( std::find( domain_node.begin(), domain_node.end(), node ) != domain_node.end() )
      {
        return equal;
      }
      domain_begin.push_back( t );
      domain_node.push_back( node );
    }
  }
  domain_begin.push_back( num_threads );
  const size_t domains = domain_node.size();

  std::vector< size_t > bounds;
  if ( blocks % domains == 0 )
  {
    const size_t per_domain = blocks / domains;
    for ( size_t d = 0; d < domains; ++d )
    {
      const size_t size = domain_begin[ d + 1 ] - domain_begin[ d ];
      if ( size < per_domain )
      {
        return equal;
      }
      for ( size_t j = 0; j < per_domain; ++j )
      {
        bounds.push_back( domain_begin[ d ] + ( j * size + per_domain - 1 ) / per_domain );
      }
    }
  }
  else if ( domains % blocks == 0 )
  {
    const size_t per_block = domains / blocks;
    for ( size_t b = 0; b < blocks; ++b )
    {
      bounds.push_back( domain_begin[ b * per_block ] );
    }
  }
  else
  {
    return equal;
  }
  bounds.push_back( num_threads );
  return bounds;
}

}  // namespace nest

#endif /* THREAD_BLOCKS_H */
