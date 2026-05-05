/*
 *  hh_psc_alpha.h
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

#ifndef HH_PSC_ALPHA_H
#define HH_PSC_ALPHA_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_sf_exp.h>

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
/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL. Internally, it is
 *       a first-class C++ function, but cannot be a member function
 *       because of the C-linkage.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 * @param void* Pointer to model neuron instance.
 */
extern "C" int hh_psc_alpha_dynamics( double, const double*, double*, void* );

// clang-format off
/* BeginUserDocs: neuron, Hodgkin-Huxley, current-based, soft threshold

Short description
+++++++++++++++++

Hodgkin-Huxley neuron model with alpha-shaped synaptic currents

Description
+++++++++++

``hh_psc_alpha`` is a Hodgkin-Huxley neuron model with

* an emergent threshold from sodium channel dynamics,
* a fixed refractory period,
* intrinsic adaptation through sodium and potassium gating variables,
* :math:`\alpha`-shaped synaptic input currents.

``hh_psc_alpha`` is an implementation of a spiking neuron using the Hodgkin-Huxley
formalism [1]_, [2]_. It maintains the membrane potential :math:`V_\text{m}` as
the main state variable, governed by the equation:

.. math::

   C_{\text{m}} \frac{dV_\text{m}}{dt} = -(I_\text{Na} + I_\text{K} + I_\text{L}) + I_{\text{syn}} + I_\text{e}

where the ionic currents are defined as:

.. math::

   I_\text{Na} &= g_\text{Na} m^3 h (V_\text{m} - E_\text{Na}) \\
   I_\text{K}  &= g_\text{K} n^4 (V_\text{m} - E_\text{K}) \\
   I_\text{L}  &= g_\text{L} (V_\text{m} - E_\text{L})

The gating variables :math:`m`, :math:`h`, and :math:`n` evolve according to:

.. math::

   \frac{dm}{dt} &= \alpha_m(1-m) - \beta_m m \\
   \frac{dh}{dt} &= \alpha_h(1-h) - \beta_h h \\
   \frac{dn}{dt} &= \alpha_n(1-n) - \beta_n n

with the rate constants given by:

.. math::

   \alpha_n(V) &= \frac{0.01(V+55)}{1-\exp(-(V+55)/10)} \\
   \beta_n(V)  &= 0.125\exp(-(V+65)/80) \\
   \alpha_m(V) &= \frac{0.1(V+40)}{1-\exp(-(V+40)/10)} \\
   \beta_m(V)  &= 4\exp(-(V+65)/18) \\
   \alpha_h(V) &= 0.07\exp(-(V+65)/20) \\
   \beta_h(V)  &= \frac{1}{1+\exp(-(V+35)/10)}

Synaptic input
..............

The synaptic input current has an excitatory and an inhibitory component

.. math::

   I_{\text{syn}}(t) = I_{\text{syn, ex}}(t) + I_{\text{syn, in}}(t)

where each component is modeled as an alpha function:

.. math::

   I_{\text{syn, X}}(t) = \sum_{j} w_j \sum_k i_{\text{syn, X}}(t-t_j^k-d_j)

The individual post-synaptic currents (PSCs) are given by

.. math::

   i_{\text{syn, X}}(t) = \frac{e}{\tau_{\text{syn, X}}} t e^{-\frac{t}{\tau_{\text{syn, X}}}} \Theta(t)

where :math:`\Theta(x)` is the Heaviside step function. The PSCs are normalized to unit maximum, that is,

.. math::

   i_{\text{syn, X}}(t= \tau_{\text{syn, X}}) = 1 \text{ pA} \;.

Spike detection
...............

In contrast to integrate-and-fire neurons, Hodgkin-Huxley neurons do not have an
explicit threshold for spike emission. Instead, spikes emerge from the continuous
dynamics when sodium channels open sufficiently to depolarize the membrane.

The ``hh_psc_alpha`` model detects spikes using a combined threshold-and-local-maximum
search: a spike is registered when the membrane potential crosses 0 mV from below
and is at a local maximum (the previous membrane potential was higher). This is
implemented as:

.. math::

   V_\text{m} \ge 0 \quad \text{and} \quad V_\text{m}(t-\Delta t) > V_\text{m}(t)

The model includes a refractory period (2 ms by default) that suppresses additional
spikes during the downstroke of the action potential.

.. note::

   For details on asynchronicity in spike and firing events with Hodgkin-Huxley
   models see :ref:`hh_details`.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

================== ================== =============================== ===========================================================================
**Parameter**      **Default**        **Math equivalent**             **Description**
================== ================== =============================== ===========================================================================
``E_L``            -54.402 mV         :math:`E_\text{L}`              Leak reversal potential (resting potential)
``C_m``            100 pF             :math:`C_{\text{m}}`            Capacity of the membrane
``g_L``            30 nS              :math:`g_\text{L}`              Leak conductance
``t_ref``          2 ms               :math:`t_{\text{ref}}`          Duration of refractory period
``g_Na``           12000 nS           :math:`g_\text{Na}`             Sodium peak conductance
``E_Na``           50 mV              :math:`E_\text{Na}`             Sodium reversal potential
``g_K``            3600 nS            :math:`g_\text{K}`              Potassium peak conductance
``E_K``            -77 mV             :math:`E_\text{K}`              Potassium reversal potential
``tau_syn_ex``     0.2 ms             :math:`\tau_{\text{syn, ex}}`   Rise time of the excitatory synaptic alpha function
``tau_syn_in``     2.0 ms             :math:`\tau_{\text{syn, in}}`   Rise time of the inhibitory synaptic alpha function
``I_e``            0 pA               :math:`I_\text{e}`              Constant input current
================== ================== =============================== ===========================================================================

The following state variables evolve during simulation and are available either as neuron properties or as recordables.

==================== ================= ========================== =============================================
**State variable**  **Initial value**  **Math equivalent**        **Description**
==================== ================= ========================== =============================================
``V_m``              -65 mV            :math:`V_\text{m}`          Membrane potential
``Act_m``            ~0.05             :math:`m`                   Sodium activation variable
``Inact_h``          ~0.6              :math:`h`                   Sodium inactivation variable
``Act_n``            ~0.32             :math:`n`                   Potassium activation variable
``I_syn_ex``         0 pA              :math:`I_{\text{syn, ex}}`  Excitatory synaptic input current
``I_syn_in``         0 pA              :math:`I_{\text{syn, in}}`  Inhibitory synaptic input current
==================== ================= ========================== =============================================


References
++++++++++

.. [1] Hodgkin AL and Huxley A F (1952). A quantitative description of
       membrane current and its application to conduction and excitation in
       nerve. The Journal of Physiology 117.
       DOI: https://doi.org/10.1113/jphysiol.1952.sp004764
.. [2] Gerstner W, Kistler W M (2002). Spiking neuron models: Single neurons,
       populations, plasticity. New York: Cambridge University Press
       DOI: https://doi.org/10.1017/CBO9780511815706

Sends
+++++

SpikeEvent

Receives
+++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

hh_cond_exp_traub, hh_psc_alpha_gap, hh_psc_alpha_clopath


Examples using this model
+++++++++++++++++++++++++

.. listexamples:: hh_psc_alpha

EndUserDocs */
// clang-format on

