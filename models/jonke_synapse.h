/*
 *  jonke_synapse.h
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

#ifndef JONKE_SYNAPSE_H
#define JONKE_SYNAPSE_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection.h"
#include "connection_manager.h"
#include "connector_model.h"
#include "event.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/* BeginUserDocs: synapse, spike-timing-dependent plasticity

Short description
+++++++++++++++++

Synapse type for spike-timing dependent plasticity with additional additive factors.

Description
+++++++++++

jonke_synapse is a connector to create synapses with spike time
dependent plasticity. Unlike ``stdp_synapse``, we use the update equations:

.. math::

   \Delta w &= \lambda \cdot w_{max} \cdot (K_+(w) \cdot F_+(t) - \beta)  & \quad  if t - t_j^(k) > 0 \\
   \Delta w &= \lambda \cdot w_{max} \cdot (-alpha \cdot K_-(w) \cdot F_-(t) - \beta)  & \quad  else

where

.. math::

   K_+(w) &= \exp(\nu_+ w) \\
   K_-(w) &= \exp(\nu_- w)

and

.. math::

   F_+(t) &= \exp((t - t_j^(k))/\tau_+) \\
   F_-(t) &= \exp((t - t_j^(k))/\tau_-)

This makes it possible to implement update rules which approximate the
rule stated in [1]_, and for examples, the rules given in [2]_ and [3]_.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

Parameters
++++++++++

========== ========  ======================================================
 lambda     double    Step size
 Wmax       double    Maximum allowed weight, note that this scales each
                      weight update
 alpha      double    Determine shape of depression term
 mu_plus    double    Set weight dependency of facilitating update
 mu_minus   double    Set weight dependency of depressing update
 tau_plus   double    Time constant of STDP window, potentiation in ms
 beta       double    Set negative offset for both updates
========== ========  ======================================================

(tau_minus is defined in the postsynaptic neuron.)

Transmits
+++++++++

SpikeEvent

References
++++++++++

.. [1] Nessler, Bernhard, et al. "Bayesian computation emerges in generic
       cortical microcircuits through spike-timing-dependent plasticity." PLoS
       computational biology 9.4 (2013): e1003037.
.. [2] Legenstein, Robert, et al. "Assembly pointers for variable binding in
       networks of spiking neurons." arXiv preprint arXiv:1611.03698 (2016).
.. [3] Jonke, Zeno, et al. "Feedback inhibition shapes emergent computational
       properties of cortical microcircuit motifs." arXiv preprint
       arXiv:1705.07614 (2017).

See also
++++++++

synapsedict, stdp_synapse

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: jonke_synapse

EndUserDocs */


/**
 * Class containing the common properties for all synapses of type dopamine
 * connection.
 */
class JonkeCommonProperties : public CommonSynapseProperties
{
public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  JonkeCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  double alpha_;
  double beta_;
  double lambda_;
  double mu_plus_;
  double mu_minus_;
  double tau_plus_;
  double Wmax_;
};


// connections are templates of target identifier type (used for pointer /
// target index addressing) derived from generic connection template
void register_jonke_synapse( const std::string& name );

template < typename targetidentifierT >
class jonke_synapse : public Connection< targetidentifierT >
{

public:
  typedef JonkeCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  jonke_synapse();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  jonke_synapse( const jonke_synapse& ) = default;

  jonke_synapse& operator=( const jonke_synapse& ) = default;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay;
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
  bool send( Event& e, size_t t, const JonkeCommonProperties& cp );


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
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike_ - get_delay(), get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  facilitate_( double w, double kplus, const JonkeCommonProperties& cp )
  {
    if ( cp.lambda_ == 0.0 )
    {
      return w;
    }

    double K_w = std::exp( cp.mu_plus_ * w );
    double F_t = kplus;

    double dW = cp.lambda_ * ( K_w * F_t - cp.beta_ );
    double new_w = w + dW;

    return new_w < cp.Wmax_ ? new_w : cp.Wmax_;
  }

  double
  depress_( double w, double kminus, const JonkeCommonProperties& cp )
  {
    if ( cp.lambda_ == 0.0 )
    {
      return w;
    }

    double K_w = std::exp( cp.mu_minus_ * w );
    double F_t = kminus;

    double dW = cp.lambda_ * ( -cp.alpha_ * K_w * F_t - cp.beta_ );
    double new_w = w + dW;

    return new_w > 0.0 ? new_w : 0.0;
  }

  // data members of each connection
  double weight_;
  double Kplus_;
  double t_lastspike_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties jonke_synapse< targetidentifierT >::properties;

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
inline bool
jonke_synapse< targetidentifierT >::send( Event& e, size_t t, const JonkeCommonProperties& cp )
{
  // synapse STDP depressing/facilitation dynamics
  const double t_spike = e.get_stamp().get_ms();

  // use accessor functions (inherited from Connection< >) to obtain delay and
  // target
  Node* target = get_target( t );
  double dendritic_delay = get_delay();

  // get spike history in relevant range (t1, t2] from postsynaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;

  // For a new synapse, t_lastspike_ contains the point in time of the last
  // spike. So we initially read the
  // history(t_last_spike - dendritic_delay, ..., T_spike-dendritic_delay]
  // which increases the access counter for these entries.
  // At registration, all entries' access counters of
  // history[0, ..., t_last_spike - dendritic_delay] have been
  // incremented by ArchivingNode::register_stdp_connection(). See bug #218 for
  // details.
  target->get_history( t_lastspike_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish );
  // facilitation due to postsynaptic spikes since last pre-synaptic spike
  double minus_dt;
  while ( start != finish )
  {
    minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );
    ++start;
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    assert( minus_dt < -1.0 * kernel::manager< ConnectionManager >.get_stdp_eps() );
    weight_ = facilitate_( weight_, Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp );
  }

  const double _K_value = target->get_K_value( t_spike - dendritic_delay );
  weight_ = depress_( weight_, _K_value, cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  // use accessor functions (inherited from Connection< >) to obtain delay in
  // steps and rport
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) / cp.tau_plus_ ) + 1.0;

  t_lastspike_ = t_spike;

  return true;
}


template < typename targetidentifierT >
jonke_synapse< targetidentifierT >::jonke_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
void
jonke_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::Kplus, Kplus_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
jonke_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::Kplus, Kplus_ );

  if ( Kplus_ < 0 )
  {
    throw BadProperty( "Kplus must be non-negative." );
  }
}

} // of namespace nest

#endif // of #ifndef JONKE_SYNAPSE_H
