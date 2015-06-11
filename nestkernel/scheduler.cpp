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
#include "gslrandomgen.h"
#include "stopwatch.h"

#include "nest_timemodifier.h"
#include "nest_timeconverter.h"

#ifdef N_DEBUG
#undef N_DEBUG
#endif

#ifdef USE_PMA
#ifdef IS_K
extern PaddedPMA poormansallocpool[];
#else
extern PoorMansAllocator poormansallocpool;
#ifdef _OPENMP
#pragma omp threadprivate( poormansallocpool )
#endif
#endif
#endif

extern int SLIsignalflag;

nest::Network* nest::Scheduler::net_ = 0;

std::vector< nest::delay > nest::Scheduler::moduli_;
std::vector< nest::delay > nest::Scheduler::slice_moduli_;

nest::delay nest::Scheduler::max_delay_ = 1;
nest::delay nest::Scheduler::min_delay_ = 1;

const nest::delay nest::Scheduler::comm_marker_ = 0;

#define DRY_RUN_SUBNET_MODE

bool nest::Scheduler::dry_run_enabled_ = false;
// arbitrarily guessed value for threshold for now
const double_t nest::Scheduler::dry_run_num_spikes_per_h_threshold_ = 4.0;

// BEGIN: DEBUGGING HACK BY WS
#ifdef COUNT_FACILITATE
size_t nest::facilitate_counter = 0;
#endif
// END: DEBUGGING HACK BY WS

nest::Scheduler::Scheduler( Network& net )
  : initialized_( false )
  , simulating_( false )
  , force_singlethreading_( false )
  , n_threads_( 1 )
  , n_rec_procs_( 0 )
  , entry_counter_( 0 )
  , exit_counter_( 0 )
  , nodes_vec_( n_threads_ )
  , nodes_vec_network_size_( 0 )
  , // zero to force update
  clock_( Time::tic( 0L ) )
  , slice_( 0L )
  , to_do_( 0L )
  , to_do_total_( 0L )
  , from_step_( 0L )
  , to_step_( 0L )
  , // consistent with to_do_ == 0
  terminate_( false )
  , off_grid_spiking_( false )
  , print_time_( false )
  , dry_run_target_rate_( -1.0 )
  , dry_run_only_relevant_spikes_( false )
  , dry_run_num_spikes_per_h_( 1.0 )
  , dry_run_indirect_access_enabled_( false )
  , gather_walltime_( 0.0 )
  , collocate_buffers_walltime_( 0.0 )
  , local_spike_counter_( 0 )
  , dry_run_irrelevant_spikes_counter_( 0 )
  , rng_()
{
  net_ = &net;
  init_();
}

nest::Scheduler::~Scheduler()
{
  finalize_();
}

void
nest::Scheduler::reset()
{
  // Reset TICS_PER_MS, MS_PER_TICS and TICS_PER_STEP to the compiled in default values.
  // See ticket #217 for details.
  nest::TimeModifier::reset_to_defaults();

  reset_profiling_counters_timers_();

  clock_.set_to_zero(); // ensures consistent state
  to_do_ = 0;
  slice_ = 0;
  from_step_ = 0;
  to_step_ = 0; // consistent with to_do_ = 0
  finalize_();
  init_();
}

void
nest::Scheduler::reset_profiling_counters_timers_()
{
  // reset various counters and timers used for profiling purposes,
  //     especially in dry-run mode

  // reset timer with cumulated runtime of gather_events_(..) or
  //     gather_events_for_dry_run_(..)
  gather_walltime_ = 0.0;
  // reset also timer for collocate_buffers_(..) within gather_events_(..)
  collocate_buffers_walltime_ = 0.0;
  // reset spike counters
  local_spike_counter_ = 0;
  dry_run_irrelevant_spikes_counter_ = 0;
// BEGIN: DEBUGGING HACK BY WS
#ifdef COUNT_FACILITATE
#pragma omp atomic write
  // reset counter for STDP facilitations
  facilitate_counter = 0;
#endif
  // END: DEBUGGING HACK BY WS
}

void
nest::Scheduler::clear_pending_spikes()
{
  configure_spike_buffers_();
}

void
nest::Scheduler::init_()
{
  assert( initialized_ == false );

  simulated_ = false;

  // The following line is executed by all processes, no need to communicate
  // this change in delays.
  min_delay_ = max_delay_ = 1;

#ifndef _OPENMP
  if ( n_threads_ > 1 )
  {
    net_->message( SLIInterpreter::M_ERROR,
      "Scheduler::reset",
      "No multithreading available, using single threading" );
    n_threads_ = 1;
    force_singlethreading_ = true;
  }
#endif

  set_num_threads( n_threads_ );

  n_sim_procs_ = Communicator::get_num_processes() - n_rec_procs_;

  // explicitly force construction of nodes_vec_ to ensure consistent state
  update_nodes_vec_();

  create_rngs_( true ); // flag that this is a call from the ctr
  create_grng_( true ); // flag that this is a call from the ctr

  initialized_ = true;
}

void
nest::Scheduler::finalize_()
{
  // clear the buffers
  local_grid_spikes_.clear();
  global_grid_spikes_.clear();
  local_offgrid_spikes_.clear();
  global_offgrid_spikes_.clear();

  initialized_ = false;
}

void
nest::Scheduler::init_moduli_()
{
  assert( min_delay_ != 0 );
  assert( max_delay_ != 0 );

  /*
   * Ring buffers use modulos to determine where to store incoming events
   * with given time stamps, relative to the beginning of the slice in which
   * the spikes are delivered from the queue, ie, the slice after the one
   * in which they were generated. The pertaining offsets are 0..max_delay-1.
   */

  moduli_.resize( min_delay_ + max_delay_ );

  for ( delay d = 0; d < min_delay_ + max_delay_; ++d )
    moduli_[ d ] = ( clock_.get_steps() + d ) % ( min_delay_ + max_delay_ );

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast< size_t >(
    std::ceil( static_cast< double >( min_delay_ + max_delay_ ) / min_delay_ ) );
  slice_moduli_.resize( min_delay_ + max_delay_ );
  for ( delay d = 0; d < min_delay_ + max_delay_; ++d )
    slice_moduli_[ d ] = ( ( clock_.get_steps() + d ) / min_delay_ ) % nbuff;
}

/**
 * This function is called after all nodes have been updated.
 * We can compute the value of (T+d) mod max_delay without explicit
 * reference to the network clock, because compute_moduli_ is
 * called whenever the network clock advances.
 * The various modulos for all available delays are stored in
 * a lookup-table and this table is rotated once per time slice.
 */
void
nest::Scheduler::compute_moduli_()
{
  assert( min_delay_ != 0 );
  assert( max_delay_ != 0 );

  /*
   * Note that for updating the modulos, it is sufficient
   * to rotate the buffer to the left.
   */
  assert( moduli_.size() == ( index )( min_delay_ + max_delay_ ) );
  std::rotate( moduli_.begin(), moduli_.begin() + min_delay_, moduli_.end() );

  /* For the slice-based ring buffer, we cannot rotate the table, but
   have to re-compute it, since max_delay_ may not be a multiple of
   min_delay_.  Reference time is the time at the beginning of the slice.
   */
  const size_t nbuff = static_cast< size_t >(
    std::ceil( static_cast< double >( min_delay_ + max_delay_ ) / min_delay_ ) );
  for ( delay d = 0; d < min_delay_ + max_delay_; ++d )
    slice_moduli_[ d ] = ( ( clock_.get_steps() + d ) / min_delay_ ) % nbuff;
}

void
nest::Scheduler::update_delay_extrema_()
{
  min_delay_ = net_->connection_manager_.get_min_delay().get_steps();
  max_delay_ = net_->connection_manager_.get_max_delay().get_steps();

  // don't do this in dry-run mode because there are no communication
  // partners; keep local min_delay and max_delay values instead
  if ( Communicator::get_num_processes() > 1 && !dry_run_enabled_ )
  {
    std::vector< delay > min_delays( Communicator::get_num_processes() );
    min_delays[ Communicator::get_rank() ] = min_delay_;
    Communicator::communicate( min_delays );
    min_delay_ = *std::min_element( min_delays.begin(), min_delays.end() );

    std::vector< delay > max_delays( Communicator::get_num_processes() );
    max_delays[ Communicator::get_rank() ] = max_delay_;
    Communicator::communicate( max_delays );
    max_delay_ = *std::max_element( max_delays.begin(), max_delays.end() );
  }

  if ( min_delay_ == Time::pos_inf().get_steps() )
    min_delay_ = Time::get_resolution().get_steps();
}

