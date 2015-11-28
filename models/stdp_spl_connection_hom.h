/*
 *  stdp_spl_connection_hom.h
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

#ifndef STDP_SPL_CONNECTION_HOM_H
#define STDP_SPL_CONNECTION_HOM_H

/* BeginDocumentation
  Name: stdp_spl_synapse

  All time units are seconds!

  FirstVersion: Nov 2015
  Author: Alexander Seeholzer, Moritz Deger
  SeeAlso: stdp_spl_synapse_hpc, stdp_synapse, static_synapse
*/

#include <cmath>
#include <math.h>
#include "connection.h"
#include <iostream>
#include <cstdio>
#include "exp_randomdev.h"

namespace nest
{

/**
 * Class containing the common properties for all synapses of type STDPConnectionHom.
 */
class STDPSplHomCommonProperties : public CommonSynapseProperties
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPSplHomCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  // data members common to all connections
  double_t tau_slow_;
  double_t tau_;
  double_t A2_corr_;
  double_t A4_corr_;
  double_t A4_post_;
  double_t alpha_;
  double_t lambda_;
  double_t dt_;
  double_t w0_;
  double_t p_fail_;
  double_t t_cache_;
  bool safe_mode_;

  // precomputed values
  long_t exp_cache_len_;
  std::vector<double_t> exp_2_;
  std::vector<double_t> exp_7_;
  std::vector<double_t> exp_8_;
  double_t pow_term_1_;
  double_t pow_term_2_;
  double_t pow_term_3_;
  double_t pow_term_4_;
  double_t pow_term_5_;
  double_t pow_term_6_;

};

// connections are templates of target identifier type
// (used for pointer / target index addressing)
// derived from generic connection template
template < typename targetidentifierT >
class STDPSplConnectionHom : public Connection< targetidentifierT >
{

public:
  typedef STDPSplHomCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPSplConnectionHom();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPSplConnectionHom( const STDPSplConnectionHom& );

  /**
   * Default Destructor.
   */
  ~STDPSplConnectionHom()
  {
    // delete allocated vector elements.
    // Do we need this?
    // No, not for private variables.
    // delete c_jk_;
    // delete r_jk_;
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
  void send( Event& e, thread t, double_t t_lastspike, const STDPSplHomCommonProperties& cp );

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
    for ( long_t i = 0; i < n_conns_; i++ )
    {
      w_jk_[ i ] = w;
    }
  }

private:
  
