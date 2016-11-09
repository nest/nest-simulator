/*
 *  stdp_structpl_connection_hom.cpp
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

#include "stdp_structpl_connection_hom.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
//
// Implementation of class STDPStructplHomCommonProperties.
//

STDPStructplHomCommonProperties::STDPStructplHomCommonProperties()
  : CommonSynapseProperties()
  , tau_slow_( 2000.0 )
  , tau_( 20.0 )
  , A2_corr_( 1.0e-6 )
  , A4_corr_( 0.02453e-6 )
  , A4_post_( 0.0163e-6 )
  , alpha_( 1.27142e-6 )
  , lambda_( 0.028 / ( 24. * 60. * 60. ) )
  , w0_( 0.01 )
  , wmax_( -1. )
  , p_fail_( 0.2 )
  , t_cache_( 1. )
  , t_grace_period_( 0. )
  , safe_mode_( true )
  , sleep_mode_( true )
{
  // recompute dependent parameters
  compute_dependent_params();
}

void
STDPStructplHomCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double >( d, "tau_slow", tau_slow_ );
  def< double >( d, "tau", tau_ );
  def< double >( d, "A2_corr", A2_corr_ );
  def< double >( d, "A4_post", A4_post_ );
  def< double >( d, "A4_corr", A4_corr_ );
  def< double >( d, "alpha", alpha_ );
  def< double >( d, "lambda", lambda_ );
  def< double >( d, "w0", w0_ );
  def< double >( d, "wmax", wmax_ );
  def< double >( d, "p_fail", p_fail_ );
  def< double >( d, "t_cache", t_cache_ );
  def< double >( d, "t_grace_period", t_grace_period_ );
  def< bool >( d, "safe_mode", safe_mode_ );
  def< bool >( d, "sleep_mode", sleep_mode_ );
}

void
STDPStructplHomCommonProperties::compute_dependent_params()
{
  // precompute power terms that occur frequently
  pow_term_1_ = tau_ * tau_;
  pow_term_2_ = tau_ - 2 * tau_slow_;
  pow_term_2_ *= pow_term_2_;
  pow_term_4_ = tau_ * tau_ * tau_;
  pow_term_6_ = tau_slow_ * tau_slow_;

  // precompute exponential decay values up to an interval of t_cache_ seconds
  exp_cache_len_ = Time( Time::ms( t_cache_ * 1000. ) ).get_steps();
  exp_2_.resize( exp_cache_len_ );
  exp_7_.resize( exp_cache_len_ );
  exp_8_.resize( exp_cache_len_ );
  for ( long i = 0; i < exp_cache_len_; i++ )
  {
    double t_i_ = Time( Time::step( i ) ).get_ms() / 1000.;
    exp_2_[ i ] = std::exp( -t_i_ / tau_slow_ );
    exp_8_[ i ] = std::exp( -t_i_ / tau_ );
    exp_7_[ i ] = std::exp( -t_i_ * alpha_ );
  }

  steps_grace_period_ = Time( Time::ms( t_grace_period_ * 1000. ) ).get_steps();
}

void
STDPStructplHomCommonProperties::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  updateValue< double >( d, "tau_slow", tau_slow_ );
  updateValue< double >( d, "tau", tau_ );
  updateValue< double >( d, "A2_corr", A2_corr_ );
  updateValue< double >( d, "A4_corr", A4_corr_ );
  updateValue< double >( d, "A4_post", A4_post_ );
  updateValue< double >( d, "alpha", alpha_ );
  updateValue< double >( d, "lambda", lambda_ );
  updateValue< double >( d, "w0", w0_ );
  updateValue< double >( d, "wmax", wmax_ );
  updateValue< double >( d, "p_fail", p_fail_ );
  updateValue< double >( d, "t_cache", t_cache_ );
  updateValue< double >( d, "t_grace_period", t_grace_period_ );
  updateValue< bool >( d, "safe_mode", safe_mode_ );
  updateValue< bool >( d, "sleep_mode", sleep_mode_ );

  if ( not( tau_slow_ > tau_ ) )
  {
    throw BadProperty(
      "Parameter tau_slow (time-constant of slow trace) must be larger than "
      "tau "
      "(time-constant of fast trace)." );
  }

  if ( not( w0_ >= 0 ) )
  {
    throw BadProperty( "w0 (creation weight) must be positive." );
  }

  if ( not( ( wmax_ < 0 ) or ( ( wmax_ > 0 ) and ( w0_ <= wmax_ ) ) ) )
  {
    throw BadProperty( "wmax must be negative (disabled) or greater than w0." );
  }

  if ( not( lambda_ >= 0 ) )
  {
    throw BadProperty( "lambda (creation rate) must be positive." );
  }

  if ( not( t_cache_ >= 0. ) )
  {
    throw BadProperty(
      "The time interval for caching of exponentials must be positive" );
  }

  if ( not( t_grace_period_ >= 0. ) )
  {
    throw BadProperty( "The grace period must be positive" );
  }

  if ( safe_mode_ )
  {
    // check that the order of the solution's time constants is correct.
    // this is assumed for the zero-crossing theorem below (check_...)
    // 7, 2, 3, 4, 6, 1, 5
    std::vector< double > rates_;
    rates_.resize( 0 );
    rates_.push_back( -alpha_ );
    rates_.push_back( -1 / tau_slow_ );
    rates_.push_back( -2 / tau_slow_ );
    rates_.push_back( -4 / tau_slow_ );
    rates_.push_back( -2 / tau_ );
    rates_.push_back( -1 * ( 1 / tau_slow_ + 2 / tau_ ) );
    rates_.push_back( -4 / tau_ );
    for ( int i = 1; i < 7; i++ )
    {
      if ( not( rates_[ i ] < rates_[ i - 1 ] ) )
      {
        throw BadProperty(
          "Safe mode is not supported for the supplied time constants" );
      }
    }
  }

  // recompute dependent parameters
  compute_dependent_params();
}

} // of namespace nest
