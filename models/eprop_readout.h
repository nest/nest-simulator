/*
 *  eprop_readout.h
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

#ifndef EPROP_READOUT_H
#define EPROP_READOUT_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node_readout.h"
#include "eprop_synapse.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

#include <model_manager.h>

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based

Short description
+++++++++++++++++

Current-based leaky integrate readout neuron model with delta-shaped
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_readout`` is an implementation of an integrate-and-fire neuron model
with delta-shaped postsynaptic currents used as readout neuron for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The membrane voltage time course :math:`v_j^t` of the neuron :math:`j` is given by:

.. math::
  v_j^t &= \kappa v_j^{t-1}+ \zeta \sum_{i \neq j} W_{ji}^\text{out} z_i^{t-1} \,, \\
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

The change of the synaptic weight is calculated from the gradient :math:`g^t` of
the loss :math:`E^t` with respect to the synaptic weight :math:`W_{ji}`:
:math:`\frac{ \text{d} E^t }{ \text{d} W_{ij} }`
which depends on the presynaptic
spikes :math:`z_i^{t-1}` and the learning signal :math:`L_j^t` emitted by the readout
neurons.

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
  \frac{ \text{d} E^t }{ \text{d} W_{ji} } = L_j^t \bar{z}_i^{t-1} \,. \\

The presynaptic spike trains are low-pass filtered with the following exponential kernel:

.. math::
  \bar{z}_i^t = \mathcal{F}_\kappa \left( z_{i}^t \right)
    = \kappa \bar{z}_i^{t-1} + \zeta z_i^t \,. \\

Since readout neurons are leaky integrators without a spiking mechanism, the
formula for computing the gradient lacks the surrogate gradient /
pseudo-derivative and a firing regularization term.

As a last step for every round in the loop over the time steps :math:`t`, the new weight is retrieved by feeding the
current gradient :math:`g^t` to the optimizer (see :doc:`weight_optimizer<../models/weight_optimizer/>`
for more information on the available optimizers):

.. math::
  w^t = \text{optimizer} \left( t, g^t, w^{t-1} \right) \,. \\

After the loop has terminated, the filtered dynamic variables of e-prop are propagated from the end of the cutoff until
the next spike:

.. math::
  p &= \text{max} \left( 0, t_\text{s}^{t} - \left( t_\text{s}^{t-1} + {\Delta t}_\text{c} \right) \right) \,, \\
  \bar{z}_i^{t+p} &= \bar{z}_i^t \alpha^p \,. \\

The learning signal :math:`L_j^t` is given by the non-plastic feedback weight
matrix :math:`B_{jk}` and the continuous error signal :math:`e_k^t` emitted by
readout neuron :math:`k` and :math:`e_k^t` defined via a mean-squared error
loss:

.. math::
  L_j^t = B_{jk} e_k^t = B_{jk} \left( y_k^t - y_k^{*,t} \right) \,. \\

where the readout signal :math:`y_k^t` corresponds to the membrane voltage of
readout neuron :math:`k` and :math:`y_k^{*,t}` is the real-valued target signal.

Furthermore, the readout and target signal are multiplied by a learning window
signal, which has a value of 1.0 within the learning window and 0.0 outside.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf<../models/eprop_iaf/>`
 * :doc:`eprop_iaf_adapt<../models/eprop_iaf_adapt/>`
 * :doc:`eprop_synapse<../models/eprop_synapse/>`
 * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

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
``tau_m``                 ms      :math:`\tau_\text{m}`               10.0 Time constant of the membrane
``V_min``                 mV      :math:`v_\text{min}`  negative maximum   Absolute lower bound of the membrane
                                                        value              voltage
                                                        representable by a
                                                        ``double`` type in
                                                        C++
========================= ======= ===================== ================== =====================================

=========================== ======= =========================== ================ ===============================
**E-prop parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                   Unit    Math equivalent             Default          Description
=========================== ======= =========================== ================ ===============================
``eprop_isi_trace_cutoff``  ms      :math:`{\Delta t}_\text{c}` maximum value    Cutoff for integration of
                                                                representable    e-prop update between two
                                                                by a ``long``    spikes
                                                                type in C++
=========================== ======= =========================== ================ ===============================

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

========================= ==== =============== ============= ==============
**E-prop state variables and recordables**
---------------------------------------------------------------------------
State variable            Unit Math equivalent Initial value Description
========================= ==== =============== ============= ==============
``error_signal``          mV   :math:`L_j`               0.0 Error signal
``readout_signal``        mV   :math:`y_j`               0.0 Readout signal
``target_signal``         mV   :math:`y^*_j`             0.0 Target signal
========================= ==== =============== ============= ==============

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

.. listexamples:: eprop_readout

EndUserDocs */

