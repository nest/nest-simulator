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
#include "eprop_archiving_node.h"
#include "eprop_archiving_node_impl.h"
#include "eprop_synapse.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based

Short description
+++++++++++++++++

Current-based leaky integrate readout neuron model with delta-shaped
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_readout`` is an implementation of a integrate-and-fire neuron model
with delta-shaped postsynaptic currents used as readout neuron for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ```` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.


The membrane voltage time course is given by:

.. math::
    v_j^t &= \alpha v_j^{t-1}+\sum_{i \neq j}W_{ji}^\mathrm{out}z_i^{t-1}
             -z_j^{t-1}v_\mathrm{th} \,, \\
    \alpha &= e^{-\frac{\delta t}{\tau_\mathrm{m}}} \,.

An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

See the documentation on the ``iaf_psc_delta`` neuron model for more information
on the integration of the subthreshold dynamics.

The change of the synaptic weight is calculated from the gradient
:math:`\frac{\mathrm{d}{E}}{\mathrm{d}{W_{ij}}}=g`
which depends on the presynaptic
spikes :math:`z_i^{t-1}` and the learning signal :math:`L_j^t` emitted by the readout
neurons.

.. math::
  \frac{\mathrm{d}E}{\mathrm{d}W_{ji}} = g &= \sum_t L_j^t \bar{z}_i^{t-1}\,. \\

The presynaptic spike trains are low-pass filtered with an exponential kernel:

.. math::
  \bar{z}_i=\mathcal{F}_\kappa(z_i) \;\text{with}\, \kappa=\exp\left(\frac{-\delta t}{\tau_\text{m}}\right)\,.

Since readout neurons are leaky integrators without a spiking mechanism, the
formula for computing the gradient lacks the surrogate gradient /
pseudo-derivative and a firing regularization term.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf<../models/eprop_iaf/>`
 * :doc:`eprop_iaf_adapt<../models/eprop_iaf_adapt/>`
 * :doc:`eprop_synapse<../models/eprop_synapse/>`
 * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

===================== ======= ===================== ================== =========================================
**Neuron parameters**
----------------------------------------------------------------------------------------------------------------
Parameter             Unit    Math equivalent       Default            Description
===================== ======= ===================== ================== =========================================
 C_m                  pF      :math:`C_\text{m}`                 250.0 Capacitance of the membrane
 E_L                  mV      :math:`E_\text{L}`                   0.0 Leak membrane potential
 I_e                  pA      :math:`I_\text{e}`                   0.0 Constant external input current
 loss                         :math:`E`             mean_squared_error Loss function
                                                                       ["mean_squared_error", "cross_entropy"]
regular_spike_arrival Boolean                                     True If True, the input spikes arrive at the
                                                                       end of the time step, if False at the
                                                                       beginning (determines PSC scale)
 tau_m                ms      :math:`\tau_\text{m}`               10.0 Time constant of the membrane
 V_min                mV      :math:`v_\text{min}`          -1.79e+308 Absolute lower bound of the membrane
                                                                       voltage
===================== ======= ===================== ================== =========================================

The following state variables evolve during simulation.

===================== ==== =============== ============= ================
**Neuron state variables and recordables**
-------------------------------------------------------------------------
State variable        Unit Math equivalent Initial value Description
===================== ==== =============== ============= ================
error_signal          mV   :math:`L_j`               0.0 Error signal
readout_signal        mV   :math:`y_j`               0.0 Readout signal
target_signal         mV   :math:`y^*_j`             0.0 Target signal
V_m                   mV   :math:`v_j`               0.0 Membrane voltage
===================== ==== =============== ============= ================

Recordables
+++++++++++

The following variables can be recorded:

  - error signal ``error_signal``
  - readout signal ``readout_signal``
  - target signal ``target_signal``
  - membrane potential ``V_m``

Usage
+++++

This model can only be used in combination with the other e-prop models,
whereby the network architecture requires specific wiring, input, and output.
The usage is demonstrated in several
:doc:`supervised regression and classification tasks <../auto_examples/eprop_plasticity/index>`
reproducing among others the original proof-of-concept tasks in [1]_.

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y
.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
       van Albada SJ, Bolten M, Diesmann M. Event-based implementation of
       eligibility propagation (in preparation)

Sends
++++++++

LearningSignalConnectionEvent, DelayedRateConnectionEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DelayedRateConnectionEvent, DataLoggingRequest

See also
++++++++

Examples using this model
++++++++++++++++++++++++++

.. listexamples:: eprop_readout

EndUserDocs */

void register_eprop_readout( const std::string& name );

/**
 * Class implementing a current-based leaky integrate readout neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al. (2020).
 */
class eprop_readout : public EpropArchivingNodeReadout
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

  void compute_gradient( const long t_spike,
    const long t_previous_spike,
    double& previous_z_buffer,
    double& z_bar,
    double& e_bar,
    double& epsilon,
    double& avg_e,
    double& weight,
    const double kappa,
    const CommonSynapseProperties& cp,
    WeightOptimizer* optimizer ) override;

  void pre_run_hook() override;
  long get_shift() const override;
  bool is_eprop_recurrent_node() const override;
  void update( Time const&, const long, const long ) override;

  //! Get maximum number of time steps integrated between two consecutive spikes.
  long get_eprop_isi_trace_cutoff() override;

protected:
  void init_buffers_() override;

private:
  //! Compute the error signal based on the mean-squared error loss.
  void compute_error_signal_mean_squared_error( const long lag );

  //! Compute the error signal based on a loss function.
  void ( eprop_readout::*compute_error_signal )( const long lag );

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_readout >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_readout >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Leak membrane potential (mV).
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

    //!< Number of time steps integrated between two consecutive spikes is equal to the minimum between
    //!< eprop_isi_trace_cutoff_ and the inter-spike distance.
    long eprop_isi_trace_cutoff_;

    //! Learning rate.
    double eta_;

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

    //! Binary input spike variables - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
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

  //! Structure of general variables.
  struct Variables_
  {
    //! Propagator matrix entry for evolving the membrane voltage.
    double P_v_m_;

    //! Propagator matrix entry for evolving the incoming spike variables.
    double P_z_in_;

    //! Propagator matrix entry for evolving the incoming currents.
    double P_i_in_;
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

  //!< Structure of parameters.
  Parameters_ P_;

  //!< Structure of state variables.
  State_ S_;

  //!< Structure of general variables.
  Variables_ V_;

  //!< Structure of buffers.
  Buffers_ B_;

  //! Map storing a static set of recordables.
  static RecordablesMap< eprop_readout > recordablesMap_;
};

inline long
eprop_readout::get_eprop_isi_trace_cutoff()
{
  return P_.eprop_isi_trace_cutoff_;
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
  size_t step_rate_model_id = kernel().model_manager.get_node_model_id( "step_rate_generator" );
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

} // namespace nest

#endif // EPROP_READOUT_H
