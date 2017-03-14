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
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "node.h"
#include "recording_device.h"

/* BeginDocumentation

Name: spin_detector - Device for detecting single spikes.

Description:
The spin_detector device is a recording device. It is used to record
spikes from a single neuron, or from multiple neurons at once. Data
is recorded in memory or to file as for all RecordingDevices.
By default, GID and time of each spike is recorded.

The spike detector can also record spike times with full precision
from neurons emitting precisely timed spikes. Set /precise_times to
achieve this.

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

Receives: SpikeEvent

SeeAlso: spike_detector, Device, RecordingDevice
*/


namespace nest
{
/**
 * Spike detector class.
 *
 * This class manages spike recording for normal and precise spikes. It
 * receives spikes via its handle(SpikeEvent&) method, buffers them, and
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
 * - After all spikes are recorded, update() clears the read_toggle() segment
 *   of the buffer.
 *
 * @ingroup Devices
 */
class spin_detector : public Node
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
  index last_in_gid_;
  Time t_last_in_spike_;
  bool user_set_precise_times_;
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