void
nest::Scheduler::configure_spike_buffers_()
{
  assert( min_delay_ != 0 );

  spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  spike_register_.resize( n_threads_, std::vector< std::vector< uint_t > >( min_delay_ ) );
  for ( size_t j = 0; j < spike_register_.size(); ++j )
    for ( size_t k = 0; k < spike_register_[ j ].size(); ++k )
      spike_register_[ j ][ k ].clear();

  offgrid_spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  offgrid_spike_register_.resize(
    n_threads_, std::vector< std::vector< OffGridSpike > >( min_delay_ ) );
  for ( size_t j = 0; j < offgrid_spike_register_.size(); ++j )
    for ( size_t k = 0; k < offgrid_spike_register_[ j ].size(); ++k )
      offgrid_spike_register_[ j ][ k ].clear();

  int send_buffer_size, recv_buffer_size;

  // variable solely used for dry-run mode
  double_t num_spikes_per_h = 0;

  if ( dry_run_enabled_ && ( dry_run_target_rate_ > 0.0 ) )
  // version of dry-run mode with prefilled spike buffer
  // and fixed target spike rate ("static mode")
  {
    // if dry-run mode is enabled, the send buffer size is calculated in a different way
    // than in standard mode

    // num_spikes_per_h holds the average number of spikes generated in a VP in a single
    // simulation step;
    // remark on the calculation of num_spikes_per_h:
    //   - (net_->size()-1) to exclude root node from calculation because we want do divide
    //     by the number of spike-generating neurons (however, we cannot account for subnets
    //     and devices)
    num_spikes_per_h = ( net_->size() - 1 ) * dry_run_target_rate_
      * ( Time::get_resolution().get_ms() / 1000.0 ) / Communicator::get_num_virtual_processes();

    // calculation of send buffer size for dry-run mode;
    // "num_spikes_per_h+2" because of comm. markers (+1) and to have some extra space
    // to account for stochastic fluctuations (+1);
    // a minimum size of 2 is also guaranteed (see regular computation of send_buffer_size)
    send_buffer_size = ceil( ( num_spikes_per_h + 2 ) * min_delay_ * n_threads_ );

    // this part is (mainly) repeated from configure_spike_buffers_(..)
    recv_buffer_size = send_buffer_size * Communicator::get_num_processes();
  }
  else
  {
    // send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces.
    send_buffer_size = n_threads_ * min_delay_ > 2 ? n_threads_ * min_delay_ : 2;
    recv_buffer_size = send_buffer_size * Communicator::get_num_processes();
  }

  Communicator::set_buffer_sizes( send_buffer_size, recv_buffer_size );

  // DEC cxx required 0U literal, HEP 2007-03-26
  // --> replaced 0U by comm_marker_ for dry-run mode, hope this does not hurt
  //     now, nearly eight years later (WS 2015-01-23)
  local_grid_spikes_.clear();
  local_grid_spikes_.resize( send_buffer_size, comm_marker_ );
  local_offgrid_spikes_.clear();
  local_offgrid_spikes_.resize( send_buffer_size, OffGridSpike( 0, 0.0 ) );
  global_grid_spikes_.clear();
  global_grid_spikes_.resize( recv_buffer_size, comm_marker_ );
  global_offgrid_spikes_.clear();
  global_offgrid_spikes_.resize( recv_buffer_size, OffGridSpike( 0, 0.0 ) );

  displacements_.clear();
  displacements_.resize( Communicator::get_num_processes(), 0 );

  if ( dry_run_enabled_ )
  {
    if ( dry_run_target_rate_ > 0.0 )
    {
      fill_spike_buffers_for_static_dry_run_(
        num_spikes_per_h, send_buffer_size, recv_buffer_size );
    }
    else
    {
      net_->message( SLIInterpreter::M_INFO,
        "Scheduler::configure_spike_buffers_",
        "Dynamic dry-run mode enabled." );
    }

    if ( dry_run_only_relevant_spikes_ )
    {
      // set number of thread-specific global spike buffers
      global_grid_spikes_thread_specific_.resize( n_threads_ );
      for ( index tid = 0; tid < n_threads_; ++tid )
      {
        global_grid_spikes_thread_specific_[ tid ] = global_grid_spikes_;
      }
    }
  }
}

inline nest::uint_t
nest::Scheduler::createNewFakeGID_SubnetMode_( nest::uint_t old_gid,
  uint_t vp_num,
  librandom::RngPtr rng )
{
  static const int max_correction_cycles = 5;

  uint_t new_gid;

  Subnet* old_gid_parent_subnet = net_->get_node( old_gid, 0 )->get_parent();
  if ( !old_gid_parent_subnet )
  {
    // fallback solution, just a hack for now
    // TODO: proper solution
    new_gid = old_gid;
    printf( "Fallback 1 required!\n" );
  }
  else
  {
    const Multirange& alt_gid_list = old_gid_parent_subnet->get_gids();
    index num_alt_gids = alt_gid_list.size();
    uint_t new_gid_corrected;

    int count = 0;
    while ( true )
    {
      // draw random gid from subnet
      new_gid = alt_gid_list[ rng->ulrand( num_alt_gids - 1 ) ];
      // correct for requested VP assignment by making a good guess
      new_gid_corrected =
        new_gid - ( new_gid % Communicator::get_num_virtual_processes() ) + vp_num;
      // increase cycle counter (for fallback exit)
      count++;
      // check if the new corrected gid is still within subnet
      // if( net_->is_local_gid( new_gid ) )
      // if( new_gid % Communicator::get_num_virtual_processes() != vp_num )
      if ( !alt_gid_list.contains( new_gid_corrected ) )
      {
        // first attempt at heuristic correction
        if ( new_gid_corrected > Communicator::get_num_virtual_processes() )
        {
          new_gid_corrected = new_gid_corrected - Communicator::get_num_virtual_processes();
          printf( "CORRECTION [%02i]: %lu --> %lu (%lu, %u)\n",
            count,
            old_gid,
            new_gid_corrected,
            num_alt_gids,
            vp_num );
        }
        // check again
        if ( !alt_gid_list.contains( new_gid_corrected ) )
        {
          // not within subnet: fallback exit or continue loop
          printf( "REJECTED   [%02i]: %lu --> %lu (%lu, %u)\n",
            count,
            old_gid,
            new_gid_corrected,
            num_alt_gids,
            vp_num );
          // fallback exit to avoid infinite loop in case there actually does not
          // exist any gid of this subnet on the requested VP
          if ( count >= max_correction_cycles )
          {
            new_gid_corrected = new_gid;
            break;
          }
        }
        else
        {
          // within subnet: regular exit
          // printf( ">> FOUND   [%02i]: %lu --> %lu (%lu, %u)\n", count, old_gid,
          // new_gid_corrected, num_alt_gids, vp_num );
          break;
        }
      }
      else
      {
        // within subnet: regular exit
        // printf( ">> FOUND   [%02i]: %lu --> %lu (%lu, %u)\n", count, old_gid, new_gid_corrected,
        // num_alt_gids, vp_num );
        break;
      }
    }
    // only variable new_gid is used below, therefore assignment necessary
    new_gid = new_gid_corrected;
  }

  // prevent fake spike events from being equal to comm_marker_
  if ( new_gid == comm_marker_ )
  {
    // implicit assumption: comm_marker_ = 0
    new_gid = rng->ulrand( net_->size() - 1 ) + 1;
    printf( "Fallback 2 required!\n" );
  }

  return new_gid;
}

inline nest::uint_t
nest::Scheduler::createNewFakeGID_( nest::uint_t old_gid, librandom::RngPtr rng )
{
  uint_t new_gid;

  // magic formula which guarantees that we only generate spike events
  // with GIDs from the respective VP (however, because we don't check
  // for subnets and extra devices, this is not perfect; nevertheless, it should
  // at least lead to similar memory access patterns within deliver_events_(..))
  new_gid = ( old_gid
              + rng->ulrand( ( net_->size() - 1 ) / Communicator::get_num_virtual_processes() + 1 )
                * Communicator::get_num_virtual_processes() ) % net_->size();

  // prevent fake spike events from being equal to comm_marker_
  if ( new_gid == comm_marker_ )
  {
    // implicit assumption: comm_marker_ = 0
    new_gid = rng->ulrand( net_->size() - 1 ) + 1;
  }

  return new_gid;
}

