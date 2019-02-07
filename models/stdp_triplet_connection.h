/*
 *  stdp_triplet_connection.h
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

#ifndef STDP_TRIPLET_CONNECTION_H
#define STDP_TRIPLET_CONNECTION_H

// C-header for math.h since copysign() is in C99 but not C++98
#include <math.h>
#include "connection.h"

namespace nest
{

/** @BeginDocumentation
Name: stdp_triplet_synapse - Synapse type with spike-timing dependent
                             plasticity (triplets).

Description:

stdp_triplet_synapse is a connection with spike time dependent
plasticity accounting for spike triplet effects (as defined in [1]).

STDP examples:
  pair-based   Aplus_triplet = Aminus_triplet = 0.0
  triplet      Aplus_triplet = Aminus_triplet = 1.0

Parameters:

tau_plus           double - time constant of short presynaptic trace
                            - (tau_plus of [1])
tau_plus_triplet   double - time constant of long presynaptic trace
                            - (tau_x of [1])
Aplus              double - weight of pair potentiation rule
                          - (A_plus_2 of [1])
Aplus_triplet      double - weight of triplet potentiation rule
                          - (A_plus_3 of [1])
Aminus             double - weight of pair depression rule
                            (A_minus_2 of [1])
Aminus_triplet     double - weight of triplet depression rule
                          - (A_minus_3 of [1])
Wmax               double - maximum allowed weight

States:
  Kplus              double: pre-synaptic trace (r_1 of [1])
  Kplus_triplet      double: triplet pre-synaptic trace (r_2 of [1])

Transmits: SpikeEvent

References:

[1] J.-P. Pfister & W. Gerstner (2006) Triplets of Spikes in a Model
      of Spike Timing-Dependent Plasticity.  The Journal of Neuroscience
      26(38):9673-9682; doi:10.1523/JNEUROSCI.1425-06.2006

Notes:
- Presynaptic traces r_1 and r_2 of [1] are stored in the connection as
  Kplus and Kplus_triplet and decay with time-constants tau_plus and
  tau_plus_triplet, respectively.
- Postsynaptic traces o_1 and o_2 of [1] are acquired from the post-synaptic
  neuron states Kminus_ and triplet_Kminus_ which decay on time-constants
  tau_minus and tau_minus_triplet, respectively. These two time-constants
  can be set as properties of the postsynaptic neuron.
- This version implements the 'all-to-all' spike interaction of [1]. The
  'nearest-spike' interaction of [1] can currently not be implemented
  without changing the postsynaptic archiving-node (clip the traces to a
  maximum of 1).

FirstVersion: Nov 2007

Author: Abigail Morrison, Eilif Muller, Alexander Seeholzer, Teo Stocco
Adapted by: Philipp Weidel

SeeAlso: stdp_triplet_synapse_hpc, synapsedict, stdp_synapse, static_synapse
*/
// connections are templates of target identifier type
// (used for pointer / target index addressing)
// derived from generic connection template
template < typename targetidentifierT >
class STDPTripletConnection : public Connection< targetidentifierT >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPTripletConnection();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPTripletConnection( const STDPTripletConnection& );

  /**
   * Default Destructor.
   */
  ~STDPTripletConnection()
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places
  // these functions are used. Since ConnectionBase depends on the template
  // parameter, they are not automatically found in the base class.
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

  /*
   * This function calls check_connection on the sender and checks if the
   * receiver accepts the event type and receptor type requested by the sender.
   * Node::check_connection() will either confirm the receiver port by returning
   * true or false if the connection should be ignored.
   * We have to override the base class' implementation, since for STDP
   * connections we have to call register_stdp_connection on the target neuron
   * to inform the Archiver to collect spikes for this connection.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   */
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
  inline double
  facilitate_( double w, double kplus, double ky )
  {
    double new_w = std::abs( w ) + kplus * ( Aplus_ + Aplus_triplet_ * ky );
    return copysign( new_w < std::abs( Wmax_ ) ? new_w : Wmax_, Wmax_ );
  }

  inline double
  depress_( double w, double kminus, double Kplus_triplet_ )
  {
    double new_w =
      std::abs( w ) - kminus * ( Aminus_ + Aminus_triplet_ * Kplus_triplet_ );
    return copysign( new_w > 0.0 ? new_w : 0.0, Wmax_ );
  }

  // data members of each connection
  double weight_;
  double tau_plus_;
  double tau_plus_triplet_;
  double Aplus_;
  double Aminus_;
  double Aplus_triplet_;
  double Aminus_triplet_;
  double Kplus_;
  double Kplus_triplet_;
  double Wmax_;
  double t_lastspike_;
};

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
inline void
STDPTripletConnection< targetidentifierT >::send( Event& e,
  thread t,
  const CommonSynapseProperties& )
{

  double t_spike = e.get_stamp().get_ms();
  double dendritic_delay = get_delay();
  Node* target = get_target( t );

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike_ - dendritic_delay,
    t_spike - dendritic_delay,
    &start,
    &finish );

  // facilitation due to post-synaptic spikes since last pre-synaptic spike
  while ( start != finish )
  {
    // post-synaptic spike is delayed by dendritic_delay so that
    // it is effectively late by that much at the synapse.
    double minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );

    // subtract 1.0 yields the triplet_Kminus value just prior to
    // the post synaptic spike, implementing the t-epsilon in
    // Pfister et al, 2006
    double ky = start->triplet_Kminus_ - 1.0;
    ++start;
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    assert( minus_dt < -1.0 * kernel().connection_manager.get_stdp_eps() );
    weight_ =
      facilitate_( weight_, Kplus_ * std::exp( minus_dt / tau_plus_ ), ky );
  }

  // depression due to new pre-synaptic spike
  Kplus_triplet_ *= std::exp( ( t_lastspike_ - t_spike ) / tau_plus_triplet_ );

  // dendritic delay means we must look back in time by that amount
  // for determining the K value, because the K value must propagate
  // out to the synapse
  weight_ = depress_(
    weight_, target->get_K_value( t_spike - dendritic_delay ), Kplus_triplet_ );

  Kplus_triplet_ += 1.0;
  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) / tau_plus_ ) + 1.0;

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  t_lastspike_ = t_spike;
}

