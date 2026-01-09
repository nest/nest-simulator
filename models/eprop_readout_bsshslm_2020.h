/*
 *  eprop_readout_bsshslm_2020.h
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

#ifndef EPROP_READOUT_BSSHSLM_2020_H
#define EPROP_READOUT_BSSHSLM_2020_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node_readout.h"
#include "event.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

#include "model_manager.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based

Short description
+++++++++++++++++

Current-based leaky integrate readout neuron model with delta-shaped or exponentially filtered
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_readout_bsshslm_2020`` is an implementation of an integrate-and-fire neuron model
with delta-shaped postsynaptic currents used as readout neuron for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ``_bsshslm_2020`` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.

The membrane voltage time course :math:`v_j^t` of the neuron :math:`j` is given by:

.. math::
  v_j^t &= \kappa v_j^{t-1} + \zeta \sum_{i \neq j} W_{ji}^\text{out} z_i^{t-1} \,, \\
  \kappa &= e^{ -\frac{ \Delta t }{ \tau_\text{m} } } \,, \\
  \zeta &=
    \begin{cases}
      1 \\
      1 - \kappa
    \end{cases} \,, \\

where :math:`W_{ji}^\text{out}` is the output synaptic weight matrix and
:math:`z_i^{t-1}` is the recurrent presynaptic spike state variable.

Descriptions of further parameters and variables can be found in the table below.

The spike state variable of a presynaptic neuron is expressed by a Heaviside function:

.. math::
  z_i^t = H \left( v_i^t - v_\text{th} \right) \,. \\

An additional state variable and the corresponding differential equation
represents a piecewise constant external current.

See the documentation on the :doc:`iaf_psc_delta<../models/iaf_psc_delta/>` neuron model
for more information on the integration of the subthreshold dynamics.

The change of the synaptic weight is calculated from the gradient :math:`g` of
the loss :math:`E` with respect to the synaptic weight :math:`W_{ji}`:
:math:`\frac{ \text{d}E }{ \text{d} W_{ij} }`
which depends on the presynaptic
spikes :math:`z_i^{t-1}` and the learning signal :math:`L_j^t` emitted by the readout
neurons.

.. math::
  \frac{ \text{d} E }{ \text{d} W_{ji} } = \sum_t L_j^t \bar{z}_i^{t-1} \,. \\

The presynaptic spike trains are low-pass filtered with the following exponential kernel:

.. math::
  \bar{z}_i^t &=\mathcal{F}_\kappa(z_i^t) \,, \\
  \mathcal{F}_\kappa(z_i^t) &= \kappa \mathcal{F}_\kappa \left( z_i^{t-1} \right) + z_i^t \,, \\
  \mathcal{F}_\kappa(z_i^0) &= z_i^0 \,. \\

Since readout neurons are leaky integrators without a spiking mechanism, the
formula for computing the gradient lacks the surrogate gradient /
pseudo-derivative and a firing regularization term.

The learning signal :math:`L_j^t` is given by the non-plastic feedback weight
matrix :math:`B_{jk}` and the continuous error signal :math:`e_k^t` emitted by
readout neuron :math:`k`:

.. math::
  L_j^t = B_{jk} e_k^t \,. \\

The error signal depends on the selected loss function.
If a mean squared error loss is selected, then:

.. math::
  e_k^t = y_k^t - y_k^{*,t} \,, \\

where the readout signal :math:`y_k^t` corresponds to the membrane voltage of
readout neuron :math:`k` and :math:`y_k^{*,t}` is the real-valued target signal.

If a cross-entropy loss is selected, then:

.. math::
  e^k_t &= \pi_k^t - \pi_k^{*,t} \,, \\
  \pi_k^t &= \text{softmax}_k \left( y_1^t, ..., y_K^t \right) =
    \frac{ \exp \left( y_k^t\right) }{ \sum_{k'} \exp \left( y_{k'}^t \right) } \,, \\

where the readout signal :math:`\pi_k^t` corresponds to the softmax of the
membrane voltage of readout neuron :math:`k` and :math:`\pi_k^{*,t}` is the
one-hot encoded target signal.

Furthermore, the readout and target signal are zero before the onset of the
learning window in each update interval.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_bsshslm_2020<../models/eprop_iaf_bsshslm_2020/>`
 * :doc:`eprop_iaf_adapt_bsshslm_2020<../models/eprop_iaf_adapt_bsshslm_2020/>`
 * :doc:`eprop_synapse_bsshslm_2020<../models/eprop_synapse_bsshslm_2020/>`
 * :doc:`eprop_learning_signal_connection_bsshslm_2020<../models/eprop_learning_signal_connection_bsshslm_2020/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

========================= ======= ===================== ================== =====================================
**Neuron parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                 Unit    Math equivalent       Default            Description
========================= ======= ===================== ================== =====================================
``C_m``                   pF      :math:`C_\text{m}`                 250.0 Capacitance of the membrane
``E_L``                   mV      :math:`E_\text{L}`                   0.0 Leak / resting membrane potential
``I_e``                   pA      :math:`I_\text{e}`                   0.0 Constant external input current
``regular_spike_arrival`` Boolean                                 ``True`` If ``True``, the input spikes arrive
                                                                           at the end of the time step, if
                                                                           ``False`` at the beginning
                                                                           (determines PSC scale)
``tau_m``                 ms      :math:`\tau_\text{m}`               10.0 Time constant of the membrane
``V_min``                 mV      :math:`v_\text{min}`  negative maximum   Absolute lower bound of the membrane
                                                        value              voltage
                                                        representable by a
                                                        ``double`` type in
                                                        C++
========================= ======= ===================== ================== =====================================

========== ======= ===================== ==================== =========================================
**E-prop parameters**
-------------------------------------------------------------------------------------------------------
Parameter  Unit    Math equivalent       Default              Description
========== ======= ===================== ==================== =========================================
``loss``           :math:`E`             "mean_squared_error" Loss function
                                                              ["mean_squared_error", "cross_entropy"]
========== ======= ===================== ==================== =========================================

Recordables
+++++++++++

The following state variables evolve during simulation and can be recorded.

=============== ==== =============== ============= ================
**Neuron state variables and recordables**
-------------------------------------------------------------------
State variable  Unit Math equivalent Initial value Description
=============== ==== =============== ============= ================
``V_m``         mV   :math:`v_j`               0.0 Membrane voltage
=============== ==== =============== ============= ================

========================= ==== =============== ============= ===============================
**E-prop state variables and recordables**
--------------------------------------------------------------------------------------------
State variable            Unit Math equivalent Initial value Description
========================= ==== =============== ============= ===============================
``error_signal``          mV   :math:`L_j`               0.0 Error signal
``readout_signal``        mV   :math:`y_j`               0.0 Readout signal
``readout_signal_unnorm`` mV                             0.0 Unnormalized readout signal
``target_signal``         mV   :math:`y^*_j`             0.0 Target signal
========================= ==== =============== ============= ===============================

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

Sends
+++++

LearningSignalConnectionEvent, DelayedRateConnectionEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DelayedRateConnectionEvent, DataLoggingRequest

See also
++++++++

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: eprop_readout_bsshslm_2020

EndUserDocs */

