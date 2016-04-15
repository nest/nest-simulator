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
#include "node_manager_impl.h"
#include "connection_builder_manager.h"
#include "connection_builder_manager_impl.h"
#include "event_delivery_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
EventDeliveryManager::EventDeliveryManager()
  : off_grid_spiking_( false )
  , moduli_()
  , slice_moduli_()
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

  for( std::vector< std::vector< std::vector< std::vector< Target > > >* >::iterator it = spike_register_5g_.begin(); it != spike_register_5g_.end(); ++it )
  {
    delete (*it);
  };
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

  spike_register_table_.configure();

  const unsigned int num_threads = kernel().vp_manager.get_num_threads();
  spike_register_5g_.resize( num_threads, 0 );
  for( thread tid = 0; tid < num_threads; ++tid )
  {
    assert( spike_register_5g_[ tid ] == 0 );
    spike_register_5g_[ tid ] = new std::vector< std::vector< std::vector< Target > > >( num_threads, std::vector< std::vector< Target > >( kernel().connection_builder_manager.get_min_delay(), std::vector< Target >( 0 ) ) );
  }

  send_buffer_spike_data_.resize( mpi_buffer_size_spike_data );
  recv_buffer_spike_data_.resize( mpi_buffer_size_spike_data );

  send_recv_count_spike_data_per_rank_ = floor( send_buffer_spike_data_.size() / kernel().mpi_manager.get_num_processes() );
  send_recv_count_spike_data_in_int_per_rank_ = sizeof( SpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_ ;

  offgrid_spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  offgrid_spike_register_.resize( kernel().vp_manager.get_num_threads(),
    std::vector< std::vector< OffGridSpike > >(
                                    kernel().connection_builder_manager.get_min_delay() ) );
  for ( size_t j = 0; j < offgrid_spike_register_.size(); ++j )
    for ( size_t k = 0; k < offgrid_spike_register_[ j ].size(); ++k )
      offgrid_spike_register_[ j ][ k ].clear();


  // this should also clear all contained elements
  // so no loop required
  secondary_events_buffer_.clear();
  secondary_events_buffer_.resize( kernel().vp_manager.get_num_threads() );


  // send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces
  // plus the fiunal marker and the done flag for iterations
  // + 1 for the final markers of each thread (invalid_synindex) of secondary events
  // + 1 for the done flag (true) of each process
  int send_buffer_size =
    kernel().vp_manager.get_num_threads() * kernel().connection_builder_manager.get_min_delay() + 2
      > 4
    ? kernel().vp_manager.get_num_threads() * kernel().connection_builder_manager.get_min_delay()
      + 2
    : 4;
  int recv_buffer_size = send_buffer_size * kernel().mpi_manager.get_num_processes();
  kernel().mpi_manager.set_buffer_sizes( send_buffer_size, recv_buffer_size );

  // DEC cxx required 0U literal, HEP 2007-03-26
  local_grid_spikes_.clear();
  local_grid_spikes_.resize( send_buffer_size, 0U );
  local_offgrid_spikes_.clear();
  local_offgrid_spikes_.resize( send_buffer_size, OffGridSpike( 0, 0.0 ) );

  global_grid_spikes_.clear();
  global_grid_spikes_.resize( recv_buffer_size, 0U );

  // insert the end marker for payload event (==invalid_synindex)
  // and insert the done flag (==true)
  // after min_delay 0's (== comm_marker)
  // use the template functions defined in event.h
  // this only needs to be done for one process, because displacements is set to 0
  // so all processes initially read out the same positions in the
  // global spike buffer
  std::vector< uint_t >::iterator pos = global_grid_spikes_.begin()
    + kernel().vp_manager.get_num_threads() * kernel().connection_builder_manager.get_min_delay();
  write_to_comm_buffer( invalid_synindex, pos );
  write_to_comm_buffer( true, pos );

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
    slice_moduli_[ d ] =
      ( ( kernel().simulation_manager.get_clock().get_steps() + d ) / min_delay ) % nbuff;
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

  // std::vector< std::vector< std::vector< uint_t > > >::iterator i;
  // std::vector< std::vector< uint_t > >::iterator j;
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
  // // of this buffer in units of uint_t

  // for ( j = secondary_events_buffer_.begin(); j != secondary_events_buffer_.end(); ++j )
  //   uintsize_secondary_events += j->size();

  // // +1 because we need one end marker invalid_synindex
  // // +1 for bool-value done
  // num_spikes = num_grid_spikes + num_offgrid_spikes + uintsize_secondary_events + 2;
  // if ( !off_grid_spiking_ ) // on grid spiking
  // {
  //   // make sure buffers are correctly sized
  //   if ( global_grid_spikes_.size()
  //     != static_cast< uint_t >( kernel().mpi_manager.get_recv_buffer_size() ) )
  //     global_grid_spikes_.resize( kernel().mpi_manager.get_recv_buffer_size(), 0 );

  //   if ( num_spikes + ( kernel().vp_manager.get_num_threads()
  //                       * kernel().connection_builder_manager.get_min_delay() )
  //     > static_cast< uint_t >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_grid_spikes_.resize(
  //       ( num_spikes + ( kernel().connection_builder_manager.get_min_delay()
  //                        * kernel().vp_manager.get_num_threads() ) ),
  //       0 );
  //   else if ( local_grid_spikes_.size()
  //     < static_cast< uint_t >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_grid_spikes_.resize( kernel().mpi_manager.get_send_buffer_size(), 0 );

  //   // collocate the entries of spike_registers into local_grid_spikes__
  //   std::vector< uint_t >::iterator pos = local_grid_spikes_.begin();
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
  //     != static_cast< uint_t >( kernel().mpi_manager.get_recv_buffer_size() ) )
  //     global_offgrid_spikes_.resize(
  //       kernel().mpi_manager.get_recv_buffer_size(), OffGridSpike( 0, 0.0 ) );

  //   if ( num_spikes + ( kernel().vp_manager.get_num_threads()
  //                       * kernel().connection_builder_manager.get_min_delay() )
  //     > static_cast< uint_t >( kernel().mpi_manager.get_send_buffer_size() ) )
  //     local_offgrid_spikes_.resize(
  //       ( num_spikes + ( kernel().connection_builder_manager.get_min_delay()
  //                        * kernel().vp_manager.get_num_threads() ) ),
  //       OffGridSpike( 0, 0.0 ) );
  //   else if ( local_offgrid_spikes_.size()
  //     < static_cast< uint_t >( kernel().mpi_manager.get_send_buffer_size() ) )
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
  //     std::vector< uint_t >::iterator n;
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
  //   std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
  //   for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
  //         lag++ )
  //   {
  //     prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
  //   }

  //   for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
  //   {
  //     size_t pid = kernel().mpi_manager.get_process_id( vp );
  //     int pos_pid = pos[ pid ];
  //     int lag = kernel().connection_builder_manager.get_min_delay() - 1;
  //     while ( lag >= 0 )
  //     {
  //       index nid = global_grid_spikes_[ pos_pid ];
  //       if ( nid != static_cast< index >( comm_marker_ ) )
  //       {
  //         // tell all local nodes about spikes on remote machines.
  //         se.set_stamp( prepared_timestamps[ lag ] );
  //         se.set_sender_gid( nid );
  //         // kernel().connection_builder_manager.send( t, nid, se );
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
  //     std::vector< uint_t >::iterator readpos = global_grid_spikes_.begin() + pos[ pid ];

  //     while ( true )
  //     {
  //       // we must not use uint_t for the type, otherwise
  //       // the encoding will be different on JUQUEEN for the
  //       // index written into the buffer and read out of it
  //       synindex synid;
  //       read_from_comm_buffer( synid, readpos );

  //       if ( synid == invalid_synindex )
  //         break;
  //       --readpos;

  //       kernel().model_manager.assert_valid_syn_id( synid );

  //       kernel().model_manager.get_secondary_event_prototype( synid, t ) << readpos;

  //       // kernel().connection_builder_manager.send_secondary(
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
  //   std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
  //   for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
  //         lag++ )
  //   {
  //     prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() - Time::step( lag );
  //   }

  //   for ( size_t vp = 0; vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes(); ++vp )
  //   {
  //     size_t pid = kernel().mpi_manager.get_process_id( vp );
  //     int pos_pid = pos[ pid ];
  //     int lag = kernel().connection_builder_manager.get_min_delay() - 1;
  //     while ( lag >= 0 )
  //     {
  //       index nid = global_offgrid_spikes_[ pos_pid ].get_gid();
  //       if ( nid != static_cast< index >( comm_marker_ ) )
  //       {
  //         // tell all local nodes about spikes on remote machines.
  //         se.set_stamp( prepared_timestamps[ lag ] );
  //         se.set_sender_gid( nid );
  //         se.set_offset( global_offgrid_spikes_[ pos_pid ].get_offset() );
  //         // kernel().connection_builder_manager.send( t, nid, se );
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
  sw_reset_restore_save.start();
  spike_register_table_.reset_entry_point( tid );
  sw_reset_restore_save.start();

  static unsigned int completed_count;
  const unsigned int half_completed_count = kernel().vp_manager.get_num_threads();
  const unsigned int max_completed_count = 2 * half_completed_count;
  bool me_completed_tid;
  bool others_completed_tid;

  // can not use while(true) and break in an omp structured block
  bool done = false;
  while ( not done )
  {
#pragma omp single
    {
      completed_count = 0;
    } // of omp single; implicit barrier
    sw_reset_restore_save.start();
    spike_register_table_.restore_entry_point( tid ); // TODO@5g: move spike_register calls into collocation of buffers?!
    kernel().connection_builder_manager.reset_current_index_target_table( tid );
    sw_reset_restore_save.stop();
      
#pragma omp barrier
    sw_collocate.start();
    me_completed_tid = collocate_spike_data_buffers_thr_( tid );
    //me_completed_tid = collocate_spike_data_buffers_( tid );
    sw_collocate.stop();

#pragma omp barrier
    clean_spike_register_5g_( tid );

#pragma omp atomic
    completed_count += me_completed_tid;
#pragma omp barrier
    sw_reset_restore_save.start();

    if ( completed_count == half_completed_count )
    {
      set_complete_marker_spike_data_( tid );
#pragma omp barrier
    }

    spike_register_table_.save_entry_point( tid );
    sw_reset_restore_save.stop();

   sw_communicate.start();
#pragma omp single
    {
      unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_spike_data_[0] );
      unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_spike_data_[0] );
      kernel().mpi_manager.communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count_spike_data_in_int_per_rank_ );
    } // of omp single
    sw_communicate.stop();
    sw_check.start();
    sw_check.stop();
    sw_deliver.start();
    others_completed_tid = deliver_events_5g_( tid );
