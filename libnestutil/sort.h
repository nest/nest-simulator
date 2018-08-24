/*
 *  sort.h
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

#ifndef SORT_H
#define SORT_H

#include <vector>
#include <cstddef>

#define INSERTION_SORT_CUTOFF 10 // use insertion sort for smaller arrays

namespace nest
{
/**
 * Exchanges elements i and j in vector vec.
 */
template < typename T >
inline void
exchange_( std::vector< T >& vec, const size_t i, const size_t j )
{
  const T tmp = vec[ i ];
  vec[ i ] = vec[ j ];
  vec[ j ] = tmp;
}

/**
 * Calculates the median of three elements.
 * See http://algs4.cs.princeton.edu/23quicksort/QuickX.java.html.
 */
template < typename T >
inline size_t
median3_( const std::vector< T >& vec,
  const size_t i,
  const size_t j,
  const size_t k )
{
  return ( ( vec[ i ] < vec[ j ] )
      ? ( ( vec[ j ] < vec[ k ] ) ? j : ( vec[ i ] < vec[ k ] ) ? k : i )
      : ( ( vec[ k ] < vec[ j ] ) ? j : ( vec[ k ] < vec[ i ] ) ? k : i ) );
}

/**
 * Insertion sort, adapted from Sedgewick & Wayne
 * (2011), Algorithms 4th edition, p251ff.
 * Sorts the two vectors vec_sort and vec_perm, by sorting the
 * entries in vec_sort and applying the same exchanges to
 * vec_perm.
 */
template < typename T1, typename T2 >
void
insertion_sort( std::vector< T1 >& vec_sort,
  std::vector< T2 >& vec_perm,
  const size_t lo,
  const size_t hi )
{
  for ( size_t i = lo + 1; i < hi + 1; ++i )
  {
    for ( size_t j = i; ( j > lo ) and ( vec_sort[ j ] < vec_sort[ j - 1 ] );
          --j )
    {
      exchange_( vec_sort, j, j - 1 );
      exchange_( vec_perm, j, j - 1 );
    }
  }
}

/**
 * Quicksort with 3-way partitioning, adapted from Sedgewick & Wayne
 * (2011), Algorithms 4th edition, p296ff
 * (see http://algs4.cs.princeton.edu/23quicksort/QuickX.java.html).
 *
 * Recursively sorts the two vectors vec_sort and vec_perm, by
 * sorting the entries in vec_sort and applying the same exchanges
 * to vec_perm.
 */
template < typename T1, typename T2 >
void
quicksort3way( std::vector< T1 >& vec_sort,
  std::vector< T2 >& vec_perm,
  const size_t lo,
  const size_t hi )
{
  if ( lo >= hi )
  {
    return;
  }

  const size_t n = hi - lo + 1;

  // switch to insertion sort for small arrays
  if ( n <= INSERTION_SORT_CUTOFF )
  {
    insertion_sort( vec_sort, vec_perm, lo, hi );
    return;
  }

  // use median-of-3 as partitioning element
  size_t m = median3_( vec_sort, lo, lo + n / 2, hi );

  // in case of many equal entries, make sure to use first entry with
  // this value (useful for sorted arrays)
  const T1 m_val = vec_sort[ m ];
  while ( m > 0 and vec_sort[ m - 1 ] == m_val )
  {
    --m;
  }

  // move pivot to the front
  exchange_( vec_sort, m, lo );
  exchange_( vec_perm, m, lo );

  // Dijkstra's three-way-sort
  size_t lt = lo;
  size_t i = lo + 1;
  size_t gt = hi;
  const T1 v = vec_sort[ lt ]; // pivot

  // adjust position of i and lt (useful for sorted arrays)
  while ( vec_sort[ i ] < v )
  {
    ++i;
  }
  exchange_( vec_sort, lo, i - 1 );
  exchange_( vec_perm, lo, i - 1 );
  lt = i - 1;

  // adjust position of gt (useful for sorted arrays)
  while ( vec_sort[ gt ] > v )
  {
    --gt;
  }

  while ( i <= gt )
  {
    if ( vec_sort[ i ] < v )
    {
      exchange_( vec_sort, lt, i );
      exchange_( vec_perm, lt, i );
      ++lt;
      ++i;
    }
    else if ( vec_sort[ i ] > v )
    {
      exchange_( vec_sort, i, gt );
      exchange_( vec_perm, i, gt );
      --gt;
    }
    else
    {
      ++i;
    }
  }

  quicksort3way( vec_sort, vec_perm, lo, lt - 1 );
  quicksort3way( vec_sort, vec_perm, gt + 1, hi );
}

/**
 * Sorts two vectors according to elements in
 * first vector. Convenience function.
 */
template < typename T1, typename T2 >
void
sort( std::vector< T1 >& vec_sort, std::vector< T2 >& vec_perm )
{
  quicksort3way( vec_sort, vec_perm, 0, vec_sort.size() - 1 );
}

} // namespace sort

#endif /* #ifndef SORT_H */
