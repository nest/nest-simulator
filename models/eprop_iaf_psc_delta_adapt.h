/*
 *  eprop_iaf_psc_delta_adapt.h
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

#ifndef EPROP_IAF_PSC_DELTA_ADAPT_H
#define EPROP_IAF_PSC_DELTA_ADAPT_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node.h"
#include "eprop_archiving_node_impl.h"
#include "eprop_synapse.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, integrate-and-fire, current-based

Short description
+++++++++++++++++

Leaky integrate-and-fire model with delta-shaped input currents
for e-prop plasticity

Description
+++++++++++

``iaf_psc_delta`` is a leaky integrate-and-fire neuron model with

* a hard threshold,
* a fixed refractory period,
* Dirac delta (:math:`\delta`)-shaped synaptic input currents.


The `eprop_iaf_psc_delta_adapt` is the standard ``iaf_psc_delta`` model endowed with e-prop plasticity.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \dot{\Delta}_{\text{syn}} +
   \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

where the derivative of change in voltage due to synaptic input :math:`\dot{\Delta}_{\text{syn}}(t)` is discussed below
and :math:`I_\text{e}` is a constant input current set as a model parameter.

A spike is emitted at time step :math:`t^*=t_{k+1}` if

.. math::

   V_\text{m}(t_k) < V_{th} \quad\text{and}\quad V_\text{m}(t_{k+1})\geq V_\text{th} \;.

Subsequently,

.. math::

   V_\text{m}(t) = V_{\text{reset}} \quad\text{for}\quad t^* \leq t < t^* + t_{\text{ref}} \;,

that is, the membrane potential is clamped to :math:`V_{\text{reset}}` during the refractory period.

Synaptic input
..............

The change in membrane potential due to synaptic inputs can be formulated as:

.. math::

   \dot{\Delta}_{\text{syn}}(t) = \sum_{j} w_j \sum_k \delta(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`w_j > 0`)
or inhibitory (:math:`w_j < 0`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, :math:`d_j`
is the delay from neuron :math:`j`, and :math:`\delta` is the Dirac delta distribution.
This implies that the jump in voltage upon a single synaptic input spike is

.. math::

   \Delta_{\text{syn}} = w \;,

where :math:`w` is the corresponding synaptic weight in mV.


The change in voltage caused by the synaptic input can be interpreted as being caused
by individual post-synaptic currents (PSCs) given by

.. math::

   i_{\text{syn}}(t) = C_{\text{m}} \cdot w \cdot \delta(t).

As a consequence, the total charge :math:`q` transferred by a single PSC is

.. math::

   q = \int_0^{\infty}  i_{\text{syn, X}}(t) dt = C_{\text{m}} \cdot w \;.

By default, :math:`V_\text{m}` is not bounded from below. To limit
hyperpolarization to biophysically plausible values, set parameter
:math:`V_{\text{min}}` as lower bound of :math:`V_\text{m}`.

For details see [1]_, [2]_, [3]_, [4]_, [5]_, [6]_, [7]_, [8]_.




.. note::
   Spikes arriving while the neuron is refractory, are discarded by
   default. If the property ``refractory_input`` is set to True, such
   spikes are added to the membrane potential at the end of the
   refractory period, dampened according to the interval between
   arrival and end of refractoriness.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========================== ======= ======================= ================ ===================================
**Neuron parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                   Unit    Math equivalent         Default          Description
=========================== ======= ======================= ================ ===================================
``C_m``                     pF      :math:`C_\text{m}`                 250.0 Capacitance of the membrane
``E_L``                     mV      :math:`E_\text{L}`                 -70.0 Leak / resting membrane potential
``I_e``                     pA      :math:`I_\text{e}`                   0.0 Constant external input current
``t_ref``                   ms      :math:`t_\text{ref}`                 2.0 Duration of the refractory period
``tau_m``                   ms      :math:`\tau_\text{m}`               10.0 Time constant of the membrane
``V_min``                   mV      :math:`v_\text{min}`    negative maximum Absolute lower bound of the
                                                            value            membrane voltage
                                                            representable
                                                            by a ``double``
                                                            type in C++
``V_th``                    mV      :math:`v_\text{th}`                -55.0 Spike threshold voltage
``V_reset``                 mV      :math:`v_\text{reset}`             -70.0 Reset voltage
``refractory_input``        Boolean                                ``False`` If ``True``, spikes arriving during
                                                                             the refractory period are damped
                                                                             until it ends and then added to the
                                                                             membrane voltage
``adapt_beta``                      :math:`\beta`                        1.0 Prefactor of the threshold
                                                                             adaptation
``adapt_tau``               ms      :math:`\tau_\text{a}`               10.0 Time constant of the threshold
                                                                             adaptation
=========================== ======= ======================= ================ ===================================

=============================== ======= =========================== ================== =========================
**E-prop parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                       Unit    Math equivalent             Default            Description
=============================== ======= =========================== ================== =========================
``c_reg``                               :math:`c_\text{reg}`                     0.0   Coefficient of firing
                                                                                       rate regularization
``eprop_isi_trace_cutoff``      ms      :math:`{\Delta t}_\text{c}` maximum value      Cutoff for integration of
                                                                    representable      e-prop update between two
                                                                    by a ``long``      spikes
                                                                    type in C++
``f_target``                    Hz      :math:`f^\text{target}`                 10.0   Target firing rate of
                                                                                       rate regularization
``kappa``                               :math:`\kappa`                          0.97   Low-pass filter of the
                                                                                       eligibility trace
``kappa_reg``                           :math:`\kappa_\text{reg}`               0.97   Low-pass filter of the
                                                                                       firing rate for
                                                                                       regularization
``beta``                                :math:`\beta`                            1.0   Width scaling of
                                                                                       surrogate gradient /
                                                                                       pseudo-derivative of
                                                                                       membrane voltage
``gamma``                               :math:`\gamma`                           0.3   Height scaling of
                                                                                       surrogate gradient /
                                                                                       pseudo-derivative of
                                                                                       membrane voltage
``surrogate_gradient_function``         :math:`\psi`                "piecewise_linear" Surrogate gradient /
                                                                                       pseudo-derivative
                                                                                       function
                                                                                       ["piecewise_linear",
                                                                                       "exponential",
                                                                                       "fast_sigmoid_derivative"
                                                                                       , "arctan"]
=============================== ======= =========================== ================== =========================

Recordables
+++++++++++

The following state variables evolve during simulation and can be recorded.

================== ==== =============== ============= ========================
**Neuron state variables and recordables**
------------------------------------------------------------------------------
State variable     Unit Math equivalent Initial value Description
================== ==== =============== ============= ========================
``adaptation``          :math:`a_j`               0.0 Adaptation variable
``V_m``              mV :math:`v_j`             -70.0 Membrane voltage
``V_th_adapt``       mV :math:`A_j`             -55.0 Adapting spike threshold
================== ==== =============== ============= ========================

====================== ==== =============== ============= =========================================
**E-prop state variables and recordables**
---------------------------------------------------------------------------------------------------
State variable         Unit Math equivalent Initial value Description
====================== ==== =============== ============= =========================================
``learning_signal``      pA :math:`L_j`               0.0 Learning signal
``surrogate_gradient``      :math:`\psi_j`            0.0 Surrogate gradient / pseudo-derivative of
                                                          membrane voltage
====================== ==== =============== ============= =========================================

Usage
+++++

This model can only be used in combination with the other e-prop models
and the network architecture requires specific wiring, input, and output.
The usage is demonstrated in several
:doc:`supervised regression and classification tasks <../auto_examples/eprop_plasticity/index>`
reproducing among others the original proof-of-concept tasks in [1]_.

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Plesser HE,
       Dahmen D, Bolten M, Van Albada SJ*, Diesmann M*. Event-based
       implementation of eligibility propagation (in preparation)

.. [3] Neftci EO, Mostafa H, Zenke F (2019). Surrogate Gradient Learning in
       Spiking Neural Networks. IEEE Signal Processing Magazine, 36(6), 51-63.
       https://doi.org/10.1109/MSP.2019.2931595

.. [4] Shrestha SB, Orchard G (2018). SLAYER: Spike Layer Error Reassignment in
       Time. Advances in Neural Information Processing Systems, 31:1412-1421.
       https://proceedings.neurips.cc/paper_files/paper/2018/hash/82.. rubric:: References

.. [5] Zenke F, Ganguli S (2018). SuperSpike: Supervised Learning in Multilayer
       Spiking Neural Networks. Neural Computation, 30:1514–1541.
       https://doi.org/10.1162/neco_a_01086

.. [6] Fang W, Yu Z, Chen Y, Huang T, Masquelier T, Tian Y (2021). Deep residual
       learning in spiking neural networks. Advances in Neural Information
       Processing Systems, 34:21056–21069.
       https://proceedings.neurips.cc/paper/2021/hash/afe434653a898da20044041262b3ac74-Abstract.html

.. [7] Rotter S,  Diesmann M (1999). Exact simulation of time-invariant linear
       systems with applications to neuronal modeling. Biological Cybernetics
       81:381-402.
       https://doi.org/10.1007/s004220050570

.. [8] Diesmann M, Gewaltig MO, Rotter S, Aertsen A (2001). State space analysis
       of synchronous spiking in cortical neural networks. Neurocomputing
       38-40:565-571.
       https://doi.org/10.1016/S0925-2312(01)00409-X

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, LearningSignalConnectionEvent, DataLoggingRequest

See also
++++++++

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: eprop_iaf_psc_delta_adapt

EndUserDocs */

