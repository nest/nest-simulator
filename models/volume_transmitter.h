/*
 *  volume_transmitter.h
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

#ifndef VOLUME_TRANSMITTER_H
#define VOLUME_TRANSMITTER_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "spikecounter.h"

// Includes from sli:
#include "namedatum.h"


/* BeginDocumentation

Name: volume_transmitter - Node used in combination with neuromodulated synaptic
plasticity. It collects all spikes emitted by the population of neurons
connected to the volume transmitter and transmits the signal to a user-specific
subset of synapses.

Description:
The volume transmitter is used in combination with neuromodulated
synaptic plasticty, plasticity that depends not only on the activity
of the pre- and the postsynaptic neuron but also on a non-local
neuromodulatory third signal. It collects the spikes from all neurons
connected to the volume transmitter and delivers the spikes to a
user-specific subset of synapses.  It is assumed that the
neuromodulatory signal is a function of the spike times of all spikes
emitted by the population of neurons connected to the volume
transmitter.  The neuromodulatory dynamics is calculated in the
synapses itself. The volume transmitter interacts in a hybrid
structure with the neuromodulated synapses. In addition to the
delivery of the neuromodulatory spikes triggered by every pre-synaptic
spike, the neuromodulatory spike history is delivered in discrete time
intervals of a manifold of the minimal synaptic delay. In order to
insure the link between the neuromodulatory synapses and the volume
transmitter, the volume transmitter is passed as a parameter when a
neuromodulatory synapse is defined. The implementation is based on the
framework presented in [1].

Examples:
/volume_transmitter Create /vol Set
/iaf_psc_alpha Create /pre_neuron Set
/iaf_psc_alpha Create /post_neuron Set
/iaf_psc_alpha Create /neuromod_neuron Set
/stdp_dopamine_synapse  << /vt vol >>  SetDefaults
neuromod_neuron vol Connect
pre_neuron post_neuron /stdp_dopamine_synapse Connect

Parameters:
deliver_interval - time interval given in d_min time steps, in which
                   the volume signal is delivered from the volume
                   transmitter to the assigned synapses

References:
[1] Potjans W, Morrison A and Diesmann M (2010). Enabling functional
    neural circuit simulations with distributed computing of
    neuromodulated plasticity.
    Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141

Author: Wiebke Potjans, Abigail Morrison
Remarks: major changes to update function after code revision in Apr 2013 (SK)
Receives: SpikeEvent

SeeAlso: stdp_dopamine_synapse

*/

namespace nest
{

class ConnectorBase;

/**
 * volume transmitter class.
 *
 * This class manages spike recording for normal and precise spikes. It
 * receives spikes via its handle(SpikeEvent&) method and buffers them. In the
 * update() method it stores the newly collected buffer elements, which are
 * delivered in time steps of (d_min*deliver_interval) to the neuromodulated
 * synapses. In addition the synapses can ask the volume transmitter to deliver
 * the elements stored in the update() method with the method deliver_spikes().
 *
 * @ingroup Devices
 */
class volume_transmitter : public Archiving_Node
{

public:
  volume_transmitter();
  volume_transmitter( const volume_transmitter& );

  bool
  has_proxies() const
  {
    return false;
  }
  bool
  local_receiver() const
  {
    return false;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d );

  const std::vector< spikecounter >& deliver_spikes();

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();

  void update( const Time&, const long, const long );

  // --------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    Parameters_();
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
    long deliver_interval_; //!< update interval in d_min time steps
  };

  //-----------------------------------------------

  struct Buffers_
  {
    RingBuffer neuromodulatory_spikes_; //!< buffer to store incoming spikes
    //! vector to store and deliver spikes
    std::vector< spikecounter > spikecounter_;
  };

  Parameters_ P_;
  Buffers_ B_;
};

inline port
volume_transmitter::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
volume_transmitter::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::other );
}

inline void
volume_transmitter::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline const std::vector< nest::spikecounter >&
volume_transmitter::deliver_spikes()
{
  return B_.spikecounter_;
}

} // namespace

#endif /* #ifndef VOLUME_TRANSMITTER_H */
