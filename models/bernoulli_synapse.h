/*
 *  bernoulli_synapse.h
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


#ifndef BERNOULLI_SYNAPSE_H
#define BERNOULLI_SYNAPSE_H

// Includes from nestkernel:
#include "connection.h"
#include "kernel_manager.h"

namespace nest
{

/* BeginUserDocs: synapse, static

Short description
+++++++++++++++++

Static synapse with stochastic transmission

Description
+++++++++++

Spikes are transmitted by bernoulli_synapse following a Bernoulli
trial with success probability p_transmit. This synaptic mechanism was
inspired by the results described in [1]_ of greater transmission
probability for stronger excitatory connections and it was previously
applied in [2]_ and [3].

bernoulli_synapse does not support any kind of plasticity. It simply
stores the parameters target, weight, transmission probability, delay
and receiver port for each connection.

Parameters
++++++++++

=========== ====== ===================================================
 p_transmit real   Transmission probability, must be between 0 and 1
=========== ====== ===================================================

Transmits
+++++++++

SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
DoubleDataEvent, DataLoggingRequest

References
++++++++++

.. [1] Lefort S, Tomm C, Sarria J-C F, Petersen CCH (2009). The excitatory
       neuronal network of the C2 barrel column in mouse primary
       somatosensory cortex. Neuron, 61(2):301-316.
       DOI: https://doi.org/10.1016/j.neuron.2008.12.020.

.. [2] Teramae J, Tsubo Y, Fukai T (2012). Optimal spike-based communication
       in excitable networks with strong-sparse and weak-dense  links,
       Scientific Reports 2,485. DOI: https://doi.org/10.1038/srep00485

.. [3] Omura Y, Carvalho MM, Inokuchi K, Fukai T (2015). A lognormal recurrent
       network model for burst generation during hippocampal sharp waves.
       Journal of Neuroscience, 35(43):14585-14601.
       DOI: https://doi.org/10.1523/JNEUROSCI.4944-14.2015

See also
++++++++

static_synapse, static_synapse_hom_w

EndUserDocs */

template < typename targetidentifierT >
class bernoulli_synapse : public Connection< targetidentifierT >
{
public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  bernoulli_synapse()
    : ConnectionBase()
    , weight_( 1.0 )
    , p_transmit_( 1.0 )
  {
  }

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  bernoulli_synapse( const bernoulli_synapse& rhs ) = default;

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
  check_connection( Node& s, Node& t, rport receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  send( Event& e, thread t, const CommonSynapseProperties& )
  {
    SpikeEvent e_spike = static_cast< SpikeEvent& >( e );

    const unsigned long n_spikes_in = e_spike.get_multiplicity();
    unsigned long n_spikes_out = 0;

    for ( unsigned long n = 0; n < n_spikes_in; ++n )
    {
      if ( get_vp_specific_rng( t )->drand() < p_transmit_ )
      {
        ++n_spikes_out;
      }
    }

    if ( n_spikes_out > 0 )
    {
      e_spike.set_multiplicity( n_spikes_out );
      e.set_weight( weight_ );
      e.set_delay_steps( get_delay_steps() );
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
bernoulli_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::p_transmit, p_transmit_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
bernoulli_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
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

#endif /* #ifndef BERNOULLI_SYNAPSE_H */
