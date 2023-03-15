/*
 *  parrot_neuron_ps.h
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

#ifndef PARROT_NEURON_PS_H
#define PARROT_NEURON_PS_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "slice_ring_buffer.h"

namespace nest
{

/* BeginUserDocs: neuron, parrot, precise

Short description
+++++++++++++++++

Neuron that repeats incoming spikes - precise spike timing version

Description
+++++++++++

The parrot neuron simply emits one spike for every incoming spike.
An important application is to provide identical poisson spike
trains to a group of neurons. The poisson_generator sends a different
spike train to each of its target neurons. By connecting one
poisson_generator to a parrot_neuron and then that parrot_neuron to
a group of neurons, all target neurons will receive the same poisson
spike train.

Please note that weights of connections *to* the ``parrot_neuron``
are ignored, while weights on connections *from* the ``parrot_neuron``
to the target are handled as usual. Delays are honored on both
incoming and outgoing connections.

Only spikes arriving on connections to port 0 will be repeated.
Connections onto port 1 will be accepted, but spikes incoming
through port 1 will be ignored. This allows setting exact pre-
and postsynaptic spike times for STDP protocols by connecting
two parrot neurons spiking at desired times by, e.g., a
stdp_synapse onto port 1 on the postsynaptic parrot neuron.

Please note that this node is capable of sending precise spike times
to target nodes (on-grid spike time plus offset).

Receives
++++++++

SpikeEvent

Sends
+++++

SpikeEvent

EndUserDocs */

class parrot_neuron_ps : public ArchivingNode
{
public:
  parrot_neuron_ps();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& ) override;
  port send_test_event( Node&, rport, synindex, bool ) override;
  port handles_test_event( SpikeEvent&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  bool
  is_off_grid() const override
  {
    return true;
  }

private:
  void init_buffers_() override;

  void
  pre_run_hook() override
  {
  } // no variables

  void update( Time const&, const long, const long ) override;

  /** Queue for incoming events. */
  struct Buffers_
  {
    SliceRingBuffer events_;
  };

  Buffers_ B_;
};

inline port
parrot_neuron_ps::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
parrot_neuron_ps::handles_test_event( SpikeEvent&, rport receptor_type )
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

} // namespace

#endif // PARROT_NEURON_PS_H
