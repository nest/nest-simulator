/*
 *  vogels_sprekeler_connection.h
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

#ifndef VOGELS_SPREKELER_CONNECTION_H
#define VOGELS_SPREKELER_CONNECTION_H

// C-header for math.h since copysign() is in C99 but not C++98
#include <math.h>
#include "connection.h"

namespace nest
{

/** @BeginDocumentation
Name: vogels_sprekeler_synapse - Synapse type for symmetric spike-timing
dependent
plasticity with constant depression.

Description:
vogels_sprekeler_synapse is a connector to create synapses with symmetric
spike time dependent plasticity and constant depression (as defined in [1]).
The learning rule is symmetric, i.e., the synapse is strengthened
irrespective of the order of the pre and post-synaptic spikes. Each
pre-synaptic spike also causes a constant depression of the synaptic weight
which differentiates this rule from other classical stdp rules.

Parameters:
tau        double - Time constant of STDP window, potentiation in ms
Wmax       double - Maximum allowed weight
eta        double - learning rate
alpha      double - constant depression (= 2 * tau * target firing rate in
 [1])

Transmits: SpikeEvent

References:
[1] Vogels et al. (2011) Inhibitory Plasticity Balances Excitation and
Inhibition in Sensory Pathways and Memory Networks.
Science Vol. 334, Issue 6062, pp. 1569-1573
DOI: 10.1126/science.1211095

FirstVersion: January 2016

Author: Ankur Sinha

SeeAlso: synapsedict
*/
// connections are templates of target identifier type (used for pointer /
// target index addressing)
// derived from generic connection template
template < typename targetidentifierT >
class VogelsSprekelerConnection : public Connection< targetidentifierT >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  VogelsSprekelerConnection();


  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  VogelsSprekelerConnection( const VogelsSprekelerConnection& );

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase.
  // This avoids explicit name prefixes in all places these functions are used.
  // Since ConnectionBase depends on the template parameter, they are not
  // automatically
  // found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_delay;
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
   * \param t_lastspike Point in time of last spike sent.
   * \param cp common properties of all synapses (empty).
   */
  void send( Event& e, thread t, const CommonSynapseProperties& cp );


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

    t.register_stdp_connection( t_lastspike_ - get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  facilitate_( double w, double kplus )
  {
    double new_w = std::abs( w ) + ( eta_ * kplus );
    return copysign( new_w < std::abs( Wmax_ ) ? new_w : Wmax_, Wmax_ );
  }

  double
  depress_( double w )
  {
    double new_w = std::abs( w ) - ( alpha_ * eta_ );
    return copysign( new_w > 0.0 ? new_w : 0.0, Wmax_ );
  }

  // data members of each connection
  double weight_;
  double tau_;
  double alpha_;
  double eta_;
  double Wmax_;
  double Kplus_;

  double t_lastspike_;
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param t_lastspike Time point of last spike emitted
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
inline void
VogelsSprekelerConnection< targetidentifierT >::send( Event& e,
  thread t,
  const CommonSynapseProperties& )
{
  // synapse STDP depressing/facilitation dynamics
  double t_spike = e.get_stamp().get_ms();
  // t_lastspike_ = 0 initially

  // use accessor functions (inherited from Connection< >) to obtain delay and
  // target
  Node* target = get_target( t );
  double dendritic_delay = get_delay();

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike_ - dendritic_delay,
    t_spike - dendritic_delay,
    &start,
    &finish );

  // presynaptic neuron j, post synaptic neuron i
  // Facilitation for each post synaptic spike
  // Wij = Wij + eta*xj
  double minus_dt;
  while ( start != finish )
  {
    minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );
    ++start;
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    assert( minus_dt < -1.0 * kernel().connection_manager.get_stdp_eps() );
    weight_ = facilitate_( weight_, Kplus_ * std::exp( minus_dt / tau_ ) );
  }

  // For pre-synaptic spikes
  // Wij = Wij + eta(xi - alpha)
  // Facilitation and constant depression
  // Getting kvalue at required time already for deferred processing, so no
  // need to transform it to the current time, and so, no exponential required
  weight_ =
    facilitate_( weight_, target->get_K_value( t_spike - dendritic_delay ) );
  weight_ = depress_( weight_ );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  // use accessor functions (inherited from Connection< >) to obtain delay in
  // steps and rport
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  // exponential part for the decay, addition of one for each spike
  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) / tau_ ) + 1.0;

  t_lastspike_ = t_spike;
}


template < typename targetidentifierT >
VogelsSprekelerConnection< targetidentifierT >::VogelsSprekelerConnection()
  : ConnectionBase()
  , weight_( 0.5 )
  , tau_( 20.0 )
  , alpha_( 0.12 )
  , eta_( 0.001 )
  , Wmax_( 1.0 )
  , Kplus_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
VogelsSprekelerConnection< targetidentifierT >::VogelsSprekelerConnection(
  const VogelsSprekelerConnection< targetidentifierT >& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , tau_( rhs.tau_ )
  , alpha_( rhs.alpha_ )
  , eta_( rhs.eta_ )
  , Wmax_( rhs.Wmax_ )
  , Kplus_( rhs.Kplus_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}

template < typename targetidentifierT >
void
VogelsSprekelerConnection< targetidentifierT >::get_status(
  DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::tau, tau_ );
  def< double >( d, names::alpha, alpha_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmax, Wmax_ );
  def< double >( d, names::Kplus, Kplus_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
VogelsSprekelerConnection< targetidentifierT >::set_status(
  const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::tau, tau_ );
  updateValue< double >( d, names::alpha, alpha_ );
  updateValue< double >( d, names::eta, eta_ );
  updateValue< double >( d, names::Wmax, Wmax_ );
  updateValue< double >( d, names::Kplus, Kplus_ );

  // if the weight_ is not 0, we check to ensure that weight_ and Wmax_ are of
  // the same sign
  if ( weight_ != 0 and ( std::signbit( weight_ ) != std::signbit( Wmax_ ) ) )
  {
    throw BadProperty( "Weight and Wmax must have same sign." );
  }

  if ( not( Kplus_ >= 0 ) )
  {
    throw BadProperty( "State Kplus must be positive." );
  }
}
} // of namespace nest

#endif // of #ifndef VOGELS_SPREKELER_CONNECTION_H
