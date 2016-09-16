/*
 *  weight_recorder.h
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

#ifndef WEIGHT_RECORDER_H
#define WEIGHT_RECORDER_H


// C++ includes:
#include <vector>
#include <omp.h>

// Includes from nestkernel:
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "node.h"
#include "recording_device.h"
#include "kernel_manager.h"

/* BeginDocumentation

Name: weight_recorder - Device for detecting single spikes.

Description:
The weight_recorder device is a recording device. It is used to record
weights from spikes of a single neuron, or from multiple neurons at once. Data
is recorded in memory or to file as for all RecordingDevices.
By default, source GID, target GID, time and weight of each spike is recorded.

The weight detector can also record weights with full precision
from neurons emitting precisely timed spikes. Set /precise_times to
achieve this.

Data is not necessarily written to file in chronological order.

Receives: WeightRecordingEvent

SeeAlso: weight_recorder, spike_detector, Device, RecordingDevice
*/


namespace nest
{
/**
 * Weight recorder class.
 *
 * This class manages weight recording for normal and precise spikes. It
 * receives events via its handle(WeightRecordingEvent&) method, buffers them, and
 * stores them via its RecordingDevice in the update() method.
 *
 * Events are buffered in a two-segment buffer. We need to distinguish between
 * two types of spikes: those delivered from the global event queue (almost all
 * events) and events delivered locally from devices that are replicated on VPs
 * (has_proxies() == false).
 * - Events from the global queue are delivered by deliver_events() at the
 *   beginning of each update cycle and are stored only until update() is called
 *   during the same update cycle. Global queue spikes are thus written to the
 *   read_toggle() segment of the buffer, from which update() reads.
 * - Events delivered locally may be delivered before or after
 *   weight_recorder::update() is executed. These events are therefore buffered
 *   in the write_toggle() segment of the buffer and output during the next
 *   cycle.
 * - After all events are recorded, update() clears the read_toggle() segment
 *   of the buffer.
 *
 * @ingroup Devices
 */
class weight_recorder : public Node
{

public:
  weight_recorder();
  weight_recorder( const weight_recorder& );

  void set_has_proxies( const bool hp );
  bool
  has_proxies() const
  {
    return has_proxies_;
  }
  bool
  potential_global_receiver() const
  {
    return false;
  }
  void set_local_receiver( const bool lr );
  bool
  local_receiver() const
  {
    return local_receiver_;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;

  void handle( WeightRecorderEvent& );

  port handles_test_event( WeightRecorderEvent&, rport );

  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void finalize();

  /**
   * Update detector by recording weights.
   *
   * All spikes in the read_toggle() half of the spike buffer are
   * recorded by passing them to the RecordingDevice, which then
   * stores them in memory or outputs them as desired.
   *
   * @see RecordingDevice
   */
  void update( Time const&, const long, const long );

  /**
   * Buffer for incoming events.
   *
   * This data structure buffers all incoming events until they are
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
      std::vector< WeightRecorderEvent > events_;
  };

  RecordingDevice device_;
  Buffers_ B_;

  bool user_set_precise_times_;
  bool has_proxies_;
  bool local_receiver_;
  omp_lock_t writelock;
};

inline void
weight_recorder::set_has_proxies( const bool hp )
{
  has_proxies_ = hp;
}

inline void
weight_recorder::set_local_receiver( const bool lr )
{
  local_receiver_ = lr;
}

inline port
weight_recorder::handles_test_event( WeightRecorderEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline void
weight_recorder::finalize()
{
  device_.finalize();
}

inline SignalType
weight_recorder::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef WEIGHT_RECORDER_H */
