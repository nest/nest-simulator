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

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
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
  , spike_register_()
  , offgrid_spike_register_()
  , local_grid_spikes_()
  , global_grid_spikes_()
  , local_offgrid_spikes_()
  , global_offgrid_spikes_()
  , displacements_()
  , comm_marker_( 0 )
  , time_collocate_( 0.0 )
  , time_communicate_( 0.0 )
  , local_spike_counter_( 0U )
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
  // ensures that ResetKernel resets off_grid_spiking_
  off_grid_spiking_ = false;
  init_moduli();
  reset_timers_counters();
}

void
EventDeliveryManager::finalize()
{
  // clear the buffers
  local_grid_spikes_.clear();
  global_grid_spikes_.clear();
  local_offgrid_spikes_.clear();
  global_offgrid_spikes_.clear();
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
    dict, names::local_spike_counter, local_spike_counter_ );
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

  spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  spike_register_.resize( kernel().vp_manager.get_num_threads(),
    std::vector< std::vector< unsigned int > >(
                            kernel().connection_manager.get_min_delay() ) );
  for ( size_t j = 0; j < spike_register_.size(); ++j )
  {
    for ( size_t k = 0; k < spike_register_[ j ].size(); ++k )
    {
      spike_register_[ j ][ k ].clear();
    }
  }
  offgrid_spike_register_.clear();
  // the following line does not compile with gcc <= 3.3.5
  offgrid_spike_register_.resize(
    kernel().vp_manager.get_num_threads(),
    std::vector< std::vector< OffGridSpike > >(
      kernel().connection_manager.get_min_delay() ) );
  for ( size_t j = 0; j < offgrid_spike_register_.size(); ++j )
  {
    for ( size_t k = 0; k < offgrid_spike_register_[ j ].size(); ++k )
    {
      offgrid_spike_register_[ j ][ k ].clear();
    }
  }

  // this should also clear all contained elements
  // so no loop required
  secondary_events_buffer_.clear();
  secondary_events_buffer_.resize( kernel().vp_manager.get_num_threads() );


  // send_buffer must be >= 2 as the 'overflow' signal takes up 2 spaces
  // plus the final marker and the done flag for iterations
  // + 1 for the final markers of each thread (invalid_synindex) of secondary
  // events
  // + 1 for the done flag (true) of each process
  int send_buffer_size = kernel().vp_manager.get_num_threads()
          * kernel().connection_manager.get_min_delay()
        + 2
      > 4
    ? kernel().vp_manager.get_num_threads()
        * kernel().connection_manager.get_min_delay()
      + 2
    : 4;
  int recv_buffer_size =
    send_buffer_size * kernel().mpi_manager.get_num_processes();
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
  // this only needs to be done for one process, because displacements is set to
  // 0 so all processes initially read out the same positions in the global
  // spike buffer
  std::vector< unsigned int >::iterator pos = global_grid_spikes_.begin()
    + kernel().vp_manager.get_num_threads()
      * kernel().connection_manager.get_min_delay();
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
  local_spike_counter_ = 0U;
}