void
nest::Scheduler::fill_spike_buffers_for_static_dry_run_( double_t num_spikes_per_h,
  int send_buffer_size,
  int recv_buffer_size )
{
  // in dry-run mode, we need a prefilled buffer of spike events, following a
  // similar pattern as in a real simulation; this prefilling is carried out in
  // the following...

  std::string msg = String::compose(
    "Entering prefilling of fake spike buffer... (num_spikes_per_h = %1)", num_spikes_per_h );
  net_->message( SLIInterpreter::M_INFO, "Scheduler::fill_spike_buffers_for_static_dry_run_", msg );

  // the displacements are perfectly regular and pre-set at this point
  for ( int pid = 0; pid < Communicator::get_num_processes(); ++pid )
  {
    displacements_[ pid ] = send_buffer_size * pid;
  }

  // get a local random generator to generate the spike events and
  // the number of events per simulation step
  librandom::RngPtr rng = get_rng( 0 );

  // fill global spike buffer as expected by deliver_events_(..)
  uint64_t num_spikes_sum = 0;
  // outermost loop: processes
  for ( int pid = 0; pid < Communicator::get_num_processes(); ++pid )
  {
    int count, pos;
    uint64_t num_spikes;

    bool stop_loop = false;
    while ( !stop_loop )
    // the following steps might have to be repeated because of their stochastic nature;
    // we never want to exceed the available space in the global spike buffer for each
    // specific process p
    {
      count = 0;
      num_spikes = 0;
      pos = displacements_[ pid ]; // displacements_ filled before

      // fill the process-related region within the global spike buffer
      // with comm. markers (local_grid_spikes_ is filled with them at this point,
      // see above) as initialization (this is important if the while loop runs for
      // more than one iteration)
      std::copy(
        local_grid_spikes_.begin(), local_grid_spikes_.end(), global_grid_spikes_.begin() + pos );

      // outer loop: threads
      for ( index tid = 0; tid < n_threads_; ++tid )
      {
        // VP counter, computed for current pid and tid
        index vp_num = pid + tid * Communicator::get_num_processes();

        // middle loop: time steps within slice
        for ( nest::delay h = 0; h < min_delay_; ++h )
        {
          // determine how many spike events are generated within this time step
          // based on the "expected value" num_spikes_per_h (either floor or ceil)
          uint64_t num_spikes_per_h_discrete;
          double_t rand_val = rng->drand();
          if ( rand_val <= ( num_spikes_per_h - floor( num_spikes_per_h ) ) )
          {
            num_spikes_per_h_discrete = ceil( num_spikes_per_h );
          }
          else
          {
            num_spikes_per_h_discrete = floor( num_spikes_per_h );
          }
          // inner loop: spike events within single time step for single VP
          for ( index n = 0; n < num_spikes_per_h_discrete; ++n )
          {
            if ( pos < recv_buffer_size ) // avoid buffer overflow
            {
              global_grid_spikes_[ pos ] = createNewFakeGID_( vp_num, rng );
              pos++;
            }
            count++;
            num_spikes++;
          }
          if ( pos < recv_buffer_size ) // avoid buffer overflow
          {
            global_grid_spikes_[ pos++ ] = comm_marker_;
          }
          count++;
        }
      }
      // if we are within the limits of available buffer space for the current process p,
      // we will continue with the next process (outside of this while loop)
      stop_loop = ( count <= send_buffer_size );
      if ( !stop_loop )
      {
        msg = String::compose(
          "Repetition of spike filling for process %1 necessary! (num_spikes = %2)",
          pid,
          num_spikes );
        net_->message(
          SLIInterpreter::M_WARNING, "Scheduler::fill_spike_buffers_for_static_dry_run_", msg );
      }
    }
    num_spikes_sum += num_spikes;
  }
  // notify the user about effective target firing rate
  msg = String::compose( "Target firing rate is set to %1 but requested rate was %2!",
    num_spikes_sum / min_delay_
      / ( ( net_->size() - 1 ) * ( Time::get_resolution().get_ms() / 1000.0 ) ),
    dry_run_target_rate_ );
  net_->message(
    SLIInterpreter::M_WARNING, "Scheduler::fill_spike_buffers_for_static_dry_run_", msg );

  // store spikes per h-step in class variable
  dry_run_num_spikes_per_h_ = num_spikes_per_h;
  // set flag for indirect access to global spike buffer
  dry_run_indirect_access_enabled_ =
    ( dry_run_num_spikes_per_h_ < dry_run_num_spikes_per_h_threshold_ );

  // for the sparse case: fill the buffer which holds the indices within the global
  // spike buffer at which actually spike events can be found
  if ( dry_run_indirect_access_enabled_ )
  {
    net_->message( SLIInterpreter::M_INFO,
      "Scheduler::fill_spike_buffers_for_static_dry_run_",
      "Preparing indirect access via dry_run_valid_spike_pos_vec_..." );
    dry_run_valid_spike_pos_vec_.resize( num_spikes_sum );
    uint_t count = 0;
    for ( u_int u = 0; u < static_cast< uint_t >( recv_buffer_size ); u++ )
    {
      if ( global_grid_spikes_[ u ] != comm_marker_ )
      {
        dry_run_valid_spike_pos_vec_[ count++ ] = u;
      }
    }
  }
  else
  {
    net_->message( SLIInterpreter::M_INFO,
      "Scheduler::fill_spike_buffers_for_static_dry_run_",
      "Direct access to global_grid_spikes_..." );
  }

  net_->message( SLIInterpreter::M_INFO,
    "Scheduler::fill_spike_buffers_for_static_dry_run_",
    "Prefilling of fake spike buffer finished." );
}

void
nest::Scheduler::simulate( Time const& t )
{
  assert( initialized_ );

  t_real_ = 0;
  t_slice_begin_ = timeval();
  t_slice_end_ = timeval();

  if ( t == Time::ms( 0.0 ) )
    return;

  if ( t < Time::step( 1 ) )
  {
    net_->message( SLIInterpreter::M_ERROR,
      "Scheduler::simulate",
      String::compose( "Simulation time must be >= %1 ms (one time step).",
                     Time::get_resolution().get_ms() ) );
    throw KernelException();
  }

  if ( t.is_finite() )
  {
    Time time1 = clock_ + t;
    if ( !time1.is_finite() )
    {
      std::string msg = String::compose(
        "A clock overflow will occur after %1 of %2 ms. Please reset network "
        "clock first!",
        ( Time::max() - clock_ ).get_ms(),
        t.get_ms() );
      net_->message( SLIInterpreter::M_ERROR, "Scheduler::simulate", msg );
      throw KernelException();
    }
  }
  else
  {
    std::string msg = String::compose(
      "The requested simulation time exceeds the largest time NEST can handle "
      "(T_max = %1 ms). Please use a shorter time!",
      Time::max().get_ms() );
    net_->message( SLIInterpreter::M_ERROR, "Scheduler::simulate", msg );
    throw KernelException();
  }

  to_do_ += t.get_steps();
  to_do_total_ = to_do_;

  prepare_simulation();

  // from_step_ is not touched here.  If we are at the beginning
  // of a simulation, it has been reset properly elsewhere.  If
  // a simulation was ended and is now continued, from_step_ will
  // have the proper value.  to_step_ is set as in advance_time().

  delay end_sim = from_step_ + to_do_;
  if ( min_delay_ < end_sim )
    to_step_ = min_delay_; // update to end of time slice
  else
    to_step_ = end_sim; // update to end of simulation time

  // Warn about possible inconsistencies, see #504.
  // This test cannot come any earlier, because we first need to compute min_delay_
  // above.
  if ( t.get_steps() % min_delay_ != 0 )
    net_->message( SLIInterpreter::M_WARNING,
      "Scheduler::simulate",
      "The requested simulation time is not an integer multiple of the minimal delay in the "
      "network. "
      "This may result in inconsistent results under the following conditions: (i) A network "
      "contains "
      "more than one source of randomness, e.g., two different poisson_generators, and (ii) "
      "Simulate "
      "is called repeatedly with simulation times that are not multiples of the minimal delay." );

  resume();

  finalize_simulation();
}

void
nest::Scheduler::prepare_simulation()
{
  if ( to_do_ == 0 )
    return;

  reset_profiling_counters_timers_();

  // find shortest and longest delay across all MPI processes
  // this call sets the member variables
  update_delay_extrema_();

  // Check for synchronicity of global rngs over processes only in case
  // of parallel MPI processes; if the parallel MPI processes are only fake processes
  // because NEST is executed in dry-run mode, skip the check;
  // we need to do the check ahead of any simulation in case random numbers
  // have been consumed on the SLI level.
  if ( Communicator::get_num_processes() > 1 && !dry_run_enabled_ )
  {
    if ( !Communicator::grng_synchrony( grng_->ulrand( 100000 ) ) )
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::simulate",
        "Global Random Number Generators are not synchronized prior to simulation." );
      throw KernelException();
    }
  }

  // if at the beginning of a simulation, set up spike buffers
  if ( !simulated_ )
    configure_spike_buffers_();

  update_nodes_vec_();
  prepare_nodes();

#ifdef HAVE_MUSIC
  // we have to do enter_runtime after prepre_nodes, since we use
  // calibrate to map the ports of MUSIC devices, which has to be done
  // before enter_runtime
  if ( !simulated_ ) // only enter the runtime mode once
  {
    net_->publish_music_in_ports_();

    double tick = Time::get_resolution().get_ms() * min_delay_;
    std::string msg = String::compose( "Entering MUSIC runtime with tick = %1 ms", tick );
    net_->message( SLIInterpreter::M_INFO, "Scheduler::resume", msg );
    Communicator::enter_runtime( tick );
  }
#endif
}

void
nest::Scheduler::finalize_simulation()
{
  if ( not simulated_ )
    return;

  // Check for synchronicity of global rngs over processes only in case
  // of parallel MPI processes; if the parallel MPI processes are only fake processes
  // because NEST is executed in dry-run mode, skip the check
  if ( Communicator::get_num_processes() > 1 && !dry_run_enabled_ )
  {
    if ( !Communicator::grng_synchrony( grng_->ulrand( 100000 ) ) )
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::simulate",
        "Global Random Number Generators are not synchronized after simulation." );
      throw KernelException();
    }
  }

  finalize_nodes();
}


