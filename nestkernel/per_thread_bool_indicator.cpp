/*
 *  per_thread_bool_indicator.cpp
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

#include "per_thread_bool_indicator.h"

// Includes from nestkernel
#include "kernel_manager.h"

namespace nest
{

BoolIndicatorUInt64::BoolIndicatorUInt64()
  : status_( false_uint64 )
{
}

BoolIndicatorUInt64::BoolIndicatorUInt64( const bool status )
  : status_( status )
{
}

BoolIndicatorUInt64&
PerThreadBoolIndicator::operator[]( const size_t tid )
{
  return per_thread_status_[ tid ];
}

void
PerThreadBoolIndicator::initialize( const size_t num_threads, const bool status )
{
  kernel().vp_manager.assert_single_threaded();
  per_thread_status_.clear();
  per_thread_status_.resize( num_threads, BoolIndicatorUInt64( status ) );
  size_ = num_threads;
  if ( status )
  {
    are_true_ = num_threads;
  }
  else
  {
    are_true_ = 0;
  }
}

bool
PerThreadBoolIndicator::all_false() const
{
  DETAILED_TIMER_START( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
// We need two barriers here to ensure that no thread can continue and change the result
// before all threads have determined the result.
#pragma omp barrier
  // We need two barriers here to ensure that no thread can continue and change the result
  // before all threads have determined the result.
  bool ret = ( are_true_ == 0 );
#pragma omp barrier

  DETAILED_TIMER_STOP( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
  return ret;
}

bool
PerThreadBoolIndicator::all_true() const
{
  DETAILED_TIMER_START( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
#pragma omp barrier
  bool ret = ( are_true_ == size_ );
#pragma omp barrier
  DETAILED_TIMER_STOP( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
  return ret;
}

bool
PerThreadBoolIndicator::any_false() const
{
  DETAILED_TIMER_START( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
#pragma omp barrier
  bool ret = ( are_true_ < size_ );
#pragma omp barrier

  DETAILED_TIMER_STOP( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
  return ret;
}

bool
PerThreadBoolIndicator::any_true() const
{
  DETAILED_TIMER_START( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
#pragma omp barrier
  bool ret = ( are_true_ > 0 );
#pragma omp barrier

  DETAILED_TIMER_STOP( kernel().simulation_manager.get_idle_stopwatch(), kernel().vp_manager.get_thread_id() );
  return ret;
}

} // namespace nest
