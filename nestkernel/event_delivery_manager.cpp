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
#include <numeric>   // accumulate

// Includes from nestkernel:
#include "connection_manager.h"
#include "connection_manager_impl.h"
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "send_buffer_position.h"
#include "source.h"
#include "vp_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
EventDeliveryManager::EventDeliveryManager()
  : off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
  , emitted_spikes_register_()
  , off_grid_emitted_spike_register_()
  , send_buffer_secondary_events_()
  , recv_buffer_secondary_events_()
  , local_spike_counter_()
  , send_buffer_spike_data_()
  , recv_buffer_spike_data_()
  , send_buffer_off_grid_spike_data_()
  , recv_buffer_off_grid_spike_data_()
  , send_buffer_target_data_()
  , recv_buffer_target_data_()
  , buffer_size_target_data_has_changed_( false )
  , buffer_size_spike_data_has_changed_( false )
  , decrease_buffer_size_spike_data_( true )
  , gather_completed_checker_()
{
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
  reset_counters();
  reset_timers_for_preparation();
  reset_timers_for_dynamics();
  emitted_spikes_register_.resize( num_threads );
  off_grid_emitted_spike_register_.resize( num_threads );
  gather_completed_checker_.initialize( num_threads, false );
  // Ensures that ResetKernel resets off_grid_spiking_
  off_grid_spiking_ = false;
  buffer_size_target_data_has_changed_ = false;
  buffer_size_spike_data_has_changed_ = false;
  decrease_buffer_size_spike_data_ = true;

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    emitted_spikes_register_[ tid ].resize( num_threads,
      std::vector< std::vector< Target > >( kernel().connection_manager.get_min_delay(), std::vector< Target >() ) );

    off_grid_emitted_spike_register_[ tid ].resize( num_threads,
      std::vector< std::vector< OffGridTarget > >(
        kernel().connection_manager.get_min_delay(), std::vector< OffGridTarget >() ) );
  } // of omp parallel
}

void
EventDeliveryManager::finalize()
{
  // clear the spike buffers
  std::vector< std::vector< std::vector< std::vector< Target > > > >().swap( emitted_spikes_register_ );
  std::vector< std::vector< std::vector< std::vector< OffGridTarget > > > >().swap( off_grid_emitted_spike_register_ );

  send_buffer_secondary_events_.clear();
  recv_buffer_secondary_events_.clear();
  send_buffer_spike_data_.clear();
  recv_buffer_spike_data_.clear();
  send_buffer_off_grid_spike_data_.clear();
  recv_buffer_off_grid_spike_data_.clear();
}

void
EventDeliveryManager::change_number_of_threads()
{
  finalize();
  initialize();
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
  def< unsigned long >(
    dict, names::local_spike_counter, std::accumulate( local_spike_counter_.begin(), local_spike_counter_.end(), 0 ) );

#ifdef TIMER_DETAILED
  def< double >( dict, names::time_collocate_spike_data, sw_collocate_spike_data_.elapsed() );
  def< double >( dict, names::time_communicate_spike_data, sw_communicate_spike_data_.elapsed() );
  def< double >( dict, names::time_deliver_spike_data, sw_deliver_spike_data_.elapsed() );
  def< double >( dict, names::time_communicate_target_data, sw_communicate_target_data_.elapsed() );
#endif
}

void
EventDeliveryManager::resize_send_recv_buffers_target_data()
{
  // compute send receive counts and allocate memory for buffers
  send_buffer_target_data_.resize( kernel().mpi_manager.get_buffer_size_target_data() );
  recv_buffer_target_data_.resize( kernel().mpi_manager.get_buffer_size_target_data() );
}

void
EventDeliveryManager::resize_send_recv_buffers_spike_data_()
{
  if ( kernel().mpi_manager.get_buffer_size_spike_data() > send_buffer_spike_data_.size() )
  {
    send_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
    recv_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
    send_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
    recv_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
  }
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
    reset_spike_register_( tid );
    resize_spike_register_( tid );
  }
}

