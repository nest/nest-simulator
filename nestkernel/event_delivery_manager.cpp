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

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "vp_manager_impl.h"
#include "spike_register_table_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
EventDeliveryManager::EventDeliveryManager()
  : off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
  , spike_register_()
  , offgrid_spike_register_()
  , local_grid_spikes_()
  , global_grid_spikes_()
  , local_offgrid_spikes_()
  , global_offgrid_spikes_()
  , displacements_()
  , comm_marker_( 0 )
{
}

EventDeliveryManager::~EventDeliveryManager()
{
  // clear the buffers
  local_grid_spikes_.clear();
  global_grid_spikes_.clear();
  local_offgrid_spikes_.clear();
  global_offgrid_spikes_.clear();
}

void
EventDeliveryManager::initialize()
{
  init_moduli();
  spike_register_table_.initialize();
}

void
EventDeliveryManager::finalize()
{
  // clear the buffers
  local_grid_spikes_.clear();
  global_grid_spikes_.clear();
  local_offgrid_spikes_.clear();
  global_offgrid_spikes_.clear();
  spike_register_table_.finalize();
}

void
EventDeliveryManager::set_status( const DictionaryDatum& dict )
{
  updateValue< bool >( dict, "off_grid_spiking", off_grid_spiking_ );
}

void
EventDeliveryManager::get_status( DictionaryDatum& dict )
{
  def< bool >( dict, "off_grid_spiking", off_grid_spiking_ );
}

void
EventDeliveryManager::clear_pending_spikes()
{
  configure_spike_buffers();
}

void
EventDeliveryManager::configure_spike_buffers()
{
  assert( kernel().connection_builder_manager.get_min_delay() != 0 );

  spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  spike_register_.resize( kernel().vp_manager.get_num_threads(),
    std::vector< std::vector< uint_t > >( kernel().connection_builder_manager.get_min_delay() ) );
  for ( size_t j = 0; j < spike_register_.size(); ++j )
    for ( size_t k = 0; k < spike_register_[ j ].size(); ++k )
      spike_register_[ j ][ k ].clear();

  spike_register_table_.clear();

  send_buffer_spike_data_.resize( mpi_buffer_size_spike_data );
  recv_buffer_spike_data_.resize( mpi_buffer_size_spike_data );

  offgrid_spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  offgrid_spike_register_.resize( kernel().vp_manager.get_num_threads(),
    std::vector< std::vector< OffGridSpike > >(
                                    kernel().connection_builder_manager.get_min_delay() ) );
  for ( size_t j = 0; j < offgrid_spike_register_.size(); ++j )
    for ( size_t k = 0; k < offgrid_spike_register_[ j ].size(); ++k )
      offgrid_spike_register_[ j ][ k ].clear();

  // send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces.
  int send_buffer_size =
    kernel().vp_manager.get_num_threads() * kernel().connection_builder_manager.get_min_delay() > 2
    ? kernel().vp_manager.get_num_threads() * kernel().connection_builder_manager.get_min_delay()
    : 2;
  int recv_buffer_size = send_buffer_size * kernel().mpi_manager.get_num_processes();
  Communicator::set_buffer_sizes( send_buffer_size, recv_buffer_size );

  // DEC cxx required 0U literal, HEP 2007-03-26
  local_grid_spikes_.clear();
  local_grid_spikes_.resize( send_buffer_size, 0U );
  local_offgrid_spikes_.clear();
  local_offgrid_spikes_.resize( send_buffer_size, OffGridSpike( 0, 0.0 ) );
  global_grid_spikes_.clear();
  global_grid_spikes_.resize( recv_buffer_size, 0U );
  global_offgrid_spikes_.clear();
  global_offgrid_spikes_.resize( recv_buffer_size, OffGridSpike( 0, 0.0 ) );

  displacements_.clear();
  displacements_.resize( kernel().mpi_manager.get_num_processes(), 0 );
}

void
EventDeliveryManager::init_moduli()
{
  delay min_delay = kernel().connection_builder_manager.get_min_delay();
  delay max_delay = kernel().connection_builder_manager.get_max_delay();
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
    moduli_[ d ] =
      ( kernel().simulation_manager.get_clock().get_steps() + d ) % ( min_delay + max_delay );
  }

  // Slice-based ring-buffers have one bin per min_delay steps,
  // up to max_delay.  Time is counted as for normal ring buffers.
  // The slice_moduli_ table maps time steps to these bins
  const size_t nbuff = static_cast< size_t >(
    std::ceil( static_cast< double >( min_delay + max_delay ) / min_delay ) );
  slice_moduli_.resize( min_delay + max_delay );
  for ( delay d = 0; d < min_delay + max_delay; ++d )
  {
    slice_moduli_[ d ] =
      ( ( kernel().simulation_manager.get_clock().get_steps() + d ) / min_delay ) % nbuff;
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
  delay min_delay = kernel().connection_builder_manager.get_min_delay();
  delay max_delay = kernel().connection_builder_manager.get_max_delay();
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
    slice_moduli_[ d ] = ( ( kernel().simulation_manager.get_clock().get_steps() + d )
                           / min_delay ) % nbuff;
  }
}

