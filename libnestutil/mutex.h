/*
 *  mutex.h
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

#ifndef MUTEX_H
#define MUTEX_H

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

namespace nest {

  class Mutex
  {
    Mutex(Mutex&);
  public:
    Mutex();
    ~Mutex();
    
    void lock();
    void unlock();
#ifdef HAVE_PTHREADS
    pthread_mutex_t* operator&();
  private:
    pthread_mutex_t mtx;
#endif
  };

  inline
  Mutex::Mutex()
  { 
#ifdef HAVE_PTHREADS
    pthread_mutex_init(&mtx,NULL);
#endif
  }

  inline
  Mutex::~Mutex()
  { 
#ifdef HAVE_PTHREADS
    pthread_mutex_destroy(&mtx);
#endif
  }
  
  inline
  void Mutex::lock()
  { 
#ifdef HAVE_PTHREADS
    pthread_mutex_lock(&mtx);
#endif
  }

  inline
  void Mutex::unlock()
  { 
#ifdef HAVE_PTHREADS
    pthread_mutex_unlock(&mtx);
#endif
  }

#ifdef HAVE_PTHREADS
  inline
  pthread_mutex_t* Mutex::operator& ()
  {
    return &mtx;
  }
#endif


}
#endif