// Defaults come from reference [1] data fitting and table 3.
template < typename targetidentifierT >
STDPTripletConnection< targetidentifierT >::STDPTripletConnection()
  : ConnectionBase()
  , weight_( 1.0 )
  , tau_plus_( 16.8 )
  , tau_plus_triplet_( 101.0 )
  , Aplus_( 5e-10 )
  , Aminus_( 7e-3 )
  , Aplus_triplet_( 6.2e-3 )
  , Aminus_triplet_( 2.3e-4 )
  , Kplus_( 0.0 )
  , Kplus_triplet_( 0.0 )
  , Wmax_( 100.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
STDPTripletConnection< targetidentifierT >::STDPTripletConnection(
  const STDPTripletConnection< targetidentifierT >& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , tau_plus_( rhs.tau_plus_ )
  , tau_plus_triplet_( rhs.tau_plus_triplet_ )
  , Aplus_( rhs.Aplus_ )
  , Aminus_( rhs.Aminus_ )
  , Aplus_triplet_( rhs.Aplus_triplet_ )
  , Aminus_triplet_( rhs.Aminus_triplet_ )
  , Kplus_( rhs.Kplus_ )
  , Kplus_triplet_( rhs.Kplus_triplet_ )
  , Wmax_( rhs.Wmax_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}

template < typename targetidentifierT >
void
STDPTripletConnection< targetidentifierT >::get_status(
  DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::tau_plus, tau_plus_ );
  def< double >( d, names::tau_plus_triplet, tau_plus_triplet_ );
  def< double >( d, names::Aplus, Aplus_ );
  def< double >( d, names::Aminus, Aminus_ );
  def< double >( d, names::Aplus_triplet, Aplus_triplet_ );
  def< double >( d, names::Aminus_triplet, Aminus_triplet_ );
  def< double >( d, names::Kplus, Kplus_ );
  def< double >( d, names::Kplus_triplet, Kplus_triplet_ );
  def< double >( d, names::Wmax, Wmax_ );
}

template < typename targetidentifierT >
void
STDPTripletConnection< targetidentifierT >::set_status(
  const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::tau_plus, tau_plus_ );
  updateValue< double >( d, names::tau_plus_triplet, tau_plus_triplet_ );
  updateValue< double >( d, names::Aplus, Aplus_ );
  updateValue< double >( d, names::Aminus, Aminus_ );
  updateValue< double >( d, names::Aplus_triplet, Aplus_triplet_ );
  updateValue< double >( d, names::Aminus_triplet, Aminus_triplet_ );
  updateValue< double >( d, names::Kplus, Kplus_ );
  updateValue< double >( d, names::Kplus_triplet, Kplus_triplet_ );
  updateValue< double >( d, names::Wmax, Wmax_ );

  // check if weight_ and Wmax_ has the same sign
  if ( not( ( ( weight_ >= 0 ) - ( weight_ < 0 ) )
         == ( ( Wmax_ >= 0 ) - ( Wmax_ < 0 ) ) ) )
  {
    throw BadProperty( "Weight and Wmax must have same sign." );
  }

  if ( not( Kplus_ >= 0 ) )
  {
    throw BadProperty( "State Kplus must be positive." );
  }

  if ( not( Kplus_triplet_ >= 0 ) )
  {
    throw BadProperty( "State Kplus_triplet must be positive." );
  }
}

} // of namespace nest

#endif // of #ifndef STDP_TRIPLET_CONNECTION_H
