/*
 *  sis_neuron.h
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

#ifndef SIS_NEURON_H
#define SIS_NEURON_H

// Includes from models:
#include "infection_neuron.h"

namespace nest
{
// clang-format off
/* BeginUserDocs: neuron, discrete state

Short description
+++++++++++++++++



SIS neuron with two discrete states: Susceptible, Infected.

Description
+++++++++++

The ``sis_neuron`` is an implementation of a neuron which has two
discrete states: susceptible (S) and infected (I) [1]_.
All ``sis_neuron``s are updated synchronously. When an update occurs,
all susceptible neurons are infected with probability equal to
:math:`\min(\beta_{SIS} h,1)`, where ``h`` is the number of infected pre-synaptic
neurons, and ``beta_sis`` is a parameter controlling the infectivity.
Susceptible neurons that are not infected remain susceptible.
Infected neurons become susceptible with probability ``mu_sis``.

The parameter ``tau_m`` controls the  length of the time step between updates,
and hence has no influence on the dynamics.
The state of the neuron is encoded in the variables ``S`` ( :math:`S=0` for
susceptible, :math:`S=1` for infected) and ``h``,
which counts the number of infected pre-synaptic neurons.

Nest also supports two variants of the SIS model: the SIR model,
where instead of transitioning to the S state, neurons transition to a recovered
(R) state, in which they remain (they can no longer transition to another state),
and the SIRS model, where neurons transition can from the R to the S state.
See `sir_neuron` and `sirs_neuron`.


Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================== ================== =============================== ==================================================================================
**Parameter**        **Default**        **Math equivalent**             **Description**
==================== ================== =============================== ==================================================================================
``tau_m``            10 ms              :math:`\tau_{\text{m}}`         inter-update-interval
``beta_sis``         0.1                :math:`\beta_{\text{SIRS}}`     infectivity per update step
``mu_sis``           0.1                :math:`\mu_{\text{SIRS}}`       prob. of recovery per update step
==================== ================== =============================== ==================================================================================



.. admonition:: Special requirements for SIS neurons

   The following requirements must be observed. NEST does not
   enforce them. Breaching the requirements can lead to meaningless
   results.

   1. SIS neurons must only be connected to other SIS neurons.

   #. No more than one connection must be created between any pair of
      SIS neurons. When using probabilistic connection rules, specify
      ``'allow_autapses': False`` to avoid accidental creation of
      multiple connections between a pair of neurons.


References
++++++++++

.. [1]  Kermack WO and McKendrick AG 1991.  Contributions to the mathematical theory of epidemics—II. the problem of endemicity. Bulletin of Mathematical Biology 53.

.. [2] Merger C, Albers J, Honerkamp C, and  Helias M. 2024. Spurious Self-Feedback of Mean-Field Predictions  Inflates Infection Curves. Physical Review E 110 (2): 024308. https://doi.org/10.1103/PhysRevE.110.024308.


Receives
++++++++

CurrentEvent

See also
++++++++

sirs_neuron, sir_neuron

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: sir_neuron

EndUserDocs */
// clang-format on
/**
 * SIS neuron with two discrete states: S, I.
 *
 * @note
 * This neuron has a special use for spike events to convey the
 * sis state of the neuron to the target. The neuron model
 * only sends a spike if a transition of its state occurs. If the
 * state makes a transition from S to I it sends a spike with multiplicity 1,
 * if a transition from I to S occurs, it sends a spike with multiplicity 2.
 * The decoding scheme relies on the feature that spikes with multiplicity
 * larger than 1 are delivered consecutively, also in a parallel setting.
 * The creation of double connections between sir neurons will
 * destroy the decoding scheme, as this effectively duplicates
 * every event. Using random connection routines it is therefore
 * advisable to set the property 'allow_multapses' to false.
 *
 * @see sir_neuron
 */

class sis_update_function
{
private:
  double beta_sis_;  //!< transition probability S->I
  double mu_sis_;    //!< transition probability I->S

public:
  sis_update_function();

  void get( Dictionary& ) const;
  void set( const Dictionary&, Node* node );

  size_t operator()( RngPtr, size_t old_state, double h ) const;
  size_t get_event_multiplicity( size_t new_state ) const;
};

inline size_t
sis_update_function::operator()( RngPtr rng, size_t old_state, double h ) const
{
  size_t new_state = 0;

  if ( old_state == 0 )  // neuron is susceptible
  {
    new_state = 0;
    if ( rng->drand() < beta_sis_ * h )
    {
      new_state = 1;  // neuron gets infected
    }
  }

  if ( old_state == 1 )  // neuron is infected
  {
    new_state = 1;
    if ( rng->drand() < mu_sis_ )
    {
      new_state = 0;  // neuron becomes susceptible
    }
  }

  return new_state;
}

inline size_t
sis_update_function::get_event_multiplicity( size_t new_state ) const
{
  return new_state == 0 ? 2 : 1;
}

typedef infection_neuron< sis_update_function > sis_neuron;
void register_sis_neuron( const std::string& name );

template <>
void RecordablesMap< sis_neuron >::create();

}  // namespace

#endif /* #ifndef SIS_NEURON_H */
