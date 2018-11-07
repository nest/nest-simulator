/*
 *  stdp_connection_hom.h
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

#ifndef STDP_CONNECTION_HOM_H
#define STDP_CONNECTION_HOM_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "connection.h"

namespace nest
{

/** @BeginDocumentation
Name: stdp_synapse_hom - Synapse type for spike-timing dependent
plasticity using homogeneous parameters.

Description:

stdp_synapse_hom is a connector to create synapses with spike time
dependent plasticity (as defined in [1]). Here the weight dependence
exponent can be set separately for potentiation and depression.

 Parameters controlling plasticity are identical for all synapses of the
 model, reducing the memory required per synapse considerably.

Examples:

multiplicative STDP [2]  mu_plus = mu_minus = 1.0
additive STDP       [3]  mu_plus = mu_minus = 0.0
Guetig STDP         [1]  mu_plus = mu_minus = [0.0,1.0]
van Rossum STDP     [4]  mu_plus = 0.0 mu_minus = 1.0

Parameters:

tau_plus   double - Time constant of STDP window, potentiation in ms
                    (tau_minus defined in post-synaptic neuron)
lambda     double - Step size
alpha      double - Asymmetry parameter (scales depressing increments as
                    alpha*lambda)
mu_plus    double - Weight dependence exponent, potentiation
mu_minus   double - Weight dependence exponent, depression
Wmax       double - Maximum allowed weight

Remarks:

The parameters are common to all synapses of the model and must be set using
SetDefaults on the synapse model.

Transmits: SpikeEvent

References:

[1] Guetig et al. (2003) Learning Input Correlations through Nonlinear
    Temporally Asymmetric Hebbian Plasticity. Journal of Neuroscience

[2] Rubin, J., Lee, D. and Sompolinsky, H. (2001). Equilibrium
    properties of temporally asymmetric Hebbian plasticity, PRL
    86,364-367

[3] Song, S., Miller, K. D. and Abbott, L. F. (2000). Competitive
    Hebbian learning through spike-timing-dependent synaptic
    plasticity,Nature Neuroscience 3:9,919--926

[4] van Rossum, M. C. W., Bi, G-Q and Turrigiano, G. G. (2000).
    Stable Hebbian learning from spike timing-dependent
    plasticity, Journal of Neuroscience, 20:23,8812--8821

FirstVersion: March 2006

Author: Moritz Helias, Abigail Morrison

SeeAlso: synapsedict, tsodyks_synapse, static_synapse
*/
/**
 * Class containing the common properties for all synapses of type
 * STDPConnectionHom.
 */
class STDPHomCommonProperties : public CommonSynapseProperties
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPHomCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  // data members common to all connections
  double tau_plus_;
  double lambda_;
  double alpha_;
  double mu_plus_;
  double mu_minus_;
  double Wmax_;
};


/**
 * Class representing an STDP connection with homogeneous parameters, i.e.
 * parameters are the same for all synapses.
 */
template < typename targetidentifierT >
class STDPConnectionHom : public Connection< targetidentifierT >
{

public:
  typedef STDPHomCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPConnectionHom();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPConnectionHom( const STDPConnectionHom& );


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
   */
  void send( Event& e, thread t, const STDPHomCommonProperties& );

  void
  set_weight( double w )
  {
    weight_ = w;
  }


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

private:
  double
  facilitate_( double w, double kplus, const STDPHomCommonProperties& cp )
  {
    double norm_w = ( w / cp.Wmax_ )
      + ( cp.lambda_ * std::pow( 1.0 - ( w / cp.Wmax_ ), cp.mu_plus_ )
                      * kplus );
    return norm_w < 1.0 ? norm_w * cp.Wmax_ : cp.Wmax_;
  }

  double
  depress_( double w, double kminus, const STDPHomCommonProperties& cp )
  {
    double norm_w =
      ( w / cp.Wmax_ ) - ( cp.alpha_ * cp.lambda_
                           * std::pow( w / cp.Wmax_, cp.mu_minus_ ) * kminus );
    return norm_w > 0.0 ? norm_w * cp.Wmax_ : 0.0;
  }

  // data members of each connection
  double weight_;
  double Kplus_;
  double t_lastspike_;
};


//
// Implementation of class STDPConnectionHom.
//

template < typename targetidentifierT >
STDPConnectionHom< targetidentifierT >::STDPConnectionHom()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
STDPConnectionHom< targetidentifierT >::STDPConnectionHom(
  const STDPConnectionHom& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , Kplus_( rhs.Kplus_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 */
template < typename targetidentifierT >
inline void
STDPConnectionHom< targetidentifierT >::send( Event& e,
  thread t,
  const STDPHomCommonProperties& cp )
{
  // synapse STDP depressing/facilitation dynamics

  const double t_spike = e.get_stamp().get_ms();

  // t_lastspike_ = 0 initially

  Node* target = get_target( t );
  double dendritic_delay = get_delay();

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike_ - dendritic_delay,
    t_spike - dendritic_delay,
    &start,
    &finish );
  // facilitation due to post-synaptic spikes since last pre-synaptic spike
  double minus_dt;
  while ( start != finish )
  {
    minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );
    ++start;
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    assert( minus_dt < -1.0 * kernel().connection_manager.get_stdp_eps() );
    weight_ =
      facilitate_( weight_, Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp );
  }

  // depression due to new pre-synaptic spike
  weight_ =
    depress_( weight_, target->get_K_value( t_spike - dendritic_delay ), cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) / cp.tau_plus_ ) + 1.0;

  t_lastspike_ = t_spike;
}

template < typename targetidentifierT >
void
STDPConnectionHom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  // base class properties, different for individual synapse
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  // own properties, different for individual synapse
  def< double >( d, names::Kplus, Kplus_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
STDPConnectionHom< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  // base class properties
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::Kplus, Kplus_ );
}

} // of namespace nest

#endif // of #ifndef STDP_CONNECTION_HOM_H
