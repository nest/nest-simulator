/*
 *  ring_buffer_impl.h
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

#ifndef RING_BUFFER_IMPL_H
#define RING_BUFFER_IMPL_H

#include "ring_buffer.h"

template < unsigned int num_channels >
nest::MultiChannelInputBuffer< num_channels >::MultiChannelInputBuffer()
  : buffer_( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay(),
      std::array< double, num_channels >() )
{
}

template < unsigned int num_channels >
void
nest::MultiChannelInputBuffer< num_channels >::resize()
{
  const size_t size = kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
  if ( buffer_.size() != size )
  {
    buffer_.resize( size, std::array< double, num_channels >() );
  }
}

template < unsigned int num_channels >
void
nest::MultiChannelInputBuffer< num_channels >::clear()
{
  resize(); // does nothing if size is fine
  // set all elements to 0.0
  for ( index slot = 0; slot < buffer_.size(); ++slot )
  {
    reset_values_all_channels( slot );
  }
}

#endif