void register_eprop_readout_bsshslm_2020( const std::string& name );

/**
 * @brief Class implementing a readout neuron model for e-prop plasticity.
 *
 * Class implementing a current-based leaky integrate readout neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al. (2020).
 */
class eprop_readout_bsshslm_2020 : public EpropArchivingNodeReadout< true >
{

public:
  //! Default constructor.
  eprop_readout_bsshslm_2020();

  //! Copy constructor.
  eprop_readout_bsshslm_2020( const eprop_readout_bsshslm_2020& );

  using Node::handle;
  using Node::handles_test_event;

  using Node::sends_secondary_event;

  void
  sends_secondary_event( LearningSignalConnectionEvent& ) override
  {
  }

  void
  sends_secondary_event( DelayedRateConnectionEvent& ) override
  {
  }

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DelayedRateConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DelayedRateConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  double compute_gradient( std::vector< long >& presyn_isis,
    const long t_previous_update,
    const long t_previous_trigger_spike,
    const double kappa,
    const bool average_gradient ) override;

  long get_shift() const override;
  bool is_eprop_recurrent_node() const override;

  //! Compute the error signal based on the mean-squared error loss.
  void compute_error_signal_mean_squared_error( const long lag );

  //! Compute the error signal based on the cross-entropy loss.
  void compute_error_signal_cross_entropy( const long lag );

  //! Compute the error signal based on a loss function.
  void ( eprop_readout_bsshslm_2020::*compute_error_signal )( const long lag );

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_readout_bsshslm_2020 >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_readout_bsshslm_2020 >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Leak / resting membrane potential (mV).
    double E_L_;

    //! Constant external input current (pA).
    double I_e_;

    //! Loss function ["mean_squared_error", "cross_entropy"].
    std::string loss_;

    //! If True, the input spikes arrive at the beginning of the time step, if False at the end (determines PSC scale).
    bool regular_spike_arrival_;

