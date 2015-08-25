/*
 *  stopwatch.cpp
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

#include "stopwatch.h"
#include <sys/time.h>
#include <cassert>

namespace nestio
{
    std::ostream& operator<<(std::ostream& os, 
                         const Stopwatch& stopwatch) 
    {
        stopwatch.print("", Stopwatch::SECONDS, os);
        return os;
    }
}

nestio::Stopwatch::Stopwatch() 
{
    reset();
}

void 
nestio::Stopwatch::pause()
{
#ifdef ENABLE_TIMING
  if(isRunning())
    {
        _end = get_timestamp(); 
        _running = false;               // we stop running
    }
#endif 
}

void
nestio::Stopwatch::start()
{
#ifdef ENABLE_TIMING
    if(!isRunning())
    {
        _prev_elapsed += _end - _beg;  // store prev. time, if we resume
        _end = _beg = get_timestamp(); // invariant: _end >= _beg
        _running = true;               // we start running
    }
#endif
}

void
nestio::Stopwatch::stop()
{
#ifdef ENABLE_TIMING
    if(isRunning())
    {
        _end = get_timestamp(); // invariant: _end >= _beg
        _running = false;       // we stopped running
    }
#endif
}

bool 
nestio::Stopwatch::isRunning() const
{
#ifdef ENABLE_TIMING
    return _running;
#else
    return false;
#endif
}

double 
nestio::Stopwatch::elapsed(timeunit_t timeunit) const
{
#ifdef ENABLE_TIMING
    assert(correct_timeunit(timeunit));
    return 1.0 * elapsed_timestamp() / timeunit;
#else
    return 0.0;
#endif
}

nestio::Stopwatch::timestamp_t 
nestio::Stopwatch::elapsed_timestamp() const
{
#ifdef ENABLE_TIMING
    if(isRunning())
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
    return (timestamp_t)0;
#endif
}

void
nestio::Stopwatch::reset() 
{
#ifdef ENABLE_TIMING
    _beg = 0; // invariant: _end >= _beg
    _end = 0;
    _prev_elapsed = 0; // erase all prev. measurments
    _running = false;  // of course not running.
#endif
}

void 
nestio::Stopwatch::print(const char* msg, timeunit_t timeunit, 
                       std::ostream& os) const
{
#ifdef ENABLE_TIMING
    assert(correct_timeunit(timeunit));
    double e = elapsed(timeunit);
    os << msg << e;
    switch (timeunit)
    {
        case MICROSEC: os << " microsec."; break;
        case MILLISEC: os << " millisec."; break;
        case SECONDS:  os << " sec."; break;
        case MINUTES:  os << " min."; break;
        case HOURS:    os << " h."; break;
        case DAYS:     os << " days."; break;
    }
#ifdef DEBUG
    os << " (running: " << (_running ? "true" : "false")
       << ", begin: "  << _beg
       << ", end: "    << _end
       << ", diff: "   << (_end - _beg)
       << ", prev: "   << _prev_elapsed << ")";
#endif
    os << std::endl;
#endif
}

nestio::Stopwatch::timestamp_t
nestio::Stopwatch::get_timestamp ()
{
    // works with:
    // * hambach (Linux 2.6.32 x86_64)
    // * MacOS 10.9
    struct timeval now;
    gettimeofday (&now, (struct timezone*)0); // alt: getrusage
    return  (nestio::Stopwatch::timestamp_t) now.tv_usec + 
            (nestio::Stopwatch::timestamp_t) now.tv_sec  * nestio::Stopwatch::SECONDS;
}
