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

// C++ includes:
#include <numeric>

// Includes from libnestutil:
#include "dict_util.h"
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "node_collection.h"
#include "kernel_manager.h"
#include "nest_datums.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

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

nest::weight_recorder::Parameters_::Parameters_()
  : senders_()
  , targets_()
{
}

nest::weight_recorder::Parameters_::Parameters_( const Parameters_& p )
  : senders_( p.senders_ )
  , targets_( p.targets_ )
{
}

void
nest::weight_recorder::Parameters_::get( DictionaryDatum& d ) const
{
  if ( senders_.get() )
  {
    ( *d )[ names::senders ] = senders_;
  }
  else
  {
    ArrayDatum ad;
    ( *d )[ names::senders ] = ad;
  }
  if ( targets_.get() )
  {
    ( *d )[ names::targets ] = targets_;
  }
  else
  {
    ArrayDatum ad;
    ( *d )[ names::targets ] = ad;
  }
}

void
nest::weight_recorder::Parameters_::set( const DictionaryDatum& d )
{
  if ( d->known( names::senders ) )
  {
    const Token& tkn = d->lookup( names::senders );
    if ( tkn.is_a< NodeCollectionDatum >() )
    {
      senders_ = getValue< NodeCollectionDatum >( tkn );
    }
    else
    {
      if ( tkn.is_a< IntVectorDatum >() )
      {
        IntVectorDatum ivd = getValue< IntVectorDatum >( tkn );
        senders_ = NodeCollection::create( ivd );
      }
      if ( tkn.is_a< ArrayDatum >() )
      {
        ArrayDatum ad = getValue< ArrayDatum >( tkn );
        senders_ = NodeCollection::create( ad );
      }
    }
  }

  if ( d->known( names::targets ) )
  {
    const Token& tkn = d->lookup( names::targets );
    if ( tkn.is_a< NodeCollectionDatum >() )
    {
      targets_ = getValue< NodeCollectionDatum >( tkn );
    }
    else
    {
      if ( tkn.is_a< IntVectorDatum >() )
      {
        IntVectorDatum ivd = getValue< IntVectorDatum >( tkn );
        targets_ = NodeCollection::create( ivd );
      }
      if ( tkn.is_a< ArrayDatum >() )
      {
        ArrayDatum ad = getValue< ArrayDatum >( tkn );
        targets_ = NodeCollection::create( ad );
      }
    }
  }
}

void
nest::weight_recorder::calibrate()
{
  RecordingDevice::calibrate(
    { nest::names::weights }, { nest::names::targets, nest::names::receptors, nest::names::ports } );
}

void
nest::weight_recorder::update( Time const&, const long from, const long to )
{
}

nest::RecordingDevice::Type
nest::weight_recorder::get_type() const
{
  return RecordingDevice::WEIGHT_RECORDER;
}

void
nest::weight_recorder::get_status( DictionaryDatum& d ) const
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
nest::weight_recorder::set_status( const DictionaryDatum& d )
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
    if ( ( P_.senders_.get() and not P_.senders_->contains( e.get_sender_node_id() ) )
      or ( P_.targets_.get() and not P_.targets_->contains( e.get_receiver_node_id() ) ) )
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
