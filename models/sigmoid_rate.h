/*
 *  sigmoid_rate.h
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

#ifndef SIGMOID_RATE_H
#define SIGMOID_RATE_H

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
@ingroup Neurons
@ingroup rate

Name: sigmoid_rate - rate model with sigmoidal gain function

Description:

sigmoid_rate is an implementation of a nonlinear rate model with input
function \f$ input(h) = g / ( 1. + \exp( -\beta * ( h - \theta ) ) ) \f$.
Input transformation can either be applied to individual inputs
or to the sum of all inputs.

The model supports connections to other rate models with either zero or
non-zero delay, and uses the secondary_event concept introduced with
the gap-junction framework.

Parameters:

The following parameters can be set in the status dictionary.
\verbatim embed:rst
==================  ======= ==============================================
 rate               real    Rate (unitless)
 tau                ms      Time constant of rate dynamics
 mu                 real    Mean input
 sigma              real    Noise parameter
 g                  real    Gain parameter
 beta               real    Slope parameter
 theta              real    Threshold
 linear_summation   boolean Specifies type of non-linearity (see above)
 rectify_output     boolean Switch to restrict rate to values >= 0
==================  ======= ==============================================
\endverbatim

Note:

The boolean parameter linear_summation determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (true), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (false). Default is true.

References:

\verbatim embed:rst
.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI: https://doi.org/10.3389/fninf.2017.00034
.. [2] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal network simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022
\endverbatim

Sends: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Receives: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent,
DataLoggingRequest

Author: Mario Senden, Jan Hahne, Jannis Schuecker

SeeAlso: rate_connection_instantaneous, rate_connection_delayed
*/

class nonlinearities_sigmoid_rate
{
private:
  /** gain factor of gain function */
  double g_;
  double beta_;
  double theta_;

public:
  /** sets default parameters */
  nonlinearities_sigmoid_rate()
    : g_( 1.0 )
    , beta_( 1.0 )
    , theta_( 0.0 )
  {
  }

  void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
  void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

  double input( double h );               // non-linearity on input
  double mult_coupling_ex( double rate ); // factor of multiplicative coupling
  double mult_coupling_in( double rate ); // factor of multiplicative coupling
};

inline double
nonlinearities_sigmoid_rate::input( double h )
{
  return g_ / ( 1. + std::exp( -beta_ * ( h - theta_ ) ) );
}

inline double
nonlinearities_sigmoid_rate::mult_coupling_ex( double rate )
{
  return 1.;
}

inline double
nonlinearities_sigmoid_rate::mult_coupling_in( double rate )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_sigmoid_rate > sigmoid_rate_ipn;
typedef rate_transformer_node< nest::nonlinearities_sigmoid_rate > rate_transformer_sigmoid;

template <>
void RecordablesMap< sigmoid_rate_ipn >::create();
template <>
void RecordablesMap< rate_transformer_sigmoid >::create();

} // namespace nest


#endif /* #ifndef SIGMOID_RATE_H */
