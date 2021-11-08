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

// C++ includes:
#include <cmath>

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_transformer_node.h"
#include "rate_transformer_node_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, rate

Short description
+++++++++++++++++

rate model with sigmoidal gain function

Description
+++++++++++

sigmoid_rate_gg_1998 is an implementation of a nonlinear rate model with
input function as in [1]_ :math:`input(h) = ( g * h )^4 / ( .1^4 + ( g * h )^4 )`.

It either models a rate neuron with input noise (see rate_neuron_ipn)
or a rate transformer (see rate_transformer_node).
Input transformation can either be applied to individual inputs
or to the sum of all inputs.

The model supports connections to other rate models with either zero or
non-zero delay, and uses the secondary_event concept introduced with
the gap-junction framework.

Nonlinear rate neurons can be created by typing
nest.Create('sigmoid_rate_gg_1998_ipn'). Nonlinear rate transformers can be
created by typing nest.Create('rate_transformer_sigmoid_rate_gg_1998').


Parameters
++++++++++

The following parameters can be set in the status dictionary. Note
that some of the parameters only apply to rate neurons and not to rate
transformers.

==================  ======= ==============================================
 rate               real    Rate (unitless)
 tau                ms      Time constant of rate dynamics
 mu                 real    Mean input
 sigma              real    Noise parameter
 g                  real    Gain parameter
 rectify_rate       real    Rectifying rate
 linear_summation   boolean Specifies type of non-linearity (see above)
 rectify_output     boolean Switch to restrict rate to values >= rectify_rate
==================  ======= ==============================================


Note:

The boolean parameter linear_summation determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (true), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (false). Default is true.

References
++++++++++

.. [1] Gancarz G, Grossberg S (1998). A neural model of the saccade generator
       in the reticular formation. Neural Networks, 11(7):1159–1174.
       DOI: https://doi.org/10.1016/S0893-6080(98)00096-3
.. [2] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI: https://doi.org/10.3389/fninf.2017.00034
.. [3] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal network simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi/org/10.3389/fninf.2015.00022


Sends
+++++

InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Receives
++++++++

InstantaneousRateConnectionEvent, DelayedRateConnectionEvent,
DataLoggingRequest

See also
++++++++

rate_connection_instantaneous, rate_connection_delayed

EndUserDocs */

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

  void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
  void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

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
nonlinearities_sigmoid_rate_gg_1998::mult_coupling_ex( double )
{
  return 1.;
}

inline double
nonlinearities_sigmoid_rate_gg_1998::mult_coupling_in( double )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_sigmoid_rate_gg_1998 > sigmoid_rate_gg_1998_ipn;
typedef rate_transformer_node< nest::nonlinearities_sigmoid_rate_gg_1998 > rate_transformer_sigmoid_gg_1998;

template <>
void RecordablesMap< sigmoid_rate_gg_1998_ipn >::create();
template <>
void RecordablesMap< rate_transformer_sigmoid_gg_1998 >::create();

} // namespace nest


#endif /* #ifndef SIGMOID_RATE_GG_1998_H */
