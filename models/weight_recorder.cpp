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

nest::weight_recorder::weight_recorder()
  : Node()
  // record time, gid and weight
  , device_( *this, RecordingDevice::SPIKE_DETECTOR, "gdf", true, true, true, true )
  , user_set_precise_times_( false )
  , has_proxies_( false )
  , local_receiver_( true )
{
}

nest::weight_recorder::weight_recorder( const weight_recorder& n )
  : Node( n )
  , device_( *this, n.device_ )
  , user_set_precise_times_( n.user_set_precise_times_ )
  , has_proxies_( false )
  , local_receiver_( true )
{
}

void
nest::weight_recorder::init_state_( const Node& np )
{
  const weight_recorder& wr = dynamic_cast< const weight_recorder& >( np );
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
  if ( !user_set_precise_times_
    && kernel().event_delivery_manager.get_off_grid_communication() )
  {
    device_.set_precise( true, 15 );

    LOG( M_INFO,
      "weight_recorder::calibrate",
      String::compose(
           "Precise neuron models exist: the property precise_times "
           "of the %1 with gid %2 has been set to true, precision has "
           "been set to 15.",
           get_name(),
           get_gid() ) );
  }

  device_.calibrate();
}

void
nest::weight_recorder::update( Time const&, const long from, const long to)
{

  for ( std::vector< WeightRecorderEvent >::iterator e =
          B_.events_.begin();
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
      ( *sibling )->get_status( d );
  }
}

void
nest::weight_recorder::set_status( const DictionaryDatum& d )
{
  if ( d->known( names::precise_times ) )
    user_set_precise_times_ = true;

  device_.set_status( d );
}

void
nest::weight_recorder::handle( WeightRecorderEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( device_.is_active( e.get_stamp() ) )
  {
    WeightRecorderEvent* event = e.clone();
    B_.events_.push_back( *event );
  }
}
