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

#include "event_delivery_manager_impl.h"

// C++ includes:
#include <algorithm> // rotate
#include <numeric>   // accumulate

// Includes from nestkernel:
#include "connection_manager.h"
#include "kernel_manager.h"
#include "model_manager.h"
#include "mpi_manager_impl.h"
#include "send_buffer_position.h"
#include "simulation_manager.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{


EventDeliveryManager::EventDeliveryManager()
  : off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
  , emitted_spikes_register_()
  , off_grid_emitted_spikes_register_()
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
  , global_max_spikes_per_rank_( 0 )
  , send_recv_buffer_shrink_limit_( 0.2 )
  , send_recv_buffer_shrink_spare_( 0.1 )
  , send_recv_buffer_grow_extra_( 0.5 )
  , send_recv_buffer_resize_log_()
  , gather_completed_checker_()
{
}

EventDeliveryManager::~EventDeliveryManager()
{
}

void
EventDeliveryManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    init_moduli();
    reset_timers_for_preparation();
    reset_timers_for_dynamics();

    // Ensures that ResetKernel resets off_grid_spiking_
    off_grid_spiking_ = false;
    buffer_size_target_data_has_changed_ = false;
    send_recv_buffer_shrink_limit_ = 0.2;
    send_recv_buffer_shrink_spare_ = 0.1;
    send_recv_buffer_grow_extra_ = 0.5;
    send_recv_buffer_resize_log_.clear();
  }

  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();

  local_spike_counter_.resize( num_threads, 0 );
  reset_counters();
  emitted_spikes_register_.resize( num_threads );
  off_grid_emitted_spikes_register_.resize( num_threads );
  gather_completed_checker_.initialize( num_threads, false );

#pragma omp parallel
  {
    const size_t tid = kernel::manager< VPManager >.get_thread_id();

    if ( not emitted_spikes_register_[ tid ] )
    {
      emitted_spikes_register_[ tid ] = new std::vector< SpikeDataWithRank >();
    }

    if ( not off_grid_emitted_spikes_register_[ tid ] )
    {
      off_grid_emitted_spikes_register_[ tid ] = new std::vector< OffGridSpikeDataWithRank >();
    }
  } // of omp parallel
}

void
EventDeliveryManager::finalize( const bool )
{
  // clear the spike buffers
  for ( auto& vec_spikedata_ptr : emitted_spikes_register_ )
  {
    delete vec_spikedata_ptr;
  }
  emitted_spikes_register_.clear(); // remove stale pointers

  for ( auto& vec_spikedata_ptr : off_grid_emitted_spikes_register_ )
  {
    delete vec_spikedata_ptr;
  }
  off_grid_emitted_spikes_register_.clear();

  send_buffer_secondary_events_.clear();
  recv_buffer_secondary_events_.clear();
  send_buffer_spike_data_.clear();
  recv_buffer_spike_data_.clear();
  send_buffer_off_grid_spike_data_.clear();
  recv_buffer_off_grid_spike_data_.clear();
}

void
EventDeliveryManager::set_status( const DictionaryDatum& dict )
{
  updateValue< bool >( dict, names::off_grid_spiking, off_grid_spiking_ );

  double bsl = send_recv_buffer_shrink_limit_;
  if ( updateValue< double >( dict, names::spike_buffer_shrink_limit, bsl ) )
  {
    if ( bsl < 0 )
    {
      throw BadProperty( "buffer_shrink_limit >= 0 required." );
    }
    send_recv_buffer_shrink_limit_ = bsl;
  }

  double bss = send_recv_buffer_shrink_spare_;
  if ( updateValue< double >( dict, names::spike_buffer_shrink_spare, bss ) )
  {
    if ( bss < 0 or bss > 1 )
    {
      throw BadProperty( "0 <= buffer_shrink_spare <= 1 required." );
    }
    send_recv_buffer_shrink_spare_ = bss;
  }

  double bge = send_recv_buffer_grow_extra_;
  if ( updateValue< double >( dict, names::spike_buffer_grow_extra, bge ) )
  {
    if ( bge < 0 )
    {
      throw BadProperty( "buffer_grow_extra >= 0 required." );
    }
    send_recv_buffer_grow_extra_ = bge;
  }
}

void
EventDeliveryManager::get_status( DictionaryDatum& dict )
{
  def< bool >( dict, names::off_grid_spiking, off_grid_spiking_ );
  def< unsigned long >(
    dict, names::local_spike_counter, std::accumulate( local_spike_counter_.begin(), local_spike_counter_.end(), 0 ) );
  def< double >( dict, names::spike_buffer_shrink_limit, send_recv_buffer_shrink_limit_ );
  def< double >( dict, names::spike_buffer_shrink_spare, send_recv_buffer_shrink_spare_ );
  def< double >( dict, names::spike_buffer_grow_extra, send_recv_buffer_grow_extra_ );

  DictionaryDatum log_events = DictionaryDatum( new Dictionary );
  ( *dict )[ names::spike_buffer_resize_log ] = log_events;
  send_recv_buffer_resize_log_.to_dict( log_events );

  sw_collocate_spike_data_.get_status( dict, names::time_collocate_spike_data, names::time_collocate_spike_data_cpu );
  sw_communicate_spike_data_.get_status(
    dict, names::time_communicate_spike_data, names::time_communicate_spike_data_cpu );
  sw_communicate_target_data_.get_status(
    dict, names::time_communicate_target_data, names::time_communicate_target_data_cpu );
}

