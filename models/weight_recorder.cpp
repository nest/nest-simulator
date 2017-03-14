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
  : Node()
  , device_( *this,
      RecordingDevice::WEIGHT_RECORDER,
      "csv",
      true,
      true,
      true,
      true )
  , user_set_precise_times_( false )
  , has_proxies_( false )
  , local_receiver_( true )
  , P_()
{
}

nest::weight_recorder::weight_recorder( const weight_recorder& n )
  : Node( n )
  , device_( *this, n.device_ )
  , user_set_precise_times_( n.user_set_precise_times_ )
  , has_proxies_( false )
  , local_receiver_( true )
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
  const weight_recorder& wr = static_cast< const weight_recorder& >( np );
  device_.init_state( wr.device_ );
  init_buffers_();
}

void
nest::weight_recorder::init_buffers_()
{
  device_.init_buffers();
  B_.events_ = std::vector< WeightRecorderEvent >();
}

void
nest::weight_recorder::calibrate()
{
  if ( kernel().event_delivery_manager.get_off_grid_communication()
    and not device_.is_precise_times_user_set() )
  {
    device_.set_precise_times( true );
    std::string msg = String::compose(
      "Precise neuron models exist: the property precise_times "
      "of the %1 with gid %2 has been set to true",
      get_name(),
      get_gid() );

    if ( device_.is_precision_user_set() )
    {
      // if user explicitly set the precision, there is no need to do anything.
      msg += ".";
    }
    else
    {
      // it makes sense to increase the precision if precise models are used.
      device_.set_precision( 15 );
      msg += ", precision has been set to 15.";
    }

    LOG( M_INFO, "weight_recoder::calibrate", msg );
  }

  device_.calibrate();
}

void
nest::weight_recorder::update( Time const&, const long from, const long to )
{

  for ( std::vector< WeightRecorderEvent >::iterator e = B_.events_.begin();
        e != B_.events_.end();
        ++e )
  {
    device_.record_event( *e );
  }

  // do not use swap here to clear, since we want to keep the reserved()
  // memory for the next round
  B_.events_.clear();
}

void
nest::weight_recorder::get_status( DictionaryDatum& d ) const
{
  // get the data from the device
  device_.get_status( d );

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( local_receiver_ && get_thread() == 0 )
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
  if ( d->known( names::precise_times ) )
  {
    user_set_precise_times_ = true;
  }

  device_.set_status( d );

  P_.set( d );
}


void
nest::weight_recorder::handle( WeightRecorderEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( device_.is_active( e.get_stamp() ) )
  {
    // P_senders_ is defined and sender is not in it
    // or P_targets_ is defined and receiver is not in it
    if ( ( not P_.senders_.empty()
           and not std::binary_search(
                 P_.senders_.begin(), P_.senders_.end(), e.get_sender_gid() ) )
      or ( not P_.targets_.empty()
           and not std::binary_search( P_.targets_.begin(),
                 P_.targets_.end(),
                 e.get_receiver_gid() ) ) )
    {
      return;
    }

    WeightRecorderEvent* event = e.clone();
    B_.events_.push_back( *event );
  }
}
