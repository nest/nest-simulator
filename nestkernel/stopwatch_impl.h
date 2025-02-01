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
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].start();
  cputime_timers_[ kernel().vp_manager.get_thread_id() ].start();
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::stop()
{
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].stop();
  cputime_timers_[ kernel().vp_manager.get_thread_id() ].stop();
}

template < StopwatchVerbosity detailed_timer >
bool
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::isRunning() const
{
  kernel().vp_manager.assert_thread_parallel();

  return walltime_timers_[ kernel().vp_manager.get_thread_id() ].isRunning();
}

template < StopwatchVerbosity detailed_timer >
double
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::elapsed( timers::timeunit_t
    timeunit ) const
{
  kernel().vp_manager.assert_thread_parallel();

  return walltime_timers_[ kernel().vp_manager.get_thread_id() ].elapsed( timeunit );
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::print( const char* msg,
  timers::timeunit_t timeunit,
  std::ostream& os ) const
{
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].print( msg, timeunit, os );
}

template < StopwatchVerbosity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchType::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchVerbosity::Normal or use_detailed_timers ) > >::reset()
{
  kernel().vp_manager.assert_single_threaded();

  const size_t num_threads = kernel().vp_manager.get_num_threads();
  walltime_timers_.resize( num_threads );
  cputime_timers_.resize( num_threads );
  for ( size_t i = 0; i < num_threads; ++i )
  {
    walltime_timers_[ i ].reset();
    cputime_timers_[ i ].reset();
  }
}
}
