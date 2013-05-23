/*
 *  scheduler.cpp
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

#include <cmath>
#include <iostream>
#include <sstream>
#include <set>

#include "config.h"
#include "compose.hpp"

#ifdef HAVE_PTHREADS
#ifdef HAVE_PTHREAD_IGNORED
#undef __PURE_CNAME
#include <pthread.h>
#define __PURE_CNAME
#else
#include <pthread.h>
#endif
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#include <climits>
#include "network.h"
#include "exceptions.h"
#include "scheduler.h"
#include "event.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "arraydatum.h"
#include "randomgen.h"
#include "random_datums.h"

#include "nest_timemodifier.h"
#include "nest_timeconverter.h"

#ifdef N_DEBUG
#undef N_DEBUG
#endif

extern int SLIsignalflag;

std::vector<nest::delay> nest::Scheduler::moduli_;
std::vector<nest::delay> nest::Scheduler::slice_moduli_;

nest::delay nest::Scheduler::max_delay_ = 1;
nest::delay nest::Scheduler::min_delay_ = 1;

const nest::delay nest::Scheduler::comm_marker_ = 0;

nest::Scheduler::Scheduler(Network &net)
        : initialized_(false),
          simulating_(false),
      	  force_singlethreading_(false),
          n_threads_(1),
	  n_nodes_(0),
          entry_counter_(0),
          exit_counter_(0),
          net_(net),
          clock_(Time::tic(0L)),
          slice_(0L),
          to_do_(0L),
          to_do_total_(0L),
          from_step_(0L),
          to_step_(0L),    // consistent with to_do_ == 0
          update_ref_(true),
          terminate_(false),
          off_grid_spiking_(false),
          print_time_(false),
          rng_()
{
  init_();
}

nest::Scheduler::~Scheduler()
{
  finalize_();
}

void nest::Scheduler::reset()
{
  // Reset TICS_PER_MS, MS_PER_TICS and TICS_PER_STEP to the compiled in default values.
  // See ticket #217 for details.
  nest::TimeModifier::reset_to_defaults();

  clock_.set_to_zero();  // ensures consistent state
  to_do_ = 0;
  slice_ = 0;
  from_step_ = 0;
  to_step_ = 0;   // consistent with to_do_ = 0

  finalize_();
  init_();
}

void nest::Scheduler::clear_pending_spikes()
{
  configure_spike_buffers_();
}

void nest::Scheduler::init_()
{
  assert(initialized_ == false);

  simulated_ = false;
  min_delay_ = max_delay_ = 0;
  update_ref_ = true;

#ifdef HAVE_PTHREADS
  int status = pthread_cond_init(&done_, NULL);
  if(status != 0)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
	       "Error initializing condition variable done_");
    throw PthreadException(status);
  }

  status = pthread_cond_init(&ready_, NULL);
  if(status != 0)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
	       "Error initializing condition variable ready_");
    throw PthreadException(status);
  }
#else
#ifndef _OPENMP
  if (n_threads_ > 1)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
		 "No multithreading available, using single threading");
    n_threads_ = 1;
    force_singlethreading_ = true;
  }
#endif
#endif

  set_num_threads(n_threads_);

  create_rngs_(true);  // flag that this is a call from the ctr
  create_grng_(true);  // flag that this is a call from the ctr

  initialized_ = true;
}

void nest::Scheduler::finalize_()
{
#ifdef HAVE_PTHREADS
  int status = pthread_cond_destroy(&done_);
  if(status != 0)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
	       "Error in destruction of condition variable done_");
    throw PthreadException(status);
  }

  status = pthread_cond_destroy(&ready_);
  if(status != 0)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
	       "Error in destruction of condition variable ready_");
    throw PthreadException(status);
  }
#endif

  // clear the buffers
  local_grid_spikes_.clear();
  global_grid_spikes_.clear();
  local_offgrid_spikes_.clear();
  global_offgrid_spikes_.clear();

  initialized_ = false;
}

void nest::Scheduler::init_moduli_()
{
  assert (min_delay_ != 0);
  assert (max_delay_ != 0);

  /*
   * Ring buffers use modulos to determine where to store incoming events
   * with given time stamps, relative to the beginning of the slice in which
   * the spikes are delivered from the queue, ie, the slice after the one
   * in which they were generated. The pertaining offsets are 0..max_delay-1.
   */

  moduli_.resize(min_delay_ + max_delay_);

  for(delay d=0; d < min_delay_ + max_delay_; ++d)
    moduli_[d]= ( clock_.get_steps()+ d ) % (min_delay_ + max_delay_);

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast<size_t>(std::ceil(static_cast<double>(min_delay_+max_delay_) / min_delay_));
  slice_moduli_.resize(min_delay_+max_delay_);
  for ( delay d = 0 ; d < min_delay_+max_delay_ ; ++d )
    slice_moduli_[d] = ( (clock_.get_steps() + d) / min_delay_ ) % nbuff;
}

/**
 * This function is called after all nodes have been updated.
 * We can compute the value of (T+d) mod max_delay without explicit
 * reference to the network clock, because compute_moduli_ is
 * called whenever the network clock advances.
 * The various modulos for all available delays are stored in
 * a lookup-table and this table is rotated once per time slice.
 */
void nest::Scheduler::compute_moduli_()
{
  assert (min_delay_ != 0);
  assert (max_delay_ != 0);

  /*
   * Note that for updating the modulos, it is sufficient
   * to rotate the buffer to the left.
   */
  assert(moduli_.size() == min_delay_ + max_delay_);
  std::rotate(moduli_.begin(),moduli_.begin()+min_delay_,moduli_.end());

  /* For the slice-based ring buffer, we cannot rotate the table, but
     have to re-compute it, since max_delay_ may not be a multiple of
     min_delay_.  Reference time is the time at the beginning of the slice.
  */
  const size_t nbuff = static_cast<size_t>(std::ceil(static_cast<double>(min_delay_+max_delay_) / min_delay_));
  for ( delay d = 0 ; d < min_delay_+max_delay_ ; ++d )
    slice_moduli_[d] = ((clock_.get_steps() + d) / min_delay_ ) % nbuff;
}

