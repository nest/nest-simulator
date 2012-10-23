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


  /* BeginDocumentation
Name: parrot_neuron - Neuron that repeats incoming spikes.

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

Receives: SpikeEvent
  
Sends: SpikeEvent
  
Parameters: 

  No parameters to be set in the status dictionary.

References:
  No references

Author:  May 2006, Reichert, Morrison
*/



/**
 * The parrot neuron emits one spike for every incoming spike.
 * It is a (strongly) simplified version of the iaf_neuron class,
 * stripped of the dynamics and unneeded features.
 * Instead of the accumulated weigths of the incoming spikes the
 * number of the spikes is stored within a ring buffer.
 * \author David Reichert
 * \date may 2006 
 */

#ifndef PARROT_NEURON_H
#define PARROT_NEURON_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"

namespace nest
{
  class Network;

  class parrot_neuron: public Archiving_Node
  {
    
  public:        
    
    parrot_neuron();

    /**
     * Import sets of overloaded virtual functions.
     * We need to explicitly include sets of overloaded
     * virtual functions into the current scope.
     * According to the SUN C++ FAQ, this is the correct
     * way of doing things, although all other compilers
     * happily live without.
     */

    using Node::connect_sender;
    using Node::handle;

    port check_connection(Connection&, port);
    
    void handle(SpikeEvent &);
    
    port connect_sender(SpikeEvent &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:
      
    void init_state_(const Node&){}  // no state
    void init_buffers_();
    void calibrate(){}  // no variables
    
    void update(Time const &, const long_t, const long_t);

    /**
       Buffers and accumulates the number of incoming spikes per time step; 
       RingBuffer stores doubles; for now the numbers are casted.
    */
    struct Buffers_ {
      RingBuffer n_spikes_;
    };
    
    Buffers_ B_;
  };

  inline
  port parrot_neuron::check_connection(Connection& c, port receptor_type)
  {
    SpikeEvent e;
    e.set_sender(*this);
    c.check_event(e);
    return c.get_target()->connect_sender(e, receptor_type);
  }

  inline
  port parrot_neuron::connect_sender(SpikeEvent&, port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }
  
} // namespace

#endif //PARROT_NEURON_H
