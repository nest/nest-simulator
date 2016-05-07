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

/* Quicksort with 3-way partitioning, adapted from Sedgewick & Wayne
 * (2011), Algorithms 4th edition, p296ff */

namespace sort
{
  template < typename T >
  inline void exchange_( std::vector< T >& vec, int i, int j )
  {
    T tmp = vec[ i ];
    vec[ i ] = vec[ j ];
    vec[ j ] = tmp;
  }

  /* recursively sorts the two vectors vec_sort and vec_perm, by
   * sorting the entries in vec_sort and applying the same exchanges
   * to vec_perm */
  template < typename T1, typename T2 >
  void sort( std::vector< T1 >& vec_sort, std::vector< T2 >& vec_perm, int lo, int hi )
  {
    if ( hi <= lo )
    {
      return;
    }
    int lt = lo;
    int i = lo + 1;
    int gt = hi;
    int pos = lo;
    while ( i <= gt )
    {
      if ( vec_sort[ i ] < vec_sort[ pos ] )
      {
        ++pos;
        exchange_( vec_sort, lt, i );
        exchange_( vec_perm, lt, i );
        ++lt;
        ++i;
      }
      else if ( vec_sort[ i ] > vec_sort[ pos ] )
      {
        exchange_( vec_sort, i, gt );
        exchange_( vec_perm, i, gt );
        --gt;
      }
      else
      {
        i++;
      }
    }
    sort( vec_sort, vec_perm, lo, lt - 1 );
    sort( vec_sort, vec_perm, gt + 1, hi );
  }

} // namespace sort

#endif // SORT_H
