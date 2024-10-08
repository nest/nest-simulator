/*
 *  beta_normalization_factor.h
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

#ifndef BETA_NORMALIZATION_FACTOR_H
#define BETA_NORMALIZATION_FACTOR_H

#include <cmath>
#include <limits>

// Includes from libnestutil
#include "numerics.h"

namespace nest
{

/**
 * @brief Computes the normalization constant for the beta function
 * @param tau_rise Synaptic rise time constant, in ms
 * @param tau_decay Synaptic decay time constant, in ms
 *
 * Calculates the factor used to normalize the synaptic conductance such
 * that incoming spike causes a peak conductance of 1 nS.
 *
 * The solution to the beta function ODE obtained by the solver is
 *
 *   g(t) = c / ( a - b ) * ( e^(-b t) - e^(-a t) )
 *
 * with a = 1/tau_rise, b = 1/tau_decay, a != b. The maximum of this
 * function is at
 *
 *   t* = 1/(a-b) ln a/b
 *
 * We want to scale the function so that
 *
 *   max g == g(t*) == g_peak
 *
 * We thus need to set
 *
 *   c = g_peak * ( a - b ) / ( e^(-b t*) - e^(-a t*) )
 *
 * See Rotter & Diesmann, Biol Cybern 81:381 (1999) and Roth and van Rossum,
 * Ch 6, in De Schutter, Computational Modeling Methods for Neuroscientists,
 * MIT Press, 2010.
 *
 * The denominator, tau_difference, that appears in the expression of the
 * peak time is computed here to check that it is not zero. Another
 * denominator, peak_value, appears in the expression of the normalization
 * factor. Both tau_difference and peak_value are zero if tau_decay =
 * tau_rise. But they can also be zero if tau_decay and tau_rise are not
 * equal but very close to each other, due to the numerical precision
 * limits. In such case the beta function reduces to the alpha function,
 * and the normalization factor for the alpha function should be used.
 */
inline double
beta_normalization_factor( const double tau_rise, const double tau_decay )
{
  const double tau_difference = tau_decay - tau_rise;
  double peak_value = 0;
  if ( std::abs( tau_difference ) > std::numeric_limits< double >::epsilon() )
  {
    // peak time
    const double t_peak = tau_decay * tau_rise * std::log( tau_decay / tau_rise ) / tau_difference;
    // another denominator is computed here to check that it is != 0
    peak_value = std::exp( -t_peak / tau_decay ) - std::exp( -t_peak / tau_rise );
  }
  if ( std::abs( peak_value ) < std::numeric_limits< double >::epsilon() )
  {
    // if rise time == decay time use alpha function
    return numerics::e / tau_decay;
  }
  else
  {
    // if rise time != decay time use beta function
    return ( 1. / tau_rise - 1. / tau_decay ) / peak_value;
  }
}

} // of namespace nest


#endif /* BETA_NORMALIZATION_FACTOR_H */
