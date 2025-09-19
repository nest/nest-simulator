/*
 *  stopwatch.h
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

#ifndef STOPWATCH_H
#define STOPWATCH_H

// C includes:
#include <sys/time.h>

// C++ includes:
#include "arraydatum.h"
#include "dictdatum.h"
#include "dictutils.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

// Includes from nestkernel:
#include "exceptions.h"

namespace nest
{

#ifdef TIMER_DETAILED
constexpr bool use_detailed_timers = true;
#else
constexpr bool use_detailed_timers = false;
#endif
#ifdef THREADED_TIMERS
constexpr bool use_threaded_timers = true;
#else
constexpr bool use_threaded_timers = false;
#endif

enum class StopwatchGranularity
{
  Normal,  //!< Always measure stopwatch
  Detailed //!< Only measure if detailed stopwatches are activated
};

enum class StopwatchParallelism
{
  MasterOnly, //!< Only the master thread owns a stopwatch
  Threaded    //!< Every thread measures an individual stopwatch
};

/**
 * This template has two template arguments. The first one "detailed_timer" controls the granularity of the stopwatch,
 * i.e., if the timer is considered a normal or detailed timer. The second one "threaded_timer" defines if the timer is
 * supposed to be measured by each thread individually. In case a timer is specified as threaded, but threaded timers
 * are turned off globally, the stopwatch will run in master-only mode instead.
 *
 * In all cases, both the (monotonic) wall-time and cpu time are measured.
 *
 * Note: Forward class declaration required here because friend declaration in timers::StopwatchTimer must refer to
 * nest::Stopwatch to be correct, and that requires the name to be known from before. See
 * https://stackoverflow.com/questions/30418270/clang-bug-namespaced-template-class-friend for details.
 */
template < StopwatchGranularity, StopwatchParallelism >
class Stopwatch;

/********************************************************************************
 * Stopwatch                                                                    *
 *   Accumulates time between start and stop, and provides the elapsed time     *
 *   with different time units. Either runs multi-threaded or only on master.   *
 *                                                                              *
 *   Usage example:                                                             *
 *     Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > x;    *
 *     x.start();                                                               *
 *     // ... do computations for 15.34 sec                                     *
 *     x.stop(); // only pauses stopwatch                                       *
 *     x.print("Time needed "); // > Time needed 15.34 sec.                     *
 *     x.start(); // resumes stopwatch                                          *
 *     // ... next computations for 11.22 sec                                   *
 *     x.stop();                                                                *
 *     x.print("Time needed "); // > Time needed 26,56 sec.                     *
 *     x.reset(); // reset to default values                                    *
 *     x.start(); // starts the stopwatch from 0                                *
 *     // ... computation 5.7 sec                                               *
 *     x.print("Time "); // > Time 5.7 sec.                                     *
 *     // ^ intermediate timing without stopping the stopwatch                  *
 *     // ... more computations 1.7643 min                                      *
 *     x.stop();                                                                *
 *     x.print("Time needed ", timeunit_t::MINUTES, std::cerr);              *
 *     // > Time needed 1,8593 min. (on cerr)                                   *
 *     // other units and output streams possible                               *
 ********************************************************************************/
namespace timers
{
enum class timeunit_t : size_t
{
  NANOSEC = 1,
  MICROSEC = NANOSEC * 1000,
  MILLISEC = MICROSEC * 1000,
  SECONDS = MILLISEC * 1000,
  MINUTES = SECONDS * 60,
  HOURS = MINUTES * 60,
  DAYS = HOURS * 24
};

/** This class represents a single timer which measures the execution time of a single thread for a given clock type.
 * Typical clocks are monotonic wall-time clocks or clocks just measuring cpu time.
 */
template < clockid_t clock_type >
class StopwatchTimer
{
  template < StopwatchGranularity, StopwatchParallelism >
  friend class nest::Stopwatch;

public:
  typedef size_t timestamp_t;

  //! Creates a stopwatch that is not running.
  StopwatchTimer()
  {
    reset();
  }