    //! Time constant of the membrane (ms).
    double tau_m_;

    //! Absolute lower bound of the membrane voltage relative to the leak membrane potential (mV).
    double V_min_;

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
    //! Error signal. Deviation between the readout and the target signal.
    double error_signal_;

    //! Readout signal. Leaky integrated spikes emitted by the recurrent network.
    double readout_signal_;

    //! Unnormalized readout signal. Readout signal not yet divided by the readout signals of other readout neurons.
    double readout_signal_unnorm_;

    //! Target / teacher signal that the network is supposed to learn.
    double target_signal_;

    //! Input current (pA).
    double i_in_;

    //! Membrane voltage relative to the leak membrane potential (mV).
    double v_m_;

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
    Buffers_( eprop_readout_bsshslm_2020& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_readout_bsshslm_2020& );

    //! Normalization rate of the readout signal. Sum of the readout signals of all readout neurons.
    double normalization_rate_;

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Logger for universal data.
    UniversalDataLogger< eprop_readout_bsshslm_2020 > logger_;
  };

  //! Structure of internal variables.
  struct Variables_
  {
    //! Propagator matrix entry for evolving the membrane voltage (mathematical symbol "kappa" in user documentation).
    double P_v_m_;

    //! Propagator matrix entry for evolving the incoming spike state variables (mathematical symbol "zeta" in user
    //! documentation).
    double P_z_in_;

    //! Propagator matrix entry for evolving the incoming currents.
    double P_i_in_;

    //! If the loss requires communication between the readout neurons and thus a buffer for the exchanged signals.
    bool signal_to_other_readouts_;
  };

  //! Minimal spike receptor type. Start with 1 to forbid port 0 and avoid accidental creation of connections with no
  //! receptor type set.
  static const size_t MIN_RATE_RECEPTOR = 1;

  //! Enumeration of spike receptor types.
  enum RateSynapseTypes
  {
    READOUT_SIG = MIN_RATE_RECEPTOR,
    TARGET_SIG,
    SUP_RATE_RECEPTOR
  };

  //! Get the current value of the membrane voltage.
  double
  get_v_m_() const
  {
    return S_.v_m_ + P_.E_L_;
  }

  //! Get the current value of the normalized readout signal.
  double
  get_readout_signal_() const
  {
    return S_.readout_signal_;
  }

  //! Get the current value of the unnormalized readout signal.
  double
  get_readout_signal_unnorm_() const
  {
    return S_.readout_signal_unnorm_;
  }

  //! Get the current value of the target signal.
  double
  get_target_signal_() const
  {
    return S_.target_signal_;
  }

  //! Get the current value of the error signal.
  double
  get_error_signal_() const
  {
    return S_.error_signal_;
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
  static RecordablesMap< eprop_readout_bsshslm_2020 > recordablesMap_;
};

inline long
eprop_readout_bsshslm_2020::get_shift() const
{
  return offset_gen_ + delay_in_rec_ + delay_rec_out_;
}

inline bool
eprop_readout_bsshslm_2020::is_eprop_recurrent_node() const
{
  return false;
}

inline size_t
eprop_readout_bsshslm_2020::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout_bsshslm_2020::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout_bsshslm_2020::handles_test_event( DelayedRateConnectionEvent& e, size_t receptor_type )
{
  size_t step_rate_model_id = kernel::manager< ModelManager >.get_node_model_id( "step_rate_generator" );
  size_t model_id = e.get_sender().get_model_id();

  if ( step_rate_model_id == model_id and receptor_type != TARGET_SIG )
  {
    throw IllegalConnection(
      "eprop_readout_bsshslm_2020 neurons expect a connection with a step_rate_generator node through receptor_type "
      "2." );
  }

  if ( receptor_type < MIN_RATE_RECEPTOR or receptor_type >= SUP_RATE_RECEPTOR )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return receptor_type;
}

inline size_t
eprop_readout_bsshslm_2020::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_readout_bsshslm_2020::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  DictionaryDatum receptor_dict_ = new Dictionary();
  ( *receptor_dict_ )[ names::readout_signal ] = READOUT_SIG;
  ( *receptor_dict_ )[ names::target_signal ] = TARGET_SIG;

  ( *d )[ names::receptor_types ] = receptor_dict_;
}

inline void
eprop_readout_bsshslm_2020::set_status( const DictionaryDatum& d )
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
void RecordablesMap< eprop_readout_bsshslm_2020 >::create();

} // namespace nest

#endif // EPROP_READOUT_BSSHSLM_2020_H
