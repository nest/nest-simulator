/*
 *  timer.h
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

#include <stdint.h>
#include <iostream>

namespace nestio
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
    typedef uint64_t timestamp_t;
    typedef uint64_t timeunit_t;
    
    enum 
    {
        MICROSEC = (timeunit_t)1,
        MILLISEC = MICROSEC * 1000,
        SECONDS  = MILLISEC * 1000,
        MINUTES  = SECONDS  * 60,
        HOURS    = MINUTES  * 60,
        DAYS     = HOURS    * 24
    };

    static bool correct_timeunit(timeunit_t t);
    
    /** 
     * Creates a stopwatch that is not running.
     */
    Stopwatch();
    
    /** 
     * Starts or resumes the stopwatch, if it is not running already.
     */
    void start();
    
    /** 
     * Stops the stopwatch, if it is not stopped already.
     */
    void stop();
    
    
    void pause();

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
    double elapsed(timeunit_t timeunit = SECONDS) const;

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
    void print(const char* msg = "", timeunit_t timeunit = SECONDS, 
               std::ostream& os = std::cout) const;
    
    /**
     * Convenient method for writing time in seconds
     * to some ostream.
     */
    friend std::ostream& operator<< (std::ostream& os, 
                                     const Stopwatch& stopwatch);
    /** 
     * Returns current time in microseconds since EPOCH.
     */
    static timestamp_t get_timestamp();
private:
#ifdef ENABLE_TIMING
    timestamp_t _beg, _end;
    uint64_t _prev_elapsed;
    bool _running;
#endif
    
    
};

inline bool Stopwatch::correct_timeunit(timeunit_t t)
{
    return t == MICROSEC || t == MILLISEC || t == SECONDS || 
           t == MINUTES  || t == HOURS    || t == DAYS;
}

template < typename T >
class OVector
{
private:
  
public:
  T *values;
  int n;
  OVector(int maxsize):
  n(0)
  {
    values = new T[maxsize];
  }
  
  ~OVector()
  {
    delete values;
  }
  
  //critical function
  void push_back(T v)
  {
    values[n]=v;
    n++;
  }
  
  int size() const
  {
    return n;
  }
  
  bool empty() const
  {
      return (n<1);
  }
  
  T operator[](int& index) const
  {
    return values[index];
  }
  
  void clear()
  {
   n=0; 
  }
};

} /* namespace timer */
#endif /* STOPWATCH_H */