/*
 *  propagators.cpp
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

#include "propagators.h"
#include <cmath>
#include "numerics.h"

double
propagator_31( double tau_syn, double tau, double C, double h )
{
  const double P31 = 1 / C * ( std::exp( -h / tau_syn ) * numerics::expm1( -h / tau + h / tau_syn )
                                 / tau * ( tau / tau_syn - 1 )
                               - h * std::exp( -h / tau_syn ) ) / tau * ( -1 - -tau / tau_syn );

  const double P31_singular = h * h / 2 / C * std::exp( -h / tau );

  if ( std::abs( tau - tau_syn ) < std::pow( 1., -15 ) )
  {
    return P31_singular;
  }
  else
  {
    return P31;
  }
}

double
propagator_32( double tau_syn, double tau, double C, double h )
{
  const double P32_singular = h / C * std::exp( -h / tau );
  const double P32 = -tau / ( C * ( 1 - tau / tau_syn ) ) * std::exp( -h / tau_syn )
    * numerics::expm1( h * ( 1 / tau_syn - 1 / tau ) );

  if ( std::abs( tau - tau_syn ) < std::pow( 1., -15 ) )
  {
    return P32_singular;
  }
  else
  {
    return P32;
  }
}
