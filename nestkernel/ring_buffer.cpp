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

#include <list>
#include <vector>

#include "connection_manager.h"
#include "kernel_manager.h"
#include "ring_buffer.h"

namespace nest
{

RingBuffer::RingBuffer()
  : min_delay_( kernel::manager< ConnectionManager >.get_min_delay() )
  , buffer_( min_delay_ + kernel::manager< ConnectionManager >.get_max_delay(), 0.0 )
{
}

void
RingBuffer::resize()
{
  min_delay_ = kernel::manager< ConnectionManager >.get_min_delay();
  const size_t size = min_delay_ + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
RingBuffer::clear()
{
  resize();  // does nothing if size is fine
  // clear all elements
  buffer_.assign( buffer_.size(), 0.0 );
}


MultRBuffer::MultRBuffer()
  : min_delay_( kernel::manager< ConnectionManager >.get_min_delay() )
  , buffer_( min_delay_ + kernel::manager< ConnectionManager >.get_max_delay(), 0.0 )
{
}

void
MultRBuffer::resize()
{
  min_delay_ = kernel::manager< ConnectionManager >.get_min_delay();
  const size_t size = min_delay_ + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
MultRBuffer::clear()
{
  // clear all elements
  buffer_.assign( buffer_.size(), 0.0 );
}


ListRingBuffer::ListRingBuffer()
  : min_delay_( kernel::manager< ConnectionManager >.get_min_delay() )
  , buffer_( min_delay_ + kernel::manager< ConnectionManager >.get_max_delay() )
{
}

void
ListRingBuffer::resize()
{
  min_delay_ = kernel::manager< ConnectionManager >.get_min_delay();
  const size_t size = min_delay_ + kernel::manager< ConnectionManager >.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size );
  }
}

void
ListRingBuffer::clear()
{
  resize();  // does nothing if size is fine
  // clear all elements
  for ( unsigned int i = 0; i < buffer_.size(); i++ )
  {
    buffer_[ i ].clear();
  }
}

}  // namespace nest