void register_eprop_readout( const std::string& name );

/**
 * @brief Class implementing a readout neuron model for e-prop plasticity with additional biological features.
 *
 * Class implementing a current-based leaky integrate readout neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al. (2020) with additional biological features described in
 * Korcsak-Gorzo, Stapmanns, and Espinoza Valverde et al. (in preparation).
 */
class eprop_readout : public EpropArchivingNodeReadout< false >
{

public:
  //! Default constructor.
  eprop_readout();

  //! Copy constructor.
  eprop_readout( const eprop_readout& );

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
  friend class RecordablesMap< eprop_readout >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_readout >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Leak / resting membrane potential (mV).
    double E_L_;

    //! Constant external input current (pA).
    double I_e_;

    //! Time constant of the membrane (ms).
    double tau_m_;

    //! Absolute lower bound of the membrane voltage relative to the leak membrane potential (mV).
    double V_min_;

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
    //! Error signal. Deviation between the readout and the target signal.
    double error_signal_;

    //! Readout signal. Leaky integrated spikes emitted by the recurrent network.
    double readout_signal_;

    //! Target / teacher signal that the network is supposed to learn.
    double target_signal_;

    //! Signal indicating whether the readout neurons are in a learning phase.
    double learning_window_signal_;

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
    Buffers_( eprop_readout& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_readout& );

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Logger for universal data.
    UniversalDataLogger< eprop_readout > logger_;
  };

  //! Structure of internal variables.
  struct Variables_
  {
    //! Propagator matrix entry for evolving the membrane voltage (mathematical symbol "kappa" in user documentation).
    double P_v_m_;

    //! Propagator matrix entry for evolving the incoming currents.
    double P_i_in_;

    //! Time steps from the previous spike until the cutoff of e-prop update integration between two spikes.
    long eprop_isi_trace_cutoff_steps_;
  };

  //! Minimal spike receptor type. Start with 1 to forbid port 0 and avoid accidental creation of connections with no
  //! receptor type set.
  static const size_t MIN_RATE_RECEPTOR = 1;

  //! Enumeration of spike receptor types.
  enum RateSynapseTypes
  {
    LEARNING_WINDOW_SIG = MIN_RATE_RECEPTOR,
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
  static RecordablesMap< eprop_readout > recordablesMap_;
};

inline long
eprop_readout::get_shift() const
{
  return offset_gen_ + delay_in_rec_;
}

inline bool
eprop_readout::is_eprop_recurrent_node() const
{
  return false;
}

inline long
eprop_readout::get_eprop_isi_trace_cutoff() const
{
  return V_.eprop_isi_trace_cutoff_steps_;
}

inline size_t
eprop_readout::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout::handles_test_event( DelayedRateConnectionEvent& e, size_t receptor_type )
{
  size_t step_rate_model_id = kernel::manager< ModelManager >.get_node_model_id( "step_rate_generator" );
  size_t model_id = e.get_sender().get_model_id();

  if ( step_rate_model_id == model_id and receptor_type != TARGET_SIG and receptor_type != LEARNING_WINDOW_SIG )
  {
    throw IllegalConnection(
      "eprop_readout neurons expect a connection with a step_rate_generator node through receptor_type "
      "1 or 2." );
  }

  if ( receptor_type < MIN_RATE_RECEPTOR or receptor_type >= SUP_RATE_RECEPTOR )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return receptor_type;
}

inline size_t
eprop_readout::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_readout::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  DictionaryDatum receptor_dict_ = new Dictionary();
  ( *receptor_dict_ )[ names::eprop_learning_window ] = LEARNING_WINDOW_SIG;
  ( *receptor_dict_ )[ names::target_signal ] = TARGET_SIG;

  ( *d )[ names::receptor_types ] = receptor_dict_;
}

inline void
eprop_readout::set_status( const DictionaryDatum& d )
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
void RecordablesMap< eprop_readout >::create();

} // namespace nest

#endif // EPROP_READOUT_H
