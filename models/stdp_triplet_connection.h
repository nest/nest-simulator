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

/* BeginDocumentation
  Name: stdp_triplet_synapse - Synapse type for spike-timing dependent 
  plasticity accounting for spike triplets as described in [1].

  Description:
   stdp_triplet_synapse is a connector to create synapses with spike time 
   dependent plasticity accounting for spike triplets (as defined in [1]). 
   
   Here, a multiplicative weight dependence is added (in contrast to [1]) 
   to depression resulting in a stable weight distribution.
  
  Parameters:

   tau_plus     time constant of STDP window, potentiation 
                (tau_minus defined in post-synaptic neuron)
   tau_x        time constant of triplet potentiation
   tau_y        time constant of triplet depression
   A_2p         weight of pair potentiation rule
   A_2m         weight of pair depression rule
   A_3p         weight of triplet potentiation rule
   A_3m         weight of triplet depression rule


  References:

   [1] J.-P. Pfister & W. Gerstner (2006) Triplets of Spikes in a Model 
   of Spike Timing-Dependent Plasticity.  The Journal of Neuroscience 
   26(38):9673-9682; doi:10.1523/JNEUROSCI.1425-06.2006

   Transmits: SpikeEvent

  FirstVersion: Nov 2007
  Author: Moritz Helias, Abigail Morrison, Eilif Muller
  SeeAlso: synapsedict, stdp_synapse, tsodyks_synapse, static_synapse
*/

#include <cmath>
#include "connection.h"

namespace nest
{
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
  STDPTripletConnection(const STDPTripletConnection &);


  /**
   * Default Destructor.
   */
  ~STDPTripletConnection() {}


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
   * \param t_lastspike Point in time of last spike sent.
   * \param cp common properties of all synapses (empty).
   */
  void send( Event& e, thread t, double_t t_lastspike, const CommonSynapseProperties& cp );


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
   * This function calls check_connection on the sender and checks if the receiver
   * accepts the event type and receptor type requested by the sender.
   * Node::check_connection() will either confirm the receiver port by returning
   * true or false if the connection should be ignored.
   * We have to override the base class' implementation, since for STDP
   * connections we have to call register_stdp_connection on the target neuron
   * to inform the Archiver to collect spikes for this connection.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   * \param t_lastspike last spike emitted by presynaptic neuron
   */
  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    double_t t_lastspike,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike - get_delay() );
  }

  void
  set_weight( double_t w )
  {
    weight_ = w;
  } 

 private:
  double_t 
  facilitate_(double_t w, double_t kplus, double_t ky)
  {
    return w+(A_2p_+A_3p_*ky)*kplus;
  }

  double_t 
  depress_(double_t w, double_t kminus, double_t kx)
  {
    double new_w = w - kminus*(A_2m_ + A_3m_*kx); 
    return new_w > 0.0 ? new_w : 0.0;
  }

  // data members of each connection
  double_t weight_;
  double_t tau_plus_;
  double_t tau_x_;
  double_t A_2p_;
  double_t A_2m_;
  double_t A_3p_;
  double_t A_3m_;
  double_t Kplus_;
  double_t Kx_;
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
STDPTripletConnection< targetidentifierT >::send( Event& e,
  thread t,
  double_t t_lastspike,
  const CommonSynapseProperties& )
{
  // synapse STDP depressing/facilitation dynamics
  
  double_t t_spike = e.get_stamp().get_ms();
  // t_lastspike_ = 0 initially

  // use accessor functions (inherited from Connection< >) to obtain delay and target
  Node* target = get_target( t );
  double_t dendritic_delay = get_delay(); 
    
  //get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque<histentry>::iterator start;
  std::deque<histentry>::iterator finish;    
  target->get_history(t_lastspike - dendritic_delay, t_spike - dendritic_delay, 
			       &start, &finish);
  //facilitation due to post-synaptic spikes since last pre-synaptic spike
  double_t minus_dt;
  double_t ky;
  while (start != finish)
  {      
    // post-synaptic spike is delayed by dendritic_delay so that
    // it is effectively late by that much at the synapse.
    minus_dt = t_lastspike - (start->t_ + dendritic_delay);
    // subtract 1.0 yields the triplet_Kminus value just prior to
    // the post synaptic spike, implementing the t-epsilon in 
    // Pfister et al, 2006
    ky = start->triplet_Kminus_-1.0;
    start++;
    if (minus_dt == 0)
      continue;
    weight_ = facilitate_(weight_, Kplus_ * std::exp(minus_dt / tau_plus_),ky);
  }

  //depression due to new pre-synaptic spike
  Kx_ *= std::exp((t_lastspike - t_spike) / tau_x_);

  // dendritic delay means we must look back in time by that amount
  // for determining the K value, because the K value must propagate
  // out to the synapse
  weight_ = depress_(weight_, target->get_K_value(t_spike - dendritic_delay),Kx_);

  Kx_ += 1.0;

  e.set_receiver( *target );
  e.set_weight( weight_ );
  // use accessor functions (inherited from Connection< >) to obtain delay in steps and rport
  e.set_delay( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ = Kplus_ * std::exp((t_lastspike - t_spike) / tau_plus_) + 1.0;

}

template < typename targetidentifierT >
STDPTripletConnection< targetidentifierT >::STDPTripletConnection()
  : ConnectionBase()
  , weight_( 1.0 )
  , tau_plus_( 20.0 )
  , tau_x_( 700.0 )
  , A_2p_( 0.01 )
  , A_2m_( 0.01 )
  , A_3p_( 0.01 )
  , A_3m_( 0.0 )
  , Kplus_( 0.0 )
  , Kx_( 0.0 )
{
}

template < typename targetidentifierT >
STDPTripletConnection< targetidentifierT >::STDPTripletConnection(
  const STDPTripletConnection< targetidentifierT >& rhs )
  : ConnectionBase(rhs)
  , weight_( rhs.weight_ )
  , tau_plus_( rhs.tau_plus_ )
  , tau_x_( rhs.tau_x_ )
  , A_2p_( rhs.A_2p_ )
  , A_2m_( rhs.A_2m_ )
  , A_3p_( rhs.A_3p_ )
  , A_3m_( rhs.A_3m_)
  , Kplus_( rhs.Kplus_ )
  , Kx_( rhs.Kx_ )
{
}

template < typename targetidentifierT >
void
STDPTripletConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double_t >( d, names::weight, weight_ );
  def< double_t >( d, "tau_plus", tau_plus_ );
  def< double_t >( d, "tau_x", tau_x_ );
  def< double_t >( d, "A_2p", A_2p_ );
  def< double_t >( d, "A_2m", A_2m_ );
  def< double_t >( d, "A_3p", A_3p_ );
  def< double_t >( d, "A_3m", A_3m_ );
  def< double_t >( d, "Kplus", Kplus_ );
  def<double_t>( d, "Kx", Kx_ );
}

template < typename targetidentifierT >
void
STDPTripletConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double_t >( d, names::weight, weight_ );
  updateValue< double_t >( d, "tau_plus", tau_plus_ );
  updateValue< double_t >( d, "tau_x", tau_x_ );
  updateValue< double_t >( d, "A_2p", A_2p_ );
  updateValue< double_t >( d, "A_2m", A_2m_ );
  updateValue< double_t >( d, "A_3p", A_3p_ );
  updateValue< double_t >( d, "A_3m", A_3m_ );
  updateValue< double_t >( d, "Kplus", Kplus_ );
  updateValue<double_t>( d, "Kx", Kx_ );
}

} // of namespace nest

#endif // of #ifndef STDP_TRIPLET_CONNECTION_H
