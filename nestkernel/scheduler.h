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
#include "nodelist.h"
#include "event.h"
#include "event_priority.h"
#include "randomgen.h"
#include "lockptr.h"
#include "communicator.h"
#include "connector_model.h"

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

class Scheduler
{

public: // Public methods
  Scheduler( Network& );
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

  /**
   * Simulate for the given time .
   * This function performs the following steps
   * 1. set the new simulation time
   * 2. call prepare_simulation()
   * 3. call resume()
   * 4. call finalize_simulation()
   */
  void simulate( Time const& );

  /** Resume simulation after an interrupt. */
  void resume();

  /**
   * All steps that must be done before a simulation.
   */
  void prepare_simulation();

  /**
   * Cleanup after the simulation.
   */
  void finalize_simulation();

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
  void send_remote( thread t, SpikeEvent& e, const long_t lag = 0 );
  void send_remote( thread t, SecondaryEvent& e );


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
  void send_offgrid_remote( thread p, SpikeEvent&, const long_t lag = 0 );

  Node* thread_lid_to_node( thread t, targetindex thread_local_id ) const;

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
  void set_num_threads( thread n_threads );

  /**
   * Return the number of processes used during simulation.
   * This functions returns the number of processes.
   * Since each process has the same number of threads, the total number
   * of threads is given by get_num_threads()*get_num_processes().
   */
  thread get_num_processes() const;

  thread get_num_rec_processes() const;
  thread get_num_sim_processes() const;

  /**
   * Set number of recording processes, switches NEST to global
   * spike detection mode.
   *
   * @param nrp  number of recording processes
   * @param called_by_reset   pass true when calling from Scheduler::reset()
   *
   * @note The `called_by_reset` parameter is a cludge to avoid a chicken-and-egg
   *       problem when resetting the kernel. It surpresses a test for existing
   *       nodes, trusting that the kernel will immediately afterwards delete all
   *       existing nodes.
   */
  void set_num_rec_processes( int nrp, bool called_by_reset = false );

  /**
   * Increment total number of global spike detectors by 1
   */
  void increment_n_gsd();

  /**
   * Get total number of global spike detectors
   */
  index get_n_gsd();

  /**
   * Return true if the node on the local machine, false if not.
   */
  bool is_local_node( Node* ) const;

  /**
   * Return true if the thread is on the local machine, false if not.
   */
  bool is_local_vp( thread ) const;

  /**
   * Return a thread number for a given global node id.
   * Each node has a default thread on which it will run.
   * The thread is defined by the relation:
   * t = (gid div P) mod T, where P is the number of simulation processes and
   * T the number of threads. This may be used by network::add_node()
   * if the user has not specified anything.
   */
  thread suggest_vp( index gid ) const;

  /**
   * Return a thread number for a given global recording node id.
   * Each node has a default thread on which it will run.
   * The thread is defined by the relation:
   * t = (gid div P) mod T, where P is the number of recording processes and
   * T the number of threads. This may be used by network::add_node()
   * if the user has not specified anything.
   */
  thread suggest_rec_vp( index gid ) const;

  thread vp_to_thread( thread vp ) const;

  thread thread_to_vp( thread t ) const;

  /**
   * Return the global thread id of a local thread.
   */
  thread get_global_thread_id( thread lt ) const;

  /**
   * Return the process id for a given virtual process. The real process' id
   * of a virtual process is defined by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  thread get_process_id( thread vp ) const;

  /**
   * Return true, if the network has already been simulated for some time.
   * This does NOT indicate that simulate has been called (i.e. if Simulate
   * is called with 0 as argument, the flag is still set to false.)
   */
  bool get_simulated() const;

  /**
   * set communication style to off_grid (true) or on_grid
   */
  void set_off_grid_communication( bool off_grid_spiking );

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

  /** Update all non-frozen nodes. This function uses OpenMP
    * for multi-threading if enabled at configure time and runs
    * with a single thread otherwise.
    */
  void update();

  void set_network_( Network* );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  /**
   * Return pointer to random number generator of the specified thread.
   */
  librandom::RngPtr get_rng( const thread ) const;
  /**
   * Return pointer to global random number generator
   */
  librandom::RngPtr get_grng() const;

