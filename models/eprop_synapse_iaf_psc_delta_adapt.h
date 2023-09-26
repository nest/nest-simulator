/*
 *  eprop_synapse_iaf_psc_delta_adapt.h
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

#ifndef EPROP_SYNAPSE_IAF_PSC_DELTA_ADAPT_H
#define EPROP_SYNAPSE_IAF_PSC_DELTA_ADAPT_H

// nestkernel
#include "connection.h"
#include "eprop_synapse.h"

namespace nest
{

/* BeginUserDocs: synapse, e-prop-plasticity

Short description
+++++++++++++++++

Synapse type for e-prop plasticity

Description
+++++++++++

``eprop_synapse`` is a connector to create e-prop synapses between postsynaptic
neurons :math:`j` and presynaptic neurons and :math:`i` as defined in [1]_.  The
change of the synaptic weight :math:`\Delta W_{ji}` depends on the presynaptic
spikes :math:`z_i^{t-1}`, the pseudo-derivative of the postsynaptic membrane
voltage :math:`\psi_j^t` (which together form the eligibility trace
:math:`e_{ji}`), and the learning signal :math:`L_j^t` emitted by the readout
neurons. The eligibility trace and the presynaptic spike trains are
low-pass-filtered with kernels :math:`\mathcal{F}_\kappa` with
:math:`\kappa=\exp\left(\frac{-\delta t}{\tau_\text{out}}\right)` and
:math:`\mathcal{F}_\alpha` with
:math:`\kappa=\exp\left(\frac{-\delta t}{\tau_\text{m, out}}\right)`.
The overall weight update is scaled by the learning rate :math:`\eta`. The
general formula computing weight updates for eprop synapses projecting onto
recurrent neurons are thus given by:

.. math::
  \Delta W_{ji}^\text{rec} &= -\eta \sum_t L_j^t \bar{e}_{ji}^t \\
   &= -\eta \sum_t L_j^t \mathcal{F}_\kappa \left( \psi^t_j \bar{z}_i^{t-1}\right) \\
   &= -\eta \sum_t L_j^t \sum_{t'\leq t} \kappa^{t-t'} \psi^t_j \mathcal{F}_\alpha\left( z_i^{t-1}\right)\,.

If the postsynaptic neuron is a adaptive neuron, next to the membrane voltage, a
second hidden state variable, the threshold adaptation, is present, which
changes the eligibility trace:

.. math::
  e_{ji}^t &= \psi_j^t \left(\bar{z}^{t-1} - \beta \epsilon_{ji,a}^{t-1}\right)\,, \\
  \epsilon^{t-1}_{ji,\text{a}} &= \psi_j^{t-1}\bar{z}_i^{t-2} + \left( \rho - \psi_j^{t-1} \beta \right)
  \epsilon^{t-2}_{ji,a}\,, \\ \rho &= \exp\left(-\frac{\delta t}{\tau_\text{a}}\right)\,.

Furthermore, a firing rate regularization mechanism keeps the average firing
rate :math:`f^\text{av}_j` of the postsynaptic neuron close to a target firing rate
:math:`f^\text{target}`.

.. math::
  \Delta W_{ji}^\text{reg} = \eta c_\text{reg}
  \sum_t \frac{1}{Tn_\text{trial}} \left( f^\text{target}-f^\text{av}_j\right)e_{ji}^t,

whereby :math:`c_\text{reg}` scales the overall regularization and the average
is taken over the time that passed since the last update, i.e., the number of
trials :math:`n_\text{trials}` times the duration of an update interval :math:`T`.

The overall recurrent weight update is given by adding :math:`\Delta W_{ji}^\text{rec}`
and :math:`\Delta W_{ji}^\text{reg}`.

Since readout neurons :math:`k` are leaky integrators without a spiking
mechanism, the formula for computing weight updates for synapses projecting onto
readout neurons lacks the pseudo derivative and a firing regularization term.

.. math::
  \Delta W_{kj}^\text{out} = -\eta \sum_t L_j^t \mathcal{F}_\kappa \left(z_j^t\right).

The weights can also be optimized with the Adam scheme [2]_:

.. math::
  m_0 &= 0, v_0 = 0, t = 1 \\
  m_t &= \beta_1 \cdot m_{t-1} + \left(1-\beta_1\right) \cdot g_t \\
  v_t &= \beta_2 \cdot v_{t-1} + \left(1-\beta_2\right) \cdot g_t^2 \\
  \hat{m}_t &= \frac{m_t}{1-\beta_1^t} \\
  \hat{v}_t &= \frac{v_t}{1-\beta_2^t} \\
  \Delta W &= - \eta\frac{\hat{m_t}}{\sqrt{\hat{v}_t} + \epsilon}

E-prop synapses require archiving of continuous quantities. Therefore e-prop
synapses can only be connected to neuron models that are capable of doing this
archiving. So far, compatible models are ``eprop_iaf_psc_delta``,
``eprop_iaf_psc_delta_adapt``, and ``eprop_readout``.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

    * :doc:`eprop_iaf_psc_delta<../models/eprop_iaf_psc_delta/>`
    * :doc:`eprop_readout<../models/eprop_readout/>`
    * :doc:`eprop_synapse<../models/eprop_synapse/>`
    * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [3]_.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

===============  ========  ================  =======  =======================================================
**Common synapse properties**
-------------------------------------------------------------------------------------------------------------
Parameter        Unit      Math equivalent   Default  Description
===============  ========  ================  =======  =======================================================
adam             boolean                     False    If True, use Adam optimizer, if False, gradient descent
adam_beta1                 :math:`\beta_1`   0.9      Beta1 parameter of Adam optimizer
adam_beta2                 :math:`\beta_2`   0.999    Beta2 parameter of Adam optimizer
adam_epsilon               :math:`\epsilon`  1e-8     Epsilon parameter of Adam optimizer
batch_size                                   1        Size of batch
recall_duration  ms                          1.0      Duration over which gradients are averaged
===============  ========  ================  =======  =======================================================


=============  ====  =========================  =======  ===============================================================
**Individual synapse properties**
------------------------------------------------------------------------------------------------------------------------
Parameter      Unit  Math equivalent            Default  Description
=============  ====  =========================  =======  ===============================================================
adam_m               :math:`m`                      0.0  Initial value of first moment estimate m of Adam optimizer
adam_v               :math:`v`                      0.0  Initial value of second moment raw estimate v of Adam optimizer
delay          ms    :math:`d_{ji}`                 1.0  Dendritic delay
eta                  :math:`\eta`                  1e-4  Learning rate
tau_m_readout  ms    :math:`\tau_\text{m,out}`      0.0  Time constant for low-pass filtering of eligibility trace
weight         pA    :math:`W_{ji}`                 1.0  Synaptic weight
Wmax           pA    :math:`W_{ji}^\text{max}`    100.0  Maximal value for synaptic weight
Wmin           pA    :math:`W_{ji}^\text{min}`      0.0  Minimal value for synaptic weight
=============  ====  =========================  =======  ===============================================================

Recordables
+++++++++++

The following variables can be recorded.

  - ``weight``

Usage
+++++

This model can only be used in combination with the other e-prop models,
whereby the network architecture requires specific wiring, input, and output.
The usage is demonstrated in a
:doc:`supervised regression task <../auto_examples/eprop_plasticity/eprop_supervised_regression/>`
and a :doc:`supervised classification task <../auto_examples/eprop_plasticity/eprop_supervised_classification>`,
reproducing the original proof-of-concept tasks in [1]_.

Transmits
+++++++++

SpikeEvent, DSSpikeEvent

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] Kingma DP, Ba JL (2015). Adam: A method for stochastic optimization.
       Proceedings of International Conference on Learning Representations (ICLR).
       https://doi.org/10.48550/arXiv.1412.6980

.. [3] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
       van Albada SJ, Bolten M, Diesmann M. Event-based implementation of
       eligibility propagation (in preparation)

See also
++++++++

EndUserDocs */