#pragma omp atomic
    completed_count += others_completed_tid;
#pragma omp barrier
    sw_deliver.stop();
    if ( completed_count == max_completed_count )
    {
      done = true;
    }
#pragma omp barrier
  } // of while(true)
  spike_register_table_.toggle_target_processed_flags( tid );
  spike_register_table_.clear( tid );
  reset_spike_register_5g_( tid );
}

bool
EventDeliveryManager::collocate_spike_data_buffers_thr_( const thread tid )
{
  AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  // number of spike-register entries that have been read
  unsigned int num_spike_data_written = 0;

  // [send_buffer_idx, send_buffer_end) defines the send-buffer slot for each assigned rank
  std::vector< unsigned int > send_buffer_idx( assigned_ranks.size, 0 );
  std::vector< unsigned int > send_buffer_begin( assigned_ranks.size, 0 );
  std::vector< unsigned int > send_buffer_end( assigned_ranks.size, 0 );
  for ( unsigned int rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const unsigned int lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    send_buffer_idx[ lr_idx ] = rank * send_recv_count_spike_data_per_rank_;
    send_buffer_begin[ lr_idx ] = rank * send_recv_count_spike_data_per_rank_;
    send_buffer_end[ lr_idx ] = (rank + 1) * send_recv_count_spike_data_per_rank_;
    send_buffer_spike_data_[ send_buffer_end[ lr_idx ] - 1 ].reset_marker();
  }

  // whether all spike-register entries have been read
  bool is_spike_register_empty = true;

  for( std::vector< std::vector< std::vector< std::vector< Target > > >* >::iterator it = spike_register_5g_.begin(); it != spike_register_5g_.end(); ++it )
  { // only for vectors that are assigned to thread tid
    for ( unsigned int lag = 0; lag < (*(*it))[ tid ].size(); ++lag )
    {
      for ( std::vector< Target >::iterator iiit = (*(*it))[ tid ][ lag ].begin(); iiit < (*(*it))[ tid ][ lag ].end(); ++iiit )
      { 
	assert ( not iiit->is_processed() );

	// thread-local index of (global) rank of target
	const unsigned int lr_idx = iiit->rank % assigned_ranks.max_size;
	assert( lr_idx < assigned_ranks.size );

	if ( send_buffer_idx[ lr_idx ] == send_buffer_end[ lr_idx ] )
	{ // send-buffer slot of this assigned rank is full
	  is_spike_register_empty = false;
	  if ( num_spike_data_written == send_recv_count_spike_data_per_rank_ * assigned_ranks.size )
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
	  send_buffer_spike_data_[ send_buffer_idx[ lr_idx ] ].set( (*iiit).tid, (*iiit).syn_index, (*iiit).lcid, lag );
	  (*iiit).set_processed(); // mark entry for removal
	  ++send_buffer_idx[ lr_idx ];
	  ++num_spike_data_written;
	}
      }
    }
  }

  for ( unsigned int rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const unsigned int lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    if ( send_buffer_idx[ lr_idx ] > send_buffer_begin[ lr_idx ] )
    {
      assert( send_buffer_idx[ lr_idx ] - 1 < send_buffer_end[ lr_idx ] );
      send_buffer_spike_data_[ send_buffer_idx[ lr_idx ] - 1 ].set_end_marker();
    }
    else
    {
      assert( send_buffer_idx[ lr_idx ] == send_buffer_begin[ lr_idx ] );
      send_buffer_spike_data_[ send_buffer_begin[ lr_idx ] ].set_invalid_marker();
    }
  }

  return is_spike_register_empty;
}

