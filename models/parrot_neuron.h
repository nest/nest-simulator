/*
 *  parrot_neuron.h
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

#ifndef PARROT_NEURON_H
#define PARROT_NEURON_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"

namespace nest
{

/* BeginUserDocs: neuron, parrot

Short description
+++++++++++++++++

Neuron that repeats incoming spikes

Description
+++++++++++

The parrot neuron simply emits one spike for every incoming spike.
An important application is to provide identical poisson spike
trains to a group of neurons. The poisson_generator sends a different
spike train to each of its target neurons. By connecting one
poisson_generator to a parrot_neuron and then that parrot_neuron to
a group of neurons, all target neurons will receive the same poisson
spike train.

Remarks:

- Weights on connection to the parrot_neuron are ignored.
- Weights on connections from the parrot_neuron are handled as usual.
- Delays are honored on incoming and outgoing connections.
- Multiplicity may be used to indicate number of spikes in a single
  time step. Instead of the accumulated weigths of the incoming spikes, the
  number of the spikes is stored within a ring buffer.

Only spikes arriving on connections to port 0 will be repeated.
Connections onto port 1 will be accepted, but spikes incoming
through port 1 will be ignored. This allows setting exact pre-
and postsynaptic spike times for STDP protocols by connecting
two parrot neurons spiking at desired times by, e.g., a
stdp_synapse onto port 1 on the postsynaptic parrot neuron.

Receives
++++++++

SpikeEvent

Sends
+++++

SpikeEvent

EndUserDocs */

class parrot_neuron : public ArchivingNode
{

public:
  parrot_neuron();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding,
   * Overloading, and Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;
  using Node::receives_signal;

  port send_test_event( Node&, rport, synindex, bool );
  SignalType sends_signal() const;
  SignalType receives_signal() const;

  void handle( SpikeEvent& );
  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void
  init_state_( const Node& )
  {
  } // no state
  void init_buffers_();
  void
  calibrate()
  {
  } // no variables

  void update( Time const&, const long, const long );

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
parrot_neuron::sends_signal() const
{
  return ALL;
}

inline SignalType
parrot_neuron::receives_signal() const
{
  return ALL;
}

} // namespace

#endif // PARROT_NEURON_H
