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
#include "vp_manager.h"
#include "vp_manager_impl.h"
#include "node_manager_impl.h"
#include "connection_manager.h"
#include "connection_manager_impl.h"
#include "event_delivery_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
EventDeliveryManager::EventDeliveryManager()
  : comm_steps_target_data( 0 )
  , comm_rounds_target_data( 0 )
  , comm_steps_spike_data( 0 )
  , comm_rounds_spike_data( 0 )
  , off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
  , comm_marker_( 0 )
  , buffer_size_target_data_has_changed_( false )
  , buffer_size_spike_data_has_changed_( false )
{
}

EventDeliveryManager::~EventDeliveryManager()
{
}

void
EventDeliveryManager::initialize()
{
  init_moduli();
  const thread num_threads = kernel().vp_manager.get_num_threads();
  spike_register_5g_.resize( num_threads, 0 );
  off_grid_spike_register_5g_.resize( num_threads, 0 );

  for( thread tid = 0; tid < num_threads; ++tid )
  {
    if ( spike_register_5g_[ tid ] != 0 )
    {
      delete( spike_register_5g_[ tid ] ); // TODO@5g: does this make sense or should we try to resize?
    }
    spike_register_5g_[ tid ] = new std::vector< std::vector< std::vector< Target > > >( num_threads, std::vector< std::vector< Target > >( kernel().connection_manager.get_min_delay(), std::vector< Target >( 0 ) ) );
    if ( off_grid_spike_register_5g_[ tid ] != 0 )
    {
      delete( off_grid_spike_register_5g_[ tid ] ); // TODO@5g: does this make sense or should we try to resize?
    }
    off_grid_spike_register_5g_[ tid ] = new std::vector< std::vector< std::vector< OffGridTarget > > >( num_threads, std::vector< std::vector< OffGridTarget > >( kernel().connection_manager.get_min_delay(), std::vector< OffGridTarget >( 0 ) ) );
  }
}