bool
EventDeliveryManager::collocate_spike_data_buffers_( const thread tid )
{
  assert( false );
  // TODO@5g: documentation
  const unsigned int num_assigned_ranks_per_thread = kernel().vp_manager.get_num_assigned_ranks_per_thread();
  const unsigned int rank_start = kernel().vp_manager.get_start_rank_per_thread( tid );
  const unsigned int rank_end = kernel().vp_manager.get_end_rank_per_thread( tid, rank_start, num_assigned_ranks_per_thread );
  // store how far each segment in sendbuffer is filled
  std::vector< unsigned int > send_buffer_offset( num_assigned_ranks_per_thread, 0 );
  unsigned int sum_send_buffer_offset = 0;
  index target_rank;
  SpikeData next_spike_data;
  bool valid_next_spike_data;
  bool is_buffer_untouched = true;

  // TODO@5g: for just one rank, only one thread fills MPI buffer
  if ( rank_start == rank_end )  // no ranks to process for this thread
  {
    return is_buffer_untouched;
  }

  while ( true )
  {
    if ( sum_send_buffer_offset == ( send_recv_count_spike_data_per_rank_ * num_assigned_ranks_per_thread ) )
    {
      return is_buffer_untouched;
    }
    else
    {
      valid_next_spike_data = spike_register_table_.get_next_spike_data( tid, target_rank, next_spike_data, rank_start, rank_end );
      if ( valid_next_spike_data )
      {
        const unsigned int target_rank_index = target_rank - rank_start;
        if ( send_buffer_offset[ target_rank_index ] < send_recv_count_spike_data_per_rank_ )
        {
          const unsigned int idx = target_rank * send_recv_count_spike_data_per_rank_ + send_buffer_offset[ target_rank_index ];
          send_buffer_spike_data_[ idx ] = next_spike_data;
          ++send_buffer_offset[ target_rank_index ];
          ++sum_send_buffer_offset;
          is_buffer_untouched = false;
        }
        else
        {
          spike_register_table_.reject_last_spike_data( tid );
          spike_register_table_.save_entry_point( tid );
        }
      }
      else // all spikes have been processed
      {
        // mark end of valid data for each rank
        for ( unsigned int target_rank = rank_start; target_rank < rank_end; ++target_rank )
        {
          const unsigned int target_rank_index = target_rank - rank_start;
          if ( send_buffer_offset[ target_rank_index ] < send_recv_count_spike_data_per_rank_ )
          {
            const unsigned int idx = target_rank * send_recv_count_spike_data_per_rank_ + send_buffer_offset[ target_rank_index ];
            send_buffer_spike_data_[ idx ].set_end_marker();
          }
        }
        return is_buffer_untouched;
      } // of else
    }
  } // of while(true)
}