  std::vector<double_t> get_exps_( const STDPSplHomCommonProperties& cp, 
                                   const long_t delta_i )
  {
      
      double_t exp_term_2_;
      double_t exp_term_8_;
      double_t exp_term_7_;
      
      if ( delta_i < cp.exp_cache_len_ )
      {
          // we read the precomputed values from cp
          exp_term_2_ = cp.exp_2_[delta_i];
          exp_term_8_ = cp.exp_8_[delta_i];
          exp_term_7_ = cp.exp_7_[delta_i];
      }
      else
      {
          // we compute the exponential terms
          double_t t_i_ = Time( Time::step(delta_i) ).get_ms() / 1000.;
          exp_term_2_ = std::exp( -t_i_ / cp.tau_slow_ );  
          exp_term_8_ = std::exp( -t_i_ / cp.tau_ );
          exp_term_7_ = std::exp( -t_i_* cp.alpha_ );
      }
      
      // the remaining terms are derived from the three basic ones
      // std::exp( -t_i_*( 2/cp.tau_) ); 
      double_t exp_term_6_ = exp_term_8_ * exp_term_8_;
      // std::exp( -t_i_*( 1/cp.tau_slow_ + 2/cp.tau_) );
      double_t exp_term_1_ = exp_term_2_ * exp_term_6_;
      // std::exp( -t_i_*( 2/cp.tau_slow_) ); 
      double_t exp_term_3_ = exp_term_2_ * exp_term_2_;
      // std::exp( -t_i_*( 4/cp.tau_slow_) ); 
      double_t exp_term_4_ = exp_term_2_*exp_term_2_*exp_term_2_*exp_term_2_;
      // std::exp( -t_i_*( 4/cp.tau_ ));
      double_t exp_term_5_ = exp_term_6_ * exp_term_6_;

      // insert the terms into the vector to be returned
      // this vector is now ordered by exponent magnitude:
      // exp_term_7_, exp_term_2_, exp_term_3_, exp_term_4_, exp_term_6_, 
      // exp_term_1_, exp_term_5_
      // in short: 7, 2, 3, 4, 6, 1, 5
      std::vector<double_t> ret_;
      ret_.push_back( exp_term_7_ );
      ret_.push_back( exp_term_2_ );
      ret_.push_back( exp_term_3_ );
      ret_.push_back( exp_term_4_ );
      ret_.push_back( exp_term_6_ );
      ret_.push_back( exp_term_1_ );
      ret_.push_back( exp_term_5_ );
      return ret_;
  }
  
  
  std::vector<double_t> compute_amps_( const STDPSplHomCommonProperties& cp, 
                                       const long_t i )
  {
      // precompute power terms without using std::pow
      double_t pow_term_1_ = -(c_jk_[ i ]*cp.tau_) + r_jk_[ i ]*r_post_*cp.tau_ 
                                + 2*c_jk_[ i ]*cp.tau_slow_;
      pow_term_1_ *= pow_term_1_;
      //std::pow(R_post_,4)
      double_t pow_term_2_ = R_post_ * R_post_ * R_post_ * R_post_;
      //std::pow(r_jk_[ i ],2)
      double_t pow_term_3_ = r_jk_[ i ] * r_jk_[ i ];
      //std::pow(r_post_,2)
      double_t pow_term_4_ = r_post_ * r_post_;
      //std::pow(c_jk_[ i ],2)  
      double_t pow_term_5_ = c_jk_[ i ] * c_jk_[ i ];
      
      // compute amplitudes of exp_terms
      double_t denom_ = ((-4 + cp.alpha_*cp.tau_)*(-2 + cp.alpha_*cp.tau_)*
            cp.pow_term_2_*
             (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
             (-1 + cp.alpha_*cp.tau_slow_)*
             (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)));
      double_t amp_1_ = (2*cp.A4_corr_*r_jk_[ i ]*r_post_*cp.pow_term_1_*
             (-4 + cp.alpha_*cp.tau_)*
             (-2 + cp.alpha_*cp.tau_)*cp.tau_slow_*(-(c_jk_[ i ]*cp.tau_) + 
             r_jk_[ i ]*r_post_*cp.tau_ + 2*c_jk_[ i ]*cp.tau_slow_)*
              (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
              (-1 + cp.alpha_*cp.tau_slow_) ) / denom_;
      double_t amp_2_ = ( cp.A2_corr_*(-4 + cp.alpha_*cp.tau_)*
             (-2 + cp.alpha_*cp.tau_)*
          (-(r_jk_[ i ]*r_post_*cp.tau_) + c_jk_[ i ]*(cp.tau_ - 
          2*cp.tau_slow_))*(cp.tau_ - 2*cp.tau_slow_)*cp.tau_slow_*
          (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
          (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)) )/ denom_;
      double_t amp_3_ = -( cp.A4_corr_*(-4 + cp.alpha_*cp.tau_)*
          (-2 + cp.alpha_*cp.tau_)*cp.tau_slow_*
          pow_term_1_*(-4 + cp.alpha_*cp.tau_slow_)*
          (-1 + cp.alpha_*cp.tau_slow_)*(-2*cp.tau_slow_ + 
          cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)) )/ denom_;
      double_t amp_4_ = -( cp.A4_post_* pow_term_2_ *(-4 + cp.alpha_*cp.tau_)*
          (-2 + cp.alpha_*cp.tau_)*cp.pow_term_2_*cp.tau_slow_*
          (-2 + cp.alpha_*cp.tau_slow_)*(-1 + cp.alpha_*cp.tau_slow_)*
          (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)) )/ denom_;
      double_t amp_5_ = -( cp.A4_corr_*pow_term_3_*pow_term_4_*
          cp.pow_term_4_*(-2 + cp.alpha_*cp.tau_)*(-4 + cp.alpha_*cp.tau_slow_)*
          (-2 + cp.alpha_*cp.tau_slow_)*(-1 + cp.alpha_*cp.tau_slow_)*
          (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)) )/ denom_;
      double_t amp_6_ = ( cp.A2_corr_*r_jk_[ i ]*r_post_*cp.pow_term_1_*
          (-4 + cp.alpha_*cp.tau_)*(cp.tau_ - 2*cp.tau_slow_)*
          (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
          (-1 + cp.alpha_*cp.tau_slow_)*
          (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)) )/ denom_;
      double_t amp_7_ = ( cp.pow_term_2_*
          (w_jk_[ i ]*(-4 + cp.alpha_*cp.tau_)*(-2 + cp.alpha_*cp.tau_)*
          (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
             (-1 + cp.alpha_*cp.tau_slow_)*(-2*cp.tau_slow_ + cp.tau_*
             (-1 + cp.alpha_*cp.tau_slow_)) + 
            cp.A2_corr_*(-4 + cp.alpha_*cp.tau_)*(-4 + cp.alpha_*cp.tau_slow_)*
            (-2 + cp.alpha_*cp.tau_slow_)*
             (r_jk_[ i ]*r_post_*cp.tau_ + c_jk_[ i ]*(2 - cp.alpha_*cp.tau_)*
             cp.tau_slow_)*(-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*
             cp.tau_slow_))\
             + (-2 + cp.alpha_*cp.tau_)*(-1 + cp.alpha_*cp.tau_slow_)*
             (cp.A4_post_*pow_term_2_*(-4 + cp.alpha_*cp.tau_)*cp.tau_slow_*
             (-2 + cp.alpha_*cp.tau_slow_)*
                (-cp.tau_ - 2*cp.tau_slow_ + cp.alpha_*cp.tau_*cp.tau_slow_) + 
               cp.A4_corr_*(-4 + cp.alpha_*cp.tau_slow_)*
                (2*pow_term_3_*pow_term_4_*cp.pow_term_1_ - 
                  c_jk_[ i ]*(c_jk_[ i ] + 2*r_jk_[ i ]*r_post_)*
                  cp.tau_*(-4 + cp.alpha_*cp.tau_)*cp.tau_slow_ + 
                  pow_term_5_*(-4 + cp.alpha_*cp.tau_)*(-2 + cp.alpha_*cp.tau_)*
                  cp.pow_term_6_))) )/ denom_;  

      // insert the amplitude terms into the vector to be returned
      // the order is sorted by the exp_terms magnitude: 7, 2, 3, 4, 6, 1, 5
      std::vector<double_t> ret_; 
      ret_.push_back( amp_7_ );
      ret_.push_back( amp_2_ );
      ret_.push_back( amp_3_ );
      ret_.push_back( amp_4_ );
      ret_.push_back( amp_6_ );
      ret_.push_back( amp_1_ );
      ret_.push_back( amp_5_ );
      return ret_;
  }
  
  
  inline double_t compose_w_sol_( const std::vector<double_t>& amps_, 
                                  const std::vector<double_t>& exps_ )
  {
      // compose weight solution
      double_t w_ = 0.;
      for (int_t k=0; k<exps_.size(); k++)
      {
        w_ += amps_[k] * exps_[k];
      }
      return w_;
  }
  
  
  bool check_crossing_possible_( const std::vector<double_t>& amps_ )
  {
    // We apply theorem 4.7 in http://www.maths.lancs.ac.uk/~jameson/zeros.pdf
    // G.J.O. Jameson (Math. Gazette 90, no. 518 (2006), 223–234)
    // Counting zeros of generalized polynomials: Descartes’ rule 
    // of signs and Laguerre’s extensions
    // Here we assume that the amplitudes (amps_) are ordered with descending 
    // decay rate (exp_terms). This is checked for in set_status.
    double_t amps_partial_sum_ = amps_[0];
    bool sign_last_ = std::signbit( amps_partial_sum_ );
    for (int_t k=1; k<amps_.size(); k++)
    {
        amps_partial_sum_ += amps_[k];
        if ( std::signbit( amps_partial_sum_ ) != sign_last_ )
        {
            return true;
        }
        sign_last_ = std::signbit( amps_partial_sum_ );
    }
    // according to the theorem, the number of zeros is not greater than
    // the number of sign changes_..
    // That means if we get here, there was no sign change, and so  there can
    // be no zero crossing in (0, infty).
    return false;
  }


  void integrate_( const STDPSplHomCommonProperties& cp, const long_t delta )
  {

   // integrate all state variables the duration t analytically, assuming 
   // no spikes arrive during the delta.
  
    double_t t_delta_ = Time( Time::step(delta) ).get_ms() / 1000.;  
    // std::cout << "t_delta_, delta: " << t_delta_ << "  " << delta <<  "\n";
  
    // precompute some exponentials
    double_t exp_term_8_; 
    double_t exp_term_9_;
    if ( delta < cp.exp_cache_len_ )
    {
        exp_term_8_ = cp.exp_8_[delta];
        exp_term_9_ = cp.exp_2_[delta];
    }
    else
    {
        exp_term_8_ =  std::exp( -t_delta_/cp.tau_ );
        exp_term_9_ =  std::exp( -t_delta_/cp.tau_slow_ );
    }
    // std::exp( t_delta_*(-2/cp.tau_ + 1/cp.tau_slow_) )
    double_t exp_term_10_ = exp_term_8_ * exp_term_8_ / exp_term_9_;
  
    // propagate all variables
    for ( long_t i = 0; i < n_conns_; i++ )
    {
      // for how long should w be integrated for this contact?
      long_t delta_i;
      if ( w_create_steps_[ i ] > delta )
      {
        // decrease creation step timer
        delta_i = 0;
        w_create_steps_[ i ] -= delta;
      }
      else if ( w_create_steps_[ i ] > 1 )
      {
        // a contact is created within the delta.
        // memorize how many steps are left to be integrated
        delta_i = delta - w_create_steps_[ i ];
        w_create_steps_[ i ] = 0;
        // set contact weight to creation value
        w_jk_[ i ] = cp.w0_;
        // increment deletion counter
        n_create_ ++;
      }
      else
      {
        delta_i = delta;
      }

      // EQ 1 only for nonzero synapses, i.e. created ones
      if (delta_i>0)
      {
          // compute amplitudes
          std::vector<double_t> amps_ = compute_amps_( cp, i );

          // compute exponentials
          std::vector<double_t> exps_ = get_exps_( cp, delta_i );

          // compose the solution
          w_jk_[ i ] = compose_w_sol_( amps_, exps_ );
    
          // delete synapse with negative or zero weights
          bool deletion_trigger = false;
          if ( w_jk_[ i ] <= 0. )
          {
              // Here we only check this at spike times. Misses zero crossing 
              // within ISIs. This may be improved.
              //std::cout << "deletion triggered in spike-time check."  << "\n";
              deletion_trigger = true;
          }
          else if (not cp.safe_mode_)
              {
              // if safe mode is off, we don't check for zero crossings within
              // the intervals
              }
          else if ( check_crossing_possible_( amps_ ) )
          {
              // if we cannot exclude zero crossings in general, 
              // we search numerically if there is a zero crossings
              // on the time grid spanned by the simulation resolution.
              std::vector<double_t> exps_d_;
              double_t w_d_;
              for (long_t d_=0; d_<delta_i; d_++)
              {
                std::vector<double_t> exps_d_ = get_exps_( cp, d_ );
                w_d_ = compose_w_sol_( amps_, exps_d_ );
                if (w_d_<=0.)
                {
                    //std::cout << "deletion triggered in step-wise check."  << "\n";
                    deletion_trigger = true;
                    break;
                }
              }
          }
          
          if ( deletion_trigger )
          {
            // generate an exponentially distributed number
            w_create_steps_[ i ] = Time( Time::ms( 
              exp_dev_( rng_ ) / cp.lambda_ *1e3 ) ).get_steps();
            
            // set synapse to equal zero
            w_jk_[ i ] = 0.;
            
            // increment deletion counter
            n_delete_ ++;
          }
      }

      // EQ 2 by analytical solution
      c_jk_[ i ] = ((-1 + exp_term_10_) * r_jk_[ i ]*r_post_*cp.tau_ 
            + c_jk_[ i ]*(cp.tau_ - 2*cp.tau_slow_))/
            (cp.tau_ - 2*cp.tau_slow_) * exp_term_9_;
      
      // EQ 4 by analytical solution
      r_jk_[ i ] *= exp_term_8_;
    }
  
   // update the postsynaptic rates
    r_post_ *= exp_term_8_;
    R_post_ *= exp_term_9_;
  }
  
  
  

  // data members of each connection
  long_t n_conns_;
  long_t n_create_;
  long_t n_delete_;
  // weights of this connection
  std::vector< double_t > w_jk_;
  // steps until creation of new weight
  std::vector< long_t > w_create_steps_;

  // traces
  std::vector< double_t > c_jk_;
  std::vector< double_t > r_jk_;
  double_t R_post_;
  double_t r_post_;
  
  // Random number generator pointer
  librandom::RngPtr rng_;
  // random deviate generator
  librandom::ExpRandomDev exp_dev_; 
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
STDPSplConnectionHom< targetidentifierT >::send( Event& e,
  thread t,
  double_t t_lastspike,
  const STDPSplHomCommonProperties& cp )
{

  double_t t_spike = e.get_stamp().get_ms();
  Node* target = get_target( t );

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike, t_spike, &start, &finish );

  Network* net = Node::network();
  rng_ = net->get_rng( target->get_vp() ); // random number generator of target thread

  double_t t_last_postspike = t_lastspike;

  while ( start != finish )
  {
    long_t delta = Time( Time::ms( start->t_ - t_last_postspike ) ).get_steps();

    // if delta == 0, several postsynaptic spikes occurred 
    // in this timestep. We have increment the traces to account
    // for these spikes too.

//     std::cout << "r_post from neuron: " << r_post_ << "\n";
//     std::cout << "R_post from neuron: " << R_post_ << "\n";
//     std::cout << "r_jk_: " << r_jk_[0] << "\n";
//     std::cout << "c_jk_: " << c_jk_[0] << "\n";
//     std::cout << "w_jk_: " << w_jk_[0] << "\n---->\n";
     
    // update iteratively all variables in the time start ->
    integrate_( cp, delta);

    t_last_postspike = start->t_;
    ++start;

//     std::cout << "r_post after decay: " << r_post_ << "\n";
//     std::cout << "R_post after decay: " << R_post_ << "\n";
//     std::cout << "r_jk_ after decay: " << r_jk_[0] << "\n";
//     std::cout << "c_jk_ after decay: " << c_jk_[0] << "\n";
//     std::cout << "w_jk_ after decay: " << w_jk_[0] << "\n\n";

    // update postsynaptic traces
    r_post_ += 1. / cp.tau_;
    R_post_ += 1. / cp.tau_slow_;
  }

  long_t remaining_delta = 
    Time( Time::ms( t_spike - t_last_postspike ) ).get_steps();
  integrate_( cp, remaining_delta );

  // spike failure at rate p_fail, i.e. presynaptic traces only get updated 
  // by this spike with probability 1-p_fail.
  double_t weight_tot = 0.;
  for ( long_t i = 0; i < n_conns_; i++ )
  {
    double_t rr = rng_->drand();

    if ( rr > cp.p_fail_ )
    {
      r_jk_[ i ] += 1. / cp.tau_;

      // count only existing synapses to total weight
      // only transmitted for successful spikes
      if ( w_jk_[ i ] > 0. )
      {
        weight_tot += w_jk_[ i ];
      }
    }
  }
  // only send the spike if it has a nonzero total weight
  if ( weight_tot > 0. ) 
    {
    e.set_receiver( *target );
    e.set_weight( weight_tot );
    e.set_delay( get_delay_steps() );
    e.set_rport( get_rport() );
    e();
    }
}

