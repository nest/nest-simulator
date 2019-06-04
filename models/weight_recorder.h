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

// Includes from nestkernel:
#include "device_node.h"
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "recording_device.h"
#include "kernel_manager.h"

namespace nest
{

/** @BeginDocumentation
Name: weight_recorder - Device for detecting single spikes.

Description:

The weight_recorder device is a recording device. It is used to record
weights from synapses. Data is recorded in memory or to file as for all
RecordingDevices.
By default, source GID, target GID, time and weight of each spike is recorded.

In order to record only from a subset of connected synapses, the
weight_recorder accepts the parameters 'senders' and 'targets', with which the
recorded data is limited to the synapses with the corresponding source or target
gid.

The weight recorder can also record weights with full precision
from neurons emitting precisely timed spikes. Set /precise_times to
achieve this.

Data is not necessarily written to file in chronological order.

Receives: WeightRecordingEvent

SeeAlso: weight_recorder, spike_detector, Device, RecordingDevice
*/
class weight_recorder : public DeviceNode
{

public:
  weight_recorder();
  weight_recorder( const weight_recorder& );

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

  void handle( WeightRecorderEvent& );

  port handles_test_event( WeightRecorderEvent&, rport );

  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void post_run_cleanup();
  void finalize();
  void update( Time const&, const long, const long );

  struct Buffers_
  {
    std::vector< WeightRecorderEvent > events_;
  };

  RecordingDevice device_;
  Buffers_ B_;

  bool user_set_precise_times_;

  struct Parameters_
  {
    std::vector< long > senders_;
    std::vector< long > targets_;

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline port
weight_recorder::handles_test_event( WeightRecorderEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
weight_recorder::post_run_cleanup()
{
  device_.post_run_cleanup();
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
