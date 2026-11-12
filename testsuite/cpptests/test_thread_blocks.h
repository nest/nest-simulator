/*
 *  test_thread_blocks.h
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

#ifndef TEST_THREAD_BLOCKS_H
#define TEST_THREAD_BLOCKS_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "thread_blocks.h"

namespace
{
//! NUMA node of every thread for domains of the given sizes, nodes numbered from first_node
std::vector< size_t >
nodes_of_domains( const std::vector< size_t >& sizes, const size_t first_node = 0 )
{
  std::vector< size_t > nodes;
  for ( size_t d = 0; d < sizes.size(); ++d )
  {
    nodes.insert( nodes.end(), sizes[ d ], first_node + d );
  }
  return nodes;
}
}

BOOST_AUTO_TEST_SUITE( test_thread_blocks )

BOOST_AUTO_TEST_CASE( test_equal_domains_give_equal_blocks )
{
  const std::vector< size_t > expected { 0, 72, 144, 216, 288 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 72, 72, 72, 72 } ), 4 ) == expected );
}

BOOST_AUTO_TEST_CASE( test_unequal_domains )
{
  // one core left free on the first of four nodes
  const std::vector< size_t > expected { 0, 71, 143, 215, 287 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 71, 72, 72, 72 } ), 4 ) == expected );
}

BOOST_AUTO_TEST_CASE( test_domains_divided )
{
  const std::vector< size_t > expected { 0, 36, 71, 107, 143 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 71, 72 } ), 4 ) == expected );
}

BOOST_AUTO_TEST_CASE( test_domains_merged )
{
  // eight NUMA nodes (e.g. four per socket), two blocks: one per socket
  const std::vector< size_t > expected { 0, 127, 255 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 31, 32, 32, 32, 32, 32, 32, 32 } ), 2 ) == expected );
}

BOOST_AUTO_TEST_CASE( test_node_numbers_need_not_start_at_zero )
{
  const std::vector< size_t > expected { 0, 10, 20 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 10, 10 }, 2 ), 2 ) == expected );
}

BOOST_AUTO_TEST_CASE( test_fallback_to_equal_blocks )
{
  const std::vector< size_t > equal { 0, 4, 7, 10 };
  // three blocks on two domains: neither a multiple of the other
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 6, 4 } ), 3 ) == equal );
  // a node unknown
  auto nodes = nodes_of_domains( { 5, 5 } );
  nodes[ 3 ] = nest::unknown_numa_node;
  BOOST_TEST( nest::thread_blocks( nodes, 3 ) == equal );
  // threads of a node not consecutive (threads not pinned in node order)
  const std::vector< size_t > alternating { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
  const std::vector< size_t > equal_two { 0, 5, 10 };
  BOOST_TEST( nest::thread_blocks( alternating, 2 ) == equal_two );
  // a domain with fewer threads than blocks per domain
  const std::vector< size_t > equal_four { 0, 3, 5, 8, 10 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 1, 9 } ), 4 ) == equal_four );
}

BOOST_AUTO_TEST_CASE( test_more_blocks_than_threads )
{
  const std::vector< size_t > expected { 0, 1, 2, 3 };
  BOOST_TEST( nest::thread_blocks( nodes_of_domains( { 3 } ), 8 ) == expected );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* TEST_THREAD_BLOCKS_H */
