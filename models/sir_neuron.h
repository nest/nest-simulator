/*
 *  sir_neuron.h
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

#ifndef SIR_NEURON_H
#define SIR_NEURON_H

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{
// clang-format off
/* BeginUserDocs: neuron, discrete state

Short description
+++++++++++++++++

Neuron with three discrete states: Susceptible, Infected, Recovered.

Description
+++++++++++

The ``sir_neuron`` is an implementation of a neuron which has three
discrete states: susceptible (S), infected (I) and recovered (R) [1]_.
All ``sir_neuron`` (s) of a population are updated synchronously.

When an update occurs, all susceptible neurons are infected with probability equal to
:math:`\min( \beta_{\text{SIR}} h, 1)`, where ``h`` is the number of infected
pre-synaptic  neurons, and ``beta_sir`` is a parameter controlling the
infectivity. Susceptible neurons that are not infected remain susceptible.
Infected neurons recover with probability ``mu_sir``. Infected neurons
that do not recover remain infected. Recovered neurons remain recovered.

The parameter ``tau_m`` controls the
length of the time step between updates, and hence has no influence on the
dynamics.
The state of the neuron is encoded in the variables ``S`` ( :math:`S=0` for
susceptible, :math:`S=1` for infected, :math:`S=2` for recovered) and ``h``,
which counts the number of infected pre-synaptic neurons.

NEST also supports two other variants of the SIR model: the SIS model,
where instead of transitioning to the R state, neurons transition to the S state again,
and the SIRS model, where neurons are able to transition from the R state to the S state. See
`sis_neuron` and `sirs_neuron`.


Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================== ================== =============================== ==================================================================================
**Parameter**        **Default**        **Math equivalent**             **Description**
==================== ================== =============================== ==================================================================================
``tau_m``            10 ms              :math:`\tau_{\text{m}}`         inter-update-interval
``beta_sir``         0.1                :math:`\beta_{\text{SIR}}`      infectivity per update step
``mu_sir``           0.1                :math:`\mu_{\text{SIR}}`        prob. of recovery per update step
==================== ================== =============================== ==================================================================================


.. admonition:: Special requirements for SIR neurons

   The following requirements must be observed. NEST does not
   enforce them. Breaching the requirements can lead to meaningless
   results.

   1. SIR neurons must only be connected to other SIR neurons.

   #. No more than one connection must be created between any pair of
      SIR neurons. When using probabilistic connection rules, specify
      ``'allow_autapses': False`` to avoid accidental creation of
      multiple connections between a pair of neurons.


References
++++++++++

.. [1] Kermack WO and McKendrick AG 1991. Contributions to the mathematical theory of epidemics—II. the problem of endemicity. Bulletin of Mathematical Biology 53.

.. [2] Merger, C, Albers J, Honerkamp C, and  Helias M. 2024. Spurious Self-Feedback of Mean-Field Predictions  Inflates Infection Curves. Physical Review E 110 (2): 024308. https://doi.org/10.1103/PhysRevE.110.024308.


Receives
++++++++

CurrentEvent

See also
++++++++

sirs_neuron, sis_neuron

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: sir_neuron


EndUserDocs */
// clang-format on
/**
 * SIR neuron with three discrete states: S, I, R.
 *
 * @note
 * This neuron has a special use for spike events to convey the
 * sir state of the neuron to the target. The neuron model
 * only sends a spike if a transition of its state occurs. If the
 * state makes a transition from S to I it sends a spike with multiplicity 1,
 * if a transition from I to R occurs, it sends a spike with multiplicity 2.
 * All other transitions are not allowed.
 * The decoding scheme relies on the feature that spikes with multiplicity
 * larger than 1 are delivered consecutively, also in a parallel setting.
 * The creation of double connections between sir neurons will
 * destroy the decoding scheme, as this effectively duplicates
 * every event. Using random connection routines it is therefore
 * advisable to set the property 'allow_multapses' to false.
 *
 * @see sirs_neuron
 */

void register_sir_neuron( const std::string& name );

class sir_neuron : public ArchivingNode
{

public:
  sir_neuron();
  sir_neuron( const sir_neuron& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;
  using Node::sends_signal;

  size_t send_test_event( Node&, size_t, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  size_t handles_test_event( SpikeEvent&, size_t );
  size_t handles_test_event( CurrentEvent&, size_t );
  size_t handles_test_event( DataLoggingRequest&, size_t );

  SignalType sends_signal() const;
  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void calibrate_time( const TimeConverter& tc );


private:
  void init_buffers_();
  void pre_run_hook();

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< sir_neuron >;
  friend class UniversalDataLogger< sir_neuron >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    //! mean inter-update interval in ms (acts like a membrane time constant).
    double tau_m_;
    //! transition probability S->I
    double beta_sir_;
    //! transition probability I->R
    double mu_sir_;


    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    size_t y_;               //!< output of neuron in [0,1,2]
    double h_;               //!< total input current to neuron
    double last_in_node_id_; //!< node ID of the last spike being received
    Time t_next_;            //!< time point of next update
    Time t_last_in_spike_;   //!< time point of last input spike seen

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( sir_neuron& );
    Buffers_( const Buffers_&, sir_neuron& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;


    //! Logger for all analog data
    UniversalDataLogger< sir_neuron > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    RngPtr rng_; //!< random number generator of my own thread
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the sir_neuron state of the neuron
  double
  get_output_state_() const
  {
    return S_.y_;
  }

  //! Read out the summed input of the neuron (= membrane potential)
  double
  get_input__() const
  {
    return S_.h_;
  }

  // ----------------------------------------------------------------

  /**
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  //! Mapping of recordables names to access functions
  static RecordablesMap< sir_neuron > recordablesMap_;
};

template <>
void RecordablesMap< sir_neuron >::create();

} // namespace

#endif /* #ifndef SIR_NEURON_H */
