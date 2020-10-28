/*
 *  spike_recorder.h
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

#ifndef SPIKE_RECORDER_H
#define SPIKE_RECORDER_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "device_node.h"
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "recording_device.h"

/* BeginUserDocs: device, recorder, spike

Short description
+++++++++++++++++

Collecting spikes from neurons

Description
+++++++++++

The most universal collector device is the ``spike_recorder``, which
collects and records all *spikes* it receives from neurons that are
connected to it. Each spike received by the spike recorder is
immediately handed over to the selected recording backend for further
processing.

Any node from which spikes are to be recorded, must be connected to
the spike recorder using the standard ``Connect`` command. The
connection ``weights`` and ``delays`` are ignored by the spike
recorder, which means that the spike recorder records the time of
spike creation rather than that of their arrival.

::

   >>> neurons = nest.Create('iaf_psc_alpha', 5)
   >>> sr = nest.Create('spike_recorder')
   >>> nest.Connect(neurons, sr)

The call to ``Connect`` will fail if the connection direction is
reversed (i.e., connecting *sr* to *neurons*).

Properties
++++++++++

All recorders have a set of common properties that can be set using
``SetDefaults`` on the model class or ``SetStatus`` on a device instance:

===========  ======================================================================
 label        A string (default: `“”`) specifying an arbitrary textual label for
              the device. Recording backends might use the label to generate
              device specific identifiers like filenames and such.
 n_events     The number of events that were collected by the recorder can be
              read out of the `n_events` entry. The number of events can be
              reset to 0. Other values cannot be set.
 origin       A positive floating point number (default : `0.0`) used as the
              reference time for `start` and `stop`.
 record_to    A string (default: `“memory”`) containing the name of the recording
              backend where to write data to. An empty string turns all
              recording of individual events off.
 start        A positive floating point number (default: `0.0`) specifying the
              activation time in ms, relative to origin.
 stop         A floating point number (default: `infinity`) specifying the
              deactivation time in ms, relative to origin. The value of stop
              must be greater than or equal to start
===========  ======================================================================

To learn more about recording devices in NEST, please refer to the
:doc:`../guides/recording_from_simulations` guide.

EndUserDocs */

namespace nest
{

/**
 * Class spike_recorder
 */

class spike_recorder : public RecordingDevice
{

public:
  spike_recorder();
  spike_recorder( const spike_recorder& );

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
spike_recorder::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline SignalType
spike_recorder::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef SPIKE_RECORDER_H */
