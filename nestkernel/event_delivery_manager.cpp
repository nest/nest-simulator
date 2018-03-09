/*
 *  event_delivery_manager.cpp
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

#include "event_delivery_manager.h"

// C++ includes:
#include <algorithm> // rotate
#include <iostream>
#include <numeric> // accumulate

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "vp_manager.h"
#include "vp_manager_impl.h"
#include "connection_manager.h"
#include "connection_manager_impl.h"
#include "event_delivery_manager_impl.h"
#include "send_buffer_position.h"
#include "source.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
EventDeliveryManager::EventDeliveryManager()
  : off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
  , comm_marker_( 0 )
  , time_collocate_( 0.0 )
  , time_communicate_( 0.0 )
  , local_spike_counter_( std::vector< unsigned long >() )
  , send_buffer_target_data_( NULL )
  , recv_buffer_target_data_( NULL )
  , buffer_size_target_data_has_changed_( false )
  , buffer_size_spike_data_has_changed_( false )
  , completed_count_( std::vector< unsigned int >() )
{
#ifndef DISABLE_COUNTS
  comm_steps_target_data = 0;
  comm_rounds_target_data = 0;
  comm_steps_spike_data = 0;
  comm_rounds_spike_data = 0;
  comm_steps_secondary_events = 0;
  call_count_deliver_events_5g = std::vector< unsigned int >();
#endif
}

EventDeliveryManager::~EventDeliveryManager()
{
}

void
EventDeliveryManager::initialize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();

  init_moduli();
  local_spike_counter_.resize( num_threads, 0 );
  reset_timers_counters();
  spike_register_5g_.resize( num_threads, NULL );
  off_grid_spike_register_5g_.resize( num_threads, NULL );
  completed_count_.resize( num_threads, 0 );
#ifndef DISABLE_COUNTS
  call_count_deliver_events_5g.resize( num_threads, 0 ); // TODO@5g: remove
#endif

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    if ( spike_register_5g_[ tid ] != 0 )
    {
      delete ( spike_register_5g_[ tid ] );
    }
    spike_register_5g_[ tid ] =
      new std::vector< std::vector< std::vector< Target > > >(
        num_threads,
        std::vector< std::vector< Target > >(
          kernel().connection_manager.get_min_delay(),
          std::vector< Target >( 0 ) ) );

    if ( off_grid_spike_register_5g_[ tid ] != 0 )
    {
      delete ( off_grid_spike_register_5g_[ tid ] );
    }
    off_grid_spike_register_5g_[ tid ] =
      new std::vector< std::vector< std::vector< OffGridTarget > > >(
        num_threads,
        std::vector< std::vector< OffGridTarget > >(
          kernel().connection_manager.get_min_delay(),
          std::vector< OffGridTarget >( 0 ) ) );
  } // of omp parallel
}

void
EventDeliveryManager::finalize()
{
  // clear the spike buffers
  for ( std::vector< std::vector< std::vector< std::vector< Target > > >* >::
          iterator it = spike_register_5g_.begin();
        it != spike_register_5g_.end();
        ++it )
  {
    delete ( *it );
  };
  spike_register_5g_.clear();

  for (
    std::vector< std::vector< std::vector< std::vector< OffGridTarget > > >* >::
      iterator it = off_grid_spike_register_5g_.begin();
    it != off_grid_spike_register_5g_.end();
    ++it )
  {
    delete ( *it );
  };
  off_grid_spike_register_5g_.clear();
}

void
EventDeliveryManager::set_status( const DictionaryDatum& dict )
{
  updateValue< bool >( dict, names::off_grid_spiking, off_grid_spiking_ );
}

void
EventDeliveryManager::get_status( DictionaryDatum& dict )
{
  def< bool >( dict, names::off_grid_spiking, off_grid_spiking_ );
  def< double >( dict, names::time_collocate, time_collocate_ );
  def< double >( dict, names::time_communicate, time_communicate_ );
  def< unsigned long >(
    dict, names::local_spike_counter, std::accumulate( local_spike_counter_.begin(), local_spike_counter_.end(), 0 ) );
}

void
EventDeliveryManager::clear_pending_spikes()
{
  configure_spike_data_buffers();
}

void
EventDeliveryManager::resize_send_recv_buffers_target_data()
{
  // clear old target data buffers
  if ( send_buffer_target_data_ != NULL )
  {
    free( send_buffer_target_data_ );
    send_buffer_target_data_ = NULL;
  }

  if ( recv_buffer_target_data_ != NULL )
  {
    free( recv_buffer_target_data_ );
    recv_buffer_target_data_ = NULL;
  }

  // compute send receive counts and allocate memory for buffers
  send_buffer_target_data_ = static_cast< TargetData* >(
    malloc( kernel().mpi_manager.get_buffer_size_target_data()
            * sizeof( TargetData ) ) );
  recv_buffer_target_data_ = static_cast< TargetData* >(
    malloc( kernel().mpi_manager.get_buffer_size_target_data()
            * sizeof( TargetData ) ) );

  assert( send_buffer_target_data_ != NULL );
  assert( recv_buffer_target_data_ != NULL );
}

void
EventDeliveryManager::resize_send_recv_buffers_spike_data_()
{
  send_buffer_spike_data_.resize(
    kernel().mpi_manager.get_buffer_size_spike_data() );
  recv_buffer_spike_data_.resize(
    kernel().mpi_manager.get_buffer_size_spike_data() );
  send_buffer_off_grid_spike_data_.resize(
    kernel().mpi_manager.get_buffer_size_spike_data() );
  recv_buffer_off_grid_spike_data_.resize(
    kernel().mpi_manager.get_buffer_size_spike_data() );
}

void
EventDeliveryManager::configure_spike_data_buffers()
{
  assert( kernel().connection_manager.get_min_delay() != 0 );

  configure_spike_register();

  send_buffer_spike_data_.clear();
  send_buffer_off_grid_spike_data_.clear();

  resize_send_recv_buffers_spike_data_();
}

void
EventDeliveryManager::configure_spike_register()
{
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid ) 
  {
    reset_spike_register_5g_( tid );
    resize_spike_register_5g_( tid );
  }
}

void
EventDeliveryManager::configure_secondary_buffers()
{
  send_buffer_secondary_events_.clear();
  send_buffer_secondary_events_.resize(
    kernel().mpi_manager.get_buffer_size_secondary_events_in_int() );
  recv_buffer_secondary_events_.clear();
  recv_buffer_secondary_events_.resize(
    kernel().mpi_manager.get_buffer_size_secondary_events_in_int() );
}

void
EventDeliveryManager::init_moduli()
{
  delay min_delay = kernel().connection_manager.get_min_delay();
  delay max_delay = kernel().connection_manager.get_max_delay();
  assert( min_delay != 0 );
  assert( max_delay != 0 );

  /*
   * Ring buffers use modulos to determine where to store incoming events
   * with given time stamps, relative to the beginning of the slice in which
   * the spikes are delivered from the queue, ie, the slice after the one
   * in which they were generated. The pertaining offsets are 0..max_delay-1.
   */

  moduli_.resize( min_delay + max_delay );

  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    moduli_[ d ] = ( kernel().simulation_manager.get_clock().get_steps() + d )
      % ( min_delay + max_delay );
  }

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast< size_t >(
    std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  slice_moduli_.resize( min_delay + max_delay );
  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel().simulation_manager.get_clock().get_steps()
                             + d ) / min_delay ) % nbuff;
  }
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
EventDeliveryManager::update_moduli()
{
  delay min_delay = kernel().connection_manager.get_min_delay();
  delay max_delay = kernel().connection_manager.get_max_delay();
  assert( min_delay != 0 );
  assert( max_delay != 0 );

  /*
   * Note that for updating the modulos, it is sufficient
   * to rotate the buffer to the left.
   */
  assert( moduli_.size() == ( index )( min_delay + max_delay ) );
  std::rotate( moduli_.begin(), moduli_.begin() + min_delay, moduli_.end() );

  /* For the slice-based ring buffer, we cannot rotate the table, but
   have to re-compute it, since max_delay_ may not be a multiple of
   min_delay_.  Reference time is the time at the beginning of the slice.
   */
  const size_t nbuff = static_cast< size_t >(
    std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel().simulation_manager.get_clock().get_steps()
                             + d ) / min_delay ) % nbuff;
  }
}

