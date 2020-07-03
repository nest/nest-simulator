/*
 *  threshold_lin_rate.h
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

#ifndef THRESHOLD_LIN_RATE_H
#define THRESHOLD_LIN_RATE_H

// C++ includes:
#include <algorithm>

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_neuron_opn.h"
#include "rate_neuron_opn_impl.h"
#include "rate_transformer_node.h"
#include "rate_transformer_node_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, rate

Short description
+++++++++++++++++

Rate model with threshold-linear gain function

Description
+++++++++++

threshold_lin_rate is an implementation of a nonlinear rate model with
input function :math:`input(h) = min( max( g * ( h - \theta ), 0 ),
\alpha )`.  It either models a rate neuron with input noise (see
rate_neuron_ipn), a rate neuron with output noise (see
rate_neuron_opn) or a rate transformer (see
rate_transformer_node). Input transformation can either be applied to
individual inputs or to the sum of all inputs.

The model supports connections to other rate models with either zero
or non-zero delay, and uses the secondary_event concept introduced
with the gap-junction framework.

The boolean parameter linear_summation determines whether the input
from different presynaptic neurons is first summed linearly and then
transformed by a nonlinearity (true), or if the input from individual
presynaptic neurons is first nonlinearly transformed and then summed
up (false). Default is true.

Nonlinear rate neuron instances can be obtained by creating models of
type ``threshold_lin_rate_ipn`` for input noise or of type
``threshold_lin_rate_opn`` output noise. Nonlinear rate transformers
can be obtained by creating models of type
``rate_transformer_threshold_lin``.

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
 alpha              real    Second Threshold
 theta              real    Threshold
 rectify_rate       real    Rectfying rate
 linear_summation   boolean Specifies type of non-linearity (see above)
 rectify_output     boolean Switch to restrict rate to values >= rectify_rate
==================  ======= ==============================================

References
++++++++++

.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI: https://doi.org/10.3389/fninf.2017.00034
.. [2] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal network simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

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

class nonlinearities_threshold_lin_rate
{
private:
  /** gain factor of gain function */
  double g_;

  /** threshold of gain function */
  double theta_;

  /** second threshold of gain function */
  double alpha_;

public:
  /** sets default parameters */
  nonlinearities_threshold_lin_rate()
    : g_( 1.0 )
    , theta_( 0.0 )
    , alpha_( std::numeric_limits< double >::infinity() )
  {
  }

  void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
  void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

  double input( double h );               // non-linearity on input
  double mult_coupling_ex( double rate ); // factor of multiplicative coupling
  double mult_coupling_in( double rate ); // factor of multiplicative coupling
};

inline double
nonlinearities_threshold_lin_rate::input( double h )
{
  return std::min( std::max( g_ * ( h - theta_ ), 0. ), alpha_ );
}

inline double
nonlinearities_threshold_lin_rate::mult_coupling_ex( double rate )
{
  return 1.;
}

inline double
nonlinearities_threshold_lin_rate::mult_coupling_in( double rate )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_threshold_lin_rate > threshold_lin_rate_ipn;
typedef rate_neuron_opn< nest::nonlinearities_threshold_lin_rate > threshold_lin_rate_opn;
typedef rate_transformer_node< nest::nonlinearities_threshold_lin_rate > rate_transformer_threshold_lin;

template <>
void RecordablesMap< threshold_lin_rate_ipn >::create();
template <>
void RecordablesMap< threshold_lin_rate_opn >::create();
template <>
void RecordablesMap< rate_transformer_threshold_lin >::create();

} // namespace nest


#endif /* #ifndef THRESHOLD_LIN_RATE_H */