void
EventDeliveryManager::collocate_buffers_()
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
  if ( !off_grid_spiking_ ) // on grid spiking
  {
    // make sure buffers are correctly sized
    if ( global_grid_spikes_.size()
      != static_cast< uint_t >( Communicator::get_recv_buffer_size() ) )
      global_grid_spikes_.resize( Communicator::get_recv_buffer_size(), 0 );

    if ( num_spikes + ( kernel().vp_manager.get_num_threads()
                        * kernel().connection_builder_manager.get_min_delay() )
      > static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_grid_spikes_.resize(
        ( num_spikes + ( kernel().connection_builder_manager.get_min_delay()
                         * kernel().vp_manager.get_num_threads() ) ),
        0 );
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

    if ( num_spikes + ( kernel().vp_manager.get_num_threads()
                        * kernel().connection_builder_manager.get_min_delay() )
      > static_cast< uint_t >( Communicator::get_send_buffer_size() ) )
      local_offgrid_spikes_.resize(
        ( num_spikes + ( kernel().connection_builder_manager.get_min_delay()
                         * kernel().vp_manager.get_num_threads() ) ),
        OffGridSpike( 0, 0.0 ) );
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
EventDeliveryManager::deliver_events( thread t )
{
  // deliver only at beginning of time slice
  if ( kernel().simulation_manager.get_from_step() > 0 )
    return;

  SpikeEvent se;

  std::vector< int > pos( displacements_ );

  if ( !off_grid_spiking_ ) // on_grid_spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
    for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
          lag++ )
    {
      prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
    }

    for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
    {
      size_t pid = kernel().mpi_manager.get_process_id( vp );
      int pos_pid = pos[ pid ];
      int lag = kernel().connection_builder_manager.get_min_delay() - 1;
      while ( lag >= 0 )
      {
        index nid = global_grid_spikes_[ pos_pid ];
        if ( nid != static_cast< index >( comm_marker_ ) )
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp( prepared_timestamps[ lag ] );
          se.set_sender_gid( nid );
          kernel().connection_builder_manager.send( t, nid, se );
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
  else // off grid spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
    for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
          lag++ )
    {
      prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
    }

    for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
    {
      size_t pid = kernel().mpi_manager.get_process_id( vp );
      int pos_pid = pos[ pid ];
      int lag = kernel().connection_builder_manager.get_min_delay() - 1;
      while ( lag >= 0 )
      {
        index nid = global_offgrid_spikes_[ pos_pid ].get_gid();
        if ( nid != static_cast< index >( comm_marker_ ) )
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp( prepared_timestamps[ lag ] );
          se.set_sender_gid( nid );
          se.set_offset( global_offgrid_spikes_[ pos_pid ].get_offset() );
          kernel().connection_builder_manager.send( t, nid, se );
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
EventDeliveryManager::gather_events()
{
  collocate_buffers_();
  if ( off_grid_spiking_ )
    Communicator::communicate( local_offgrid_spikes_, global_offgrid_spikes_, displacements_ );
  else
    Communicator::communicate( local_grid_spikes_, global_grid_spikes_, displacements_ );
}

void
EventDeliveryManager::gather_spike_data( const thread tid )
{
  unsigned int send_recv_count = sizeof( SpikeData ) / sizeof( unsigned int ) *
    ceil( send_buffer_spike_data_.size() / kernel().mpi_manager.get_num_processes() );

  spike_register_table_.reset_entry_point( tid );
  while ( true )
  {
    static bool me_completed;
    static bool others_completed;
#pragma omp single
    {
      me_completed = false;
      others_completed = false;
    }
    spike_register_table_.restore_entry_point( tid );
    prepare_spike_data_buffers_( me_completed );
      
#pragma omp barrier
    collocate_spike_data_buffers_( tid );
#pragma omp barrier
#pragma omp single
    {
      me_completed = check_spike_data_me_completed_();
    }
    if ( me_completed )
    {
#pragma omp barrier
      prepare_spike_data_buffers_( me_completed );
    }
      
    spike_register_table_.save_entry_point( tid );
#pragma omp single
    {
      unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_spike_data_[0] );
      unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_spike_data_[0] );
      Communicator::communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count );
    } // of omp single
#pragma omp single
    {
      others_completed = check_spike_data_others_completed_();
    }
    deliver_events_5g_( tid );
    if ( me_completed && others_completed )
    {
      break;
    }
  } // of while(true)
}

bool
EventDeliveryManager::check_spike_data_me_completed_() const
{
  for ( std::vector< SpikeData >::const_iterator it = send_buffer_spike_data_.begin();
        it != send_buffer_spike_data_.end(); ++it )
  {
    if ( not it->is_empty() )
    {
      return false;
    }
  }
  return true;
}

bool
EventDeliveryManager::check_spike_data_others_completed_() const
{
  for ( std::vector< SpikeData >::const_iterator it = recv_buffer_spike_data_.begin();
        it != recv_buffer_spike_data_.end(); ++it )
  {
    if ( not it->is_complete() )
    {
      return false;
    }
  }
  return true;
}


void
EventDeliveryManager::prepare_spike_data_buffers_( const bool me_completed )
{
  if ( not me_completed )
  {
#pragma omp for
    for ( unsigned int i = 0; i < send_buffer_spike_data_.size(); ++i )
    {
      send_buffer_spike_data_[ i ].set_empty();
    } // of omp for
  }
  else
  {
#pragma omp for
    for ( unsigned int i = 0; i < send_buffer_spike_data_.size(); ++i )
    {
      send_buffer_spike_data_[ i ].set_complete();
    } // of omp for
  }

}

void
EventDeliveryManager::collocate_spike_data_buffers_( const thread tid )
{
  // TODO@5g:
  const unsigned int num_assigned_ranks_per_thread = kernel().vp_manager.get_num_assigned_ranks_per_thread();
  const unsigned int rank_start = kernel().vp_manager.get_start_rank_per_thread();
  const unsigned int rank_end = kernel().vp_manager.get_end_rank_per_thread();
  // store how far each segment in sendbuffer is filled
  std::vector< unsigned int > send_buffer_offset( num_assigned_ranks_per_thread, 0 );
  unsigned int sum_send_buffer_offset = 0;
  unsigned int num_spike_data_per_rank = ceil( send_buffer_spike_data_.size() / kernel().mpi_manager.get_num_processes() );
  index target_rank;
  SpikeData next_spike_data;

  if ( rank_start != rank_end )
  {
    while ( true )
    {
      if ( spike_register_table_.get_next_spike_data( tid, target_rank, next_spike_data, rank_start, rank_end ) )
      {
        thread target_rank_index = target_rank - rank_start;
        if ( send_buffer_offset[ target_rank_index ] < num_spike_data_per_rank )
        {
          unsigned int idx = target_rank * num_spike_data_per_rank + send_buffer_offset[ target_rank_index ];
          send_buffer_spike_data_[ idx ] = next_spike_data;
          ++send_buffer_offset[ target_rank_index ];
          ++sum_send_buffer_offset;
        }
        else
        {
          spike_register_table_.reject_last_spike_data( tid );
          spike_register_table_.save_entry_point( tid );
        }
        if ( sum_send_buffer_offset == ( num_spike_data_per_rank * num_assigned_ranks_per_thread ) )
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
  }
}

void
EventDeliveryManager::deliver_events_5g_( const thread tid )
{
  // deliver only at beginning of time slice
  if ( kernel().simulation_manager.get_from_step() > 0 )
    return;

  SpikeEvent se;

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
  for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
        lag++ )
  {
    prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() + Time::step( lag + 1 );
  }

  for ( std::vector< SpikeData >::const_iterator it = recv_buffer_spike_data_.begin();
        it != recv_buffer_spike_data_.end(); ++it )
  {
    if ( it->tid == tid )
    {
      se.set_stamp( prepared_timestamps[ it->lag ] );
      kernel().connection_builder_manager.send_5g( tid, it->syn_index, it->lcid, se );
    }
  }

}

void
EventDeliveryManager::gather_target_data()
{
  assert( not kernel().connection_builder_manager.is_source_table_cleared() );

  std::vector< TargetData > send_buffer_target_data( mpi_buffer_size_target_data );
  std::vector< TargetData > recv_buffer_target_data( mpi_buffer_size_target_data );

  bool me_completed = false;
  bool others_completed = false;
  unsigned int send_recv_count = sizeof( TargetData ) / sizeof( unsigned int ) *
    ceil( send_buffer_target_data.size() / kernel().mpi_manager.get_num_processes() );

#pragma omp parallel
  {
    thread tid = kernel().vp_manager.get_thread_id();
    kernel().connection_builder_manager.prepare_target_table( tid );
    kernel().connection_builder_manager.reset_source_table_entry_point( tid );
    while ( not others_completed || not me_completed )
    {
      kernel().connection_builder_manager.restore_source_table_entry_point( tid );
      prepare_target_data_buffers_( me_completed, send_buffer_target_data );
      
#pragma omp barrier
      collocate_target_data_buffers_( tid, send_buffer_target_data );
#pragma omp barrier
      me_completed = check_target_data_me_completed_( send_buffer_target_data );
      if ( me_completed )
      {
        prepare_target_data_buffers_( me_completed, send_buffer_target_data );
      }
      
      kernel().connection_builder_manager.save_source_table_entry_point( tid );      
#pragma omp single
      {
        unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_target_data[0] );
        unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_target_data[0] );
        Communicator::communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count );
        
      } // of omp single
      
      others_completed  = check_target_data_others_completed_( recv_buffer_target_data );
      distribute_target_data_buffers_( tid, recv_buffer_target_data );
    }
  } // of omp parallel
}

