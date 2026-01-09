/*
 *  static_synapse_hom_w.h
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

#ifndef STATICSYNAPSE_HOM_W_H
#define STATICSYNAPSE_HOM_W_H

// Includes from nestkernel:
#include "common_properties_hom_w.h"
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, static

Short description
+++++++++++++++++

Synapse type for static connections with homogeneous weight

Description
+++++++++++

``static_synapse_hom_w`` does not support any kind of plasticity. It simply
stores the parameters delay, target, and receiver port for each connection
and uses a common weight for all connections.

The common weight for all connections of this model must be set by
``SetDefaults`` on the model. If you create copies of this model using
``CopyModel``, each derived model can have a different weight.

Transmits
+++++++++

SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
DataLoggingRequest, DoubleDataEvent

See also
++++++++

static_synapse

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: static_synpase_hom_w

EndUserDocs */

void register_static_synapse_hom_w( const std::string& name );

template < typename targetidentifierT >
class static_synapse_hom_w : public Connection< targetidentifierT, TotalDelay >
{

public:
  // this line determines which common properties to use
  typedef CommonPropertiesHomW CommonPropertiesType;
  typedef Connection< targetidentifierT, TotalDelay > ConnectionBase;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL;

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

  void get_status( DictionaryDatum& d ) const;

  void
  check_connection( Node& s, Node& t, const size_t receptor_type, const synindex syn_id, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, syn_id, receptor_type );
  }

  /**
   * Checks to see if weight is given in syn_spec.
   */
  void
  check_synapse_params( const DictionaryDatum& syn_spec ) const
  {
    if ( syn_spec->known( names::weight ) )
    {
      throw BadProperty(
        "Weight cannot be specified since it needs to be equal "
        "for all connections when static_synapse_hom_w is used." );
    }
  }

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param tid Thread ID of the target
   * \param cp Common properties-object of the synapse
   */
  bool
  send( Event& e, const size_t tid, const CommonPropertiesHomW& cp )
  {
    e.set_weight( cp.get_weight() );
    e.set_delay_steps( get_delay_steps() );
    e.set_receiver( *get_target( tid ) );
    e.set_rport( get_rport() );
    e();

    return true;
  }

  void
  set_weight( double )
  {
    throw BadProperty(
      "Setting of individual weights is not possible! The common weights can "
      "be changed via "
      "CopyModel()." );
  }
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties static_synapse_hom_w< targetidentifierT >::properties;

template < typename targetidentifierT >
void
static_synapse_hom_w< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< long >( d, names::size_of, sizeof( *this ) );
}

} // namespace

#endif /* #ifndef STATICSYNAPSE_HOM_W_H */
