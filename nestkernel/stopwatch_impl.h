/*
 *  stopwatch_impl.h
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

#include "kernel_manager.h"
#include "stopwatch.h"

namespace nest
{

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::start()
{
  timers_[ kernel().vp_manager.get_thread_id() ].start();
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::stop()
{
  timers_[ kernel().vp_manager.get_thread_id() ].stop();
}

template < StopwatchVerbosity detailed_timer >
bool
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::isRunning() const
{
  return timers_[ kernel().vp_manager.get_thread_id() ].isRunning();
}

template < StopwatchVerbosity detailed_timer >
double
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::elapsed( StopwatchBase::timeunit_t
    timeunit ) const
{
  return timers_[ kernel().vp_manager.get_thread_id() ].elapsed( timeunit );
}

template < StopwatchVerbosity detailed_timer >
StopwatchBase::timestamp_t
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::elapsed_timestamp() const
{
  return timers_[ kernel().vp_manager.get_thread_id() ].elapsed_timestamp();
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::print( const char* msg,
  StopwatchBase::timeunit_t timeunit,
  std::ostream& os ) const
{
  timers_[ kernel().vp_manager.get_thread_id() ].print( msg, timeunit, os );
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::reset()
{
  const size_t num_threads = kernel().vp_manager.get_num_threads();
  timers_.resize( num_threads );
  for ( size_t i = 0; i < num_threads; ++i )
  {
    timers_[ i ].reset();
  }
}
}
