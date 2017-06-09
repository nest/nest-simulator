/*
 *  lossy_connection.h
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
  Name: lossy_synapse - Static synapse with stochastic transmission.

  Description:
   lossy_synapse does not support any kind of plasticity. It simply stores
   the parameters target, weight, transmission probability, delay and 
   receiver port for each connection.

  Parameters:
   p_transmit double - Transmission probability, must be between 0 and 1

  FirstVersion: June 2017
  Author: Susanne Kunkel, Maximilian Schmidt, Milena Menezes Carvalho

  Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
  DoubleDataEvent, DataLoggingRequest

  SeeAlso: synapsedict, static_synapse, static_synapse_hom_w
*/

#ifndef LOSSY_CONNECTION_H
#define LOSSY_CONNECTION_H

// Includes from nestkernel:
#include "connection.h"
#include "kernel_manager.h"

namespace nest
{

/**
 * Class representing a lossy connection. A lossy connection has the
 * properties weight, transmission probability, delay and receiver port. 
 * A suitable Connector containing these connections can be obtained from
 * the template GenericConnector.
 */

template < typename targetidentifierT >
class LossyConnection : public Connection< targetidentifierT >
{
public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;

  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  LossyConnection()
    : ConnectionBase()
    , weight_( 1.0 )
    , p_transmit_(1.0)
  {
  }

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  LossyConnection( const LossyConnection& rhs )
    : ConnectionBase( rhs )
    , weight_( rhs.weight_ )
    , p_transmit_( rhs.p_transmit_ )
  {
  }

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
    port
    handles_test_event( SpikeEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( RateEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( DataLoggingRequest&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( CurrentEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( ConductanceEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( DoubleDataEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( DSSpikeEvent&, rport )
    {
      return invalid_port_;
    }
    port
    handles_test_event( DSCurrentEvent&, rport )
    {
      return invalid_port_;
    }
  };

  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    double,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  send( Event& e, thread t, double, const CommonSynapseProperties& )
  {
    SpikeEvent e_spike = static_cast<SpikeEvent &>(e);

    librandom::RngPtr rng = kernel().rng_manager.get_rng( t );
    ulong n_spikes_in = e_spike.get_multiplicity();
    ulong n_spikes_out = 0;

    for ( ulong n = 0; n < n_spikes_in; n++ )
    {
      if ( rng->drand() < p_transmit_ )
      {
        n_spikes_out++;
      }
    }

    if ( n_spikes_out > 0 ) 
    {
      e_spike.set_multiplicity(n_spikes_out);
      e.set_weight( weight_ );
      e.set_delay( get_delay_steps() );
      e.set_receiver( *get_target( t ) );
      e.set_rport( get_rport() );
      e();
    }

    // Resets multiplicity for consistency
    e_spike.set_multiplicity( n_spikes_in );  
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double weight_;
  double p_transmit_;

};

template < typename targetidentifierT >
void
LossyConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::p_transmit, p_transmit_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
LossyConnection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::p_transmit, p_transmit_);

  if ( p_transmit_ < 0 || p_transmit_ > 1 )
  {
    throw BadProperty("Spike transmission probability must be in [0, 1].");
  }
}

} // namespace

#endif /* #ifndef LOSSY_CONNECTION_H */