void
EventDeliveryManager::resize_send_recv_buffers_target_data()
{
  // compute send receive counts and allocate memory for buffers
  send_buffer_target_data_.resize( kernel::manager< MPIManager >.get_buffer_size_target_data() );
  recv_buffer_target_data_.resize( kernel::manager< MPIManager >.get_buffer_size_target_data() );
}

void
EventDeliveryManager::resize_send_recv_buffers_spike_data_()
{
  if ( kernel::manager< MPIManager >.get_buffer_size_spike_data() > send_buffer_spike_data_.size() )
  {
    send_buffer_spike_data_.resize( kernel::manager< MPIManager >.get_buffer_size_spike_data() );
    recv_buffer_spike_data_.resize( kernel::manager< MPIManager >.get_buffer_size_spike_data() );
    send_buffer_off_grid_spike_data_.resize( kernel::manager< MPIManager >.get_buffer_size_spike_data() );
    recv_buffer_off_grid_spike_data_.resize( kernel::manager< MPIManager >.get_buffer_size_spike_data() );
  }
}

void
EventDeliveryManager::configure_spike_data_buffers()
{
  assert( kernel::manager< ConnectionManager >.get_min_delay() != 0 );

  configure_spike_register();

  send_buffer_spike_data_.clear();
  send_buffer_off_grid_spike_data_.clear();

  resize_send_recv_buffers_spike_data_();
}

void
EventDeliveryManager::configure_spike_register()
{
#pragma omp parallel
  {
    const size_t tid = kernel::manager< VPManager >.get_thread_id();
    reset_spike_register_( tid );
  }
}

void
EventDeliveryManager::configure_secondary_buffers()
{
  send_buffer_secondary_events_.clear();
  send_buffer_secondary_events_.resize( kernel::manager< MPIManager >.get_send_buffer_size_secondary_events_in_int() );
  recv_buffer_secondary_events_.clear();
  recv_buffer_secondary_events_.resize( kernel::manager< MPIManager >.get_recv_buffer_size_secondary_events_in_int() );
}

void
EventDeliveryManager::init_moduli()
{
  long min_delay = kernel::manager< ConnectionManager >.get_min_delay();
  long max_delay = kernel::manager< ConnectionManager >.get_max_delay();
  assert( min_delay != 0 );
  assert( max_delay != 0 );

  // Ring buffers use modulos to determine where to store incoming events
  // with given time stamps, relative to the beginning of the slice in which
  // the spikes are delivered from the queue, ie, the slice after the one
  // in which they were generated. The pertaining offsets are 0..max_delay-1.
  moduli_.resize( min_delay + max_delay );

  for ( long d = 0; d < min_delay + max_delay; ++d )
  {
    moduli_[ d ] = ( kernel::manager< SimulationManager >.get_clock().get_steps() + d ) % ( min_delay + max_delay );
  }

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast< size_t >( std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  slice_moduli_.resize( min_delay + max_delay );
  for ( long d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel::manager< SimulationManager >.get_clock().get_steps() + d ) / min_delay ) % nbuff;
  }
}

