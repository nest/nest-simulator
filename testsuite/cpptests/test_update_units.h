/*
 *  test_update_units.h
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

#ifndef TEST_UPDATE_UNITS_H
#define TEST_UPDATE_UNITS_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <map>
#include <vector>

// Includes from nestkernel:
#include "update_block.h"

namespace nest
{

//! A block that only exists to be grouped.
class TestBlock : public UpdateBlock
{
public:
  void
  update( const Time&, const long, const long ) override
  {
  }
};

//! Group the "nodes" 0, 1, ... given their blocks (nullptr: no block); return the first node of every unit.
std::vector< int >
unit_starts( std::vector< UpdateBlock* > blocks, std::vector< UpdateBlock* >& unit_blocks )
{
  std::vector< int > nodes( blocks.size() );
  std::vector< int* > ptrs;
  for ( size_t i = 0; i < nodes.size(); ++i )
  {
    nodes[ i ] = static_cast< int >( i );
    ptrs.push_back( &nodes[ i ] );
  }
  const auto units = group_update_units( ptrs.begin(), ptrs.end(), [ & ]( int* n ) { return blocks[ *n ]; } );
  std::vector< int > starts;
  unit_blocks.clear();
  for ( const auto& [ block, node ] : units )
  {
    starts.push_back( *node );
    unit_blocks.push_back( block );
  }
  return starts;
}

BOOST_AUTO_TEST_SUITE( test_update_units )

BOOST_AUTO_TEST_CASE( no_blocks_one_unit_per_node )
{
  std::vector< UpdateBlock* > ub;
  const auto starts = unit_starts( { nullptr, nullptr, nullptr }, ub );
  BOOST_REQUIRE( starts == std::vector< int >( { 0, 1, 2 } ) );
  BOOST_REQUIRE( ub == std::vector< UpdateBlock* >( { nullptr, nullptr, nullptr } ) );
}

BOOST_AUTO_TEST_CASE( consecutive_nodes_of_a_block_form_one_unit )
{
  TestBlock a, b, c;
  std::vector< UpdateBlock* > ub;
  // node 0 alone, nodes 1-3 in block a, node 4 alone, nodes 5-6 in block b, nodes 7-8 in block c
  const auto starts = unit_starts( { nullptr, &a, &a, &a, nullptr, &b, &b, &c, &c }, ub );
  BOOST_REQUIRE( starts == std::vector< int >( { 0, 1, 4, 5, 7 } ) );
  BOOST_REQUIRE( ub == std::vector< UpdateBlock* >( { nullptr, &a, nullptr, &b, &c } ) );
}

BOOST_AUTO_TEST_CASE( adjacent_blocks_stay_separate )
{
  TestBlock a, b;
  std::vector< UpdateBlock* > ub;
  const auto starts = unit_starts( { &a, &a, &b, &b }, ub );
  BOOST_REQUIRE( starts == std::vector< int >( { 0, 2 } ) );
}

BOOST_AUTO_TEST_CASE( empty_thread )
{
  std::vector< UpdateBlock* > ub;
  BOOST_REQUIRE( unit_starts( {}, ub ).empty() );
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace nest

#endif /* TEST_UPDATE_UNITS_H */
