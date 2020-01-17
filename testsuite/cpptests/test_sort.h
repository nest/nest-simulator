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

void
nest_quicksort( BlockVector< size_t >& bv0, BlockVector< size_t >& bv1 )
{
  nest::quicksort3way( bv0, bv1, 0, bv0.size() - 1 );
}

bool
is_sorted( BlockVector< size_t >::const_iterator begin, BlockVector< size_t >::const_iterator end )
{
  for ( BlockVector< size_t >::const_iterator it = begin; it < --end; )
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
  BlockVector< size_t > bv0( N );
  BlockVector< size_t > bv1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    const size_t k = std::rand() % N;
    bv0[ i ] = k;
    bv1[ i ] = k;
  }

  nest_quicksort( bv0, bv1 );

  BOOST_REQUIRE( is_sorted( bv0.begin(), bv0.end() ) );
  BOOST_REQUIRE( is_sorted( bv1.begin(), bv1.end() ) );
}

/**
 * Tests whether two arrays with linearly increasing numbers are sorted
 * correctly by a single call to sort.
 */
BOOST_AUTO_TEST_CASE( test_linear )
{
  const size_t N = 20000;
  BlockVector< size_t > bv0( N );
  BlockVector< size_t > bv1( N );

  for ( size_t i = 0; i < N; ++i )
  {
    bv0[ i ] = N - i - 1;
    bv1[ i ] = N - i - 1;
  }

  nest_quicksort( bv0, bv1 );

  BOOST_REQUIRE( is_sorted( bv0.begin(), bv0.end() ) );
  BOOST_REQUIRE( is_sorted( bv1.begin(), bv1.end() ) );
}

BOOST_AUTO_TEST_SUITE_END()

} // of namespace nest

#endif /* TEST_SORT_H */
