/*
 *  spike_detector.h
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

#ifndef SPIKE_DETECTOR_H
#define SPIKE_DETECTOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "device_node.h"
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "recording_device.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Devices
@ingroup detector

Name: spike_detector - Device for detecting single spikes.

Description:

The spike_detector device is a recording device. It is used to record
spikes from a single neuron, or from multiple neurons at once. Data
is recorded in memory or to file as for all RecordingDevices.
By default, GID and time of each spike is recorded.

The spike detector will record spike times with full precision
from neurons emitting precisely timed spikes.

Any node from which spikes are to be recorded, must be connected to
the spike detector using a normal connect command. Any connection weight
and delay will be ignored for that connection.

Simulations progress in cycles defined by the minimum delay. During each
cycle, the spike detector records (stores in memory or writes to screen/file)
the spikes. Setting the /stop parameter stops the recording before the end of
simulation.

Spike are not necessarily written to file in chronological order.


Receives: SpikeEvent

SeeAlso: spike_detector, Device, RecordingDevice
*/

class spike_detector : public RecordingDevice
{

public:
  spike_detector();
  spike_detector( const spike_detector& );

  bool
  has_proxies() const
  {
    return false;
  }

  bool
  local_receiver() const
  {
    return true;
  }

  Name
  get_element_type() const
  {
    return names::recorder;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;

  /*
   * Write incoming spikes.
   */
  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  Type get_type() const;
  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();

  /**
   * Update detector.
   *
   * @see RecordingDevice
   */
  void update( Time const&, const long, const long );
};

inline port
spike_detector::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline SignalType
spike_detector::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef SPIKE_DETECTOR_H */
