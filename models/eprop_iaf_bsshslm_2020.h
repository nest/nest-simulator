/*
 *  eprop_iaf_bsshslm_2020.h
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

#ifndef EPROP_IAF_BSSHSLM_2020_H
#define EPROP_IAF_BSSHSLM_2020_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node_recurrent.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based, integrate-and-fire, hard threshold

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron model with delta-shaped or exponentially filtered
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_iaf_bsshslm_2020`` is an implementation of a leaky integrate-and-fire
neuron model with delta-shaped postsynaptic currents used for eligibility
propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ``_bsshslm_2020`` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.

.. note::
  The neuron dynamics of the ``eprop_iaf_bsshslm_2020`` model (excluding e-prop
  plasticity) are similar to the neuron dynamics of the ``iaf_psc_delta`` model,
  with minor differences, such as the propagator of the post-synaptic current
  and the voltage reset upon a spike.

The membrane voltage time course :math:`v_j^t` of the neuron :math:`j` is given by:

.. math::
  v_j^t &= \alpha v_j^{t-1} + \zeta \sum_{i \neq j} W_{ji}^\text{rec} z_i^{t-1}
    + \zeta \sum_i W_{ji}^\text{in} x_i^t - z_j^{t-1} v_\text{th} \,, \\
  \alpha &= e^{ -\frac{ \Delta t }{ \tau_\text{m} } } \,, \\
  \zeta &=
    \begin{cases}
      1 \\
      1 - \alpha
    \end{cases} \,, \\

where :math:`W_{ji}^\text{rec}` and :math:`W_{ji}^\text{in}` are the recurrent and
input synaptic weight matrices, and :math:`z_i^{t-1}` is the recurrent presynaptic
state variable, while :math:`x_i^t` represents the input at time :math:`t`.

Descriptions of further parameters and variables can be found in the table below.

The spike state variable is expressed by a Heaviside function:

.. math::
  z_j^t = H \left( v_j^t - v_\text{th} \right) \,. \\

If the membrane voltage crosses the threshold voltage :math:`v_\text{th}`, a spike is
emitted and the membrane voltage is reduced by :math:`v_\text{th}` in the next
time step. After the time step of the spike emission, the neuron is not
able to spike for an absolute refractory period :math:`t_\text{ref}`.

An additional state variable and the corresponding differential equation
represents a piecewise constant external current.

See the documentation on the :doc:`iaf_psc_delta<../models/iaf_psc_delta/>` neuron model
for more information on the integration of the subthreshold dynamics.

The change of the synaptic weight is calculated from the gradient :math:`g` of
the loss :math:`E` with respect to the synaptic weight :math:`W_{ji}`:
:math:`\frac{ \text{d}E }{ \text{d} W_{ij} }`
which depends on the presynaptic
spikes :math:`z_i^{t-1}`, the surrogate gradient or pseudo-derivative
of the spike state variable with respect to the postsynaptic membrane
voltage :math:`\psi_j^t` (the product of which forms the eligibility
trace :math:`e_{ji}^t`), and the learning signal :math:`L_j^t` emitted
by the readout neurons.

.. math::
  \frac{ \text{d} E }{ \text{d} W_{ji} } &= \sum_t L_j^t \bar{e}_{ji}^t \,, \\
   e_{ji}^t &= \psi^t_j \bar{z}_i^{t-1} \,, \\

.. include:: ../models/eprop_iaf.rst
   :start-after: .. start_surrogate-gradient-functions
   :end-before: .. end_surrogate-gradient-functions

The eligibility trace and the presynaptic spike trains are low-pass filtered
with the following exponential kernels:

.. math::
  \bar{e}_{ji}^t &= \mathcal{F}_\kappa \left( e_{ji}^t \right) \,, \\
  \kappa &= e^{ -\frac{\Delta t }{ \tau_\text{m,out} }} \,, \\
  \bar{z}_i^t &= \mathcal{F}_\alpha(z_i^t) \,, \\
  \mathcal{F}_\alpha \left( z_i^t \right) &= \alpha \mathcal{F}_\alpha \left( z_i^{t-1} \right) + z_i^t \,, \\
  \mathcal{F}_\alpha \left( z_i^0 \right) &= z_i^0 \,, \\

where :math:`\tau_\text{m,out}` is the membrane time constant of the readout neuron.

Furthermore, a firing rate regularization mechanism keeps the average firing
rate :math:`f^\text{av}_j` of the postsynaptic neuron close to a target firing rate
:math:`f^\text{target}`. The gradient :math:`g_\text{reg}` of the regularization loss :math:`E_\text{reg}`
with respect to the synaptic weight :math:`W_{ji}` is given by:

.. math::
  \frac{ \text{d} E_\text{reg} }{ \text{d} W_{ji} }
    = c_\text{reg} \sum_t \frac{ 1 }{ T n_\text{trial} }
    \left( f^\text{target} - f^\text{av}_j \right) e_{ji}^t \,, \\

where :math:`c_\text{reg}` is a constant scaling factor and the average
is taken over the time that passed since the previous update, that is, the number of
trials :math:`n_\text{trial}` times the duration of an update interval :math:`T`.

The overall gradient is given by the addition of the two gradients.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_adapt_bsshslm_2020<../models/eprop_iaf_adapt_bsshslm_2020/>`
 * :doc:`eprop_readout_bsshslm_2020<../models/eprop_readout_bsshslm_2020/>`
 * :doc:`eprop_synapse_bsshslm_2020<../models/eprop_synapse_bsshslm_2020/>`
 * :doc:`eprop_learning_signal_connection_bsshslm_2020<../models/eprop_learning_signal_connection_bsshslm_2020/>`

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
``regular_spike_arrival``   Boolean                                 ``True`` If ``True``, the input spikes
                                                                             arrive at the end of the time step,
                                                                             if ``False`` at the beginning
                                                                             (determines PSC scale)
``t_ref``                   ms      :math:`t_\text{ref}`                 2.0 Duration of the refractory period
``tau_m``                   ms      :math:`\tau_\text{m}`               10.0 Time constant of the membrane
``V_min``                   mV      :math:`v_\text{min}`    negative maximum Absolute lower bound of the
                                                            value            membrane voltage
                                                            representable by
                                                            a ``double``
                                                            type in C++
``V_th``                    mV      :math:`v_\text{th}`                -55.0 Spike threshold voltage
=========================== ======= ======================= ================ ===================================

=============================== ==== ======================= ================== ================================
**E-prop parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                       Unit Math equivalent         Default            Description
=============================== ==== ======================= ================== ================================
``c_reg``                            :math:`c_\text{reg}`                 0.0   Coefficient of firing rate
                                                                                regularization
``f_target``                    Hz   :math:`f^\text{target}`             10.0   Target firing rate of rate
                                                                                regularization
``beta``                             :math:`\beta`                        1.0   Width scaling of surrogate
                                                                                gradient / pseudo-derivative of
                                                                                membrane voltage
``gamma``                            :math:`\gamma`                       0.3   Height scaling of surrogate
                                                                                gradient / pseudo-derivative of
                                                                                membrane voltage
``surrogate_gradient_function``      :math:`\psi`            "piecewise_linear" Surrogate gradient /
                                                                                pseudo-derivative function
                                                                                ["piecewise_linear",
                                                                                "exponential",
                                                                                "fast_sigmoid_derivative",
                                                                                "arctan"]
=============================== ==== ======================= ================== ================================

Recordables
+++++++++++

The following state variables evolve during simulation and can be recorded.

================== ==== =============== ============= ================
**Neuron state variables and recordables**
----------------------------------------------------------------------
State variable     Unit Math equivalent Initial value Description
================== ==== =============== ============= ================
``V_m``            mV   :math:`v_j`             -70.0 Membrane voltage
================== ==== =============== ============= ================

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

.. include:: ../models/eprop_iaf.rst
   :start-after: .. start_surrogate-gradient-references
   :end-before: .. end_surrogate-gradient-references

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

.. listexamples:: eprop_iaf_bsshslm_2020

EndUserDocs */

