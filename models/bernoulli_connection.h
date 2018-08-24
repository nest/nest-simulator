/*
 *  bernoulli_connection.h
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
  Name: bernoulli_synapse - Static synapse with stochastic transmission.

  Description:
    Spikes are transmitted by bernoulli_synapse following a Bernoulli trial with
    success probability p_transmit. This synaptic mechanism was inspired by the
    results described in [1] of greater transmission probability for stronger
    excitatory connections and it was previously applied in [2] and [3].

    bernoulli_synapse does not support any kind of plasticity. It simply stores
    the parameters target, weight, transmission probability, delay and
    receiver port for each connection.

  Parameters:
    p_transmit double - Transmission probability, must be between 0 and 1

  FirstVersion: June 2017
  Author: Susanne Kunkel, Maximilian Schmidt, Milena Menezes Carvalho

  Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
  DoubleDataEvent, DataLoggingRequest

  SeeAlso: synapsedict, static_synapse, static_synapse_hom_w

  References:

    [1] Sandrine Lefort, Christian Tomm, J.-C. Floyd Sarria, Carl C.H. Petersen,
  The Excitatory Neuronal Network of the C2 Barrel Column in Mouse Primary
  Somatosensory Cortex, Neuron, Volume 61, Issue 2, 29 January 2009, Pages
  301-316, DOI: 10.1016/j.neuron.2008.12.020.

    [2] Jun-nosuke Teramae, Yasuhiro Tsubo & Tomoki Fukai, Optimal spike-based
  communication in excitable networks with strong-sparse and weak-dense links,
  Scientific Reports 2, Article number: 485 (2012), DOI: 10.1038/srep00485

    [3] Yoshiyuki Omura, Milena M. Carvalho, Kaoru Inokuchi, Tomoki Fukai, A
  Lognormal Recurrent Network Model for Burst Generation during Hippocampal
  Sharp Waves, Journal of Neuroscience 28 October 2015, 35 (43) 14585-14601,
  DOI: 10.1523/JNEUROSCI.4944-14.2015
*/

#ifndef BERNOULLI_CONNECTION_H
#define BERNOULLI_CONNECTION_H

// Includes from nestkernel:
#include "connection.h"
#include "kernel_manager.h"

namespace nest
{

/**
 * Class representing a Bernoulli connection. A Bernoulli connection has the
 * properties weight, transmission probability, delay and receiver port.
 * A suitable Connector containing these connections can be obtained from
 * the template GenericConnector.
 */

template < typename targetidentifierT >
class BernoulliConnection : public Connection< targetidentifierT >
{
public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  BernoulliConnection()
    : ConnectionBase()
    , weight_( 1.0 )
    , p_transmit_( 1.0 )
  {
  }

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  BernoulliConnection( const BernoulliConnection& rhs )
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
  };

  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  send( Event& e, thread t, const CommonSynapseProperties& )
  {
    SpikeEvent e_spike = static_cast< SpikeEvent& >( e );

    librandom::RngPtr rng = kernel().rng_manager.get_rng( t );
    const unsigned long n_spikes_in = e_spike.get_multiplicity();
    unsigned long n_spikes_out = 0;

    for ( unsigned long n = 0; n < n_spikes_in; ++n )
    {
      if ( rng->drand() < p_transmit_ )
      {
        ++n_spikes_out;
      }
    }

    if ( n_spikes_out > 0 )
    {
      e_spike.set_multiplicity( n_spikes_out );
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
BernoulliConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::p_transmit, p_transmit_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
BernoulliConnection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::p_transmit, p_transmit_ );

  if ( p_transmit_ < 0 || p_transmit_ > 1 )
  {
    throw BadProperty( "Spike transmission probability must be in [0, 1]." );
  }
}

} // namespace

#endif /* #ifndef BERNOULLI_CONNECTION_H */
