/*
 *  scheduler.h
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

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <queue>
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include <sys/time.h>

#include "nest.h"
#include "nest_time.h"
#include "net_thread.h"
#include "mutex.h"
#include "nodelist.h"
#include "event.h"
#include "event_priority.h"
#include "randomgen.h"
#include "lockptr.h"
#include "communicator.h"

namespace nest
{

  using std::priority_queue;
  using std::vector;
  typedef Communicator::OffGridSpike OffGridSpike;

  class Network;
  
  /**
   * Schedule update of Nodes and Events during simulation.
   * The scheduler controls a number of threads which are responsible
   * for updating a batch of Nodes independently from each other. The
   * number of threads as well as the batch size of each thread can be
   * configured with get_status and set_status methods.
   *
   * The scheduler also controls the random number clients which are
   * associated to the threads.
   *
   * The scheduler is usually hidden inside the network class. Thus,
   * its interface is of little interest to the "normal" model
   * developer. 
   */

  class Scheduler {
    
  public: // Public methods

    Scheduler(Network &);
    virtual ~Scheduler();

    /**
     * Bring scheduler back to its initial state.
     * @note Threading parameters as well as random number state 
     * are not reset. This has to be done manually.
     */
    void reset();

    /**
     * Clear all pending spikes, but do not otherwise manipulate scheduler.
     * @note This is used by Network::reset_network().
     */
    void clear_pending_spikes();

    /** Simulate for the given time */
    void simulate(Time const&);

    /** Resume simulation after an interrupt. */
    void resume();

    void terminate();

    /**
     * Add global id of event sender to the spike_register. 
     * An event sent through this method will remain in the queue until
     * the network time has advanced by min_delay_ steps. After this period
     * the buffers are collocated and sent to the partner machines.

     * Old documentation from network.h:
     * Place an event in the global event queue.
     * Add event to the queue to be delivered
     * when it is due.
     * At the delivery time, the target list of the sender is iterated
     * and the event is delivered to all targets.
     * The event is guaranteed to arrive at the receiver when all
     * elements are updated and the system is 
     * in a synchronised (single threaded) state.
     * @see send_to_targets()
     */
    void send_remote(thread p, SpikeEvent&, const long_t lag = 0);
    
    /**
     * Add global id of event sender to the spike_register. 
     * Store event offset with global id.
     * An event sent through this method will remain in the queue until
     * the network time has advanced by min_delay_ steps. After this period
     * the buffers are collocated and sent to the partner machines.

     * Old documentation from network.h:
     * Place an event in the global event queue.
     * Add event to the queue to be delivered
     * when it is due.
     * At the delivery time, the target list of the sender is iterated
     * and the event is delivered to all targets.
     * The event is guaranteed to arrive at the receiver when all
     * elements are updated and the system is 
     * in a synchronised (single threaded) state.
     * @see send_to_targets()
     */
    void send_offgrid_remote(thread p, SpikeEvent&, const long_t lag = 0);

    /**
     * Return the number of threads used during simulation.
     * This functions returns the number of threads per process. 
     * Since each process has the same number of threads, the total number
     * of threads is given by get_num_threads()*get_num_processes().
     */
    thread get_num_threads() const;
    
    /**
     * Set the number of threads by setting the internal variable
     * n_threads_, the corresponding value in the Communicator, and
     * the OpenMP number of threads.
     */
    void set_num_threads(thread n_threads);
    
    /**
     * Return the number of processes used during simulation.
     * This functions returns the number of processes. 
     * Since each process has the same number of threads, the total number
     * of threads is given by get_num_threads()*get_num_processes().
     */
    thread get_num_processes() const;

    /**
     * Return true if the node on the local machine, false if not.
     */
    bool is_local_node(Node*) const;
    
    /**
     * Return true if the thread is on the local machine, false if not.
     */
    bool is_local_vp(thread) const;
    
    /**
     * Return a thread number for a given global node id.
     * Each node has a default thread on which it will run. 
     * The thread is defined by the relation:
     * t = (gid div P) mod T, where P is the number of processes and 
     * T the number of threads. This may be used by network::add_node()
     * if the user has not specified anything.
     */
    thread suggest_vp(index gid) const;

    thread vp_to_thread(thread vp) const;
    
    thread thread_to_vp(thread t) const;

    /**
     * Return the global thread id of a local thread.
     */
    thread get_global_thread_id(thread lt) const;
        
    /**
     * Return the process id for a given virtual process. The real process' id
     * of a virtual process is defined by the relation: p = (vp mod P), where
     * P is the total number of processes.
     */
    thread get_process_id(thread vp) const;

    /**
     * Return true, if the network has already been simulated for some time.
     * This does NOT indicate that simulate has been called (i.e. if Simulate
     * is called with 0 as argument, the flag is still set to false.)
     */
    bool get_simulated() const;

    /**
     * set communication style to off_grid (true) or on_grid
     */
    void set_off_grid_communication(bool off_grid_spiking);

    /**
     * return current communication style.
     * A result of true means off_grid, false means on_grid communication.
     */
    bool get_off_grid_communication() const;

    /**
     * Time at beginning of current slice.
     */
    Time const& get_slice_origin() const;

    /**
     * Time at beginning of previous slice.
     */
    Time get_previous_slice_origin() const;

    /**
     * Precise time of simulation.
     * @note The precise time of the simulation is defined only
     *       while the simulation is not in progress.
     */
    Time const get_time() const;

    bool is_busy() const;
    bool is_updated() const;
    bool update_reference() const;

    /**
     * Update a fixed set of nodes per thread using pthreads.
     * This is called by a thread.
     */
    void threaded_update(thread);

    /**
     * Update a fixed set of nodes per thread using OpenMP.
     */
    void threaded_update_openmp();

    /** Update without any threading. */
    void serial_update();
 
    void set_network_(Network*);

    void set_status(const DictionaryDatum&);
    void get_status(DictionaryDatum &) const;

    /**
     * Return pointer to random number generator of the specified thread.
     */
    librandom::RngPtr get_rng(const thread) const;
    /**
     * Return pointer to global random number generator
     */
    librandom::RngPtr get_grng() const;

    /**
     * Return (T+d) mod max_delay.
     */
    static
    delay get_modulo(delay d);

    /**
     * Index to slice-based buffer.
     * Return ((T+d)/min_delay) % ceil(max_delay/min_delay).
     */
    static
    delay get_slice_modulo(delay d);

    /**
     * Return minimal connection delay.
     */
    static
    delay get_min_delay(); 

    /**
     * Return maximal connection delay.
     */
    static
    delay get_max_delay();

    /**
     * Get slice number. Increased by one for each slice. Can be used
     * to choose alternating buffers.
     */
    size_t get_slice() const;

    /**
     * Calibrate clock after resolution change.
     */
    void calibrate_clock();

  private:

    /**
     * Initialize the scheduler by initializing the buffers.
     */
    void init_();

    /**
     * Finalize the scheduler by freeing the buffers and destoying the mutexes.
     */
    void finalize_();
    
    void update_(Node*);
    void advance_time_();

    void print_progress_();

    /**
     * Prepare nodes for simulation and register nodes in node_list.
     * Calls prepare_node_() for each pertaining Node.
     * @see prepare_node_()
     */
    void prepare_nodes();

    /**
     * Calibrate, initialized buffers, register in list of nodes to update/finalize.
     * @see prepare_nodes()
     */
    void prepare_node_(Node *);
    
    /**
     * Invoke finalize() on nodes registered for finalization.
     */
    void finalize_nodes();

    /**
     * Re-compute table of fixed modulos, including slice-based.
     */
    void compute_moduli_(); 
    
    void init_moduli_();

    void create_rngs_(const bool ctor_call = false);
    void create_grng_(const bool ctor_call = false);

    void compute_delay_extrema_(delay&, delay&) const;
    
    Mutex ready_mutex_;
    Mutex terminate_mutex_; //!< protect terminate() calls.

    bool initialized_;
    bool simulating_;  //!< true if simulation in progress
    bool force_singlethreading_;

    index n_threads_; //!< Number of threads per process.
    index n_nodes_;   //!< Effective number of simulated nodes.

    volatile index   entry_counter_; //!< Counter for entry barrier.
    volatile index   exit_counter_;  //!< Counter for exit barrier.

#ifdef HAVE_PTHREADS
    pthread_cond_t   ready_;
    pthread_cond_t   done_;
#endif

    vector<Thread>   threads_;
    vector<vector<Node*> > nodes_vec_;   //!< Nodelists for unfrozen nodes
    
    Network  &net_;         //!< Reference to network object.
    Time     clock_;        //!< Network clock, updated once per slice
    long_t   slice_;        //!< current update slice
    long_t   to_do_;        //!< number of pending cycles.
    long_t   to_do_total_;  //!< number of requested cycles in current simulation.
    long_t   from_step_;    //!< update clock_+from_step<=T<clock_+to_step_
    long_t   to_step_;      //!< update clock_+from_step<=T<clock_+to_step_

    timeval t_slice_begin_; //!< Wall-clock time at the begin of a time slice
    timeval t_slice_end_;   //!< Wall-clock time at the end of time slice
    long t_real_;           //!< Accumunated wall-clock time spent simulating (in us) 
    
    bool update_ref_;       //!< reference for node update state.
    bool terminate_;        //!< Terminate on signal or error
    bool simulated_;        //!< indicates whether the network has already been simulated for some time
    bool off_grid_spiking_; //!< indicates whether spikes are not constrained to the grid 
    bool print_time_;       //!< Indicates whether time should be printed during simulations (or not)

    std::vector<long_t> rng_seeds_;  //!< The seeds of the local RNGs. These do not neccessarily describe the state of the RNGs.
    long_t grng_seed_;   //!< The seed of the global RNG, not neccessarily describing the state of the GRNG.
    
    static
    delay min_delay_; //!< Value of the smallest delay in the network.

    static
    delay max_delay_; //!< Value of the largest delay in the network in steps.
    
    /** 
     * Table of pre-computed modulos.
     * This table is used to map time steps, given as offset from now,
     * to ring-buffer bins.  There are min_delay+max_delay bins in a ring buffer,
     * and the moduli_ array is rotated by min_delay elements after
     * each slice is completed.
     * @see RingBuffer
     */
    static 
    vector<delay> moduli_; 

    /**
     * Table of pre-computed slice-based modulos.
     * This table is used to map time steps, give as offset from now,
     * to slice-based ring-buffer bins.  There are ceil(max_delay/min_delay)
     * bins in a slice-based ring buffer, one per slice within max_delay.
     * Since max_delay may not be a multiple of min_delay, we cannot simply
     * rotate the table content after each slice, but have to recompute
     * the table anew.
     * @see SliceRingBuffer
     */
    static 
    vector<delay> slice_moduli_;

    /**
     * Vector of random number generators for threads.
     * There must be PRECISELY one rng per thread.
     */
    vector<librandom::RngPtr> rng_;
    /**
     * Global random number generator.
     * This rng must be synchronized on all threads
     */
    librandom::RngPtr grng_;

    /** 
     * Register for gids of neurons that spiked. This is a 3-dim
     * structure.
     * - First dim: Each thread has its own vector to write to.
     * - Second dim: A vector for each slice of the min_delay interval
     * - Third dim: The gids.
     */
    std::vector<std::vector<std::vector<uint_t> > > spike_register_;

    /** 
     * Register for off-grid spikes.
     * This is a 3-dim structure.
     * - First dim: Each thread has its own vector to write to.
     * - Second dim: A vector for each slice of the min_delay interval
     * - Third dim: Struct containing GID and offset.
     */
    std::vector<std::vector<std::vector<OffGridSpike> > > 
      offgrid_spike_register_;

    /**
     * Buffer containing the gids of local neurons that spiked in the 
     * last min_delay_ interval. The single slices are separated by a
     * marker value. 
     */
    std::vector<uint_t> local_grid_spikes_;

    /**
     * Buffer containing the gids of all neurons that spiked in the 
     * last min_delay_ interval. The single slices are separated by a
     * marker value 
     */
    std::vector<uint_t> global_grid_spikes_;

    /**
     * Buffer containing the gids and offsets for local neurons that
     * fired off-grid spikes in the last min_delay_ interval. The 
     * single slices are separated by a marker value. 
     */
     std::vector<OffGridSpike> local_offgrid_spikes_;

    /**
     * Buffer containing the gids and offsets for all neurons that
     * fired off-grid spikes in the last min_delay_ interval. The 
     * single slices are separated by a marker value. 
     */
     std::vector<OffGridSpike> global_offgrid_spikes_;  

    /**
     * Buffer containing the starting positions for the spikes from
     * each process within the global_(off)grid_spikes_ buffer.
     */
     std::vector<int> displacements_;
          

    /**
     * Marker Value to be put between the data fields from different time
     * steps during communication. 
     */
    static
    const uint_t comm_marker_;

    /**
     * Resize spike_register and comm_buffer to correct dimensions.
     * Resizes also offgrid_*_buffer_.
     * This is done by resume() when called for the first time.
     * The spike buffers cannot be reconfigured later, whence neither
     * the number of local threads or the min_delay can change after
     * simulate() has been called. ConnectorModel::check_delay() and
     * Scheduler::set_status() ensure this.
     */
    void configure_spike_buffers_();
    
    /**
     * Clear nodes_vec_ prior to each network calibration.
     */  
    void clear_nodes_vec_();

    /**
     * Rearrange the spike_register into a 2-dim structure. This is
     * done by collecting the spikes from all threads in each slice of
     * the min_delay_ interval.
     */
    void collocate_buffers_();

    /**
     * Collocate buffers and exchange events with other MPI processes.
     */
    void gather_events_();

    /**
     * Read all event buffers for thread t and send the corresponding
     * Events to the Nodes that are targeted.
     *
     * @note It is a crucial property of deliver_events_() that events
     * are delivered ordered by non-decreasing time stamps. BUT: this 
     * ordering applies to time stamps only, it does NOT take into 
     * account the offsets of precise spikes.
     */
    void deliver_events_(thread t);
  };

  /**
   * Used by threads to update the element.
   * Thus, we keep the responsibility of calling
   * private Node methods at the Scheduler.
   * 
   * This function will perform two things:
   * -# Call the node's update method @see Node::update
   * -# Change the updated flag at the Node.
   */
  inline
  void Scheduler::update_(Node *n)
  {
    n->update(clock_, from_step_, to_step_);
    n->flip(Node::updated);
  }

  inline
  bool Scheduler::is_busy() const
  {
    return (to_do_ != 0) && (! terminate_);
  }

  inline
  Time const& Scheduler::get_slice_origin() const
  {
    return clock_;
  }
  
  inline
  Time Scheduler::get_previous_slice_origin() const
  {
    return clock_ - Time::step(min_delay_);
  }

  inline
  Time const Scheduler::get_time() const
  {
    assert(!simulating_);
    return clock_ + Time::step(from_step_);
  }

  inline 
  thread Scheduler::get_num_threads() const
  {
    return n_threads_;
  }

  inline 
  thread Scheduler::get_num_processes() const
  {
    return Communicator::get_num_processes();
  }

  inline
  thread Scheduler::get_process_id(thread vp) const
  {
    return vp % Communicator::get_num_processes();
  }

  inline
  bool Scheduler::is_local_node(Node* n) const
  {
    return is_local_vp(n->get_vp());
  }

  inline
  bool Scheduler::is_local_vp(thread vp) const
  {
    return get_process_id(vp) == Communicator::get_rank();
  }

  inline
  thread Scheduler::suggest_vp(index gid) const
  {
    return gid % Communicator::get_num_virtual_processes(); 
  }

  inline
  thread Scheduler::vp_to_thread(thread vp) const
  {
    return vp / Communicator::get_num_processes();
  }

  inline
  thread Scheduler::thread_to_vp(thread t) const
  {
    return t * Communicator::get_num_processes() + Communicator::get_rank();
  }

  inline
  bool Scheduler::get_simulated() const
  {
    return simulated_;
  }

  inline
  void Scheduler::set_off_grid_communication(bool off_grid_spiking)
  {
    off_grid_spiking_ = off_grid_spiking;
  }

  inline
  bool Scheduler::get_off_grid_communication() const
  {
    return off_grid_spiking_;
  }
  
  inline 
  bool Scheduler::is_updated() const
  {
    return (to_do_==0) || terminate_;
  }

  inline 
  bool Scheduler::update_reference() const
  {
    return update_ref_;
  }

  inline
  librandom::RngPtr Scheduler::get_rng(const thread thrd) const
  {
    assert(thrd < static_cast<thread>(rng_.size()));
    return rng_[thrd];
  }

  inline
  librandom::RngPtr Scheduler::get_grng() const
  {
    return grng_;
  }

  inline 
  void Scheduler::calibrate_clock()
  {
    clock_.calibrate();
  }

  inline
  void Scheduler::prepare_node_(Node *n)
  {
    // Frozen nodes are initialized and calibrated, so that they
    // have ring buffers and can accept incoming spikes. They
    // are not placed in the nodes_vec_, as they shall not be
    // updated. See #414.
    n->init_buffers();
    n->calibrate();

    if(n->is_frozen())
      return;
    
    nodes_vec_[n->get_thread()].push_back(n);
  }

  inline
  void Scheduler::send_remote(thread t, SpikeEvent& e, const long_t lag)
  {
    // Put the spike in a buffer for the remote machines
    for (int_t i = 0; i < e.get_multiplicity(); ++i)
      spike_register_[t][lag].push_back(e.get_sender().get_gid());
  }

  inline
  void Scheduler::send_offgrid_remote(thread t, SpikeEvent& e, const long_t lag)
  {
    // Put the spike in a buffer for the remote machines
    OffGridSpike ogs(e.get_sender().get_gid(), e.get_offset());
    for (int_t i = 0; i < e.get_multiplicity(); ++i)
      offgrid_spike_register_[t][lag].push_back(ogs);
  }

  inline
  delay Scheduler::get_modulo(delay d)
  {
    // Note, here d may be 0, since bin 0 represents the "current" time
    // when all evens due are read out.
    assert(d < moduli_.size());

    return moduli_[d];
  }

  inline
  delay Scheduler::get_slice_modulo(delay d)
  {
    // Note, here d may be 0, since bin 0 represents the "current" time
    // when all evens due are read out.
    assert(d < slice_moduli_.size());

    return slice_moduli_[d];
  }

  inline
  delay Scheduler::get_min_delay()
  {
    return min_delay_;
  }

  inline
  delay Scheduler::get_max_delay()
  {
    return max_delay_;
  }

  inline
  size_t Scheduler::get_slice() const
  {
    return slice_;
  }
  
  inline 
  void Scheduler::terminate()
  {
    terminate_mutex_.lock();
    terminate_=true;
    terminate_mutex_.unlock();
  }
}

#endif //SCHEDULER_H