void nest::Scheduler::compute_delay_extrema_(delay& min_delay, delay& max_delay) const
{
  min_delay = net_.connection_manager_.get_min_delay().get_steps();
  max_delay = net_.connection_manager_.get_max_delay().get_steps();

  if (Communicator::get_num_processes() > 1)
  {
    std::vector<int_t> min_delays(Communicator::get_num_processes());
    min_delays[Communicator::get_rank()] = min_delay;
    Communicator::communicate(min_delays);
    min_delay = *std::min_element(min_delays.begin(), min_delays.end());

    std::vector<int_t> max_delays(Communicator::get_num_processes());
    max_delays[Communicator::get_rank()] = max_delay;
    Communicator::communicate(max_delays);
    max_delay = *std::max_element(max_delays.begin(), max_delays.end());
  }
}

void nest::Scheduler::configure_spike_buffers_()
{
  assert(min_delay_ != 0);

  spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  spike_register_.resize(n_threads_, std::vector<std::vector<uint_t> >(min_delay_));
  for (size_t j = 0; j < spike_register_.size(); ++j)
      for (size_t k = 0; k < spike_register_[j].size(); ++k)
	  spike_register_[j][k].clear();

  offgrid_spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  offgrid_spike_register_.resize(n_threads_, std::vector<std::vector<OffGridSpike> >(min_delay_));
  for (size_t j = 0; j < offgrid_spike_register_.size(); ++j)
      for (size_t k = 0; k < offgrid_spike_register_[j].size(); ++k)
      	offgrid_spike_register_[j][k].clear();

  //send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces.
  int send_buffer_size = n_threads_ * min_delay_ > 2 ? n_threads_ * min_delay_ : 2;
  int recv_buffer_size = send_buffer_size * Communicator::get_num_processes();
  Communicator::set_buffer_sizes(send_buffer_size, recv_buffer_size);

  // DEC cxx required 0U literal, HEP 2007-03-26
  local_grid_spikes_.clear();
  local_grid_spikes_.resize(send_buffer_size, 0U);
  local_offgrid_spikes_.clear();
  local_offgrid_spikes_.resize(send_buffer_size, OffGridSpike(0,0.0));
  global_grid_spikes_.clear();
  global_grid_spikes_.resize(recv_buffer_size, 0U);
  global_offgrid_spikes_.clear();
  global_offgrid_spikes_.resize(recv_buffer_size, OffGridSpike(0,0.0));

  displacements_.clear();
  displacements_.resize(Communicator::get_num_processes(), 0);
}

void nest::Scheduler::clear_nodes_vec_()
{
  nodes_vec_.resize(n_threads_);
  for(index t = 0; t < n_threads_; ++t)
    nodes_vec_[t].clear();
}

void nest::Scheduler::simulate(Time const & t)
{
  assert(initialized_);

  t_real_ = 0;
  t_slice_begin_ = timeval();
  t_slice_end_ = timeval();

  if (t == Time::ms(0.0))
    return;

  if (t < Time::step(1))
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::simulate",
		 String::compose("Simulation time must be >= %1 ms (one time step).",
				 Time::get_resolution().get_ms()));
    throw KernelException();
  }

  // Check for synchronicity of global rngs over processes
  if(Communicator::get_num_processes() > 1)
    if (!Communicator::grng_synchrony(grng_->ulrand(100000)))
      {
        net_.message(SLIInterpreter::M_ERROR, "Scheduler::simulate",
	  	     "Global Random Number Generators are not synchronized prior to simulation.");
        throw KernelException();
      }

  // As default, we have to place the iterator at the
  // leftmost node of the tree.
  // If the iteration process was suspended, we leave the
  // iterator where it was and continue the iteration
  // process by calling "resume()"

  // Note that the time argument accumulates.
  // This is needed so that the following two
  // calling sequences yield the same results:
  // a: net.simulate(t1); // suspend is called here
  //    net.simulate(t2);
  //    assert(net.t == t1+t2);
  // b: net.simulate(t1); // NO suspend  called
  //    net.simulate(t2);
  //    assert(net.t == t1+t2);
  // The philosophy behind this behaviour is that the user
  // can in principle not know whether an element will suspend
  // the cycle.

  // check whether the simulation clock
  // will overflow during the simulation.
  if ( t.is_finite() )
  {
    Time time1 = clock_ + t;
    if( !time1.is_finite() )
    {
      std::string msg = String::compose("A clock overflow will occur after %1 of %2 ms. Please reset network "
                                        "clock first!", (Time::max()-clock_).get_ms(), t.get_ms());
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::simulate", msg);
      throw KernelException();
    }
  }
  else
  {
    std::string msg = String::compose("The requested simulation time exceeds the largest time NEST can handle "
                                      "(T_max = %1 ms). Please use a shorter time!", Time::max().get_ms());
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::simulate", msg);
    throw KernelException();
  }
  to_do_ += t.get_steps();
  to_do_total_ = to_do_;

  // find shortest and longest delay across all MPI processes
  // this call sets the member variables
  compute_delay_extrema_(min_delay_, max_delay_);

  // Warn about possible inconsistencies, see #504.
  // This test cannot come any earlier, because we first need to compute min_delay_
  // above.
  if ( t.get_steps() % min_delay_ != 0 )
      net_.message(SLIInterpreter::M_WARNING, "Scheduler::simulate",
		   "The requested simulation time is not an integer multiple of the minimal delay in the network. "
                   "This may result in inconsistent results under the following conditions: (i) A network contains "
                   "more than one source of randomness, e.g., two different poisson_generators, and (ii) Simulate "
                   "is called repeatedly with simulation times that are not multiples of the minimal delay.");

  // from_step_ is not touched here.  If we are at the beginning
  // of a simulation, it has been reset properly elsewhere.  If
  // a simulation was ended and is now continued, from_step_ will
  // have the proper value.  to_step_ is set as in advance_time().
  ulong_t end_sim = from_step_ + to_do_;
  if (min_delay_ < end_sim)
    to_step_ = min_delay_;    // update to end of time slice
  else
    to_step_ = end_sim;       // update to end of simulation time


  resume();
  simulated_ = true;

  // Check for synchronicity of global rngs over processes
  if(Communicator::get_num_processes() > 1)
    if (!Communicator::grng_synchrony(grng_->ulrand(100000)))
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::simulate",
                   "Global Random Number Generators are not synchronized after simulation.");
      throw KernelException();
    }
}

void nest::Scheduler::resume()
{
  assert(initialized_);

  terminate_ = false;

  if(to_do_ == 0)
    return;

#ifdef HAVE_PTHREAD_SETCONCURRENCY
  // The following is needed on Solaris 7 and higher
  // to get one processor per thread
  int status = pthread_setconcurrency(n_threads_);
  if(status != 0)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::resume","Could not set concurrency level.");
    throw PthreadException(status);
  }
