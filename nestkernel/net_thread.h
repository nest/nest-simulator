/*
 *  net_thread.h
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

#ifndef NETTHREAD_H
#define NETTHREAD_H

#include "config.h"

#ifdef HAVE_PTHREADS
#ifdef HAVE_PTHREAD_IGNORED
#undef __PURE_CNAME
#include <pthread.h>
#define __PURE_CNAME
#else
#include <pthread.h>
#endif
#endif

#include <vector>

using std::vector;

namespace nest
{
  class Node;
  class Scheduler;

  /**
   * Class to update a batch of Nodes in a single thread.
   *
   * Objects of this class are created by the Scheduler to update a
   * batch of Nodes.
   * 
   * Threads are created at the beginning of a simulation epoch and are
   * destroyed after the simulation time has elapsed, or the simulation has
   * been suspended by a user signal.
   */
  class Thread
  {
  public:
    
    Thread();

    Thread(const Thread &);
    Thread operator=(const Thread&);      
    
    void init(int, Scheduler *);
    int  get_id(void) const; 
    
#ifdef HAVE_PTHREADS
    int join();
    void run(void);  // main driver of the thread
#endif

  private:
    
    int        id_;         //!< number of the thread, may be negative

#ifdef HAVE_PTHREADS
    pthread_t  p_;          //!< Thread info
#endif

    Scheduler  *scheduler_; //!< The scheduler.
  };

  inline
  int Thread::get_id(void) const 
  {
    return id_;
  }
  
} // namespace

#ifdef HAVE_PTHREADS
extern "C"
void* nest_thread_handler(void *);
#endif

#endif

