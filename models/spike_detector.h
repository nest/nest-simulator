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

/* BeginDocumentation

Collecting spikes from neurons
##############################

The most universal collector device is the ``spike_detector``, which
collects and records all *spikes* it receives from neurons that are
connected to it. Each spike received by the spike detector is
immediately handed over to the selected recording backend for further
processing.

Any node from which spikes are to be recorded, must be connected to
the spike detector using the standard ``Connect`` command. The
connection ``weights`` and ``delays`` are ignored by the spike detector, which
means that the spike detector records the time of spike creation
rather than that of their arrival.

::

   >>> neurons = nest.Create('iaf_psc_alpha', 5)
   >>> sd = nest.Create('spike_detector')
   >>> nest.Connect(neurons, sd)

The call to ``Connect`` will fail if the connection direction is reversed (i.e., connecting
*sd* to *neurons*).

EndDocumentation */

namespace nest
{

/**
 * Class spike_detector
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
