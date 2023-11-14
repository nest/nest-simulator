/*
 *  rate_connection_delayed.h
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


#ifndef RATE_CONNECTION_DELAYED_H
#define RATE_CONNECTION_DELAYED_H

#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, rate

Short description
+++++++++++++++++

Synapse type for rate connections with delay

Description
+++++++++++

``rate_connection_delayed`` is a connector to create connections with delay
between rate model neurons.

To create instantaneous rate connections please use
the synapse type ``rate_connection_instantaneous``.

See also [1]_.

Transmits
+++++++++

DelayedRateConnectionEvent

References
++++++++++

.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI: https://doi.org/10.3389/fninf.2017.00034

See also
++++++++

rate_connection_instantaneous, rate_neuron_ipn, rate_neuron_opn

EndUserDocs */

/**
 * Class representing a delayed rate connection. A rate_connection_delayed
 * has the properties weight, delay and receiver port.
 */
void register_rate_connection_delayed( const std::string& name );

template < typename targetidentifierT >
class rate_connection_delayed : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  rate_connection_delayed()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  SecondaryEvent* get_secondary_event();

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
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    DelayedRateConnectionEvent ge;

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
  send( Event& e, size_t t, const CommonSynapseProperties& )
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

private:
  double weight_; //!< connection weight
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties rate_connection_delayed< targetidentifierT >::properties;

template < typename targetidentifierT >
void
rate_connection_delayed< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
rate_connection_delayed< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

template < typename targetidentifierT >
SecondaryEvent*
rate_connection_delayed< targetidentifierT >::get_secondary_event()
{
  return new DelayedRateConnectionEvent();
}

} // namespace

#endif /* #ifndef RATE_CONNECTION_DELAYED_H */
