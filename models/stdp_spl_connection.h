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

#ifndef STDP_SPL_CONNECTION_H
#define STDP_SPL_CONNECTION_H

/* BeginDocumentation
  Name: stdp_triplet_synapse - Synapse type with spike-timing dependent
  plasticity accounting for spike triplets as described in [1].

  Description:
    stdp_triplet_synapse is a connection with spike time dependent
    plasticity accounting for spike triplet effects (as defined in [1]).

  STDP examples:
    pair-based   A2_corr_triplet = alpha_triplet = 0.0
    triplet      A2_corr_triplet = alpha_triplet = 1.0

  Parameters:
    tau_plus           double: time constant of short presynaptic trace (tau_plus of [1])
    tau_slow_triplet   double: time constant of long presynaptic trace (tau_x of [1])
    Aplus              double: weight of pair potentiation rule (A_plus_2 of [1])
    A2_corr_triplet      double: weight of triplet potentiation rule (A_plus_3 of [1])
    Aminus             double: weight of pair depression rule (A_minus_2 of [1])
    alpha_triplet     double: weight of triplet depression rule (A_minus_3 of [1])

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
      tau_slow_triplet, respectively.
    - Postsynaptic traces o_1 and o_2 of [1] are acquired from the post-synaptic
      neuron states Kminus_ and triplet_Kminus_ which decay on time-constants
      tau_minus and tau_minus_triplet, respectively. These two time-constants can
      can be set as properties of the postsynaptic neuron.
    - This version implements the 'all-to-all' spike interaction of [1]. The 'nearest-spike'
      interaction of [1] can currently not be implemented without changing the postsynaptic
      archiving-node (clip the traces to a maximum of 1).

  FirstVersion: Nov 2007
  Author: Moritz Helias, Abigail Morrison, Eilif Muller, Alexander Seeholzer, Teo Stocco
  SeeAlso: stdp_triplet_synapse_hpc, synapsedict, stdp_synapse, static_synapse
*/

#include <cmath>
#include "connection.h"
#include <iostream>
#include <cstdio>

namespace nest
{
// connections are templates of target identifier type
// (used for pointer / target index addressing)
// derived from generic connection template
template < typename targetidentifierT >
class STDPSplConnection : public Connection< targetidentifierT >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPSplConnection();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPSplConnection( const STDPSplConnection& );

  /**
   * Default Destructor.
   */
  ~STDPSplConnection()
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
    //weight_ = w;
  }