void register_eprop_iaf_psc_delta_adapt( const std::string& name );

/**
 * @brief Class implementing a LIF neuron model for e-prop plasticity with additional biological features.
 *
 * Class implementing a current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al. (2020) with additional biological features described in
 * Korcsak-Gorzo, Stapmanns, and Espinoza Valverde et al. (in preparation).
 */
class eprop_iaf_psc_delta_adapt : public EpropArchivingNodeRecurrent
{

public:
  //! Default constructor.
  eprop_iaf_psc_delta_adapt();

  //! Copy constructor.
  eprop_iaf_psc_delta_adapt( const eprop_iaf_psc_delta_adapt& );

  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( LearningSignalConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( LearningSignalConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  void compute_gradient( const long,
    const long,
    double&,
    double&,
    double&,
    double&,
    double&,
    double&,
    const CommonSynapseProperties&,
    WeightOptimizer* ) override;

  long get_shift() const override;
  bool is_eprop_recurrent_node() const override;
  long get_eprop_isi_trace_cutoff() const override;

  //! Pointer to member function selected for computing the surrogate gradient.
  surrogate_gradient_function compute_surrogate_gradient_;

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_iaf_psc_delta_adapt >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_iaf_psc_delta_adapt >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Time constant of the membrane (ms).
    double tau_m_;

    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Duration of the refractory period (ms).
    double t_ref_;

    //! Leak / resting membrane potential (mV).
    double E_L_;

    //! Constant external input current (pA).
    double I_e_;

    //! Spike threshold voltage relative to the leak membrane potential (mV).
    double V_th_;

    //! Absolute lower bound of the membrane voltage relative to the leak membrane potential (mV).
    double V_min_;

    //! Reset voltage relative to the leak membrane potential (mV).
    double V_reset_;

    //! If True, count spikes arriving during the refractory period.
    bool with_refr_input_;

    //! Prefactor of the threshold adaptation.
    double adapt_beta_;

    //! Time constant of the threshold adaptation (ms).
    double adapt_tau_;


    //! Coefficient of firing rate regularization.
    double c_reg_;

    //! Target firing rate of rate regularization (spikes/s).
    double f_target_;

    //! Width scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double beta_;

    //! Height scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double gamma_;

    //! Surrogate gradient / pseudo-derivative function of the membrane voltage ["piecewise_linear", "exponential",
    //! "fast_sigmoid_derivative", "arctan"]
    std::string surrogate_gradient_function_;

    //! Low-pass filter of the eligibility trace.
    double kappa_;

    //! Low-pass filter of the firing rate for regularization.
    double kappa_reg_;

    //! Time interval from the previous spike until the cutoff of e-prop update integration between two spikes (ms).
    double eprop_isi_trace_cutoff_;

    //! Default constructor.
    Parameters_();

    //! Get the parameters and their values.
    void get( DictionaryDatum& ) const;

    //! Set the parameters and throw errors in case of invalid values.
    double set( const DictionaryDatum&, Node* );
  };

  //! Structure of state variables.
  struct State_
  {
    //! Input current (pA).
    double i_in_;

    //! Membrane voltage relative to the leak membrane potential (mV).
    double v_m_;

    //! Number of remaining refractory steps.
    int r_;

    //! Count of spikes arriving during refractory period discounted for decay until end of refractory period.
    double refr_spikes_buffer_;

    //! Adaptation variable.
    double adapt_;

    //! Adapting spike threshold voltage.
    double v_th_adapt_;

    //! Learning signal. Sum of weighted error signals coming from the readout neurons.
    double learning_signal_;

    //! Surrogate gradient / pseudo-derivative of the membrane voltage.
    double surrogate_gradient_;

    //! Default constructor.
    State_();

    //! Get the state variables and their values.
    void get( DictionaryDatum&, const Parameters_& ) const;

    //! Set the state variables.
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };

  //! Structure of buffers.
  struct Buffers_
  {
    //! Default constructor.
    Buffers_( eprop_iaf_psc_delta_adapt& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_iaf_psc_delta_adapt& );

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Logger for universal data.
    UniversalDataLogger< eprop_iaf_psc_delta_adapt > logger_;
  };

  //! Structure of internal variables.
  struct Variables_
  {
    //! Propagator matrix entry for evolving the membrane voltage (mathematical symbol "alpha" in user documentation).
    double P_v_m_;

    //! Propagator matrix entry for evolving the incoming spike variables (mathematical symbol "zeta" in user
    //! documentation).
    double P_z_in_;

    //! Propagator matrix entry for evolving the incoming currents.
    double P_i_in_;

    //! Propagator matrix entry for evolving the adaptation (mathematical symbol "rho" in user documentation).
    double P_adapt_;

    //! Total refractory steps.
    int RefractoryCounts_;

    //! Time steps from the previous spike until the cutoff of e-prop update integration between two spikes.
    long eprop_isi_trace_cutoff_steps_;
  };

  //! Get the current value of the membrane voltage.
  double
  get_v_m_() const
  {
    return S_.v_m_ + P_.E_L_;
  }

  //! Get the current value of the surrogate gradient.
  double
  get_surrogate_gradient_() const
  {
    return S_.surrogate_gradient_;
  }

  //! Get the current value of the learning signal.
  double
  get_learning_signal_() const
  {
    return S_.learning_signal_;
  }

  //! Get the current value of the adapting threshold.
  double
  get_v_th_adapt_() const
  {
    return S_.v_th_adapt_ + P_.E_L_;
  }

  //! Get the current value of the adaptation.
  double
  get_adaptation_() const
  {
    return S_.adapt_;
  }

  // the order in which the structure instances are defined is important for speed

  //! Structure of parameters.
  Parameters_ P_;

  //! Structure of state variables.
  State_ S_;

  //! Structure of internal variables.
  Variables_ V_;

  //! Structure of buffers.
  Buffers_ B_;

  //! Map storing a static set of recordables.
  static RecordablesMap< eprop_iaf_psc_delta_adapt > recordablesMap_;
};

inline long
eprop_iaf_psc_delta_adapt::get_eprop_isi_trace_cutoff() const
{
  return V_.eprop_isi_trace_cutoff_steps_;
}

inline size_t
eprop_iaf_psc_delta_adapt::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_iaf_psc_delta_adapt::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
eprop_iaf_psc_delta_adapt::set_status( const DictionaryDatum& d )
{
  // temporary copies in case of errors
  Parameters_ ptmp = P_;
  State_ stmp = S_;

  // make sure that ptmp and stmp consistent - throw BadProperty if not
  const double delta_EL = ptmp.set( d, this );
  stmp.set( d, ptmp, delta_EL, this );

  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif // EPROP_IAF_PSC_DELTA_ADAPT_H