bool
EventDeliveryManager::check_target_data_me_completed_( const std::vector< TargetData >& buffer )
{
  for ( std::vector< TargetData >::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
  {
    if ( not it->is_empty() )
    {
      return false;
    }
  }
  return true;
}

bool
EventDeliveryManager::check_target_data_others_completed_( const std::vector< TargetData >& buffer )
{
  for ( std::vector< TargetData >::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
  {
    if ( not it->is_complete() )
    {
      return false;
    }
  }
  return true;
}

void
EventDeliveryManager::prepare_target_data_buffers_( const bool me_completed, std::vector< TargetData >& send_buffer )
{
  if ( not me_completed )
  {
#pragma omp for
    for ( unsigned int i = 0; i < send_buffer.size(); ++i )
    {
      send_buffer[ i ].set_empty();
    } // of omp for
  }
  else
  {
#pragma omp for
    for ( unsigned int i = 0; i < send_buffer.size(); ++i )
    {
      send_buffer[ i ].set_complete();
    } // of omp for
  }
}

void
EventDeliveryManager::collocate_target_data_buffers_( const thread tid, std::vector< TargetData >& send_buffer )
{
  const unsigned int num_assigned_ranks_per_thread = kernel().vp_manager.get_num_assigned_ranks_per_thread();
  const unsigned int rank_start = kernel().vp_manager.get_start_rank_per_thread();
  const unsigned int rank_end = kernel().vp_manager.get_end_rank_per_thread();
  // store how far each segment in sendbuffer is filled
  std::vector< unsigned int > send_buffer_offset( num_assigned_ranks_per_thread, 0 );
  unsigned int sum_send_buffer_offset = 0;
  unsigned int num_target_data_per_rank = ceil( send_buffer.size() / kernel().mpi_manager.get_num_processes() );
  TargetData next_target_data;

  if ( rank_start != rank_end )
  {
    while ( true )
    {
      kernel().connection_builder_manager.get_next_target_data( tid, next_target_data, rank_start, rank_end );
      if ( next_target_data.gid == invalid_index )
      {
        break;
      }
      else
      {
        thread target_rank = kernel().mpi_manager.get_process_id_of_gid( next_target_data.gid );
        thread target_rank_index = target_rank - rank_start;
        if ( send_buffer_offset[ target_rank_index ] < num_target_data_per_rank )
        {
          unsigned int idx = target_rank * num_target_data_per_rank + send_buffer_offset[ target_rank_index ];
          send_buffer[ idx ] = next_target_data;
          ++send_buffer_offset[ target_rank_index ];
          ++sum_send_buffer_offset;
        }
        else
        {
          kernel().connection_builder_manager.reject_last_target_data( tid );
          kernel().connection_builder_manager.save_source_table_entry_point( tid );
        }
      }
      if ( sum_send_buffer_offset == ( num_target_data_per_rank * num_assigned_ranks_per_thread ) )
      {
        break;
      }
    }
  }
}

void
nest::EventDeliveryManager::distribute_target_data_buffers_( const thread tid, const std::vector< TargetData >& recv_buffer )
{
  for ( std::vector< TargetData >::const_iterator it = recv_buffer.begin(); it != recv_buffer.end(); ++it )
  {
    if ( ( not it->is_empty() ) && ( not it->is_complete() )
         && kernel().vp_manager.is_vp_local( it->gid ) )
    {
      std::cout << kernel().vp_manager.get_vp() << " add conn" << std::endl;
      kernel().connection_builder_manager.add_target( tid, *it );
    }
  }
}

} // of namespace nest
