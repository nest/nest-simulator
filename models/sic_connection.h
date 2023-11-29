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

/* BeginUserDocs: synapse, astrocyte

Short description
+++++++++++++++++

Synapse type for astrocyte-neuron connections

Description
+++++++++++

``sic_connection`` connects an astrocyte to a target neuron. It exposes the target neuron to a slow inward current (SIC)
induced by the astrocyte. The amplitude of the current is the product of the astrocytic current and the weight of the
``sic_connection``.

The source node of a ``sic_connection`` must be an astrocyte emitting a slow inward current, and the target node must be
able to handle slow inward current input. Currently, :doc:`aeif_cond_alpha_astro` is the only neuron model that can
receive ``sic_connection``. The connection may have a delay.

Sends
+++++

SICEvent

See also
++++++++

astrocyte_lr_1994, aeif_cond_alpha_astro

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: sic_connection

EndUserDocs */

void register_sic_connection( const std::string& name );

template < typename targetidentifierT >
class sic_connection : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef SICEvent EventType;
  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  sic_connection()
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
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
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
constexpr ConnectionModelProperties sic_connection< targetidentifierT >::properties;

template < typename targetidentifierT >
void
sic_connection< targetidentifierT >::get_status( DictionaryDatum& d ) const
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
sic_connection< targetidentifierT >::get_secondary_event()
{
  return new SICEvent();
}

template < typename targetidentifierT >
void
sic_connection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef SIC_CONNECTION_H */