void
EventDeliveryManager::finalize()
{
  // clear the spike buffers
  for( std::vector< std::vector< std::vector< std::vector< Target > > >* >::iterator it = spike_register_5g_.begin(); it != spike_register_5g_.end(); ++it )
  {
    delete (*it);
  };
  spike_register_5g_.clear();

  for( std::vector< std::vector< std::vector< std::vector< OffGridTarget > > >* >::iterator it = off_grid_spike_register_5g_.begin(); it != off_grid_spike_register_5g_.end(); ++it )
  {
    delete (*it);
  };
  off_grid_spike_register_5g_.clear();
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
  assert( kernel().connection_manager.get_min_delay() != 0 );

  for( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    reset_spike_register_5g_( tid );
    resize_spike_register_5g_( tid );
  }

  send_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
  recv_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
  send_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
  recv_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );

  send_recv_count_spike_data_per_rank_ = floor( send_buffer_spike_data_.size() / kernel().mpi_manager.get_num_processes() );
  send_recv_count_spike_data_in_int_per_rank_ = sizeof( SpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_ ;
  send_recv_count_off_grid_spike_data_in_int_per_rank_ = sizeof( OffGridSpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_ ;

  // this should also clear all contained elements
  // so no loop required
  secondary_events_buffer_.clear();
  secondary_events_buffer_.resize( kernel().vp_manager.get_num_threads() );


  // send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces
  // plus the fiunal marker and the done flag for iterations
  // + 1 for the final markers of each thread (invalid_synindex) of secondary
  // events
  // + 1 for the done flag (true) of each process
  // int send_buffer_size = kernel().vp_manager.get_num_threads()
  //         * kernel().connection_manager.get_min_delay()
  //       + 2
  //     > 4
  //   ? kernel().vp_manager.get_num_threads()
  //       * kernel().connection_manager.get_min_delay()
  //     + 2
  //   : 4;
  // int recv_buffer_size =
  //   send_buffer_size * kernel().mpi_manager.get_num_processes();
  // kernel().mpi_manager.set_buffer_sizes( send_buffer_size, recv_buffer_size );
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

// TODO@5g: implement
void
EventDeliveryManager::collocate_buffers_( bool done )
{
  assert( false );
  // // count number of spikes in registers
  // int num_spikes = 0;
  // int num_grid_spikes = 0;
  // int num_offgrid_spikes = 0;
  // int uintsize_secondary_events = 0;

  // std::vector< std::vector< std::vector< unsigned int > > >::iterator i;
  // std::vector< std::vector< unsigned int > >::iterator j;
  // for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  //   for ( j = i->begin(); j != i->end(); ++j )
  //     num_grid_spikes += j->size();

  // std::vector< std::vector< std::vector< OffGridSpike > > >::iterator it;
  // std::vector< std::vector< OffGridSpike > >::iterator jt;
  // for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
  //   for ( jt = it->begin(); jt != it->end(); ++jt )
  //     num_offgrid_spikes += jt->size();

  // // here we need to count the secondary events and take them
  // // into account in the size of the buffers
  // // assume that we already serialized all secondary
  // // events into the secondary_events_buffer_
  // // and that secondary_events_buffer_.size() contains the correct size
  // // of this buffer in units of unsigned int

  // for ( j = secondary_events_buffer_.begin(); j != secondary_events_buffer_.end(); ++j )
  //   uintsize_secondary_events += j->size();

  // // +1 because we need one end marker invalid_synindex
  // // +1 for bool-value done
  // num_spikes = num_grid_spikes + num_offgrid_spikes + uintsize_secondary_events + 2;
  // if ( !off_grid_spiking_ ) // on grid spiking
  // {
  //   // make sure buffers are correctly sized
  //   if ( global_grid_spikes_.size()
  //     != static_cast< unsigned int >( kernel().mpi_manager.get_recv_buffer_size() ) )
  //     global_grid_spikes_.resize( kernel().mpi_manager.get_recv_buffer_size(), 0 );

  //   if ( num_spikes + ( kernel().vp_manager.get_num_threads()
  //                       * kernel().connection_manager.get_min_delay() )
  //     > static_cast< unsigned int >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_grid_spikes_.resize(
  //       ( num_spikes + ( kernel().connection_manager.get_min_delay()
  //                        * kernel().vp_manager.get_num_threads() ) ),
  //       0 );
  //   else if ( local_grid_spikes_.size()
  //     < static_cast< unsigned int >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_grid_spikes_.resize( kernel().mpi_manager.get_send_buffer_size(), 0 );

  //   // collocate the entries of spike_registers into local_grid_spikes__
  //   std::vector< unsigned int >::iterator pos = local_grid_spikes_.begin();
  //   if ( num_offgrid_spikes == 0 )
  //   {
  //     for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  //       for ( j = i->begin(); j != i->end(); ++j )
  //       {
  //         pos = std::copy( j->begin(), j->end(), pos );
  //         *pos = comm_marker_;
  //         ++pos;
  //       }
  //   }
  //   else
  //   {
  //     std::vector< OffGridSpike >::iterator n;
  //     it = offgrid_spike_register_.begin();
  //     for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  //     {
  //       jt = it->begin();
  //       for ( j = i->begin(); j != i->end(); ++j )
  //       {
  //         pos = std::copy( j->begin(), j->end(), pos );
  //         for ( n = jt->begin(); n != jt->end(); ++n )
  //         {
  //           *pos = n->get_gid();
  //           ++pos;
  //         }
  //         *pos = comm_marker_;
  //         ++pos;
  //         ++jt;
  //       }
  //       ++it;
  //     }
  //     for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
  //       for ( jt = it->begin(); jt != it->end(); ++jt )
  //         jt->clear();
  //   }

  //   // remove old spikes from the spike_register_
  //   for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  //     for ( j = i->begin(); j != i->end(); ++j )
  //       j->clear();

  //   // here all spikes have been written to the local_grid_spikes buffer
  //   // pos points to next position in this outgoing communication buffer
  //   for ( j = secondary_events_buffer_.begin(); j != secondary_events_buffer_.end(); ++j )
  //   {
  //     pos = std::copy( j->begin(), j->end(), pos );
  //     j->clear();
  //   }

  //   // end marker after last secondary event
  //   // made sure in resize that this position is still allocated
  //   write_to_comm_buffer( invalid_synindex, pos );
  //   // append the boolean value indicating whether we are done here
  //   write_to_comm_buffer( done, pos );
  // }
  // else // off_grid_spiking
  // {
  //   // make sure buffers are correctly sized
  //   if ( global_offgrid_spikes_.size()
  //     != static_cast< unsigned int >( kernel().mpi_manager.get_recv_buffer_size() ) )
  //     global_offgrid_spikes_.resize(
  //       kernel().mpi_manager.get_recv_buffer_size(), OffGridSpike( 0, 0.0 ) );

  //   if ( num_spikes + ( kernel().vp_manager.get_num_threads()
  //                       * kernel().connection_manager.get_min_delay() )
  //     > static_cast< unsigned int >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_offgrid_spikes_.resize(
  //       ( num_spikes + ( kernel().connection_manager.get_min_delay()
  //                        * kernel().vp_manager.get_num_threads() ) ),
  //       OffGridSpike( 0, 0.0 ) );
  //   else if ( local_offgrid_spikes_.size()
  //     < static_cast< unsigned int >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_offgrid_spikes_.resize(
  //       kernel().mpi_manager.get_send_buffer_size(), OffGridSpike( 0, 0.0 ) );

  //   // collocate the entries of spike_registers into local_offgrid_spikes__
  //   std::vector< OffGridSpike >::iterator pos = local_offgrid_spikes_.begin();
  //   if ( num_grid_spikes == 0 )
  //     for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
  //       for ( jt = it->begin(); jt != it->end(); ++jt )
  //       {
  //         pos = std::copy( jt->begin(), jt->end(), pos );
  //         pos->set_gid( comm_marker_ );
  //         ++pos;
  //       }
  //   else
  //   {
  //     std::vector< unsigned int >::iterator n;
  //     i = spike_register_.begin();
  //     for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
  //     {
  //       j = i->begin();
  //       for ( jt = it->begin(); jt != it->end(); ++jt )
  //       {
  //         pos = std::copy( jt->begin(), jt->end(), pos );
  //         for ( n = j->begin(); n != j->end(); ++n )
  //         {
  //           *pos = OffGridSpike( *n, 0 );
  //           ++pos;
  //         }
  //         pos->set_gid( comm_marker_ );
  //         ++pos;
  //         ++j;
  //       }
  //       ++i;
  //     }
  //     for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  //       for ( j = i->begin(); j != i->end(); ++j )
  //         j->clear();
  //   }

  //   // empty offgrid_spike_register_
  //   for ( it = offgrid_spike_register_.begin(); it != offgrid_spike_register_.end(); ++it )
  //     for ( jt = it->begin(); jt != it->end(); ++jt )
  //       jt->clear();
  // }
}

// returns the done value
bool
EventDeliveryManager::deliver_events( thread t )
{
  assert( false ); // TODO@5g: implement off_grid & secondary events
  // // are we done?
  // bool done = true;

  // // deliver only at beginning of time slice
  // if ( kernel().simulation_manager.get_from_step() > 0 )
  //   return done;

  // SpikeEvent se;

  // std::vector< int > pos( displacements_ );

  // if ( !off_grid_spiking_ ) // on_grid_spiking
  // {
  //   // prepare Time objects for every possible time stamp within min_delay_
  //   std::vector< Time > prepared_timestamps( kernel().connection_manager.get_min_delay() );
  //   for ( size_t lag = 0; lag < ( size_t ) kernel().connection_manager.get_min_delay();
  //         lag++ )
  //   {
  //     prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
  //   }

  //   for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
  //   {
  //     size_t pid = kernel().mpi_manager.get_process_id( vp );
  //     int pos_pid = pos[ pid ];
  //     int lag = kernel().connection_manager.get_min_delay() - 1;
  //     while ( lag >= 0 )
  //     {
  //       index nid = global_grid_spikes_[ pos_pid ];
  //       if ( nid != static_cast< index >( comm_marker_ ) )
  //       {
  //         // tell all local nodes about spikes on remote machines.
  //         se.set_stamp( prepared_timestamps[ lag ] );
  //         se.set_sender_gid( nid );
  //         // kernel().connection_manager.send( t, nid, se );
  //       }
  //       else
  //       {
  //         --lag;
  //       }
  //       ++pos_pid;
  //     }
  //     pos[ pid ] = pos_pid;
  //   }

  //   // here we are done with the spiking events
  //   // pos[pid] for each pid now points to the first entry of
  //   // the secondary events

  //   for ( size_t pid = 0; pid < ( size_t ) kernel().mpi_manager.get_num_processes(); ++pid )
  //   {
  //     std::vector< unsigned int >::iterator readpos = global_grid_spikes_.begin() + pos[ pid ];

  //     while ( true )
  //     {
  //       // we must not use unsigned int for the type, otherwise
  //       // the encoding will be different on JUQUEEN for the
  //       // index written into the buffer and read out of it
  //       synindex synid;
  //       read_from_comm_buffer( synid, readpos );

  //       if ( synid == invalid_synindex )
  //         break;
  //       --readpos;

  //       kernel().model_manager.assert_valid_syn_id( synid );

  //       kernel().model_manager.get_secondary_event_prototype( synid, t ) << readpos;

  //       // kernel().connection_manager.send_secondary(
  //       //   t, kernel().model_manager.get_secondary_event_prototype( synid, t ) );
  //     } // of while (true)

  //     // read the done value of the p-th num_process

  //     // must be a bool (same type as on the sending side)
  //     // otherwise the encoding will be inconsistent on JUQUEEN
  //     bool done_p;
  //     read_from_comm_buffer( done_p, readpos );
  //     done = done && done_p;
  //   }
  // }
  // else // off grid spiking
  // {
  //   // prepare Time objects for every possible time stamp within min_delay_
  //   std::vector< Time > prepared_timestamps( kernel().connection_manager.get_min_delay() );
  //   for ( size_t lag = 0; lag < ( size_t ) kernel().connection_manager.get_min_delay();
  //         lag++ )
  //   {
  //     prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
  //   }

  //   for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
  //   {
  //     size_t pid = kernel().mpi_manager.get_process_id( vp );
  //     int pos_pid = pos[ pid ];
  //     int lag = kernel().connection_manager.get_min_delay() - 1;
  //     while ( lag >= 0 )
  //     {
  //       index nid = global_offgrid_spikes_[ pos_pid ].get_gid();
  //       if ( nid != static_cast< index >( comm_marker_ ) )
  //       {
  //         // tell all local nodes about spikes on remote machines.
  //         se.set_stamp( prepared_timestamps[ lag ] );
  //         se.set_sender_gid( nid );
  //         se.set_offset( global_offgrid_spikes_[ pos_pid ].get_offset() );
  //         // kernel().connection_manager.send( t, nid, se );
  //       }
  //       else
  //       {
  //         --lag;
  //       }
  //       ++pos_pid;
  //     }
  //     pos[ pid ] = pos_pid;
  //   }
  // }

  // return done;
}

// TODO@5g: is replace by gather_spike_data & gather_secondary_data(?)
void
EventDeliveryManager::gather_events( bool done )
{
  assert( false );
  // collocate_buffers_( done );
  // if ( off_grid_spiking_ )
  //   kernel().mpi_manager.communicate(
  //     local_offgrid_spikes_, global_offgrid_spikes_, displacements_ );
  // else
  //   kernel().mpi_manager.communicate( local_grid_spikes_, global_grid_spikes_, displacements_ );
}

void
EventDeliveryManager::gather_spike_data( const thread tid )
{
#pragma omp single
  {
    ++comm_steps_spike_data;
  }

  static unsigned int completed_count;
  const unsigned int half_completed_count = 2 * kernel().vp_manager.get_num_threads();
  const unsigned int max_completed_count = half_completed_count + kernel().vp_manager.get_num_threads();
  size_t me_completed_tid;
  size_t others_completed_tid;

  // const size_t num_grid_spikes = std::accumulate( num_grid_spikes_.begin(), num_grid_spikes_.end(), 0 );
  // const size_t num_off_grid_spikes = std::accumulate( num_off_grid_spikes_.begin(), num_off_grid_spikes_.end(), 0 );

  // can not use while(true) and break in an omp structured block
  bool done = false;
  while ( not done )
  {
#pragma omp single
    {
      completed_count = 0;
      ++comm_rounds_spike_data;
      if ( kernel().mpi_manager.adaptive_spike_buffers() && buffer_size_spike_data_has_changed_ )
        {
          // resize buffer
          send_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
          recv_buffer_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
          send_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
          recv_buffer_off_grid_spike_data_.resize( kernel().mpi_manager.get_buffer_size_spike_data() );
          // calculate new send counts
          send_recv_count_spike_data_per_rank_ = floor( send_buffer_spike_data_.size() / kernel().mpi_manager.get_num_processes() );
          send_recv_count_spike_data_in_int_per_rank_ = sizeof( SpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_ ;
          send_recv_count_off_grid_spike_data_in_int_per_rank_ = sizeof( OffGridSpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_ ;
          buffer_size_spike_data_has_changed_ = false;

          assert( send_buffer_spike_data_.size() <= send_recv_count_spike_data_per_rank_ * kernel().mpi_manager.get_num_processes() );
          assert( send_buffer_off_grid_spike_data_.size() <= send_recv_count_spike_data_per_rank_ * kernel().mpi_manager.get_num_processes() );
        }
    } // of omp single; implicit barrier
    sw_collocate.start();

    const AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );
    SendBufferPosition send_buffer_position( assigned_ranks, send_recv_count_spike_data_per_rank_ );

    if ( not off_grid_spiking_ )
    {
      me_completed_tid = collocate_spike_data_buffers_( tid,
        assigned_ranks,
        send_buffer_position,
        spike_register_5g_,
        send_buffer_spike_data_ );
      me_completed_tid += collocate_spike_data_buffers_( tid,
        assigned_ranks,
        send_buffer_position,
        off_grid_spike_register_5g_,
        send_buffer_spike_data_ );
      set_end_and_invalid_markers_( assigned_ranks, send_buffer_position, send_buffer_spike_data_ );
    }
    else
    {
      me_completed_tid = collocate_spike_data_buffers_( tid,
        assigned_ranks,
        send_buffer_position,
        spike_register_5g_,
        send_buffer_off_grid_spike_data_ );
      me_completed_tid += collocate_spike_data_buffers_( tid,
        assigned_ranks,
        send_buffer_position,
        off_grid_spike_register_5g_,
        send_buffer_off_grid_spike_data_ );
      set_end_and_invalid_markers_( assigned_ranks, send_buffer_position, send_buffer_off_grid_spike_data_ );
    }

#pragma omp barrier
    clean_spike_register_( tid );

#pragma omp atomic
    completed_count += me_completed_tid;
#pragma omp barrier

    if ( completed_count == half_completed_count )
    {
      if ( not off_grid_spiking_ )
      {
        set_complete_marker_spike_data_( assigned_ranks, send_buffer_spike_data_ );
      }
      else
      {
        set_complete_marker_spike_data_( assigned_ranks, send_buffer_off_grid_spike_data_ );
      }
#pragma omp barrier
    }
    sw_collocate.stop();

    sw_communicate.start();
#pragma omp single
    {
      if ( not off_grid_spiking_ )
      {
        unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_spike_data_[0] );
        unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_spike_data_[0] );
        kernel().mpi_manager.communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count_spike_data_in_int_per_rank_ );
      }
      else
      {
        unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_off_grid_spike_data_[0] );
        unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_off_grid_spike_data_[0] );
        kernel().mpi_manager.communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count_off_grid_spike_data_in_int_per_rank_ );
      }
    } // of omp single
    sw_communicate.stop();

    sw_deliver.start();
    if ( not off_grid_spiking_ )
    {
      others_completed_tid = deliver_events_5g_( tid, recv_buffer_spike_data_ );
    }
    else
    {
      others_completed_tid = deliver_events_5g_( tid, recv_buffer_off_grid_spike_data_ );
    }
