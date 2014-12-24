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

#include "nest.h"
#include "event.h"
#include "node.h"
#include "slice_ring_buffer.h"
#include "connection.h"

/* BeginDocumentation
Name: parrot_neuron_ps - Neuron that repeats incoming spikes handling
precise spike times.

Description:
  The parrot neuron simply emits one spike for every incoming spike.
  One possible application for this is to create different channels
  for the output of generator devices such as the poisson_generator
  or the mip_generator.

Remarks:
  Network-wise the parrot neuron behaves like other neuron models
  regarding connections and communication. While the number of
  outgoing spikes equals that of incoming ones, the weigth of the 
  outgoing spikes solely depends on the weigth of outgoing connections.

  A Poisson generator that would send multiple spikes during a single
  time step due to a high rate will send single spikes with 
  multiple synaptic strength instead, for effiacy reasons.
  This can be realized because of the way devices are implemented
  in the threaded environment. A parrot neuron on the other 
  hand always emits single spikes. Hence, in a situation where for 
  example a poisson generator with a high firing rate is connected
  to a parrot neuron, the communication cost associated with outgoing
  spikes is much bigger for the latter.

  Please note that this node is capable of sending precise spike times
  to target nodes (on-grid spike time plus offset). If this node is
  connected to a spike_detector, the property "precise_times" of the
  spike_detector has to be set to true in order to record the offsets
  in addition to the on-grid spike times.

Parameters: 
  No parameters to be set in the status dictionary.

References:
  No references

Sends: SpikeEvent

Receives: SpikeEvent
  
Author: adapted from parrot_neuron by Kunkel
*/

namespace nest
{
  class parrot_neuron_ps :
    public Node
  {
  class Network;  
  
  public:        
    
    parrot_neuron_ps();

    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using Node::handle;
    using Node::handles_test_event;
    
    void handle(SpikeEvent &);
    port send_test_event(Node&, rport, synindex, bool);
    port handles_test_event(SpikeEvent &, rport);

    void get_status(DictionaryDatum &) const {}
    void set_status(const DictionaryDatum &) {}

    // uses off_grid events
    bool is_off_grid() const
    {
      return true;
    }

  private:
      
    void init_state_(Node const &){} // no state
    void init_buffers_();
    void calibrate(){}               // no variables
    
    void update(Time const &, const long_t, const long_t);

    /** Queue for incoming events. */
    struct Buffers_
    {
      SliceRingBuffer events_;
    };
    
    Buffers_ B_;
  };

  inline
  port parrot_neuron_ps::send_test_event(Node& target, rport receptor_type, synindex, bool)
  {
    SpikeEvent e;
    e.set_sender(*this);
  
    return target.handles_test_event(e, receptor_type);
  }

  inline
  port parrot_neuron_ps::handles_test_event(SpikeEvent&, rport receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }
  
} // namespace

#endif //PARROT_NEURON_PS_H
