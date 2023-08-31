/*
 *  eprop_synapse.h
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

#ifndef EPROP_SYNAPSE_H
#define EPROP_SYNAPSE_H

// nestkernel
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, e-prop plasticity

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

For more information on e-prop plasticity see the documentation on the other e-prop models,
:doc:`eprop_iaf_psc_delta<../models/eprop_iaf_psc_delta/>`,
:doc:`eprop_readout<../models/eprop_readout/>`, and
:doc:`eprop_synapse<../models/eprop_synapse/>`,
:doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`.

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


=========  ====  =========================  =======  ===============================================================
**Individual synapse properties**
--------------------------------------------------------------------------------------------------------------------
Parameter  Unit  Math equivalent            Default  Description
=========  ====  =========================  =======  ===============================================================
adam_m           :math:`m`                      0.0  Initial value of first moment estimate m of Adam optimizer
adam_v           :math:`v`                      0.0  Initial value of second moment raw estimate v of Adam optimizer
c_reg            :math:`c_\text{reg}`           0.0  Prefactor of firing rate regularization
delay      ms    :math:`d_{ji}`                 1.0  Dendritic delay
eta              :math:`\eta`                  1e-4  Learning rate
f_target   Hz    :math:`f^\text{target}`       10.0  Target firing rate of rate regularization
tau_m_out  ms    :math:`\tau_\text{m,out}`      0.0  Time constant for low-pass filtering of eligibility trace
weight     pA    :math:`W_{ji}`                 1.0  Synaptic weight
Wmax       pA    :math:`W_{ji}^\text{max}`    100.0  Maximal value for synaptic weight
Wmin       pA    :math:`W_{ji}^\text{min}`      0.0  Minimal value for synaptic weight
=========  ====  =========================  =======  ===============================================================

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

class EpropCommonProperties : public CommonSynapseProperties
{
public:
  EpropCommonProperties();

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  bool adam_;
  double adam_beta1_;
  double adam_beta2_;
  double adam_epsilon_;
  long batch_size_;
  double recall_duration_;
};

EpropCommonProperties::EpropCommonProperties()
  : CommonSynapseProperties()
  , adam_( false )
  , adam_beta1_( 0.9 )
  , adam_beta2_( 0.999 )
  , adam_epsilon_( 1e-8 )
  , batch_size_( 1 )
  , recall_duration_( 1.0 )
{
}

void
EpropCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< bool >( d, names::adam, adam_ );
  def< double >( d, names::adam_beta1, adam_beta1_ );
  def< double >( d, names::adam_beta2, adam_beta2_ );
  def< double >( d, names::adam_epsilon, adam_epsilon_ );
  def< long >( d, names::batch_size, batch_size_ );
  def< double >( d, names::recall_duration, recall_duration_ );
}

void
EpropCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );
  updateValue< bool >( d, names::adam, adam_ );
  updateValue< double >( d, names::adam_beta1, adam_beta1_ );
  updateValue< double >( d, names::adam_beta2, adam_beta2_ );
  updateValue< double >( d, names::adam_epsilon, adam_epsilon_ );
  updateValue< long >( d, names::batch_size, batch_size_ );
  updateValue< double >( d, names::recall_duration, recall_duration_ );
}

template < typename targetidentifierT >
class eprop_synapse : public Connection< targetidentifierT >
{

public:
  typedef EpropCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::REQUIRES_EPROP_ARCHIVING
    | ConnectionModelProperties::SUPPORTS_HPC | ConnectionModelProperties::SUPPORTS_LBL;

  eprop_synapse();

  eprop_synapse( const eprop_synapse& ) = default;
  eprop_synapse& operator=( const eprop_synapse& ) = default;

  using ConnectionBase::get_delay;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void send( Event& e, size_t thread, const EpropCommonProperties& cp );

  void optimize( long current_optimization_step_, long& last_optimization_step_, const EpropCommonProperties& cp );

  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    using ConnTestDummyNodeBase::handles_test_event;

    size_t
    handles_test_event( SpikeEvent&, size_t )
    {
      return invalid_port;
    }

    size_t
    handles_test_event( DSSpikeEvent&, size_t )
    {
      return invalid_port;
    }
  };

  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    double update_interval_ = kernel().simulation_manager.get_eprop_update_interval();
    double transmission_shift = 2.0 * get_delay(); // correct for travel time of learning signal to synchronize signals
    t_next_update_ = update_interval_ + transmission_shift;
    t_last_update_ = transmission_shift;

    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    int n_buffer_entries = t.get_eprop_node_type() == "readout" ? 3 : 2;
    t.init_eprop_buffers( n_buffer_entries * get_delay() );
    t.register_eprop_connection( t_last_spike_ - get_delay(), get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double weight_;
  double eta_;
  double Wmin_;
  double Wmax_;
  long last_optimization_step_;
  double t_last_spike_;
  double t_last_update_;
  double t_next_update_;
  double c_reg_;
  double f_target_;
  double tau_m_out_; // time constant for low pass filtering of eligibility trace
  double kappa_;     // exp( -dt / tau_m_out_ )
  double adam_m_;    // auxiliary variable for Adam optimizer
  double adam_v_;    // auxiliary variable for Adam optimizer
  double sum_grads_; // sum of the gradients in one batch

  std::vector< double > presyn_spike_times_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties eprop_synapse< targetidentifierT >::properties;

template < typename targetidentifierT >
inline void
eprop_synapse< targetidentifierT >::send( Event& e, size_t thread, const EpropCommonProperties& cp )
{
  double t_spike = e.get_stamp().get_ms();
  Node* target = get_target( thread );
  double dendritic_delay = get_delay();
  std::string target_node = target->get_eprop_node_type();

  double update_interval_ = kernel().simulation_manager.get_eprop_update_interval();

  if ( ( ( std::fmod( t_spike, update_interval_ ) - dendritic_delay ) != 0.0 ) or ( target_node == "readout" ) )
  {
    presyn_spike_times_.push_back( t_spike );

    if ( t_spike >= t_next_update_ )
    {
      std::deque< histentry_eprop >::iterator start;
      std::deque< histentry_eprop >::iterator finish;

      double const dt = Time::get_resolution().get_ms();
      double idx_current_update = floor( ( t_spike - dt ) / update_interval_ );
      double t_current_update_ = idx_current_update * update_interval_ + 2.0 * dendritic_delay;
      int current_optimization_step_ = 1 + ( int ) idx_current_update / cp.batch_size_;
      double grad = 0.0;
      double last_z_bar = 0.0;

      double shift = target_node == "readout" ? dendritic_delay : 0.0;

      presyn_spike_times_.insert( --presyn_spike_times_.end(), t_next_update_ - ( dendritic_delay - shift ) );
      target->get_eprop_history(
        presyn_spike_times_[ 0 ] + dendritic_delay, t_last_update_ + shift + update_interval_, &start, &finish );
      target->register_update( t_last_update_ + shift, t_current_update_ + shift );

      std::vector< double > presyn_isis( presyn_spike_times_.size() - 1 );
      std::adjacent_difference( presyn_spike_times_.begin(), --presyn_spike_times_.end(), presyn_isis.begin() );
      presyn_isis.erase( presyn_isis.begin() );

      if ( target_node == "readout" )
      {
        for ( auto presyn_isi : presyn_isis )
        {
          last_z_bar += 1.0 - kappa_;
          for ( int t = 0; t < presyn_isi; ++t )
          {
            grad += start->learning_signal_ * last_z_bar;
            last_z_bar *= kappa_;
            ++start;
          }
        }
      }
      else
      {
        double alpha = target->get_leak_propagator();
        double alpha_complement = target->get_leak_propagator_complement();
        double sum_t_prime = 0.0;
        double sum_e_bar = 0.0;

        double beta = 0.0;
        double rho = 0.0;
        double epsilon = 0.0;

        if ( target_node == "adaptive" )
        {
          beta = target->get_adapt_beta();
          rho = target->get_adapt_propagator();
        }

        for ( auto presyn_isi : presyn_isis )
        {
          last_z_bar += alpha_complement;
          for ( int t = 0; t < presyn_isi; ++t )
          {
            double psi = start->V_m_pseudo_deriv_;
            double e_bar = psi * last_z_bar;

            if ( target_node == "adaptive" )
            {
              e_bar -= psi * beta * epsilon;
              epsilon = psi * last_z_bar + ( rho - psi * beta ) * epsilon;
            }
            sum_t_prime = kappa_ * sum_t_prime + ( 1.0 - kappa_ ) * e_bar;
            grad += sum_t_prime * dt * start->learning_signal_;
            sum_e_bar += e_bar;
            last_z_bar *= alpha;
            ++start;
          }
        }

        grad /= Time( Time::ms( cp.recall_duration_ ) ).get_steps();

        // firing rate regularization
        std::deque< double >::iterator start_spike;
        std::deque< double >::iterator finish_spike;

        target->get_spike_history( t_last_update_, t_last_update_ + update_interval_, &start_spike, &finish_spike );

        int nspikes = std::distance( start_spike, finish_spike );
        double f_av = nspikes / update_interval_;
        double f_target = f_target_ / 1000.; // convert to kHz

        grad += c_reg_ * dt * ( f_av - f_target ) * sum_e_bar / update_interval_;
      }

      grad *= dt;

      sum_grads_ += grad;

      if ( last_optimization_step_ < current_optimization_step_ )
        optimize( current_optimization_step_, last_optimization_step_, cp );

      t_last_update_ = t_current_update_;
      t_next_update_ += ( floor( ( t_spike - t_next_update_ ) / update_interval_ ) + 1 ) * update_interval_;

      presyn_spike_times_.clear();
      presyn_spike_times_.push_back( t_spike );

      target->tidy_eprop_history();
      target->tidy_spike_history();
    }
  }

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  t_last_spike_ = t_spike;
}


template < typename targetidentifierT >
inline void
eprop_synapse< targetidentifierT >::optimize( long current_optimization_step_,
  long& last_optimization_step_,
  const EpropCommonProperties& cp )
{
  sum_grads_ /= cp.batch_size_; // mean over batches

  if ( cp.adam_ ) // adam optimizer, see Kingma and Lai Ba (2015)
  {
    for ( ; last_optimization_step_ < current_optimization_step_; ++last_optimization_step_ )
    {
      double adam_beta1_factor = 1.0 - std::pow( cp.adam_beta1_, last_optimization_step_ );
      double adam_beta2_factor = 1.0 - std::pow( cp.adam_beta2_, last_optimization_step_ );

      double alpha_t = eta_ * std::sqrt( adam_beta2_factor ) / adam_beta1_factor;

      adam_m_ = cp.adam_beta1_ * adam_m_ + ( 1.0 - cp.adam_beta1_ ) * sum_grads_;
      adam_v_ = cp.adam_beta2_ * adam_v_ + ( 1.0 - cp.adam_beta2_ ) * sum_grads_ * sum_grads_;

      weight_ -= alpha_t * adam_m_ / ( std::sqrt( adam_v_ ) + cp.adam_epsilon_ );

      sum_grads_ = 0.0; // reset for following iterations
      // since more than 1 cycle through loop indicates past learning periods with vanishing gradients
    }
  }
  else // gradient descent
  {
    weight_ -= eta_ * sum_grads_;
    last_optimization_step_ = current_optimization_step_;
  }

  if ( weight_ > Wmax_ )
    weight_ = Wmax_;
  if ( weight_ < Wmin_ )
    weight_ = Wmin_;

  sum_grads_ = 0.0;
}

template < typename targetidentifierT >
eprop_synapse< targetidentifierT >::eprop_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , eta_( 0.0001 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
  , last_optimization_step_( 1 )
  , t_last_spike_( 0.0 )
  , t_last_update_( 2.0 )
  , t_next_update_( 1002.0 )
  , c_reg_( 0.0 )
  , f_target_( 10.0 )
  , tau_m_out_( 10.0 )
  , kappa_( 0.0 )
  , adam_m_( 0.0 )
  , adam_v_( 0.0 )
  , sum_grads_( 0.0 )
{
}

template < typename targetidentifierT >
void
eprop_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::tau_m_out, tau_m_out_ );
  def< long >( d, names::size_of, sizeof( *this ) );
  def< double >( d, names::adam_m, adam_m_ );
  def< double >( d, names::adam_v, adam_v_ );
}

template < typename targetidentifierT >
void
eprop_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::eta, eta_ );
  updateValue< double >( d, names::Wmin, Wmin_ );
  updateValue< double >( d, names::Wmax, Wmax_ );
  updateValue< double >( d, names::c_reg, c_reg_ );
  updateValue< double >( d, names::f_target, f_target_ );
  updateValue< double >( d, names::tau_m_out, tau_m_out_ );
  updateValue< double >( d, names::adam_m, adam_m_ );
  updateValue< double >( d, names::adam_v, adam_v_ );

  if ( tau_m_out_ <= 0 )
    throw BadProperty( "Membrane time of readout neuron constant must be > 0." );

  const double h = Time::get_resolution().get_ms();
  kappa_ = exp( -h / tau_m_out_ );

  if ( not( ( Wmax_ >= weight_ ) && ( Wmin_ <= weight_ ) ) )
  {
    throw BadProperty( "Wmax, Wmin and the weight have to satisfy Wmax >= weight >= Wmin" );
  }
}

} // namespace nest

#endif // EPROP_SYNAPSE_H
