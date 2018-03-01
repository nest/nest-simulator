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
  : has_proxies_( false )
  , local_receiver_( true )
{
}

nest::spike_detector::spike_detector( const spike_detector& n )
  : RecordingDevice( n )
  , has_proxies_( false )
  , local_receiver_( true )
{
}

void
nest::spike_detector::init_state_( const Node& )
{
  init_buffers_();
}

void
nest::spike_detector::init_buffers_()
{
  std::vector< std::vector< Event* > > tmp( 2, std::vector< Event* >() );
  B_.spikes_.swap( tmp );
}

void
nest::spike_detector::calibrate()
{
  RecordingDevice::calibrate();
  RecordingDevice::enroll();
}

void
nest::spike_detector::update( Time const&, const long, const long )
{
  for ( std::vector< Event* >::iterator e =
          B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].begin();
        e != B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].end();
        ++e )
  {
    assert( *e != 0 );

    RecordingDevice::write( **e );
    delete *e;
  }

  // do not use swap here to clear, since we want to keep the reserved()
  // memory for the next round
  B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].clear();
}

nest::RecordingDevice::Type
nest::spike_detector::get_type() const
{
  return RecordingDevice::SPIKE_DETECTOR;
}

void
nest::spike_detector::get_status( DictionaryDatum& d ) const
{
  // get the data from the device
  RecordingDevice::get_status( d );

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
}

void
nest::spike_detector::set_status( const DictionaryDatum& d )
{
  RecordingDevice::set_status( d );
}

void
nest::spike_detector::handle( SpikeEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( is_active( e.get_stamp() ) )
  {
    assert( e.get_multiplicity() > 0 );

    long dest_buffer;
    if ( kernel()
           .modelrange_manager.get_model_of_gid( e.get_sender_gid() )
           ->has_proxies() )
    {
      // events from central queue
      dest_buffer = kernel().event_delivery_manager.read_toggle();
    }
    else
    {
      // locally delivered events
      dest_buffer = kernel().event_delivery_manager.write_toggle();
    }

    for ( int i = 0; i < e.get_multiplicity(); ++i )
    {
      // We store the complete events
      Event* event = e.clone();
      B_.spikes_[ dest_buffer ].push_back( event );
    }
  }
}
