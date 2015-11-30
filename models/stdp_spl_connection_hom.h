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
  
  void get_exps_( const STDPSplHomCommonProperties& cp, 
                                   const long_t& delta_i, 
                                   std::vector<double_t>& exps_ )
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
      exps_[0] = exp_term_7_ ;
      exps_[1] = exp_term_2_ ;
      exps_[2] = exp_term_3_ ;
      exps_[3] = exp_term_4_ ;
      exps_[4] = exp_term_6_ ;
      exps_[5] = exp_term_1_ ;
      exps_[6] = exp_term_5_ ;
  }
  
  
  void compute_amps_( const STDPSplHomCommonProperties& cp, 
                                       const long_t& i, 
                                       const double_t& r_post_i_, 
                                       const double_t& R_post_i_,
                                       std::vector<double_t>& amps_)
  {
      // precompute power terms without using std::pow
      double_t pow_term_1_ = -(c_jk_[ i ]*cp.tau_) + r_jk_[ i ]*r_post_i_*cp.tau_ 
                                + 2*c_jk_[ i ]*cp.tau_slow_;
      pow_term_1_ *= pow_term_1_;
      //std::pow(R_post_,4)
      double_t pow_term_2_ = R_post_i_ * R_post_i_ * R_post_i_ * R_post_i_;
      //std::pow(r_jk_[ i ],2)
      double_t pow_term_3_ = r_jk_[ i ] * r_jk_[ i ];
      //std::pow(r_post_,2)
      double_t pow_term_4_ = r_post_i_ * r_post_i_;
      //std::pow(c_jk_[ i ],2)  
      double_t pow_term_5_ = c_jk_[ i ] * c_jk_[ i ];
      
      // compute amplitudes of exp_terms
      double_t denom_ = ((-4 + cp.alpha_*cp.tau_)*(-2 + cp.alpha_*cp.tau_)*
            cp.pow_term_2_*
             (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
             (-1 + cp.alpha_*cp.tau_slow_)*
             (-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*cp.tau_slow_)));
      double_t amp_1_ = (2*cp.A4_corr_*r_jk_[ i ]*r_post_i_*cp.pow_term_1_*
             (-4 + cp.alpha_*cp.tau_)*
             (-2 + cp.alpha_*cp.tau_)*cp.tau_slow_*(-(c_jk_[ i ]*cp.tau_) + 
             r_jk_[ i ]*r_post_i_*cp.tau_ + 2*c_jk_[ i ]*cp.tau_slow_)*
              (-4 + cp.alpha_*cp.tau_slow_)*(-2 + cp.alpha_*cp.tau_slow_)*
              (-1 + cp.alpha_*cp.tau_slow_) ) / denom_;
      double_t amp_2_ = ( cp.A2_corr_*(-4 + cp.alpha_*cp.tau_)*
             (-2 + cp.alpha_*cp.tau_)*
          (-(r_jk_[ i ]*r_post_i_*cp.tau_) + c_jk_[ i ]*(cp.tau_ - 
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
      double_t amp_6_ = ( cp.A2_corr_*r_jk_[ i ]*r_post_i_*cp.pow_term_1_*
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
             (r_jk_[ i ]*r_post_i_*cp.tau_ + c_jk_[ i ]*(2 - cp.alpha_*cp.tau_)*
             cp.tau_slow_)*(-2*cp.tau_slow_ + cp.tau_*(-1 + cp.alpha_*
             cp.tau_slow_))\
             + (-2 + cp.alpha_*cp.tau_)*(-1 + cp.alpha_*cp.tau_slow_)*
             (cp.A4_post_*pow_term_2_*(-4 + cp.alpha_*cp.tau_)*cp.tau_slow_*
             (-2 + cp.alpha_*cp.tau_slow_)*
                (-cp.tau_ - 2*cp.tau_slow_ + cp.alpha_*cp.tau_*cp.tau_slow_) + 
               cp.A4_corr_*(-4 + cp.alpha_*cp.tau_slow_)*
                (2*pow_term_3_*pow_term_4_*cp.pow_term_1_ - 
                  c_jk_[ i ]*(c_jk_[ i ] + 2*r_jk_[ i ]*r_post_i_)*
                  cp.tau_*(-4 + cp.alpha_*cp.tau_)*cp.tau_slow_ + 
                  pow_term_5_*(-4 + cp.alpha_*cp.tau_)*(-2 + cp.alpha_*cp.tau_)*
                  cp.pow_term_6_))) )/ denom_;  

      // insert the amplitude terms into the vector to be returned
      // the order is sorted by the exp_terms magnitude: 7, 2, 3, 4, 6, 1, 5
      amps_[0] = amp_7_ ;
      amps_[1] = amp_2_ ;
      amps_[2] = amp_3_ ;
      amps_[3] = amp_4_ ;
      amps_[4] = amp_6_ ;
      amps_[5] = amp_1_ ;
      amps_[6] = amp_5_ ;
  }
  
  
  inline double_t compose_w_sol_( const std::vector<double_t>& amps_, 
                                  const std::vector<double_t>& exps_ )
  {
      // compose weight solution
      double_t w_ = 0.;
      for (int_t k=0; k<7; k++)
      {
        w_ += amps_[k] * exps_[k];
      }
      return w_;
  }
  
  
  inline bool check_crossing_possible_( const std::vector<double_t>& amps_ )
  {
    // We apply theorem 4.7 in http://www.maths.lancs.ac.uk/~jameson/zeros.pdf
    // G.J.O. Jameson (Math. Gazette 90, no. 518 (2006), 223–234)
    // Counting zeros of generalized polynomials: Descartes’ rule 
    // of signs and Laguerre’s extensions
    // Here we assume that the amplitudes (amps_) are ordered with descending 
    // decay rate (exp_terms). This is checked for in set_status.
    double_t amps_partial_sum_ = amps_[0];
    bool sign_last_ = std::signbit( amps_partial_sum_ );
    for (int_t k=1; k<7; k++)
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
  
    // propagate all variables
    for ( long_t i = 0; i < n_conns_; i++ )
    {
      // set the local r_post/R_post (for this contact) to the global 
      // initial value at beginning of contact update loop
      r_post_i_ = r_post_;
      R_post_i_ = R_post_;

      long_t delta_done = 0;
      while ( delta_done < delta )
      {
      // how many steps are left to be processed?
      long_t delta_this = delta - delta_done;

      // for how long should we integrate w_jk in this round?
      long_t delta_i;
      if ( w_create_steps_[ i ] > delta_this )
      {
        // if the contact is waiting for creation, decrease creation step timer
        w_create_steps_[ i ] -= delta_this;
        // no integration to be done
        delta_i = 0;
      }
      else if ( w_create_steps_[ i ] > 1 )
      {
        // the contact is going to be created within this delta.
        // how many steps will elapse until this happens?
        delta_this = w_create_steps_[ i ];
        // no integration to be done
        delta_i = 0;
        // set contact weight to creation value
        w_jk_[ i ] = cp.w0_;
        // clear creation step counter
        w_create_steps_[ i ] = 0;
        // increment creation counter
        n_create_ ++;
      }
      else
      {
        // the contact exists, so it can be integrated for the remaining delta
        delta_i = delta_this;
      }
      
      // weight integration, only for existing contacts
      if (delta_i>0)
      {
          // the delta_done in this case may be different from delta_this, in
          // case the contact is deleted before delta_this is elapsed. If so,
          // we reenter this while loop from the top.
          // Therefore we update r_jk and the other variables at every round.
          
          // compute amplitudes of exponential terms of w_jk solution
          compute_amps_( cp, i, r_post_i_, R_post_i_, amps_ );

          // compute exponentials terms
          get_exps_( cp, delta_i, exps_ );

          // compose the solution
          w_jk_[ i ] = compose_w_sol_( amps_, exps_ );
    
          // delete contacts with negative or zero weights
          if ( w_jk_[ i ] <= 0. )
          {
              deletion_trigger = true;
              // in safe mode we compute the exact zero crossing times in case
              // of deletions, to correct the creation step counter.
              stepeval_trigger = cp.safe_mode_;
          }
          else 
          {
              // no deletion has been triggered yet (w_jk is positive).
              // There may have been a zero crossing before, though.
              deletion_trigger = false;
              if (cp.safe_mode_)
              {
                 // if we cannot exclude zero crossings in the interval, 
                 // we search numerically if there is a zero crossings,
                 // on the time grid spanned by the simulation resolution.
                 stepeval_trigger = check_crossing_possible_( amps_ );
              }
              else
              {
                 stepeval_trigger = false;
              }
          }
          if (stepeval_trigger)
          {
              long_t d_stepeval_ = 0;
              // we search numerically for a zero crossing
              // on the time grid spanned by the simulation resolution.
              while (d_stepeval_<delta_i)
              {
                get_exps_( cp, d_stepeval_, exps_ );
                double_t w_stepeval_ = compose_w_sol_( amps_, exps_ );
                if (w_stepeval_<=0.)
                {
                    // we stop searching because we have found the first zero
                    // crossing, upon which the contact is immediately deleted.
                    //std::cout << "deletion triggered in step-wise check."  << "\n";
                    deletion_trigger = true;
                    break;
                }
                d_stepeval_++;
              }
              // because the deletion may have happened before reaching 
              // delta_this, the effective interval that was integrated 
              // may have been shorter
              delta_this = d_stepeval_;
          }
          else
          {
              // if stepeval was not triggered (not in safe mode), 
              // we assume that the deletion event, if any, happened at the end  
              // of the integration interval, so that we have integrated the 
              // whole delta_i
              delta_this = delta_i;
          }
          
          if ( deletion_trigger )
          {
            // generate an exponentially distributed number.
            w_create_steps_[ i ] = Time( Time::ms( 
              exp_dev_( rng_ ) / cp.lambda_ *1e3 ) ).get_steps();
            // set synapse to equal zero
            w_jk_[ i ] = 0.;
            // increment deletion counter
            n_delete_ ++;
          }
      }

      // now we integrate the remaining variables for delta_this steps

      // precompute some exponentials
      if ( delta_this < cp.exp_cache_len_ )
      {
          // we copy the exp terms from the cache if possible
          exp_term_8_ = cp.exp_8_[delta_this];
          exp_term_9_ = cp.exp_2_[delta_this];
      }
      else
      {
          // otherwise we compute them
          double_t t_delta_ = Time( Time::step(delta_this) ).get_ms() / 1000.;  
          exp_term_8_ =  std::exp( -t_delta_/cp.tau_ );
          exp_term_9_ =  std::exp( -t_delta_/cp.tau_slow_ );
      }
      // std::exp( t_delta_*(-2/cp.tau_ + 1/cp.tau_slow_) )
      exp_term_10_ = exp_term_8_ * exp_term_8_ / exp_term_9_;

      // c_jk update by analytical solution
      c_jk_[ i ] = ((-1 + exp_term_10_) * r_jk_[ i ]*r_post_i_*cp.tau_ 
            + c_jk_[ i ]*(cp.tau_ - 2*cp.tau_slow_))/
            (cp.tau_ - 2*cp.tau_slow_) * exp_term_9_;
      
      // r_jk update by analytical solution
      r_jk_[ i ] *= exp_term_8_;

      // r/R post update by analytical solution
      r_post_i_ *= exp_term_8_;
      R_post_i_ *= exp_term_9_;
      
      // increment the step counter of the loop over delta
      delta_done += delta_this;
      }
    }
   // When we get here all contacts have been updated for the whole delta.
   // We copy the last postsynaptic values as the new postsynaptic state.
   r_post_ = r_post_i_;
   R_post_ = R_post_i_;
  }
  
  // declarations of data members of each connection
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

  // we need local copies of these, because intermediate values are
  // required as we go through the contacts.
  double_t r_post_i_;
  double_t R_post_i_;
  double_t exp_term_8_; 
  double_t exp_term_9_;
  double_t exp_term_10_;
  bool deletion_trigger;
  bool stepeval_trigger;
  std::vector<double_t> amps_;
  std::vector<double_t> exps_;
};

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param t_lastspike Time point of last spike emitted
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
void
STDPSplConnectionHom< targetidentifierT >::send( Event& e,
  thread t,
  double_t t_lastspike,
  const STDPSplHomCommonProperties& cp )
{
  // once the synapse receives a spike event, it updates its state, from the 
  // last spike to this one.
  double_t t_spike = e.get_stamp().get_ms();

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  Node* target = get_target( t );
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike, t_spike, &start, &finish );

  // get random number generator of target thread
  Network* net = Node::network();
  rng_ = net->get_rng( target->get_vp() ); 

  // integration of synapse state starts from the last spike received
  double_t t_last_postspike = t_lastspike;

  // integration proceeds from postsynaptic spike to postsyn. spike in range.
  while ( start != finish )
  {
    long_t delta = Time( Time::ms( start->t_ - t_last_postspike ) ).get_steps();
     
    // integrate the state variables for this delta
    // here we use the analytical solution of the ODEs in between spikes
    // (homogeneous solution)
    integrate_( cp, delta );

    // increment postsynaptic traces once for each spike
    r_post_ += 1. / cp.tau_;
    R_post_ += 1. / cp.tau_slow_;

    // proceed to the next postsynaptic spike
    t_last_postspike = start->t_;
    ++start;
  }

  // it remains to integrate from the last postsynaptic spike to the time of 
  // the presynaptic spike received.
  long_t remaining_delta = 
    Time( Time::ms( t_spike - t_last_postspike ) ).get_steps();
  integrate_( cp, remaining_delta );

  // Now, after updating the synapse state, we are ready to transmit the spike.
  // Spike transmission failures occur at each contact with rate p_fail, i.e. 
  // presynaptic traces only get updated by the spike with probability 1-p_fail.
  double_t weight_tot = 0.;
  for ( long_t i = 0; i < n_conns_; i++ )
  {
    // go through all synaptic contacts and draw a random number
    double_t rr = rng_->drand();
    if ( rr > cp.p_fail_ )
    {
      // increment the presynaptic trace of contact i if transmission successful
      r_jk_[ i ] += 1. / cp.tau_;

      // count only existing synapses to total weight
      // only transmitted for successful spikes
      if ( w_jk_[ i ] > 0. )
      {
        weight_tot += w_jk_[ i ];
      }
    }
  }
  // Only send the spike if it has a nonzero total weight
  // Sending spikes causes computations in postsynaptic neurons and 
  // network communication, which is not necessary for zero-weight spikes. 
  if ( weight_tot > 0. )
    {
    e.set_receiver( *target );
    e.set_weight( weight_tot );
    e.set_delay( get_delay_steps() );
    e.set_rport( get_rport() );
    e();
    }
}


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
  amps_.resize(7);
  exps_.resize(7);
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
