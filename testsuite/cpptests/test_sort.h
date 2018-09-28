/*
 *  test_sort.h
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

#ifndef TEST_SORT_H
#define TEST_SORT_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "sort.h"

namespace nest
{

const bool
is_sorted( std::vector< size_t >::const_iterator begin,
  std::vector< size_t >::const_iterator end )
{
  for ( std::vector< size_t >::const_iterator it = begin; it < --end; )
  {
    if ( *it > *( ++it ) )
    {
      return false;
    }
  }
  return true;
}

const bool is_sorted( Seque< size_t >::const_iterator begin,
    Seque< size_t >::const_iterator end )
{
  for ( Seque< size_t >::const_iterator it = begin; it < --end; )
  {
    if ( *it > *( ++it ) )
    {
      return false;
    }
  }
  return true;
}

BOOST_AUTO_TEST_SUITE( test_sort )

/**
 * Tests whether two arrays with randomly generated numbers are sorted
 * correctly by a single call to sort.
 */
BOOST_AUTO_TEST_CASE( test_random_vec_seq )
{
  const size_t N = 20000;
  std::vector< size_t > vec0( N );
  Seque< size_t > seq1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    const size_t k = std::rand() % N;
    vec0[ i ] = k;
    seq1[ i ] = k;
  }

  nest::sort( vec0, seq1 );

  BOOST_REQUIRE( is_sorted( vec0.begin(), vec0.end() ) );
  BOOST_REQUIRE( is_sorted( seq1.begin(), seq1.end() ) );
}

BOOST_AUTO_TEST_CASE( test_random_seq_seq )
{
  const size_t N = 20000;
  Seque< size_t > seq0( N );
  Seque< size_t > seq1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    const size_t k = std::rand() % N;
    seq0[ i ] = k;
    seq1[ i ] = k;
  }

  nest::sort( seq0, seq1 );

  BOOST_REQUIRE( is_sorted( seq0.begin(), seq0.end() ) );
  BOOST_REQUIRE( is_sorted( seq1.begin(), seq1.end() ) );
}

/**
 * Tests whether two arrays with linearly increasing numbers are sorted
 * correctly by a single call to sort.
*/
BOOST_AUTO_TEST_CASE( test_linear_vec_seq )
{
  const size_t N = 20000;
  std::vector< size_t > vec0( N );
  Seque< size_t > seq1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    vec0[ i ] = N - i - 1;
    seq1[ i ] = N - i - 1;
  }

  nest::sort( vec0, seq1 );

  BOOST_REQUIRE( is_sorted( vec0.begin(), vec0.end() ) );
  BOOST_REQUIRE( is_sorted( seq1.begin(), seq1.end() ) );
}

BOOST_AUTO_TEST_CASE( test_linear_seq_seq )
{
  const size_t N = 20000;
  Seque< size_t > seq0( N );
  Seque< size_t > seq1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    seq0[ i ] = N - i - 1;
    seq1[ i ] = N - i - 1;
  }

  nest::sort( seq0, seq1 );

  BOOST_REQUIRE( is_sorted( seq0.begin(), seq0.end() ) );
  BOOST_REQUIRE( is_sorted( seq1.begin(), seq1.end() ) );
}

BOOST_AUTO_TEST_SUITE_END()

} // of namespace nest

#endif /* TEST_SORT_H */