void
EventDeliveryManager::configure_secondary_buffers()
{
  send_buffer_secondary_events_.clear();
  send_buffer_secondary_events_.resize( kernel().mpi_manager.get_send_buffer_size_secondary_events_in_int() );
  recv_buffer_secondary_events_.clear();
  recv_buffer_secondary_events_.resize( kernel().mpi_manager.get_recv_buffer_size_secondary_events_in_int() );
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
    moduli_[ d ] = ( kernel().simulation_manager.get_clock().get_steps() + d ) % ( min_delay + max_delay );
  }

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast< size_t >( std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  slice_moduli_.resize( min_delay + max_delay );
  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel().simulation_manager.get_clock().get_steps() + d ) / min_delay ) % nbuff;
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
  assert( moduli_.size() == ( index ) ( min_delay + max_delay ) );
  std::rotate( moduli_.begin(), moduli_.begin() + min_delay, moduli_.end() );

  /*
   For the slice-based ring buffer, we cannot rotate the table, but
   have to re-compute it, since max_delay_ may not be a multiple of
   min_delay_.  Reference time is the time at the beginning of the slice.
   */
  const size_t nbuff = static_cast< size_t >( std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel().simulation_manager.get_clock().get_steps() + d ) / min_delay ) % nbuff;
  }
}

void
EventDeliveryManager::reset_counters()
{
  for ( auto& spike_counter : local_spike_counter_ )
  {
    spike_counter = 0;
  }
}

void
EventDeliveryManager::reset_timers_for_preparation()
{
#ifdef TIMER_DETAILED
  sw_communicate_target_data_.reset();
#endif
}

void
EventDeliveryManager::reset_timers_for_dynamics()
{
#ifdef TIMER_DETAILED
  sw_collocate_spike_data_.reset();
  sw_communicate_spike_data_.reset();
  sw_deliver_spike_data_.reset();
#endif
}

void
EventDeliveryManager::write_done_marker_secondary_events_( const bool done )
{
  // write done marker at last position in every chunk
  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    send_buffer_secondary_events_[ kernel().mpi_manager.get_done_marker_position_in_secondary_events_send_buffer(
      rank ) ] = done;
  }
}