void
nest::Scheduler::resume()
{
  assert( initialized_ );

  terminate_ = false;

  if ( to_do_ == 0 )
    return;

  if ( print_time_ )
  {
    std::cout << std::endl;
    print_progress_();
  }

  simulating_ = true;
  simulated_ = true;

#ifndef _OPENMP
  if ( n_threads_ > 1 )
  {
    net_->message( SLIInterpreter::M_ERROR,
      "Scheduler::resume",
      "No multithreading available, using single threading" );
  }
#endif

  update();

  simulating_ = false;

  if ( print_time_ )
    std::cout << std::endl;

  Communicator::synchronize();

  if ( terminate_ )
  {
    net_->message(
      SLIInterpreter::M_ERROR, "Scheduler::resume", "Exiting on error or user signal." );
    net_->message( SLIInterpreter::M_ERROR,
      "Scheduler::resume",
      "Scheduler: Use 'ResumeSimulation' to resume." );

    if ( SLIsignalflag != 0 )
    {
      SystemSignal signal( SLIsignalflag );
      SLIsignalflag = 0;
      throw signal;
    }
    else
      throw SimulationError();
  }

  net_->message( SLIInterpreter::M_INFO, "Scheduler::resume", "Simulation finished." );
}

void
nest::Scheduler::update()
{
#ifdef _OPENMP
  net_->message( SLIInterpreter::M_INFO, "Scheduler::update", "Simulating using OpenMP." );
#endif

  std::vector< lockPTR< WrappedThreadException > > exceptions_raised( net_->get_num_threads() );
// parallel section begins
#pragma omp parallel
  {
    std::vector< Node* >::iterator i;
    int t = net_->get_thread_id(); // which thread am I

    index num_t = 1;
#ifdef _OPENMP
    num_t = static_cast< index >( omp_get_num_threads() ); // how many threads there are
    if ( ( num_t != n_threads_ ) && ( t == 0 ) )
    {
      std::string msg =
        String::compose( "Setting number of OpenMP threads failed! (num_t = %1)", num_t );
      net_->message( SLIInterpreter::M_ERROR, "Scheduler::update", msg );
    }
#endif

    do
    {
      if ( print_time_ )
        gettimeofday( &t_slice_begin_, NULL );

      if ( from_step_ == 0 ) // deliver only at beginning of slice
      {
        deliver_events_( t );
#ifdef HAVE_MUSIC
// advance the time of music by one step (min_delay * h) must
// be done after deliver_events_() since it calls
// music_event_out_proxy::handle(), which hands the spikes over to
// MUSIC *before* MUSIC time is advanced

// wait until all threads are done -> synchronize
#pragma omp barrier
// the following block is executed by the master thread only
// the other threads are enforced to wait at the end of the block
#pragma omp master
        {
          // advance the time of music by one step (min_delay * h) must
          // be done after deliver_events_() since it calls
          // music_event_out_proxy::handle(), which hands the spikes over to
          // MUSIC *before* MUSIC time is advanced
          if ( slice_ > 0 )
            Communicator::advance_music_time( 1 );

          // the following could be made thread-safe
          net_->update_music_event_handlers_( clock_, from_step_, to_step_ );
        }
// end of master section, all threads have to synchronize at this point
#pragma omp barrier
#endif
      }

      for ( i = nodes_vec_[ t ].begin(); i != nodes_vec_[ t ].end(); ++i )
      {
        // We update in a parallel region. Therefore, we need to catch exceptions
        // here and then handle them after the parallel region.
        try
        {
          if ( not( *i )->is_frozen() )
            ( *i )->update( clock_, from_step_, to_step_ );
        }
        catch ( std::exception& e )
        {
          // so throw the exception after parallel region
          exceptions_raised.at( t ) =
            lockPTR< WrappedThreadException >( new WrappedThreadException( e ) );
          terminate_ = true;
        }
      }

      if ( dry_run_enabled_ )
      { // in dry-run mode, gather events in parallel section after barrier
#pragma omp barrier
        if ( to_step_ == min_delay_ ) // gather only at end of slice
          gather_events_for_dry_run_( static_cast< uint_t >( t ), static_cast< uint_t >( num_t ) );
      }

// parallel section ends, wait until all threads are done -> synchronize
#pragma omp barrier

// the following block is executed by the master thread only
// the other threads are enforced to wait at the end of the block
#pragma omp master
      {
        if ( !dry_run_enabled_ )
        {                               // in real simulation, gather events in single thread
          if ( to_step_ == min_delay_ ) // gather only at end of slice
            gather_events_();
        }

        advance_time_();

        if ( SLIsignalflag != 0 )
        {
          net_->message(
            SLIInterpreter::M_INFO, "Scheduler::update", "Simulation exiting on user signal." );
          terminate_ = true;
        }

        if ( print_time_ )
        {
          gettimeofday( &t_slice_end_, NULL );
          print_progress_();
        }
      }
// end of master section, all threads have to synchronize at this point
#pragma omp barrier

    } while ( ( to_do_ != 0 ) && ( !terminate_ ) );

  } // end of #pragma parallel omp
  // check if any exceptions have been raised
  for ( thread thr = 0; thr < net_->get_num_threads(); ++thr )
    if ( exceptions_raised.at( thr ).valid() )
      throw WrappedThreadException( *( exceptions_raised.at( thr ) ) );
}

