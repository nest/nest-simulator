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
#include "kernel_manager.h"
#include "nest_types.h"
#include "recording_device.h"

namespace nest
{

/** @BeginDocumentation
@ingroup detector

@name weight_recorder
@short_description Device for detecting single spikes.

Weight recorder
###############

The weight recorder is a recording device used to record weights from
synapses. It records the source and target global IDs together with
the weight of each spike event that travels through the observed
synapses.

In order to only record from a subset of the connected synapses, the
weight recorder accepts two list parameters 'senders' and 'targets'.
If set, they restrict the recording of data to synapses with the
corresponding source or target global IDs.

The weight recorder will record weights with full precision from
neurons emitting precisely timed spikes.

[[Needs connect example]]

*/

class weight_recorder : public RecordingDevice
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

  void handle( WeightRecorderEvent& );

  port handles_test_event( WeightRecorderEvent&, rport );

  Type get_type() const;
  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void update( Time const&, const long, const long );

  struct Parameters_
  {
    GIDCollectionDatum senders_;
    GIDCollectionDatum targets_;

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, Node* node );
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

inline SignalType
weight_recorder::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef WEIGHT_RECORDER_H */