#endif
  threads_.clear();
  threads_.resize(n_threads_);
  assert(threads_.size() == n_threads_);

  // if at the beginning of a simulation, set up spike buffers
  if ( !simulated_ )
    configure_spike_buffers_();

  prepare_nodes();

#ifdef HAVE_MUSIC
  // we have to do enter_runtime after prepre_nodes, since we use
  // calibrate to map the ports of MUSIC devices, which has to be done
  // before enter_runtime
  if (!simulated_) // only enter the runtime mode once
  {
    net_.publish_music_in_ports_();

    double tick = Time::get_resolution().get_ms() * min_delay_;
    std::string msg = String::compose("Entering MUSIC runtime with tick = %1 ms", tick);
    net_.message(SLIInterpreter::M_INFO, "Scheduler::resume", msg);
    Communicator::enter_runtime(tick);
  }
#endif

  simulating_ = true;

  if (print_time_)
  {
    std::cout << std::endl;
    print_progress_();
  }

  if (n_threads_ == 1)
    serial_update();
  else
  {
#ifdef HAVE_PTHREADS
    // Now we fire up the threads ...
    for(index i = 0; i < threads_.size(); ++i)
      threads_[i].init(i, this);

    // ... and wait until all are done.
    for(vector<Thread>::iterator i = threads_.begin(); i != threads_.end(); ++i)
      i->join();
#else
#ifdef _OPENMP
    // use openmp, if compiled with right compiler switch
    // for gcc this is -fopenmp
    threaded_update_openmp();
#else
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::reset",
		 "No multithreading available, using single threading");
    serial_update();
#endif
#endif
  }
  simulating_ = false;
  finalize_nodes();

  if (print_time_)
    std::cout << std::endl;

  Communicator::synchronize();

  if(terminate_)
  {
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::resume", "Exiting on error or user signal.");
    net_.message(SLIInterpreter::M_ERROR, "Scheduler::resume", "Scheduler: Use 'ResumeSimulation' to resume.");

    if(SLIsignalflag != 0)
    {
      SystemSignal signal(SLIsignalflag);
      SLIsignalflag=0;
      throw signal;
    }
    else
      throw SimulationError();

    return;
  }

  net_.message(SLIInterpreter::M_INFO, "Scheduler::resume", "Simulation finished.");
}

/**
 * the single-threaded update is just a single loop over all nodes
 * per time-slice.
 */
void nest::Scheduler::serial_update()
{
  std::vector<Node*>::iterator i;

  do
  {
    if (print_time_)
      gettimeofday(&t_slice_begin_, NULL);

    if ( from_step_ == 0 )  // deliver only at beginning of slice
    {
      deliver_events_(0);
#ifdef HAVE_MUSIC
      // advance the time of music by one step (min_delay * h) must
      // be done after deliver_events_() since it calls
      // music_event_out_proxy::handle(), which hands the spikes over to
      // MUSIC *before* MUSIC time is advanced
      if (slice_ > 0)
        Communicator::advance_music_time(1);

      net_.update_music_event_handlers_(clock_, from_step_, to_step_);
#endif
    }

    for (i = nodes_vec_[0].begin(); i != nodes_vec_[0].end(); ++i)
      update_(*i);

    if ( static_cast<ulong_t>(to_step_) == min_delay_ ) // gather only at end of slice
      gather_events_();

    advance_time_();

    if(SLIsignalflag != 0)
    {
      net_.message(SLIInterpreter::M_INFO, "Scheduler::serial_update", "Simulation exiting on user signal.");
      terminate_=true;
    }

    if (print_time_)
    {
      gettimeofday(&t_slice_end_, NULL);
      print_progress_();
    }
  } while((to_do_ != 0) && (! terminate_));
}

void nest::Scheduler::threaded_update_openmp()
{
#ifdef _OPENMP
  net_.message(SLIInterpreter::M_INFO, "Scheduler::threaded_update_openmp", "Simulating using OpenMP.");
#endif

// parallel section begins
#pragma omp parallel
  {
    int t = 0;
#ifdef _OPENMP
    t = omp_get_thread_num(); // which thread am I
#endif

    do {
      if (print_time_)
        gettimeofday(&t_slice_begin_, NULL);

      if ( from_step_ == 0 )  // deliver only at beginning of slice
	{
	  deliver_events_(t);
#ifdef HAVE_MUSIC
	  // advance the time of music by one step (min_delay * h) must
	  // be done after deliver_events_() since it calls
	  // music_event_out_proxy::handle(), which hands the spikes over to
	  // MUSIC *before* MUSIC time is advanced
	  if (slice_ > 0)
	    Communicator::advance_music_time(1);

	  net_.update_music_event_handlers_(clock_, from_step_, to_step_);
#endif
	}

      for (std::vector<Node*>::iterator i = nodes_vec_[t].begin(); i != nodes_vec_[t].end(); ++i)
	update_(*i);

      // parallel section ends, wait until all threads are done -> synchronize
#pragma omp barrier

      // the following block is executed by a single thread
      // the other threads wait at the end of the block
#pragma omp single
      {
	if ( static_cast<ulong_t>(to_step_) == min_delay_ ) // gather only at end of slice
	  gather_events_();

	advance_time_();

	if(SLIsignalflag != 0)
	  {
	    net_.message(SLIInterpreter::M_INFO, "Scheduler::serial_update", "Simulation exiting on user signal.");
	    terminate_ = true;
	  }

	if (print_time_)
        {
          gettimeofday(&t_slice_end_, NULL);
          print_progress_();
        }
      }
      // end of single section, all threads synchronize at this point

    }
    while((to_do_ != 0) && (! terminate_));

  } // end of #pragma parallel omp
}