void
nest::Scheduler::prepare_nodes()
{
  assert( initialized_ );

  init_moduli_();

  net_->message(
    SLIInterpreter::M_INFO, "Scheduler::prepare_nodes", "Please wait. Preparing elements." );

  /* We initialize the buffers of each node and calibrate it. */

  size_t num_active_nodes = 0; // counts nodes that will be updated

  std::vector< lockPTR< WrappedThreadException > > exceptions_raised( net_->get_num_threads() );

#ifdef _OPENMP
#pragma omp parallel reduction( + : num_active_nodes )
  {
    size_t t = net_->get_thread_id();
#else
  for ( index t = 0; t < n_threads_; ++t )
  {
#endif

    // We prepare nodes in a parallel region. Therefore, we need to catch exceptions
    // here and then handle them after the parallel region.
    try
    {
      for ( std::vector< Node* >::iterator it = nodes_vec_[ t ].begin();
            it != nodes_vec_[ t ].end();
            ++it )
      {
        prepare_node_( *it );
        if ( not( *it )->is_frozen() )
          ++num_active_nodes;
      }
    }
    catch ( std::exception& e )
    {
      // so throw the exception after parallel region
      exceptions_raised.at( t ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( e ) );
      terminate_ = true;
    }

  } // end of parallel section / end of for threads

  // check if any exceptions have been raised
  for ( thread thr = 0; thr < net_->get_num_threads(); ++thr )
    if ( exceptions_raised.at( thr ).valid() )
      throw WrappedThreadException( *( exceptions_raised.at( thr ) ) );

  net_->message(
    SLIInterpreter::M_INFO,
    "Scheduler::prepare_nodes",
    String::compose(
      "Simulating %1 local node%2.", num_active_nodes, num_active_nodes == 1 ? "" : "s" ) );
}

void
nest::Scheduler::update_nodes_vec_()
{
  // Check if the network size changed, in order to not enter
  // the critical region if it is not necessary. Note that this
  // test also covers that case that nodes have been deleted
  // by reset.
  if ( net_->size() == nodes_vec_network_size_ )
    return;

#ifdef _OPENMP
#pragma omp critical
  {
// This code may be called from a thread-parallel context, when it is
// invoked by TargetIdentifierIndex::set_target() during parallel
// wiring. Nested OpenMP parallelism is problematic, therefore, we
// enforce single threading here. This should be unproblematic wrt
// performance, because the nodes_vec_ is rebuilt only once after
// changes in network size.
#endif

    // Check again, if the network size changed, since a previous thread
    // can have updated nodes_vec_ before.
    if ( net_->size() != nodes_vec_network_size_ )
    {

      /* We clear the existing nodes_vec_ and then rebuild it. */
      assert( nodes_vec_.size() == n_threads_ );

      for ( index t = 0; t < n_threads_; ++t )
      {
        nodes_vec_[ t ].clear();

        // Loops below run from index 1, because index 0 is always the root network,
        // which is never updated.
        size_t num_thread_local_nodes = 0;
        for ( size_t idx = 1; idx < net_->local_nodes_.size(); ++idx )
        {
          Node* node = net_->local_nodes_.get_node_by_index( idx );
          if ( !node->is_subnet() && ( static_cast< index >( node->get_thread() ) == t
                                       || node->num_thread_siblings_() > 0 ) )
            num_thread_local_nodes++;
        }
        nodes_vec_[ t ].reserve( num_thread_local_nodes );

        for ( size_t idx = 1; idx < net_->local_nodes_.size(); ++idx )
        {
          Node* node = net_->local_nodes_.get_node_by_index( idx );

          // Subnets are never updated and therefore not included.
          if ( node->is_subnet() )
            continue;

          // If a node has thread siblings, it is a sibling container, and we
          // need to add the replica for the current thread. Otherwise, we have
          // a normal node, which is added only on the thread it belongs to.
          if ( node->num_thread_siblings_() > 0 )
          {
            node->get_thread_sibling_( t )->set_thread_lid( nodes_vec_[ t ].size() );
            nodes_vec_[ t ].push_back( node->get_thread_sibling_( t ) );
          }
          else if ( static_cast< index >( node->get_thread() ) == t )
          {
            // these nodes cannot be subnets
            node->set_thread_lid( nodes_vec_[ t ].size() );
            nodes_vec_[ t ].push_back( node );
          }
        }
      } // end of for threads

      nodes_vec_network_size_ = net_->size();
    }
#ifdef _OPENMP
  } // end of omp critical region
#endif
}

//!< This function is called only if the thread data structures are properly set up.
void
nest::Scheduler::finalize_nodes()
{
#ifdef _OPENMP
  net_->message( SLIInterpreter::M_INFO, "Scheduler::finalize_nodes()", " using OpenMP." );
// parallel section begins
#pragma omp parallel
  {
    index t = net_->get_thread_id(); // which thread am I
#else
  for ( index t = 0; t < n_threads_; ++t )
  {
#endif
    for ( size_t idx = 0; idx < net_->local_nodes_.size(); ++idx )
    {
      Node* node = net_->local_nodes_.get_node_by_index( idx );
      if ( node != 0 )
      {
        if ( node->num_thread_siblings_() > 0 )
          node->get_thread_sibling_( t )->finalize();
        else
        {
          if ( static_cast< index >( node->get_thread() ) == t )
            node->finalize();
        }
      }
    }
  }
}


void
nest::Scheduler::set_status( DictionaryDatum const& d )
{
  assert( initialized_ );

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
  if ( updateValue< double_t >( d, "time", time ) )
  {
    if ( time != 0.0 )
      throw BadProperty( "The simulation time can only be set to 0.0." );

    if ( clock_ > TimeZero )
    {
      // reset only if time has passed
      net_->message( SLIInterpreter::M_WARNING,
        "Scheduler::set_status",
        "Simulation time reset to t=0.0. Resetting the simulation time is not "
        "fully supported in NEST at present. Some spikes may be lost, and "
        "stimulating devices may behave unexpectedly. PLEASE REVIEW YOUR "
        "SIMULATION OUTPUT CAREFULLY!" );

      clock_ = Time::step( 0 );
      from_step_ = 0;
      slice_ = 0;
      configure_spike_buffers_(); // clear all old spikes
    }
  }

  updateValue< bool >( d, "print_time", print_time_ );

  long n_threads;
  bool n_threads_updated = updateValue< long >( d, "local_num_threads", n_threads );
  if ( n_threads_updated )
  {
    if ( net_->size() > 1 )
      throw KernelException( "Nodes exist: Thread/process number cannot be changed." );
    if ( net_->models_.size() > net_->pristine_models_.size() )
      throw KernelException(
        "Custom neuron models exist: Thread/process number cannot be changed." );
    if ( net_->connection_manager_.has_user_prototypes() )
      throw KernelException(
        "Custom synapse types exist: Thread/process number cannot be changed." );
    if ( net_->connection_manager_.get_user_set_delay_extrema() )
      throw KernelException(
        "Delay extrema have been set: Thread/process number cannot be changed." );
    if ( net_->get_simulated() )
      throw KernelException(
        "The network has been simulated: Thread/process number cannot be changed." );
    if ( not Time::resolution_is_default() )
      throw KernelException(
        "The resolution has been set: Thread/process number cannot be changed." );
    if ( net_->model_defaults_modified() )
      throw KernelException(
        "Model defaults have been modified: Thread/process number cannot be changed." );

    if ( n_threads > 1 && force_singlethreading_ )
    {
      net_->message( SLIInterpreter::M_WARNING,
        "Scheduler::set_status",
        "No multithreading available, using single threading" );
      n_threads_ = 1;
    }

    // it is essential to call net_->reset() here to adapt memory pools and more
    // to the new number of threads and VPs.
    n_threads_ = n_threads;
    net_->reset();
  }

  long n_vps;
  bool n_vps_updated = updateValue< long >( d, "total_num_virtual_procs", n_vps );
  if ( n_vps_updated )
  {
    if ( net_->size() > 1 )
      throw KernelException( "Nodes exist: Thread/process number cannot be changed." );
    if ( net_->models_.size() > net_->pristine_models_.size() )
      throw KernelException(
        "Custom neuron models exist: Thread/process number cannot be changed." );
    if ( net_->connection_manager_.has_user_prototypes() )
      throw KernelException(
        "Custom synapse types exist: Thread/process number cannot be changed." );
    if ( net_->connection_manager_.get_user_set_delay_extrema() )
      throw KernelException(
        "Delay extrema have been set: Thread/process number cannot be changed." );
    if ( net_->get_simulated() )
      throw KernelException(
        "The network has been simulated: Thread/process number cannot be changed." );
    if ( not Time::resolution_is_default() )
      throw KernelException(
        "The resolution has been set: Thread/process number cannot be changed." );
    if ( net_->model_defaults_modified() )
      throw KernelException(
        "Model defaults have been modified: Thread/process number cannot be changed." );

    if ( n_vps % Communicator::get_num_processes() != 0 )
      throw BadProperty(
        "Number of virtual processes (threads*processes) must be an integer "
        "multiple of the number of processes. Value unchanged." );

    n_threads_ = n_vps / Communicator::get_num_processes();
    if ( ( n_threads > 1 ) && ( force_singlethreading_ ) )
    {
      net_->message( SLIInterpreter::M_WARNING,
        "Scheduler::set_status",
        "No multithreading available, using single threading" );
      n_threads_ = 1;
    }

    // it is essential to call net_->reset() here to adapt memory pools and more
    // to the new number of threads and VPs
    set_num_threads( n_threads_ );
    net_->reset();
  }

  // tics_per_ms and resolution must come after local_num_thread / total_num_threads
  // because they might reset the network and the time representation
  nest::double_t tics_per_ms;
  bool tics_per_ms_updated = updateValue< nest::double_t >( d, "tics_per_ms", tics_per_ms );
  double_t resd;
  bool res_updated = updateValue< double_t >( d, "resolution", resd );

  if ( tics_per_ms_updated || res_updated )
  {
    if ( net_->size() > 1 ) // root always exists
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "Cannot change time representation after nodes have been created. Please call ResetKernel "
        "first." );
      throw KernelException();
    }
    else if ( net_->get_simulated() ) // someone may have simulated empty network
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "Cannot change time representation after the network has been simulated. Please call "
        "ResetKernel first." );
      throw KernelException();
    }
    else if ( net_->connection_manager_.get_num_connections() != 0 )
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "Cannot change time representation after connections have been created. Please call "
        "ResetKernel first." );
      throw KernelException();
    }
    else if ( res_updated
      && tics_per_ms_updated ) // only allow TICS_PER_MS to be changed together with resolution
    {
      if ( resd < 1.0 / tics_per_ms )
      {
        net_->message( SLIInterpreter::M_ERROR,
          "Scheduler::set_status",
          "Resolution must be greater than or equal to one tic. Value unchanged." );
        throw KernelException();
      }
      else
      {
        nest::TimeModifier::set_time_representation( tics_per_ms, resd );
        clock_.calibrate(); // adjust to new resolution
        net_->connection_manager_.calibrate(
          time_converter ); // adjust delays in the connection system to new resolution
        net_->message(
          SLIInterpreter::M_INFO, "Scheduler::set_status", "tics per ms and resolution changed." );
      }
    }
    else if ( res_updated ) // only resolution changed
    {
      if ( resd < Time::get_ms_per_tic() )
      {
        net_->message( SLIInterpreter::M_ERROR,
          "Scheduler::set_status",
          "Resolution must be greater than or equal to one tic. Value unchanged." );
        throw KernelException();
      }
      else
      {
        Time::set_resolution( resd );
        clock_.calibrate(); // adjust to new resolution
        net_->connection_manager_.calibrate(
          time_converter ); // adjust delays in the connection system to new resolution
        net_->message(
          SLIInterpreter::M_INFO, "Scheduler::set_status", "Temporal resolution changed." );
      }
    }
    else
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "change of tics_per_step requires simultaneous specification of resolution." );
      throw KernelException();
    }
  }

  if ( updateValue< bool >( d, "off_grid_spiking", off_grid_spiking_ ) && dry_run_enabled_ )
    throw KernelException( "Off-grid spiking must not be enabled when in dry-run mode." );

  bool comm_allgather;
  bool commstyle_updated = updateValue< bool >( d, "communicate_allgather", comm_allgather );
  if ( commstyle_updated )
    Communicator::set_use_Allgather( comm_allgather );

  // set RNGs --- MUST come after n_threads_ is updated
  if ( d->known( "rngs" ) )
  {
    // this array contains pre-seeded RNGs, so they can be used
    // directly, no seeding required
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( ( *d )[ "rngs" ].datum() );
    if ( ad == 0 )
      throw BadProperty();

    // n_threads_ is the new value after a change of the number of
    // threads
    if ( ad->size() != ( size_t )( Communicator::get_num_virtual_processes() ) )
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "Number of RNGs must equal number of virtual processes (threads*processes). RNGs "
        "unchanged." );
      throw DimensionMismatch(
        ( size_t )( Communicator::get_num_virtual_processes() ), ad->size() );
    }

    // delete old generators, insert new generators this code is
    // robust under change of thread number in this call to
    // set_status, as long as it comes AFTER n_threads_ has been
    // upated
    rng_.clear();
    for ( index i = 0; i < ad->size(); ++i )
      if ( is_local_vp( i ) )
        rng_.push_back( getValue< librandom::RngDatum >( ( *ad )[ suggest_vp( i ) ] ) );
  }
  else if ( n_threads_updated && net_->size() == 0 )
  {
    net_->message( SLIInterpreter::M_WARNING,
      "Scheduler::set_status",
      "Equipping threads with new default RNGs" );
    create_rngs_();
  }

  if ( d->known( "rng_seeds" ) )
  {
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( ( *d )[ "rng_seeds" ].datum() );
    if ( ad == 0 )
      throw BadProperty();

    if ( ad->size() != ( size_t )( Communicator::get_num_virtual_processes() ) )
    {
      net_->message( SLIInterpreter::M_ERROR,
        "Scheduler::set_status",
        "Number of seeds must equal number of virtual processes (threads*processes). RNGs "
        "unchanged." );
      throw DimensionMismatch(
        ( size_t )( Communicator::get_num_virtual_processes() ), ad->size() );
    }

    // check if seeds are unique
    std::set< ulong_t > seedset;
    for ( index i = 0; i < ad->size(); ++i )
    {
      long s = ( *ad )[ i ]; // SLI has no ulong tokens
      if ( !seedset.insert( s ).second )
      {
        net_->message( SLIInterpreter::M_WARNING,
          "Scheduler::set_status",
          "Seeds are not unique across threads!" );
        break;
      }
    }

    // now apply seeds, resets generators automatically
    for ( index i = 0; i < ad->size(); ++i )
    {
      long s = ( *ad )[ i ];

      if ( is_local_vp( i ) )
        rng_[ vp_to_thread( suggest_vp( i ) ) ]->seed( s );

      rng_seeds_[ i ] = s;
    }
  } // if rng_seeds

  // set GRNG
  if ( d->known( "grng" ) )
  {
    // pre-seeded grng that can be used directly, no seeding required
    updateValue< librandom::RngDatum >( d, "grng", grng_ );
  }
  else if ( n_threads_updated && net_->size() == 0 )
  {
    net_->message( SLIInterpreter::M_WARNING,
      "Scheduler::set_status",
      "Equipping threads with new default GRNG" );
    create_grng_();
  }

  if ( d->known( "grng_seed" ) )
  {
    const long gseed = getValue< long >( d, "grng_seed" );

    // check if grng seed is unique with respect to rng seeds
    // if grng_seed and rng_seeds given in one SetStatus call
    std::set< ulong_t > seedset;
    seedset.insert( gseed );
    if ( d->known( "rng_seeds" ) )
    {
      ArrayDatum* ad_rngseeds = dynamic_cast< ArrayDatum* >( ( *d )[ "rng_seeds" ].datum() );
      if ( ad_rngseeds == 0 )
        throw BadProperty();
      for ( index i = 0; i < ad_rngseeds->size(); ++i )
      {
        const long vpseed = ( *ad_rngseeds )[ i ]; // SLI has no ulong tokens
        if ( !seedset.insert( vpseed ).second )
        {
          net_->message( SLIInterpreter::M_WARNING,
            "Scheduler::set_status",
            "Seeds are not unique across threads!" );
          break;
        }
      }
    }
    // now apply seed, resets generator automatically
    grng_seed_ = gseed;
    grng_->seed( gseed );

  } // if grng_seed
}

