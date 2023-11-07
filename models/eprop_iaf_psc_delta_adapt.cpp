/*
 *  eprop_iaf_psc_delta_adapt.cpp
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
#include "eprop_iaf_psc_delta_adapt.h"

// C++
#include <limits>

// libnestutil
#include "dict_util.h"
#include "numerics.h"

// nestkernel
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// sli
#include "dictutils.h"

namespace nest
{
void
register_eprop_iaf_psc_delta_adapt( const std::string& name )
{
  register_node_model< eprop_iaf_psc_delta_adapt >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf_psc_delta_adapt > eprop_iaf_psc_delta_adapt::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf_psc_delta_adapt >::create()
{
  insert_( names::adaptation, &eprop_iaf_psc_delta_adapt::get_adaptation_ );
  insert_( names::adapting_threshold, &eprop_iaf_psc_delta_adapt::get_adapting_threshold_ );
  insert_( names::learning_signal, &eprop_iaf_psc_delta_adapt::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf_psc_delta_adapt::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf_psc_delta_adapt::get_V_m_ );
}
} // namespace nest

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta_adapt::Parameters_::Parameters_()
  : adapt_beta_( 1.0 )
  , adapt_tau_( 10.0 )
  , C_m_( 250.0 )
  , c_reg_( 0.0 )
  , E_L_( -70.0 )
  , f_target_( 10.0 )
  , gamma_( 0.3 )
  , I_e_( 0.0 )
  , propagator_idx_( 0 )
  , surrogate_gradient_( "piecewise_linear" )
  , t_ref_( 2.0 )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , V_th_( -55.0 - E_L_ )
{
}

nest::eprop_iaf_psc_delta_adapt::State_::State_()
  : adaptation_( 0.0 )
  , learning_signal_( 0.0 )
  , r_( 0 )
  , surrogate_gradient_( 0.0 )
  , y0_( 0.0 )
  , y3_( 0.0 )
{
}

nest::eprop_iaf_psc_delta_adapt::Buffers_::Buffers_( eprop_iaf_psc_delta_adapt& n )
  : logger_( n )
{
}

nest::eprop_iaf_psc_delta_adapt::Buffers_::Buffers_( const Buffers_&, eprop_iaf_psc_delta_adapt& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta_adapt::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::adapt_beta, adapt_beta_ );
  def< double >( d, names::adapt_tau, adapt_tau_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::gamma, gamma_ );
  def< double >( d, names::I_e, I_e_ );
  def< long >( d, names::propagator_idx, propagator_idx_ );
  def< std::string >( d, names::surrogate_gradient, surrogate_gradient_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::V_th, V_th_ + E_L_ );
}

double
nest::eprop_iaf_psc_delta_adapt::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if leak potential is changed, adjust all variables defined relative to it
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  V_th_ -= updateValueParam< double >( d, names::V_th, V_th_, node ) ? E_L_ : delta_EL;
  V_min_ -= updateValueParam< double >( d, names::V_min, V_min_, node ) ? E_L_ : delta_EL;

  updateValueParam< double >( d, names::adapt_beta, adapt_beta_, node );
  updateValueParam< double >( d, names::adapt_tau, adapt_tau_, node );
  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::c_reg, c_reg_, node );
  updateValueParam< double >( d, names::f_target, f_target_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< long >( d, names::propagator_idx, propagator_idx_, node );
  updateValueParam< std::string >( d, names::surrogate_gradient, surrogate_gradient_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );

  if ( adapt_tau_ <= 0 )
  {
    throw BadProperty( "Time constant of threshold adaptation must be > 0." );
  }

  if ( C_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be > 0." );
  }

  if ( propagator_idx_ != 0 and propagator_idx_ != 1 )
  {
    throw BadProperty( "One of two available propagators indexed by 0 and 1 must be selected." );
  }

  if ( surrogate_gradient_ != "piecewise_linear" )
  {
    throw BadProperty( "One of the available surrogate gradients [\"piecewise_linear\"] needs to be selected." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant must be > 0." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  if ( surrogate_gradient_ == "piecewise_linear" and fabs( V_th_ ) < 1e-6 )
  {
    throw BadProperty( "V_th-E_L must be != 0 if surrogate_gradient is \"piecewise_linear\"." );
  }

  return delta_EL;
}

void
nest::eprop_iaf_psc_delta_adapt::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::adaptation, adaptation_ );
  def< double >( d, names::V_m, y3_ + p.E_L_ );
}

void
nest::eprop_iaf_psc_delta_adapt::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  double delta_EL,
  Node* node )
{
  updateValueParam< double >( d, names::adaptation, adaptation_, node );

  y3_ -= updateValueParam< double >( d, names::V_m, y3_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta_adapt::eprop_iaf_psc_delta_adapt()
  : EpropArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::eprop_iaf_psc_delta_adapt::eprop_iaf_psc_delta_adapt( const eprop_iaf_psc_delta_adapt& n )
  : EpropArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta_adapt::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize

  S_.z_ = 0.0;
}

void
nest::eprop_iaf_psc_delta_adapt::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  const double dt = Time::get_resolution().get_ms();

  V_.P33_ = std::exp( -dt / P_.tau_m_ ); // alpha
  V_.P30_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P33_ );

  const double propagators[] = { 1.0 - V_.P33_, 1.0 };
  V_.P33_complement_ = propagators[ P_.propagator_idx_ ];

  V_.Pa_ = std::exp( -dt / P_.adapt_tau_ );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  if ( P_.surrogate_gradient_ == "piecewise_linear" )
  {
    compute_surrogate_gradient = &eprop_iaf_psc_delta_adapt::compute_piecewise_linear_derivative;
  }
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta_adapt::update( Time const& origin, const long from, const long to )
{
  const long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();
  const bool with_reset = kernel().simulation_manager.get_eprop_reset_neurons_on_update();
  const long shift = get_shift();

  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;
    const long interval_step = ( t - shift ) % update_interval;

    if ( interval_step == 0 )
    {
      erase_unneeded_firing_rate_reg_history();
      erase_unneeded_update_history();
      erase_unneeded_eprop_history();

      if ( with_reset )
      {
        S_.y3_ = 0.0;
        S_.adaptation_ = 0.0;
        S_.r_ = 0;
        S_.z_ = 0.0;
      }
    }

    S_.adaptation_ *= V_.Pa_;
    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + V_.P33_complement_ * B_.spikes_.get_value( lag );

    S_.adaptation_ += S_.z_;
    S_.y3_ -= S_.z_ * P_.V_th_;
    S_.z_ = 0.0;

    S_.y3_ = std::max( S_.y3_, P_.V_min_ );

    S_.adapting_threshold_ = P_.V_th_ + P_.adapt_beta_ * S_.adaptation_;

    S_.surrogate_gradient_ = ( this->*compute_surrogate_gradient )();

    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );

    if ( S_.y3_ >= S_.adapting_threshold_ and S_.r_ == 0 )
    {
      count_spike();

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      S_.z_ = 1.0;

      if ( V_.RefractoryCounts_ > 0 )
      {
        S_.r_ = V_.RefractoryCounts_;
      }
    }

    if ( interval_step == update_interval - 1 )
    {
      write_firing_rate_reg_to_history( t, P_.f_target_, P_.c_reg_ );
      reset_spike_count();
    }

    const auto it_eprop_hist = get_eprop_history( t - shift );
    S_.learning_signal_ = it_eprop_hist->learning_signal_;

    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    S_.y0_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Surrogate gradient functions
 * ---------------------------------------------------------------- */

