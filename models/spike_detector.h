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
@ingroup detector
@ingroup spike_detector

Spike detector
##############

The most universal collector device is the ``spike_detector``. It
collects and records all *spikes* it receives from neurons that are
connected to it. Each spike received by the spike detector is
immediately handed over to the prescribed recording backend for
further processing.

Any node from which spikes are to be recorded, must be connected to
the spike detector using the standard ``Connect`` command. The
connection weight and delay are ignored by the spike detector.

::

   >>> neurons = nest.Create('iaf_psc_alpha', 5)
   >>> sd = nest.Create('spike_detector')
   >>> nest.Connect(neurons, sd)

The call to ``Connect`` in the example above would fail, if the
*neurons* would not be sending ``SpikeEvent``s during a
simulation. Likewise, a reversed connection direction (i.e. connecting
*sd* to *neurons*) would fail.

.. note::
   The spike detector records spike times with full precision from
   neurons emitting :doc:`precisely timed spikes
   <simulations_with_precise_spike_time>`.

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

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  Type get_type() const;
  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
    void calibrate();
    void init_state_( Node const& ) {}
    void init_buffers_() {}
    void update( Time const&, const long, const long ) {}
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