void register_hh_psc_alpha( const std::string& name );

class hh_psc_alpha : public ArchivingNode
{

public:
  hh_psc_alpha();
  hh_psc_alpha( const hh_psc_alpha& );
  ~hh_psc_alpha() override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( Dictionary& ) const override;
  void set_status( const Dictionary& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;
  void update( Time const&, const long, const long ) override;

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int hh_psc_alpha_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< hh_psc_alpha >;
  friend class UniversalDataLogger< hh_psc_alpha >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double t_ref_;    //!< refractory time in ms
    double g_Na;      //!< Sodium Conductance in nS
    double g_K;       //!< Potassium Conductance in nS
    double g_L;       //!< Leak Conductance in nS
    double C_m;       //!< Membrane Capacitance in pF
    double E_Na;      //!< Sodium Reversal Potential in mV
    double E_K;       //!< Potassium Reversal Potential in mV
    double E_L;       //!< Leak reversal Potential (aka resting potential) in mV
    double tau_synE;  //!< Synaptic Time Constant Excitatory Synapse in ms
    double tau_synI;  //!< Synaptic Time Constant for Inhibitory Synapse in ms
    double I_e;       //!< Constant Current in pA

    Parameters_();  //!< Sets default parameter values

    void get( Dictionary& ) const;              //!< Store current values in Dictionary
    void set( const Dictionary&, Node* node );  //!< Set values from Dictionary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor required because of C-style array.
   */
  struct State_
  {
    /**
     * Enumeration identifying elements in state array State_::y_.
     * The state vector must be passed to GSL as a C array. This enum
     * identifies the elements of the vector. It must be public to be
     * accessible from the iteration function.
     */
    enum StateVecElems
    {
      V_M = 0,
      HH_M,    // 1
      HH_H,    // 2
      HH_N,    // 3
      DI_EXC,  // 4
      I_EXC,   // 5
      DI_INH,  // 6
      I_INH,   // 7
      STATE_VEC_SIZE
    };


    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_;  //!< number of refractory steps remaining

    State_( const Parameters_& );  //!< Default initialization
    State_( const State_& );

    State_& operator=( const State_& );

    void get( Dictionary& ) const;
    void set( const Dictionary&, Node* node );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( hh_psc_alpha& );                   //!< Sets buffer pointers to 0
    Buffers_( const Buffers_&, hh_psc_alpha& );  //!< Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< hh_psc_alpha > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;     //!< stepping function
    gsl_odeiv_control* c_;  //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;   //!< evolution function
    gsl_odeiv_system sys_;  //!< struct describing system

    // Since IntegrationStep_ is initialized with step_, and the resolution
    // cannot change after nodes have been created, it is safe to place both
    // here.
    double step_;             //!< step size in ms
    double IntegrationStep_;  //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** initial value to normalise excitatory synaptic current */
    double PSCurrInit_E_;

    /** initial value to normalise inhibitory synaptic current */
    double PSCurrInit_I_;

    int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< hh_psc_alpha > recordablesMap_;
};


inline size_t
hh_psc_alpha::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline size_t
hh_psc_alpha::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
hh_psc_alpha::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
hh_psc_alpha::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
hh_psc_alpha::get_status( Dictionary& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  d[ names::recordables ] = recordablesMap_.get_list();
}

inline void
hh_psc_alpha::set_status( const Dictionary& d )
{
  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set( d, this );    // throws if BadProperty
  State_ stmp = S_;       // temporary copy in case of errors
  stmp.set( d, this );    // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

}  // namespace

#endif  // HAVE_GSL
#endif  // HH_PSC_ALPHA_H
