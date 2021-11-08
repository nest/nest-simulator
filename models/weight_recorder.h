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

/* BeginUserDocs: device, recorder

Short description
+++++++++++++++++

Recording weights from synapses

Description
+++++++++++

The change in synaptic weights over time is a key observable property in
studies of plasticity in neuronal network models. To access this information, the
``weight_recorder`` can be used. In contrast to other recording
devices, which are connected to a specific set of neurons, the weight
recorder is instead set as a parameter in the synapse model.

After assigning an instance of a weight recorder to the synapse model
by setting its ``weight_recorder`` property, the weight
recorder collects the global IDs of source and target neurons together
with the weight for each spike event that travels through the observed
synapses.

To only record from a subset of connected synapses, the
weight recorder accepts NodeCollections in the parameters ``senders`` and
``targets``. If set, they restrict the recording of data to only
synapses that fulfill the given criteria.

::

   >>> wr = nest.Create('weight_recorder')
   >>> nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr})

   >>> pre = nest.Create("iaf_psc_alpha", 10)
   >>> post = nest.Create("iaf_psc_alpha", 10)

   >>> nest.Connect(pre, post, syn_spec="stdp_synapse_rec")

.. include:: ../models/recording_device.rst

See also
++++++++

EndUserDocs */

namespace nest
{

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
  void calibrate();
  void update( Time const&, const long, const long );

  struct Parameters_
  {
    NodeCollectionDatum senders_;
    NodeCollectionDatum targets_;

    Parameters_();
    Parameters_( const Parameters_& ) = default;
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

inline SignalType
weight_recorder::receives_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef WEIGHT_RECORDER_H */