#ifdef HAVE_PTHREADS
void nest::Scheduler::threaded_update(thread t)
{
  /** @todo Should not all variables related to thread control
   *  (ready_mutex_, ready_, done_, etnry_counter_, exit_counter_)
   *  be local variables in this function instead of class variables?
   *  Creating the pthread variables must be fast compared to simulation
   *  time.
   */

  assert(initialized_);
  assert(t >= 0);

  ready_mutex_.lock();

  while ( true )
  {
    /* The default proceeding:
     * - update the nodes.
     * - synchronise threads.
     * - advance time.

     * This loop is left if and only if
     * - the simulation time has elapsed and all nodes
     *   of the current thread are updated (test on to_do_).
     * - the end of a time slice has been reached and
     *   a user interrupt occured (test on SLISignalflag).
     */

    //////////// Serial section beyond this line

    // Wait until all threads have finished communicating and
    // have advance time. The final thread arriving here has
    // entry_counter_ == n_threads_, owns the ready_mutex_ and
    // initiates unlocking everything as follows:
    // 1. pthread_cond_signal(&ready_) wakes one arbitry waiting thread
    // 2. the awoken thread tries to acquire the ready_mutex_ as part of
    //    waking up, but blocks on it
    // 3. the signaling thread unlocks the ready_mutex_ and proceeds into
    //    the parallel section
    // 4. the awoken thread acquires the ready_mutex_ and the process
    //    starts at 1.
    // 5. the last thread to awake signals, but this signal has no effect
    // 6. all threads are now in the parallel section

    if (print_time_)
      gettimeofday(&t_slice_begin_, NULL);

    ++entry_counter_;
    if(entry_counter_ == n_threads_)
      exit_counter_ = 0;

    // protect against spurious wake-ups by while loop
    // see Butenhof, Programming with POSIX Threads, sec 3.3.2
    while(entry_counter_ < n_threads_)
      pthread_cond_wait(&ready_,&ready_mutex_);

    pthread_cond_signal(&ready_);
    ready_mutex_.unlock();

    //////////// Parallel section beyond this line

    // Deliver events and update all neurons in parallel
    if ( from_step_ == 0 )  // deliver only at beginning of slice
    {
      deliver_events_(t);

#ifdef HAVE_MUSIC
      if (t == 0) // only the first thread handles MUSIC stuff
      {
        // advance the time of music by one step (min_delay * h) must
        // be done after deliver_events_() since it calls
        // music_event_out_proxy::handle(), which hands the spikes over to
        // MUSIC *before* MUSIC time is advanced
        if (slice_ > 0)
          Communicator::advance_music_time(1);

        net_.update_music_event_handlers_(clock_, from_step_, to_step_);
      }
#endif
    }

    vector<Node*>::iterator i;
    for(i = nodes_vec_[t].begin(); i != nodes_vec_[t].end(); ++i)
      update_(*i);

    ready_mutex_.lock();

    //////////// Serial section beyond this line

    ++exit_counter_;   // Mark thread idle.

    // We are at the end of a time slice or the end of simulation time,
    // which may be inside a time slice.
    // We wait for all threads to finish the parallel section above. The
    // last thread arriving here fulfills exit_counter_==n_threads_ and
    // performs event collection and communication with all other MPI
    // processes, and advances time.
    // Threads are then awoken one by one similar as described above.
    // Each awoken thread retains the ready_mutex_ though, until it goes
    // to sleep again by calling pthread_cond_wait(&ready_,...) above.
    //
    // The overall effect of this is that we first wait for all threads to
    // finish updating, then communicate and advance time by help of the
    // last thread, and then check if we need to leave the simulation loop
    // because of a signal flag or because time has elapsed. In the latter
    // case, each thread releases the ready_mutex_ once it has broken out
    // of the while loop, so that the next thread can wake up.

    if (exit_counter_ == n_threads_)
    {
      if ( static_cast<ulong_t>(to_step_) == min_delay_ ) // gather only at end of slice
        gather_events_();

      advance_time_();

      entry_counter_ = 0;
    }

    // protect against spurious wake-ups by while loop
    // see Butenhof, Programming with POSIX Threads, sec 3.3.2
    while(exit_counter_ < n_threads_)
      pthread_cond_wait(&done_,&ready_mutex_);

    pthread_cond_signal(&done_);

    if ( SLIsignalflag != 0 )
    {
      std::string msg = String::compose("Thread %1 exiting on error or user signal.", t);
      net_.message(SLIInterpreter::M_INFO, "Scheduler::threaded_update", msg);
      break;
    }

    if(to_do_ == 0) // is time up?
    {
      break;
    }

    assert(to_do_ > 0);  // for safety's sake

    if (print_time_)
    {
      gettimeofday(&t_slice_end_, NULL);
      print_progress_();
    }
  } // while

  ready_mutex_.unlock();  // unlock mutex after breaking out of while loop

}
#else
void nest::Scheduler::threaded_update(thread)
{
   net_.message(SLIInterpreter::M_ERROR, "Scheduler::threaded_update", "Multithreading mode not available");
   throw KernelException();

}
#endif