void
EventDeliveryManager::reset_timers_counters()
{
  time_collocate_ = 0.0;
  time_communicate_ = 0.0;
  for ( std::vector< unsigned long >::iterator it = local_spike_counter_.begin();
        it != local_spike_counter_.end(); ++it )
  {
    ( *it ) = 0;
  }
}

void
EventDeliveryManager::gather_secondary_events( const bool done )
{
#ifndef DISABLE_COUNTS
  ++comm_steps_secondary_events;
#endif

  // write done marker at last position in every chunk
  const size_t chunk_size_in_int =
    kernel().mpi_manager.get_chunk_size_secondary_events_in_int();
  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes();
        ++rank )
  {
    send_buffer_secondary_events_[ ( rank + 1 ) * chunk_size_in_int - 1 ] = done;
  }

#ifndef DISABLE_TIMING
  kernel().mpi_manager.synchronize(); // to get an accurate time measurement across ranks
  sw_communicate_secondary_events.start();
#endif
  kernel().mpi_manager.communicate_secondary_events_Alltoall(
    &send_buffer_secondary_events_[ 0 ], &recv_buffer_secondary_events_[ 0 ] );
#ifndef DISABLE_TIMING
  sw_communicate_secondary_events.stop();
#endif
}

bool
EventDeliveryManager::deliver_secondary_events( const thread tid, const bool called_from_wfr_update )
{
  return kernel().connection_manager.deliver_secondary_events(
    tid, called_from_wfr_update, recv_buffer_secondary_events_ );
}

