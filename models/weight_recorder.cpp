/*
 *  weight_recorder.cpp
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

#include "weight_recorder.h"


// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest_impl.h"
#include "node_collection.h"

void
nest::register_weight_recorder( const std::string& name )
{
  register_node_model< weight_recorder >( name );
}

// record time, node ID, weight and receiver node ID
nest::weight_recorder::weight_recorder()
  : RecordingDevice()
  , P_()
{
}

nest::weight_recorder::weight_recorder( const weight_recorder& n )
  : RecordingDevice( n )
  , P_( n.P_ )
{
}

// We must initialize senders and targets here with empty NCs because
// they will be returned by get_status()
nest::weight_recorder::Parameters_::Parameters_()
  : senders_( new NodeCollectionPrimitive() )
  , targets_( new NodeCollectionPrimitive() )
{
}

void
nest::weight_recorder::Parameters_::get( Dictionary& d ) const
{
  d[ names::senders ] = senders_;
  d[ names::targets ] = targets_;
}

void
nest::weight_recorder::Parameters_::set( const Dictionary& d )
{
  auto get_or_create_nc = [ &d ]( NodeCollectionPTR& nc, const std::string& key )
  {
    if ( not d.empty() and d.known( key ) )
    {
      const auto value = d.at( key );
      if ( is_type< NodeCollectionPTR >( value ) )
      {
        nc = d.get< NodeCollectionPTR >( key );
      }
      else
      {
        throw TypeMismatch( "NodeCollection", debug_type( d.at( key ) ) );
      }
    }
  };

  get_or_create_nc( senders_, names::senders );
  get_or_create_nc( targets_, names::targets );
}

void
nest::weight_recorder::pre_run_hook()
{
  RecordingDevice::pre_run_hook(
    { nest::names::weights }, { nest::names::targets, nest::names::receptors, nest::names::ports } );
}

void
nest::weight_recorder::update( Time const&, const long, const long )
{
}

nest::RecordingDevice::Type
nest::weight_recorder::get_type() const
{
  return RecordingDevice::WEIGHT_RECORDER;
}

void
nest::weight_recorder::get_status( Dictionary& d ) const
{
  // get the data from the device
  RecordingDevice::get_status( d );

  if ( is_model_prototype() )
  {
    return; // no data to collect
  }

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const std::vector< Node* > siblings = kernel().node_manager.get_thread_siblings( get_node_id() );
    std::vector< Node* >::const_iterator s;
    for ( s = siblings.begin() + 1; s != siblings.end(); ++s )
    {
      ( *s )->get_status( d );
    }
  }

  P_.get( d );
}

void
nest::weight_recorder::set_status( const Dictionary& d )
{
  Parameters_ ptmp = P_;
  ptmp.set( d );

  RecordingDevice::set_status( d );
  P_ = ptmp;
}


void
nest::weight_recorder::handle( WeightRecorderEvent& e )
{
  // accept spikes only if recorder was active when spike was emitted
  if ( is_active( e.get_stamp() ) )
  {
    // P_senders_ is defined and sender is not in it
    // or P_targets_ is defined and receiver is not in it
    if ( ( P_.senders_->size() != 0 and not P_.senders_->contains( e.get_sender_node_id() ) )
      or ( P_.targets_->size() != 0 and not P_.targets_->contains( e.get_receiver_node_id() ) ) )
    {
      return;
    }

    write( e,
      { e.get_weight() },
      { static_cast< long >( e.get_receiver_node_id() ),
        static_cast< long >( e.get_rport() ),
        static_cast< long >( e.get_port() ) } );
  }
}