void nest::Scheduler::prepare_nodes()
{
  assert(initialized_);

  init_moduli_();

  net_.message(SLIInterpreter::M_INFO, "Scheduler::prepare_nodes", "Please wait. Preparing elements.");

  /* We clear the existing nodes_vec_ and then rebuild it.
     prepare_node_() below inserts each node into the appropriate
     nodes_vec_ after initializing its buffers and calibrating it.
     We must rebuild the nodes_vec_ each time Simulate is called,
     in case nodes have been added or deleted between Simulate calls.
   */
  clear_nodes_vec_();

#ifdef _OPENMP
#pragma omp parallel
  {
    size_t t = omp_get_thread_num();
#else
  for (index t = 0; t < n_threads_; ++t)
  {
#endif

    size_t num_thread_local_nodes = 0;
    for (size_t idx = 0; idx < net_.size(); ++idx)
    {
      Node* node = net_.get_node(idx);
      if (static_cast<index>(node->get_thread()) == t || node->num_thread_siblings_() > 0)
        num_thread_local_nodes++;
    }
    nodes_vec_[t].reserve(num_thread_local_nodes);

    for(index n = 0; n < net_.size(); ++n)
    {
      if (net_.is_local_gid(n) && net_.nodes_[n] != 0)
      {
	if ((*net_.nodes_[n]).num_thread_siblings_() > 0)
	  prepare_node_((*net_.nodes_[n]).get_thread_sibling_(t));
	else
	{
	  Node* node = net_.get_node(n, t);
	  if (static_cast<uint_t>(node->get_thread()) == t)
	    prepare_node_(node);
	}
      }
    }
  } // end of parallel section / end of for threads

  n_nodes_ = 0;
  for (index t = 0; t < n_threads_; ++t)
  {
    n_nodes_ += nodes_vec_[t].size();
  }

  std::string msg = String::compose("Simulating %1 nodes.", n_nodes_);
  net_.message(SLIInterpreter::M_INFO, "Scheduler::prepare_nodes", msg);
}

void nest::Scheduler::finalize_nodes()
{
  for (index t = 0; t < n_threads_; ++t)
     for(index n = 0; n < net_.size(); ++n)
     {
       if ( net_.is_local_gid(n) && net_.nodes_[n]!=0 )
       {
         if ((*net_.nodes_[n]).num_thread_siblings_() > 0)
           (*net_.nodes_[n]).get_thread_sibling_(t)->finalize();
         else
         {
           Node* node = net_.get_node(n, t);
           if (static_cast<uint_t>(node->get_thread()) == t)
             node->finalize();
         }
       }
     }
}

void nest::Scheduler::set_status(DictionaryDatum const &d)
{
  assert(initialized_);

  // Create an instance of time converter here to capture the current
  // representation of time objects: TICS_PER_MS and TICS_PER_STEP
  // will be stored in time_converter.
  // This object can then be used to convert times in steps
  // (e.g. Connection::delay_) or tics to the new representation.
  // We pass this object to ConnectionManager::calibrate to update
  // all time objects in the connection system to the new representation.
  // MH 08-04-14
  TimeConverter time_converter;

  double_t time;
  if(updateValue<double_t>(d, "time", time))
  {
    if ( time != 0.0 )
      throw BadProperty("The simulation time can only be set to 0.0.");

    if ( clock_ > Time(Time::step(0)) )
    {
      // reset only if time has passed
      net_.message(SLIInterpreter::M_WARNING, "Scheduler::set_status",
                   "Simulation time reset to t=0.0. Resetting the simulation time is not"
                   "fully supported in NEST at present. Some spikes may be lost, and"
                   "stimulating devices may behave unexpectedly. PLEASE REVIEW YOUR"
                   "SIMULATION OUTPUT CAREFULLY!");

      clock_     = Time::step(0);
      from_step_ = 0;
      slice_     = 0;
      configure_spike_buffers_();  // clear all old spikes
    }
  }

  updateValue<bool>(d, "print_time", print_time_);

  long n_threads;
  bool n_threads_updated = updateValue<long>(d, "local_num_threads", n_threads);
  if (n_threads_updated)
  {
    if ( net_.size() > 1 )
      throw KernelException("Nodes exist: Thread/process number cannot be changed.");
    if ( net_.connection_manager_.has_user_prototypes() )
      throw KernelException("Custom synapse types exist: Thread/process number cannot be changed.");
    if ( net_.connection_manager_.get_user_set_delay_extrema() )
      throw KernelException("Delay extrema have been set: Thread/process number cannot be changed.");
    if ( net_.get_simulated() )
      throw KernelException("The network has been simulated: Thread/process number cannot be changed.");
    if ( not Time::resolution_is_default() )
      throw KernelException("The resolution has been set: Thread/process number cannot be changed.");

    if ( n_threads > 1 && force_singlethreading_ )
    {
      net_.message(SLIInterpreter::M_WARNING, "Scheduler::set_status",
                   "No multithreading available, using single threading");
      n_threads_ = 1;
    }

    // it is essential to call net_.reset() here to adapt memory pools and more
    // to the new number of threads and VPs.
    n_threads_ = n_threads;
    net_.reset();
  }

  long n_vps;
  bool n_vps_updated = updateValue<long>(d, "total_num_virtual_procs", n_vps);
  if (n_vps_updated)
  {
    if ( net_.size() > 1 )
      throw KernelException("Nodes exist: Thread/process number cannot be changed.");
    if ( net_.connection_manager_.has_user_prototypes() )
      throw KernelException("Custom synapse types exist: Thread/process number cannot be changed.");
    if ( net_.connection_manager_.get_user_set_delay_extrema() )
      throw KernelException("Delay extrema have been set: Thread/process number cannot be changed.");
    if ( net_.get_simulated() )
      throw KernelException("The network has been simulated: Thread/process number cannot be changed.");
    if ( not Time::resolution_is_default() )
      throw KernelException("The resolution has been set: Thread/process number cannot be changed.");

    if (n_vps % Communicator::get_num_processes() != 0)
      throw BadProperty("Number of virtual processes (threads*processes) must be an integer "
                        "multiple of the number of processes. Value unchanged.");
    
    n_threads_ = n_vps / Communicator::get_num_processes();
    if ((n_threads > 1) && (force_singlethreading_))
    {
      net_.message(SLIInterpreter::M_WARNING, "Scheduler::set_status",
                   "No multithreading available, using single threading");
      n_threads_ = 1;
    }

    // it is essential to call net_.reset() here to adapt memory pools and more
    // to the new number of threads and VPs
    set_num_threads(n_threads_);
    net_.reset();
  }

  // tics_per_ms and resolution must come after local_num_thread / total_num_threads
  // because they might reset the network and the time representation
  nest::double_t tics_per_ms;
  bool tics_per_ms_updated = updateValue<nest::double_t>(d, "tics_per_ms", tics_per_ms);
  double_t resd;
  bool res_updated = updateValue<double_t>(d, "resolution", resd);

  if (tics_per_ms_updated || res_updated)
  {
    if (net_.size() > 1) // root always exists
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                   "Cannot change time representation after nodes have been created. Please call ResetKernel first.");
      throw KernelException();
    }
    else if ( net_.get_simulated() )  // someone may have simulated empty network
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                   "Cannot change time representation after the network has been simulated. Please call ResetKernel first.");
      throw KernelException();
    }
    else if ( net_.connection_manager_.get_num_connections() != 0 )
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                   "Cannot change time representation after connections have been created. Please call ResetKernel first.");
      throw KernelException();
    }
    else if (res_updated && tics_per_ms_updated) // only allow TICS_PER_MS to be changed together with resolution
    {
      if ( resd < 1.0 / tics_per_ms )
      {
        net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                     "Resolution must be greater than or equal to one tic. Value unchanged.");
        throw KernelException();
      }
      else
      {
	nest::TimeModifier::set_time_representation(tics_per_ms, resd);
	clock_.calibrate();          // adjust to new resolution
	net_.connection_manager_.calibrate(time_converter); // adjust delays in the connection system to new resolution
	net_.message(SLIInterpreter::M_INFO, "Scheduler::set_status", "tics per ms and resolution changed.");
      }
    }
    else if (res_updated) // only resolution changed
    {
      if ( resd < Time::get_ms_per_tic() )
      {
        net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                     "Resolution must be greater than or equal to one tic. Value unchanged.");
        throw KernelException();
      }
      else
      {
	Time::set_resolution(resd);
	clock_.calibrate();          // adjust to new resolution
	net_.connection_manager_.calibrate(time_converter); // adjust delays in the connection system to new resolution
	net_.message(SLIInterpreter::M_INFO, "Scheduler::set_status", "Temporal resolution changed.");
      }
    }
    else
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                   "change of tics_per_step requires simultaneous specification of resolution.");
      throw KernelException();
    }
  }


  bool off_grid_spiking;
  bool grid_spiking_updated = updateValue<bool>(d, "off_grid_spiking", off_grid_spiking);
  if (grid_spiking_updated)
      off_grid_spiking_ = off_grid_spiking;

  bool comm_allgather;
  bool commstyle_updated = updateValue<bool>(d, "communicate_allgather", comm_allgather);
  if (commstyle_updated)
      Communicator::set_use_Allgather(comm_allgather);

  // set RNGs --- MUST come after n_threads_ is updated
  if (d->known("rngs"))
  {
    // this array contains pre-seeded RNGs, so they can be used
    // directly, no seeding required
    ArrayDatum *ad =
       dynamic_cast<ArrayDatum *>((*d)["rngs"].datum());
    if ( ad == 0 )
      throw BadProperty();

    // n_threads_ is the new value after a change of the number of
    // threads
    if (ad->size() != (size_t)(Communicator::get_num_virtual_processes()))
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                 "Number of RNGs must equal number of virtual processes (threads*processes). RNGs unchanged.");
      throw DimensionMismatch((size_t)(Communicator::get_num_virtual_processes()), ad->size());
    }

    // delete old generators, insert new generators this code is
    // robust under change of thread number in this call to
    // set_status, as long as it comes AFTER n_threads_ has been
    // upated
    rng_.clear();
    for (index i = 0 ; i < ad->size() ; ++i)
      if(is_local_vp(i))
        rng_.push_back(getValue<librandom::RngDatum>((*ad)[suggest_vp(i)]));
  }
  else if (n_threads_updated  && net_.size() == 0)
  {
    net_.message(SLIInterpreter::M_WARNING, "Schedulder::set_status", "Equipping threads with new default RNGs");
    create_rngs_();
  }

  if ( d->known("rng_seeds") )
  {
    ArrayDatum *ad = dynamic_cast<ArrayDatum *>((*d)["rng_seeds"].datum());
    if ( ad == 0 )
      throw BadProperty();

    if (ad->size() != (size_t)(Communicator::get_num_virtual_processes()))
    {
      net_.message(SLIInterpreter::M_ERROR, "Scheduler::set_status",
                 "Number of seeds must equal number of virtual processes (threads*processes). RNGs unchanged.");
      throw DimensionMismatch((size_t)(Communicator::get_num_virtual_processes()), ad->size());
    }

    // check if seeds are unique
    std::set<ulong_t> seedset;
    for ( index i = 0 ; i < ad->size() ; ++i )
    {
      long s = (*ad)[i];  // SLI has no ulong tokens
      if ( !seedset.insert(s).second )
      {
        net_.message(SLIInterpreter::M_WARNING, "Scheduler::set_status",
                     "Seeds are not unique across threads!");
        break;
      }
    }

    // now apply seeds, resets generators automatically
    for ( index i = 0 ; i < ad->size() ; ++i )
    {
      long s = (*ad)[i];

      if(is_local_vp(i))
	rng_[vp_to_thread(suggest_vp(i))]->seed(s);

      rng_seeds_[i] = s;
    }
  } // if rng_seeds

 // set GRNG
  if (d->known("grng"))
  {
    // pre-seeded grng that can be used directly, no seeding required
    updateValue<librandom::RngDatum>(d, "grng", grng_);
  }
  else if (n_threads_updated  && net_.size() == 0)
  {
    net_.message(SLIInterpreter::M_WARNING, "Schedulder::set_status", "Equipping threads with new default GRNG");
    create_grng_();
  }

  if ( d->known("grng_seed") )
  {
    long s = getValue<long>(d, "grng_seed");

    // check if grng seed is unique with respect to rng seeds
    // if grng_seed and rng_seeds given in one SetStatus call
    std::set<ulong_t> seedset;
    seedset.insert(s);
    if (d->known("rng_seeds"))
    {
      ArrayDatum *ad_rngseeds =
	dynamic_cast<ArrayDatum *>((*d)["rng_seeds"].datum());
      if ( ad_rngseeds == 0 )
	throw BadProperty();
      for ( index i = 0 ; i < ad_rngseeds->size() ; ++i )
      {
	s = (*ad_rngseeds)[i];  // SLI has no ulong tokens
	if ( !seedset.insert(s).second )
	{
	  net_.message(SLIInterpreter::M_WARNING, "Scheduler::set_status",
		       "Seeds are not unique across threads!");
	  break;
	}
      }
    }
    // now apply seed, resets generator automatically
    grng_seed_ = s;
    grng_->seed(s);

  } // if grng_seed

  long rng_bs;
  if ( updateValue<long>(d, "rng_buffsize", rng_bs) )
  {
    if ( rng_bs < 1 )
      throw BadProperty();

    // now apply seeds, resets generators automatically
    for ( index i = 0 ; i < rng_.size() ; ++i )
      rng_[i]->set_buffsize(rng_bs);
    grng_->set_buffsize(rng_bs);
  }

}