void
EventDeliveryManager::gather_spike_data( const thread tid )
{
  if ( off_grid_spiking_ )
  {
    gather_spike_data_(
      tid,
      send_buffer_off_grid_spike_data_,
      recv_buffer_off_grid_spike_data_ );
  }
  else
  {
    gather_spike_data_(
      tid,
      send_buffer_spike_data_,
      recv_buffer_spike_data_ );
  }
}

// TODO@5g: make pretty and use reasonable variable names -> Jakob
template< typename SpikeDataT >
void
EventDeliveryManager::gather_spike_data_( const thread tid,
                                          std::vector< SpikeDataT >& send_buffer,
                                          std::vector< SpikeDataT >& recv_buffer )
{
#ifndef DISABLE_COUNTS
#pragma omp single
  {
    ++comm_steps_spike_data;
  }
#endif

  // counters to keep track of threads and ranks that have send out
  // all spikes
  static unsigned int completed_count; // needs to be static to be visible across threads
  const unsigned int half_completed_count =
    2 * kernel().vp_manager.get_num_threads(); // count for on grid and off grid spikes
  const unsigned int max_completed_count =
    half_completed_count + kernel().vp_manager.get_num_threads();
  unsigned int me_completed_tid = 0;
  unsigned int others_completed_tid = 0;

  const AssignedRanks assigned_ranks =
    kernel().vp_manager.get_assigned_ranks( tid );

  // can not use while(true) and break in an omp structured block
  bool done = false;
  while ( not done )
  {
#pragma omp single
    {
      completed_count = 0;
      if ( kernel().mpi_manager.adaptive_spike_buffers()
        and buffer_size_spike_data_has_changed_ )
      {
        resize_send_recv_buffers_spike_data_();
        buffer_size_spike_data_has_changed_ = false;
      }
    } // of omp single; implicit barrier

#ifndef DISABLE_TIMING
    if ( tid == 0 and kernel().mpi_manager.get_rank() < 30 )
    {
      sw_collocate_spike_data.start();
    }
#endif

    // need to get new positions in case buffer size has changed
    SendBufferPosition send_buffer_position(
      assigned_ranks, kernel().mpi_manager.get_send_recv_count_spike_data_per_rank() );

    // collocate spikes to send buffer
    me_completed_tid = collocate_spike_data_buffers_( tid,
                                                      assigned_ranks,
                                                      send_buffer_position,
                                                      spike_register_5g_,
                                                      send_buffer );

    if ( off_grid_spiking_ )
    {
      me_completed_tid += collocate_spike_data_buffers_( tid,
                                                         assigned_ranks,
                                                         send_buffer_position,
                                                         off_grid_spike_register_5g_,
                                                         send_buffer );
    }
    else
    {
      ++me_completed_tid;
    }

    // set markers to signal end of valid spikes, and remove spikes
    // from register that have been collected in send buffer
#pragma omp barrier
    set_end_and_invalid_markers_(
      assigned_ranks, send_buffer_position, send_buffer );
    clean_spike_register_( tid );

#pragma omp atomic
    completed_count += me_completed_tid;
#pragma omp barrier

    // if we do not have any spikes left, set corresponding marker in
    // send buffer
    if ( completed_count == half_completed_count )
    {
      // needs to be called /after/ set_end_and_invalid_markers_
      set_complete_marker_spike_data_(
        assigned_ranks, send_buffer_position, send_buffer );
#pragma omp barrier
    }

    // communicate spikes using a single thread
#pragma omp single
    {
#ifndef DISABLE_COUNTS
      ++comm_rounds_spike_data;
#endif
#ifndef DISABLE_TIMING
      sw_collocate_spike_data.stop();
      kernel().mpi_manager.synchronize(); // to get an accurate time measurement across ranks
      sw_communicate_spike_data.start();
#endif
      unsigned int* send_buffer_int =
        reinterpret_cast< unsigned int* >( &send_buffer[ 0 ] );
      unsigned int* recv_buffer_int =
        reinterpret_cast< unsigned int* >( &recv_buffer[ 0 ] );

      if ( off_grid_spiking_ )
      {
        kernel().mpi_manager.communicate_off_grid_spike_data_Alltoall( send_buffer_int,
                                                                       recv_buffer_int );
      }
      else
      {
        kernel().mpi_manager.communicate_spike_data_Alltoall( send_buffer_int,
                                                              recv_buffer_int );
      }
#ifndef DISABLE_TIMING
      sw_communicate_spike_data.stop();
      sw_deliver_spike_data.start();
#endif
    } // of omp single; implicit barrier

    // deliver spikes from receive buffer to ring buffers
    others_completed_tid = deliver_events_5g_( tid, recv_buffer );
#pragma omp atomic
    completed_count += others_completed_tid;

    // exit gather loop if all local threads and remote processes are
    // done
#pragma omp barrier

#ifndef DISABLE_TIMING
    if ( tid == 0 and kernel().mpi_manager.get_rank() < 30 )
    {
      sw_deliver_spike_data.stop();
    }
#endif

    if ( completed_count == max_completed_count )
    {
      done = true;
    }
    // otherwise, resize mpi buffers, if allowed
    else if ( kernel().mpi_manager.adaptive_spike_buffers() )
    {
#pragma omp single
      {
        buffer_size_spike_data_has_changed_ =
          kernel().mpi_manager.increase_buffer_size_spike_data();
      }
    }
#pragma omp barrier

  } // of while( true )

  reset_spike_register_5g_( tid );
}

