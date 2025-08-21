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

// Includes from nestkernel:
#include "dictutils.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "simulation_manager.h"

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
  time_steps_.emplace_back( kernel::manager< SimulationManager >.get_clock().get_steps() );
  global_max_spikes_sent_.emplace_back( global_max_spikes_sent );
  new_buffer_size_.emplace_back( new_buffer_size );
}

void
BufferResizeLog::to_dict( DictionaryDatum& events ) const
{
  initialize_property_intvector( events, names::times );
  append_property( events, names::times, time_steps_ );
  initialize_property_intvector( events, "global_max_spikes_sent" );
  append_property( events, "global_max_spikes_sent", global_max_spikes_sent_ );
  initialize_property_intvector( events, "new_buffer_size" );
  append_property( events, "new_buffer_size", new_buffer_size_ );
}

}
