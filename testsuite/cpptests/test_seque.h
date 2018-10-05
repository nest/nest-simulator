/*
 *  test_seque.h
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

#ifndef TEST_SEQUE_H
#define TEST_SEQUE_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "seque.h"

BOOST_AUTO_TEST_SUITE( test_seque )

BOOST_AUTO_TEST_CASE( test_size )
{
  // TODO: check both branches of if test in size()
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  BOOST_REQUIRE( seque.size() == ( size_t ) N );
}

BOOST_AUTO_TEST_CASE( test_random_access )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  BOOST_REQUIRE( seque[ 10 ] == 10 );
  BOOST_REQUIRE( seque[ 100 ] == 100 );
  BOOST_REQUIRE( seque[ N - 1 ] == N - 1 );
}

BOOST_AUTO_TEST_CASE( test_clear )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  seque.clear();

  BOOST_REQUIRE( seque.size() == 0 );

  int n_elements = 0;
  for ( int& it : seque )
  {
    BOOST_TEST_MESSAGE( it );
    ++n_elements;
  }
  BOOST_REQUIRE( n_elements == 0 );
}

BOOST_AUTO_TEST_CASE( test_erase )
{
  int N = 10;
  Seque< int > seque;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  auto seque_mid = seque;
  seque_mid.erase( seque_mid.begin() + 2, seque_mid.begin() + 8 );
  BOOST_REQUIRE( seque_mid.size() == 4 );
  BOOST_REQUIRE( seque_mid[ 0 ] == 0 );
  BOOST_REQUIRE( seque_mid[ 1 ] == 1 );
  BOOST_REQUIRE( seque_mid[ 2 ] == 8 );
  BOOST_REQUIRE( seque_mid[ 3 ] == 9 );

  auto seque_front = seque;
  seque_front.erase( seque_front.begin(), seque_front.begin() + 7 );
  BOOST_REQUIRE( seque_front.size() == 3 );
  BOOST_REQUIRE( seque_front[ 0 ] == 7 );
  BOOST_REQUIRE( seque_front[ 1 ] == 8 );
  BOOST_REQUIRE( seque_front[ 2 ] == 9 );

  auto seque_back = seque;
  seque_back.erase( seque_back.begin() + 3, seque_back.end() );
  BOOST_REQUIRE( seque_back.size() == 3 );
  BOOST_REQUIRE( seque_back[ 0 ] == 0 );
  BOOST_REQUIRE( seque_back[ 1 ] == 1 );
  BOOST_REQUIRE( seque_back[ 2 ] == 2 );
}

BOOST_AUTO_TEST_CASE( test_begin )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  Seque< int >::const_iterator begin = seque.begin();
  BOOST_REQUIRE( *begin == 0 );
}

BOOST_AUTO_TEST_CASE( test_end )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  Seque< int >::const_iterator end = seque.end();

  BOOST_REQUIRE( *( --end ) == N - 1 );
}

BOOST_AUTO_TEST_CASE( test_iterating )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  // Iterating forwards
  int j = 0;
  for ( int& it : seque )
  {
    BOOST_REQUIRE( it == j );
    ++j;
  }

  // Iterator decrement operator
  int k = N - 1;
  for ( Seque< int >::const_iterator it = --seque.end(); it != seque.begin();
        --it )
  {
    BOOST_REQUIRE( *it == k );
    --k;
  }
}

BOOST_AUTO_TEST_CASE( test_iterator_arithmetic )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }
  BOOST_REQUIRE( *( seque.begin() + 1 ) == seque[ 1 ] );
  BOOST_REQUIRE( *( seque.begin() + ( N - 1 ) ) == seque[ N - 1 ] );

  auto it = seque.begin();
  it += N - 5;
  BOOST_REQUIRE( *it == seque[ N - 5 ] );

  auto it_2 = seque.begin();
  it_2 += 3;

  auto it_3 = seque.begin();
  ++it_3;

  decltype( it )::difference_type expected_diff = N - 5 - 3;
  BOOST_REQUIRE( it - it_2 == expected_diff );
  BOOST_REQUIRE( it_2 - it_3 == 2 );
  BOOST_REQUIRE( it - it == 0 );
}

BOOST_AUTO_TEST_CASE( test_iterator_dereference )
{
  // Using operator*
  Seque< int > seque;
  seque.push_back( 42 );
  BOOST_REQUIRE( *( seque.begin() ) == seque[ 0 ] );

  // Using operator->
  Seque< std::vector< int > > nested_seque;
  std::vector< int > tmp = { 42 };
  nested_seque.push_back( tmp );
  BOOST_REQUIRE( nested_seque.begin()->size() == 1 );
}

BOOST_AUTO_TEST_CASE( test_iterator_assign )
{
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  int shift = N - 5;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }

  auto it = seque.begin();
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
  Seque< int > seque;
  int N = seque.get_max_block_size() + 10;
  for ( int i = 0; i < N; ++i )
  {
    seque.push_back( i );
  }
  BOOST_REQUIRE( seque.begin() < seque.end() );

  auto it_a = seque.begin();
  auto it_b = seque.begin();
  BOOST_REQUIRE( it_a == it_b );

  ++it_b;
  BOOST_REQUIRE( it_a != it_b );
  BOOST_REQUIRE( it_a < it_b );
  BOOST_REQUIRE( not( it_b < it_a ) );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* TEST_SORT_H */