void register_eprop_iaf_bsshslm_2020( const std::string& name );

/**
 * @brief Class implementing a LIF neuron model for e-prop plasticity.
 *
 * Class implementing a current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al. (2020).
 */
class eprop_iaf_bsshslm_2020 : public EpropArchivingNodeRecurrent< true >
{

public:
  //! Default constructor.
  eprop_iaf_bsshslm_2020();

  //! Copy constructor.
  eprop_iaf_bsshslm_2020( const eprop_iaf_bsshslm_2020& );

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

  double compute_gradient( std::vector< long >&, const long, const long, const double, const bool ) override;

  long get_shift() const override;
  bool is_eprop_recurrent_node() const override;

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_iaf_bsshslm_2020 >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_iaf_bsshslm_2020 >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Coefficient of firing rate regularization.
    double c_reg_;

    //! Leak / resting membrane potential (mV).
    double E_L_;

    //! Target firing rate of rate regularization (spikes/s).
    double f_target_;

    //! Width scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double beta_;

    //! Height scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double gamma_;

    //! Constant external input current (pA).
    double I_e_;

    //! If True, the input spikes arrive at the beginning of the time step, if False at the end (determines PSC scale).
    bool regular_spike_arrival_;

    //! Surrogate gradient / pseudo-derivative function of the membrane voltage ["piecewise_linear", "exponential",
    //! "fast_sigmoid_derivative", "arctan"]
    std::string surrogate_gradient_function_;

