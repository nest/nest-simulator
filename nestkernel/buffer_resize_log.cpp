/*
 *  buffer_resize_log.cpp
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

#include "buffer_resize_log.h"

// Includes from libnestutil
#include "dict_util.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"

namespace nest
{

BufferResizeLog::BufferResizeLog()
  : time_steps_()
  , global_max_spikes_sent_()
  , new_buffer_size_()
{
}

void
BufferResizeLog::clear()
{
  time_steps_.clear();
  global_max_spikes_sent_.clear();
  new_buffer_size_.clear();
}

void
BufferResizeLog::add_entry( size_t global_max_spikes_sent, size_t new_buffer_size )
{
  time_steps_.emplace_back( kernel().simulation_manager.get_clock().get_steps() );
  global_max_spikes_sent_.emplace_back( global_max_spikes_sent );
  new_buffer_size_.emplace_back( new_buffer_size );
}

void
BufferResizeLog::to_dict( Dictionary& events ) const
{
  auto& times = events.get_vector< int >( names::times );
  times.insert( times.end(), time_steps_.begin(), time_steps_.end() );

  auto& gmss = events.get_vector< int >( names::global_max_spikes_sent );
  gmss.insert( gmss.end(), global_max_spikes_sent_.begin(), global_max_spikes_sent_.end() );

  auto& nbs = events.get_vector< int >( names::new_buffer_size );
  nbs.insert( nbs.end(), new_buffer_size_.begin(), new_buffer_size_.end() );
}

}
