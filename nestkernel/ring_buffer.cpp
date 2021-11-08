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

#include "ring_buffer.h"

nest::RingBuffer::RingBuffer()
  : buffer_( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay(), 0.0 )
{
}

void
nest::RingBuffer::resize()
{
  size_t size = kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
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
  : buffer_( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay(), 0.0 )
{
}

void
nest::MultRBuffer::resize()
{
  size_t size = kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
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
  : buffer_( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay() )
{
}

void
nest::ListRingBuffer::resize()
{
  size_t size = kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
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
