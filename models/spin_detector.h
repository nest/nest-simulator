/*
 *  spin_detector.h
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

#ifndef SPIN_DETECTOR_H
#define SPIN_DETECTOR_H


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
Name: spin_detector - Device for detecting binary states in neurons.

Description:

The spin_detector is a recording device. It is used to decode and
record binary states from spiking activity from a single neuron, or
from multiple neurons at once. A single spike signals the 0 state, two
spikes at the same time signal the 1 state. If a neuron is in the 0 or
1 state and emits the spiking activity corresponding to the same
state, the same state is recorded again.  Therefore, it is not only
the transitions that are recorded. Data is recorded in memory or to
file as for all RecordingDevices. By default, GID, time, and binary
state (0 or 1) for each decoded state is recorded. The state can be
accessed from ['events']['weight'].

The spin_detector can also record binary state times with full
precision from neurons emitting precisely timed spikes. Set
/precise_times to true to achieve this. If there are precise models
and /precise_times is not set, it will be set to True at the start of
the simulation and /precision will be increased to 15 from its default
value of 3.

Any node from which binary states are to be recorded, must be
connected to the spin_detector using the Connect command. Any
connection weight and delay will be ignored for that connection.

Simulations progress in cycles defined by the minimum delay. During
each cycle, the spin_detector records (stores in memory or writes to
screen/file) the states during the previous cycle. As a consequence,
any state information that was decoded during the cycle immediately
preceding the end of the simulation time will not be recorded. Setting
the /stop parameter to at the latest one min_delay period before the
end of the simulation time ensures that all binary states desired to
be recorded, are recorded.

states are not necessarily written to file in chronological order.

Receives: SpikeEvent

SeeAlso: spike_detector, Device, RecordingDevice
*/
/**
 * Spin detector class.
 *
 * This class decodes binary states based on incoming spikes. It receives
 * spikes via its handle(SpikeEvent&) method, decodes the state, and
 * stores them via its RecordingDevice in the update() method.
 *
 * Spikes are buffered in a two-segment buffer. We need to distinguish between
 * two types of spikes: those delivered from the global event queue (almost all
 * spikes) and spikes delivered locally from devices that are replicated on VPs
 * (has_proxies() == false).
 * - Spikes from the global queue are delivered by deliver_events() at the
 *   beginning of each update cycle and are stored only until update() is called
 *   during the same update cycle. Global queue spikes are thus written to the
 *   read_toggle() segment of the buffer, from which update() reads.
 * - Spikes delivered locally may be delivered before or after
 *   spin_detector::update() is executed. These spikes are therefore buffered in
 *   the write_toggle() segment of the buffer and output during the next cycle.
 * - After all spikes are recieved and states are decoded, update()
 *   clears the read_toggle() segment of the buffer.
 *
 * @ingroup Devices
 */
class spin_detector : public DeviceNode
{

public:
  spin_detector();
  spin_detector( const spin_detector& );

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
   * used to detect binary states.
   *
   * @see RecordingDevice
   */
  void update( Time const&, const long, const long );

  /**
   * Buffer for binary states.
   *
   * This is a buffer for all binary states from decoded spikes until they are
   * passed to the RecordingDevice for storage or output during update().
   * update() always reads from spikes_[Network::get_network().read_toggle()]
   * and deletes all events that have been read.
   *
   * Events arriving from locally sending nodes, i.e., devices without
   * proxies, are first decoded and then stored in
   * spikes_[Network::get_network().write_toggle()], to ensure order-independent
   * results.
   *
   * Events arriving from globally sending nodes are delivered from the
   * global event queue by Network::deliver_events() at the beginning
   * of the time slice. They are therefore first decoded and then written to
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
  index last_in_gid_;
  Time t_last_in_spike_;
};

inline port
spin_detector::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
spin_detector::post_run_cleanup()
{
  device_.post_run_cleanup();
}

inline void
spin_detector::finalize()
{
  device_.finalize();
}

inline SignalType
spin_detector::receives_signal() const
{
  return BINARY;
}

} // namespace

#endif /* #ifndef SPIN_DETECTOR_H */
