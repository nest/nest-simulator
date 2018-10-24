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
Name: spike_detector - Device for detecting single spikes.

Description:

The spike_detector device is a recording device. It is used to record
spikes from a single neuron, or from multiple neurons at once. Data
is recorded in memory or to file as for all RecordingDevices.
By default, GID and time of each spike is recorded.

The spike detector can also record spike times with full precision
from neurons emitting precisely timed spikes. Set /precise_times to
achieve this. If there are precise models and /precise_times is not
set, it will be set to True at the start of the simulation and
/precision will be increased to 15 from its default value of 3.

Any node from which spikes are to be recorded, must be connected to
the spike detector using a normal connect command. Any connection weight
and delay will be ignored for that connection.

Simulations progress in cycles defined by the minimum delay. During each
cycle, the spike detector records (stores in memory or writes to screen/file)
the spikes generated during the previous cycle. As a consequence, any
spikes generated during the cycle immediately preceding the end of the
simulation time will not be recorded. Setting the /stop parameter to at the
latest one min_delay period before the end of the simulation time ensures that
all spikes desired to be recorded, are recorded.

Spike are not necessarily written to file in chronological order.

Note:

Spikes are buffered in a two-segment buffer. We need to distinguish between
two types of spikes: those delivered from the global event queue (almost all
spikes) and spikes delivered locally from devices that are replicated on VPs
(has_proxies() == false).
- Spikes from the global queue are delivered by deliver_events() at the
  beginning of each update cycle and are stored only until update() is called
  during the same update cycle. Global queue spikes are thus written to the
  read_toggle() segment of the buffer, from which update() reads.
- Spikes delivered locally may be delivered before or after
  spike_detector::update() is executed. These spikes are therefore buffered
  in the write_toggle() segment of the buffer and output during the next
  cycle.
- After all spikes are recorded, update() clears the read_toggle() segment
  of the buffer.


Receives: SpikeEvent

SeeAlso: spike_detector, Device, RecordingDevice
*/
class spike_detector : public DeviceNode
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

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void post_run_cleanup();
  void finalize();

  /**
   * Update detector by recording spikes.
   *
   * All spikes in the read_toggle() half of the spike buffer are
   * recorded by passing them to the RecordingDevice, which then
   * stores them in memory or outputs them as desired.
   *
   * @see RecordingDevice
   */
  void update( Time const&, const long, const long );

  /**
   * Buffer for incoming spikes.
   *
   * This data structure buffers all incoming spikes until they are
   * passed to the RecordingDevice for storage or output during update().
   * update() always reads from spikes_[Network::get_network().read_toggle()]
   * and deletes all events that have been read.
   *
   * Events arriving from locally sending nodes, i.e., devices without
   * proxies, are stored in spikes_[Network::get_network().write_toggle()], to
   * ensure order-independent results.
   *
   * Events arriving from globally sending nodes are delivered from the
   * global event queue by Network::deliver_events() at the beginning
   * of the time slice. They are therefore written to
   * spikes_[Network::get_network().read_toggle()]
   * so that they can be recorded by the subsequent call to update().
   * This does not violate order-independence, since all spikes are delivered
   * from the global queue before any node is updated.
   */
  struct Buffers_
  {
    std::vector< std::vector< Event* > > spikes_;
  };

  RecordingDevice device_;
  Buffers_ B_;
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

inline void
spike_detector::post_run_cleanup()
{
  device_.post_run_cleanup();
}

inline SignalType
spike_detector::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef SPIKE_DETECTOR_H */
