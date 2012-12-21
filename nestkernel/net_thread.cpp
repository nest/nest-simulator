/*
 *  net_thread.cpp
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

#include <iostream>
#include <cstring>
#include "net_thread.h"
#include "scheduler.h"

nest::Thread::Thread()
  :id_(-1),
   scheduler_(NULL)
{}

nest::Thread::Thread(const Thread&)
  :id_(-1),
   scheduler_(NULL)
{}


nest::Thread nest::Thread::operator=(const Thread &t)
{  
  assert(t.id_==-1);
  id_=-1;
  scheduler_=NULL;
  return *this;
}

void nest::Thread::init(int i, Scheduler* s)
{

  assert(s != NULL);
  assert(i>=0);
  assert(id_==-1);

  scheduler_=s;
  id_=i;

#ifdef HAVE_PTHREADS
  // We have only a small number of threads, so it is better
  // to assign them individually to LWPs and have them scheduled
  // by the OS kernel
  pthread_attr_t thread_attribute;
  pthread_attr_init(&thread_attribute);

  int status=pthread_attr_setscope(&thread_attribute, PTHREAD_SCOPE_SYSTEM);
  if(status != 0)
  {
    std::cerr << "Error while setting the scheduling scope." << std::endl;
    throw PthreadException(status);
  }

  //std::cerr << "Starting Thread no. " << i << std::endl;
  
  status= pthread_create(&p_, &thread_attribute,
			 nest_thread_handler,
			 static_cast<void *>(this)
			 );
  if(status!=0)
  {
    std::cerr << "Error creating thread. Error code " 
	      << status << std::endl
	      << "which is: " << std::strerror(status) << std::endl;
    throw PthreadException(status);
  }
#else
  if (i > 0)
    {
      std::cerr << "Multithreading not available" << std::endl;
      throw KernelException();
    }
#endif
}

#ifdef HAVE_PTHREADS
void nest::Thread::run(void)
{
  assert(id_ >= 0);
  scheduler_->threaded_update(id_);
}

int nest::Thread::join()
{
  return pthread_join(p_,(void **)NULL);
}

// global thread handler function.
extern "C"
void* nest_thread_handler(void *t)
{
  nest::Thread *my_thread=static_cast<nest::Thread *>(t);
  assert(my_thread != NULL);

  my_thread->run();
  pthread_exit(0);
  return NULL;
}

#endif //HAVE_PTHREADS
