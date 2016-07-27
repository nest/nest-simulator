/*
 *  spike_detector.cpp
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

#include "spike_detector.h"

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

nest::spike_detector::spike_detector()
  : Node()
  // record time and gid
  , device_( *this, RecordingDevice::SPIKE_DETECTOR, "gdf", true, true )
  , has_user_set_precise_times_( false )
  , has_proxies_( false )
  , local_receiver_( true )
{
}

nest::spike_detector::spike_detector( const spike_detector& n )
  : Node( n )
  , device_( *this, n.device_ )
  , has_user_set_precise_times_( n.has_user_set_precise_times_ )
  , has_proxies_( false )
  , local_receiver_( true )
{
}

void
nest::spike_detector::init_state_( const Node& np )
{
  const spike_detector& sd = dynamic_cast< const spike_detector& >( np );
  device_.init_state( sd.device_ );
  init_buffers_();
}

void
nest::spike_detector::init_buffers_()
{
  device_.init_buffers();

  std::vector< std::vector< Event* > > tmp( 2, std::vector< Event* >() );
  B_.spikes_.swap( tmp );
}

void
nest::spike_detector::calibrate()
{
  if ( !has_user_set_precise_times_
    && kernel().event_delivery_manager.get_off_grid_communication() )
  {
    device_.set_precise( true, 15 );

    LOG( M_INFO,
      "spike_detector::calibrate",
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
nest::spike_detector::update( Time const&, const long_t, const long_t )
{
  for ( std::vector< Event* >::iterator e =
          B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].begin();
        e != B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].end();
        ++e )
  {
    assert( *e != 0 );
    device_.record_event( **e );
    delete *e;
  }

  // do not use swap here to clear, since we want to keep the reserved()
  // memory for the next round
  B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].clear();
}

void
nest::spike_detector::get_status( DictionaryDatum& d ) const
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
nest::spike_detector::set_status( const DictionaryDatum& d )
{
  if ( d->known( names::precise_times ) )
    has_user_set_precise_times_ = true;

  // By setting precision value, precise_times is implicitly set.
  if ( d->known( names::precision ) )
  {

    if ( d->known( names::precise_times ) )
    {
      if ( getValue< bool >( d, names::precise_times ) )
        device_.set_precise( true, getValue< long >( d, names::precision ) );
    }

    else
      device_.set_precise( true, getValue< long >( d, names::precision ) );

    has_user_set_precise_times_ = true;
  }

  device_.set_status( d );
}

void
nest::spike_detector::handle( SpikeEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( device_.is_active( e.get_stamp() ) )
  {
    assert( e.get_multiplicity() > 0 );

    long_t dest_buffer;
    if ( kernel()
           .modelrange_manager.get_model_of_gid( e.get_sender_gid() )
           ->has_proxies() )
      // events from central queue
      dest_buffer = kernel().event_delivery_manager.read_toggle();
    else
      // locally delivered events
      dest_buffer = kernel().event_delivery_manager.write_toggle();

    for ( int_t i = 0; i < e.get_multiplicity(); ++i )
    {
      // We store the complete events
      Event* event = e.clone();
      B_.spikes_[ dest_buffer ].push_back( event );
    }
  }
}

