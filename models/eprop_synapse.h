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

Synapse type for eprop-plasticity

Description
+++++++++++

``eprop_synapse`` is a connector to create e-prop synapses as defined
in [1]_. The change of the synaptic weight depends on the presynaptic
spikes, the pseudo-derivative of the postsynaptic membrane voltage and
the learning signal emitted by the readout neurons.

.. math::
    \Delta W_{ji}^\text{rec} &= -\eta \sum_t L_j^t \bar{e}_{ji}^t \\
                             &= -\eta \sum_t L_j^t \mathcal{F}_\kappa
                                \left( \psi^t_j \bar{z}_i^{t-1}\right) \\
                             &= -\eta \sum_t L_j^t \sum_{t'\leq t} \kappa^{t-t'}
                                \psi^t_j \mathcal{F}_\alpha\left( z_j^{t-1}\right)

Furthermore, a firing rate regularization mechanism keeps the firing rate of the
postsynaptic neuron close to the set ``target_firing_rate``:

.. match..
    \Delta W_{ji}^\text{reg} &= \eta c_\text{reg}
    \sum_t \frac{1}{Tn_\text{trial}} \left( f^\text{target}-f^\text{av}_j\right)e_{ji}^t 

E-prop synapses require archiving of continuous quantities. Therefore e-prop
synapses can only be connected to neuron models that are capable of doing this
archiving. So far, compatible models are ``eprop_iaf_psc_delta``,
``eprop_iaf_psc_delta_adapt``, and ``eprop_readout``.

Common Synapse Parameters
+++++++++++++++++++++++++

The following parameters can be set in the status dictionary and will be shared by
all synapses of that type.

More details on the event-based NEST implementation of e-prop can be found in [2]_.

===============  ======  ==========================================================
adam             bool    If True, use Adam optimizer, if False, gradient descent
adam_beta1               Beta1 parameter of Adam optimizer
adam_beta2               Beta2 parameter of Adam optimizer
adam epsilon             Epsilon parameter of Adam optimizer
batch_size               Size of batch
recall_duration  ms      Duration over which gradients are averaged
===============  ======  ==========================================================

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================  ===  ===============================================================
adam_m                   Initial value of first moment estimate m of Adam optimizer
adam_v                   Initial value of second moment raw estimate v of Adam optimizer
eta                      Learning rate
delay               ms   Dendritic delay
c_reg                    Prefactor of firing rate regularization
target_firing_rate  Hz   Target firing rate of rate regularization
tau_decay           ms   Time constant for low-pass filtering of eligibility trace
weight              pA   Synaptic weight
Wmax                pA   Maximal value for synaptic weight
Wmin                pA   Minimal value for synaptic weight
==================  ===  ===============================================================


Transmits
+++++++++

SpikeEvent, DSSpikeEvent

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       DOI: https://doi.org/10.1038/s41467-020-17236-y
.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
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
  , recall_duration_( 1. )
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
  double target_firing_rate_;
  double tau_decay_; // time constant for low pass filtering of eligibility trace
  double kappa_;     // exp( -dt / tau_decay_ )
  double adam_m_;    // auxiliary variable for adam optimizer
  double adam_v_;    // auxiliary variable for adam optimizer
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

      double t_next_ud = t_next_update_;
      double t_last_ud = t_last_update_;
      double t_curr_ud = t_current_update_;

      if ( target_node == "readout" )
      {
        t_last_ud += dendritic_delay;
        t_curr_ud += dendritic_delay;
      }
      else
      {
        t_next_ud -= dendritic_delay;
      }
      presyn_spike_times_.insert( --presyn_spike_times_.end(), t_next_ud );
      target->get_eprop_history( presyn_spike_times_[ 0 ] + dendritic_delay,
        t_last_ud + update_interval_,
        t_last_ud,
        t_curr_ud,
        &start,
        &finish );

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

        double beta;
        double rho;
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
              epsilon = psi * last_z_bar + ( rho - beta * psi ) * epsilon;
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
        double avg_firing_rate = nspikes / update_interval_;
        double target_firing_rate_kHz = target_firing_rate_ / 1000.;

        grad += c_reg_ * ( avg_firing_rate - target_firing_rate_kHz ) * sum_e_bar / update_interval_;
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
  , c_reg_( 0. )
  , target_firing_rate_( 10. )
  , tau_decay_( 0.0 )
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
  def< double >( d, names::target_firing_rate, target_firing_rate_ );
  def< double >( d, names::tau_decay, tau_decay_ );
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
  updateValue< double >( d, names::target_firing_rate, target_firing_rate_ );
  updateValue< double >( d, names::tau_decay, tau_decay_ );
  updateValue< double >( d, names::adam_m, adam_m_ );
  updateValue< double >( d, names::adam_v, adam_v_ );

  if ( tau_decay_ > 0.0 )
  {
    const double h = Time::get_resolution().get_ms();
    kappa_ = exp( -h / tau_decay_ );
  }
  else if ( tau_decay_ == 0.0 )
  {
    kappa_ = 0.0;
  }
  else
  {
    throw BadProperty( "The synaptic time constant tau_decay must be greater than zero." );
  }

  if ( not( ( Wmax_ >= weight_ ) && ( Wmin_ <= weight_ ) ) )
  {
    throw BadProperty( "Wmax, Wmin and the weight have to satisfy Wmax >= weight >= Wmin" );
  }
}

} // namespace nest

#endif // EPROP_SYNAPSE_H
