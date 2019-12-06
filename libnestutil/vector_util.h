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

#include <vector>
#include <cstddef>

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

} // namespace vector_util

#endif // VECTOR_UTIL_H