double
nest::eprop_iaf_psc_delta_adapt::compute_piecewise_linear_derivative()
{
  const double v_m = S_.r_ > 0 ? 0.0 : S_.y3_;
  const double v_th = S_.r_ > 0 ? P_.V_th_ : S_.adapting_threshold_;
  const double psi = P_.gamma_ * std::max( 0.0, 1.0 - std::fabs( ( v_m - v_th ) / P_.V_th_ ) ) / P_.V_th_;
  return psi;
}


/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta_adapt::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::eprop_iaf_psc_delta_adapt::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::eprop_iaf_psc_delta_adapt::handle( LearningSignalConnectionEvent& e )
{
  std::vector< unsigned int >::iterator it_event = e.begin();
  std::vector< unsigned int >::iterator it_event_end = e.end();

  if ( it_event != it_event_end )
  {
    const long time_step = e.get_stamp().get_steps();
    const long delay_out_rec = e.get_delay_steps();
    const double weight = e.get_weight();
    const double error_signal = e.get_coeffvalue( it_event ); // get_coeffvalue advances iterator

    write_learning_signal_to_history( time_step, delay_out_rec, weight, error_signal );
  }
}

void
nest::eprop_iaf_psc_delta_adapt::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

double
nest::eprop_iaf_psc_delta_adapt::gradient_change( std::vector< long >& presyn_isis,
  const long t_previous_update,
  const long t_previous_trigger_spike,
  const double kappa )
{
  auto eprop_hist_it = get_eprop_history( t_previous_trigger_spike );

  double e_bar = 0.0;
  double sum_e = 0.0;
  double previous_z_bar = 0.0;
  double epsilon = 0.0;
  double grad = 0.0;

  for ( long presyn_isi : presyn_isis )
  {
    previous_z_bar += V_.P33_complement_;
    for ( long t = 0; t < presyn_isi; ++t )
    {
      assert( eprop_hist_it != eprop_history_.end() );
      const double psi = eprop_hist_it->surrogate_gradient_;
      const double e = psi * ( previous_z_bar - P_.adapt_beta_ * epsilon );
      epsilon = psi * previous_z_bar + ( V_.Pa_ - psi * P_.adapt_beta_ ) * epsilon;

      previous_z_bar *= V_.P33_;
      sum_e += e;

      e_bar = kappa * e_bar + ( 1.0 - kappa ) * e;
      grad += e_bar * eprop_hist_it->learning_signal_;

      ++eprop_hist_it;
    }
  }
  presyn_isis.clear();

  const long learning_window = kernel().simulation_manager.get_eprop_learning_window().get_steps();
  const long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();

  if ( learning_window != update_interval )
  {
    grad /= learning_window;
  }

  const auto it_reg_hist = get_firing_rate_reg_history( t_previous_update + get_shift() + update_interval );
  grad += it_reg_hist->firing_rate_reg_ * sum_e;

  return grad;
}