void
EventDeliveryManager::collocate_buffers_( bool done )
{
  // count number of spikes in registers
  int num_spikes = 0;
  int num_grid_spikes = 0;
  int num_offgrid_spikes = 0;
  int uintsize_secondary_events = 0;

  std::vector< std::vector< std::vector< unsigned int > > >::iterator i;
  std::vector< std::vector< unsigned int > >::iterator j;
  for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
  {
    for ( j = i->begin(); j != i->end(); ++j )
    {
      num_grid_spikes += j->size();
    }
  }
  std::vector< std::vector< std::vector< OffGridSpike > > >::iterator it;
  std::vector< std::vector< OffGridSpike > >::iterator jt;
  for ( it = offgrid_spike_register_.begin();
        it != offgrid_spike_register_.end();
        ++it )
  {
    for ( jt = it->begin(); jt != it->end(); ++jt )
    {
      num_offgrid_spikes += jt->size();
    }
  } // accumulate number of generated spikes in the local spike counter
  local_spike_counter_ += num_grid_spikes + num_offgrid_spikes;

  // here we need to count the secondary events and take them
  // into account in the size of the buffers
  // assume that we already serialized all secondary
  // events into the secondary_events_buffer_
  // and that secondary_events_buffer_.size() contains the correct size
  // of this buffer in units of unsigned int

  for ( j = secondary_events_buffer_.begin();
        j != secondary_events_buffer_.end();
        ++j )
  {
    uintsize_secondary_events += j->size();
  }
  // +1 because we need one end marker invalid_synindex
  // +1 for bool-value done
  num_spikes =
    num_grid_spikes + num_offgrid_spikes + uintsize_secondary_events + 2;

  if ( not off_grid_spiking_ ) // on grid spiking
  {
    // make sure buffers are correctly sized
    if ( global_grid_spikes_.size()
      != static_cast< unsigned int >(
           kernel().mpi_manager.get_recv_buffer_size() ) )
    {
      global_grid_spikes_.resize(
        kernel().mpi_manager.get_recv_buffer_size(), 0 );
    }
    if ( num_spikes + ( kernel().vp_manager.get_num_threads()
                        * kernel().connection_manager.get_min_delay() )
      > static_cast< unsigned int >(
           kernel().mpi_manager.get_send_buffer_size() ) )
    {
      local_grid_spikes_.resize(
        ( num_spikes + ( kernel().connection_manager.get_min_delay()
                         * kernel().vp_manager.get_num_threads() ) ),
        0 );
    }
    else if ( local_grid_spikes_.size()
      < static_cast< unsigned int >(
                kernel().mpi_manager.get_send_buffer_size() ) )
    {
      local_grid_spikes_.resize(
        kernel().mpi_manager.get_send_buffer_size(), 0 );
    }

    // collocate the entries of spike_registers into local_grid_spikes__
    std::vector< unsigned int >::iterator pos = local_grid_spikes_.begin();
    if ( num_offgrid_spikes == 0 )
    {
      for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
      {
        for ( j = i->begin(); j != i->end(); ++j )
        {
          pos = std::copy( j->begin(), j->end(), pos );
          *pos = comm_marker_;
          ++pos;
        }
      }
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
      for ( it = offgrid_spike_register_.begin();
            it != offgrid_spike_register_.end();
            ++it )
      {
        for ( jt = it->begin(); jt != it->end(); ++jt )
        {
          jt->clear();
        }
      }
    }

    // remove old spikes from the spike_register_
    for ( i = spike_register_.begin(); i != spike_register_.end(); ++i )
    {
      for ( j = i->begin(); j != i->end(); ++j )
      {
        j->clear();
      }
    }
    // here all spikes have been written to the local_grid_spikes buffer
    // pos points to next position in this outgoing communication buffer
    for ( j = secondary_events_buffer_.begin();
          j != secondary_events_buffer_.end();
          ++j )
    {
      pos = std::copy( j->begin(), j->end(), pos );
      j->clear();
    }

    // end marker after last secondary event
    // made sure in resize that this position is still allocated
    write_to_comm_buffer( invalid_synindex, pos );
    // append the boolean value indicating whether we are done here
    write_to_comm_buffer( done, pos );
  }
  else // off_grid_spiking
  {
    // make sure buffers are correctly sized
    if ( global_offgrid_spikes_.size()
      != static_cast< unsigned int >(
           kernel().mpi_manager.get_recv_buffer_size() ) )
    {
      global_offgrid_spikes_.resize(
        kernel().mpi_manager.get_recv_buffer_size(), OffGridSpike( 0, 0.0 ) );
    }
    if ( num_spikes + ( kernel().vp_manager.get_num_threads()
                        * kernel().connection_manager.get_min_delay() )
      > static_cast< unsigned int >(
           kernel().mpi_manager.get_send_buffer_size() ) )
    {
      local_offgrid_spikes_.resize(
        ( num_spikes + ( kernel().connection_manager.get_min_delay()
                         * kernel().vp_manager.get_num_threads() ) ),
        OffGridSpike( 0, 0.0 ) );
    }
    else if ( local_offgrid_spikes_.size()
      < static_cast< unsigned int >(
                kernel().mpi_manager.get_send_buffer_size() ) )
    {
      local_offgrid_spikes_.resize(
        kernel().mpi_manager.get_send_buffer_size(), OffGridSpike( 0, 0.0 ) );
    }

    // collocate the entries of spike_registers into local_offgrid_spikes__
    std::vector< OffGridSpike >::iterator pos = local_offgrid_spikes_.begin();
    if ( num_grid_spikes == 0 )
    {
      for ( it = offgrid_spike_register_.begin();
            it != offgrid_spike_register_.end();
            ++it )
      {
        for ( jt = it->begin(); jt != it->end(); ++jt )
        {
          pos = std::copy( jt->begin(), jt->end(), pos );
          pos->set_gid( comm_marker_ );
          ++pos;
        }
      }
    }
    else
    {
      std::vector< unsigned int >::iterator n;
      i = spike_register_.begin();
      for ( it = offgrid_spike_register_.begin();
            it != offgrid_spike_register_.end();
            ++it )
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
      {
        for ( j = i->begin(); j != i->end(); ++j )
        {
          j->clear();
        }
      }
    }

    // empty offgrid_spike_register_
    for ( it = offgrid_spike_register_.begin();
          it != offgrid_spike_register_.end();
          ++it )
    {
      for ( jt = it->begin(); jt != it->end(); ++jt )
      {
        jt->clear();
      }
    }
  }
}

