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

/* BeginUserDocs: neuron, binary

Short description
+++++++++++++++++

Binary deterministic neuron with Heaviside activation function

Description
+++++++++++

The ``mcculloch_pitts_neuron`` is an implementation of a binary
neuron that is irregularly updated as Poisson time points [1]_. At
each update point the total synaptic input h into the neuron is
summed up, passed through a Heaviside gain function :math:`g(h) = H(h-\theta)`,
whose output is either 1 (if input is above) or 0 (if input is below
threshold theta).
The time constant :math:`\tau_m` is defined as the
mean inter-update-interval that is drawn from an exponential
distribution with this parameter. Using this neuron to reproduce
simulations with asynchronous update [1]_, the time constant needs
to be chosen as :math:`\tau_m = dt \times N`, where :math:`dt` is the simulation time
step and :math:`N` the number of neurons in the original simulation with
asynchronous update. This ensures that a neuron is updated on
average every :math:`\tau_m` ms. Since in the original paper [1]_ neurons
are coupled with zero delay, this implementation follows this
definition. It uses the update scheme described in [3]_ to
maintain causality: The incoming events in time step :math:`t_i` are
taken into account at the beginning of the time step to calculate
the gain function and to decide upon a transition.  In order to
obtain delayed coupling with delay :math:`d`, the user has to specify the
delay :math:`d+h` upon connection, where :math:`h` is the simulation time step.


Parameters
++++++++++

======= =======  ====================================================
 tau_m   ms      Membrane time constant (mean inter-update-interval)
 theta   mV      Threshold for sigmoidal activation function
======= =======  ====================================================


.. admonition:: Special requirements for binary neurons

   As the ``mcculloch_pitts_neuron`` is a binary neuron, the user must
   ensure that the following requirements are observed. NEST does not
   enforce them. Breaching the requirements can lead to meaningless
   results.

   1. Binary neurons must only be connected to other binary neurons.

   #. No more than one connection must be created between any pair of
      binary neurons. When using probabilistic connection rules, specify
      ``'allow_autapses': False`` to avoid accidental creation of
      multiple connections between a pair of neurons.

   #. Binary neurons can be driven by current-injecting devices, but
      *not* by spike generators.

   #. Activity of binary neurons can only be recored using a ``spin_detector``
      or ``correlospinmatrix_detector``.


References
++++++++++

.. [1] McCulloch W, Pitts W (1943). A logical calculus of the ideas
       immanent in nervous activity. Bulletin of Mathematical Biophysics,
       5:115-133. DOI: https://doi.org/10.1007/BF02478259
.. [2] Hertz J, Krogh A, Palmer R (1991). Introduction to the theory of neural
       computation. Addison-Wesley Publishing Conmpany.
.. [3] Morrison A, Diesmann M (2007). Maintaining causality in discrete time
       neuronal simulations. In: Lectures in Supercomputational Neuroscience,
       p. 267. Peter beim Graben, Changsong Zhou, Marco Thiel, Juergen Kurths
       (Eds.), Springer.
       DOI: https://doi.org/10.1007/978-3-540-73159-7_10


Receives
++++++++

CurrentEvent


See also
++++++++


EndUserDocs */

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

  void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
  void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

  bool operator()( RngPtr, double h );
};

inline bool gainfunction_mcculloch_pitts::operator()( RngPtr, double h )
{
  return h > theta_;
}

typedef nest::binary_neuron< nest::gainfunction_mcculloch_pitts > mcculloch_pitts_neuron;

template <>
void RecordablesMap< mcculloch_pitts_neuron >::create();

} // namespace nest

#endif /* #ifndef MCCULLOCH_PITTS_NEURON_H */