void
EventDeliveryManager::update_moduli()
{
  long min_delay = kernel::manager< ConnectionManager >.get_min_delay();
  long max_delay = kernel::manager< ConnectionManager >.get_max_delay();
  assert( min_delay != 0 );
  assert( max_delay != 0 );

  /*
   * Note that for updating the modulos, it is sufficient
   * to rotate the buffer to the left.
   */
  assert( moduli_.size() == ( size_t ) ( min_delay + max_delay ) );
  std::rotate( moduli_.begin(), moduli_.begin() + min_delay, moduli_.end() );

  // For the slice-based ring buffer, we cannot rotate the table, but
  // have to re-compute it, since max_delay_ may not be a multiple of
  // min_delay_.  Reference time is the time at the beginning of the slice.
  const size_t nbuff = static_cast< size_t >( std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  for ( long d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] = ( ( kernel::manager< SimulationManager >.get_clock().get_steps() + d ) / min_delay ) % nbuff;
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
  sw_communicate_target_data_.reset();
}

void
EventDeliveryManager::reset_timers_for_dynamics()
{
  sw_collocate_spike_data_.reset();
  sw_communicate_spike_data_.reset();
}

void
EventDeliveryManager::write_done_marker_secondary_events_( const bool done )
{
  // write done marker at last position in every chunk
  for ( size_t rank = 0; rank < kernel::manager< MPIManager >.get_num_processes(); ++rank )
  {
    send_buffer_secondary_events_
      [ kernel::manager< MPIManager >.get_done_marker_position_in_secondary_events_send_buffer( rank ) ] = done;
  }
}

void
EventDeliveryManager::gather_secondary_events( const bool done )
{
  write_done_marker_secondary_events_( done );
  kernel::manager< MPIManager >.communicate_secondary_events_Alltoallv(
    send_buffer_secondary_events_, recv_buffer_secondary_events_ );
}

bool
EventDeliveryManager::deliver_secondary_events( const size_t tid, const bool called_from_wfr_update )
{
  return kernel::manager< ConnectionManager >.deliver_secondary_events(
    tid, called_from_wfr_update, recv_buffer_secondary_events_ );
}

void
EventDeliveryManager::gather_spike_data()
{
  if ( off_grid_spiking_ )
  {
    gather_spike_data_( send_buffer_off_grid_spike_data_, recv_buffer_off_grid_spike_data_ );
  }
  else
  {
    gather_spike_data_( send_buffer_spike_data_, recv_buffer_spike_data_ );
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::gather_spike_data_( std::vector< SpikeDataT >& send_buffer,
  std::vector< SpikeDataT >& recv_buffer )
{
  // NOTE: For meaning and logic of SpikeData flags for detecting complete transmission
  //       and information for shrink/grow, see comment in spike_data.h.

  const size_t old_buff_size_per_rank = kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();

  if ( global_max_spikes_per_rank_ < send_recv_buffer_shrink_limit_ * old_buff_size_per_rank )
  {
    const size_t new_buff_size_per_rank =
      std::max( 2UL, static_cast< size_t >( ( 1 + send_recv_buffer_shrink_spare_ ) * global_max_spikes_per_rank_ ) );
    kernel::manager< MPIManager >.set_buffer_size_spike_data(
      kernel::manager< MPIManager >.get_num_processes() * new_buff_size_per_rank );
    resize_send_recv_buffers_spike_data_();
    send_recv_buffer_resize_log_.add_entry( global_max_spikes_per_rank_, new_buff_size_per_rank );
  }

  /* The following do-while loop is executed
   * - once if all spikes fit into current send buffers on all ranks
   * - twice if send buffer size needs to be increased to fit in all spikes
   */
  bool all_spikes_transmitted = false;
  do
  {
    // Need to get new positions in case buffer size has changed
    SendBufferPosition send_buffer_position;

    sw_collocate_spike_data_.start();

    // Set marker at end of each chunk to DEFAULT
    reset_complete_marker_spike_data_( send_buffer_position, send_buffer );
    std::vector< size_t > num_spikes_per_rank( kernel::manager< MPIManager >.get_num_processes(), 0 );

    // Collocate spikes to send buffer
    collocate_spike_data_buffers_( send_buffer_position, emitted_spikes_register_, send_buffer, num_spikes_per_rank );

    if ( off_grid_spiking_ )
    {
      collocate_spike_data_buffers_(
        send_buffer_position, off_grid_emitted_spikes_register_, send_buffer, num_spikes_per_rank );
    }

    // Largest number of spikes sent from this rank to any other rank.
    const auto local_max_spikes_per_rank = *std::max_element( num_spikes_per_rank.begin(), num_spikes_per_rank.end() );

    // At this point, all send_buffer entries with spikes to be transmitted, as well
    // as all chunk-end entries, have marker DEFAULT.
    set_end_marker_( send_buffer_position, send_buffer, local_max_spikes_per_rank );

    sw_collocate_spike_data_.stop();
    sw_communicate_spike_data_.start();
#ifdef MPI_SYNC_TIMER
    // We introduce an explicit barrier at this point to measure how long each process idles until all other processes
    // reached this point as well. This barrier is directly followed by another implicit barrier due to global
    // communication.
    kernel::manager< SimulationManager >.get_mpi_synchronization_stopwatch().start();
    kernel::manager< MPIManager >.synchronize();
    kernel::manager< SimulationManager >.get_mpi_synchronization_stopwatch().stop();
#endif

    // Given that we templatize by plain vs offgrid, this if should not be necessary, but ...
    if ( off_grid_spiking_ )
    {
      kernel::manager< MPIManager >.communicate_off_grid_spike_data_Alltoall( send_buffer, recv_buffer );
    }
    else
    {
      kernel::manager< MPIManager >.communicate_spike_data_Alltoall( send_buffer, recv_buffer );
    }

    sw_communicate_spike_data_.stop();

    global_max_spikes_per_rank_ = get_global_max_spikes_per_rank_( send_buffer_position, recv_buffer );

    all_spikes_transmitted =
      global_max_spikes_per_rank_ <= kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();

    if ( not all_spikes_transmitted )
    {
      const size_t new_buff_size_per_rank =
        static_cast< size_t >( ( 1 + send_recv_buffer_grow_extra_ ) * global_max_spikes_per_rank_ );

      kernel::manager< MPIManager >.set_buffer_size_spike_data(
        kernel::manager< MPIManager >.get_num_processes() * new_buff_size_per_rank );
      resize_send_recv_buffers_spike_data_();
      send_recv_buffer_resize_log_.add_entry( global_max_spikes_per_rank_, new_buff_size_per_rank );
    }

  } while ( not all_spikes_transmitted );

  // We cannot shrink buffers here, because they first need to be read out by
  // deliver events. Shrinking will happen at beginning of next gather.

  /* emitted_spike_register is cleared by deliver_events in a thread-parallel context.
     We could in principle clear it here, but since it can conveniently be done thread-parallel,
     it is best to postpone.
   */
}

template < typename SpikeDataWithRankT, typename SpikeDataT >
void
EventDeliveryManager::collocate_spike_data_buffers_( SendBufferPosition& send_buffer_position,
  std::vector< std::vector< SpikeDataWithRankT >* >& emitted_spikes_register,
  std::vector< SpikeDataT >& send_buffer,
  std::vector< size_t >& num_spikes_per_rank )
{
  // First dimension: loop over writing thread
  for ( auto& emitted_spikes_per_thread : emitted_spikes_register )
  {
    // Second dimension: loop over entries
    for ( auto& emitted_spike : *emitted_spikes_per_thread )
    {
      const size_t rank = emitted_spike.rank;

      // We need to count here even though send_buffer_position also counts,
      // but send_buffer_position will only count spikes actually written,
      // we need all spikes to have information for buffer resizing.
      ++num_spikes_per_rank[ rank ];

      // We do not break if condition is false, because there may be spikes that
      // can be sent to other ranks than the one that is full.
      if ( not send_buffer_position.is_chunk_filled( rank ) )
      {
        send_buffer[ send_buffer_position.idx( rank ) ] = emitted_spike.spike_data;
        send_buffer_position.increase( rank );
      }
    }
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::set_end_marker_( const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer,
  size_t local_max_spikes_per_rank )
{
  // See comment in spike_data.h for logic.
  const bool collocate_complete = local_max_spikes_per_rank
    <= static_cast< size_t >( kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank() );

  for ( size_t rank = 0; rank < kernel::manager< MPIManager >.get_num_processes(); ++rank )
  {
    const size_t end_idx = send_buffer_position.end( rank ) - 1;
    if ( not collocate_complete )
    {
      SpikeDataT dummy;
      dummy.set_lcid( local_max_spikes_per_rank );
      dummy.set_invalid_marker();
      send_buffer[ end_idx ] = dummy;
      continue;
    }

    const size_t next_write_idx = send_buffer_position.idx( rank );
    if ( next_write_idx == send_buffer_position.begin( rank ) )
    {
      // No spikes for this rank, mark by INVALID in begin
      send_buffer[ send_buffer_position.begin( rank ) ].set_invalid_marker();
    }
    else
    {
      // At least one spike, set END on last position written to
      send_buffer[ next_write_idx - 1 ].set_end_marker();
    }

    if ( next_write_idx < end_idx + 1 )
    {
      // at least one spike written, but none to end_idx, thus we need complete marker
      // and size information
      SpikeDataT dummy;
      dummy.set_lcid( local_max_spikes_per_rank );
      dummy.set_complete_marker();
      send_buffer[ end_idx ] = dummy;
    }
  }
}

template < typename SpikeDataT >
void
EventDeliveryManager::reset_complete_marker_spike_data_( const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& send_buffer ) const
{
  for ( size_t rank = 0; rank < kernel::manager< MPIManager >.get_num_processes(); ++rank )
  {
    const size_t idx = send_buffer_position.end( rank ) - 1;
    send_buffer[ idx ].reset_marker();
  }
}

template < typename SpikeDataT >
size_t
EventDeliveryManager::get_global_max_spikes_per_rank_( const SendBufferPosition& send_buffer_position,
  std::vector< SpikeDataT >& recv_buffer ) const
{
  // TODO: send_buffer_position not needed here, only used to get endpoint of each per-rank section of buffer

  size_t maximum = 0;
  for ( size_t target_rank = 0; target_rank < kernel::manager< MPIManager >.get_num_processes(); ++target_rank )
  {
    const auto& end_entry = recv_buffer[ send_buffer_position.end( target_rank ) - 1 ];
    size_t max_per_thread_max_spikes_per_rank = 0;
    if ( end_entry.is_complete_marker() or end_entry.is_invalid_marker() )
    {
      max_per_thread_max_spikes_per_rank = end_entry.get_lcid();
    }
    else
    {
      assert( end_entry.is_end_marker() );
      max_per_thread_max_spikes_per_rank = kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();
    }
    maximum = std::max( max_per_thread_max_spikes_per_rank, maximum );
  }
  return maximum;
}

void
EventDeliveryManager::deliver_events( const size_t tid )
{
  if ( off_grid_spiking_ )
  {
    deliver_events_( tid, recv_buffer_off_grid_spike_data_ );
  }
  else
  {
    deliver_events_( tid, recv_buffer_spike_data_ );
  }
  reset_spike_register_( tid );
}

template < typename SpikeDataT >
void
EventDeliveryManager::deliver_events_( const size_t tid, const std::vector< SpikeDataT >& recv_buffer )
{
  // deliver only at beginning of time slice
  if ( kernel::manager< SimulationManager >.get_from_step() > 0 )
  {
    return;
  }

  const size_t spike_buffer_size_per_rank = kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();
  const std::vector< ConnectorModel* >& cm = kernel::manager< ModelManager >.get_connection_models( tid );

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps( kernel::manager< ConnectionManager >.get_min_delay() );
  for ( size_t lag = 0; lag < static_cast< size_t >( kernel::manager< ConnectionManager >.get_min_delay() ); ++lag )
  {
    // Subtract min_delay because spikes were emitted in previous time slice and we use current clock.
    prepared_timestamps[ lag ] = kernel::manager< SimulationManager >.get_clock()
      + Time::step( lag + 1 - kernel::manager< ConnectionManager >.get_min_delay() );
  }

  // Deliver spikes sent by each rank in order
  for ( size_t rank = 0; rank < kernel::manager< MPIManager >.get_num_processes(); ++rank )
  {
    // Continue with next rank if no spikes were sent by current rank
    if ( recv_buffer[ rank * spike_buffer_size_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    // Find number of spikes received from current rank
    size_t num_spikes_received = 0;
    for ( size_t i = 0; i < spike_buffer_size_per_rank; ++i )
    {
      const SpikeDataT& spike_data = recv_buffer[ rank * spike_buffer_size_per_rank + i ];

      // break if this was the last valid entry from this rank
      if ( spike_data.is_end_marker() )
      {
        num_spikes_received = i + 1;
        break;
      }
    }

    // For each batch, extract data first from receive buffer into value-specific arrays, then deliver from these arrays
    constexpr size_t SPIKES_PER_BATCH = 8;
    const size_t num_batches = num_spikes_received / SPIKES_PER_BATCH;
    const size_t num_remaining_entries = num_spikes_received - num_batches * SPIKES_PER_BATCH;

    SpikeEvent se_batch[ SPIKES_PER_BATCH ];
    size_t tid_batch[ SPIKES_PER_BATCH ];
    size_t syn_id_batch[ SPIKES_PER_BATCH ];
    size_t lcid_batch[ SPIKES_PER_BATCH ];

    if ( not kernel::manager< ConnectionManager >.use_compressed_spikes() )
    {
      for ( size_t i = 0; i < num_batches; ++i )
      {
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          const SpikeDataT& spike_data = recv_buffer[ rank * spike_buffer_size_per_rank + i * SPIKES_PER_BATCH + j ];
          se_batch[ j ].set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
          se_batch[ j ].set_offset( spike_data.get_offset() );
          tid_batch[ j ] = spike_data.get_tid();
          syn_id_batch[ j ] = spike_data.get_syn_id();
          lcid_batch[ j ] = spike_data.get_lcid();
          se_batch[ j ].set_sender_node_id_info( tid_batch[ j ], syn_id_batch[ j ], lcid_batch[ j ] );
        }
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          if ( tid_batch[ j ] == tid )
          {
            kernel::manager< ConnectionManager >.send(
              tid_batch[ j ], syn_id_batch[ j ], lcid_batch[ j ], cm, se_batch[ j ] );
          }
        }
      }

      // Processed all regular-sized batches, now do remainder
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        const SpikeDataT& spike_data =
          recv_buffer[ rank * spike_buffer_size_per_rank + num_batches * SPIKES_PER_BATCH + j ];
        se_batch[ j ].set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
        se_batch[ j ].set_offset( spike_data.get_offset() );
        tid_batch[ j ] = spike_data.get_tid();
        syn_id_batch[ j ] = spike_data.get_syn_id();
        lcid_batch[ j ] = spike_data.get_lcid();
        se_batch[ j ].set_sender_node_id_info( tid_batch[ j ], syn_id_batch[ j ], lcid_batch[ j ] );
      }
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        if ( tid_batch[ j ] == tid )
        {
          kernel::manager< ConnectionManager >.send(
            tid_batch[ j ], syn_id_batch[ j ], lcid_batch[ j ], cm, se_batch[ j ] );
        }
      }
    }
    else // compressed spikes
    {
      for ( size_t i = 0; i < num_batches; ++i )
      {
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          const SpikeDataT& spike_data = recv_buffer[ rank * spike_buffer_size_per_rank + i * SPIKES_PER_BATCH + j ];

          se_batch[ j ].set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
          se_batch[ j ].set_offset( spike_data.get_offset() );

          syn_id_batch[ j ] = spike_data.get_syn_id();
          // for compressed spikes lcid holds the index in the
          // compressed_spike_data structure
          lcid_batch[ j ] = spike_data.get_lcid();
        }
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          // find the spike-data entry for this thread
          const std::vector< SpikeData >& compressed_spike_data =
            kernel::manager< ConnectionManager >.get_compressed_spike_data( syn_id_batch[ j ], lcid_batch[ j ] );
          lcid_batch[ j ] = compressed_spike_data[ tid ].get_lcid();
        }
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          if ( lcid_batch[ j ] != invalid_lcid )
          {
            // non-local sender -> receiver retrieves ID of sender Node from SourceTable based on tid, syn_id, lcid
            // only if needed, as this is computationally costly
            se_batch[ j ].set_sender_node_id_info( tid, syn_id_batch[ j ], lcid_batch[ j ] );
          }
        }
        for ( size_t j = 0; j < SPIKES_PER_BATCH; ++j )
        {
          if ( lcid_batch[ j ] != invalid_lcid )
          {
            kernel::manager< ConnectionManager >.send( tid, syn_id_batch[ j ], lcid_batch[ j ], cm, se_batch[ j ] );
          }
        }
      }

      // Processed all regular-sized batches, now do remainder
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        const SpikeDataT& spike_data =
          recv_buffer[ rank * spike_buffer_size_per_rank + num_batches * SPIKES_PER_BATCH + j ];
        se_batch[ j ].set_stamp( prepared_timestamps[ spike_data.get_lag() ] );
        se_batch[ j ].set_offset( spike_data.get_offset() );
        syn_id_batch[ j ] = spike_data.get_syn_id();
        // for compressed spikes lcid holds the index in the
        // compressed_spike_data structure
        lcid_batch[ j ] = spike_data.get_lcid();
      }
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        // find the spike-data entry for this thread
        const std::vector< SpikeData >& compressed_spike_data =
          kernel::manager< ConnectionManager >.get_compressed_spike_data( syn_id_batch[ j ], lcid_batch[ j ] );
        lcid_batch[ j ] = compressed_spike_data[ tid ].get_lcid();
      }
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        if ( lcid_batch[ j ] != invalid_lcid )
        {
          // non-local sender -> receiver retrieves ID of sender Node from SourceTable based on tid, syn_id, lcid
          // only if needed, as this is computationally costly
          se_batch[ j ].set_sender_node_id_info( tid, syn_id_batch[ j ], lcid_batch[ j ] );
        }
      }
      for ( size_t j = 0; j < num_remaining_entries; ++j )
      {
        if ( lcid_batch[ j ] != invalid_lcid )
        {
          kernel::manager< ConnectionManager >.send( tid, syn_id_batch[ j ], lcid_batch[ j ], cm, se_batch[ j ] );
        }
      }
    } // if-else not compressed
  }   // for rank
}

