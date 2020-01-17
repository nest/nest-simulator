/*
 *  test_block_vector.h
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

#ifndef TEST_BLOCK_VECTOR_H
#define TEST_BLOCK_VECTOR_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "block_vector.h"

BOOST_AUTO_TEST_SUITE( test_seque )

BOOST_AUTO_TEST_CASE( test_size )
{
  BlockVector< int > block_vector_a;
  BlockVector< int > block_vector_b;
  int N_a = block_vector_a.get_max_block_size() + 10;
  int N_b = block_vector_a.get_max_block_size();
  for ( int i = 0; i < N_a; ++i )
  {
    block_vector_a.push_back( i );
  }
  for ( int i = 0; i < N_b; ++i )
  {
    block_vector_b.push_back( i );
  }

  BOOST_REQUIRE( block_vector_a.size() == ( size_t ) N_a );
  BOOST_REQUIRE( block_vector_b.size() == ( size_t ) N_b );
}

BOOST_AUTO_TEST_CASE( test_random_access )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  BOOST_REQUIRE( block_vector[ 10 ] == 10 );
  BOOST_REQUIRE( block_vector[ 100 ] == 100 );
  BOOST_REQUIRE( block_vector[ N - 1 ] == N - 1 );
}

BOOST_AUTO_TEST_CASE( test_clear )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  block_vector.clear();

  BOOST_REQUIRE( block_vector.size() == 0 );

  int n_elements = 0;
  for ( int& it : block_vector )
  {
    BOOST_TEST_MESSAGE( it );
    ++n_elements;
  }
  BOOST_REQUIRE( n_elements == 0 );
}

BOOST_AUTO_TEST_CASE( test_erase )
{
  int N = 10;
  BlockVector< int > block_vector;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  auto seque_mid = block_vector;
  seque_mid.erase( seque_mid.begin() + 2, seque_mid.begin() + 8 );
  BOOST_REQUIRE( seque_mid.size() == 4 );
  BOOST_REQUIRE( seque_mid[ 0 ] == 0 );
  BOOST_REQUIRE( seque_mid[ 1 ] == 1 );
  BOOST_REQUIRE( seque_mid[ 2 ] == 8 );
  BOOST_REQUIRE( seque_mid[ 3 ] == 9 );

  auto seque_front = block_vector;
  seque_front.erase( seque_front.begin(), seque_front.begin() + 7 );
  BOOST_REQUIRE( seque_front.size() == 3 );
  BOOST_REQUIRE( seque_front[ 0 ] == 7 );
  BOOST_REQUIRE( seque_front[ 1 ] == 8 );
  BOOST_REQUIRE( seque_front[ 2 ] == 9 );

  auto seque_back = block_vector;
  seque_back.erase( seque_back.begin() + 3, seque_back.end() );
  BOOST_REQUIRE( seque_back.size() == 3 );
  BOOST_REQUIRE( seque_back[ 0 ] == 0 );
  BOOST_REQUIRE( seque_back[ 1 ] == 1 );
  BOOST_REQUIRE( seque_back[ 2 ] == 2 );
}

BOOST_AUTO_TEST_CASE( test_begin )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  BlockVector< int >::const_iterator begin = block_vector.begin();
  BOOST_REQUIRE( *begin == 0 );
}

BOOST_AUTO_TEST_CASE( test_end )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  BlockVector< int >::const_iterator end = block_vector.end();

  BOOST_REQUIRE( *( --end ) == N - 1 );
}

BOOST_AUTO_TEST_CASE( test_iterating )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  // Iterating forwards
  int j = 0;
  for ( int& it : block_vector )
  {
    BOOST_REQUIRE( it == j );
    ++j;
  }

  // Iterator decrement operator
  int k = N - 1;
  for ( BlockVector< int >::const_iterator it = --block_vector.end(); it != block_vector.begin(); --it )
  {
    BOOST_REQUIRE( *it == k );
    --k;
  }
}

BOOST_AUTO_TEST_CASE( test_iterator_arithmetic )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }
  BOOST_REQUIRE( *( block_vector.begin() + 1 ) == block_vector[ 1 ] );
  BOOST_REQUIRE( *( block_vector.begin() + ( N - 1 ) ) == block_vector[ N - 1 ] );

  auto it = block_vector.begin();
  it += N - 5;
  BOOST_REQUIRE( *it == block_vector[ N - 5 ] );

  auto it_2 = block_vector.begin();
  it_2 += 3;

  auto it_3 = block_vector.begin();
  ++it_3;

  decltype( it )::difference_type expected_diff = N - 5 - 3;
  BOOST_REQUIRE( it - it_2 == expected_diff );
  BOOST_REQUIRE( it_2 - it_3 == 2 );
  BOOST_REQUIRE( it - it == 0 );
}

BOOST_AUTO_TEST_CASE( test_iterator_dereference )
{
  // Using operator*
  BlockVector< int > block_vector;
  block_vector.push_back( 42 );
  BOOST_REQUIRE( *( block_vector.begin() ) == block_vector[ 0 ] );

  // Using operator->
  BlockVector< std::vector< int > > nested_seque;
  std::vector< int > tmp = { 42 };
  nested_seque.push_back( tmp );
  BOOST_REQUIRE( nested_seque.begin()->size() == 1 );
}

BOOST_AUTO_TEST_CASE( test_iterator_assign )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  int shift = N - 5;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }

  auto it = block_vector.begin();
  ++it;
  int current_value = *it;
  auto it_copy = it;
  // increment the iterator a bit to make it different
  for ( int j = 0; j < shift; ++j )
  {
    ++it;
  }

  BOOST_REQUIRE( it_copy != it );
  BOOST_REQUIRE( *it == current_value + shift );
  BOOST_REQUIRE( *it_copy == current_value );
}

BOOST_AUTO_TEST_CASE( test_iterator_compare )
{
  BlockVector< int > block_vector;
  int N = block_vector.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    block_vector.push_back( i );
  }
  BOOST_REQUIRE( block_vector.begin() < block_vector.end() );

  auto it_a = block_vector.begin();
  auto it_b = block_vector.begin();
  BOOST_REQUIRE( it_a == it_b );

  ++it_b;
  BOOST_REQUIRE( it_a != it_b );
  BOOST_REQUIRE( it_a < it_b );
  BOOST_REQUIRE( not( it_b < it_a ) );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* TEST_SORT_H */
