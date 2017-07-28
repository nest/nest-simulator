/*
 *  lin_rate.h
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

#ifndef LIN_RATE_H
#define LIN_RATE_H

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_neuron_opn.h"
#include "rate_neuron_opn_impl.h"


namespace nest
{
/* BeginDocumentation
Name: lin_rate - Linear rate model

Description:

 lin_rate is an implementation of a linear rate model with either
 input (lin_rate_ipn) or output noise (lin_rate_opn) and gain function
 Phi(h) = g * h.

 The model supports connections to other rate models with either zero or
 non-zero delay, and uses the secondary_event concept introduced with
 the gap-junction framework.

Parameters:

 The following parameters can be set in the status dictionary.

 rate                double - Rate (unitless)
 tau                 double - Time constant of rate dynamics in ms.
 mean                double - Mean of Gaussian white noise.
 std                 double - Standard deviation of Gaussian white noise.
 g                   double - Gain parameter

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

Author: David Dahmen, Jan Hahne, Jannis Schuecker
SeeAlso: rate_connection_instantaneous, rate_connection_delayed
*/

class gainfunction_lin_rate
{
private:
  /** gain factor of gain function */
  double g_;

public:
  /** sets default parameters */
  gainfunction_lin_rate()
    : g_( 1.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  double operator()( double h ); // non-linearity
};

inline double gainfunction_lin_rate::operator()( double h )
{
  return g_ * h;
}

typedef rate_neuron_ipn< nest::gainfunction_lin_rate > lin_rate_ipn;
typedef rate_neuron_opn< nest::gainfunction_lin_rate > lin_rate_opn;

template <>
void RecordablesMap< lin_rate_ipn >::create();
template <>
void RecordablesMap< lin_rate_opn >::create();

} // namespace nest


#endif /* #ifndef LIN_RATE_H */
