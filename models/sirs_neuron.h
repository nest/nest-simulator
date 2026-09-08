/*
 *  sirs_neuron.h
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

#ifndef SIRS_NEURON_H
#define SIRS_NEURON_H

// Includes from models:
#include "infection_neuron.h"

namespace nest
{
// clang-format off
/* BeginUserDocs: neuron, discrete state


Short description
+++++++++++++++++

Neuron with three discrete states: Susceptible, Infected, Recovered.

Description
+++++++++++

The ``sirs_neuron`` is an implementation of a neuron which has three
discrete states: susceptible (S), infected (I) and recovered (R) [1]_.
All ``sirs_neuron`` (s) of a population are updated synchronously.

When an update occurs, all susceptible neurons are infected with probability equal to
:math:`\min( \beta_{\text{SIRS}} h, 1)`, where ``h`` is the number of infected
pre-synaptic  neurons, and ``beta_sirs`` is a parameter controlling the
infectivity. Susceptible neurons that are not infected remain susceptible.
Infected neurons recover with probability ``mu_sirs``. Infected neurons
that do not recover remain infected. Recovered neurons become susceptible with
probability ``eta_sirs``.

The parameter ``tau_m`` controls the
length of the time step between updates, and hence has no influence on the
dynamics.
The state of the neuron is encoded in the variables ``S`` ( :math:`S=0` for
susceptible, :math:`S=1` for infected, :math:`S=2` for recovered) and ``h``,
which counts the number of infected pre-synaptic neurons.

NEST also supports two variants of the SIRS model: the SIR model,
where neurons remain in the R state instead of transitioning from the R to the S
state, and the SIS model, where neurons transition directly from the I to the S
state.
See `sir_neuron` and `sis_neuron`.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================== ================== =============================== ==================================================================================
**Parameter**        **Default**        **Math equivalent**             **Description**
==================== ================== =============================== ==================================================================================
``tau_m``            10 ms              :math:`\tau_{\text{m}}`         inter-update-interval
``beta_sirs``         0.1                :math:`\beta_{\text{SIRS}}`     infectivity per update step
``mu_sirs``           0.1                :math:`\mu_{\text{SIRS}}`       prob. of recovery per update step
``eta_sirs``          0.1                :math:`\eta_{\text{SIRS}}`      prob. of becoming susceptible per update step
==================== ================== =============================== ==================================================================================


.. admonition:: Special requirements for SIRS neurons

   The following requirements must be observed. NEST does not
   enforce them. Breaching the requirements can lead to meaningless
   results.

   1. SIRS neurons must only be connected to other SIR neurons.

   #. No more than one connection must be created between any pair of
      SIRS neurons. When using probabilistic connection rules, specify
      ``'allow_autapses': False`` to avoid accidental creation of
      multiple connections between a pair of neurons.


References
++++++++++

.. [1] Kermack WO and  McKendrick AG. 1991. Contributions to the mathematical theory of epidemics—II. the problem of endemicity. Bulletin of Mathematical Biology 53.

.. [2] Merger C, Albers J, Honerkamp C, and Helias M. 2024. Spurious Self-Feedback of Mean-Field Predictions Inflates Infection Curves. Physical Review E 110 (2): 024308. https://doi.org/10.1103/PhysRevE.110.024308.

Receives
++++++++

CurrentEvent

See also
++++++++

sir_neuron, sis_neuron

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: sir_neuron

EndUserDocs */
// clang-format on
/**
 * SIRS neuron with three discrete states: S, I, R.
 *
 * @note
 * This neuron has a special use for spike events to convey the
 * sirs state of the neuron to the target. The neuron model
 * only sends a spike if a transition of its state occurs. If the
 * state makes a transition from S to I it sends a spike with multiplicity 1,
 * if a transition from I to R occurs, it sends a spike with multiplicity 2.
 * If a neuron transitions from R to S, no spike is sent because this state
 * change is not relevant for other receiving neurons.
 * The decoding scheme relies on the feature that spikes with multiplicity
 * larger than 1 are delivered consecutively, also in a parallel setting.
 * The creation of double connections between sir neurons will
 * destroy the decoding scheme, as this effectively duplicates
 * every event. Using random connection routines it is therefore
 * advisable to set the property 'allow_multapses' to false.
 *
 * @see sir_neuron
 */

class sirs_update_function
{
private:
  double beta_sirs_;  //!< transition probability S->I
  double mu_sirs_;    //!< transition probability I->R
  double eta_sirs_;   //!< transition probability R->S

public:
  sirs_update_function();

  void get( Dictionary& ) const;
  void set( const Dictionary&, Node* node );

  size_t operator()( RngPtr, size_t old_state, double h ) const;
  size_t get_event_multiplicity( size_t new_state ) const;
};

inline size_t
sirs_update_function::operator()( RngPtr rng, size_t old_state, double h ) const
{
  size_t new_state = 0;

  if ( old_state == 0 )  // neuron is susceptible
  {
    new_state = 0;
    if ( rng->drand() < beta_sirs_ * h )
    {
      new_state = 1;  // neuron gets infected
    }
  }

  if ( old_state == 1 )  // neuron is infected
  {
    new_state = 1;
    if ( rng->drand() < mu_sirs_ )
    {
      new_state = 2;  // neuron recovers
    }
  }

  if ( old_state == 2 )  // neuron is recovered
  {
    new_state = 2;
    if ( rng->drand() < eta_sirs_ )
    {
      new_state = 0;  // neuron becomes susceptible
    }
  }

  return new_state;
}

inline size_t
sirs_update_function::get_event_multiplicity( size_t new_state ) const
{
  if ( new_state == 0 )
  {
    return 0;
  }
  return new_state == 2 ? 2 : 1;
}

typedef infection_neuron< sirs_update_function > sirs_neuron;
void register_sirs_neuron( const std::string& name );

template <>
void RecordablesMap< sirs_neuron >::create();

}  // namespace

#endif /* #ifndef SIRS_NEURON_H */