  /**
   * Return (T+d) mod max_delay.
   */
  static delay get_modulo( delay d );

  /**
   * Index to slice-based buffer.
   * Return ((T+d)/min_delay) % ceil(max_delay/min_delay).
   */
  static delay get_slice_modulo( delay d );

  /**
   * Return minimal connection delay.
   */
  static delay get_min_delay();

  static double_t get_prelim_tol();
  static size_t get_prelim_interpolation_order();

  /**
   * Return maximal connection delay.
   */
  static delay get_max_delay();

  /**
   * Get slice number. Increased by one for each slice. Can be used
   * to choose alternating buffers.
   */
  size_t get_slice() const;

  /**
   * Calibrate clock after resolution change.
   */
  void calibrate_clock();

  void register_secondary_synapse_prototype( ConnectorModel* cm, synindex synid );
  void create_secondary_events_prototypes();
  void delete_secondary_events_prototypes();

  /**
   * Ensure that all nodes in the network have valid thread-local IDs.
   */
  void
  ensure_valid_thread_local_ids()
  {
    update_nodes_vec_();
  }

private:
  /**
   * Initialize the scheduler by initializing the buffers.
   */
  void init_();

  /**
   * Finalize the scheduler by freeing the buffers and destoying the mutexes.
   */
  void finalize_();

  bool prelim_update_( Node* );

  void advance_time_();

  void print_progress_();

  /**
   * Prepare nodes for simulation and register nodes in node_list.
   * Calls prepare_node_() for each pertaining Node.
   * @see prepare_node_()
   */
  void prepare_nodes();

  /**
   * Initialized buffers, register in list of nodes to update/finalize.
   * @see prepare_nodes()
   */
  void prepare_node_( Node* );

  /**
   * Invoke finalize() on nodes registered for finalization.
   */
  void finalize_nodes();

  /**
   * Re-compute table of fixed modulos, including slice-based.
   */
  void compute_moduli_();

  void init_moduli_();

  void create_rngs_( const bool ctor_call = false );
  void create_grng_( const bool ctor_call = false );

  /**
   * Update delay extrema to current values.
   *
   * Static since it only operates in static variables. This allows it to be
   * called from const-method get_status() as well.
   */
  static void update_delay_extrema_();

  bool initialized_;
  bool simulating_; //!< true if simulation in progress
  bool force_singlethreading_;

  index n_threads_; //!< Number of threads per process.

  index n_rec_procs_; //!< MPI processes dedicated for recording devices
  index n_sim_procs_; //!< MPI processes used for simulation

  index n_gsd_; //!< Total number of global spike detectors, used for distributing them over
                //!< recording processes

  volatile index entry_counter_; //!< Counter for entry barrier.
  volatile index exit_counter_;  //!< Counter for exit barrier.

  vector< vector< Node* > > nodes_vec_; //!< Nodelists for nodes for each thread
  index nodes_vec_network_size_;        //!< Network size when nodes_vec_ was last updated

  vector< vector< Node* > > nodes_prelim_up_vec_; //!< Nodelists for unfrozen nodes that require an
  // additional preliminary update (e.g. gap
  // junctions)

  Time clock_;        //!< Network clock, updated once per slice
  delay slice_;       //!< current update slice
  delay to_do_;       //!< number of pending cycles.
  delay to_do_total_; //!< number of requested cycles in current simulation.
  delay from_step_;   //!< update clock_+from_step<=T<clock_+to_step_
  delay to_step_;     //!< update clock_+from_step<=T<clock_+to_step_

  timeval t_slice_begin_; //!< Wall-clock time at the begin of a time slice
  timeval t_slice_end_;   //!< Wall-clock time at the end of time slice
  long t_real_;           //!< Accumunated wall-clock time spent simulating (in us)

  bool terminate_; //!< Terminate on signal or error
  bool simulated_; //!< indicates whether the network has already been simulated for some time
  bool off_grid_spiking_; //!< indicates whether spikes are not constrained to the grid
  bool print_time_;       //!< Indicates whether time should be printed during simulations (or not)

  bool needs_prelim_update_; //!< there is at least one neuron model that needs preliminary update
  long max_num_prelim_iterations_; //!< maximal number of iterations used for preliminary update