template <>
void
EventDeliveryManager::send< SpikeEvent >( Node& source, SpikeEvent& e, const long lag )
{
  const size_t tid = source.get_thread();
  const size_t source_node_id = source.get_node_id();
  e.set_sender_node_id( source_node_id );
  if ( source.has_proxies() )
  {
    local_spike_counter_[ tid ] += e.get_multiplicity();

    e.set_stamp( kernel::manager< SimulationManager >.get_slice_origin() + Time::step( lag + 1 ) );
    e.set_sender( source );

    if ( source.is_off_grid() )
    {
      send_off_grid_remote( tid, e, lag );
    }
    else
    {
      send_remote( tid, e, lag );
    }
    kernel::manager< ConnectionManager >.send_to_devices( tid, source_node_id, e );
  }
  else
  {
    send_local_( source, e, lag );
  }
}

template <>
void
EventDeliveryManager::send< DSSpikeEvent >( Node& source, DSSpikeEvent& e, const long lag )
{
  e.set_sender_node_id( source.get_node_id() );
  send_local_( source, e, lag );
}

void
EventDeliveryManager::gather_target_data( const size_t tid )
{
  assert( not kernel::manager< ConnectionManager >.is_source_table_cleared() );

  // assume all threads have some work to do
  gather_completed_checker_.set_false( tid );
  assert( gather_completed_checker_.all_false() );

  const AssignedRanks assigned_ranks = kernel::manager< VPManager >.get_assigned_ranks( tid );

  kernel::manager< ConnectionManager >.prepare_target_table( tid );
  kernel::manager< ConnectionManager >.reset_source_table_entry_point( tid );

  while ( gather_completed_checker_.any_false() )
  {
    // assume this is the last gather round and change to false
    // otherwise
    gather_completed_checker_.set_true( tid );

#pragma omp master
    {
      if ( kernel::manager< MPIManager >.adaptive_target_buffers() and buffer_size_target_data_has_changed_ )
      {
        resize_send_recv_buffers_target_data();
      }
    } // of omp master; (no barrier)
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();

    kernel::manager< ConnectionManager >.restore_source_table_entry_point( tid );

    TargetSendBufferPosition send_buffer_position(
      assigned_ranks, kernel::manager< MPIManager >.get_send_recv_count_target_data_per_rank() );

    const bool gather_completed = collocate_target_data_buffers_( tid, assigned_ranks, send_buffer_position );
    gather_completed_checker_.logical_and( tid, gather_completed );

    if ( gather_completed_checker_.all_true() )
    {
      set_complete_marker_target_data_( assigned_ranks, send_buffer_position );
    }
    kernel::manager< ConnectionManager >.save_source_table_entry_point( tid );
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
    kernel::manager< ConnectionManager >.clean_source_table( tid );

#pragma omp master
    {
      sw_communicate_target_data_.start();
      kernel::manager< MPIManager >.communicate_target_data_Alltoall(
        send_buffer_target_data_, recv_buffer_target_data_ );
      sw_communicate_target_data_.stop();
    } // of omp master (no barriers!)
#pragma omp barrier

    const bool distribute_completed = distribute_target_data_buffers_( tid );
    gather_completed_checker_.logical_and( tid, distribute_completed );

    // resize mpi buffers, if necessary and allowed
    if ( gather_completed_checker_.any_false() and kernel::manager< MPIManager >.adaptive_target_buffers() )
    {
#pragma omp master
      {
        buffer_size_target_data_has_changed_ = kernel::manager< MPIManager >.increase_buffer_size_target_data();
      }
#pragma omp barrier
    }
  } // of while

  kernel::manager< ConnectionManager >.clear_source_table( tid );
}