  //! Starts or resumes the stopwatch, if it is not running already.
  void start();

  //! Stops the stopwatch, if it is not stopped already.
  void stop();

  /**
   * Returns the time elapsed between the start and stop of the stopwatch in the given unit. If it is running, it
   * returns the time from start until now. If the stopwatch is run previously, the previous runtime is added. If you
   * want only the last measurement, you have to reset the timer, before stating the measurement.
   * Does not change the running state.
   */
  double elapsed( timeunit_t timeunit = timeunit_t::SECONDS ) const;

  //! Resets the stopwatch.
  void reset();

  //! This method prints out the currently elapsed time.
  void
  print( const std::string& msg = "", timeunit_t timeunit = timeunit_t::SECONDS, std::ostream& os = std::cout ) const;

private:
  //! Returns, whether the stopwatch is running.
  bool is_running_() const;

#ifndef DISABLE_TIMING
  timestamp_t _beg, _end;
  size_t _prev_elapsed;
  bool _running;
#endif

  //! Returns current time in microseconds since EPOCH.
  static size_t get_current_time();
};

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


//! This template only measures a single timer, owned by the master thread, for both detailed and regular timers.
template < StopwatchGranularity detailed_timer >
class Stopwatch< detailed_timer, StopwatchParallelism::MasterOnly >
{
public:
  void
  start()
  {
    if constexpr ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers )
    {
#pragma omp master
      {
        walltime_timer_.start();
        cputime_timer_.start();
      }
    }
  }

  void
  stop()
  {
#pragma omp master
    {
      walltime_timer_.stop();
      cputime_timer_.stop();
    }
  }

  double
  elapsed( timers::timeunit_t timeunit = timers::timeunit_t::SECONDS ) const
  {
    double elapsed = 0.;
#pragma omp master
    {
      elapsed = walltime_timer_.elapsed( timeunit );
    };
    return elapsed;
  }

  void
  reset()
  {
#pragma omp master
    {
      walltime_timer_.reset();
      cputime_timer_.reset();
    }
  }

  void
  print( const std::string& msg = "",
    timers::timeunit_t timeunit = timers::timeunit_t::SECONDS,
    std::ostream& os = std::cout ) const
  {
#pragma omp master
    walltime_timer_.print( msg, timeunit, os );
  }

  void
  get_status( DictionaryDatum& d, const Name& walltime_name, const Name& cputime_name ) const
  {
    def< double >( d, walltime_name, walltime_timer_.elapsed() );
    def< double >( d, cputime_name, cputime_timer_.elapsed() );
  }

private:
  bool
  is_running_() const
  {
    bool is_running_ = false;
#pragma omp master
    {
      is_running_ = walltime_timer_.is_running_();
    };
    return is_running_;
  }

  // We use a monotonic timer to make sure the stopwatch is not influenced by time jumps (e.g. summer/winter time).
  timers::StopwatchTimer< CLOCK_MONOTONIC > walltime_timer_;
  timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID > cputime_timer_;
};

//! Stopwatch template specialization for threaded timer instances.
template < StopwatchGranularity detailed_timer >
class Stopwatch< detailed_timer, StopwatchParallelism::Threaded >
{
public:
  void start();

  void stop();

  double elapsed( timers::timeunit_t timeunit = timers::timeunit_t::SECONDS ) const;

  void reset();

  void print( const std::string& msg = "",
    timers::timeunit_t timeunit = timers::timeunit_t::SECONDS,
    std::ostream& os = std::cout ) const;

  void get_status( DictionaryDatum& d, const Name& walltime_name, const Name& cputime_name ) const;

private:
  bool is_running_() const;

  // We use a monotonic timer to make sure the stopwatch is not influenced by time jumps (e.g. summer/winter time).
  std::vector< timers::StopwatchTimer< CLOCK_MONOTONIC > > walltime_timers_;
  std::vector< timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID > > cputime_timers_;
};

} /* namespace nest */
#endif /* STOPWATCH_H */
