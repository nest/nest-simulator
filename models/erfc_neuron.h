/*
 *  erfc_neuron.h
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

#ifndef ERFC_NEURON_H
#define ERFC_NEURON_H

// Includes from models:
#include "binary_neuron.h"

namespace nest
{
/* BeginDocumentation
   Name: erfc_neuron - Binary stochastic neuron with complementary error
   function as activation function.

   Description:
   The erfc_neuron is an implementation of a binary neuron that
   is irregularly updated at Poisson time points. At each update
   point the total synaptic input h into the neuron is summed up,
   passed through a gain function g whose output is interpreted as
   the probability of the neuron to be in the active (1) state.

   The gain function g used here is

   g(h) = 0.5 * erfc (( h - theta_ ) / ( sqrt( 2. ) * sigma)).

   This corresponds to a McCulloch-Pitts neuron receiving additional
   Gaussian noise with mean 0 and standard deviation sigma.
   The time constant tau_m is defined as the mean of the
   inter-update-interval that is drawn from an exponential
   distribution with this parameter. Using this neuron to reproduce
   simulations with asynchronous update (similar to [1,2]), the time
   constant needs to be chosen as tau_m = dt*N, where dt is the simulation time
   step and N the number of neurons in the original simulation with
   asynchronous update. This ensures that a neuron is updated on
   average every tau_m ms. Since in the original papers [1,2] neurons
   are coupled with zero delay, this implementation follows that
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
   tau_m      double - Membrane time constant (mean inter-update-interval) (ms)
   theta      double - threshold for sigmoidal activation function (mV)
   sigma      double - 1/sqrt(2pi) x inverse of maximal slope (mV)

   References:
   [1] Iris Ginzburg, Haim Sompolinsky. Theory of correlations in stochastic
   neural networks (1994). PRE 50(4) p. 3171
   [2] W. McCulloch und W. Pitts (1943). A logical calculus of the ideas
   immanent in nervous activity. Bulletin of Mathematical Biophysics, 5:115-133.
   [3] Abigail Morrison, Markus Diesmann. Maintaining Causality in Discrete Time
   Neuronal Simulations. In: Lectures in Supercomputational Neuroscience,
   p. 267. Peter beim Graben, Changsong Zhou, Marco Thiel, Juergen Kurths
   (Eds.), Springer 2008.

   Sends: SpikeEvent
   Receives: SpikeEvent, PotentialRequest
   FirstVersion: May 2016
   Authors: Jakob Jordan, Tobias Kuehn
   SeeAlso: mcculloch_pitts_neuron, ginzburg_neuron
*/

class gainfunction_erfc
{
private:
  /** threshold of activation function */
  double_t theta_;

  /** 1/sqrt(2pi) x inverse of the maximal slope of gain function */
  double_t sigma_;

public:
  /** sets default parameters */

  gainfunction_erfc()
    : theta_( 0.0 )
    , sigma_( 1.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dictionary

  bool operator()( librandom::RngPtr rng, double_t h );
};

inline bool gainfunction_erfc::operator()( librandom::RngPtr rng, double_t h )
{
  return rng->drand() < 0.5 * erfc( -( h - theta_ ) / ( sqrt( 2. ) * sigma_ ) );
}

typedef binary_neuron< nest::gainfunction_erfc > erfc_neuron;

template <>
void RecordablesMap< erfc_neuron >::create();

} // namespace nest


#endif /* #ifndef ERFC_NEURON_H */