void
nest::Scheduler::get_status( DictionaryDatum& d ) const
{
  assert( initialized_ );

  def< long >( d, "local_num_threads", n_threads_ );
  def< long >( d, "total_num_virtual_procs", Communicator::get_num_virtual_processes() );
  def< long >( d, "num_processes", Communicator::get_num_processes() );

  def< double_t >( d, "time", get_time().get_ms() );
  def< long >( d, "to_do", to_do_ );
  def< bool >( d, "print_time", print_time_ );

  def< double >( d, "tics_per_ms", Time::get_tics_per_ms() );
  def< double >( d, "resolution", Time::get_resolution().get_ms() );

  update_delay_extrema_();
  def< double >( d, "min_delay", Time( Time::step( min_delay_ ) ).get_ms() );
  def< double >( d, "max_delay", Time( Time::step( max_delay_ ) ).get_ms() );

  def< double >( d, "ms_per_tic", Time::get_ms_per_tic() );
  def< double >( d, "tics_per_ms", Time::get_tics_per_ms() );
  def< long >( d, "tics_per_step", Time::get_tics_per_step() );

  def< double >( d, "T_min", Time::min().get_ms() );
  def< double >( d, "T_max", Time::max().get_ms() );

  ( *d )[ "rng_seeds" ] = Token( rng_seeds_ );
  def< long >( d, "grng_seed", grng_seed_ );
  def< bool >( d, "off_grid_spiking", off_grid_spiking_ );
  def< bool >( d, "communicate_allgather", Communicator::get_use_Allgather() );
  def< long >( d, "send_buffer_size", Communicator::get_send_buffer_size() );
  def< long >( d, "receive_buffer_size", Communicator::get_recv_buffer_size() );

  def< bool >( d, "dry_run", dry_run_enabled_ );
  if ( dry_run_enabled_ )
  {
    def< double >( d, "dry_run_target_rate", dry_run_target_rate_ );
    def< bool >( d, "dry_run_spike_filter", dry_run_only_relevant_spikes_ );
    def< double >( d, "dry_run_num_spikes_per_h", dry_run_num_spikes_per_h_ );
    def< bool >( d, "dry_run_indirect_access_enabled", dry_run_indirect_access_enabled_ );
    def< double >( d, "dry_run_num_spikes_per_h_threshold", dry_run_num_spikes_per_h_threshold_ );
  }

  if ( n_rec_procs_ > 0 )
    def< double >( d, "num_rec_processes", n_rec_procs_ );

  def< double >( d, "gather_walltime", Scheduler::get_gather_walltime_() );
  def< double >( d, "collocate_buffers_walltime", Scheduler::get_collocate_buffers_walltime_() );
  def< size_t >( d, "local_spike_counter", Scheduler::get_local_spike_counter_() );
  // BEGIN: DEBUGGING HACK BY WS
  def< size_t >( d, "facilitate_counter", Scheduler::get_facilitate_counter_() );
  // END: DEBUGGING HACK BY WS
}

void
nest::Scheduler::create_rngs_( const bool ctor_call )
{
  // net_->message(SLIInterpreter::M_INFO, ) calls must not be called
  // if create_rngs_ is called from Scheduler::Scheduler(), since net_
  // is not fully constructed then

  // if old generators exist, remove them; since rng_ contains
  // lockPTRs, we don't have to worry about deletion
  if ( !rng_.empty() )
  {
    if ( !ctor_call )
      net_->message( SLIInterpreter::M_INFO,
        "Scheduler::create_rngs_",
        "Deleting existing random number generators" );

    rng_.clear();
  }

  // create new rngs
  if ( !ctor_call )
    net_->message( SLIInterpreter::M_INFO, "Scheduler::create_rngs_", "Creating default RNGs" );

  rng_seeds_.resize( Communicator::get_num_virtual_processes() );

  for ( index i = 0; i < static_cast< index >( Communicator::get_num_virtual_processes() ); ++i )
  {
    unsigned long s = i + 1;
    if ( is_local_vp( i ) )
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
#ifdef HAVE_GSL
      librandom::RngPtr rng( new librandom::GslRandomGen( gsl_rng_knuthran2002, s ) );
#else
      librandom::RngPtr rng = librandom::RandomGen::create_knuthlfg_rng( s );
#endif

      if ( !rng )
      {
        if ( !ctor_call )
          net_->message(
            SLIInterpreter::M_ERROR, "Scheduler::create_rngs_", "Error initializing knuthlfg" );
        else
          std::cerr << "\nScheduler::create_rngs_\n"
                    << "Error initializing knuthlfg" << std::endl;

        throw KernelException();
      }

      rng_.push_back( rng );
    }

    rng_seeds_[ i ] = s;
  }
}

void
nest::Scheduler::create_grng_( const bool ctor_call )
{

  // create new grng
  if ( !ctor_call )
    net_->message(
      SLIInterpreter::M_INFO, "Scheduler::create_grng_", "Creating new default global RNG" );

// create default RNG with default seed
#ifdef HAVE_GSL
  grng_ = librandom::RngPtr(
    new librandom::GslRandomGen( gsl_rng_knuthran2002, librandom::RandomGen::DefaultSeed ) );
#else
  grng_ = librandom::RandomGen::create_knuthlfg_rng( librandom::RandomGen::DefaultSeed );
#endif

  if ( !grng_ )
  {
    if ( !ctor_call )
      net_->message(
        SLIInterpreter::M_ERROR, "Scheduler::create_grng_", "Error initializing knuthlfg" );
    else
      std::cerr << "\nScheduler::create_grng_\n"
                << "Error initializing knuthlfg" << std::endl;

    throw KernelException();
  }

  /*
   The seed for the global rng should be different from the seeds
   of the local rngs_ for each thread seeded with 1,..., n_vps.
   */
  long s = 0;
  grng_seed_ = s;
  grng_->seed( s );
}