#pragma omp atomic
    completed_count += others_completed_tid;
#pragma omp barrier
    if ( completed_count == max_completed_count )
    {
      done = true;
    }
    else if ( kernel().mpi_manager.adaptive_spike_buffers() )
    {
#pragma omp single
      {
        buffer_size_spike_data_has_changed_ = kernel().mpi_manager.increase_buffer_size_spike_data();
      }
    }
#pragma omp barrier
    sw_deliver.stop();
  } // of while(true)

  reset_spike_register_5g_( tid );
}

template< typename TargetT, typename SpikeDataT >
bool
EventDeliveryManager::collocate_spike_data_buffers_( const thread tid,
  const AssignedRanks& assigned_ranks,
  SendBufferPosition& send_buffer_position,
  std::vector< std::vector< std::vector< std::vector< TargetT > > >* >& spike_register,
  std::vector< SpikeDataT >& send_buffer )
{
  // reset complete marker
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    const thread lr_idx = rank % assigned_ranks.max_size;
    send_buffer[ send_buffer_position.end[ lr_idx ] - 1 ].reset_marker();
  }

  // whether all spike-register entries have been read
  bool is_spike_register_empty = true;

  for( typename std::vector< std::vector< std::vector< std::vector< TargetT > > >* >::iterator it = spike_register.begin(); it != spike_register.end(); ++it )
  { // only for vectors that are assigned to thread tid
    for ( unsigned int lag = 0; lag < (*(*it))[ tid ].size(); ++lag )
    {
      for ( typename std::vector< TargetT >::iterator iiit = (*(*it))[ tid ][ lag ].begin(); iiit < (*(*it))[ tid ][ lag ].end(); ++iiit )
      { 
	assert ( not iiit->is_processed() );

	// thread-local index of (global) rank of target
	const thread lr_idx = iiit->get_rank() % assigned_ranks.max_size;
	assert( lr_idx < assigned_ranks.size );

	if ( send_buffer_position.idx[ lr_idx ] == send_buffer_position.end[ lr_idx ] )
	{ // send-buffer slot of this assigned rank is full
	  is_spike_register_empty = false;
	  if ( send_buffer_position.num_spike_data_written == send_recv_count_spike_data_per_rank_ * assigned_ranks.size )
	  { // send-buffer slots of all assigned ranks are full
            return is_spike_register_empty;
	  }
          else
          {
            continue;
          }
	}
	else
	{
	  send_buffer[ send_buffer_position.idx[ lr_idx ] ].set( (*iiit).get_tid(), (*iiit).get_syn_index(), (*iiit).get_lcid(), lag, (*iiit).get_offset() );
	  (*iiit).set_processed( true ); // mark entry for removal
	  ++send_buffer_position.idx[ lr_idx ];
	  ++send_buffer_position.num_spike_data_written;
	}
      }
    }
  }

  return is_spike_register_empty;
}