void nest::Scheduler::get_status(DictionaryDatum &d) const
{
  assert(initialized_);

  def<long>(d, "local_num_threads", n_threads_);
  def<long>(d, "total_num_virtual_procs", Communicator::get_num_virtual_processes());
  def<long>(d, "num_processes", Communicator::get_num_processes());

  def<double_t>(d, "time", get_time().get_ms());
  def<long>(d, "to_do", to_do_);
  def<bool>(d, "print_time", print_time_);

  def<double>(d, "tics_per_ms", Time::get_tics_per_ms());
  def<double>(d, "resolution", Time::get_resolution().get_ms());

  delay min_delay = 0;
  delay max_delay = 0;
  compute_delay_extrema_(min_delay, max_delay);
  Time tmp_delay = Time::step(min_delay);
  def<double>(d, "min_delay", tmp_delay.get_ms());
  tmp_delay = Time::step(max_delay);
  def<double>(d, "max_delay", tmp_delay.get_ms());

  def<double>(d, "ms_per_tic", Time::get_ms_per_tic());
  def<double>(d, "tics_per_ms", Time::get_tics_per_ms());
  def<long>(d, "tics_per_step", Time::get_tics_per_step());

  def<double>(d, "T_min", Time::min().get_ms());
  def<double>(d, "T_max", Time::max().get_ms());

  // buffer size is the same for all threads,
  // so we get it from the first thread
  def<long>(d, "rng_buffsize", rng_[0]->get_buffsize());
  (*d)["rng_seeds"] = Token(rng_seeds_);
  def<long>(d, "grng_seed", grng_seed_);
  def<bool>(d, "off_grid_spiking", off_grid_spiking_);
  def<bool>(d, "communicate_allgather", Communicator::get_use_Allgather());
}

