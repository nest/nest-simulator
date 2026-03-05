/*
 *  cycle_time_log.cpp
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

#include "cycle_time_log.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"

namespace nest
{

CycleTimeLog::CycleTimeLog()
  : cycle_update_time_()
  , communicate_time_()
  , local_spike_counter_()
{
}

void
CycleTimeLog::clear()
{
  cycle_update_time_.clear();
  communicate_time_.clear();
  local_spike_counter_.clear();
}

void
CycleTimeLog::add_entry( double cycle_update_time, double communicate_time, long local_spike_counter )
{
  cycle_update_time_.emplace_back( cycle_update_time );
  communicate_time_.emplace_back( communicate_time );
  local_spike_counter_.emplace_back( local_spike_counter );
}

void
CycleTimeLog::to_dict( Dictionary& events ) const
{
  // initialize_property_doublevector( events, names::times );
  // append_property( events, names::times, cycle_update_time_ );
  auto& times = events.get_vector< int >( names::times );
  times.insert( times.end(), cycle_update_time_.begin(), cycle_update_time_.end() );

  // initialize_property_doublevector( events, "communicate_time" );
  // append_property( events, "communicate_time", communicate_time_ );
  auto& communicate_time = events.get_vector< int >( names::communicate_time );
  communicate_time.insert( communicate_time.end(), communicate_time_.begin(), communicate_time_.end() );

  // initialize_property_intvector( events, "local_spike_counter" );
  // append_property( events, "local_spike_counter", local_spike_counter_ );
  auto& local_spike_counter = events.get_vector< int >( names::local_spike_counter );
  times.insert( local_spike_counter.end(), local_spike_counter_.begin(), local_spike_counter_.end() );
}

}
