/*
 *  slice_ring_buffer.cpp
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

#include "slice_ring_buffer.h"

// C++ includes:
#include <cmath>
#include <limits>

nest::SliceRingBuffer::SliceRingBuffer()
  : refract_( std::numeric_limits< long >::max(), 0, 0 )
{
}

void
nest::SliceRingBuffer::resize()
{
  // We want to compute ceil( ( d_min + d_max ) / d_min ) = 1 + ceil( d_max / d_min )
  // and can do so safely without casting to double.
  const size_t d_min = kernel().connection_manager.get_min_delay();
  const size_t d_max = kernel().connection_manager.get_max_delay();
  const size_t new_size = 1 + ( d_max + d_min - 1 ) / d_min;

  if ( queue_.size() != new_size )
  {
    queue_.resize( new_size );
    clear();
  }

#ifndef HAVE_STL_VECTOR_CAPACITY_BASE_UNITY
  // Ensure capacity doubling starts from capacity 1.
  for ( auto& q : queue_ )
  {
    q.reserve( 1 );
  }
#endif
}

void
nest::SliceRingBuffer::clear()
{
  for ( auto& q : queue_ )
  {
    q.clear();
  }
}

void
nest::SliceRingBuffer::prepare_delivery()
{
  // vector to deliver from in this slice
  deliver_ = &( queue_[ kernel().event_delivery_manager.get_slice_modulo( 0 ) ] );

  // sort events, first event last
  std::sort( deliver_->begin(), deliver_->end(), std::greater< SpikeInfo >() );
}

void
nest::SliceRingBuffer::discard_events()
{
  // vector to deliver from in this slice
  deliver_ = &( queue_[ kernel().event_delivery_manager.get_slice_modulo( 0 ) ] );

  deliver_->clear();
}
