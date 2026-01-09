/*
 *  eprop_iaf.cpp
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

// nest models
#include "eprop_iaf.h"

// C++
#include <limits>

// libnestutil
#include "dict_util.h"
#include "numerics.h"

// nestkernel
#include "eprop_archiving_node_recurrent_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// sli
#include "dictutils.h"

namespace nest
{

void
register_eprop_iaf( const std::string& name )
{
  register_node_model< eprop_iaf >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf > eprop_iaf::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf >::create()
{
  insert_( names::eprop_history_duration, &eprop_iaf::get_eprop_history_duration );
  insert_( names::learning_signal, &eprop_iaf::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf::get_v_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

eprop_iaf::Parameters_::Parameters_()
  : C_m_( 250.0 )
  , c_reg_( 0.0 )
  , E_L_( -70.0 )
  , f_target_( 0.01 )
  , beta_( 1.0 )
  , gamma_( 0.3 )
  , I_e_( 0.0 )
  , surrogate_gradient_function_( "piecewise_linear" )
  , t_ref_( 2.0 )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , V_th_( -55.0 - E_L_ )
  , kappa_( 0.97 )
  , kappa_reg_( 0.97 )
  , eprop_isi_trace_cutoff_( 1000.0 )
  , activation_interval_( 3000.0 )
{
}

eprop_iaf::State_::State_()
  : learning_signal_( 0.0 )
  , r_( 0 )
  , surrogate_gradient_( 0.0 )
  , i_in_( 0.0 )
  , v_m_( 0.0 )
  , z_( 0.0 )
  , z_in_( 0.0 )
{
}

eprop_iaf::Buffers_::Buffers_( eprop_iaf& n )
  : logger_( n )
{
}

eprop_iaf::Buffers_::Buffers_( const Buffers_&, eprop_iaf& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
eprop_iaf::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::beta, beta_ );
  def< double >( d, names::gamma, gamma_ );
  def< double >( d, names::I_e, I_e_ );
  def< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::V_th, V_th_ + E_L_ );
  def< double >( d, names::kappa, kappa_ );
  def< double >( d, names::kappa_reg, kappa_reg_ );
  def< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );
  def< double >( d, names::activation_interval, activation_interval_ );
}

double
eprop_iaf::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if leak potential is changed, adjust all variables defined relative to it
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  V_th_ -= updateValueParam< double >( d, names::V_th, V_th_, node ) ? E_L_ : delta_EL;
  V_min_ -= updateValueParam< double >( d, names::V_min, V_min_, node ) ? E_L_ : delta_EL;

  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::c_reg, c_reg_, node );

  if ( updateValueParam< double >( d, names::f_target, f_target_, node ) )
  {
    f_target_ /= 1000.0; // convert from spikes/s to spikes/ms
  }

  updateValueParam< double >( d, names::beta, beta_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );

  if ( updateValueParam< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_, node ) )
  {
    eprop_iaf* nrn = dynamic_cast< eprop_iaf* >( node );
    assert( nrn );
    nrn->compute_surrogate_gradient_ = nrn->find_surrogate_gradient( surrogate_gradient_function_ );
  }

  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::kappa, kappa_, node );
  updateValueParam< double >( d, names::kappa_reg, kappa_reg_, node );
  updateValueParam< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_, node );
  updateValueParam< double >( d, names::activation_interval, activation_interval_, node );

  if ( C_m_ <= 0 )
  {
    throw BadProperty( "Membrane capacitance C_m > 0 required." );
  }

  if ( c_reg_ < 0 )
  {
    throw BadProperty( "Firing rate regularization coefficient c_reg ≥ 0 required." );
  }

  if ( f_target_ < 0 )
  {
    throw BadProperty( "Firing rate regularization target rate f_target ≥ 0 required." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant tau_m > 0 required." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time t_ref ≥ 0 required." );
  }

  if ( V_th_ < V_min_ )
  {
    throw BadProperty( "Spike threshold voltage V_th ≥ minimal voltage V_min required." );
  }

  if ( kappa_ < 0.0 or kappa_ > 1.0 )
  {
    throw BadProperty( "Eligibility trace low-pass filter kappa from range [0, 1] required." );
  }

  if ( kappa_reg_ < 0.0 or kappa_reg_ > 1.0 )
  {
    throw BadProperty( "Firing rate low-pass filter for regularization kappa_reg from range [0, 1] required." );
  }

  if ( activation_interval_ < 0 )
  {
    throw BadProperty( "Interval between activations activation_interval ≥ 0 required." );
  }

  if ( eprop_isi_trace_cutoff_ < 0.0 or eprop_isi_trace_cutoff_ > activation_interval_ )
  {
    throw BadProperty(
      "Computation cutoff of eprop trace 0 ≤ eprop trace eprop_isi_trace_cutoff ≤ activation_interval required." );
  }
  return delta_EL;
}

void
eprop_iaf::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::surrogate_gradient, surrogate_gradient_ );
  def< double >( d, names::learning_signal, learning_signal_ );
}

void
eprop_iaf::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  v_m_ -= updateValueParam< double >( d, names::V_m, v_m_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

eprop_iaf::eprop_iaf()
  : EpropArchivingNodeRecurrent()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

eprop_iaf::eprop_iaf( const eprop_iaf& n )
  : EpropArchivingNodeRecurrent( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
eprop_iaf::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_iaf::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  V_.eprop_isi_trace_cutoff_steps_ = Time( Time::ms( P_.eprop_isi_trace_cutoff_ ) ).get_steps();
  V_.activation_interval_steps_ = Time( Time::ms( P_.activation_interval_ ) ).get_steps();

  // calculate the entries of the propagator matrix for the evolution of the state vector

  const double dt = Time::get_resolution().get_ms();

  V_.P_v_m_ = std::exp( -dt / P_.tau_m_ );
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P_v_m_ );
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_iaf::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;

    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    S_.z_ = 0.0;

    S_.surrogate_gradient_ = ( this->*compute_surrogate_gradient_ )( S_.r_, S_.v_m_, P_.V_th_, P_.beta_, P_.gamma_ );

    if ( S_.v_m_ >= P_.V_th_ and S_.r_ == 0 )
    {
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      S_.z_ = 1.0;
      S_.v_m_ -= P_.V_th_ * S_.z_;
      S_.r_ = V_.RefractoryCounts_;
      set_last_event_time( t );
    }
    else if ( get_last_event_time() > 0 and t - get_last_event_time() >= V_.activation_interval_steps_ )
    {
      SpikeEvent se;
      se.set_activation();
      kernel().event_delivery_manager.send( *this, se, lag );
      set_last_event_time( t );
    }

    append_new_eprop_history_entry( t );
    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );
    write_firing_rate_reg_to_history( t, S_.z_, P_.f_target_, P_.kappa_reg_, P_.c_reg_ );

    S_.learning_signal_ = get_learning_signal_from_history( t );

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
eprop_iaf::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
eprop_iaf::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
eprop_iaf::handle( LearningSignalConnectionEvent& e )
{
  for ( auto it_event = e.begin(); it_event != e.end(); )
  {
    const long time_step = e.get_stamp().get_steps();
    const double weight = e.get_weight();
    const double error_signal = e.get_coeffvalue( it_event ); // get_coeffvalue advances iterator
    const double learning_signal = weight * error_signal;

    write_learning_signal_to_history( time_step, learning_signal );
  }
}

void
eprop_iaf::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
eprop_iaf::compute_gradient( const long t_spike,
  const long t_spike_previous,
  double& z_previous_buffer,
  double& z_bar,
  double& e_bar,
  double& e_bar_reg,
  double& epsilon,
  double& weight,
  const CommonSynapseProperties& cp,
  WeightOptimizer* optimizer,
  const bool activation,
  const bool previous_event_was_activation,
  double& sum_grad )
{
  const auto& ecp = static_cast< const EpropSynapseCommonProperties& >( cp );
  const auto& opt_cp = *ecp.optimizer_cp_;
  const bool optimize_each_step = opt_cp.optimize_each_step_;

  if ( not previous_event_was_activation )
  {
    sum_grad = 0.0; // sum of gradients
  }

  auto eprop_hist_it = get_eprop_history( t_spike_previous - 1 );

  const long cutoff_end = t_spike_previous + V_.eprop_isi_trace_cutoff_steps_;
  const long t_compute_until = std::min( cutoff_end, t_spike );

  if ( not previous_event_was_activation )
  {
    double z_current_buffer = 1.0; // spike that triggered current computation

    for ( long t = t_spike_previous; t < t_compute_until; ++t, ++eprop_hist_it )
    {
      const double z = z_previous_buffer; // spiking variable
      z_previous_buffer = z_current_buffer;
      z_current_buffer = 0.0;

      const double psi = eprop_hist_it->surrogate_gradient_;          // surrogate gradient
      const double L = eprop_hist_it->learning_signal_;               // learning signal
      const double firing_rate_reg = eprop_hist_it->firing_rate_reg_; // firing rate regularization

      z_bar = V_.P_v_m_ * z_bar + z;
      const double e = psi * z_bar; // eligibility trace
      e_bar = P_.kappa_ * e_bar + e;
      e_bar_reg = P_.kappa_reg_ * e_bar_reg + ( 1.0 - P_.kappa_reg_ ) * e;

      const double grad = L * e_bar + firing_rate_reg * e_bar_reg;

      if ( optimize_each_step )
      {
        sum_grad = grad;
        weight = optimizer->optimized_weight( opt_cp, t, sum_grad, weight );
      }
      else
      {
        sum_grad += grad;
      }
    }
  }

  const long trace_decay_interval = t_spike - ( previous_event_was_activation ? t_spike_previous : t_compute_until );

  if ( trace_decay_interval > 0 )
  {
    z_bar *= std::exp( std::log( V_.P_v_m_ ) * trace_decay_interval );
    e_bar *= std::exp( std::log( P_.kappa_ ) * trace_decay_interval );
    e_bar_reg *= std::exp( std::log( P_.kappa_reg_ ) * trace_decay_interval );
  }

  if ( not( activation or optimize_each_step ) )
  {
    weight = optimizer->optimized_weight( opt_cp, t_compute_until, sum_grad, weight );
  }
}

} // namespace nest
