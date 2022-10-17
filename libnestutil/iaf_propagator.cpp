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


IAFPropagator::IAFPropagator()
  : tau_syn_( 0.0 )
  , tau_m_( 0.0 )
  , c_m_( 0.0 )
  , alpha_( 0.0 )
  , beta_( 0.0 )
  , gamma_( 0.0 )
{
}

IAFPropagator::IAFPropagator( double tau_syn, double tau_m, double c_m )
  : tau_syn_( tau_syn )
  , tau_m_( tau_m )
  , c_m_( c_m )
  , alpha_( 1 / ( c_m * tau_m * tau_m ) * ( tau_syn - tau_m ) )
  , beta_( tau_syn * tau_m / ( tau_m - tau_syn ) ) // == inf if tau_m == tau_syn, thus well-defined
  , gamma_( beta_ / c_m )
{
}

inline bool
IAFPropagator::possibly_singular_() const
{
  // Boundary 0.1 ms chosen ad hoc. As the criterium is only used
  // to decide whether to perform a more careful check for singularity,
  // the precise value is not important.
  return std::abs( tau_m_ - tau_syn_ ) < 0.1;
}

std::tuple< double, double, double, double >
IAFPropagator::evaluate_P32_( double h ) const
{
  const double exp_h_tau_syn = std::exp( -h / tau_syn_ );
  const double expm1_h_tau = numerics::expm1( -h / tau_m_ + h / tau_syn_ );

  double P32 = gamma_ * exp_h_tau_syn * expm1_h_tau;

  double exp_h_tau = std::numeric_limits< double >::signaling_NaN(); // only used if condition below fulfilled
  if ( possibly_singular_() )
  {
    exp_h_tau = std::exp( -h / tau_m_ );

    const double P32_singular = h / c_m_ * exp_h_tau;
    if ( tau_m_ == tau_syn_ )
    {
      P32 = P32_singular;
    }
    else
    {
      const double P32_linear = alpha_ * h * h * exp_h_tau / 2.;
      const double dev_P32 = std::abs( P32 - P32_singular );

      if ( dev_P32 > 2 * std::abs( P32_linear ) )
      {
        P32 = P32_singular;
      }
    }
  }

  return std::make_tuple( P32, exp_h_tau_syn, expm1_h_tau, exp_h_tau );
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
  std::tie( P32, exp_h_tau_syn, expm1_h_tau, exp_h_tau ) = evaluate_P32_( h );

  double P31 = gamma_ * exp_h_tau_syn * ( beta_ * expm1_h_tau - h );

  if ( possibly_singular_() )
  {
    const double P31_singular = h * h / 2 / c_m_ * exp_h_tau;

    if ( tau_m_ == tau_syn_ )
    {
      P31 = P31_singular;
    }
    else
    {
      const double P31_linear = alpha_ * h * h * h * exp_h_tau / 3.;
      const double dev_P31 = std::abs( P31 - P31_singular );

      if ( dev_P31 > 2 * std::abs( P31_linear ) )
      {
        P31 = P31_singular;
      }
    }
  }

  return std::make_tuple( P31, P32 );
}