template < typename targetidentifierT >
class eprop_synapse_iaf_psc_delta_adapt : public eprop_synapse< targetidentifierT >
{

public:
  typedef EpropCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  using ConnectionBase::get_delay;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  void update_gradient( EpropArchivingNode* target,
    double& sum_grads,
    std::vector< double >& presyn_isis,
    const EpropCommonProperties& cp ) const override;
  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );
  void check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& ) override;
  double get_shift() const override;
  bool do_update( const double& t_spike ) const override;
};

template < typename targetidentifierT >
void
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  eprop_synapse< targetidentifierT >::get_status( d );
}

template < typename targetidentifierT >
void
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  eprop_synapse< targetidentifierT >::set_status( d, cm );
}

template < typename targetidentifierT >
void
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::check_connection( Node& s,
  Node& t,
  size_t receptor_type,
  const CommonPropertiesType& )
{
  typename eprop_synapse< targetidentifierT >::ConnTestDummyNode dummy_target;
  ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

  EpropArchivingNode& t_arch = dynamic_cast< EpropArchivingNode& >( t );
  t_arch.init_update_history( 2.0 * get_delay() );
}


template < typename targetidentifierT >
void
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::update_gradient( EpropArchivingNode* target,
  double& sum_grads,
  std::vector< double >& presyn_isis,
  const EpropCommonProperties& cp ) const
{
  std::deque< HistEntryEpropArchive >::iterator it_eprop_hist;
  target->get_eprop_history( this->t_last_trigger_spike_ + get_delay(), &it_eprop_hist );

  std::map< std::string, double >& eprop_parameter_map = target->get_eprop_parameter_map();
  double alpha = eprop_parameter_map[ "leak_propagator" ];
  double alpha_complement = eprop_parameter_map[ "leak_propagator_complement" ];
  double beta = eprop_parameter_map[ "adapt_beta" ];
  double rho = eprop_parameter_map[ "adapt_propagator" ];

  double sum_t_prime = 0.0;
  double sum_e_bar = 0.0;
  double last_z_bar = 0.0;
  double epsilon = 0.0;
  double grad = 0.0;

  for ( auto presyn_isi : presyn_isis )
  {
    last_z_bar += alpha_complement;
    for ( int t = 0; t < presyn_isi; ++t )
    {
      double psi = it_eprop_hist->V_m_pseudo_deriv_;
      double e_bar = psi * last_z_bar;

      e_bar -= psi * beta * epsilon;
      epsilon = psi * last_z_bar + ( rho - psi * beta ) * epsilon;
      sum_t_prime = this->kappa_ * sum_t_prime + ( 1.0 - this->kappa_ ) * e_bar;
      grad += sum_t_prime * this->dt_ * it_eprop_hist->learning_signal_;
      sum_e_bar += e_bar;
      last_z_bar *= alpha;
      ++it_eprop_hist;
    }
  }
  presyn_isis.clear();

  grad /= Time( Time::ms( cp.recall_duration_ ) ).get_steps();

  grad += target->get_firing_rate_reg( this->t_last_update_ ) * sum_e_bar;

  grad *= this->dt_;

  sum_grads += grad;
}

template < typename targetidentifierT >
double
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::get_shift() const
{
  return 0.0;
}

template < typename targetidentifierT >
bool
eprop_synapse_iaf_psc_delta_adapt< targetidentifierT >::do_update( const double& t_spike ) const
{
  bool is_update = std::fmod( t_spike - this->delay_, this->update_interval_ ) != 0.0;
  return is_update;
}

} // namespace nest

#endif // EPROP_SYNAPSE_IAF_PSC_DELTA_ADAPT_H
