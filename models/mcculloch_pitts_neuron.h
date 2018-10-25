/*
 *  mcculloch_pitts_neuron.h
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

#ifndef MCCULLOCH_PITTS_NEURON_H
#define MCCULLOCH_PITTS_NEURON_H

// Includes from models:
#include "binary_neuron.h"

namespace nest
{

/** @BeginDocumentation
Name: mcculloch_pitts_neuron - Binary deterministic neuron with Heaviside
                              activation function.

Description:

The mcculloch_pitts_neuron is an implementation of a binary
neuron that is irregularly updated as Poisson time points [1]. At
each update point the total synaptic input h into the neuron is
summed up, passed through a Heaviside gain function g(h) = H(h-theta),
whose output is either 1 (if input is above) or 0 (if input is below
threshold theta).
The time constant tau_m is defined as the
mean inter-update-interval that is drawn from an exponential
distribution with this parameter. Using this neuron to reprodce
simulations with asynchronous update [1], the time constant needs
to be chosen as tau_m = dt*N, where dt is the simulation time
step and N the number of neurons in the original simulation with
asynchronous update. This ensures that a neuron is updated on
average every tau_m ms. Since in the original paper [1] neurons
are coupled with zero delay, this implementation follows this
definition. It uses the update scheme described in [3] to
maintain causality: The incoming events in time step t_i are
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
destroy the decoding scheme, as this effectively duplicates
every event. Using random connection routines it is therefore
advisable to set the property 'multapses' to false.
The neuron accepts several sources of currents, e.g. from a
noise_generator.

Parameters:

tau_m      double - Membrane time constant (mean inter-update-interval)
                   in ms.
theta      double - threshold for sigmoidal activation function mV

References:

[1] W. McCulloch und W. Pitts (1943). A logical calculus of the ideas
immanent in nervous activity. Bulletin of Mathematical Biophysics, 5:115-133.
[2] Hertz Krogh, Palmer. Introduction to the theory of neural computation.
Westview (1991).
[3] Abigail Morrison, Markus Diesmann. Maintaining Causality in Discrete Time
Neuronal Simulations.
In: Lectures in Supercomputational Neuroscience, p. 267. Peter beim Graben,
Changsong Zhou, Marco Thiel, Juergen Kurths (Eds.), Springer 2008.

Sends: SpikeEvent

Receives: SpikeEvent, PotentialRequest

FirstVersion: February 2013

Author: Moritz Helias

SeeAlso: pp_psc_delta
*/
class gainfunction_mcculloch_pitts
{
private:
  /** threshold of sigmoidal activation function */
  double theta_;

public:
  /** sets default parameters */
  gainfunction_mcculloch_pitts()
    : theta_( 0.0 ) // mV
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  bool operator()( librandom::RngPtr, double h );
};

inline bool gainfunction_mcculloch_pitts::operator()( librandom::RngPtr,
  double h )
{
  return h > theta_;
}

typedef nest::binary_neuron< nest::gainfunction_mcculloch_pitts >
  mcculloch_pitts_neuron;

template <>
void RecordablesMap< mcculloch_pitts_neuron >::create();

} // namespace nest

#endif /* #ifndef MCCULLOCH_PITTS_NEURON_H */