private:

  inline double_t
  facilitate_( double_t w, double_t kplus, double_t ky )
  {
    return w + kplus * ( A2_corr_ + A4_post_ * ky );
  }

  inline double_t
  depress_( double_t w, double_t kminus, double_t lambda_ )
  {
    double new_w = w - kminus * ( alpha_ + A4_corr_ * lambda_ );
    return new_w > 0.0 ? new_w : 0.0;
  }

  void
  propagate_()//double_t &r_post_before, double_t &R_post_before)
  {  
    // propagate all variables
    for ( long_t i=0; i<n_conns_; i++ )
    {
      // EQ 1 only for nonzero synapses
      // if ( w_jk_[i] > cutoff_ )
      // {
        w_jk_[i] *= exp(-dt_*alpha_);
        w_jk_[i] += A2_corr_ * c_jk_[i] + A4_corr_ * pow(c_jk_[i],2) + A4_post_ * pow(r_jk_[i],4);
      // }
      
      // EQ 2
      c_jk_[i] *= exp(-dt_/tau_);
      c_jk_[i] += dt_ * (r_jk_[i] * (r_post_));
      // EQ 4
      r_jk_[i] *= exp(-dt_/tau_);
    }
    r_post_ *= exp(-dt_/tau_);
    R_post_ *= exp(-dt_/tau_);
    // std::cout<< "w_jk" << w_jk_[0] << "\n";
    // std::cout<< "c_jk" << c_jk_[0] << "\n";
    // std::cout<< "r_jk" << r_jk_[0] << "\n";
    // std::cout<< "r_post_before" << r_post_before << "\n";
    // std::cout<< "R_post_before" << R_post_before << "\n";
    // std::cout<< dt_ << tau_ << "\n";   
  }

  // data members of each connection
   
  long_t n_conns_;
  // weights of this connection
  std::vector< double_t >  w_jk_;
  // traces
  std::vector< double_t >  c_jk_;
  std::vector< double_t >  r_jk_;
  double_t R_post_;
  double_t r_post_;

  double_t tau_slow_;
  double_t tau_;
  double_t A2_corr_;
  double_t A4_corr_;
  double_t A4_post_;
  double_t alpha_;
  double_t lambda_;
  double_t dt_;

  double_t cutoff_;  
  double_t w0_;  

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
STDPSplConnection< targetidentifierT >::send( Event& e,
  thread t,
  double_t t_lastspike,
  const CommonSynapseProperties& )
{

  double_t t_spike = e.get_stamp().get_ms();
  Node* target = get_target( t );

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike, t_spike, &start, &finish );

  double_t t_last_postspike = t_lastspike;

  Network* net = Node::network();

  while ( start != finish )
  {
    // post-synaptic spike is delayed by dendritic_delay so that
    // it is effectively late by that much at the synapse.
    double_t delta = start->t_ - t_last_postspike;

    std::cout << "time0: " << t_last_postspike << "\n";
    std::cout << "time1: " << start->t_ << "\n";
    std::cout << "delta: " << delta << "\n";

    if ( delta == 0 )
    {
      t_last_postspike = start->t_;
      ++start;
      continue;
    }

    // get values of K after the last postspike
    // target->get_K_values( t_last_postspike, r_post_before, R_post_before );
    // r_post_before = (r_post_before)/tau_;
    // R_post_before = (R_post_before)/tau_slow_;
      
    std::cout << "r_post from neuron: " << r_post_ << "\n";
    std::cout << "R_post from neuron: " << R_post_ << "\n";
    std::cout << "w_jk_: " << w_jk_[0] << "\n---->\n";

    //double_t ky = start->triplet_Kminus_ - 1.0;
    // update iteratively all variables in the time start -> 
    for ( long_t k=0; k<floor(delta); k++ )
    {
      propagate_();//r_post_before, R_post_before);
    }

    t_last_postspike = start->t_;
    ++start;

    std::cout << "r_post after decay: " << r_post_ << "\n";
    std::cout << "R_post after decay: " << R_post_ << "\n";
    std::cout << "w_jk_ after decay: " << w_jk_[0] << "\n\n";

    // subtract 1.0 yields the triplet_Kminus value just prior to
    // the post synaptic spike, implementing the t-epsilon in
    // Pfister et al, 2006
    //double_t ky = start->triplet_Kminus_ - 1.0;
    //std::cout << "ky: " << ky << "\n";

    
    // update postsynaptic traces
    r_post_ += 1./tau_;
    R_post_ += 1./tau_slow_;

    //weight_ = facilitate_( weight_, Kplus_ * std::exp( minus_dt / tau_slow_ ), ky );
    //    double_t minus_dt = t_lastspike - ( start->t_ + dendritic_delay );
  }

  double_t remaing_delta_ = t_spike - t_last_postspike;
  for ( long_t k=0; k<floor(remaing_delta_); k++ )
  {
    propagate_();//r_post_before, R_post_before);
  }
  
  // spike failure at rate 20%, i.e. presynaptic traces only get updated by this spike
  // in 80% of the transmitted spikes.
  const int vp = get_target( t )->get_vp();
  double_t rr;
  for ( long_t i=0; i<n_conns_; i++ )
  {
    rr = net->get_rng( vp )->drand();
    if (rr>.2)
    {
      r_jk_[i] += 1./tau_;
    }
  }

  e.set_receiver( *target );
  e.set_weight( 1. );
  e.set_delay( get_delay_steps() );
  e.set_rport( get_rport() );
  e();
}