void
EventDeliveryManager::gather_secondary_events( const bool done )
{
  write_done_marker_secondary_events_( done );
  kernel().mpi_manager.communicate_secondary_events_Alltoallv(
    send_buffer_secondary_events_, recv_buffer_secondary_events_ );
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
    gather_spike_data_( tid, send_buffer_off_grid_spike_data_, recv_buffer_off_grid_spike_data_ );
  }
  else
  {
    gather_spike_data_( tid, send_buffer_spike_data_, recv_buffer_spike_data_ );
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::gather_spike_data_( const thread tid,
  std::vector< SpikeDataT >& send_buffer,
  std::vector< SpikeDataT >& recv_buffer )
{
  // Assume all threads have some work to do
  gather_completed_checker_[ tid ].set_false();
  assert( gather_completed_checker_.all_false() );

  const AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  // Assume a single gather round
#pragma omp single
  {
    decrease_buffer_size_spike_data_ = true;
  }

  while ( gather_completed_checker_.any_false() )
  {
    // Assume this is the last gather round and change to false
    // otherwise
    gather_completed_checker_[ tid ].set_true();

#pragma omp single
    {
      if ( kernel().mpi_manager.adaptive_spike_buffers() and buffer_size_spike_data_has_changed_ )
      {
        resize_send_recv_buffers_spike_data_();
        buffer_size_spike_data_has_changed_ = false;
      }
    } // of omp single; implicit barrier
#ifdef TIMER_DETAILED
    if ( tid == 0 )
    {
      sw_collocate_spike_data_.start();
    }
#endif

    // Need to get new positions in case buffer size has changed
    SendBufferPosition send_buffer_position(
      assigned_ranks, kernel().mpi_manager.get_send_recv_count_spike_data_per_rank() );

    // Collocate spikes to send buffer
    const bool collocate_completed =
      collocate_spike_data_buffers_( tid, assigned_ranks, send_buffer_position, emitted_spikes_register_, send_buffer );
    gather_completed_checker_[ tid ].logical_and( collocate_completed );

    if ( off_grid_spiking_ )
    {
      const bool collocate_completed_off_grid = collocate_spike_data_buffers_(
        tid, assigned_ranks, send_buffer_position, off_grid_emitted_spike_register_, send_buffer );
      gather_completed_checker_[ tid ].logical_and( collocate_completed_off_grid );
    }

#pragma omp barrier
    // Set markers to signal end of valid spikes, and remove spikes
    // from register that have been collected in send buffer.
    set_end_and_invalid_markers_( assigned_ranks, send_buffer_position, send_buffer );
    clean_spike_register_( tid );

    // If we do not have any spikes left, set corresponding marker in
    // send buffer.
    if ( gather_completed_checker_.all_true() )
    {
      // Needs to be called /after/ set_end_and_invalid_markers_.
      set_complete_marker_spike_data_( assigned_ranks, send_buffer_position, send_buffer );
#pragma omp barrier
    }

#ifdef TIMER_DETAILED
    if ( tid == 0 )
    {
      sw_collocate_spike_data_.stop();
      sw_communicate_spike_data_.start();
    }
#endif

// Communicate spikes using a single thread.
#pragma omp single
    {
      if ( off_grid_spiking_ )
      {
        kernel().mpi_manager.communicate_off_grid_spike_data_Alltoall( send_buffer, recv_buffer );
      }
      else
      {
        kernel().mpi_manager.communicate_spike_data_Alltoall( send_buffer, recv_buffer );
      }
    } // of omp single; implicit barrier

#ifdef TIMER_DETAILED
    if ( tid == 0 )
    {
      sw_communicate_spike_data_.stop();
      sw_deliver_spike_data_.start();
    }
#endif

    // Deliver spikes from receive buffer to ring buffers.
    const bool deliver_completed = deliver_events_( tid, recv_buffer );
    gather_completed_checker_[ tid ].logical_and( deliver_completed );

#ifdef TIMER_DETAILED
    if ( tid == 0 )
    {
      sw_deliver_spike_data_.stop();
    }
#endif

    // Resize mpi buffers, if necessary and allowed.
    if ( gather_completed_checker_.any_false() and kernel().mpi_manager.adaptive_spike_buffers() )
    {
#pragma omp single
      {
        buffer_size_spike_data_has_changed_ = kernel().mpi_manager.increase_buffer_size_spike_data();
        decrease_buffer_size_spike_data_ = false;
      }
    }

  } // of while

#pragma omp single
  {
    if ( decrease_buffer_size_spike_data_ and kernel().mpi_manager.adaptive_spike_buffers() )
    {
      kernel().mpi_manager.decrease_buffer_size_spike_data();
    }
  } // of omp single; implicit barrier

  reset_spike_register_( tid );
}

template < typename TargetT, typename SpikeDataT >
bool
EventDeliveryManager::collocate_spike_data_buffers_( const thread tid,
  const AssignedRanks& assigned_ranks,
  SendBufferPosition& send_buffer_position,
  std::vector< std::vector< std::vector< std::vector< TargetT > > > >& emitted_spikes_register,
  std::vector< SpikeDataT >& send_buffer )
{
  reset_complete_marker_spike_data_( assigned_ranks, send_buffer_position, send_buffer );

  // Assume register is empty, will change to false if any entry can
  // not be fit into the MPI buffer.
  bool is_spike_register_empty = true;

  // First dimension: loop over writing thread
  for ( auto& emitted_spikes_per_thread : emitted_spikes_register )
  {
    // Second dimension: Set the reading thread to the current running thread

    // Third dimension: loop over lags
    for ( unsigned int lag = 0; lag < ( emitted_spikes_per_thread )[ tid ].size(); ++lag )
    {
      // Fourth dimension: loop over entries
      for ( auto& emitted_spike : ( emitted_spikes_per_thread )[ tid ][ lag ] )
      {
        assert( not emitted_spike.is_processed() );

        const thread rank = emitted_spike.get_rank();

        if ( send_buffer_position.is_chunk_filled( rank ) )
        {
          is_spike_register_empty = false;
          if ( send_buffer_position.are_all_chunks_filled() )
          {
            return is_spike_register_empty;
          }
        }
        else
        {
          send_buffer[ send_buffer_position.idx( rank ) ].set( emitted_spike, lag );
          emitted_spike.mark_for_removal();
          send_buffer_position.increase( rank );
        }
      }
    }
  }

  return is_spike_register_empty;
}

template < typename SpikeDataT >
void
EventDeliveryManager::set_end_and_invalid_markers_( const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer )
{
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    if ( send_buffer_position.idx( rank ) > send_buffer_position.begin( rank ) )
    {
      // Set end marker at last position that contains a valid
      // entry. This could possibly be the last entry in this
      // chunk. Since we call set_complete_marker_spike_data_ /after/
      // this function, the end marker would be replaced by a complete
      // marker. However, the effect of an end marker and a complete
      // marker /at the last position in a chunk/ leads effectively
      // to the same behavior: after this entry, the first entry of
      // the next chunk is read, i.e., the next element in the buffer.
      assert( send_buffer_position.idx( rank ) - 1 < send_buffer_position.end( rank ) );
      send_buffer[ send_buffer_position.idx( rank ) - 1 ].set_end_marker();
    }
    else
    {
      assert( send_buffer_position.idx( rank ) == send_buffer_position.begin( rank ) );
      send_buffer[ send_buffer_position.begin( rank ) ].set_invalid_marker();
    }
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::reset_complete_marker_spike_data_( const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer ) const
{
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    const thread idx = send_buffer_position.end( rank ) - 1;
    send_buffer[ idx ].reset_marker();
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::set_complete_marker_spike_data_( const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer ) const
{
  for ( thread target_rank = assigned_ranks.begin; target_rank < assigned_ranks.end; ++target_rank )
  {
    // Use last entry for completion marker. For possible collision
    // with end marker, see comment in set_end_and_invalid_markers_.
    const thread idx = send_buffer_position.end( target_rank ) - 1;
    send_buffer[ idx ].set_complete_marker();
  }
}

template < typename SpikeDataT >
bool
EventDeliveryManager::deliver_events_( const thread tid, const std::vector< SpikeDataT >& recv_buffer )
{
  const unsigned int send_recv_count_spike_data_per_rank =
    kernel().mpi_manager.get_send_recv_count_spike_data_per_rank();
  const std::vector< ConnectorModel* >& cm = kernel().model_manager.get_connection_models( tid );

  bool are_others_completed = true;

  // deliver only at end of time slice
  assert( kernel().simulation_manager.get_to_step() == kernel().connection_manager.get_min_delay() );

  SpikeEvent se;

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps( kernel().connection_manager.get_min_delay() );
  for ( size_t lag = 0; lag < static_cast< size_t >( kernel().connection_manager.get_min_delay() ); ++lag )
  {
    prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() + Time::step( lag + 1 );
  }

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    // check last entry for completed marker; needs to be done before
    // checking invalid marker to assure that this is always read
    if ( not recv_buffer[ ( rank + 1 ) * send_recv_count_spike_data_per_rank - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // continue with next rank if no spikes were sent by this rank
    if ( recv_buffer[ rank * send_recv_count_spike_data_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_spike_data_per_rank; ++i )
    {
      const SpikeDataT& spike_data = recv_buffer[ rank * send_recv_count_spike_data_per_rank + i ];

      se.set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
      se.set_offset( spike_data.get_offset() );

      if ( not kernel().connection_manager.use_compressed_spikes() )
      {
        if ( spike_data.get_tid() == tid )
        {
          const index syn_id = spike_data.get_syn_id();
          const index lcid = spike_data.get_lcid();

          // non-local sender -> receiver retrieves ID of sender Node from SourceTable based on tid, syn_id, lcid
          // only if needed, as this is computationally costly
          se.set_sender_node_id_info( tid, syn_id, lcid );
          kernel().connection_manager.send( tid, syn_id, lcid, cm, se );
        }
      }
      else
      {
        const index syn_id = spike_data.get_syn_id();
        // for compressed spikes lcid holds the index in the
        // compressed_spike_data structure
        const index idx = spike_data.get_lcid();
        const std::vector< SpikeData >& compressed_spike_data =
          kernel().connection_manager.get_compressed_spike_data( syn_id, idx );
        for ( auto it = compressed_spike_data.cbegin(); it != compressed_spike_data.cend(); ++it )
        {
          if ( it->get_tid() == tid )
          {
            const index lcid = it->get_lcid();

            // non-local sender -> receiver retrieves ID of sender Node from SourceTable based on tid, syn_id, lcid
            // only if needed, as this is computationally costly
            se.set_sender_node_id_info( tid, syn_id, lcid );
            kernel().connection_manager.send( tid, syn_id, lcid, cm, se );
          }
        }
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

void
EventDeliveryManager::gather_target_data( const thread tid )
{
  assert( not kernel().connection_manager.is_source_table_cleared() );

  // assume all threads have some work to do
  gather_completed_checker_[ tid ].set_false();
  assert( gather_completed_checker_.all_false() );

  const AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  kernel().connection_manager.prepare_target_table( tid );
  kernel().connection_manager.reset_source_table_entry_point( tid );

  while ( gather_completed_checker_.any_false() )
  {
    // assume this is the last gather round and change to false
    // otherwise
    gather_completed_checker_[ tid ].set_true();

#pragma omp single
    {
      if ( kernel().mpi_manager.adaptive_target_buffers() and buffer_size_target_data_has_changed_ )
      {
        resize_send_recv_buffers_target_data();
      }
    } // of omp single; implicit barrier

    kernel().connection_manager.restore_source_table_entry_point( tid );

    SendBufferPosition send_buffer_position(
      assigned_ranks, kernel().mpi_manager.get_send_recv_count_target_data_per_rank() );

    const bool gather_completed = collocate_target_data_buffers_( tid, assigned_ranks, send_buffer_position );
    gather_completed_checker_[ tid ].logical_and( gather_completed );

    if ( gather_completed_checker_.all_true() )
    {
      set_complete_marker_target_data_( assigned_ranks, send_buffer_position );
    }
    kernel().connection_manager.save_source_table_entry_point( tid );
#pragma omp barrier
    kernel().connection_manager.clean_source_table( tid );

#pragma omp single
    {
#ifdef TIMER_DETAILED
      sw_communicate_target_data_.start();
#endif
      kernel().mpi_manager.communicate_target_data_Alltoall( send_buffer_target_data_, recv_buffer_target_data_ );
#ifdef TIMER_DETAILED
      sw_communicate_target_data_.stop();
#endif
    } // of omp single (implicit barrier)


    const bool distribute_completed = distribute_target_data_buffers_( tid );
    gather_completed_checker_[ tid ].logical_and( distribute_completed );

    // resize mpi buffers, if necessary and allowed
    if ( gather_completed_checker_.any_false() and kernel().mpi_manager.adaptive_target_buffers() )
    {
#pragma omp single
      {
        buffer_size_target_data_has_changed_ = kernel().mpi_manager.increase_buffer_size_target_data();
      }
    }
  } // of while

  kernel().connection_manager.clear_source_table( tid );
}

bool
EventDeliveryManager::collocate_target_data_buffers_( const thread tid,
  const AssignedRanks& assigned_ranks,
  SendBufferPosition& send_buffer_position )
{
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
    valid_next_target_data = kernel().connection_manager.get_next_target_data(
      tid, assigned_ranks.begin, assigned_ranks.end, source_rank, next_target_data );
    if ( valid_next_target_data ) // add valid entry to MPI buffer
    {
      if ( send_buffer_position.is_chunk_filled( source_rank ) )
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
        if ( send_buffer_position.are_all_chunks_filled() ) // buffer is full
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
      for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
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
nest::EventDeliveryManager::set_complete_marker_target_data_( const AssignedRanks& assigned_ranks,
  const SendBufferPosition& send_buffer_position )
{
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    const thread idx = send_buffer_position.end( rank ) - 1;
    send_buffer_target_data_[ idx ].set_complete_marker();
  }
}

bool
nest::EventDeliveryManager::distribute_target_data_buffers_( const thread tid )
{
  bool are_others_completed = true;
  const unsigned int send_recv_count_target_data_per_rank =
    kernel().mpi_manager.get_send_recv_count_target_data_per_rank();

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    // Check last entry for completed marker
    if ( not recv_buffer_target_data_[ ( rank + 1 ) * send_recv_count_target_data_per_rank - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // Were targets sent by this rank?
    if ( recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_target_data_per_rank; ++i )
    {
      const TargetData& target_data = recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank + i ];
      if ( target_data.get_source_tid() == tid )
      {
        kernel().connection_manager.add_target( tid, rank, target_data );
      }

      // Is this the last target from this rank?
      if ( target_data.is_end_marker() )
      {
        break;
      }
    }
  }

  return are_others_completed;
}

void
EventDeliveryManager::resize_spike_register_( const thread tid )
{
  for ( auto& emitted_spikes_for_current_thread : emitted_spikes_register_[ tid ] )
  {
    emitted_spikes_for_current_thread.resize( kernel().connection_manager.get_min_delay(), std::vector< Target >() );
  }

  for ( auto& off_grid_emitted_spike_for_current_thread : off_grid_emitted_spike_register_[ tid ] )
  {
    off_grid_emitted_spike_for_current_thread.resize(
      kernel().connection_manager.get_min_delay(), std::vector< OffGridTarget >() );
  }
}

} // of namespace nest