void
EventDeliveryManager::set_complete_marker_spike_data_( const thread tid )
{
  AssignedRanks assigned_ranks = kernel().vp_manager.get_assigned_ranks( tid );

  for ( unsigned int target_rank = assigned_ranks.begin; target_rank < assigned_ranks.end; ++target_rank )
  {
    // use last entry for completion marker
    const unsigned int idx = ( target_rank + 1 ) * send_recv_count_spike_data_per_rank_ - 1;
    send_buffer_spike_data_[ idx ].set_complete_marker();
  }
}

bool
EventDeliveryManager::deliver_events_5g_( const thread tid )
{
  bool are_others_completed = true;
  // deliver only at beginning of time slice
  if ( kernel().simulation_manager.get_from_step() > 0 )
  {
    return are_others_completed;
  }

  SpikeEvent se;

  // prepare Time objects for every possible time stamp within min_delay_
  std::vector< Time > prepared_timestamps( kernel().connection_builder_manager.get_min_delay() );
  for ( size_t lag = 0; lag < ( size_t ) kernel().connection_builder_manager.get_min_delay();
        lag++ )
  {
    prepared_timestamps[ lag ] = kernel().simulation_manager.get_clock() + Time::step( lag + 1 );
  }

  for ( unsigned int rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    // check last entry for completed marker
    if ( not recv_buffer_spike_data_[ ( rank + 1 ) * send_recv_count_spike_data_per_rank_ - 1 ].is_complete_marker() )
    {
      are_others_completed = false;
    }

    // were spikes sent by this rank?
    if ( recv_buffer_spike_data_[ rank * send_recv_count_spike_data_per_rank_ ].is_invalid_marker() )
    {
      continue;
    }

    for ( unsigned int i = 0; i < send_recv_count_spike_data_per_rank_; ++i )
    {
      SpikeData& spike_data = recv_buffer_spike_data_[ rank * send_recv_count_spike_data_per_rank_ + i ];

      if ( spike_data.tid == tid )
      {
        se.set_stamp( prepared_timestamps[ spike_data.lag ] );
        kernel().connection_builder_manager.send_5g( tid, spike_data.syn_index,
                                                     spike_data.lcid, se );
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
  assert( not kernel().connection_builder_manager.is_source_table_cleared() );

  std::vector< TargetData > send_buffer_target_data( mpi_buffer_size_target_data );
  std::vector< TargetData > recv_buffer_target_data( mpi_buffer_size_target_data );

  // when a thread does not have any more spike to collocate and when
  // it detects a remote MPI rank is finished this cound is increased
  // by 1 in each case. only if all threads are done AND all threads
  // detect all remote ranks are done, we are allowed to stop
  // communication.
  unsigned int completed_count;
  unsigned int half_completed_count = kernel().vp_manager.get_num_threads();
  unsigned int max_completed_count = 2 * half_completed_count;

  const unsigned int send_recv_count_target_data_per_rank = floor( send_buffer_target_data.size() / kernel().mpi_manager.get_num_processes() );
  const unsigned int send_recv_count_target_data_in_int_per_rank = sizeof( TargetData ) / sizeof( unsigned int ) * send_recv_count_target_data_per_rank;

#pragma omp parallel shared(completed_count)
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    bool me_completed_tid;
    bool others_completed_tid;
    kernel().connection_builder_manager.prepare_target_table( tid );
    kernel().connection_builder_manager.reset_source_table_entry_point( tid );

    // can not use while(true) and break in an omp structured block
    bool done = false;
    while ( not done )
    {
#pragma omp single
      {
        completed_count = 0;
      } // of omp single; implicit barrier
      kernel().connection_builder_manager.restore_source_table_entry_point( tid );

      me_completed_tid = collocate_target_data_buffers_( tid, send_recv_count_target_data_per_rank, send_buffer_target_data );
#pragma omp atomic
      completed_count += me_completed_tid;
#pragma omp barrier
      if ( completed_count == half_completed_count )
      {
        set_complete_marker_target_data_( tid, send_recv_count_target_data_per_rank, send_buffer_target_data );
#pragma omp barrier
      }

      kernel().connection_builder_manager.save_source_table_entry_point( tid );      
#pragma omp single
      {
        unsigned int* send_buffer_int = reinterpret_cast< unsigned int* >( &send_buffer_target_data[0] );
        unsigned int* recv_buffer_int = reinterpret_cast< unsigned int* >( &recv_buffer_target_data[0] );
        kernel().mpi_manager.communicate_Alltoall( send_buffer_int, recv_buffer_int, send_recv_count_target_data_in_int_per_rank );
      } // of omp single

      others_completed_tid = distribute_target_data_buffers_( tid, send_recv_count_target_data_per_rank, recv_buffer_target_data );

#pragma omp atomic
      completed_count += others_completed_tid;
#pragma omp barrier
      if ( completed_count == max_completed_count )
      {
        done = true;
      }
#pragma omp barrier
    } // of while(true)
  } // of omp parallel
}

bool
EventDeliveryManager::collocate_target_data_buffers_( const thread tid, const unsigned int num_target_data_per_rank, std::vector< TargetData >& send_buffer )
{
  const unsigned int num_assigned_ranks_per_thread = kernel().vp_manager.get_num_assigned_ranks_per_thread();
  const unsigned int rank_start = kernel().vp_manager.get_start_rank_per_thread( tid );
  const unsigned int rank_end = kernel().vp_manager.get_end_rank_per_thread( tid, rank_start, num_assigned_ranks_per_thread );
  // store how far each segment in sendbuffer is filled
  std::vector< unsigned int > send_buffer_offset( num_assigned_ranks_per_thread, 0 );
  unsigned int sum_send_buffer_offset = 0;
  index target_rank;
  TargetData next_target_data;
  bool valid_next_target_data;
  bool is_buffer_untouched = true;

  if ( rank_start == rank_end ) // no ranks to process for this thread
  {
    return is_buffer_untouched;
  }

  while ( true )
  {
    if ( sum_send_buffer_offset == ( num_target_data_per_rank * num_assigned_ranks_per_thread ) ) // buffer is full
    {
      return is_buffer_untouched;
    }
    else
    {
      valid_next_target_data = kernel().connection_builder_manager.get_next_target_data( tid, target_rank, next_target_data, rank_start, rank_end );
      if ( valid_next_target_data ) // add valid entry to MPI buffer
      {
        const thread target_rank_index = target_rank - rank_start;
        if ( send_buffer_offset[ target_rank_index ] < num_target_data_per_rank )
        {
          const unsigned int idx = target_rank * num_target_data_per_rank + send_buffer_offset[ target_rank_index ];
          send_buffer[ idx ] = next_target_data;
          ++send_buffer_offset[ target_rank_index ];
          ++sum_send_buffer_offset;
          is_buffer_untouched = false;
        }
        else
        {
          kernel().connection_builder_manager.reject_last_target_data( tid );
          kernel().connection_builder_manager.save_source_table_entry_point( tid );
        }
      }
      else  // all connections have been processed
      {
        // mark end of valid data for each rank
        for ( unsigned int target_rank = rank_start; target_rank < rank_end; ++target_rank )
        {
          const thread target_rank_index = target_rank - rank_start;
          if ( send_buffer_offset[ target_rank_index ] < num_target_data_per_rank )
          {
            const unsigned int idx = target_rank * num_target_data_per_rank + send_buffer_offset[ target_rank_index ];
            send_buffer[ idx ].set_end_marker();
          }
        }
        return is_buffer_untouched;
      } // of else
    }
  } // of while(true)
}

void
nest::EventDeliveryManager::set_complete_marker_target_data_( const thread tid, const unsigned int num_target_data_per_rank, std::vector< TargetData >& send_buffer )
{
  const unsigned int num_assigned_ranks_per_thread = kernel().vp_manager.get_num_assigned_ranks_per_thread();
  const unsigned int rank_start = kernel().vp_manager.get_start_rank_per_thread( tid );
  const unsigned int rank_end = kernel().vp_manager.get_end_rank_per_thread( tid, rank_start, num_assigned_ranks_per_thread );

  for ( unsigned int target_rank = rank_start; target_rank < rank_end; ++target_rank )
  {
    const unsigned int idx = target_rank * num_target_data_per_rank;
    send_buffer[ idx ].set_complete_marker();
  }
}

bool
nest::EventDeliveryManager::distribute_target_data_buffers_( const thread tid, const unsigned int num_target_data_per_rank, const std::vector< TargetData >& recv_buffer )
{
  bool are_others_completed = true;
  for ( unsigned int rank = 0; rank < kernel().mpi_manager.get_num_processes(); ++rank )
  {
    if ( not recv_buffer[ rank * num_target_data_per_rank ].is_complete_marker() )
    {
      are_others_completed = false;
      for ( unsigned int i = 0; i < num_target_data_per_rank; ++i )
      {
        const TargetData& target_data = recv_buffer[ rank * num_target_data_per_rank + i ];
        if ( target_data.is_end_marker() )
        {
          break;
        }
        else if ( target_data.tid == tid )
        {
          kernel().connection_builder_manager.add_target( tid, target_data );
        }
        else
        {
          continue;
        }
      }
    }
  }
  return are_others_completed;
}

} // of namespace nest