template < typename TargetT, typename SpikeDataT >
bool
EventDeliveryManager::collocate_spike_data_buffers_( const thread tid,
  const AssignedRanks& assigned_ranks,
  SendBufferPosition& send_buffer_position,
  std::vector< std::vector< std::vector< std::vector< TargetT > > >* >&
    spike_register,
  std::vector< SpikeDataT >& send_buffer )
{
  // reset complete marker
  // TODO@5g: reset_complete_marker_( assigned_ranks, send_buffer_position, send_buffer ); -> Susi
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    send_buffer[ send_buffer_position.end( rank ) - 1 ].reset_marker();
  }

  // assume empty, changed to false if any entry found
  bool is_spike_register_empty = true;

  // first dimension: loop over writing thread
  for ( typename std::vector< std::
            vector< std::vector< std::vector< TargetT > > >* >::iterator it =
          spike_register.begin();
        it != spike_register.end();
        ++it )
  {
    // second dimension: fixed reading thread

    // third dimension: loop over lags
    for ( unsigned int lag = 0; lag < ( *( *it ) )[ tid ].size(); ++lag )
    {
      // fourth dimension: loop over entries
      for ( typename std::vector< TargetT >::iterator iiit =
              ( *( *it ) )[ tid ][ lag ].begin();
            iiit < ( *( *it ) )[ tid ][ lag ].end();
            ++iiit )
      {
        assert( not iiit->is_processed() );

        const thread rank = iiit->get_rank();

        if ( send_buffer_position.is_chunk_filled( rank ) )
        {
          is_spike_register_empty = false;
          if ( send_buffer_position.are_all_chunks_filled() )
          {
            return is_spike_register_empty;
          }
          else
          {
            continue;
          }
        }
        else
        {
          send_buffer[ send_buffer_position.idx( rank ) ].set(
            ( *iiit ).get_tid(),
            ( *iiit ).get_syn_id(),
            ( *iiit ).get_lcid(),
            lag,
            ( *iiit ).get_offset() );
          ( *iiit ).set_is_processed( true ); // mark entry for removal
          send_buffer_position.increase( rank );
        }
      }
    }
  }

  return is_spike_register_empty;
}

