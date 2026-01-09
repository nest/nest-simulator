/*
 *  static_synapse.h
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

#ifndef STATICSYNAPSE_H
#define STATICSYNAPSE_H

// Includes from nestkernel:
#include "connection.h"
#include "event_delivery_manager.h"

namespace nest
{

/* BeginUserDocs: synapse, static

Short description
+++++++++++++++++

Synapse type for static connections

Description
+++++++++++

``static_synapse`` does not support any kind of plasticity. It simply stores
the parameters target, weight, delay and receiver port for each connection.

Transmits
+++++++++

SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
DoubleDataEvent, DataLoggingRequest

See also
++++++++

tsodyks_synapse, stdp_synapse

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: static_synapse

EndUserDocs */

void register_static_synapse( const std::string& name );

template < typename targetidentifierT >
class static_synapse : public Connection< targetidentifierT >
{
  double weight_;

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  static_synapse()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  static_synapse( const static_synapse& rhs ) = default;
  static_synapse& operator=( const static_synapse& rhs ) = default;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;


  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    size_t
    handles_test_event( SpikeEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( RateEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DataLoggingRequest&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( CurrentEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( ConductanceEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DoubleDataEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DSSpikeEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DSCurrentEvent&, size_t ) override
    {
      return invalid_port;
    }
  };

  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  bool
  send( Event& e, const size_t tid, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_delay_steps( get_delay_steps() );
    e.set_receiver( *get_target( tid ) );
    e.set_rport( get_rport() );
    e();
    return true;
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double w )
  {
    weight_ = w;
  }
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties static_synapse< targetidentifierT >::properties;

template < typename targetidentifierT >
void
static_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
static_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef STATICSYNAPSE_H */
