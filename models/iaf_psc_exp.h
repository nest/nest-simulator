/*
 *  iaf_psc_exp.h
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

#ifndef IAF_PSC_EXP_H
#define IAF_PSC_EXP_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{
// Disable clang-formatting for documentation due to over-wide table.
// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, current-based, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model with exponential-shaped input currents

Description
+++++++++++

``iaf_psc_exp``  a leaky integrate-and-fire model with

* a hard threshold (if :math:`\delta=0`, see below)
* a fixed refractory period,
* no adaptation mechanisms,
* exponential-shaped synaptic input currents according to [1]_.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

where the synaptic input current :math:`I_{\text{syn}}(t)` is discussed below and :math:`I_\text{e}` is
a constant input current set as a model parameter.

A spike is emitted at time step :math:`t^*=t_{k+1}` if

.. math::

   V_\text{m}(t_k) < V_{\text{th}} \quad\text{and}\quad V_\text{m}(t_{k+1})\geq V_\text{th} \;.

Subsequently,

.. math::

   V_\text{m}(t) = V_{\text{reset}} \quad\text{for}\quad t^* \leq t < t^* + t_{\text{ref}} \;,

that is, the membrane potential is clamped to :math:`V_{\text{reset}}` during the refractory period.

.. note::

	Spiking in this model can be either deterministic (:math:`\delta=0`) or stochastic (:math:`\delta > 0`).
	In the stochastic case, this model implements a type of spike response model with escape noise.
	Spiking is given by an inhomogeneous Poisson process with rate

	.. math::
		\rho \exp \left( \frac{V_{\text{m}} - V_{\text{th}}}{\delta} \right).

Synaptic input
..............

The synaptic input current has an excitatory and an inhibitory component

.. math::

   I_{\text{syn}}(t) = I_{\text{syn, ex}}(t) + I_{\text{syn, in}}(t)

where

.. math::

   I_{\text{syn, X}}(t) = \sum_{j} \sum_{k} i_{\text{syn, X}}(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`\text{X} = \text{ex}`)
or inhibitory (:math:`\text{X} = \text{in}`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, and :math:`d_j`
is the delay from neuron :math:`j`.

The individual post-synaptic currents (PSCs) are given by

.. math::

   i_{\text{syn, X}}(t) = w \cdot e^{-\frac{t}{\tau_{\text{syn, X}}}} \cdot \Theta(t)

where :math:`w` is a weight (excitatory if :math:`w > 0` or inhibitory if :math:`w < 0`), and :math:`\Theta(x)` is the Heaviside step function. The time dependent components of the PSCs are normalized to unit maximum, so that,

.. math::

   i_{\text{syn, X}}(t= 0) = w \;.

As a consequence, the total charge :math:`q` transferred by a single PSC depends
on the synaptic time constant according to

.. math::

   q = \int_0^{\infty}  i_{\text{syn, X}}(t) dt = w \cdot \tau_{\text{syn, X}} \;.


.. note::


  If ``tau_m`` is very close to ``tau_syn_ex`` or ``tau_syn_in``, the model
  will numerically behave as if ``tau_m`` is equal to ``tau_syn_ex`` or
  ``tau_syn_in``, respectively, to avoid numerical instabilities.

  NEST uses exact integration [2]_, [3]_ to integrate subthreshold membrane dynamics
  with  maximum precision.

  For implementation details see the
  `IAF Integration Singularity notebook <../model_details/IAF_Integration_Singularity.ipynb>`_.

``iaf_psc_exp`` can handle current input in two ways:

1. Current input through ``receptor_type`` 0 is handled as a stepwise constant
   current input as in other iaf models, that is, this current directly enters the
   membrane potential equation.
2. In contrast, current input through ``receptor_type`` 1 is filtered through an
   exponential kernel with the time constant of the excitatory synapse,
   ``tau_syn_ex``.

   For an example application, see [4]_.

   **Warning:** this current input is added to the state variable
   ``i_syn_ex_``. If this variable is being recorded, its numerical value
   will thus not correspond to the excitatory synaptic input current, but to
   the sum of excitatory synaptic input current and the contribution from
   receptor type 1 currents.

For conversion between postsynaptic potentials (PSPs) and PSCs,
please refer to the ``postsynaptic_potential_to_current`` function in
`PyNEST Microcircuit: Helper Functions <https://github.com/INM-6/microcircuit-PD14-model/blob/main/PyNEST/src/microcircuit/helpers.py>`_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.


=============== ================== =============================== ========================================================================
**Parameter**   **Default**        **Math equivalent**             **Description**
=============== ================== =============================== ========================================================================
``E_L``         -70 mV             :math:`E_\text{L}`              Resting membrane potential
``C_m``         250 pF             :math:`C_{\text{m}}`            Capacity of the membrane
``tau_m``       10 ms              :math:`\tau_{\text{m}}`         Membrane time constant
``t_ref``       2 ms               :math:`t_{\text{ref}}`          Duration of refractory period
``V_th``        -55 mV             :math:`V_{\text{th}}`           Spike threshold
``V_reset``     -70 mV             :math:`V_{\text{reset}}`        Reset potential of the membrane
``tau_syn_ex``  2 ms               :math:`\tau_{\text{syn, ex}}`   Rise time of the excitatory synaptic alpha function
``tau_syn_in``  2 ms               :math:`\tau_{\text{syn, in}}`   Rise time of the inhibitory synaptic alpha function
``I_e``         0 pA               :math:`I_\text{e}`              Constant input current
``delta``       0 mV               :math:`\delta`                  Parameter scaling stochastic spiking
``rho``         0.01 1/s           :math:`\rho`                    Baseline stochastic spiking
=============== ================== =============================== ========================================================================

The following state variables evolve during simulation and are available either as neuron properties or as recordables.

================== ================= ========================== =================================
**State variable** **Initial value** **Math equivalent**        **Description**
================== ================= ========================== =================================
``V_m``            -70 mV            :math:`V_{\text{m}}`       Membrane potential
``I_syn_ex``       0 pA              :math:`I_{\text{syn, ex}}` Excitatory synaptic input current
``I_syn_in``       0 pA              :math:`I_{\text{syn, in}}` Inhibitory synaptic input current
================== ================= ========================== =================================


References
++++++++++

.. [1] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. The Journal of Neuroscience,
       20,RC50:1-5. URL: https://infoscience.epfl.ch/record/183402
.. [2] Rotter S,  Diesmann M (1999). Exact simulation of
       time-invariant linear systems with applications to neuronal
       modeling. Biologial Cybernetics 81:381-402.
       DOI: https://doi.org/10.1007/s004220050570
.. [3] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001). State
       space analysis of synchronous spiking in cortical neural
       networks. Neurocomputing 38-40:565-571.
       DOI: https://doi.org/10.1016/S0925-2312(01)00409-X
.. [4] Schuecker J, Diesmann M, Helias M (2015). Modulated escape from a
       metastable state driven by colored noise. Physical Review E 92:052119
       DOI: https://doi.org/10.1103/PhysRevE.92.052119

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_cond_exp, iaf_psc_exp_ps

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_exp

EndUserDocs */

