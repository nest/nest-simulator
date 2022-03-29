/*
 *  propagator_stability.cpp
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

#include "propagator_stability.h"


Propagator::Propagator()
  : tau_syn_( 0.0 )
  , tau_m_( 0.0 )
  , c_m_( 0.0 )
  , alpha_( 0.0 )
  , beta_( 0.0 )
  , gamma_( 0.0 )
{
}

Propagator::Propagator( double tau_syn, double tau_m, double c_m )
  : tau_syn_( tau_syn )
  , tau_m_( tau_m )
  , c_m_( c_m )
  , alpha_( 1 / ( c_m * tau_m * tau_m ) * ( tau_syn - tau_m ) )
  , beta_( tau_syn * tau_m / ( tau_m - tau_syn ) )
  , gamma_( beta_ / c_m )
{
}

PropagatorExp::PropagatorExp()
  : Propagator( 0., 0., 0. )
{
}

PropagatorExp::PropagatorExp( double tau_syn, double tau_m, double c_m )
  : Propagator( tau_syn, tau_m, c_m )
{
}

double
PropagatorExp::evaluate( double h ) const
{
  double P32;
  std::tie( P32, std::ignore, std::ignore, std::ignore ) = evaluate_P32_( h );

  return P32;
}

PropagatorAlpha::PropagatorAlpha()
  : Propagator( 0., 0., 0. )
{
}

PropagatorAlpha::PropagatorAlpha( double tau_syn, double tau_m, double c_m )
  : Propagator( tau_syn, tau_m, c_m )
{
}

std::tuple< double, double >
PropagatorAlpha::evaluate( double h ) const
{
  double P32;
  double exp_h_tau_syn;
  double expm1_h_tau;
  double exp_h_tau;
  std::tie( P32, exp_h_tau_syn, expm1_h_tau, exp_h_tau ) = evaluate_P32_( h );

  double P31 = gamma_ * exp_h_tau_syn * ( beta_ * expm1_h_tau - h );

  if ( std::abs( tau_m_ - tau_syn_ ) < 0.1 )
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
