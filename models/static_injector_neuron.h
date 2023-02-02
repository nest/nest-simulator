/*
 *  static_injector_neuron.h
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

#ifndef STATIC_INJECTOR_NEURON_H
#define STATIC_INJECTOR_NEURON_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"

namespace nest
{

/* BeginUserDocs: neuron, static injector

Short description
+++++++++++++++++

Neuron that emits prescribed spikes

Description
+++++++++++

The static injector neuron simply emits spikes at prescribed spike times.
Incoming spikes will be ignored. The static injector neuron behaves similarly
to a spike generator, but is treated internally as a neuron and not a device.
Unlike a spike generator which is replicated at each virtual process, the
static injector neuron resides on a single virtual process. Spikes emitted
by the static injector neuron will be counted by the local spike count.

Receives
++++++++

None

Sends
+++++

SpikeEvent

EndUserDocs */

class static_injector_neuron : public Node
{

public:
  static_injector_neuron();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding,
   * Overloading, and Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool ) override;
  SignalType sends_signal() const override;
  SignalType receives_signal() const override;

  void handle( SpikeEvent& ) override;
  port handles_test_event( SpikeEvent&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void
  pre_run_hook() override
  {
  } // no variables

  void update( Time const&, const long, const long ) override;

  /**
     Buffers and accumulates the number of incoming spikes per time step;
     RingBuffer stores doubles; for now the numbers are casted.
  */
  struct Buffers_
  {
    RingBuffer n_spikes_;
  };

  Buffers_ B_;
};

inline port
parrot_neuron::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

// ------------------------------
inline port
spike_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  enforce_single_syn_type( syn_id );

  if ( dummy_target )
  {
    DSSpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
  else
  {
    SpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
}

void
nest::StimulationDevice::enforce_single_syn_type( synindex syn_id )
{
  if ( first_syn_id_ == invalid_synindex )
  {
    first_syn_id_ = syn_id;
  }
  if ( syn_id != first_syn_id_ )
  {
    throw IllegalConnection( "All outgoing connections from a device must use the same synapse type." );
  }
}
// -------------------------------

inline port
parrot_neuron::handles_test_event( SpikeEvent&, rport receptor_type )
{
  // Allow connections to port 0 (spikes to be repeated)
  // and port 1 (spikes to be ignored).
  if ( receptor_type == 0 or receptor_type == 1 )
  {
    return receptor_type;
  }
  else
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
}

inline SignalType
static_injector_neuron::sends_signal() const
{
  return SPIKE;
}

inline SignalType
static_injector_neuron::receives_signal() const
{
  return NONE;
}

} // namespace

#endif // STATIC_INJECTOR_NEURON_H
