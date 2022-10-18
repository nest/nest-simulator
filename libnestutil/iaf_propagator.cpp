/*
 *  iaf_propagator.cpp
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

#include "iaf_propagator.h"


// Just dummy for buffer construction, these objects will never be used
IAFPropagator::IAFPropagator()
  : tau_syn_( numerics::nan )
  , tau_m_( numerics::nan )
  , c_m_( numerics::nan )
  , beta_( numerics::nan )
  , gamma_( numerics::nan )
  , inv_tau_syn_( numerics::nan )
  , inv_tau_m_( numerics::nan )
  , inv_c_m_( numerics::nan )
  , inv_beta_( numerics::nan )
{
}

// Precompute inverses to avoid division during evaluate()
IAFPropagator::IAFPropagator( double tau_syn, double tau_m, double c_m )
  : tau_syn_( tau_syn )
  , tau_m_( tau_m )
  , c_m_( c_m )
  , beta_( tau_syn_ * tau_m_ / ( tau_m_ - tau_syn_ ) ) // == inf if tau_m == tau_syn, thus well-defined
  , gamma_( beta_ / c_m_ )
  , inv_tau_syn_( 1 / tau_syn_ )
  , inv_tau_m_( 1 / tau_m_ )
  , inv_c_m_( 1 / c_m_ )
  , inv_beta_( ( tau_m_ - tau_syn_ ) / ( tau_syn_ * tau_m_ ) ) // explicit in case tau_m == tau_syn
{
}

std::tuple< double, double, double, double >
IAFPropagator::evaluate_P32_( double h ) const
{
  const double exp_h_tau_syn = std::exp( -h * inv_tau_syn_ );
  const double expm1_h_tau = numerics::expm1( h * inv_beta_ );

  const double P32 = gamma_ * exp_h_tau_syn * expm1_h_tau;

  // Two-step check for instability to avoid calculation of P32_singular if definitely not singular
  if ( std::abs( tau_m_ - tau_syn_ ) >= NUMERICAL_STABILITY_LIMIT_ )
  {
    return std::make_tuple( P32, exp_h_tau_syn, expm1_h_tau, numerics::nan );
  }
  else
  {
    const double exp_h_tau = std::exp( -h * inv_tau_m_ );
    const double P32_singular = h * inv_c_m_ * exp_h_tau;

    if ( tau_m_ == tau_syn_ or std::abs( P32 - P32_singular ) > P32_singular )
    {
      return std::make_tuple( P32_singular, exp_h_tau_syn, expm1_h_tau, exp_h_tau );
    }
    else
    {
      return std::make_tuple( P32, exp_h_tau_syn, expm1_h_tau, numerics::nan );
    }
  }
}

IAFPropagatorExp::IAFPropagatorExp()
{
}

IAFPropagatorExp::IAFPropagatorExp( double tau_syn, double tau_m, double c_m )
  : IAFPropagator( tau_syn, tau_m, c_m )
{
}

IAFPropagatorAlpha::IAFPropagatorAlpha()
{
}

IAFPropagatorAlpha::IAFPropagatorAlpha( double tau_syn, double tau_m, double c_m )
  : IAFPropagator( tau_syn, tau_m, c_m )
{
}

std::tuple< double, double >
IAFPropagatorAlpha::evaluate( double h ) const
{
  double P32;
  double exp_h_tau_syn;
  double expm1_h_tau;
  double exp_h_tau;

  // Compute P32 first, including singularity handling. If conditions are
  // numerically stable, this will return exp_h_tau == NaN. Since instability
  // would be introduced to P31 through the same expm1_h_tau term as in P32,
  // we can use the stability information from P32 for P31 and can directly
  // evaluate either the stable or singular case.
  std::tie( P32, exp_h_tau_syn, expm1_h_tau, exp_h_tau ) = evaluate_P32_( h );

  if ( numerics::is_nan( exp_h_tau ) )
  {
    const double P31 = gamma_ * exp_h_tau_syn * ( beta_ * expm1_h_tau - h );
    return std::make_tuple( P31, P32 );
  }
  else
  {
    const double P31_singular = 0.5 * h * h * inv_c_m_ * exp_h_tau;
    return std::make_tuple( P31_singular, P32 );
  }
}
