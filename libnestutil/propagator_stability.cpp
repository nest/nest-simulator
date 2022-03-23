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

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "numerics.h"

propagator::propagator()
  : alpha_( 0 )
  , beta_ ( 0 )
  , gamma_ ( 0 )
  , tau_syn_ ( 0 )
  , tau_ ( 0 )
  , c_ ( 0 )
{
}

propagator::propagator( double tau_syn, double tau, double c )
  : tau_syn_ ( tau_syn )
  , tau_ ( tau )
  , c_ ( c )
{
  alpha_ = 1 / ( c * tau * tau ) * ( tau_syn - tau );
  beta_ = tau_syn  * tau / ( tau - tau_syn );
  gamma_ = beta_ / c;
}

void propagator::update_constants( double tau_syn, double tau, double c )
{
  alpha_ = 1 / ( c * tau * tau ) * ( tau_syn - tau );
  beta_ = tau_syn  * tau / ( tau - tau_syn );
  gamma_ = beta_ / c;

  tau_syn_ = tau_syn;
  tau_ = tau;
  c_ = c;
}

propagators
propagator::propagate( double h ) const
{
  const double exp_h_tau_syn = std::exp( -h / tau_syn_ );
  const double expm1_h_tau = numerics::expm1( -h / tau_ + h / tau_syn_ );

  propagators P;

  P.P31 = gamma_ * ( beta_ * exp_h_tau_syn * expm1_h_tau - h * exp_h_tau_syn );
  P.P32 = gamma_ * exp_h_tau_syn * expm1_h_tau;

  if ( tau_ == tau_syn_ or std::abs( tau_ - tau_syn_ ) < 0.1 )
  {
    const double exp_h_tau = std::exp( -h / tau_ );

    const double P31_linear = alpha_ * h * h * h * exp_h_tau / 3.;
    const double P31_singular = h * h / 2 / c_ * exp_h_tau;
    const double dev_P31 = std::abs( P.P31 - P31_singular );

    if ( dev_P31 > 2 * std::abs( P31_linear ) )
    {
      P.P31 = P31_singular;
    }

    const double P32_linear = alpha_ * h * h * exp_h_tau / 2.;
    const double P32_singular = h / c_ * exp_h_tau;
    const double dev_P32 = std::abs( P.P32 - P32_singular );

    if ( dev_P32 > 2 * std::abs( P32_linear ) )
    {
      P.P32 = P32_singular;
    }
  }
  return P;
}
