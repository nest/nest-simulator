/*
 *  spin_detector.cpp
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

#include "spin_detector.h"

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

nest::spin_detector::spin_detector()
  : last_in_gid_( 0 )
  , t_last_in_spike_( Time::neg_inf() )
{
}

nest::spin_detector::spin_detector( const spin_detector& n )
  : RecordingDevice( n )
  , last_in_gid_( 0 )
  , t_last_in_spike_( Time::neg_inf() ) // mark as not initialized
{
}

void
nest::spin_detector::init_state_( const Node& np )
{
  // const spin_detector& sd = dynamic_cast< const spin_detector& >( np );
  // device_.init_state( sd.device_ ); // FIXME (??)
  init_buffers_();
}

void
nest::spin_detector::init_buffers_()
{
  // device_.init_buffers();

  std::vector< std::vector< Event* > > tmp( 2, std::vector< Event* >() );
  B_.spikes_.swap( tmp );
}

void
nest::spin_detector::calibrate()
{
  // device_.calibrate(); //FIXME
}

void
nest::spin_detector::update( Time const&, const long, const long )
{
  for ( std::vector< Event* >::iterator e =
          B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].begin();
        e != B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].end();
        ++e )
  {
    assert( *e != 0 );
    // device_.write( **e ); //FIXME
    delete *e;
  }

  // do not use swap here to clear, since we want to keep the reserved()
  // memory for the next round
  B_.spikes_[ kernel().event_delivery_manager.read_toggle() ].clear();
}

nest::RecordingDevice::Type
nest::spin_detector::get_type() const
{
  return RecordingDevice::SPIN_DETECTOR;
}

void
nest::spin_detector::get_status( DictionaryDatum& d ) const
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
}

void
nest::spin_detector::set_status( const DictionaryDatum& d )
{
  RecordingDevice::set_status( d );
}


void
nest::spin_detector::handle( SpikeEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( true ) // device_.is_active( e.get_stamp() ) ) // FIXME
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


    // The following logic implements the decoding
    // A single spike signals a transition to the 0 state, two
    // spikes at the same time step signal a transition to the 1
    // state.
    //
    // Remember the global id of the sender of the last spike being
    // received this assumes that several spikes being sent by the
    // same neuron in the same time step are received consecutively or
    // are conveyed by setting the multiplicity accordingly.

    long m = e.get_multiplicity();
    index gid = e.get_sender_gid();
    const Time& t_spike = e.get_stamp();

    if ( m == 1 )
    {
      // multiplicity == 1, either a single 1->0 event or the first or
      // second of a pair of 0->1 events
      if ( gid == last_in_gid_ && t_spike == t_last_in_spike_ )
      {
        // received twice the same gid, so transition 0->1
        // revise the last event written to the buffer
        ( *( B_.spikes_[ dest_buffer ].end() - 1 ) )->set_weight( 1. );
      }
      else
      {
        // count this event negatively, assuming it comes as single event
        // transition 1->0
        Event* event = e.clone();
        // assume it will stay alone, so meaning a 1->0 transition
        event->set_weight( 0 );
        B_.spikes_[ dest_buffer ].push_back( event );
      }
    }
    else // multiplicity != 1
      if ( m == 2 )
    {
      // count this event positively, transition 0->1
      Event* event = e.clone();
      event->set_weight( 1. );
      B_.spikes_[ dest_buffer ].push_back( event );
    }

    last_in_gid_ = gid;
    t_last_in_spike_ = t_spike;
  }
}
