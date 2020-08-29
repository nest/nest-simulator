/*
 *  ginzburg_neuron.h
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

#ifndef GINZBURG_NEURON_H
#define GINZBURG_NEURON_H

// Includes from models:
#include "binary_neuron.h"

namespace nest
{

/* BeginUserDocs: neuron, binary

Short description
+++++++++++++++++

Binary stochastic neuron with sigmoidal activation function

Description
+++++++++++

The ginzburg_neuron is an implementation of a binary neuron that
is irregularly updated as Poisson time points. At each update
point, the total synaptic input h into the neuron is summed up,
passed through a gain function g whose output is interpreted as
the probability of the neuron to be in the active (1) state.

The gain function g used here is :math:`g(h) = c_1*h + c_2 * 0.5*(1 +
\tanh(c_3*(h-\theta)))` (output clipped to [0,1]). This allows to
obtain affine-linear (:math:`c_1\neq0, c_2\neq0, c_3=0`) or sigmoidal (:math:`c_1=0,
c_2=1, c_3\neq0`) shaped gain functions. The latter choice
corresponds to the definition in [1]_, giving the name to this
neuron model.
The choice :math:`c_1=0, c_2=1, c_3=\beta/2` corresponds to the Glauber
dynamics [2]_, :math:`g(h) = 1 / (1 + \exp(-\beta (h-\theta)))`.
The time constant :math:`\tau_m` is defined as the mean
inter-update-interval that is drawn from an exponential
distribution with this parameter. Using this neuron to reproduce
simulations with asynchronous update [1]_, the time constant needs
to be chosen as :math:`\tau_m = dt*N`, where dt is the simulation time
step and N the number of neurons in the original simulation with
asynchronous update. This ensures that a neuron is updated on
average every :math:`\tau_m` ms. Since in the original paper [1]_ neurons
are coupled with zero delay, this implementation follows this
definition. It uses the update scheme described in [3]_ to
maintain causality: The incoming events in time step :math:`t_i` are
taken into account at the beginning of the time step to calculate
the gain function and to decide upon a transition.  In order to
obtain delayed coupling with delay d, the user has to specify the
delay d+h upon connection, where h is the simulation time step.

Remarks:

This neuron has a special use for spike events to convey the
binary state of the neuron to the target. The neuron model
only sends a spike if a transition of its state occurs. If the
state makes an up-transition it sends a spike with multiplicity 2,
if a down transition occurs, it sends a spike with multiplicity 1.
The decoding scheme relies on the feature that spikes with multiplicity
larger 1 are delivered consecutively, also in a parallel setting.
The creation of double connections between binary neurons will
destroy the deconding scheme, as this effectively duplicates
every event. Using random connection routines it is therefore
advisable to set the property 'allow_multapses' to false.
The neuron accepts several sources of currents, e.g. from a
noise_generator.

Parameters
++++++++++

====== ============= ===========================================================
tau_m  ms            Membrane time constant (mean inter-update-interval)
theta  mV            Threshold for sigmoidal activation function
c_1    probability/  Linear gain factor
       mV
c_2    probability   Prefactor of sigmoidal gain
c_3    1/mV          Slope factor of sigmoidal gain
====== ============= ===========================================================

References
++++++++++

.. [1] Ginzburg I, Sompolinsky H (1994). Theory of correlations in stochastic
       neural networks. PRE 50(4) p. 3171
       DOI: https://doi.org/10.1103/PhysRevE.50.3171
.. [2] Hertz J, Krogh A, Palmer R (1991). Introduction to the theory of neural
       computation. Addison-Wesley Publishing Conmpany.
.. [3] Morrison A, Diesmann M (2007). Maintaining causality in discrete time
       neuronal simulations. In: Lectures in Supercomputational Neuroscience,
       p. 267. Peter beim Graben, Changsong Zhou, Marco Thiel, Juergen Kurths
       (Eds.), Springer.
       DOI: https://doi.org/10.1007/978-3-540-73159-7_10

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, PotentialRequest

See also
++++++++

pp_psc_delta

EndUserDocs */

class gainfunction_ginzburg
{
private:
  /** threshold of sigmoidal activation function */
  double theta_;

  /** linear gain factor of gain function */
  double c1_;

  /** prefactor of sigmoidal gain function */
  double c2_;

  /** gain factor of sigmoidal gain function */
  double c3_;

public:
  /** sets default parameters */

  gainfunction_ginzburg()
    : theta_( 0.0 )
    , // mV
    c1_( 0.0 )
    , // (mV)^-1
    c2_( 1.0 )
    ,          // dimensionless
    c3_( 1.0 ) // (mV)^-1
  {
  }

  void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
  void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

  bool operator()( librandom::RngPtr rng, double h );
};

inline bool gainfunction_ginzburg::operator()( librandom::RngPtr rng, double h )
{
  return rng->drand() < c1_ * h + c2_ * 0.5 * ( 1.0 + tanh( c3_ * ( h - theta_ ) ) );
}

typedef binary_neuron< nest::gainfunction_ginzburg > ginzburg_neuron;

template <>
void RecordablesMap< ginzburg_neuron >::create();

} // namespace nest


#endif /* #ifndef GINZBURG_NEURON_H */
