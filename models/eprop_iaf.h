/*
 *  eprop_iaf.h
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

#ifndef EPROP_IAF_H
#define EPROP_IAF_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node.h"
#include "eprop_archiving_node_impl.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based, integrate-and-fire

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron model with delta-shaped
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_iaf`` is an implementation of a leaky integrate-and-fire
neuron model with delta-shaped postsynaptic currents used for eligibility
propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ```` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.

.. note::
  The neuron dynamics of the ``eprop_iaf`` model (excluding e-prop
  plasticity) are similar to the neuron dynamics of the ``iaf_psc_delta`` model,
  with minor differences, such as the propagator of the post-synaptic current
  and the voltage reset upon a spike.

The membrane voltage time course is given by:

.. math::
    v_j^t &= \alpha v_j^{t-1}+\sum_{i \neq j}W_{ji}^\mathrm{rec}z_i^{t-1}
             + \sum_i W_{ji}^\mathrm{in}x_i^t-z_j^{t-1}v_\mathrm{th} \,, \\
    \alpha &= e^{-\frac{\delta t}{\tau_\mathrm{m}}} \,.

The spike state variable is expressed by a Heaviside function:

.. math::
    z_j^t = H\left(v_j^t-v_\mathrm{th}\right) \,.

If the membrane voltage crosses the threshold voltage :math:`v_\text{th}`, a spike is
emitted and the membrane voltage is reduced by :math:`v_\text{th}` in the next
time step. After the time step of the spike emission, the neuron is not
able to spike for an absolute refractory period :math:`t_\text{ref}`.

An additional state variable and the corresponding differential equation
represents a piecewise constant external current.

Furthermore, the pseudo derivative of the membrane voltage needed for e-prop
plasticity is calculated:

.. math::
    \psi_j^t = \frac{\gamma}{v_\text{th}} \text{max}
               \left(0, 1-\left| \frac{v_j^t-v_\mathrm{th}}{v_\text{th}}\right| \right) \,.

See the documentation on the ``iaf_psc_delta`` neuron model for more information
on the integration of the subthreshold dynamics.

The change of the synaptic weight is calculated from the gradient
:math:`\frac{\mathrm{d}{E}}{\mathrm{d}{W_{ij}}}=g`
which depends on the presynaptic
spikes :math:`z_i^{t-1}`, the surrogate-gradient / pseudo-derivative of the postsynaptic membrane
voltage :math:`\psi_j^t` (which together form the eligibility trace
:math:`e_{ji}`), and the learning signal :math:`L_j^t` emitted by the readout
neurons.

.. math::
  \frac{\mathrm{d}E}{\mathrm{d}W_{ji}} = g &= \sum_t L_j^t \bar{e}_{ji}^t, \\
   e_{ji}^t &= \psi^t_j \bar{z}_i^{t-1}\,, \\

The eligibility trace and the presynaptic spike trains are low-pass filtered
with some exponential kernels:

.. math::
  \bar{e}_{ji}=\mathcal{F}_\kappa(e_{ji}) \;\text{with}\, \kappa=\exp\left(\frac{-\delta t}{
    \tau_\text{m,out}}\right)\,,\\ \bar{z}_i=\mathcal{F}_\alpha(z_i) \;\text{with}\,
    \alpha=\exp\left(\frac{-\delta t}{\tau_\text{m}}\right)\,.

Furthermore, a firing rate regularization mechanism keeps the average firing
rate :math:`f^\text{av}_j` of the postsynaptic neuron close to a target firing rate
:math:`f^\text{target}`:

.. math::
  \frac{\mathrm{d}E^\text{reg}}{\mathrm{d}W_{ji}} = g^\text{reg} = c_\text{reg}
  \sum_t \frac{1}{Tn_\text{trial}} \left( f^\text{target}-f^\text{av}_j\right)e_{ji}^t\,,

whereby :math:`c_\text{reg}` scales the overall regularization and the average
is taken over the time that passed since the previous update, that is, the number of
trials :math:`n_\text{trial}` times the duration of an update interval :math:`T`.

The overall gradient is given by the addition of the two gradients.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_adapt<../models/eprop_iaf_adapt/>`
 * :doc:`eprop_readout<../models/eprop_readout/>`
 * :doc:`eprop_synapse<../models/eprop_synapse/>`
 * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========================== ==== ======================= ================ ======================================
**Neuron parameters**
----------------------------------------------------------------------------------------------------------------
Parameter                   Unit Math equivalent         Default          Description
=========================== ==== ======================= ================ ======================================
C_m                         pF   :math:`C_\text{m}`                 250.0 Capacitance of the membrane
c_reg                            :math:`c_\text{reg}`                 0.0 Prefactor of firing rate
                                                                          regularization
E_L                         mV   :math:`E_\text{L}`                 -70.0 Leak membrane potential
f_target                    Hz   :math:`f^\text{target}`             10.0 Target firing rate of rate
                                                                          regularization
gamma                            :math:`\gamma`                       0.3 Scaling of surrogate gradient /
                                                                          pseudo-derivative of membrane voltage
I_e                         pA   :math:`I_\text{e}`                   0.0 Constant external input current
psc_scale_factor                                         alpha_complement Scale factor for presynaptic current
                                                                          ["alpha_complement": :math:`1 -
                                                                          \alpha`, "unity": :math:`1`]
surrogate_gradient_function      :math:`\psi`            piecewise_linear Surrogate gradient / pseudo-derivative
                                                                          function ["piecewise_linear"]
t_ref                       ms   :math:`t_\text{ref}`                 2.0 Duration of the refractory period
tau_m                       ms   :math:`\tau_\text{m}`               10.0 Time constant of the membrane
V_min                       mV   :math:`v_\text{min}`          -1.79e+308 Absolute lower bound of the membrane
                                                                          voltage
V_th                        mV   :math:`v_\text{th}`                -55.0 Spike threshold voltage
=========================== ==== ======================= ================ ======================================

The following state variables evolve during simulation.

================== ==== =============== ============= ==========================================================
**Neuron state variables and recordables**
----------------------------------------------------------------------------------------------------------------
State variable     Unit Math equivalent Initial value Description
================== ==== =============== ============= ==========================================================
learning_signal    pA   :math:`L_j`               0.0 Learning signal
surrogate_gradient      :math:`\psi_j`            0.0 Surrogate gradient / pseudo-derivative of membrane voltage
V_m                mV   :math:`v_j`             -70.0 Membrane voltage
================== ==== =============== ============= ==========================================================

Recordables
+++++++++++

The following variables can be recorded:

  - learning signal ``learning_signal``
  - membrane potential ``V_m``
  - surrogate gradient ``surrogate_gradient``

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

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, LearningSignalConnectionEvent, DataLoggingRequest

See also
++++++++

Examples using this model
++++++++++++++++++++++++++

.. listexamples:: eprop_iaf

EndUserDocs */

