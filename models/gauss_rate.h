/*
 *  gauss_rate.h
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

#ifndef GAUSS_RATE_H
#define GAUSS_RATE_H

// Includes from c++:
#include <cmath>

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_transformer_node.h"
#include "rate_transformer_node_impl.h"


namespace nest
{

/** @BeginDocumentation
Name: gauss_rate - rate model with Gaussian gain function

Description:

gauss_rate is an implementation of a nonlinear rate model with input
function input(h) = g * exp( -( x - mu )^2 / ( 2 * sigma^2 ) ).
Input transformation can either be applied to individual inputs
or to the sum of all inputs.

The model supports connections to other rate models with either zero or
non-zero delay, and uses the secondary_event concept introduced with
the gap-junction framework.

Parameters:

The following parameters can be set in the status dictionary.

rate                double - Rate (unitless)
tau                 double - Time constant of rate dynamics in ms.
mu                  double - Mean input.
sigma               double - Noise parameter.
g                   double - Gain parameter.
mu                  double - Mean of the Gaussian gain function.
sigma               double - Standard deviation of Gaussian gain function.
linear_summation    bool   - Specifies type of non-linearity (see above).
rectify_output      bool   - Switch to restrict rate to values >= 0.

Note:

The boolean parameter linear_summation determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (true), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (false). Default is true.

References:

[1] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,
Bolten, M., Helias, M. and Diesmann, M. (2017).
Integration of Continuous-Time Dynamics in a
Spiking Neural Network Simulator.
Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034

[2] Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,
Bolten, M., Frommer, A. and Diesmann, M. (2015).
A unified framework for spiking and gap-junction interactions
in distributed neuronal network simulations.
Front. Neuroinform. 9:22. doi: 10.3389/fninf.2015.00022

Sends: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Receives: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent,
DataLoggingRequest

Author: Mario Senden, Jan Hahne, Jannis Schuecker

SeeAlso: rate_connection_instantaneous, rate_connection_delayed
*/
class nonlinearities_gauss_rate
{
private:
  /** gain factor of gain function */
  double g_;
  double mu_;
  double sigma_;

public:
  /** sets default parameters */
  nonlinearities_gauss_rate()
    : g_( 1.0 )
    , mu_( 0.0 )
    , sigma_( 0.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  double input( double h );               // non-linearity on input
  double mult_coupling_ex( double rate ); // factor of multiplicative coupling
  double mult_coupling_in( double rate ); // factor of multiplicative coupling
};

inline double
nonlinearities_gauss_rate::input( double h )
{
  return g_ * ( std::exp( -pow( h - mu_, 2. ) / ( 2. * pow( sigma_, 2. ) ) ) );
}

inline double
nonlinearities_gauss_rate::mult_coupling_ex( double rate )
{
  return 1.;
}

inline double
nonlinearities_gauss_rate::mult_coupling_in( double rate )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_gauss_rate > gauss_rate_ipn;
typedef rate_transformer_node< nest::nonlinearities_gauss_rate >
  rate_transformer_gauss;

template <>
void RecordablesMap< gauss_rate_ipn >::create();
template <>
void RecordablesMap< rate_transformer_gauss >::create();

} // namespace nest


#endif /* #ifndef GAUSS_RATE_H */
