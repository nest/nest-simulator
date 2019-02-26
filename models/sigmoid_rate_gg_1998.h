/*
 *  sigmoid_rate_gg_1998.h
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

#ifndef SIGMOID_RATE_GG_1998_H
#define SIGMOID_RATE_GG_1998_H

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
Name: sigmoid_rate_gg_1998 - rate model with sigmoidal gain function
as defined in [1].

Description:

sigmoid_rate_gg_1998 is an implementation of a nonlinear rate model with
input function input(h) = ( g * h )^4 / ( .1^4 + ( g * h )^4 ).
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
linear_summation    bool   - Specifies type of non-linearity (see above).
rectify_output      bool   - Switch to restrict rate to values >= 0.

Note:

The boolean parameter linear_summation determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (true), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (false). Default is true.

References:

[1] Gancarz, G., & Grossberg, S. (1998).
     A neural model of the saccade generator in the reticular formation.
     Neural Networks, 11(7), 1159–1174. doi: 10.1016/S0893-6080(98)00096-3

[2] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,
    Bolten, M., Helias, M. and Diesmann, M. (2017).
    Integration of Continuous-Time Dynamics in a
    Spiking Neural Network Simulator.
    Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034

[3] Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,
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
class nonlinearities_sigmoid_rate_gg_1998
{
private:
  /** gain factor of gain function */
  double g_;

public:
  /** sets default parameters */
  nonlinearities_sigmoid_rate_gg_1998()
    : g_( 1.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  double input( double h );               // non-linearity on input
  double mult_coupling_ex( double rate ); // factor of multiplicative coupling
  double mult_coupling_in( double rate ); // factor of multiplicative coupling
};

inline double
nonlinearities_sigmoid_rate_gg_1998::input( double h )
{
  return pow( g_ * h, 4. ) / ( pow( .1, 4. ) + pow( g_ * h, 4. ) );
}

inline double
nonlinearities_sigmoid_rate_gg_1998::mult_coupling_ex( double rate )
{
  return 1.;
}

inline double
nonlinearities_sigmoid_rate_gg_1998::mult_coupling_in( double rate )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_sigmoid_rate_gg_1998 >
  sigmoid_rate_gg_1998_ipn;
typedef rate_transformer_node< nest::nonlinearities_sigmoid_rate_gg_1998 >
  rate_transformer_sigmoid_gg_1998;

template <>
void RecordablesMap< sigmoid_rate_gg_1998_ipn >::create();
template <>
void RecordablesMap< rate_transformer_sigmoid_gg_1998 >::create();

} // namespace nest


#endif /* #ifndef SIGMOID_RATE_GG_1998_H */
