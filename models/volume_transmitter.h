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


namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Node used in combination with neuromodulated synaptic plasticity

Description
+++++++++++

The volume transmitter is used in combination with neuromodulated
synaptic plasticity, plasticity that depends not only on the activity
of the pre- and the postsynaptic neuron but also on a non-local
neuromodulatory third signal. It collects the spikes from all neurons
connected to the volume transmitter and delivers the spikes to a subset
of synapses in the network. The user specifies this subset by
passing the volume transmitter as a parameter when a
neuromodulatory synapse is defined.

It is assumed that the neuromodulatory signal is a function of the
spike times of all spikes emitted by the population of neurons connected
to the volume transmitter. The neuromodulatory dynamics is calculated
in the synapses itself.

The volume transmitter interacts in a hybrid structure with the neuromodulated
synapses: In addition to the delivery of the neuromodulatory spikes triggered
by every pre-synaptic spike, the neuromodulatory spike history is delivered
at regular time intervals. The interval is equal to ``deliver_interval * d_min``,
where ``deliver_interval`` is an (integer) entry in the parameter dictionary and
``d_min`` is the minimal synaptic delay.

The implementation is based on the framework presented in [1]_.

Parameters
++++++++++

- deliver_interval - time interval given in d_min time steps in which
                     the volume signal is delivered from the volume
                     transmitter to the assigned synapses. Must be integer.

References
++++++++++


.. [1] Potjans W, Morrison A, Diesmann M (2010). Enabling functional
       neural circuit simulations with distributed computing of
       neuromodulated plasticity. Frontiers in Computattional Neuroscience,
       4:141. DOI: https://doi.org/10.3389/fncom.2010.00141


Receives
++++++++

SpikeEvent

See also
++++++++

stdp_dopamine_synapse

EndUserDocs */

class ConnectorBase;

class volume_transmitter : public ArchivingNode
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

  Name
  get_element_type() const
  {
    return names::other;
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

  /**
   * Since volume transmitters are duplicated on each thread, and are
   * hence treated just as devices during node creation, we need to
   * define the corresponding setter and getter for local_device_id.
   **/
  void set_local_device_id( const index ldid );
  index get_local_device_id() const;

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
    void set( const DictionaryDatum&, Node* node );
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

  index local_device_id_;
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
  ArchivingNode::get_status( d );
}

inline void
volume_transmitter::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline const std::vector< nest::spikecounter >&
volume_transmitter::deliver_spikes()
{
  return B_.spikecounter_;
}

inline void
volume_transmitter::set_local_device_id( const index ldid )
{
  local_device_id_ = ldid;
}

inline index
volume_transmitter::get_local_device_id() const
{
  return local_device_id_;
}

} // namespace

#endif /* #ifndef VOLUME_TRANSMITTER_H */