void
EventDeliveryManager::gather_target_data_compressed( const size_t tid )
{
  assert( not kernel::manager< ConnectionManager >.is_source_table_cleared() );

  // assume all threads have some work to do
  gather_completed_checker_.set_false( tid );
  assert( gather_completed_checker_.all_false() );

  const AssignedRanks assigned_ranks = kernel::manager< VPManager >.get_assigned_ranks( tid );

  kernel::manager< ConnectionManager >.prepare_target_table( tid );

  while ( gather_completed_checker_.any_false() )
  {
    // assume this is the last gather round and change to false otherwise
    gather_completed_checker_.set_true( tid );

#pragma omp master
    {
      if ( kernel::manager< MPIManager >.adaptive_target_buffers() and buffer_size_target_data_has_changed_ )
      {
        resize_send_recv_buffers_target_data();
      }
    } // of omp master; no barrier
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();

    TargetSendBufferPosition send_buffer_position(
      assigned_ranks, kernel::manager< MPIManager >.get_send_recv_count_target_data_per_rank() );

    const bool gather_completed =
      collocate_target_data_buffers_compressed_( tid, assigned_ranks, send_buffer_position );

    gather_completed_checker_.logical_and( tid, gather_completed );

    if ( gather_completed_checker_.all_true() )
    {
      set_complete_marker_target_data_( assigned_ranks, send_buffer_position );
    }

    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
    kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();

#pragma omp master
    {
      sw_communicate_target_data_.start();
      kernel::manager< MPIManager >.communicate_target_data_Alltoall(
        send_buffer_target_data_, recv_buffer_target_data_ );
      sw_communicate_target_data_.stop();
    } // of omp master (no barrier)
#pragma omp barrier

    // Up to here, gather_completed_checker_ just has local info: has this thread been able to write
    // all data it is responsible for to buffers. Now combine with information on whether other ranks
    // have sent all their data. Note: All threads will return the same value for distribute_completed.
    const bool distribute_completed = distribute_target_data_buffers_( tid );
    gather_completed_checker_.logical_and( tid, distribute_completed );

    // resize mpi buffers, if necessary and allowed
    if ( gather_completed_checker_.any_false() and kernel::manager< MPIManager >.adaptive_target_buffers() )
    {
#pragma omp master
      {
        buffer_size_target_data_has_changed_ = kernel::manager< MPIManager >.increase_buffer_size_target_data();
      } // of omp master (no barrier)
      kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
      kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
    }

  } // of while

  kernel::manager< ConnectionManager >.clear_source_table( tid );
}