// returns the done value
bool
EventDeliveryManager::deliver_events( thread t )
{
  // are we done?
  bool done = true;

  // deliver only at beginning of time slice
  if ( kernel().simulation_manager.get_from_step() > 0 )
  {
    return done;
  }
  SpikeEvent se;

  std::vector< int > pos( displacements_ );

  if ( not off_grid_spiking_ ) // on_grid_spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps(
      kernel().connection_manager.get_min_delay() );
    for ( size_t lag = 0;
          lag < ( size_t ) kernel().connection_manager.get_min_delay();
          lag++ )
    {
      prepared_timestamps[ lag ] =
        kernel().simulation_manager.get_clock() - Time::step( lag );
    }

    for ( size_t vp = 0;
          vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes();
          ++vp )
    {
      size_t pid = kernel().mpi_manager.get_process_id( vp );
      int pos_pid = pos[ pid ];
      int lag = kernel().connection_manager.get_min_delay() - 1;
      while ( lag >= 0 )
      {
        index nid = global_grid_spikes_[ pos_pid ];
        if ( nid != static_cast< index >( comm_marker_ ) )
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp( prepared_timestamps[ lag ] );
          se.set_sender_gid( nid );
          kernel().connection_manager.send( t, nid, se );
        }
        else
        {
          --lag;
        }
        ++pos_pid;
      }
      pos[ pid ] = pos_pid;
    }

    // here we are done with the spiking events
    // pos[pid] for each pid now points to the first entry of
    // the secondary events

    for ( size_t pid = 0;
          pid < ( size_t ) kernel().mpi_manager.get_num_processes();
          ++pid )
    {
      std::vector< unsigned int >::iterator readpos =
        global_grid_spikes_.begin() + pos[ pid ];

      while ( true )
      {
        // we must not use unsigned int for the type, otherwise
        // the encoding will be different on JUQUEEN for the
        // index written into the buffer and read out of it
        synindex synid;
        read_from_comm_buffer( synid, readpos );
        if ( synid == invalid_synindex )
        {
          break;
        }
        --readpos;

        kernel().model_manager.assert_valid_syn_id( synid );

        kernel().model_manager.get_secondary_event_prototype( synid, t )
          << readpos;

        // set time stamp (used by weight_recorder)
        kernel()
          .model_manager.get_secondary_event_prototype( synid, t )
          .set_stamp( kernel().simulation_manager.get_clock() );

        kernel().connection_manager.send_secondary(
          t, kernel().model_manager.get_secondary_event_prototype( synid, t ) );
      } // of while (true)

      // read the done value of the p-th num_process

      // must be a bool (same type as on the sending side)
      // otherwise the encoding will be inconsistent on JUQUEEN
      bool done_p;
      read_from_comm_buffer( done_p, readpos );
      done = done && done_p;
    }
  }
  else // off grid spiking
  {
    // prepare Time objects for every possible time stamp within min_delay_
    std::vector< Time > prepared_timestamps(
      kernel().connection_manager.get_min_delay() );
    for ( size_t lag = 0;
          lag < ( size_t ) kernel().connection_manager.get_min_delay();
          lag++ )
    {
      prepared_timestamps[ lag ] =
        kernel().simulation_manager.get_clock() - Time::step( lag );
    }

    for ( size_t vp = 0;
          vp < ( size_t ) kernel().vp_manager.get_num_virtual_processes();
          ++vp )
    {
      size_t pid = kernel().mpi_manager.get_process_id( vp );
      int pos_pid = pos[ pid ];
      int lag = kernel().connection_manager.get_min_delay() - 1;
      while ( lag >= 0 )
      {
        index nid = global_offgrid_spikes_[ pos_pid ].get_gid();
        if ( nid != static_cast< index >( comm_marker_ ) )
        {
          // tell all local nodes about spikes on remote machines.
          se.set_stamp( prepared_timestamps[ lag ] );
          se.set_sender_gid( nid );
          se.set_offset( global_offgrid_spikes_[ pos_pid ].get_offset() );
          kernel().connection_manager.send( t, nid, se );
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

  return done;
}

void
EventDeliveryManager::gather_events( bool done )
{
  // IMPORTANT: Ensure that gather_events(..) is called from a single thread and
  //            NOT from a parallel OpenMP region!!!

  // Stop watch for time measurements within this function
  static Stopwatch stw_local;

  stw_local.reset();
  stw_local.start();
  collocate_buffers_( done );
  stw_local.stop();
  time_collocate_ += stw_local.elapsed();
  stw_local.reset();
  stw_local.start();
  if ( off_grid_spiking_ )
  {
    kernel().mpi_manager.communicate(
      local_offgrid_spikes_, global_offgrid_spikes_, displacements_ );
  }
  else
  {
    kernel().mpi_manager.communicate(
      local_grid_spikes_, global_grid_spikes_, displacements_ );
  }
  stw_local.stop();
  time_communicate_ += stw_local.elapsed();
}
}
