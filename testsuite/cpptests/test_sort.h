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

BOOST_AUTO_TEST_SUITE( test_sort )

/**
 * Tests whether two arrays with randomly generated numbers are sorted
 * correctly by a single call to sort.
 */
BOOST_AUTO_TEST_CASE( test_random )
{
  const size_t N = 20000;
  std::vector< size_t > vec0( N );
  std::vector< size_t > vec1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    const size_t k = std::rand() % N;
    vec0[ i ] = k;
    vec1[ i ] = k;
  }

  nest::sort( vec0, vec1 );

  BOOST_REQUIRE( is_sorted( vec0.begin(), vec0.end() ) );
  BOOST_REQUIRE( is_sorted( vec1.begin(), vec1.end() ) );
}

/**
 * Tests whether two arrays with linearly increasing numbers are sorted
 * correctly by a single call to sort.
*/
BOOST_AUTO_TEST_CASE( test_linear )
{
  const size_t N = 20000;
  std::vector< size_t > vec0( N );
  std::vector< size_t > vec1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    vec0[ i ] = N - i - 1;
    vec1[ i ] = N - i - 1;
  }

  nest::sort( vec0, vec1 );

  BOOST_REQUIRE( is_sorted( vec0.begin(), vec0.end() ) );
  BOOST_REQUIRE( is_sorted( vec1.begin(), vec1.end() ) );
}

BOOST_AUTO_TEST_SUITE_END()

} // of namespace nest

#endif /* TEST_SORT_H */
