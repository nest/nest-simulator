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

#ifndef STOPWATCH_IMPL_H
#define STOPWATCH_IMPL_H

#include "kernel_manager.h"
#include "stopwatch.h"

namespace nest
{

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
void
Stopwatch< detailed_timer, threaded_timer >::start()
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_thread_parallel();
    if constexpr ( use_timer_array )
    {
      walltime_timer_[ kernel().vp_manager.get_thread_id() ].start();
      cputime_timer_[ kernel().vp_manager.get_thread_id() ].start();
    }
    else
    {
#pragma omp master
      {
        walltime_timer_.start();
        cputime_timer_.start();
      }
    }
  }
}

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
void
Stopwatch< detailed_timer, threaded_timer >::stop()
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_thread_parallel();
    if constexpr ( use_timer_array )
    {
      walltime_timer_[ kernel().vp_manager.get_thread_id() ].stop();
      cputime_timer_[ kernel().vp_manager.get_thread_id() ].stop();
    }
    else
    {
#pragma omp master
      {
        walltime_timer_.stop();
        cputime_timer_.stop();
      }
    }
  }
}

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
bool
Stopwatch< detailed_timer, threaded_timer >::is_running_() const
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_thread_parallel();
    if constexpr ( use_timer_array )
    {
      return walltime_timer_[ kernel().vp_manager.get_thread_id() ].is_running_();
    }
    else
    {
      bool is_running_ = false;
#pragma omp master
      {
        is_running_ = walltime_timer_.is_running_();
      };
      return is_running_;
    }
  }
  return false;
}

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
double
Stopwatch< detailed_timer, threaded_timer >::elapsed( timers::timeunit_t timeunit ) const
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_thread_parallel();
    if constexpr ( use_timer_array )
    {
      return walltime_timer_[ kernel().vp_manager.get_thread_id() ].elapsed( timeunit );
    }
    else
    {
      double elapsed = 0.;
#pragma omp master
      {
        elapsed = walltime_timer_.elapsed( timeunit );
      };
      return elapsed;
    }
  }
  return std::numeric_limits< double >().quiet_NaN();
}

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
void
Stopwatch< detailed_timer, threaded_timer >::print( const std::string& msg,
  timers::timeunit_t timeunit,
  std::ostream& os ) const
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_thread_parallel();
    if constexpr ( use_timer_array )
    {
      walltime_timer_[ kernel().vp_manager.get_thread_id() ].print( msg, timeunit, os );
    }
    else
    {
#pragma omp master
      {
        walltime_timer_.print( msg, timeunit, os );
      }
    }
  }
}
template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
void
Stopwatch< detailed_timer, threaded_timer >::get_status( DictionaryDatum& d,
  const Name& walltime_name,
  const Name& cputime_name ) const
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_single_threaded();
    if constexpr ( use_timer_array )
    {
      std::vector< double > wall_times( walltime_timer_.size() );
      std::transform( walltime_timer_.begin(),
        walltime_timer_.end(),
        wall_times.begin(),
        []( const timers::StopwatchTimer< CLOCK_MONOTONIC >& timer ) { return timer.elapsed(); } );
      def< ArrayDatum >( d, walltime_name, ArrayDatum( wall_times ) );

      std::vector< double > cpu_times( cputime_timer_.size() );
      std::transform( cputime_timer_.begin(),
        cputime_timer_.end(),
        cpu_times.begin(),
        []( const timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID >& timer ) { return timer.elapsed(); } );
      def< ArrayDatum >( d, cputime_name, ArrayDatum( cpu_times ) );
    }
    else
    {
      def< double >( d, walltime_name, walltime_timer_.elapsed() );
      def< double >( d, cputime_name, cputime_timer_.elapsed() );
    }
  }
}

template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer >
void
Stopwatch< detailed_timer, threaded_timer >::reset()
{
  if constexpr ( enable_timer )
  {
    kernel().vp_manager.assert_single_threaded();
    if constexpr ( use_timer_array )
    {
      const size_t num_threads = kernel().vp_manager.get_num_threads();
      walltime_timer_.resize( num_threads );
      cputime_timer_.resize( num_threads );
      for ( size_t i = 0; i < num_threads; ++i )
      {
        walltime_timer_[ i ].reset();
        cputime_timer_[ i ].reset();
      }
    }
    else
    {
#pragma omp master
      {
        walltime_timer_.reset();
        cputime_timer_.reset();
      }
    }
  }
}
}

#endif