template< typename SpikeDataT >
void
EventDeliveryManager::set_end_and_invalid_markers_( const AssignedRanks& assigned_ranks, const SendBufferPosition& send_buffer_position, std::vector< SpikeDataT >& send_buffer )
{
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const thread lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    if ( send_buffer_position.idx[ lr_idx ] > send_buffer_position.begin[ lr_idx ] )
    {
      assert( send_buffer_position.idx[ lr_idx ] - 1 < send_buffer_position.end[ lr_idx ] );
      send_buffer[ send_buffer_position.idx[ lr_idx ] - 1 ].set_end_marker();
    }
    else
    {
      assert( send_buffer_position.idx[ lr_idx ] == send_buffer_position.begin[ lr_idx ] );
      send_buffer[ send_buffer_position.begin[ lr_idx ] ].set_invalid_marker();
    }
  }
}

template< typename SpikeDataT >
void
EventDeliveryManager::set_complete_marker_spike_data_( const AssignedRanks& assigned_ranks, std::vector< SpikeDataT >& send_buffer )
{
  for ( thread target_rank = assigned_ranks.begin; target_rank < assigned_ranks.end; ++target_rank )
  {
    // use last entry for completion marker
    const thread idx = ( target_rank + 1 ) * send_recv_count_spike_data_per_rank_ - 1;
    send_buffer[ idx ].set_complete_marker();
  }
}