void
nest::Scheduler::collocate_buffers_()
{
  // count number of spikes in registers
  int num_spikes = 0;
  int num_grid_spikes = 0;
  int num_offgrid_spikes = 0;

  std::vector< std::vector< std::vector< uint_t > > >::iterator i;
  std::vector< std::vector< uint_t > >::iterator j;
  for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
    for ( j = i->begin(); j != i->end(); ++j )
      num_grid_spikes += j->size();

  std::vector< std::vector< std::vector< OffGridSpike > > >::iterator it;
  std::vector< std::vector< OffGridSpike > >::iterator jt;
  for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
    for ( jt = it->begin(); jt != it->end(); ++jt )
      num_offgrid_spikes += jt->size();

  num_spikes = num_grid_spikes + num_offgrid_spikes;

  // for counting of locally generated spikes, especially for dry-run mode
  local_spike_counter_ += num_spikes;

  if ( !off_grid_spiking_ ) // on grid spiking
  {
    // make sure buffers are correctly sized
    if ( global_grid_spikes_.size()
      != static_cast< uint_t >( Communicator::get_recv_buffer_size() ) )
      global_grid_spikes_.resize( Communicator::get_recv_buffer_size(), 0 );

    if ( num_spikes + ( n_threads_ * min_delay_ )
      > static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_grid_spikes_.resize( ( num_spikes + ( min_delay_ * n_threads_ ) ), 0 );
    else if ( local_grid_spikes_.size()
      < static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_grid_spikes_.resize( Communicator::get_send_buffer_size(), 0 );

    // collocate the entries of spike_registers into local_grid_spikes__
    std::vector< uint_t >::iterator pos = local_grid_spikes_.begin();
    if ( num_offgrid_spikes == 0 )
      for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
        for ( j = i->begin(); j != i->end(); ++j )
        {
          pos = std::copy( j->begin(), j->end(), pos );
          *pos = comm_marker_;
          ++pos;
        }
    else
    {
      std::vector< OffGridSpike >::iterator n;
      it = offgrid_spike_register_.begin();
      for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
      {
        jt = it->begin();
        for ( j = i->begin(); j != i->end(); ++j )
        {
          pos = std::copy( j->begin(), j->end(), pos );
          for ( n = jt->begin(); n != jt->end(); ++n )
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
      for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
        for ( jt = it->begin(); jt != it->end(); ++jt )
          jt->clear();
    }

    // remove old spikes from the spike_register_
    for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
      for ( j = i->begin(); j != i->end(); ++j )
        j->clear();
  }
  else // off_grid_spiking
  {
    // make sure buffers are correctly sized
    if ( global_offgrid_spikes_.size()
      != static_cast< uint_t >( Communicator::get_recv_buffer_size() ) )
      global_offgrid_spikes_.resize( Communicator::get_recv_buffer_size(), OffGridSpike( 0, 0.0 ) );

    if ( num_spikes + ( n_threads_ * min_delay_ )
      > static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_offgrid_spikes_.resize(
        ( num_spikes + ( min_delay_ * n_threads_ ) ), OffGridSpike( 0, 0.0 ) );
    else if ( local_offgrid_spikes_.size()
      < static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_offgrid_spikes_.resize( Communicator::get_send_buffer_size(), OffGridSpike( 0, 0.0 ) );

    // collocate the entries of spike_registers into local_offgrid_spikes__
    std::vector< OffGridSpike >::iterator pos = local_offgrid_spikes_.begin();
    if ( num_grid_spikes == 0 )
      for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
        for ( jt = it->begin(); jt != it->end(); ++jt )
        {
          pos = std::copy( jt->begin(), jt->end(), pos );
          pos->set_gid( comm_marker_ );
          ++pos;
        }
    else
    {
      std::vector< uint_t >::iterator n;
      i = spike_register_.begin();
      for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
      {
        j = i->begin();
        for ( jt = it->begin(); jt != it->end(); ++jt )
        {
          pos = std::copy( jt->begin(), jt->end(), pos );
          for ( n = j->begin(); n != j->end(); ++n )
          {
            *pos = OffGridSpike( *n, 0 );
            ++pos;
          }
          pos->set_gid( comm_marker_ );
          ++pos;
          ++j;
        }
        ++i;
      }
      for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
        for ( j = i->begin(); j != i->end(); ++j )
          j->clear();
    }

    // empty offgrid_spike_register_
    for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
      for ( jt = it->begin(); jt != it->end(); ++jt )
        jt->clear();
  }
}

void
nest::Scheduler::deliver_events_( thread t )
{
  // deliver only at beginning of time slice
  if ( from_step_ > 0 )
    return;

  SpikeEvent se;

  std::vector< int > pos( displacements_ );

  if ( !off_grid_spiking_ ) // on_grid_spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps( min_delay_ );
    for ( size_t lag = 0; lag < ( size_t ) min_delay_; lag++ )
    {
      prepared_timestamps[ lag ] = clock_ - Time::step( lag );
    }

    if ( dry_run_enabled_ && dry_run_only_relevant_spikes_ )
    {
      for ( size_t vp = 0; vp < ( size_t ) Communicator::get_num_virtual_processes(); ++vp )
      {
        size_t pid = get_process_id( vp );
        int pos_pid = pos[ pid ];
        int lag = min_delay_ - 1;
        while ( lag >= 0 )
        {
          index nid = global_grid_spikes_thread_specific_[ t ][ pos_pid ];
          if ( nid != static_cast< index >( comm_marker_ ) )
          {
            // tell all local nodes about spikes on remote machines.
            se.set_stamp( prepared_timestamps[ lag ] );
            se.set_sender_gid( nid );

            //// trusted send because preceding spike filtering ensures
            //// that only relevant spikes are present in spike buffer
            // net_->connection_manager_.send_trusted(t, nid, se);
            net_->connection_manager_.send( t, nid, se );
          }
          else
          {
            --lag;
          }
          ++pos_pid;
        }
        pos[ pid ] = pos_pid;
      }
    }
    else
    {
      for ( size_t vp = 0; vp < ( size_t ) Communicator::get_num_virtual_processes(); ++vp )
      {
        size_t pid = get_process_id( vp );
        int pos_pid = pos[ pid ];
        int lag = min_delay_ - 1;
        while ( lag >= 0 )
        {
          index nid = global_grid_spikes_[ pos_pid ];
          if ( nid != static_cast< index >( comm_marker_ ) )
          {
            // tell all local nodes about spikes on remote machines.
            se.set_stamp( prepared_timestamps[ lag ] );
            se.set_sender_gid( nid );
            net_->connection_manager_.send( t, nid, se );
          }
          else
          {
            --lag;
          }
          ++pos_pid;
        }
        pos[ pid ] = pos_pid;
      }
    }
  }
  else // off grid spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps( min_delay_ );
    for ( size_t lag = 0; lag < ( size_t ) min_delay_; lag++ )
    {
      prepared_timestamps[ lag ] = clock_ - Time::step( lag );
    }

    for ( size_t vp = 0; vp < ( size_t ) Communicator::get_num_virtual_processes(); ++vp )
    {
      size_t pid = get_process_id( vp );
      int pos_pid = pos[ pid ];
      int lag = min_delay_ - 1;
      while ( lag >= 0 )
      {
        index nid = global_offgrid_spikes_[ pos_pid ].get_gid();
        if ( nid != static_cast< index >( comm_marker_ ) )
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp( prepared_timestamps[ lag ] );
          se.set_sender_gid( nid );
          se.set_offset( global_offgrid_spikes_[ pos_pid ].get_offset() );
          net_->connection_manager_.send( t, nid, se );
        }
        else
        {
          --lag;
        }
        ++pos_pid;
      }
      pos[ pid ] = pos_pid;
    }
  }
}

void
nest::Scheduler::gather_events_()
{
  // time measurements: init (assuming that this function is only called from one thread)
  Stopwatch stw, stw_cb;
  stw.start();

  // buffer handling (with time measurement)
  stw_cb.start();
  collocate_buffers_();
  stw_cb.stop();
  collocate_buffers_walltime_ += stw_cb.elapsed();

  // invoke MPI communication
  if ( off_grid_spiking_ )
    Communicator::communicate( local_offgrid_spikes_, global_offgrid_spikes_, displacements_ );
  else
    Communicator::communicate( local_grid_spikes_, global_grid_spikes_, displacements_ );
  // main stuff: end

  // time measurement: finish
  stw.stop();
  gather_walltime_ += stw.elapsed();
}

void
nest::Scheduler::gather_events_for_dry_run_( uint_t t, uint_t num_t )
{
  // IMPORTANT: Assume that all threads are synchronized at this point

  Stopwatch stw, stw_cb; // only used in master thread

#pragma omp master
  { // only master thread (or only thread) does this work
    // time measurement: init
    stw.start();

    // clean local spike buffer (not really necessary, but nicer for debugging)
    fill( local_grid_spikes_.begin(), local_grid_spikes_.end(), 0 );

    // usual buffer handling (with time measurement)
    stw_cb.start();
    collocate_buffers_();
    stw_cb.stop();
    collocate_buffers_walltime_ += stw_cb.elapsed();

    if ( dry_run_target_rate_ <= 0.0 ) // only for dry-run mode
                                       // without fixed target spike rate
    {
      int send_buffer_size = local_grid_spikes_.size();
      int recv_buffer_size = global_grid_spikes_.size();

      int old_send_buffer_size = Communicator::get_send_buffer_size();

      // detect that local spike buffer has increased in size
      // and update global spike buffer and Communicator state accordingly
      if ( send_buffer_size > old_send_buffer_size )
      {
        recv_buffer_size = send_buffer_size * Communicator::get_num_processes();
        global_grid_spikes_.resize( recv_buffer_size );
        Communicator::set_buffer_sizes( send_buffer_size, recv_buffer_size );

        // update displacements_ as well;
        // the displacements are perfectly regular
        for ( int pid = 0; pid < Communicator::get_num_processes(); ++pid )
        {
          displacements_[ pid ] = send_buffer_size * pid;
        }
      }

      // copy spike events from only real process to global spike buffer
      // (for the real process AND for all fake processes)
      for ( int pid = 0; pid < Communicator::get_num_processes(); ++pid )
      {
        std::copy( local_grid_spikes_.begin(),
          local_grid_spikes_.end(),
          global_grid_spikes_.begin() + pid * send_buffer_size );
      }
    }
  }

// synchronize here
#pragma omp barrier
  // all threads do the following work

  if ( dry_run_target_rate_ <= 0.0 ) // dry-run mode without fixed target spike rate
  {
    // get a local random generator
    librandom::RngPtr rng = get_rng( t );

// ...and now modify the spike events of all processes such
// that the GIDs match the VPs somehow (plus some extra randomness)

#ifdef DRY_RUN_SUBNET_MODE
    int sqrt_M = static_cast< int >( sqrt( Communicator::get_num_processes() ) );
#endif // DRY_RUN_SUBNET_MODE

// loop over all processes (real and fake; note that pid counting starts
// with 1 to preserve spike events on single real process)
#pragma omp for
    for ( int pid = 1; pid < Communicator::get_num_processes(); ++pid )
    {
      int vp_num = pid;                // VP counter, starting at pid for every process
      int pos = displacements_[ pid ]; // displacements_ filled before
      index n_markers = 0;             // counter for comm. markers
      nest::delay h_count = 0;         // counter for h-steps within slice

      // loop until max. number of comm. markers is exceeded
      while ( n_markers < n_threads_ * min_delay_ )
      {
        // get spike event (gid)
        index gid = global_grid_spikes_[ pos ];
        // in case of comm. marker increase all counters
        if ( gid == comm_marker_ )
        {
          n_markers++;
          h_count++;
          // check if we have already reached the end of a slice
          if ( h_count == min_delay_ )
          {
            // if slice is finished, skip to next VP on process
            vp_num += Communicator::get_num_processes();
            // reset h_count (for next slice)
            h_count = 0;
          }
        }
        else
        {
#ifdef DRY_RUN_SUBNET_MODE
          if ( pid <= sqrt_M )
          {
            global_grid_spikes_[ pos ] = createNewFakeGID_SubnetMode_( gid, vp_num, rng );
          }
          else
          {
            global_grid_spikes_[ pos ] = createNewFakeGID_( vp_num, rng );
          }
#else  // DRY_RUN_SUBNET_MODE
          global_grid_spikes_[ pos ] = createNewFakeGID_( vp_num, rng );
#endif // DRY_RUN_SUBNET_MODE
        }
        // iterate forward in global spike buffer
        pos++;
      }
    }
  }
  else // dry-run mode with fixed target spike rate
  {
    // get a local random generator
    librandom::RngPtr rng = get_rng( t );

    // update every spike event in the prefilled global spike buffer such that
    // there remains a realistic spike pattern (GIDs fitting to VPs)
    if ( dry_run_indirect_access_enabled_ )
    { // sparse case, few spike events in global spike buffer, access global_grid_spikes_ indirectly
      // (only at positions at which are for sure spike events)
#pragma omp for
      for ( uint_t u = 0; u < dry_run_valid_spike_pos_vec_.size(); u++ )
      {
        uint_t u2 = dry_run_valid_spike_pos_vec_[ u ];
        global_grid_spikes_[ u2 ] = createNewFakeGID_( global_grid_spikes_[ u2 ], rng );
      }
    }
    else
    { // non-sparse case, iterate simply over the whole global spike buffer and avoid
      // hitting comm. markers
#pragma omp for
      for ( index i = 0; i < global_grid_spikes_.size(); i++ )
      {
        if ( global_grid_spikes_[ i ] != comm_marker_ )
        {
          global_grid_spikes_[ i ] = createNewFakeGID_( global_grid_spikes_[ i ], rng );
        }
      }
    }
  } // versions of dry-run mode

  // filter global spike buffer and leave only relevant spikes
  // (for which target exists on real virtual processes)
  if ( dry_run_only_relevant_spikes_ )
  {
// synchronize all threads
#pragma omp barrier

    // ensure proper sizing of thread-specific global spike buffers
    if ( global_grid_spikes_thread_specific_[ t ].size() != global_grid_spikes_.size() )
    {
      global_grid_spikes_thread_specific_[ t ].resize( global_grid_spikes_.size() );
    }

    index n_markers;
    int irrelevantCount = 0;

    // loop over global spike buffer (process-wise)
    for ( int pid = 0; pid < Communicator::get_num_processes(); ++pid )
    {
      int pos = displacements_[ pid ];
      int pos_only_relevant = displacements_[ pid ];

      n_markers = 0;
      while ( n_markers < n_threads_ * min_delay_ )
      {
        index gid = global_grid_spikes_[ pos ];
        if ( gid != comm_marker_ )
        {
          if ( net_->connection_manager_.check_source_relevance( t, gid ) )
          {
            // valid entry -> gets into as relevant spike into thread-specific
            //                global spike buffer
            global_grid_spikes_thread_specific_[ t ][ pos_only_relevant++ ] = gid;
          }
          else
          {
            irrelevantCount++;
          }
        }
        else
        {
          global_grid_spikes_thread_specific_[ t ][ pos_only_relevant++ ] = comm_marker_;
          ++n_markers;
        }
        ++pos;
      }
    }
#pragma omp critical
    {
      dry_run_irrelevant_spikes_counter_ = dry_run_irrelevant_spikes_counter_ + irrelevantCount;
    }
  }

// finally synchronize all threads
#pragma omp barrier

#pragma omp master
  {
    // time measurement: finish
    stw.stop();
    gather_walltime_ += stw.elapsed();
  }
}


void
nest::Scheduler::advance_time_()
{
  // time now advanced time by the duration of the previous step
  to_do_ -= to_step_ - from_step_;

  // advance clock, update modulos, slice counter only if slice completed
  if ( ( delay ) to_step_ == min_delay_ )
  {
    clock_ += Time::step( min_delay_ );
    ++slice_;
    compute_moduli_();
    from_step_ = 0;
  }
  else
    from_step_ = to_step_;

  long_t end_sim = from_step_ + to_do_;

  if ( min_delay_ < ( delay ) end_sim )
    to_step_ = min_delay_; // update to end of time slice
  else
    to_step_ = end_sim; // update to end of simulation time

  assert( to_step_ - from_step_ <= ( long_t ) min_delay_ );
}

void
nest::Scheduler::print_progress_()
{
  double_t rt_factor = 0.0;

  if ( t_slice_end_.tv_sec != 0 )
  {
    long t_real_s = ( t_slice_end_.tv_sec - t_slice_begin_.tv_sec ) * 1e6;   // usec
    t_real_ += t_real_s + ( t_slice_end_.tv_usec - t_slice_begin_.tv_usec ); // usec
    double_t t_real_acc = ( t_real_ ) / 1000.; // ms
    double_t t_sim_acc = ( to_do_total_ - to_do_ ) * Time::get_resolution().get_ms();
    rt_factor = t_sim_acc / t_real_acc;
  }

  int_t percentage = ( 100 - int( float( to_do_ ) / to_do_total_ * 100 ) );

  std::cout << "\r" << std::setw( 3 ) << std::right << percentage << " %: "
            << "network time: " << std::fixed << std::setprecision( 1 ) << clock_.get_ms()
            << " ms, "
            << "realtime factor: " << std::setprecision( 4 ) << rt_factor
            << std::resetiosflags( std::ios_base::floatfield );
  std::flush( std::cout );
}

void
nest::Scheduler::set_num_rec_processes( int nrp, bool called_by_reset )
{
  if ( dry_run_enabled_ && ( nrp > 0 ) )
    throw KernelException(
      "Global spike detection mode must not be enabled when in dry-run mode." );
  if ( net_->size() > 1 and not called_by_reset )
    throw KernelException(
      "Global spike detection mode must be enabled before nodes are created." );
  if ( nrp >= Communicator::get_num_processes() )
    throw KernelException(
      "Number of processes used for recording must be smaller than total number of processes." );
  n_rec_procs_ = nrp;
  n_sim_procs_ = Communicator::get_num_processes() - n_rec_procs_;
  create_rngs_( true );
  if ( nrp > 0 )
  {
    std::string msg = String::compose(
      "Entering global spike detection mode with %1 recording MPI processes and %2 simulating MPI "
      "processes.",
      n_rec_procs_,
      n_sim_procs_ );
    net_->message( SLIInterpreter::M_INFO, "Scheduler::set_num_rec_processes", msg );
  }
}

void
nest::Scheduler::set_num_threads( thread n_threads )
{
  n_threads_ = n_threads;
  nodes_vec_.resize( n_threads_ );

#ifdef _OPENMP
  omp_set_num_threads( n_threads_ );

#ifdef USE_PMA
// initialize the memory pools
#ifdef IS_K
  assert( n_threads <= MAX_THREAD && "MAX_THREAD is a constant defined in allocator.h" );

#pragma omp parallel
  poormansallocpool[ omp_get_thread_num() ].init();
#else
#pragma omp parallel
  poormansallocpool.init();
#endif
#endif

#endif
  Communicator::set_num_threads( n_threads_ );
}
