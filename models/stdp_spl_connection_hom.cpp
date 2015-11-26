/*
 *  stdp_spl_connection_hom.cpp
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

#include "network.h"
#include "dictdatum.h"
#include "connector_model.h"
#include "common_synapse_properties.h"
#include "stdp_spl_connection_hom.h"
#include "event.h"
#include "numerics.h"

namespace nest
{
//
// Implementation of class STDPSplHomCommonProperties.
//

STDPSplHomCommonProperties::STDPSplHomCommonProperties()
  : CommonSynapseProperties()
  , tau_slow_( 2000.0 )
  , tau_( 20.0 )
  , A2_corr_( 1.0e-6 )
  , A4_corr_( 0.02453e-6 )
  , A4_post_( 0.0163e-6 )
  , alpha_( 1.27142e-6 )
  , lambda_( 0.028 / ( 24. * 60. * 60. ) )
  , w0_( 0.01 )
  , p_fail_( 0.2 )
{
}

void
STDPSplHomCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double_t >( d, "tau_slow", tau_slow_ );
  def< double_t >( d, "tau", tau_ );
  def< double_t >( d, "A2_corr", A2_corr_ );
  def< double_t >( d, "A4_post", A4_post_ );
  def< double_t >( d, "A4_corr", A4_corr_ );
  def< double_t >( d, "alpha", alpha_ );
  def< double_t >( d, "lambda", lambda_ );
  def< double_t >( d, "w0", w0_ );
  def< double_t >( d, "p_fail", p_fail_ );
}

void
STDPSplHomCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  updateValue< double_t >( d, "tau_slow", tau_slow_ );
  updateValue< double_t >( d, "tau", tau_ );
  updateValue< double_t >( d, "A2_corr", A2_corr_ );
  updateValue< double_t >( d, "A4_corr", A4_corr_ );
  updateValue< double_t >( d, "A4_post", A4_post_ );
  updateValue< double_t >( d, "alpha", alpha_ );
  updateValue< double_t >( d, "lambda", lambda_ );
  updateValue< double_t >( d, "w0", w0_ );
  updateValue< double_t >( d, "p_fail", p_fail_ );

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

//  dt_ = Time::get_resolution().get_ms() * 0.001;
//  e_dt_alpha_ = std::exp( -dt_ * alpha_ );
//  e_dt_alpha_1m_ = -numerics::expm1( -dt_ * alpha_ );
  
//    if ExactIntegration:
//        dec_f = np.exp(-sim_dt/tau)
//        inp_f = 1.-np.exp(-sim_dt/tau)
//        dec_f_slow = np.exp(-sim_dt/tau_slow)
//        inp_f_slow = 1.-np.exp(-sim_dt/tau_slow)
  
//  e_dt_tau_ = std::exp( -dt_ / tau_ );
//  e_dt_tau_slow_ = std::exp( -dt_ / tau_slow_ );
//  e_dt_tau_1m_ = -numerics::expm1( -dt_ / tau_ );
//  e_dt_tau_slow_1m_ = -numerics::expm1( -dt_ / tau_slow_ );
  
//    else:  # Forward Euler integration
//        dec_f = 1.-sim_dt/tau
//        inp_f = sim_dt/tau    
//        dec_f_slow = 1.-sim_dt/tau_slow
//        inp_f_slow = sim_dt/tau_slow

//  e_dt_tau_ = 1. - dt_ / tau_ ;
//  e_dt_tau_slow_ = 1. - dt_ / tau_slow_ ;
//  e_dt_tau_1m_ = dt_ / tau_ ;
//  e_dt_tau_slow_1m_ = dt_ / tau_slow_ ;
  
  pow_term_1_ = std::pow(tau_,2);
  pow_term_2_ = std::pow(tau_ - 2*tau_slow_,2);
  pow_term_3_ = std::pow(tau_,2);
  pow_term_4_ = std::pow(tau_,3);
  pow_term_5_ = std::pow(tau_ - 2*tau_slow_,2);
  pow_term_6_ = std::pow(tau_slow_,2);
  
  
  
// std::cout << "e_dt_alpha_: " << e_dt_alpha_ << "\n";
// std::cout << "e_dt_tau_: " << e_dt_tau_ << "\n";
// std::cout << "e_dt_tau_slow_: " << e_dt_tau_slow_ << "\n";
// std::cout << "e_dt_alpha_1m_: " << e_dt_alpha_1m_ << "\n";  
// std::cout << "e_dt_tau_1m_: " << e_dt_tau_1m_ << "\n";
// std::cout << "e_dt_tau_slow_1m_: " << e_dt_tau_slow_1m_ << "\n";
}

} // of namespace nest