  static size_t prelim_interpolation_order; //!< interpolation order for prelim iterations

  static double_t prelim_tol; //!< Tolerance of prelim iterations

  std::vector< long_t > rng_seeds_; //!< The seeds of the local RNGs. These do not neccessarily
                                    //!< describe the state of the RNGs.
  long_t
    grng_seed_; //!< The seed of the global RNG, not neccessarily describing the state of the GRNG.

  /**
   * Pointer to network object.
   *
   * Maintained as a static pointer so that update_delay_extrema_() can be a static function
   * and update thet static min/max_delay_ variables even from get_status() const.
   */
  static Network* net_;

  /**
   *  Value of the smallest delay in the network.
   *
   *  Static because get_min_delay() must be static to allow access from neuron models.
   */
  static delay min_delay_;

  /**
   *  Value of the largest delay in the network.
   *
   *  Static because get_max_delay() must be static to allow access from neuron models.
   */
  static delay max_delay_;

  /**
   * Table of pre-computed modulos.
   * This table is used to map time steps, given as offset from now,
   * to ring-buffer bins.  There are min_delay+max_delay bins in a ring buffer,
   * and the moduli_ array is rotated by min_delay elements after
   * each slice is completed.
   * @see RingBuffer
   */
  static vector< delay > moduli_;

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
  static vector< delay > slice_moduli_;

  /**
   * Vector of random number generators for threads.
   * There must be PRECISELY one rng per thread.
   */
  vector< librandom::RngPtr > rng_;
  /**
   * Global random number generator.
   * This rng must be synchronized on all threads
   */
  librandom::RngPtr grng_;

  /**
   * prototypes of events
   */
  std::vector< Event* > event_prototypes_;

  /**
   * Register for gids of neurons that spiked. This is a 3-dim
   * structure.
   * - First dim: Each thread has its own vector to write to.
   * - Second dim: A vector for each slice of the min_delay interval
   * - Third dim: The gids.
   */
  std::vector< std::vector< std::vector< uint_t > > > spike_register_;

  /**
   * Register for off-grid spikes.
   * This is a 3-dim structure.
   * - First dim: Each thread has its own vector to write to.
   * - Second dim: A vector for each slice of the min_delay interval
   * - Third dim: Struct containing GID and offset.
   */
  std::vector< std::vector< std::vector< OffGridSpike > > > offgrid_spike_register_;

  /**
   * Buffer to collect the secondary events
   * after serialization.
   */
  std::vector< std::vector< uint_t > > secondary_events_buffer_;

  /**
   * Buffer containing the gids of local neurons that spiked in the
   * last min_delay_ interval. The single slices are separated by a
   * marker value.
   */
  std::vector< uint_t > local_grid_spikes_;

  /**
   * Buffer containing the gids of all neurons that spiked in the
   * last min_delay_ interval. The single slices are separated by a
   * marker value
   */
  std::vector< uint_t > global_grid_spikes_;

  /**
   * Buffer containing the gids and offsets for local neurons that
   * fired off-grid spikes in the last min_delay_ interval. The
   * single slices are separated by a marker value.
   */
  std::vector< OffGridSpike > local_offgrid_spikes_;

  /**
   * Buffer containing the gids and offsets for all neurons that
   * fired off-grid spikes in the last min_delay_ interval. The
   * single slices are separated by a marker value.
   */
  std::vector< OffGridSpike > global_offgrid_spikes_;

  /**
   * Buffer containing the starting positions for the spikes from
   * each process within the global_(off)grid_spikes_ buffer.
   */
  std::vector< int > displacements_;


  std::vector< ConnectorModel* > secondary_connector_models_;
  std::vector< std::vector< SecondaryEvent* > > secondary_events_prototypes_;

  /**
   * Marker Value to be put between the data fields from different time
   * steps during communication.
   */
  static const delay comm_marker_;

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
   * Create up-to-date vector of local nodes, nodes_vec_.
   *
   * This method also sets the thread-local ID on all local nodes.
   */
  void update_nodes_vec_();

  /**
   * Rearrange the spike_register into a 2-dim structure. This is
   * done by collecting the spikes from all threads in each slice of
   * the min_delay_ interval.
   */
  void collocate_buffers_( bool );

