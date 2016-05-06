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

namespace sort
{
  template < typename T >
  inline void exchange_( std::vector< T >& vec, int i, int j )
  {
    T tmp = vec[ i ];
    vec[ i ] = vec[ j ];
    vec[ j ] = tmp;
  }

  template < typename T >
  inline short compare_( T& lhs, T& rhs )
  {
    if ( lhs < rhs )
    {
      return -1;
    }
    else if ( lhs > rhs )
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  template < typename T >
  void sort( std::vector< T >& vec, std::vector< size_t >& perm, int lo, int hi )
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
      short cmp = compare_( vec[ i ], vec[ pos ] );
      if ( cmp < 0 )
      {
        ++pos;
        exchange_( vec, lt, i );
        exchange_( perm, lt, i );
        ++lt;
        ++i;
      }
      else if ( cmp > 0 )
      {
        exchange_( vec, i, gt );
        exchange_( perm, i, gt );
        --gt;
      }
      else
      {
        i++;
      }
    }
    sort( vec, perm, lo, lt - 1 );
    sort( vec, perm, gt + 1, hi );
  }

  template< typename T >
  std::vector< T > apply_permutation( const std::vector< T >& vec, const std::vector< std::size_t >& perm )
  {
    std::vector< T > sorted_vec( perm.size() );
    for ( unsigned int i = 0; i < perm.size(); ++i )
    {
      sorted_vec[ i ] = vec[ perm[ i ] ];
    }
    return sorted_vec;
  }

} // namespace sort

#endif // SORT_H
