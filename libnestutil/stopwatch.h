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
#include <cassert>
#include <iostream>

namespace nest
{

/***********************************************************************
 * Stopwatch                                                           *
 *   Accumulates time between start and stop, and provides             *
 *   the elapsed time with different time units.                       *
 *                                                                     *
 *   Partly inspired by com.google.common.base.Stopwatch.java          *
 *   Not thread-safe: - Do not share stopwatches among threads.        *
 *                    - Let each thread have its own stopwatch.        *
 *                                                                     *
 *   Usage example:                                                    *
 *     Stopwatch x;                                                    *
 *     x.start();                                                      *
 *     // ... do computations for 15.34 sec                            *
 *     x.stop(); // only pauses stopwatch                              *
 *     x.print("Time needed "); // > Time needed 15.34 sec.            *
 *     x.start(); // resumes stopwatch                                 *
 *     // ... next computations for 11.22 sec                          *
 *     x.stop();                                                       *
 *     x.print("Time needed "); // > Time needed 26,56 sec.            *
 *     x.reset(); // reset to default values                           *
 *     x.start(); // starts the stopwatch from 0                       *
 *     // ... computation 5.7 sec                                      *
 *     x.print("Time "); // > Time 5.7 sec.                            *
 *     // ^ intermediate timing without stopping the stopwatch         *
 *     // ... more computations 1.7643 min                             *
 *     x.stop();                                                       *
 *     x.print("Time needed ", Stopwatch::MINUTES, std::cerr);         *
 *     // > Time needed 1,8593 min. (on cerr)                          *
 *     // other units and output streams possible                      *
 ***********************************************************************/
class Stopwatch
{
public:
  typedef size_t timestamp_t;
  typedef size_t timeunit_t;

  enum
  {
    MICROSEC = ( timeunit_t ) 1,
    MILLISEC = MICROSEC * 1000,
    SECONDS = MILLISEC * 1000,
    MINUTES = SECONDS * 60,
    HOURS = MINUTES * 60,
    DAYS = HOURS * 24
  };

  static bool correct_timeunit( timeunit_t t );

  /**
   * Creates a stopwatch that is not running.
   */
  Stopwatch()
  {
    reset();
  }

  /**
   * Starts or resumes the stopwatch, if it is not running already.
   */
  void start();

  /**
   * Stops the stopwatch, if it is not stopped already.
   */
  void stop();

  /**
   * Returns, whether the stopwatch is running.
   */
  bool isRunning() const;

  /**
   * Returns the time elapsed between the start and stop of the
   * stopwatch. If it is running, it returns the time from start
   * until now. If the stopwatch is run previously, the previous
   * runtime is added. If you want only the last measurment, you
   * have to reset the timer, before stating the measurment.
   * Does not change the running state.
   */
  double elapsed( timeunit_t timeunit = SECONDS ) const;

  /**
   * Returns the time elapsed between the start and stop of the
   * stopwatch. If it is running, it returns the time from start
   * until now. If the stopwatch is run previously, the previous
   * runtime is added. If you want only the last measurment, you
   * have to reset the timer, before stating the measurment.
   * Does not change the running state.
   * In contrast to Stopwatch::elapsed(), only the timestamp is returned,
   * that is the number if microseconds as an integer.
   */
  timestamp_t elapsed_timestamp() const;

  /**
   * Resets the stopwatch.
   */
  void reset();

  /**
   * This method prints out the currently elapsed time.
   */
  void print( const char* msg = "", timeunit_t timeunit = SECONDS, std::ostream& os = std::cout ) const;

  /**
   * Convenient method for writing time in seconds
   * to some ostream.
   */
  friend std::ostream& operator<<( std::ostream& os, const Stopwatch& stopwatch );

private:
#ifndef DISABLE_TIMING
  timestamp_t _beg, _end;
  size_t _prev_elapsed;
  bool _running;
#endif

  /**
   * Returns current time in microseconds since EPOCH.
   */
  static timestamp_t get_timestamp();
};

inline bool
Stopwatch::correct_timeunit( timeunit_t t )
{
  return t == MICROSEC || t == MILLISEC || t == SECONDS || t == MINUTES || t == HOURS || t == DAYS;
}

inline void
nest::Stopwatch::start()
{
#ifndef DISABLE_TIMING
  if ( not isRunning() )
  {
    _prev_elapsed += _end - _beg;  // store prev. time, if we resume
    _end = _beg = get_timestamp(); // invariant: _end >= _beg
    _running = true;               // we start running
  }
#endif
}

inline void
nest::Stopwatch::stop()
{
#ifndef DISABLE_TIMING
  if ( isRunning() )
  {
    _end = get_timestamp(); // invariant: _end >= _beg
    _running = false;       // we stopped running
  }
#endif
}

inline bool
nest::Stopwatch::isRunning() const
{
#ifndef DISABLE_TIMING
  return _running;
#else
  return false;
#endif
}

inline double
nest::Stopwatch::elapsed( timeunit_t timeunit ) const
{
#ifndef DISABLE_TIMING
  assert( correct_timeunit( timeunit ) );
  return 1.0 * elapsed_timestamp() / timeunit;
#else
  return 0.0;
#endif
}

inline nest::Stopwatch::timestamp_t
nest::Stopwatch::elapsed_timestamp() const
{
#ifndef DISABLE_TIMING
  if ( isRunning() )
  {
    // get intermediate elapsed time; do not change _end, to be const
    return get_timestamp() - _beg + _prev_elapsed;
  }
  else
  {
    // stopped before, get time of current measurment + last measurments
    return _end - _beg + _prev_elapsed;
  }
#else
  return ( timestamp_t ) 0;
#endif
}

inline void
nest::Stopwatch::reset()
{
#ifndef DISABLE_TIMING
  _beg = 0; // invariant: _end >= _beg
  _end = 0;
  _prev_elapsed = 0; // erase all prev. measurments
  _running = false;  // of course not running.
#endif
}

inline void
nest::Stopwatch::print( const char* msg, timeunit_t timeunit, std::ostream& os ) const
{
#ifndef DISABLE_TIMING
  assert( correct_timeunit( timeunit ) );
  double e = elapsed( timeunit );
  os << msg << e;
  switch ( timeunit )
  {
  case MICROSEC:
    os << " microsec.";
    break;
  case MILLISEC:
    os << " millisec.";
    break;
  case SECONDS:
    os << " sec.";
    break;
  case MINUTES:
    os << " min.";
    break;
  case HOURS:
    os << " h.";
    break;
  case DAYS:
    os << " days.";
    break;
  }
#ifdef DEBUG
  os << " (running: " << ( _running ? "true" : "false" ) << ", begin: " << _beg << ", end: " << _end
     << ", diff: " << ( _end - _beg ) << ", prev: " << _prev_elapsed << ")";
#endif
  os << std::endl;
#endif
}

inline nest::Stopwatch::timestamp_t
nest::Stopwatch::get_timestamp()
{
  // works with:
  // * hambach (Linux 2.6.32 x86_64)
  // * JuQueen (BG/Q)
  // * MacOS 10.9
  struct timeval now;
  gettimeofday( &now, ( struct timezone* ) 0 );
  return ( nest::Stopwatch::timestamp_t ) now.tv_usec
    + ( nest::Stopwatch::timestamp_t ) now.tv_sec * nest::Stopwatch::SECONDS;
}

} /* namespace timer */
#endif /* STOPWATCH_H */