    //! Duration of the refractory period (ms).
    double t_ref_;

    //! Time constant of the membrane (ms).
    double tau_m_;

    //! Absolute lower bound of the membrane voltage relative to the leak membrane potential (mV).
    double V_min_;

    //! Spike threshold voltage relative to the leak membrane potential (mV).
    double V_th_;

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
    //! Learning signal. Sum of weighted error signals coming from the readout neurons.
    double learning_signal_;

    //! Number of remaining refractory steps.
    int r_;

    //! Surrogate gradient / pseudo-derivative of the membrane voltage.
    double surrogate_gradient_;

    //! Input current (pA).
    double i_in_;

    //! Membrane voltage relative to the leak membrane potential (mV).
    double v_m_;

    //! Binary spike state variable - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
    double z_;

    //! Binary input spike state variable - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
    double z_in_;

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
    Buffers_( eprop_iaf_bsshslm_2020& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_iaf_bsshslm_2020& );

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Logger for universal data.
    UniversalDataLogger< eprop_iaf_bsshslm_2020 > logger_;
  };

  //! Structure of internal variables.
  struct Variables_
  {
    //! Propagator matrix entry for evolving the membrane voltage (mathematical symbol "alpha" in user documentation).
    double P_v_m_;

    //! Propagator matrix entry for evolving the incoming spike state variables (mathematical symbol "zeta" in user
    //! documentation).
    double P_z_in_;

    //! Propagator matrix entry for evolving the incoming currents.
    double P_i_in_;

    //! Total refractory steps.
    int RefractoryCounts_;
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
  static RecordablesMap< eprop_iaf_bsshslm_2020 > recordablesMap_;
};

inline long
eprop_iaf_bsshslm_2020::get_shift() const
{
  return offset_gen_ + delay_in_rec_;
}

inline bool
eprop_iaf_bsshslm_2020::is_eprop_recurrent_node() const
{
  return true;
}

inline size_t
eprop_iaf_bsshslm_2020::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
eprop_iaf_bsshslm_2020::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_bsshslm_2020::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_bsshslm_2020::handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_bsshslm_2020::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_iaf_bsshslm_2020::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
eprop_iaf_bsshslm_2020::set_status( const DictionaryDatum& d )
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

template <>
void RecordablesMap< eprop_iaf_bsshslm_2020 >::create();

} // namespace nest

#endif // EPROP_IAF_BSSHSLM_2020_H
