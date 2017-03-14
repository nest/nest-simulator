/*
 *  stdp_pl_connection_hom.h
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

#ifndef STDP_PL_CONNECTION_HOM_H
#define STDP_PL_CONNECTION_HOM_H

/* BeginDocumentation
  Name: stdp_pl_synapse_hom - Synapse type for spike-timing dependent
   plasticity with power law implementation using homogeneous parameters, i.e.
   all synapses have the same parameters.

  Description:
   stdp_pl_synapse is a connector to create synapses with spike time
   dependent plasticity (as defined in [1]).


  Parameters:
   tau_plus  double - Time constant of STDP window, potentiation in ms
                      (tau_minus defined in post-synaptic neuron)
   lambda    double - Learning rate
   alpha     double - Asymmetry parameter (scales depressing increments as
                      alpha*lambda)
   mu        double - Weight dependence exponent, potentiation

  Remarks:
   The parameters can only be set by SetDefaults and apply to all synapses of
   the model.

  References:
   [1] Morrison et al. (2007) Spike-timing dependent plasticity in balanced
       random networks. Neural Computation.

  Transmits: SpikeEvent

  FirstVersion: May 2007
  Author: Abigail Morrison
  SeeAlso: synapsedict, stdp_synapse, tsodyks_synapse, static_synapse
*/

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "connection.h"

namespace nest
{

/**
 * Class containing the common properties for all synapses of type
 * STDPConnectionHom.
 */
class STDPPLHomCommonProperties : public CommonSynapseProperties
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPPLHomCommonProperties();

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
  double mu_;
};


/**
 * Class representing an STDP connection with homogeneous parameters, i.e.
 * parameters are the same for all synapses.
 */
template < typename targetidentifierT >
class STDPPLConnectionHom : public Connection< targetidentifierT >
{

public:
  typedef STDPPLHomCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;


  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPPLConnectionHom();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPPLConnectionHom( const STDPPLConnectionHom& );

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
   * \param t_lastspike Point in time of last spike sent.
   */
  void send( Event& e,
    thread t,
    double t_lastspike,
    const STDPPLHomCommonProperties& );

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
   * connections we have to call register_stdp_pl_connection on the target
   * neuron to inform the Archiver to collect spikes for this connection.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   * \param t_lastspike last spike produced by presynaptic neuron (in ms)
   */
  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    double t_lastspike,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike - get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  facilitate_( double w, double kplus, const STDPPLHomCommonProperties& cp )
  {
    return w + ( cp.lambda_ * std::pow( w, cp.mu_ ) * kplus );
  }

  double
  depress_( double w, double kminus, const STDPPLHomCommonProperties& cp )
  {
    double new_w = w - ( cp.lambda_ * cp.alpha_ * w * kminus );
    return new_w > 0.0 ? new_w : 0.0;
  }

  // data members of each connection
  double weight_;
  double Kplus_;
};

//
// Implementation of class STDPPLConnectionHom.
//

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
template < typename targetidentifierT >
inline void
STDPPLConnectionHom< targetidentifierT >::send( Event& e,
  thread t,
  double t_lastspike,
  const STDPPLHomCommonProperties& cp )
{
  // synapse STDP depressing/facilitation dynamics

  double t_spike = e.get_stamp().get_ms();

  // t_lastspike_ = 0 initially

  Node* target = get_target( t );

  double dendritic_delay = get_delay();

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history(
    t_lastspike - dendritic_delay, t_spike - dendritic_delay, &start, &finish );

  // facilitation due to post-synaptic spikes since last pre-synaptic spike
  double minus_dt;
  while ( start != finish )
  {
    minus_dt = t_lastspike - ( start->t_ + dendritic_delay );
    start++;
    if ( minus_dt == 0 )
    {
      continue;
    }
    weight_ =
      facilitate_( weight_, Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp );
  }

  // depression due to new pre-synaptic spike
  weight_ =
    depress_( weight_, target->get_K_value( t_spike - dendritic_delay ), cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ = Kplus_ * std::exp( ( t_lastspike - t_spike ) / cp.tau_plus_ ) + 1.0;
}

template < typename targetidentifierT >
STDPPLConnectionHom< targetidentifierT >::STDPPLConnectionHom()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
{
}

template < typename targetidentifierT >
STDPPLConnectionHom< targetidentifierT >::STDPPLConnectionHom(
  const STDPPLConnectionHom& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , Kplus_( rhs.Kplus_ )
{
}

template < typename targetidentifierT >
void
STDPPLConnectionHom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  // base class properties, different for individual synapse
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  // own properties, different for individual synapse
  def< double >( d, "Kplus", Kplus_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
STDPPLConnectionHom< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  // base class properties
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, "Kplus", Kplus_ );
}

} // of namespace nest

#endif // of #ifndef STDP_PL_CONNECTION_HOM_H
