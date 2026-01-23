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
#include "eprop_archiving_node_impl.h"
#include "eprop_archiving_node_recurrent.h"
#include "eprop_synapse.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based, integrate-and-fire, adaptive threshold

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron model with delta-shaped
postsynaptic currents and threshold adaptation for e-prop plasticity

Description
+++++++++++

``eprop_iaf_psc_delta_adapt`` is an implementation of a leaky integrate-and-fire
neuron model with delta-shaped postsynaptic currents and threshold adaptation
used for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

 .. note::
   The neuron dynamics of the ``eprop_iaf_psc_delta_adapt`` model (excluding
   e-prop plasticity and the threshold adaptation) are similar to the neuron
   dynamics of the ``iaf_psc_delta`` model, with minor differences, such as the
   propagator of the post-synaptic current and the voltage reset upon a spike.

The membrane voltage time course :math:`v_j^t` of the neuron :math:`j` is given by:

.. math::
  v_j^t &= \alpha v_j^{t-1} + \sum_{i \neq j} W_{ji}^\text{rec} z_i^{t-1}
    + \sum_i W_{ji}^\text{in} x_i^t \,, \\
  \alpha &= e^{ -\frac{ \Delta t }{ \tau_\text{m} } } \,, \\

where :math:`W_{ji}^\text{rec}` and :math:`W_{ji}^\text{in}` are the recurrent and
input synaptic weight matrices, and :math:`z_i^{t-1}` is the recurrent presynaptic
state variable, while :math:`x_i^t` represents the input at time :math:`t`.

Descriptions of further parameters and variables can be found in the table below.

The threshold adaptation is given by:

.. math::
  A_j^t &= v_\text{th} + \beta a_j^t \,, \\
  a_j^t &= \rho a_j^{t-1} + z_j^{t-1} \,, \\
  \rho &= e^{-\frac{ \Delta t }{ \tau_\text{a} }} \,. \\

The spike state variable is expressed by a Heaviside function:

.. math::
  z_j^t = H \left( v_j^t - A_j^t \right) \,. \\

If the membrane voltage crosses the adaptive threshold voltage :math:`A_j^t`, a spike is
emitted and the membrane voltage is reset to :math:`v_\text{reset}`. After the time step
of the spike emission, the neuron is not able to spike for an absolute refractory period
:math:`t_\text{ref}` during which the membrane potential stays clamped to the reset voltage
:math:`v_\text{reset}`, thus

.. math::
  v_m = v_\text{reset} \quad \text{for} \quad t_\text{spk} \leq t \leq t_\text{spk} + t_\text{ref} \,.

Spikes arriving while the neuron is refractory are discarded by default. However,
if ``refractory_input`` is set to ``True`` they are damped for each time step
until the end of the refractory period and then added to the membrane voltage.

An additional state variable and the corresponding differential equation
represents a piecewise constant external current.

See the documentation on the :doc:`iaf_psc_delta<../models/iaf_psc_delta/>` neuron model
for more information on the integration of the subthreshold dynamics.

The change of the synaptic weight is calculated from the gradient :math:`g^t` of
the loss :math:`E^t` with respect to the synaptic weight :math:`W_{ji}`:
:math:`\frac{ \text{d} E^t }{ \text{d} W_{ij} }`
which depends on the presynaptic
spikes :math:`z_i^{t-2}`, the surrogate gradient or pseudo-derivative
of the spike state variable with respect to the postsynaptic membrane
voltage :math:`\psi_j^{t-1}` (the product of which forms the eligibility
trace :math:`e_{ji}^{t-1}`), and the learning signal :math:`L_j^t` emitted
by the readout neurons.

Surrogate gradients help overcome the challenge of the spiking function's
non-differentiability, facilitating the use of gradient-based learning
techniques such as e-prop. The non-existent derivative of the spiking
variable with respect to the membrane voltage,
:math:`\frac{\partial z^t_j}{ \partial v^t_j}`, can be effectively
replaced with a variety of surrogate gradient functions, as detailed in
various studies (see, e.g., [3]_). NEST currently provides four
different surrogate gradient functions:

1. A piecewise linear function used among others in [1]_:

.. math::
  \psi_j^t = \frac{ \gamma }{ v_\text{th} } \text{max}
    \left( 0, 1-\beta \left| \frac{ v_j^t - v_\text{th} }{ v_\text{th} }\right| \right) \,. \\

2. An exponential function used in [4]_:

.. math::
  \psi_j^t = \gamma \exp \left( -\beta \left| v_j^t - v_\text{th} \right| \right) \,. \\

3. The derivative of a fast sigmoid function used in [5]_:

.. math::
  \psi_j^t = \gamma \left( 1 + \beta \left| v_j^t - v_\text{th} \right| \right)^2 \,. \\

4. An arctan function used in [6]_:

.. math::
  \psi_j^t = \frac{\gamma}{\pi} \frac{1}{ 1 + \left( \beta \pi \left( v_j^t - v_\text{th} \right) \right)^2 } \,. \\