template < typename SpikeDataT >
void
EventDeliveryManager::set_end_and_invalid_markers_(
  const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer )
{
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank TODO@5g: [see above]
    if ( send_buffer_position.idx( rank )
      > send_buffer_position.begin( rank ) )
    {
      // set end marker at last position that contains a valid
      // entry. this could possibly be the last entry in this
      // chunk. since we call set_complete_marker_spike_data_ /after/
      // this function, the end marker would be replaced by a complete
      // marker. however, the effect of an end marker and a complete
      // marker /at the last postition in a chunk/ leads effectively
      // to the same behaviour: afer this entry, the first entry of
      // the next chunk is read, i.e., the next element in the buffer
      assert( send_buffer_position.idx( rank ) - 1
        < send_buffer_position.end( rank ) );
      send_buffer[ send_buffer_position.idx( rank ) - 1 ].set_end_marker();
    }
    else
    {
      assert( send_buffer_position.idx( rank )
        == send_buffer_position.begin( rank ) );
      send_buffer[ send_buffer_position.begin( rank ) ].set_invalid_marker();
    }
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::set_complete_marker_spike_data_(
  const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer )
{
  for ( thread target_rank = assigned_ranks.begin;
        target_rank < assigned_ranks.end;
        ++target_rank )
  {
    // use last entry for completion marker. for possible collision
    // with end marker, see comment in set_end_and_invalid_markers_
    const thread idx =
      send_buffer_position.end( target_rank ) - 1;
    send_buffer[ idx ].set_complete_marker();
  }
}

template < typename SpikeDataT >
bool
EventDeliveryManager::deliver_events_5g_( const thread tid,
  const std::vector< SpikeDataT >& recv_buffer )
{
#ifndef DISABLE_COUNTS
  ++call_count_deliver_events_5g[ tid ];
#endif
  const unsigned int send_recv_count_spike_data_per_rank = kernel().mpi_manager.get_send_recv_count_spike_data_per_rank();
  const std::vector< ConnectorModel* >& cm = kernel().model_manager.get_synapse_prototypes( tid );

  bool are_others_completed = true;

  // deliver only at end of time slice
  assert( kernel().simulation_manager.get_to_step() == kernel().connection_manager.get_min_delay() );

  SpikeEvent se;

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps(
    kernel().connection_manager.get_min_delay() );
  for ( size_t lag = 0;
        lag < ( size_t ) kernel().connection_manager.get_min_delay();
        ++lag )
  {
    prepared_timestamps[ lag ] =
      kernel().simulation_manager.get_clock() + Time::step( lag + 1 );
  }

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes();
        ++rank )
  {
    // check last entry for completed marker; needs to be done before
    // checking invalid marker to assure that this is always read
    if ( not recv_buffer[ ( rank + 1 ) * send_recv_count_spike_data_per_rank
               - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // continue with next rank if no spikes were sent by this rank
    if ( recv_buffer[ rank * send_recv_count_spike_data_per_rank ]
           .is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_spike_data_per_rank; ++i )
    {
      const SpikeDataT& spike_data =
        recv_buffer[ rank * send_recv_count_spike_data_per_rank + i ];

      if ( spike_data.get_tid() == tid )
      {
        se.set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
        se.set_offset( spike_data.get_offset() );
        kernel().connection_manager.send_5g(
          tid, spike_data.get_syn_id(), spike_data.get_lcid(), cm, se );
      }

      // break if this was the last valid entry from this rank
      if ( spike_data.is_end_marker() )
      {
        break;
      }
    }
  }

  return are_others_completed;
}

// TODO@5g: documentation
void
EventDeliveryManager::gather_target_data( const thread tid )
{
  assert( not kernel().connection_manager.is_source_table_cleared() );

#ifndef DISABLE_COUNTS
  if ( tid == 0 and kernel().mpi_manager.get_rank() < 30 )
  {
    ++comm_steps_target_data;
  }
#endif

  // when a thread does not have any more spike to collocate and when
  // it detects a remote MPI rank is finished this count is increased
  // by 1 in each case. only if all threads are done AND all threads
  // detect all remote ranks are done, we are allowed to stop
  // communication.
  unsigned int completed_count; // TODO@5g: use same strategy in
				// gather_spike data? could use same
				// completed_count_ vector -> Susi
  const unsigned int half_completed_count =
    kernel().vp_manager.get_num_threads();
  const unsigned int max_completed_count = 2 * half_completed_count;

  bool me_completed_tid;
  bool others_completed_tid;
  const AssignedRanks assigned_ranks =
    kernel().vp_manager.get_assigned_ranks( tid );

  kernel().connection_manager.prepare_target_table( tid );
  kernel().connection_manager.reset_source_table_entry_point( tid );

  // can not use while(true) and break in an omp structured block
  bool done = false;
  while ( not done )
  {
    completed_count_[ tid ] = 0;
#pragma omp single
    {
      if ( kernel().mpi_manager.adaptive_target_buffers()
        and buffer_size_target_data_has_changed_ )
      {
        resize_send_recv_buffers_target_data();
      }
    } // of omp single; implicit barrier
#ifndef DISABLE_TIMING
    if ( tid == 0 and kernel().mpi_manager.get_rank() < 30 )
    {
      sw_collocate_target_data.start();
    }
#endif

    kernel().connection_manager.restore_source_table_entry_point( tid );

    SendBufferPosition send_buffer_position(
      assigned_ranks, kernel().mpi_manager.get_send_recv_count_target_data_per_rank() );

    me_completed_tid = collocate_target_data_buffers_( tid, assigned_ranks, send_buffer_position );
    completed_count_[ tid ] += static_cast< unsigned int >( me_completed_tid );
#pragma omp barrier

    completed_count = std::accumulate( completed_count_.begin(), completed_count_.end(), 0 );

   if ( completed_count == half_completed_count )
    {
      set_complete_marker_target_data_( tid, assigned_ranks, send_buffer_position );
#pragma omp barrier
    }
    kernel().connection_manager.save_source_table_entry_point( tid );
#pragma omp barrier
    kernel().connection_manager.clean_source_table( tid );
#pragma omp single
    {
#ifndef DISABLE_COUNTS
      ++comm_rounds_target_data;
#endif
#ifndef DISABLE_TIMING
      sw_collocate_target_data.stop();
      kernel().mpi_manager.synchronize(); // to get an accurate time measurement across ranks
      sw_communicate_target_data.start();
#endif
      unsigned int* send_buffer_int =
        reinterpret_cast< unsigned int* >( &send_buffer_target_data_[ 0 ] );
      unsigned int* recv_buffer_int =
        reinterpret_cast< unsigned int* >( &recv_buffer_target_data_[ 0 ] );
      kernel().mpi_manager.communicate_target_data_Alltoall( send_buffer_int,
        recv_buffer_int );
#ifndef DISABLE_TIMING
      sw_communicate_target_data.stop();
      sw_distribute_target_data.start();
#endif
    } // of omp single

    others_completed_tid = distribute_target_data_buffers_( tid );
    completed_count_[ tid ] += static_cast< unsigned int >( others_completed_tid );
#pragma omp barrier

#ifndef DISABLE_TIMING
    if ( tid == 0 and kernel().mpi_manager.get_rank() < 30 )
    {
      sw_distribute_target_data.stop();
    }
#endif

    completed_count = std::accumulate( completed_count_.begin(), completed_count_.end(), 0 );

    if ( completed_count == max_completed_count )
    {
      done = true;
    }
    else if ( kernel().mpi_manager.adaptive_target_buffers() )
    {
#pragma omp single
      {
        buffer_size_target_data_has_changed_ =
          kernel().mpi_manager.increase_buffer_size_target_data();
      }
    }
#pragma omp barrier
  } // of while(true)

  kernel().connection_manager.clear_source_table( tid );
}

bool
EventDeliveryManager::collocate_target_data_buffers_(
  const thread tid,
  const AssignedRanks& assigned_ranks,
  SendBufferPosition& send_buffer_position )
{
  unsigned int num_target_data_written = 0;
  thread source_rank;
  TargetData next_target_data;
  bool valid_next_target_data;
  bool is_source_table_read = true;

  // no ranks to process for this thread
  if ( assigned_ranks.begin == assigned_ranks.end )
  {
    kernel().connection_manager.no_targets_to_process( tid );
    return is_source_table_read;
  }

  // reset markers
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // reset last entry to avoid accidentally communicating done
    // marker
    send_buffer_target_data_[ send_buffer_position.end( rank ) - 1 ].reset_marker();
    // set first entry to invalid to avoid accidentally reading
    // uninitialized parts of the receive buffer
    send_buffer_target_data_[ send_buffer_position.begin( rank ) ].set_invalid_marker();
  }

  while ( true )
  {
    valid_next_target_data =
      kernel().connection_manager.get_next_target_data( tid,
        assigned_ranks.begin,
        assigned_ranks.end,
        source_rank,
        next_target_data );
    if ( valid_next_target_data ) // add valid entry to MPI buffer
    {
      if ( send_buffer_position.idx( source_rank ) == send_buffer_position.end( source_rank ) )
      {
        // entry does not fit in this part of the MPI buffer any more,
        // so we need to reject it
        kernel().connection_manager.reject_last_target_data( tid );
        // after rejecting the last target, we need to save the
        // position to start at this point again next communication
        // round
        kernel().connection_manager.save_source_table_entry_point( tid );
        // we have just rejected an entry, so source table can not be
        // fully read
        is_source_table_read = false;
        if ( num_target_data_written
          == ( send_buffer_position.send_recv_count_per_rank
               * assigned_ranks.size ) ) // buffer is full
        {
          return is_source_table_read;
        }
        else
        {
          continue;
        }
      }
      else
      {
        send_buffer_target_data_[ send_buffer_position.idx( source_rank ) ] = next_target_data;
        send_buffer_position.increase( source_rank );
      }
    }
    else // all connections have been processed
    {
      // mark end of valid data for each rank
      for ( thread rank = assigned_ranks.begin;
            rank < assigned_ranks.end;
            ++rank )
      {
        if ( send_buffer_position.idx( rank ) > send_buffer_position.begin( rank ) )
        {
          send_buffer_target_data_[ send_buffer_position.idx( rank ) - 1 ].set_end_marker();
        }
        else
        {
          send_buffer_target_data_[ send_buffer_position.begin( rank ) ].set_invalid_marker();
        }
      }
      return is_source_table_read;
    } // of else
  }   // of while(true)
}

void
nest::EventDeliveryManager::set_complete_marker_target_data_( const thread tid,
  const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position )
{
  for ( thread rank = assigned_ranks.begin;
        rank < assigned_ranks.end;
        ++rank )
  {
    const thread idx = send_buffer_position.end( rank ) - 1;
    send_buffer_target_data_[ idx ].set_complete_marker();
  }
}

// TODO@5g: can we also use a receive_buffer_position, similar to the send_buffer_position during collocate?
bool
nest::EventDeliveryManager::distribute_target_data_buffers_( const thread tid )
{
  bool are_others_completed = true;
  const unsigned int send_recv_count_target_data_per_rank = kernel().mpi_manager.get_send_recv_count_target_data_per_rank();

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes();
        ++rank )
  {
    // check last entry for completed marker
    if ( not recv_buffer_target_data_[ ( rank + 1 ) * send_recv_count_target_data_per_rank - 1 ]
               .is_complete_marker() )
    {
      are_others_completed = false;
    }

    // were targets sent by this rank?
    if ( recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_target_data_per_rank; ++i )
    {
      const TargetData& target_data =
        recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank + i ];
      if ( target_data.get_source_tid() == tid )
      {
        kernel().connection_manager.add_target( tid, rank, target_data );
      }

      // is this the last target from this rank?
      if ( target_data.is_end_marker() )
      {
        break;
      }
    }
  }

  return are_others_completed;
}

} // of namespace nest
