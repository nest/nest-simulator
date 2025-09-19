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

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::start()
{
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].start();
  cputime_timers_[ kernel().vp_manager.get_thread_id() ].start();
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::stop()
{
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].stop();
  cputime_timers_[ kernel().vp_manager.get_thread_id() ].stop();
}

template < StopwatchGranularity detailed_timer >
bool
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::is_running_() const
{
  kernel().vp_manager.assert_thread_parallel();

  return walltime_timers_[ kernel().vp_manager.get_thread_id() ].is_running_();
}

template < StopwatchGranularity detailed_timer >
double
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::elapsed( timers::timeunit_t timeunit ) const
{
  kernel().vp_manager.assert_thread_parallel();

  return walltime_timers_[ kernel().vp_manager.get_thread_id() ].elapsed( timeunit );
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::print( const std::string& msg,
  timers::timeunit_t timeunit,
  std::ostream& os ) const
{
  kernel().vp_manager.assert_thread_parallel();

  walltime_timers_[ kernel().vp_manager.get_thread_id() ].print( msg, timeunit, os );
}
template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::get_status( DictionaryDatum& d,
  const Name& walltime_name,
  const Name& cputime_name ) const

{
  std::vector< double > wall_times( walltime_timers_.size() );
  std::transform( walltime_timers_.begin(),
    walltime_timers_.end(),
    wall_times.begin(),
    []( const timers::StopwatchTimer< CLOCK_MONOTONIC >& timer ) { return timer.elapsed(); } );
  def< ArrayDatum >( d, walltime_name, ArrayDatum( wall_times ) );

  std::vector< double > cpu_times( cputime_timers_.size() );
  std::transform( cputime_timers_.begin(),
    cputime_timers_.end(),
    cpu_times.begin(),
    []( const timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID >& timer ) { return timer.elapsed(); } );
  def< ArrayDatum >( d, cputime_name, ArrayDatum( cpu_times ) );
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer, StopwatchParallelism::Threaded >::reset()
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