bool
EventDeliveryManager::collocate_target_data_buffers_( const size_t tid,
  const AssignedRanks& assigned_ranks,
  TargetSendBufferPosition& send_buffer_position )
{
  size_t source_rank;
  TargetData next_target_data;
  bool valid_next_target_data;
  bool is_source_table_read = true;

  // no ranks to process for this thread
  if ( assigned_ranks.begin == assigned_ranks.end )
  {
    kernel::manager< ConnectionManager >.no_targets_to_process( tid );
    return is_source_table_read;
  }

  // reset markers
  for ( size_t rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
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
    valid_next_target_data = kernel::manager< ConnectionManager >.get_next_target_data(
      tid, assigned_ranks.begin, assigned_ranks.end, source_rank, next_target_data );
    if ( valid_next_target_data ) // add valid entry to MPI buffer
    {
      if ( send_buffer_position.is_chunk_filled( source_rank ) )
      {
        // entry does not fit in this part of the MPI buffer any more,
        // so we need to reject it
        kernel::manager< ConnectionManager >.reject_last_target_data( tid );
        // after rejecting the last target, we need to save the
        // position to start at this point again next communication
        // round
        kernel::manager< ConnectionManager >.save_source_table_entry_point( tid );
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
      for ( size_t rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
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

bool
EventDeliveryManager::collocate_target_data_buffers_compressed_( const size_t tid,
  const AssignedRanks& assigned_ranks,
  TargetSendBufferPosition& send_buffer_position )
{
  // no ranks to process for this thread
  if ( assigned_ranks.begin == assigned_ranks.end )
  {
    return true;
  }

  // reset markers
  for ( size_t rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // reset last entry to avoid accidentally communicating done
    // marker
    send_buffer_target_data_[ send_buffer_position.end( rank ) - 1 ].reset_marker();
    // set first entry to invalid to avoid accidentally reading
    // uninitialized parts of the receive buffer
    send_buffer_target_data_[ send_buffer_position.begin( rank ) ].set_invalid_marker();
  }

  const bool is_source_table_read = kernel::manager< ConnectionManager >.fill_target_buffer(
    tid, assigned_ranks.begin, assigned_ranks.end, send_buffer_target_data_, send_buffer_position );

  return is_source_table_read;
}


void
nest::EventDeliveryManager::set_complete_marker_target_data_( const AssignedRanks& assigned_ranks,
  const TargetSendBufferPosition& send_buffer_position )
{
  for ( size_t rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    const size_t idx = send_buffer_position.end( rank ) - 1;
    send_buffer_target_data_[ idx ].set_complete_marker();
  }
}

bool
nest::EventDeliveryManager::distribute_target_data_buffers_( const size_t tid )
{
  bool are_others_completed = true;
  const unsigned int send_recv_count_target_data_per_rank =
    kernel::manager< MPIManager >.get_send_recv_count_target_data_per_rank();

  for ( size_t rank = 0; rank < kernel::manager< MPIManager >.get_num_processes(); ++rank )
  {
    // Check last entry for completed marker
    if ( not recv_buffer_target_data_[ ( rank + 1 ) * send_recv_count_target_data_per_rank - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // Were any targets sent by this rank?
    if ( recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_target_data_per_rank; ++i )
    {
      const TargetData& target_data = recv_buffer_target_data_[ rank * send_recv_count_target_data_per_rank + i ];
      if ( target_data.get_source_tid() == tid )
      {
        kernel::manager< ConnectionManager >.add_target( tid, rank, target_data );
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

size_t
EventDeliveryManager::write_toggle() const
{
  return kernel::manager< SimulationManager >.get_slice() % 2;
}

void
EventDeliveryManager::send_secondary( Node& source, SecondaryEvent& e )
{
  const size_t tid = kernel::manager< VPManager >.get_thread_id();
  const size_t source_node_id = source.get_node_id();
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );

  if ( source.has_proxies() )
  {
    // We need to consider every synapse type this event supports to make sure also labeled and connection created by
    // CopyModel are considered.
    const std::set< synindex >& supported_syn_ids = e.get_supported_syn_ids();
    for ( const auto& syn_id : supported_syn_ids )
    {
      const std::vector< size_t >& positions =
        kernel::manager< ConnectionManager >.get_secondary_send_buffer_positions( tid, lid, syn_id );

      for ( size_t i = 0; i < positions.size(); ++i )
      {
        std::vector< unsigned int >::iterator it = send_buffer_secondary_events_.begin() + positions[ i ];
        e >> it;
      }
    }
    kernel::manager< ConnectionManager >.send_to_devices( tid, source_node_id, e );
  }
  else
  {
    // need to pass lag (last argument), but not used in template specialization, so pass zero as dummy value
    send_local_( source, e, 0 );
  }
}

void
EventDeliveryManager::send_off_grid_remote( size_t tid, SpikeEvent& e, const long lag )
{
  // Put the spike in a buffer for the remote machines
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( e.get_sender().get_node_id() );
  const auto& targets = kernel::manager< ConnectionManager >.get_remote_targets_of_local_node( tid, lid );

  for ( const auto& target : targets )
  {
    // Unroll spike multiplicity as plastic synapses only handle individual spikes.
    for ( size_t i = 0; i < e.get_multiplicity(); ++i )
    {
      off_grid_emitted_spikes_register_[ tid ]->emplace_back( target, lag, e.get_offset() );
    }
  }
}

void
EventDeliveryManager::send_remote( size_t tid, SpikeEvent& e, const long lag )
{
  // Put the spike in a buffer for the remote machines
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( e.get_sender().get_node_id() );
  const auto& targets = kernel::manager< ConnectionManager >.get_remote_targets_of_local_node( tid, lid );

  for ( const auto& target : targets )
  {
    // Unroll spike multiplicity as plastic synapses only handle individual spikes.
    for ( size_t i = 0; i < e.get_multiplicity(); ++i )
    {
      emitted_spikes_register_[ tid ]->emplace_back( target, lag );
    }
  }
}

template <>
void
EventDeliveryManager::send_local_( Node& source, SecondaryEvent& e, const long )
{
  assert( not source.has_proxies() );
  e.set_stamp( kernel::manager< SimulationManager >.get_slice_origin() + Time::step( 1 ) );
  e.set_sender( source );
  const size_t t = source.get_thread();
  const size_t ldid = source.get_local_device_id();
  kernel::manager< ConnectionManager >.send_from_device( t, ldid, e );
}

long
EventDeliveryManager::get_slice_modulo( long d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time when all events due are read out.
  assert( static_cast< std::vector< long >::size_type >( d ) < slice_moduli_.size() );

  return slice_moduli_[ d ];
}

long
EventDeliveryManager::get_modulo( long d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time when all events due are read out.
  assert( static_cast< std::vector< long >::size_type >( d ) < moduli_.size() );

  return moduli_[ d ];
}

size_t
EventDeliveryManager::read_toggle() const
{
  // define in terms of write_toggle() to ensure consistency
  return 1 - write_toggle();
}

void
EventDeliveryManager::set_off_grid_communication( bool off_grid_spiking )
{
  off_grid_spiking_ = off_grid_spiking;
}

bool
EventDeliveryManager::get_off_grid_communication() const
{
  return off_grid_spiking_;
}

void
EventDeliveryManager::send_to_node( Event& e )
{
  e();
}

bool
EventDeliveryManager::is_marked_for_removal_( const Target& target )
{

  return target.is_processed();
}

void
EventDeliveryManager::reset_spike_register_( const size_t tid )
{

  emitted_spikes_register_[ tid ]->clear();
  off_grid_emitted_spikes_register_[ tid ]->clear();
}

} // of namespace nest
