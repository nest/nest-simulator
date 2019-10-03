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
  //  resize();  // sets up queue_
}

void
nest::SliceRingBuffer::resize()
{
  long newsize = static_cast< long >( std::ceil(
    static_cast< double >( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay() )
    / kernel().connection_manager.get_min_delay() ) );
  if ( queue_.size() != static_cast< unsigned long >( newsize ) )
  {
    queue_.resize( newsize );
    clear();
  }

#ifndef HAVE_STL_VECTOR_CAPACITY_BASE_UNITY
  // create 1-element buffers
  for ( size_t j = 0; j < queue_.size(); ++j )
  {
    queue_[ j ].reserve( 1 );
  }
#endif
}

void
nest::SliceRingBuffer::clear()
{
  for ( size_t j = 0; j < queue_.size(); ++j )
  {
    queue_[ j ].clear();
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
