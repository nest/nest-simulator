/*
 *  spike_recorder.cpp
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

#include "spike_recorder.h"

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

nest::spike_recorder::spike_recorder()
  : RecordingDevice()
{
}

nest::spike_recorder::spike_recorder( const spike_recorder& n )
  : RecordingDevice( n )
{
}

void
nest::spike_recorder::calibrate()
{
  RecordingDevice::calibrate( RecordingBackend::NO_DOUBLE_VALUE_NAMES, RecordingBackend::NO_LONG_VALUE_NAMES );
}

void
nest::spike_recorder::update( Time const&, const long, const long )
{
  // Nothing to do. Writing to the backend happens in handle().
}

nest::RecordingDevice::Type
nest::spike_recorder::get_type() const
{
  return RecordingDevice::SPIKE_RECORDER;
}

void
nest::spike_recorder::get_status( DictionaryDatum& d ) const
{
  RecordingDevice::get_status( d );

  if ( is_model_prototype() )
  {
    return; // no data to collect
  }

  // if we are the device on thread 0, also get the data from the siblings on other threads
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
nest::spike_recorder::set_status( const DictionaryDatum& d )
{
  RecordingDevice::set_status( d );
}

void
nest::spike_recorder::handle( SpikeEvent& e )
{
  // accept spikes only if detector was active when spike was emitted
  if ( is_active( e.get_stamp() ) )
  {
    assert( e.get_multiplicity() > 0 );

    for ( int i = 0; i < e.get_multiplicity(); ++i )
    {
      write( e, RecordingBackend::NO_DOUBLE_VALUES, RecordingBackend::NO_LONG_VALUES );
    }
  }
}
