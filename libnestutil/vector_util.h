/*
 *  vector_util.h
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

#ifndef VECTOR_UTIL_H
#define VECTOR_UTIL_H

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace vector_util
{

template < typename T >
inline void
grow( std::vector< T >& v )
{
  // set maximal growth of vector to 256MiB; this allows for fast
  // growth while the vector is small, but limits capacity wasted
  // while growing large vectors; value determined by experimenting
  // with different max block sizes
  const size_t max_block_size_MiB = 256;
  const size_t max_block_size =
    static_cast< size_t >( max_block_size_MiB * ( 2 << 20 ) / static_cast< double >( sizeof( T ) ) );

  if ( v.size() == v.capacity() )
  {
    v.reserve( v.size() < max_block_size ? 2 * v.size() : ( v.size() + max_block_size ) );
  }
}

template < typename T >
using Predicate = std::function< bool( const T&, const T& ) >;


template < typename T >
using Filter = std::function< bool( const T& ) >;


template < typename Container >
inline std::vector< std::pair< size_t, size_t > >
split_into_contiguous_slices(
  const Container& container,
  const bool& useIndex = false,
  Predicate< typename Container::value_type > pred = []( const auto& current, const auto& next )
  { return next == current + 1; },
  Filter< typename Container::value_type > filter = []( const auto& ) { return true; } )
{
  if ( container.empty() )
  {
    return {};
  }

  auto get_value = [ &container, &useIndex ]( size_t index ) -> size_t
  {
    if ( index > container.size() and !useIndex )
    {
      return container[ index - 1 ] + 1;
    }
    return useIndex ? index : static_cast< size_t >( container[ index ] );
  };

  if ( container.size() == 1 )
  {
    return { std::make_pair( get_value( 0 ), get_value( 1 ) ) };
  }

  auto ret = std::vector< std::pair< size_t, size_t > > {};
  ret.reserve( container.size() );

  size_t current = 0;

  for ( size_t index = 1; index < container.size(); index++ )
  {
    if ( not pred( container[ current ], container[ index ] ) )
    {
      if ( filter( container[ current ] ) )
      {
        ret.emplace_back( get_value( current ), get_value( index ) );
      }
      current = index;
    }
    if ( index + 1 == container.size() and filter( container[ current ] ) )
    {
      ret.emplace_back( get_value( current ), get_value( index + 1 ) );
    }
  }

  return ret;
}
} // namespace vector_util

#endif // VECTOR_UTIL_H
