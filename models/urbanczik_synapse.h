/*
 *  urbanczik_synapse.h
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

#ifndef URBANCZIK_SYNAPSE_H
#define URBANCZIK_SYNAPSE_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection.h"
#include "connector_model.h"
#include "event.h"
#include "ring_buffer.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/* BeginUserDocs: synapse, spike-timing-dependent plasticity

Short description
+++++++++++++++++

Synapse type for a plastic synapse after Urbanczik and Senn

Description
+++++++++++

``urbanczik_synapse`` is a connector to create Urbanczik synapses as defined in
[1]_ that can connect suitable :ref:`multicompartment models
<multicompartment-models>`. In contrast to most STDP models, the synaptic weight
depends on the postsynaptic dendritic potential, in addition to the pre- and
postsynaptic spike timing.

Urbanczik synapses require the archiving of the dendritic membrane potential
which is continuous in time. Therefore they can only be connected to neuron
models that are capable of doing this archiving. So far, the only compatible
model is :doc:`pp_cond_exp_mc_urbanczik <pp_cond_exp_mc_urbanczik>`.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

Parameters
++++++++++

=========   ====   =========================================================
eta         real   Learning rate
tau_Delta   real   Time constant of low pass filtering of the weight change
Wmax        real   Maximum allowed weight
Wmin        real   Minimum allowed weight
=========   ====   =========================================================

All other parameters are stored in the neuron models that are compatible
with the Urbanczik synapse.

Transmits
+++++++++

SpikeEvent

References
++++++++++

.. [1] Urbanczik R. and Senn W (2014). Learning by the dendritic
       prediction of somatic spiking. Neuron, 81:521 - 528.
       https://doi.org/10.1016/j.neuron.2013.11.030

See also
++++++++

stdp_synapse, clopath_synapse, pp_cond_exp_mc_urbanczik

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: urbanczik_synapse

EndUserDocs */

// connections are templates of target identifier type (used for pointer /
// target index addressing) derived from generic connection template

void register_urbanczik_synapse( const std::string& name );

template < typename targetidentifierT >
class urbanczik_synapse : public Connection< targetidentifierT, TotalDelay >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT, TotalDelay > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::REQUIRES_URBANCZIK_ARCHIVING
    | ConnectionModelProperties::SUPPORTS_HPC | ConnectionModelProperties::SUPPORTS_LBL
    | ConnectionModelProperties::SUPPORTS_WFR;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  urbanczik_synapse();


  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  urbanczik_synapse( const urbanczik_synapse& ) = default;
  urbanczik_synapse& operator=( const urbanczik_synapse& ) = default;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_ms;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param cp common properties of all synapses (empty).
   */
  bool send( Event& e, size_t t, const CommonSynapseProperties& cp );


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
  };

  void
  check_connection( Node& s, Node& t, const size_t receptor_type, const synindex syn_id, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, syn_id, receptor_type );

    t.register_stdp_connection( t_lastspike_ - get_delay_ms(), get_delay_ms(), 0 );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  // data members of each connection
  double weight_;
  double init_weight_;
  double tau_Delta_;
  double eta_;
  double Wmin_;
  double Wmax_;
  double PI_integral_;
  double PI_exp_integral_;
  double tau_L_trace_;
  double tau_s_trace_;

  double t_lastspike_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties urbanczik_synapse< targetidentifierT >::properties;

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
inline bool
urbanczik_synapse< targetidentifierT >::send( Event& e, size_t t, const CommonSynapseProperties& )
{
  double t_spike = e.get_stamp().get_ms();
  // use accessor functions (inherited from Connection< >) to obtain delay and target
  Node* target = get_target( t );
  double dendritic_delay = get_delay_ms();

  // get spike history in relevant range (t1, t2] from postsynaptic neuron
  std::deque< histentry_extended >::iterator start;
  std::deque< histentry_extended >::iterator finish;

  // for now we only support two-compartment neurons
  // in this case the dendritic compartment has index 1
  const int comp = 1;

  target->get_urbanczik_history( t_lastspike_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish, comp );

  double const g_L = target->get_g_L( comp );
  double const tau_L = target->get_tau_L( comp );
  double const C_m = target->get_C_m( comp );
  double const tau_s = weight_ > 0.0 ? target->get_tau_syn_ex( comp ) : target->get_tau_syn_in( comp );
  double dPI_exp_integral = 0.0;

  while ( start != finish )
  {
    double const t_up = start->t_ + dendritic_delay;     // from t_lastspike to t_spike
    double const minus_delta_t_up = t_lastspike_ - t_up; // from 0 to -delta t
    double const minus_t_down = t_up - t_spike;          // from -t_spike to 0
    double const PI =
      ( tau_L_trace_ * exp( minus_delta_t_up / tau_L ) - tau_s_trace_ * exp( minus_delta_t_up / tau_s ) ) * start->dw_;
    PI_integral_ += PI;
    dPI_exp_integral += exp( minus_t_down / tau_Delta_ ) * PI;
    ++start;
  }

  PI_exp_integral_ = ( exp( ( t_lastspike_ - t_spike ) / tau_Delta_ ) * PI_exp_integral_ + dPI_exp_integral );
  weight_ = PI_integral_ - PI_exp_integral_;
  weight_ = init_weight_ + weight_ * 15.0 * C_m * tau_s * eta_ / ( g_L * ( tau_L - tau_s ) );

  if ( weight_ > Wmax_ )
  {
    weight_ = Wmax_;
  }
  else if ( weight_ < Wmin_ )
  {
    weight_ = Wmin_;
  }

  e.set_receiver( *target );
  e.set_weight( weight_ );
  // use accessor functions (inherited from Connection< >) to obtain delay in steps and rport
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  // compute the trace of the presynaptic spike train
  tau_L_trace_ = tau_L_trace_ * std::exp( ( t_lastspike_ - t_spike ) / tau_L ) + 1.0;
  tau_s_trace_ = tau_s_trace_ * std::exp( ( t_lastspike_ - t_spike ) / tau_s ) + 1.0;

  t_lastspike_ = t_spike;

  return true;
}


template < typename targetidentifierT >
urbanczik_synapse< targetidentifierT >::urbanczik_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , init_weight_( 1.0 )
  , tau_Delta_( 100.0 )
  , eta_( 0.07 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
  , PI_integral_( 0.0 )
  , PI_exp_integral_( 0.0 )
  , tau_L_trace_( 0.0 )
  , tau_s_trace_( 0.0 )
  , t_lastspike_( -1.0 )
{
}

template < typename targetidentifierT >
void
urbanczik_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::tau_Delta, tau_Delta_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
urbanczik_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::tau_Delta, tau_Delta_ );
  updateValue< double >( d, names::eta, eta_ );
  updateValue< double >( d, names::Wmin, Wmin_ );
  updateValue< double >( d, names::Wmax, Wmax_ );

  init_weight_ = weight_;
  // check if weight_ and Wmin_ has the same sign
  if ( std::signbit( weight_ ) != std::signbit( Wmax_ ) )
  {
    throw BadProperty( "Weight and Wmin must have same sign." );
  }

  // check if weight_ and Wmax_ has the same sign
  if ( not( ( ( weight_ >= 0 ) - ( weight_ < 0 ) ) == ( ( Wmax_ > 0 ) - ( Wmax_ <= 0 ) ) ) )
  {
    throw BadProperty( "Weight and Wmax must have same sign." );
  }
}

} // of namespace nest

#endif // of #ifndef URBANCZIK_SYNAPSE_H
