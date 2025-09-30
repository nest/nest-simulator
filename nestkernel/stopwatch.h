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

// C++ includes:
#include "arraydatum.h"
#include "dictdatum.h"
#include "dictutils.h"
#include <algorithm>
#include <iostream>
#include <vector>

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "vp_manager.h"

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
  Normal,  //<! Always measure stopwatch
  Detailed //<! Only measure if detailed stopwatches are activated
};

enum class StopwatchParallelism
{
  MasterOnly, //<! Only the master thread owns a stopwatch
  Threaded    //<! Every thread measures an individual stopwatch
};

// Forward class declaration required here because friend declaration
// in timers::StopwatchTimer must refer to nest::Stopwatch to be correct,
// and that requires the name to be known from before. See
// https://stackoverflow.com/questions/30418270/clang-bug-namespaced-template-class-friend
// for details.
template < StopwatchGranularity, StopwatchParallelism, typename >
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
  template < StopwatchGranularity, StopwatchParallelism, typename >
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
} // namespace timers


/** This is the base template for all Stopwatch specializations.
 *
 * This template will be specialized in case detailed timers are deactivated or the timer is supposed to be run by
 * multiple threads. If the timer should be deactivated, because detailed timers are disabled, the template
 * specialization will be empty and optimized away by the compiler.
 * This base template only measures a single timer, owned by the master thread, which applies for both detailed and
 * regular timers. Detailed, master-only timers that are deactivated when detailed timers are turned off are handled
 * by one of the template specializations below.
 *
 * The template has three template arguments of which two act as actual parameters that need to be specified for each
 * stopwatch instance. The first one "detailed_timer" controls the granularity of the stopwatch, i.e., if the timer is
 * considered a normal or detailed timer. The second one "threaded_timer" defines if the timer is supposed to be
 * measured by each thread individually. In case a timer is specified as threaded, but threaded timers are turned off
 * globally, the stopwatch will run in master-only mode instead. The third template argument is used to enable or
 * disable certain template specializations based on compiler flags (i.e., detailed timers and threaded timers).
 *
 * In all cases, both the (monotonic) wall-time and cpu time are measured.
 */
template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer, typename = void >
class Stopwatch
{
public:
  void
  start()
  {
#pragma omp master
    {
      walltime_timer_.start();
      cputime_timer_.start();
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

//! Stopwatch template specialization for detailed, master-only timer instances if detailed timers are deactivated.
template <>
class Stopwatch< StopwatchGranularity::Detailed,
  StopwatchParallelism::MasterOnly,
  std::enable_if< not use_detailed_timers > >
{
public:
  void
  start()
  {
  }
  void
  stop()
  {
  }
  double
  elapsed( timers::timeunit_t = timers::timeunit_t::SECONDS ) const
  {
    return 0;
  }
  void
  reset()
  {
  }
  void
  print( const std::string& = "", timers::timeunit_t = timers::timeunit_t::SECONDS, std::ostream& = std::cout ) const
  {
  }
  void
  get_status( DictionaryDatum&, const Name&, const Name& ) const
  {
  }

private:
  bool
  is_running_() const
  {
    return false;
  }
};

//! Stopwatch template specialization for detailed, threaded timer instances if detailed timers are deactivated.
template < StopwatchGranularity detailed_timer >
class Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Detailed and not use_detailed_timers ) > >
{
public:
  void
  start()
  {
  }
  void
  stop()
  {
  }
  double
  elapsed( timers::timeunit_t = timers::timeunit_t::SECONDS ) const
  {
    return 0;
  }
  void
  reset()
  {
  }
  void
  print( const std::string& = "", timers::timeunit_t = timers::timeunit_t::SECONDS, std::ostream& = std::cout ) const
  {
  }
  void
  get_status( DictionaryDatum&, const Name&, const Name& ) const
  {
  }

private:
  bool
  is_running_() const
  {
    return false;
  }
};

/** Stopwatch template specialization for threaded timer instances if the timer is a detailed one and detailed timers
 * are activated or the timer is not a detailed one in the first place.
 */
template < StopwatchGranularity detailed_timer >
class Stopwatch< detailed_timer,
  StopwatchParallelism::Threaded,
  std::enable_if_t< use_threaded_timers
    and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >
{
public:
  void start();

  void stop();

  double elapsed( timers::timeunit_t timeunit = timers::timeunit_t::SECONDS ) const;

  void reset();

  void print( const std::string& msg = "",
    timers::timeunit_t timeunit = timers::timeunit_t::SECONDS,
    std::ostream& os = std::cout ) const;

  void
  get_status( DictionaryDatum& d, const Name& walltime_name, const Name& cputime_name ) const
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

private:
  bool is_running_() const;

  // We use a monotonic timer to make sure the stopwatch is not influenced by time jumps (e.g. summer/winter time).
  std::vector< timers::StopwatchTimer< CLOCK_MONOTONIC > > walltime_timers_;
  std::vector< timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID > > cputime_timers_;
};

} /* namespace nest */

#include "stopwatch_impl.h"

#endif /* STOPWATCH_H */
