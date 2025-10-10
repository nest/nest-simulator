/*
 *  ring_buffer.cpp
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

#include "connection_manager.h"
#include "ring_buffer_impl.h"

nest::RingBuffer::RingBuffer()
  : buffer_(
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay(),
    0.0 )
{
}

void
nest::RingBuffer::resize()
{
  size_t size =
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
nest::RingBuffer::clear()
{
  resize(); // does nothing if size is fine
  // clear all elements
  buffer_.assign( buffer_.size(), 0.0 );
}


nest::MultRBuffer::MultRBuffer()
  : buffer_(
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay(),
    0.0 )
{
}

void
nest::MultRBuffer::resize()
{
  size_t size =
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
nest::MultRBuffer::clear()
{
  // clear all elements
  buffer_.assign( buffer_.size(), 0.0 );
}


nest::ListRingBuffer::ListRingBuffer()
  : buffer_(
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay() )
{
}

void
nest::ListRingBuffer::resize()
{
  size_t size =
    kernel::manager< ConnectionManager >.get_min_delay() + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
nest::ListRingBuffer::clear()
{
  resize(); // does nothing if size is fine
  // clear all elements
  for ( unsigned int i = 0; i < buffer_.size(); i++ )
  {
    buffer_[ i ].clear();
  }
}


void
nest::RingBuffer::add_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ] += v;
}

void
nest::RingBuffer::set_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ] = v;
}

double
nest::RingBuffer::get_value( const long offs )
{
  assert( 0 <= offs and static_cast< size_t >( offs ) < buffer_.size() );
  assert( offs < kernel::manager< ConnectionManager >.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  buffer_[ idx ] = 0.0; // clear buffer after reading
  return val;
}

double
nest::RingBuffer::get_value_wfr_update( const long offs )
{
  assert( 0 <= offs and static_cast< size_t >( offs ) < buffer_.size() );
  assert( offs < kernel::manager< ConnectionManager >.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  return val;
}

size_t
nest::RingBuffer::get_index_( const long d ) const
{
  const long idx = kernel::manager< EventDeliveryManager >.get_modulo( d );
  assert( 0 <= idx );
  assert( static_cast< size_t >( idx ) < buffer_.size() );
  return idx;
}


void
nest::MultRBuffer::add_value( const long offs, const double v )
{
  assert( 0 <= offs and static_cast< size_t >( offs ) < buffer_.size() );
  buffer_[ get_index_( offs ) ] *= v;
}

double
nest::MultRBuffer::get_value( const long offs )
{
  assert( 0 <= offs and static_cast< size_t >( offs ) < buffer_.size() );
  assert( offs < kernel::manager< ConnectionManager >.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  buffer_[ idx ] = 0.0; // clear buffer after reading
  return val;
}

size_t
nest::MultRBuffer::get_index_( const long d ) const
{
  const long idx = kernel::manager< EventDeliveryManager >.get_modulo( d );
  assert( 0 <= idx and static_cast< size_t >( idx ) < buffer_.size() );
  return idx;
}

void
nest::ListRingBuffer::append_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ].push_back( v );
}

std::list< double >&
nest::ListRingBuffer::get_list( const long offs )
{
  assert( 0 <= offs and static_cast< size_t >( offs ) < buffer_.size() );
  assert( offs < kernel::manager< ConnectionManager >.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  return buffer_[ idx ];
}

size_t
nest::ListRingBuffer::get_index_( const long d ) const
{
  const long idx = kernel::manager< EventDeliveryManager >.get_modulo( d );
  assert( 0 <= idx );
  assert( static_cast< size_t >( idx ) < buffer_.size() );
  return idx;
}