void register_eprop_iaf( const std::string& name );

/**
 * Class implementing a current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents for
 * e-prop plasticity according to Bellec et al (2020).
 */
class eprop_iaf : public EpropArchivingNodeRecurrent
{

public:
  //! Default constructor.
  eprop_iaf();

  //! Copy constructor.
  eprop_iaf( const eprop_iaf& );

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

  void compute_gradient( const long t_spike,
  const long t_prev_spike,
  long& t,
  double& prev_z_buffer,
  double& z_bar,
  double& e_bar,
  double& sum_e,
  double& grad,
  const double kappa);

  void pre_run_hook() override;
  bool is_eprop_recurrent_node() const override;
  void update( Time const&, const long, const long ) override;

protected:
  void init_buffers_() override;

private:
  //! Compute the piecewise linear surrogate gradient.
  double compute_piecewise_linear_derivative();

  //! Compute the surrogate gradient.
  double ( eprop_iaf::*compute_surrogate_gradient )();

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_iaf >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_iaf >;

  //! Structure of parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Prefactor of firing rate regularization.
    double c_reg_;

    //! Leak membrane potential (mV).
    double E_L_;

    //! Target firing rate of rate regularization (spikes/s).
    double f_target_;

    //! Scaling of surrogate-gradient / pseudo-derivative of membrane voltage.
    double gamma_;

    //! Constant external input current (pA).
    double I_e_;

    //! Scale factor for presynaptic current ["unity", "alpha_complement"]
    std::string psc_scale_factor_;

    //! Surrogate gradient / pseudo-derivative function ["piecewise_linear"].
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

    //! Binary spike variable - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
    double z_;

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
    Buffers_( eprop_iaf& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_iaf& );

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Logger for universal data.
    UniversalDataLogger< eprop_iaf > logger_;
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

  //!< Structure of parameters.
  Parameters_ P_;

  //!< Structure of state variables.
  State_ S_;

  //!< Structure of general variables.
  Variables_ V_;

  //!< Structure of buffers.
  Buffers_ B_;

  //! Map storing a static set of recordables.
  static RecordablesMap< eprop_iaf > recordablesMap_;
};

inline size_t
eprop_iaf::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
eprop_iaf::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf::handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_iaf::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
eprop_iaf::set_status( const DictionaryDatum& d )
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

#endif // EPROP_IAF_H
