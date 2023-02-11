/*
 *  sic_connection.h
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

#ifndef SIC_CONNECTION_H
#define SIC_CONNECTION_H

#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse

Short description
+++++++++++++++++

Synapse type for connections from astrocytes to neurons

Description
+++++++++++

``sic_connection`` is a connector to create connections from astrocytes to
neurons. It is adapted from ``gap_junction`` but unidirectional.
``sic_connection`` sends current to neurons. The value of current is
determined by astrocytes. The neuron model compatible with this connector is
``aeif_cond_alpha_astro``.

As a part of neuron-astrocyte networks, connections of this type should be
created with the connectivity rule ``pairwise_bernoulli_astro``.

The value of the parameter ``delay`` is ignored for connections of
type ``sic_connection``.

See also [1]_, [2]_.

Sends
+++++

SICEvent

References
++++++++++

.. [1] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101
.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473.
.. [3] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal netowrk simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

See also
++++++++

gap_junction, astrocyte, aeif_cond_alpha_astro

EndUserDocs */

template < typename targetidentifierT >
class SICConnection : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef SICEvent EventType;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  SICConnection()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  SecondaryEvent* get_secondary_event();

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  void
  check_connection( Node& s, Node& t, rport receptor_type, const CommonPropertiesType& )
  {
    EventType ge;

    s.sends_secondary_event( ge );
    ge.set_sender( s );
    Connection< targetidentifierT >::target_.set_rport( t.handles_test_event( ge, receptor_type ) );
    Connection< targetidentifierT >::target_.set_target( &t );
  }

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param p The port under which this connection is stored in the Connector.
   */
  void
  send( Event& e, thread t, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_delay_steps( get_delay_steps() );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double w )
  {
    weight_ = w;
  }

  // void
  // set_delay( double )
  // {
  //   throw BadProperty( "sic_connection connection has no delay" );
  // }

private:
  double weight_; //!< connection weight
};

template < typename targetidentifierT >
void
SICConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  // We have to include the delay here to prevent
  // errors due to internal calls of
  // this function in SLI/pyNEST
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
SecondaryEvent*
SICConnection< targetidentifierT >::get_secondary_event()
{
  return new SICEvent();
}

template < typename targetidentifierT >
void
SICConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // // If the delay is set, we throw a BadProperty
  // if ( d->known( names::delay ) )
  // {
  //   throw BadProperty( "sic_connection connection has no delay" );
  // }

  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef SIC_CONNECTION_H */