// Defaults come from reference [1] data fitting and table 3.
template < typename targetidentifierT >
STDPSplConnectionHom< targetidentifierT >::STDPSplConnectionHom()
  : ConnectionBase()
  , n_conns_( 1 )
{
  w_jk_.resize( n_conns_, 1. );
  w_create_steps_.resize( n_conns_, 0 );
  r_jk_.resize( n_conns_, 0. );
  c_jk_.resize( n_conns_, 0. );
  r_post_ = 0.;
  R_post_ = 0.;
  n_create_ = 0;
  n_delete_ = 0;
}

template < typename targetidentifierT >
STDPSplConnectionHom< targetidentifierT >::STDPSplConnectionHom(
  const STDPSplConnectionHom< targetidentifierT >& rhs )
  : ConnectionBase( rhs )
  , n_conns_( rhs.n_conns_ )
{
  w_jk_ = rhs.w_jk_;
  w_create_steps_ = rhs.w_create_steps_;
  r_jk_ = rhs.r_jk_;
  c_jk_ = rhs.c_jk_;
  r_post_ = rhs.r_post_;
  R_post_ = rhs.R_post_;
  n_create_ = rhs.n_create_;
  n_delete_ = rhs.n_delete_;
}

template < typename targetidentifierT >
void
STDPSplConnectionHom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< long_t >( d, "n_pot_conns", n_conns_ );
  def< long_t >( d, "n_create", n_create_ );
  def< long_t >( d, "n_delete", n_delete_ );
  def< std::vector< double_t > >( d, "w_jk", w_jk_ );
  def< double_t >( d, "r_post", r_post_ );
  def< double_t >( d, "R_post", R_post_ );
  def< std::vector< double_t > >( d, "c_jk", c_jk_ );
  def< std::vector< double_t > >( d, "r_jk", r_jk_ );
  def< std::vector< long_t > >( d, "w_create_steps", w_create_steps_ );
}