void nest::Scheduler::create_rngs_(const bool ctor_call)
{
  // net_.message(SLIInterpreter::M_INFO, ) calls must not be called
  // if create_rngs_ is called from Scheduler::Scheduler(), since net_
  // is not fully constructed then

  // if old generators exist, remove them; since rng_ contains
  // lockPTRs, we don't have to worry about deletion
  if ( rng_.size() > 0 )
  {
    if ( !ctor_call )
      net_.message(SLIInterpreter::M_INFO, "Scheduler::create_rngs_", "Deleting existing random number generators");

    rng_.clear();
  }

  // create new rngs
  if ( !ctor_call )
    net_.message(SLIInterpreter::M_INFO, "Scheduler::create_rngs_", "Creating default RNGs");

  rng_seeds_.resize(Communicator::get_num_virtual_processes());

  for ( index i = 0; i < static_cast<index>(Communicator::get_num_virtual_processes()); ++i )
  {
    unsigned long s = i + 1;
    if(is_local_vp(i))
    {
      /*
        We have to ensure that each thread is provided with a different
        stream of random numbers.  The seeding method for Knuth's LFG
        generator guarantees that different seeds yield non-overlapping
        random number sequences.

        We therefore have to seed with known numbers: using random
        seeds here would run the risk of using the same seed twice.
        For simplicity, we use 1 .. n_vps.
      */
      librandom::RngPtr rng = librandom::RandomGen::create_knuthlfg_rng(s);

      if ( !rng )
      {
        if ( !ctor_call )
          net_.message(SLIInterpreter::M_ERROR, "Scheduler::create_rngs_", "Error initializing knuthlfg");
        else
          std::cerr << "\nScheduler::create_rngs_\n" << "Error initializing knuthlfg" << std::endl;

        throw KernelException();
      }

      rng_.push_back(rng);
    }

    rng_seeds_[i] = s;
  }
}

void nest::Scheduler::create_grng_(const bool ctor_call)
{

  // create new grng
  if ( !ctor_call )
    net_.message(SLIInterpreter::M_INFO, "Scheduler::create_grng_", "Creating new default global RNG");

  // create default RNG with default seed
  grng_ = librandom::RandomGen::create_knuthlfg_rng(librandom::RandomGen::DefaultSeed);

  if ( !grng_ )
    {
      if ( !ctor_call )
	net_.message(SLIInterpreter::M_ERROR, "Scheduler::create_grng_",
		   "Error initializing knuthlfg");
      else
	std::cerr << "\nScheduler::create_grng_\n"
		  << "Error initializing knuthlfg"
		  << std::endl;

      throw KernelException();
    }

  /*
    The seed for the global rng should be different from the seeds
    of the local rngs_ for each thread seeded with 1,..., n_vps.
  */
  long s = Communicator::get_num_virtual_processes() + 1;
  grng_seed_ =  s;
  grng_->seed(s);
}


void nest::Scheduler::collocate_buffers_()
{
  //count number of spikes in registers
  int num_spikes = 0;
  int num_grid_spikes = 0;
  int num_offgrid_spikes = 0;

  std::vector<std::vector<std::vector<uint_t> > >::iterator i;
  std::vector<std::vector<uint_t> >::iterator j;
  for (i = spike_register_.begin(); i != spike_register_.end(); ++i)
    for (j = i->begin(); j != i->end(); ++j)
      num_grid_spikes += j->size();

  std::vector<std::vector<std::vector<OffGridSpike> > >::iterator it;
  std::vector<std::vector<OffGridSpike> >::iterator jt;
  for (it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it)
    for (jt = it->begin(); jt != it->end(); ++jt)
      num_offgrid_spikes += jt->size();

  num_spikes = num_grid_spikes + num_offgrid_spikes;
  if (!off_grid_spiking_)  //on grid spiking
  {
    // make sure buffers are correctly sized and empty
    std::vector<uint_t> tmp(global_grid_spikes_.size(), 0);
    global_grid_spikes_.swap(tmp);

    if (global_grid_spikes_.size() != static_cast<uint_t>(Communicator::get_recv_buffer_size()))
      global_grid_spikes_.resize(Communicator::get_recv_buffer_size(), 0);

    std::vector<uint_t> tmp2(local_grid_spikes_.size(), 0);
    local_grid_spikes_.swap(tmp2);

    if (num_spikes + (n_threads_ * min_delay_) > static_cast<uint_t>(Communicator::get_send_buffer_size()))
      local_grid_spikes_.resize((num_spikes + (min_delay_ * n_threads_)),0);
    else if (local_grid_spikes_.size() < static_cast<uint_t>(Communicator::get_send_buffer_size()))
      local_grid_spikes_.resize(Communicator::get_send_buffer_size(), 0);


    // collocate the entries of spike_registers into local_grid_spikes__
    std::vector<uint_t>::iterator pos = local_grid_spikes_.begin();
    if (num_offgrid_spikes == 0)
      for (i = spike_register_.begin(); i != spike_register_.end(); ++i)
        for (j = i->begin(); j != i->end(); ++j)
        {
          pos = std::copy(j->begin(), j->end(), pos);
          *pos = comm_marker_;
          ++pos;
        }
    else
    {
      std::vector<OffGridSpike>::iterator n;
      it = offgrid_spike_register_.begin();
      for (i = spike_register_.begin(); i != spike_register_.end(); ++i)
      {
        jt = it->begin();
        for (j = i->begin(); j != i->end(); ++j)
        {
          pos = std::copy(j->begin(), j->end(), pos);
          for (n = jt->begin() ; n != jt->end() ; ++n )
          {
            *pos = n->get_gid();
            ++pos;
          }
          *pos = comm_marker_;
          ++pos;
          ++jt;
        }
        ++it;
      }
      for (it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it)
        for (jt = it->begin(); jt != it->end(); ++jt)
	  jt->clear();
    }

    // remove old spikes from the spike_register_
    for (i = spike_register_.begin(); i != spike_register_.end(); ++i)
      for (j = i->begin(); j != i->end(); ++j)
        j->clear();
  }
  else  //off_grid_spiking
  {
    // make sure buffers are correctly sized and empty
    std::vector<OffGridSpike> tmp(global_offgrid_spikes_.size(), OffGridSpike(0,0.0));
    global_offgrid_spikes_.swap(tmp);

    if (global_offgrid_spikes_.size() != static_cast<uint_t>(Communicator::get_recv_buffer_size()))
      global_offgrid_spikes_.resize(Communicator::get_recv_buffer_size(), OffGridSpike(0,0.0));

    std::vector<OffGridSpike> tmp2(local_offgrid_spikes_.size(),  OffGridSpike(0,0.0));
    local_offgrid_spikes_.swap(tmp2);

    if (num_spikes + (n_threads_ * min_delay_) > static_cast<uint_t>(Communicator::get_send_buffer_size()))
      local_offgrid_spikes_.resize((num_spikes + (min_delay_ * n_threads_)), OffGridSpike(0,0.0));
    else if (local_offgrid_spikes_.size() < static_cast<uint_t>(Communicator::get_send_buffer_size()))
      local_offgrid_spikes_.resize(Communicator::get_send_buffer_size(),  OffGridSpike(0,0.0));

    // collocate the entries of spike_registers into local_offgrid_spikes__
    std::vector<OffGridSpike>::iterator pos = local_offgrid_spikes_.begin();
    if (num_grid_spikes == 0)
      for (it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it)
        for (jt = it->begin(); jt != it->end(); ++jt)
        {
          pos = std::copy(jt->begin(), jt->end(), pos);
          pos->set_gid(comm_marker_);
          ++pos;
        }
    else
    {
      std::vector<uint_t>::iterator n;
      i = spike_register_.begin();
      for (it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it)
      {
        j = i->begin();
        for (jt = it->begin(); jt != it->end(); ++jt)
        {
          pos = std::copy(jt->begin(), jt->end(), pos);
          for (n = j->begin() ; n != j->end() ; ++n )
          {
            *pos = OffGridSpike(*n,0);
            ++pos;
          }
          pos->set_gid(comm_marker_);
          ++pos;
          ++j;
        }
        ++i;
      }
      for (i = spike_register_.begin(); i != spike_register_.end(); ++i)
        for (j = i->begin(); j != i->end(); ++j)
          j->clear();
    }

    //empty offgrid_spike_register_
    for (it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it)
      for (jt = it->begin(); jt != it->end(); ++jt)
        jt->clear();
  }
}