In the interval between two presynaptic spikes, the gradient is calculated
at each time step until the cutoff time point. This computation occurs over
the time range:

:math:`t \in \left[ t_\text{spk,prev}, \min \left( t_\text{spk,prev} + \Delta t_\text{c}, t_\text{spk,curr} \right)
\right]`.

Here, :math:`t_\text{spk,prev}` represents the time of the previous spike that
passed the synapse, while :math:`t_\text{spk,curr}` is the time of the
current spike, which triggers the application of the learning rule and the
subsequent synaptic weight update. The cutoff :math:`\Delta t_\text{c}`
defines the maximum allowable interval for integration between spikes.
The expression for the gradient is given by:

.. math::
  \frac{ \text{d} E^t }{ \text{d} W_{ji} } &= L_j^t \bar{e}_{ji}^{t-1} \,, \\
  e_{ji}^{t-1} &= \psi_j^{t-1} \left( \bar{z}_i^{t-2} - \beta \epsilon_{ji,a}^{t-2} \right) \,, \\
  \epsilon^{t-2}_{ji,\text{a}} &= e_{ji}^{t-1} + \rho \epsilon_{ji,a}^{t-3} \,. \\

The eligibility trace and the presynaptic spike trains are low-pass filtered
with the following exponential kernels:

.. math::
  \bar{e}_{ji}^t &= \mathcal{F}_\kappa \left( e_{ji}^t \right)
    = \kappa \bar{e}_{ji}^{t-1} + \left( 1 - \kappa \right) e_{ji}^t \,, \\
  \bar{z}_i^t &= \mathcal{F}_\alpha \left( z_{i}^t \right)= \alpha \bar{z}_i^{t-1} + z_i^t \,. \\

Furthermore, a firing rate regularization mechanism keeps the exponential moving average of the postsynaptic
neuron's firing rate :math:`f_j^{\text{ema},t}` close to a target firing rate
:math:`f^\text{target}`. The gradient :math:`g_\text{reg}^t` of the regularization loss :math:`E_\text{reg}^t`
with respect to the synaptic weight :math:`W_{ji}` is given by:

.. math::
  \frac{ \text{d} E_\text{reg}^t }{ \text{d} W_{ji}}
    &\approx c_\text{reg} \left( f^{\text{ema},t}_j - f^\text{target} \right) \bar{e}_{ji}^t \,, \\
  f^{\text{ema},t}_j &= \mathcal{F}_{\kappa_\text{reg}} \left( \frac{z_j^t}{\Delta t} \right)
    = \kappa_\text{reg} f^{\text{ema},t-1}_j + \left( 1 - \kappa_\text{reg} \right) \frac{z_j^t}{\Delta t} \,, \\

where :math:`c_\text{reg}` is a constant scaling factor.

The overall gradient is given by the addition of the two gradients.

As a last step for every round in the loop over the time steps :math:`t`, the new weight is retrieved by feeding the
current gradient :math:`g^t` to the optimizer (see :doc:`weight_optimizer<../models/weight_optimizer/>`
for more information on the available optimizers):

.. math::
  w^t = \text{optimizer} \left( t, g^t, w^{t-1} \right) \,. \\

After the loop has terminated, the filtered dynamic variables of e-prop are propagated from the end of the cutoff until
the next spike:

.. math::
  p &= \text{max} \left( 0, t_\text{s}^{t} - \left( t_\text{s}^{t-1} + {\Delta t}_\text{c} \right) \right) \,, \\
  \bar{e}_{ji}^{t+p} &= \bar{e}_{ji}^t \kappa^p \,, \\
  \bar{z}_i^{t+p} &= \bar{z}_i^t \alpha^p \,, \\
  \epsilon^{t+p} &= \epsilon^t \rho^p \,. \\

For more information on the implementation details of the neuron model, see [7]_ and [8]_.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_psc_delta<../models/eprop_iaf_psc_delta/>`
 * :doc:`eprop_readout<../models/eprop_readout/>`
 * :doc:`eprop_synapse<../models/eprop_synapse/>`
 * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

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
       Dahmen D, Bolten M, Van Albada SJ, Diesmann M. Event-based
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
 * @brief Class implementing an adaptive LIF neuron model for e-prop plasticity with additional biological features.
 *
 * Class implementing a current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents
 * and spike threshold adaptation for e-prop plasticity according to Bellec et al. (2020) with additional biological
 * features described in Korcsak-Gorzo, Stapmanns, and Espinoza Valverde et al. (in preparation).
 */
class eprop_iaf_psc_delta_adapt : public EpropArchivingNodeRecurrent< false >
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

    //! Binary spike state variable - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
    double z_;

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
eprop_iaf_psc_delta_adapt::get_shift() const
{
  return offset_gen_ + delay_in_rec_;
}

inline bool
eprop_iaf_psc_delta_adapt::is_eprop_recurrent_node() const
{
  return true;
}

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
