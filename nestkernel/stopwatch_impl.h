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

#include "stopwatch.h"


namespace nest
{
namespace timers{


template < clockid_t clock_type >
inline void
StopwatchTimer< clock_type >::start()
{
#ifndef DISABLE_TIMING
  if ( not is_running_() )
  {
    _prev_elapsed += _end - _beg;     // store prev. time, if we resume
    _end = _beg = get_current_time(); // invariant: _end >= _beg
    _running = true;                  // we start running
  }
#endif
}

template < clockid_t clock_type >
inline void
StopwatchTimer< clock_type >::stop()
{
#ifndef DISABLE_TIMING
  if ( is_running_() )
  {
    _end = get_current_time(); // invariant: _end >= _beg
    _running = false;          // we stopped running
  }
#endif
}

template < clockid_t clock_type >
inline bool
StopwatchTimer< clock_type >::is_running_() const
{
#ifndef DISABLE_TIMING
  return _running;
#else
  return false;
#endif
}

template < clockid_t clock_type >
inline double
StopwatchTimer< clock_type >::elapsed( timeunit_t timeunit ) const
{
#ifndef DISABLE_TIMING
  size_t time_elapsed;
  if ( is_running_() )
  {
    // get intermediate elapsed time; do not change _end, to be const
    time_elapsed = get_current_time() - _beg + _prev_elapsed;
  }
  else
  {
    // stopped before, get time of current measurement + last measurements
    time_elapsed = _end - _beg + _prev_elapsed;
  }
  return static_cast< double >( time_elapsed ) / static_cast< double >( timeunit );
#else
  return 0.;
#endif
}

template < clockid_t clock_type >
inline void
StopwatchTimer< clock_type >::reset()
{
#ifndef DISABLE_TIMING
  _beg = 0; // invariant: _end >= _beg
  _end = 0;
  _prev_elapsed = 0; // erase all prev. measurements
  _running = false;  // of course not running.
#endif
}

template < clockid_t clock_type >
inline void
StopwatchTimer< clock_type >::print( const std::string& msg, timeunit_t timeunit, std::ostream& os ) const
{
#ifndef DISABLE_TIMING
  double e = elapsed( timeunit );
  os << msg << e;
  switch ( timeunit )
  {
  case timeunit_t::NANOSEC:
    os << " nanosec.";
  case timeunit_t::MICROSEC:
    os << " microsec.";
    break;
  case timeunit_t::MILLISEC:
    os << " millisec.";
    break;
  case timeunit_t::SECONDS:
    os << " sec.";
    break;
  case timeunit_t::MINUTES:
    os << " min.";
    break;
  case timeunit_t::HOURS:
    os << " h.";
    break;
  case timeunit_t::DAYS:
    os << " days.";
    break;
  default:
    throw BadParameter( "Invalid timeunit provided to stopwatch." );
  }
#ifdef DEBUG
  os << " (running: " << ( _running ? "true" : "false" ) << ", begin: " << _beg << ", end: " << _end
     << ", diff: " << ( _end - _beg ) << ", prev: " << _prev_elapsed << ")";
#endif
  os << std::endl;
#endif
}

template < clockid_t clock_type >
inline size_t
StopwatchTimer< clock_type >::get_current_time()
{
  timespec now;
  clock_gettime( clock_type, &now );
  return now.tv_nsec + now.tv_sec * static_cast< long >( timeunit_t::SECONDS );
}

template < clockid_t clock_type >
inline std::ostream&
operator<<( std::ostream& os, const StopwatchTimer< clock_type >& stopwatch )
{
  stopwatch.print( "", timeunit_t::SECONDS, os );
  return os;
}

} // namespace timers


template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::start()
{
  kernel::manager< VPManager >.assert_thread_parallel();

  walltime_timers_[ kernel::manager< VPManager >.get_thread_id() ].start();
  cputime_timers_[ kernel::manager< VPManager >.get_thread_id() ].start();
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::stop()
{
  kernel::manager< VPManager >.assert_thread_parallel();

  walltime_timers_[ kernel::manager< VPManager >.get_thread_id() ].stop();
  cputime_timers_[ kernel::manager< VPManager >.get_thread_id() ].stop();
}

template < StopwatchGranularity detailed_timer >
bool
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::is_running_() const
{
  kernel::manager< VPManager >.assert_thread_parallel();

  return walltime_timers_[ kernel::manager< VPManager >.get_thread_id() ].is_running_();
}

template < StopwatchGranularity detailed_timer >
double
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::elapsed( timers::timeunit_t
    timeunit ) const
{
  kernel::manager< VPManager >.assert_thread_parallel();

  return walltime_timers_[ kernel::manager< VPManager >.get_thread_id() ].elapsed( timeunit );
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::print( const std::string& msg,
  timers::timeunit_t timeunit,
  std::ostream& os ) const
{
  kernel::manager< VPManager >.assert_thread_parallel();

  walltime_timers_[ kernel::manager< VPManager >.get_thread_id() ].print( msg, timeunit, os );
}

template < StopwatchGranularity detailed_timer >
void
Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::reset()
{
  kernel::manager< VPManager >.assert_single_threaded();

  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();
  walltime_timers_.resize( num_threads );
  cputime_timers_.resize( num_threads );
  for ( size_t i = 0; i < num_threads; ++i )
  {
    walltime_timers_[ i ].reset();
    cputime_timers_[ i ].reset();
  }
}



} //namespace nest

#endif 