// Defaults come from reference [1] data fitting and table 3.
template < typename targetidentifierT >
STDPSplConnection< targetidentifierT >::STDPSplConnection()
  : ConnectionBase()
  , n_conns_( 10 )
  , tau_slow_( 2000.0 )
  , tau_( 20.0 )
  , A2_corr_( 1.0e-6 )
  , A4_corr_( 0.02453e-6 )
  , A4_post_( 0.0163e-6 )
  , alpha_( 1.27142e-6 )
  , lambda_( 0.028/(24. * 60. * 1e3) )
  , dt_( 1.0 )
  , cutoff_( 0. )
  , w0_(0.01)
{
  w_jk_.resize(n_conns_,0.);
  r_jk_.resize(n_conns_,0.);
  c_jk_.resize(n_conns_,0.);
  r_post_ = 0.;
  R_post_ = 0.;
}

template < typename targetidentifierT >
STDPSplConnection< targetidentifierT >::STDPSplConnection(
  const STDPSplConnection< targetidentifierT >& rhs )
  : ConnectionBase( rhs )
  , n_conns_( rhs.n_conns_ )
  , tau_slow_( rhs.tau_slow_ )
  , tau_( rhs.tau_ )
  , A2_corr_( rhs.A2_corr_ )
  , A4_corr_( rhs.A4_corr_ )
  , A4_post_( rhs.A4_post_ )
  , alpha_( rhs.alpha_ )
  , lambda_( rhs.lambda_ )
  , dt_( rhs.dt_ )
  , cutoff_( rhs.cutoff_ )
  , w0_( rhs.w0_ )
{
}

template < typename targetidentifierT >
void
STDPSplConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< long_t >( d, "n_pot_conns", n_conns_ );
  def< double_t >( d, "tau_slow", tau_slow_ );
  def< double_t >( d, "tau", tau_ );
  def< double_t >( d, "A2_corr", A2_corr_ );
  def< double_t >( d, "A4_post", A4_post_ );
  def< double_t >( d, "A4_corr", A4_corr_ );
  def< double_t >( d, "alpha", alpha_ );
  def< double_t >( d, "lambda", lambda_ );
  def< double_t >( d, "dt", dt_ );
  def< double_t >( d, "cutoff", cutoff_ );
  def< double_t >( d, "w0", w0_ );

  def< double_t >( d, "n_conns1", w_jk_.size() );  
  def< double_t >( d, "n_conns2", c_jk_.size() );  
  def< double_t >( d, "n_conns3", r_jk_.size() );  
}

template < typename targetidentifierT >
void
STDPSplConnection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< long_t >( d, "n_pot_conns", n_conns_ );
  updateValue< double_t >( d, "tau_slow", tau_slow_ );
  updateValue< double_t >( d, "tau", tau_ );
  updateValue< double_t >( d, "A2_corr", A2_corr_ );
  updateValue< double_t >( d, "A4_corr", A4_corr_ );
  updateValue< double_t >( d, "A4_post", A4_post_ );
  updateValue< double_t >( d, "alpha", alpha_ );
  updateValue< double_t >( d, "lambda", lambda_ );
  updateValue< double_t >( d, "dt", dt_ );
  updateValue< double_t >( d, "cutoff", cutoff_ );
  updateValue< double_t >( d, "w0", w0_ );

  if ( not( tau_slow_ > tau_ ) )
  {
    throw BadProperty(
      "Parameter tau_slow_triplet (time-constant of long trace) must be larger than tau_plus "
      "(time-constant of short trace)." );
  }

  if ( not( lambda_ >= 0 ) )
  {
    throw BadProperty( "lambda must be positive." );
  }

  if ( not( n_conns_ > 1) )
  {
    throw BadProperty( "Number of potential connections must be positive" );
  }

  w_jk_.resize(n_conns_,0.);
  r_jk_.resize(n_conns_,0.);
  c_jk_.resize(n_conns_,0.);
  r_post_ = 0.;
  R_post_ = 0.;
}

} // of namespace nest

#endif // of #ifndef STDP_SPL_CONNECTION_H