template< typename SpikeDataT >
bool
EventDeliveryManager::deliver_events_5g_( const thread tid, const std::vector< SpikeDataT >& recv_buffer )
{
  bool are_others_completed = true;
  // deliver only at beginning of time slice
  if ( kernel().simulation_manager.get_from_step() > 0 )
  {
    return are_others_completed;
  }

  SpikeEvent se;

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps( kernel().connection_manager.get_min_delay() );
  for ( size_t lag = 0; lag < ( size_t ) kernel().connection_manager.get_min_delay();
        lag++ )
  {
    prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() + Time::step( lag + 1 );
  }

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    // check last entry for completed marker
    if ( not recv_buffer[ ( rank + 1 ) * send_recv_count_spike_data_per_rank_ - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // were spikes sent by this rank?
    if ( recv_buffer[ rank * send_recv_count_spike_data_per_rank_ ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_spike_data_per_rank_; ++i )
    {
      const SpikeDataT& spike_data = recv_buffer[ rank * send_recv_count_spike_data_per_rank_ + i ];

      if ( spike_data.tid == tid )
      {
        se.set_stamp( prepared_timestamps[ spike_data.lag ] );
        se.set_offset( spike_data.get_offset() );
        sw_send.start();
        kernel().connection_manager.send_5g( tid, spike_data.syn_index,
          spike_data.lcid, se );
        sw_send.stop();
      }

      // is this the last spike from this rank?
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
EventDeliveryManager::gather_target_data()
{
  assert( not kernel().connection_manager.is_source_table_cleared() );

  ++comm_steps_target_data;

  // use calloc to zero initialize all entries
  TargetData* send_buffer_target_data = static_cast< TargetData* >( calloc( kernel().mpi_manager.get_buffer_size_target_data(), sizeof( TargetData) ) );
  TargetData* recv_buffer_target_data = static_cast< TargetData* >( calloc( kernel().mpi_manager.get_buffer_size_target_data(), sizeof( TargetData) ) );

  // when a thread does not have any more spike to collocate and when
  // it detects a remote MPI rank is finished this count is increased
  // by 1 in each case. only if all threads are done AND all threads
  // detect all remote ranks are done, we are allowed to stop
  // communication.
  unsigned int completed_count;
  const unsigned int half_completed_count = kernel().vp_manager.get_num_threads();
  const unsigned int max_completed_count = 2 * half_completed_count;

  unsigned int send_recv_count_target_data_per_rank = floor( kernel().mpi_manager.get_buffer_size_target_data() / kernel().mpi_manager.get_num_processes() );
  unsigned int send_recv_count_target_data_in_int_per_rank = sizeof( TargetData ) / sizeof( unsigned int ) * send_recv_count_target_data_per_rank;

#pragma omp parallel shared(completed_count)
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    bool me_completed_tid;
    bool others_completed_tid;
    kernel().connection_manager.prepare_target_table( tid );
    kernel().connection_manager.reset_source_table_entry_point( tid );

    // can not use while(true) and break in an omp structured block
    bool done = false;
    while ( not done )
    {
#pragma omp single
      {
        completed_count = 0;
        ++comm_rounds_target_data;
        if ( kernel().mpi_manager.adaptive_target_buffers() && buffer_size_target_data_has_changed_ )
        {
          free( send_buffer_target_data );
          free( recv_buffer_target_data );
          send_buffer_target_data = static_cast< TargetData* >( calloc( kernel().mpi_manager.get_buffer_size_target_data(), sizeof( TargetData) ) );
          recv_buffer_target_data = static_cast< TargetData* >( calloc( kernel().mpi_manager.get_buffer_size_target_data(), sizeof( TargetData) ) );
          send_recv_count_target_data_per_rank = floor( kernel().mpi_manager.get_buffer_size_target_data() / kernel().mpi_manager.get_num_processes() );
          send_recv_count_target_data_in_int_per_rank = sizeof( TargetData ) / sizeof( unsigned int ) * send_recv_count_target_data_per_rank;
        }
      } // of omp single; implicit barrier
      kernel().connection_manager.restore_source_table_entry_point( tid );

      sw_collocate_target_data.start();
      me_completed_tid = collocate_target_data_buffers_( tid, send_recv_count_target_data_per_rank, send_buffer_target_data );
#pragma omp atomic
      completed_count += me_completed_tid;
#pragma omp barrier
      sw_collocate_target_data.stop();
      if ( completed_count == half_completed_count )
      {
        set_complete_marker_target_data_( tid, send_recv_count_target_data_per_rank, send_buffer_target_data );
#pragma omp barrier
      }
      kernel().connection_manager.save_source_table_entry_point( tid );
#pragma omp barrier
      kernel().connection_manager.clean_source_table( tid );
#pragma omp single
      {
        sw_communicate_target_data.start();
        unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_target_data[0] );
        unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_target_data[0] );
        kernel().mpi_manager.communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count_target_data_in_int_per_rank );
        sw_communicate_target_data.stop();
      } // of omp single

      sw_deliver_target_data.start();
      others_completed_tid = distribute_target_data_buffers_( tid, send_recv_count_target_data_per_rank, recv_buffer_target_data );
      sw_deliver_target_data.stop();

#pragma omp atomic
      completed_count += others_completed_tid;
#pragma omp barrier
      if ( completed_count == max_completed_count )
      {
        done = true;
      }
      else if ( kernel().mpi_manager.adaptive_target_buffers() )
      {
#pragma omp single
        {
          buffer_size_target_data_has_changed_ = kernel().mpi_manager.increase_buffer_size_target_data();
        }
      }
#pragma omp barrier
    } // of while(true)
    kernel().connection_manager.clear_source_table( tid );
  } // of omp parallel

  free( send_buffer_target_data );
  free( recv_buffer_target_data );
}

bool
EventDeliveryManager::collocate_target_data_buffers_( const thread tid, const unsigned int num_target_data_per_rank, TargetData* send_buffer )
{
  const AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  unsigned int num_target_data_written = 0;
  thread target_rank;
  TargetData next_target_data;
  bool valid_next_target_data;
  bool is_source_table_read = true;

  // no ranks to process for this thread
  if ( assigned_ranks.begin == assigned_ranks.end )
  {
    kernel().connection_manager.no_targets_to_process( tid );
    return is_source_table_read;
  }

  // build lookup table for buffer indices and reset marker
  std::vector< unsigned int > send_buffer_idx( assigned_ranks.size, 0 );
  std::vector< unsigned int > send_buffer_begin( assigned_ranks.size, 0 );
  std::vector< unsigned int > send_buffer_end( assigned_ranks.size, 0 );
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const thread lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    send_buffer_idx[ lr_idx ] = rank * num_target_data_per_rank;
    send_buffer_begin[ lr_idx ] = rank * num_target_data_per_rank;
    send_buffer_end[ lr_idx ] = (rank + 1) * num_target_data_per_rank;
    send_buffer[ send_buffer_end[ lr_idx ] - 1 ].reset_marker();
  }

  while ( true )
  {
    valid_next_target_data = kernel().connection_manager.get_next_target_data( tid, assigned_ranks.begin, assigned_ranks.end, target_rank, next_target_data );
    if ( valid_next_target_data ) // add valid entry to MPI buffer
    {
      const unsigned int lr_idx = target_rank % assigned_ranks.max_size;
      if ( send_buffer_idx[ lr_idx ] == send_buffer_end[ lr_idx ] )
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
        if ( num_target_data_written == ( num_target_data_per_rank * assigned_ranks.size ) ) // buffer is full
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
        send_buffer[ send_buffer_idx[ lr_idx ] ] = next_target_data;
        ++send_buffer_idx[ lr_idx ];
        ++num_target_data_written;
      }
    }
    else  // all connections have been processed
    {
      // mark end of valid data for each rank
      for ( thread target_rank = assigned_ranks.begin; target_rank < assigned_ranks.end; ++target_rank )
      {
        const thread lr_idx = target_rank % assigned_ranks.max_size;
        if ( send_buffer_idx[ lr_idx ] > send_buffer_begin[ lr_idx ] )
        {
          send_buffer[ send_buffer_idx[ lr_idx ] - 1 ].set_end_marker();
        }
        else
        {
          send_buffer[ send_buffer_begin[ lr_idx ] ].set_invalid_marker();
        }
      }
      return is_source_table_read;
    } // of else
  } // of while(true)
}

void
nest::EventDeliveryManager::set_complete_marker_target_data_( const thread tid, const unsigned int num_target_data_per_rank, TargetData* send_buffer )
{
  const AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  for ( thread target_rank = assigned_ranks.begin; target_rank < assigned_ranks.end; ++target_rank )
  {
    const thread idx = ( target_rank + 1 ) * num_target_data_per_rank - 1;
    send_buffer[ idx ].set_complete_marker();
  }
}

bool
nest::EventDeliveryManager::distribute_target_data_buffers_( const thread tid, const unsigned int num_target_data_per_rank, TargetData const* const recv_buffer )
{
  bool are_others_completed = true;

  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    // check last entry for completed marker
    if ( not recv_buffer[ ( rank + 1 ) * num_target_data_per_rank - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // were spikes sent by this rank?
    if ( recv_buffer[ rank * num_target_data_per_rank ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < num_target_data_per_rank; ++i )
    {
      const TargetData& target_data = recv_buffer[ rank * num_target_data_per_rank + i ];
      if ( target_data.tid == tid )
      {
        kernel().connection_manager.add_target( tid, target_data );
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
