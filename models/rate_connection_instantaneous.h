/*
 *  rate_connection_instantaneous.h
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


#ifndef RATE_CONNECTION_INSTANTANEOUS_H
#define RATE_CONNECTION_INSTANTANEOUS_H

#include "connection.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Synapses
@ingroup inst_rate

Name: rate_connection_instantaneous - Synapse type for instantaneous rate
connections.

Description:

rate_connection_instantaneous is a connector to create
instantaneous connections between rate model neurons.

The value of the parameter delay is ignored for connections of
this type. To create rate connections with delay please use
the synapse type rate_connection_delayed.

Transmits: InstantaneousRateConnectionEvent

References:

\verbatim embed:rst
.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI: https://doi.org/10.3389/fninf.2017.00034
\endverbatim

Author: David Dahmen, Jan Hahne, Jannis Schuecker

SeeAlso: rate_connection_delayed, rate_neuron_ipn, rate_neuron_opn
*/

/**
 * Class representing a rate connection. A rate connection
 * has the properties weight and receiver port.
 */
template < typename targetidentifierT >
class RateConnectionInstantaneous : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef InstantaneousRateConnectionEvent EventType;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  RateConnectionInstantaneous()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase.
  // This avoids explicit name prefixes in all places these functions are used.
  // Since ConnectionBase depends on the template parameter, they are not
  // automatically
  // found in the base class.
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

  void
  set_delay( double )
  {
    throw BadProperty(
      "rate_connection_instantaneous has no delay. Please use "
      "rate_connection_delayed." );
  }

private:
  double weight_; //!< connection weight
};

template < typename targetidentifierT >
void
RateConnectionInstantaneous< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
RateConnectionInstantaneous< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // If the delay is set, we throw a BadProperty
  if ( d->known( names::delay ) )
  {
    throw BadProperty(
      "rate_connection_instantaneous has no delay. Please use "
      "rate_connection_delayed." );
  }

  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef RATE_CONNECTION_INSTANTANEOUS_H */
