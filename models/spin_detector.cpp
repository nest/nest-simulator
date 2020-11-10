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
#include "dict_util.h"
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

nest::spin_detector::spin_detector()
  : last_in_node_id_( 0 )
  , t_last_in_spike_( Time::neg_inf() )
{
}

nest::spin_detector::spin_detector( const spin_detector& n )
  : RecordingDevice( n )
  , last_in_node_id_( 0 )
  , t_last_in_spike_( Time::neg_inf() ) // mark as not initialized
{
}

void
nest::spin_detector::init_state_( const Node& )
{
  init_buffers_();
}

void
nest::spin_detector::init_buffers_()
{
}

void
nest::spin_detector::calibrate()
{
  RecordingDevice::calibrate( RecordingBackend::NO_DOUBLE_VALUE_NAMES, { nest::names::state } );
}

void
nest::spin_detector::update( Time const&, const long, const long )
{
  if ( last_in_node_id_ != 0 ) // if last_* is empty we dont write
  {
    write( last_event_, RecordingBackend::NO_DOUBLE_VALUES, { static_cast< int >( last_event_.get_weight() ) } );
    last_in_node_id_ = 0;
  }
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
  if ( is_active( e.get_stamp() ) )
  {
    assert( e.get_multiplicity() > 0 );

    // The following logic implements the decoding
    // A single spike signals a transition to the 0 state, two
    // spikes at the same time step signal a transition to the 1
    // state.
    //
    // Remember the node ID of the sender of the last spike being
    // received this assumes that several spikes being sent by the
    // same neuron in the same time step are received consecutively or
    // are conveyed by setting the multiplicity accordingly.

    long m = e.get_multiplicity();
    index node_id = e.get_sender_node_id();
    const Time& t_spike = e.get_stamp();
    if ( m == 1 && node_id == last_in_node_id_ && t_spike == t_last_in_spike_ )
    {
      // received twice the same node ID, so transition 0->1
      // revise the last event
      // if m == 2 this will just trigger writing in the following sections
      last_event_.set_weight( 1.0 );
    }
    if ( last_in_node_id_ != 0 ) // if last_* is empty we dont write
    {
      // if it's the second event we write out the last event first
      write( last_event_, RecordingBackend::NO_DOUBLE_VALUES, { static_cast< int >( last_event_.get_weight() ) } );
    }
    if ( m == 2 )
    { // already full event
      write( e, RecordingBackend::NO_DOUBLE_VALUES, { 1 } );
      last_in_node_id_ = 0;
    }
    else
    {
      if ( last_in_node_id_ == 0 )
      {
        // store the new event as last_event_ for the next iteration
        last_event_ = e;
        last_event_.set_weight( 0.0 );
        last_in_node_id_ = node_id;
        t_last_in_spike_ = t_spike;
      }
      else
      {
        last_in_node_id_ = 0;
      }
    }
  }
}
