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
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "sibling_container.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

// record time, gid, weight and receiver gid
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
  ( *d )[ names::senders ] = senders_;
  ( *d )[ names::targets ] = targets_;
}

void
nest::weight_recorder::Parameters_::set( const DictionaryDatum& d )
{
  if ( d->known( names::senders ) )
  {
    senders_ = getValue< std::vector< long > >( d->lookup( names::senders ) );
    std::sort( senders_.begin(), senders_.end() );
  }

  if ( d->known( names::targets ) )
  {
    targets_ = getValue< std::vector< long > >( d->lookup( names::targets ) );
    std::sort( targets_.begin(), targets_.end() );
  }
}

void
nest::weight_recorder::init_state_( const Node& np )
{
  init_buffers_();
}

void
nest::weight_recorder::init_buffers_()
{
}

void
nest::weight_recorder::calibrate()
{
  RecordingDevice::calibrate();
  RecordingDevice::enroll(
    {
      nest::names::weights,
    }, {
      nest::names::targets,
      nest::names::receptors,
      nest::names::ports,
    }
  );
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

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const SiblingContainer* siblings =
      kernel().node_manager.get_thread_siblings( get_gid() );
    std::vector< Node* >::const_iterator sibling;
    for ( sibling = siblings->begin() + 1; sibling != siblings->end();
          ++sibling )
    {
      ( *sibling )->get_status( d );
    }
  }

  P_.get( d );
}

void
nest::weight_recorder::set_status( const DictionaryDatum& d )
{
  RecordingDevice::set_status( d );
  P_.set( d );
}


void
nest::weight_recorder::handle( WeightRecorderEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( is_active( e.get_stamp() ) )
  {
    bool senders_set = not P_.senders_.empty();
    bool sender_gid_in_senders = std::binary_search(
	P_.senders_.begin(), P_.senders_.end(), e.get_sender_gid() );
    bool targets_set = not P_.targets_.empty();
    bool target_gid_in_targets = std::binary_search(
	P_.targets_.begin(), P_.targets_.end(), e.get_receiver_gid() );
    
    if ( ( senders_set and not sender_gid_in_senders )
	 or ( targets_set  and not target_gid_in_targets ) )
    {
      return;
    }

    RecordingDevice::write( e,
      { e.get_weight() },
      { static_cast<long>(e.get_receiver_gid()),
        static_cast<long>(e.get_rport()),
        static_cast<long>(e.get_port()) }
    );
  }
}