  /**
   * Collocate buffers and exchange events with other MPI processes.
   */
  void gather_events_( bool );

  /**
   * Read all event buffers for thread t and send the corresponding
   * Events to the Nodes that are targeted.
   *
   * @note It is a crucial property of deliver_events_() that events
   * are delivered ordered by non-decreasing time stamps. BUT: this
   * ordering applies to time stamps only, it does NOT take into
   * account the offsets of precise spikes.
   */
  bool deliver_events_( thread t );
};

inline bool
Scheduler::prelim_update_( Node* n )
{
  return ( n->prelim_update( clock_, from_step_, to_step_ ) );
}

inline Time const&
Scheduler::get_slice_origin() const
{
  return clock_;
}

inline Time
Scheduler::get_previous_slice_origin() const
{
  return clock_ - Time::step( min_delay_ );
}

inline Time const
Scheduler::get_time() const
{
  assert( !simulating_ );
  return clock_ + Time::step( from_step_ );
}

inline thread
Scheduler::get_num_threads() const
{
  return n_threads_;
}

inline thread
Scheduler::get_num_processes() const
{
  return Communicator::get_num_processes();
}

inline thread
Scheduler::get_num_rec_processes() const
{
  return n_rec_procs_;
}

inline thread
Scheduler::get_num_sim_processes() const
{
  return n_sim_procs_;
}

inline void
Scheduler::increment_n_gsd()
{
  ++n_gsd_;
}

inline index
Scheduler::get_n_gsd()
{
  return n_gsd_;
}

inline thread
Scheduler::get_process_id( thread vp ) const
{
  if ( vp >= static_cast< thread >( n_sim_procs_ * n_threads_ ) ) // vp belongs to recording VPs
  {
    return ( vp - n_sim_procs_ * n_threads_ ) % n_rec_procs_ + n_sim_procs_;
  }
  else // vp belongs to simulating VPs
  {
    return vp % n_sim_procs_;
  }
}

inline bool
Scheduler::is_local_node( Node* n ) const
{
  return is_local_vp( n->get_vp() );
}

inline bool
Scheduler::is_local_vp( thread vp ) const
{
  return get_process_id( vp ) == Communicator::get_rank();
}

inline thread
Scheduler::suggest_vp( index gid ) const
{
  return gid % ( n_sim_procs_ * n_threads_ );
}

inline thread
Scheduler::suggest_rec_vp( index gid ) const
{
  return gid % ( n_rec_procs_ * n_threads_ ) + n_sim_procs_ * n_threads_;
}

inline thread
Scheduler::vp_to_thread( thread vp ) const
{
  if ( vp >= static_cast< thread >( n_sim_procs_ * n_threads_ ) )
  {
    return ( vp + n_sim_procs_ * ( 1 - n_threads_ ) - Communicator::get_rank() ) / n_rec_procs_;
  }
  else
  {
    return vp / n_sim_procs_;
  }
}

inline thread
Scheduler::thread_to_vp( thread t ) const
{
  if ( Communicator::get_rank()
    >= static_cast< int >( n_sim_procs_ ) ) // Rank is a recording process
  {
    return t * n_rec_procs_ + Communicator::get_rank() - n_sim_procs_ + n_sim_procs_ * n_threads_;
  }
  else // Rank is a simulating process
  {
    return t * n_sim_procs_ + Communicator::get_rank();
  }
}

inline bool
Scheduler::get_simulated() const
{
  return simulated_;
}

inline void
Scheduler::set_off_grid_communication( bool off_grid_spiking )
{
  off_grid_spiking_ = off_grid_spiking;
}

inline bool
Scheduler::get_off_grid_communication() const
{
  return off_grid_spiking_;
}

inline librandom::RngPtr
Scheduler::get_rng( const thread thrd ) const
{
  assert( thrd < static_cast< thread >( rng_.size() ) );
  return rng_[ thrd ];
}

inline librandom::RngPtr
Scheduler::get_grng() const
{
  return grng_;
}

inline void
Scheduler::calibrate_clock()
{
  clock_.calibrate();
}