void nest::Scheduler::deliver_events_(thread t)
{
  // deliver only at beginning of time slice
  if ( from_step_ > 0 )
    return;

  size_t n_markers = 0;
  SpikeEvent se;

  std::vector<int> pos(displacements_);

  if (!off_grid_spiking_) //on_grid_spiking
  {
    for (size_t vp = 0; vp < (size_t)Communicator::get_num_virtual_processes(); ++vp)
    {
      size_t pid = get_process_id(vp);
      int lag = min_delay_ - 1;
      while(n_markers < min_delay_)
      {
        index nid = global_grid_spikes_[pos[pid]];
        if (nid != comm_marker_)
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp(clock_ - Time::step(lag));
          se.set_sender_gid(nid);
          net_.connection_manager_.send(t, nid, se);
        }
        else
        {
          ++n_markers;
          --lag;
        }
        ++pos[pid];
      }
      n_markers = 0;
    }
  }
  else //off grid spiking
  {
    for (size_t vp = 0; vp < (size_t)Communicator::get_num_virtual_processes(); ++vp)
    {
      size_t pid = get_process_id(vp);
      int lag = min_delay_ - 1;
      while(n_markers < min_delay_)
      {
        index nid = global_offgrid_spikes_[pos[pid]].get_gid();
        if (nid != comm_marker_)
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp(clock_ - Time::step(lag));
          se.set_sender_gid(nid);
          se.set_offset(global_offgrid_spikes_[pos[pid]].get_offset());
          net_.connection_manager_.send(t, nid, se);
        }
        else
        {
          ++n_markers;
          --lag;
        }
        ++pos[pid];
      }
      n_markers = 0;
    }
  }
}

void nest::Scheduler::gather_events_()
{
  collocate_buffers_();
  if (off_grid_spiking_)
    Communicator::communicate(local_offgrid_spikes_, global_offgrid_spikes_, displacements_);
  else
    Communicator::communicate(local_grid_spikes_, global_grid_spikes_, displacements_);
}

void nest::Scheduler::advance_time_()
{
  /*
   * After each time slice, we flip the value of the
   * update_ref_ flag.
   * The updated flag of a Node is changed after each
   * update.
   * We determine whether a Node was updated in the
   * current cycle by comparing its updated flag with the update_ref
   * flag.
   */
  update_ref_= !update_ref_;

  // time now advanced time by the duration of the previous step
  to_do_ -= to_step_ - from_step_;

  // advance clock, update modulos, slice counter only if slice completed
  if ( (delay)to_step_ == min_delay_ )
  {
    clock_ += Time::step(min_delay_);
    ++slice_;
    compute_moduli_();
    from_step_ = 0;
  }
  else
    from_step_ = to_step_;

  long_t end_sim = from_step_ + to_do_;

  if ( min_delay_ < (delay)end_sim )
    to_step_ = min_delay_;    // update to end of time slice
  else
    to_step_ = end_sim;       // update to end of simulation time

  assert(to_step_ - from_step_ <= (long_t)min_delay_);
}

void nest::Scheduler::print_progress_()
{
  double_t rt_factor = 0.0;

  if (t_slice_end_.tv_sec != 0)
  {
    long t_real_s = (t_slice_end_.tv_sec - t_slice_begin_.tv_sec) * 1e6; // usec
    t_real_ += t_real_s + (t_slice_end_.tv_usec - t_slice_begin_.tv_usec); // usec
    double_t t_real_acc = (t_real_) / 1000.; //ms
    double_t t_sim_acc = (to_do_total_ - to_do_) * Time::get_resolution().get_ms();
    rt_factor = t_sim_acc / t_real_acc;
  }

  int_t percentage = (100 - int(float(to_do_) / to_do_total_ * 100));

  std::cout << "\r" << std::setw(3) << std::right << percentage << " %: "
            << "network time: " << std::fixed << std::setprecision(1) << clock_.get_ms() << " ms, "
            << "realtime factor: " << std::setprecision(4) << rt_factor
            << std::resetiosflags(std::ios_base::floatfield);
  std::flush(std::cout);
}


void nest::Scheduler::set_num_threads(thread n_threads)
{
  n_threads_ = n_threads;

#ifdef _OPENMP
  omp_set_num_threads(n_threads_);
#endif
  Communicator::set_num_threads(n_threads_);
}