template < typename targetidentifierT >
void
STDPSplConnectionHom< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );

  bool n_updated = updateValue< long_t >( d, "n_pot_conns", n_conns_ );
  updateValue< long_t >( d, "n_create", n_create_ );
  updateValue< long_t >( d, "n_delete", n_delete_ );
  updateValue< long_t >( d, "n_pot_conns", n_conns_ );
  updateValue< double_t >( d, "r_post", r_post_ );
  updateValue< double_t >( d, "R_post", R_post_ );

  if ( not( n_conns_ > 0 ) )
  {
    throw BadProperty( "Number of potential connections must be positive" );
  }

  if ( n_updated == true )
  {
    w_jk_.resize( n_conns_, 1. );
    w_create_steps_.resize( n_conns_, 0 );
    r_jk_.resize( n_conns_, 0. );
    c_jk_.resize( n_conns_, 0. );
  }

  if ( not( n_create_ >= 0 ) )
  {
    throw BadProperty( "Number of creation events must be positive" );
  }

  if ( not( n_delete_ >= 0 ) )
  {
    throw BadProperty( "Number of deletion events must be positive" );
  }

  std::vector< double_t > r_jk_tmp;
  if ( updateValue< std::vector< double > >( d, "r_jk", r_jk_tmp ) )
  {
    if ( r_jk_tmp.size() != ( unsigned ) n_conns_ )
    {
      throw BadProperty( "Size of r_jk must be equal to n_pot_conns" );
    }
    r_jk_ = r_jk_tmp;
  }

  std::vector< double_t > c_jk_tmp;
  if ( updateValue< std::vector< double > >( d, "c_jk", c_jk_tmp ) )
  {
    if ( c_jk_tmp.size() != ( unsigned ) n_conns_ )
    {
      throw BadProperty( "Size of c_jk must be equal to n_pot_conns" );
    }
    c_jk_ = c_jk_tmp;
  }

  std::vector< double_t > w_jk_tmp;
  if ( updateValue< std::vector< double > >( d, "w_jk", w_jk_tmp ) )
  {
    if ( w_jk_tmp.size() != ( unsigned ) n_conns_ )
    {
      throw BadProperty( "Size of w_jk must be equal to n_pot_conns" );
    }
    w_jk_ = w_jk_tmp;
  }

  std::vector< long_t > w_create_steps_tmp;
  if ( updateValue< std::vector< long_t > >( d, "w_create_steps", 
                                             w_create_steps_tmp ) )
  {
    if ( w_create_steps_tmp.size() != ( unsigned ) n_conns_ )
    {
    throw BadProperty( "Size of w_create_steps must be equal to n_pot_conns" );
    }
    w_create_steps_ = w_create_steps_tmp;
  }
}

} // of namespace nest

#endif // of #ifndef STDP_SPL_CONNECTION_HOM_H