/**
 * The present implementation uses individual variables for the
 * components of the state vector and the non-zero matrix elements of
 * the propagator. Because the propagator is a lower triangular matrix,
 * no full matrix multiplication needs to be carried out and the
 * computation can be done "in place", i.e. no temporary state vector
 * object is required.
 *
 * The template support of recent C++ compilers enables a more succinct
 * formulation without loss of runtime performance already at minimal
 * optimization levels. A future version of iaf_psc_exp will probably
 * address the problem of efficient usage of appropriate vector and
 * matrix objects.
 */

void register_iaf_psc_exp( const std::string& name );

class iaf_psc_exp : public ArchivingNode
{

public:
  iaf_psc_exp();
  iaf_psc_exp( const iaf_psc_exp& );

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

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( const Time&, const long, const long ) override;

  // intensity function
  double phi_() const;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_exp >;
  friend class UniversalDataLogger< iaf_psc_exp >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Membrane time constant in ms. */
    double Tau_;

    /** Membrane capacitance in pF. */
    double C_;

    /** Refractory period in ms. */
    double t_ref_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Threshold, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real threshold is (E_L_+Theta_). */
    double Theta_;

    /** reset value of the membrane potential */
    double V_reset_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    /** Stochastic firing intensity at threshold in 1/s. **/
    double rho_;

    /** Width of threshold region in mV. **/
    double delta_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    // state variables
    double i_0_;      //!< Stepwise constant input current
    double i_1_;      //!< Current input that is filtered through the excitatory synapse exponential kernel
    double i_syn_ex_; //!< Postsynaptic current for excitatory inputs (includes contribution from current input on
                      //!< receptor type 1)
    double i_syn_in_; //!< Postsynaptic current for inhibitory inputs
    double V_m_;      //!< Membrane potential
    int r_ref_;       //!< Absolute refractory counter (no membrane potential propagation)

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, const double, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_exp& );
    Buffers_( const Buffers_&, iaf_psc_exp& );

    //! Indices for access to different channels of input_buffer_
    enum
    {
      SYN_IN = 0,
      SYN_EX,
      I0,
      I1,
      NUM_INPUT_CHANNELS
    };

    /** buffers and sums up incoming spikes/currents */
    MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** Amplitude of the synaptic current.
        This value is chosen such that a postsynaptic potential with
        weight one has an amplitude of 1 mV.
        @note mog - I assume this, not checked.
    */
    //    double PSCInitialValue_;

    // time evolution operator
    double P20_;
    double P11ex_;
    double P11in_;
    double P21ex_;
    double P21in_;
    double P22_;

    double weighted_spikes_ex_;
    double weighted_spikes_in_;

    int RefractoryCounts_;

    RngPtr rng_; //!< random number generator of my own thread
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  inline double
  get_V_m_() const
  {
    return S_.V_m_ + P_.E_L_;
  }

  inline double
  get_I_syn_ex_() const
  {
    return S_.i_syn_ex_;
  }

  inline double
  get_I_syn_in_() const
  {
    return S_.i_syn_in_;
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
  static RecordablesMap< iaf_psc_exp > recordablesMap_;
};


inline size_t
nest::iaf_psc_exp::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_psc_exp::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_exp::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type == 0 )
  {
    return 0;
  }
  else if ( receptor_type == 1 )
  {
    return 1;
  }
  else
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
}

inline size_t
iaf_psc_exp::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_exp::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

inline double
iaf_psc_exp::phi_() const
{
  assert( P_.delta_ > 0. );
  return P_.rho_ * std::exp( 1. / P_.delta_ * ( S_.V_m_ - P_.Theta_ ) );
}

template <>
void RecordablesMap< iaf_psc_exp >::create();

} // namespace

#endif // IAF_PSC_EXP_H