inline void
Scheduler::prepare_node_( Node* n )
{
  // Frozen nodes are initialized and calibrated, so that they
  // have ring buffers and can accept incoming spikes.
  n->init_buffers();
  n->calibrate();
}

inline void
Scheduler::register_secondary_synapse_prototype( ConnectorModel* cm, synindex synid )
{
  // idea: save *cm in data structure
  // otherwise when number of threads is increased no way to get further elements
  if ( secondary_connector_models_.size() < synid + ( unsigned int ) 1 )
    secondary_connector_models_.resize( synid + 1, NULL );

  secondary_connector_models_[ synid ] = cm;
}

inline void
Scheduler::create_secondary_events_prototypes()
{
  if ( secondary_events_prototypes_.size() < n_threads_ )
  {
    delete_secondary_events_prototypes();
    std::vector< SecondaryEvent* > prototype;
    prototype.resize( secondary_connector_models_.size(), NULL );
    secondary_events_prototypes_.resize( n_threads_, prototype );


    for ( size_t i = 0; i < secondary_connector_models_.size(); i++ )
    {
      if ( secondary_connector_models_[ i ] != NULL )
      {
        prototype = secondary_connector_models_[ i ]->create_event( n_threads_ );
        for ( size_t j = 0; j < secondary_events_prototypes_.size(); j++ )
          secondary_events_prototypes_[ j ][ i ] = prototype[ j ];
      }
    }
  }
}

inline void
Scheduler::delete_secondary_events_prototypes()
{
  for ( size_t i = 0; i < secondary_connector_models_.size(); i++ )
  {
    if ( secondary_connector_models_[ i ] != NULL )
    {
      for ( size_t j = 0; j < secondary_events_prototypes_.size(); j++ )
        delete secondary_events_prototypes_[ j ][ i ];
    }
  }

  for ( size_t j = 0; j < secondary_events_prototypes_.size(); j++ )
    secondary_events_prototypes_[ j ].clear();
  secondary_events_prototypes_.clear();
}

inline Node*
Scheduler::thread_lid_to_node( thread t, targetindex thread_local_id ) const
{
  return nodes_vec_[ t ][ thread_local_id ];
}

inline void
Scheduler::send_remote( thread t, SpikeEvent& e, const delay lag )
{
  // Put the spike in a buffer for the remote machines
  for ( int_t i = 0; i < e.get_multiplicity(); ++i )
    spike_register_[ t ][ lag ].push_back( e.get_sender().get_gid() );
}

inline void
Scheduler::send_remote( thread t, SecondaryEvent& e )
{

  // put the secondary events in a buffer for the remote machines
  size_t old_size = secondary_events_buffer_[ t ].size();

  secondary_events_buffer_[ t ].resize( old_size + e.size() );
  std::vector< uint_t >::iterator it = secondary_events_buffer_[ t ].begin() + old_size;
  e >> it;
}

inline void
Scheduler::send_offgrid_remote( thread t, SpikeEvent& e, const delay lag )
{
  // Put the spike in a buffer for the remote machines
  OffGridSpike ogs( e.get_sender().get_gid(), e.get_offset() );
  for ( int_t i = 0; i < e.get_multiplicity(); ++i )
    offgrid_spike_register_[ t ][ lag ].push_back( ogs );
}

inline delay
Scheduler::get_modulo( delay d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time
  // when all evens due are read out.
  assert( static_cast< vector< delay >::size_type >( d ) < moduli_.size() );

  return moduli_[ d ];
}

inline delay
Scheduler::get_slice_modulo( delay d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time
  // when all evens due are read out.
  assert( static_cast< vector< delay >::size_type >( d ) < slice_moduli_.size() );

  return slice_moduli_[ d ];
}

inline delay
Scheduler::get_min_delay()
{
  return min_delay_;
}

inline delay
Scheduler::get_max_delay()
{
  return max_delay_;
}

inline double_t
Scheduler::get_prelim_tol()
{
  return prelim_tol;
}

inline size_t
Scheduler::get_prelim_interpolation_order()
{
  return prelim_interpolation_order;
}

inline size_t
Scheduler::get_slice() const
{
  return slice_;
}

inline void
Scheduler::terminate()
{
  terminate_ = true;
}
}

#endif // SCHEDULER_H
