/*
 *  eprop_iaf_bsshslm_2020.cpp
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
#include "eprop_iaf_bsshslm_2020.h"

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
register_eprop_iaf_bsshslm_2020( const std::string& name )
{
  register_node_model< eprop_iaf_bsshslm_2020 >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf_bsshslm_2020 > eprop_iaf_bsshslm_2020::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf_bsshslm_2020 >::create()
{
  insert_( names::learning_signal, &eprop_iaf_bsshslm_2020::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf_bsshslm_2020::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf_bsshslm_2020::get_v_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

eprop_iaf_bsshslm_2020::Parameters_::Parameters_()
  : C_m_( 250.0 )
  , c_reg_( 0.0 )
  , E_L_( -70.0 )
  , f_target_( 0.01 )
  , gamma_( 0.3 )
  , I_e_( 0.0 )
  , regular_spike_arrival_( true )
  , surrogate_gradient_function_( "piecewise_linear" )
  , t_ref_( 2.0 )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , V_th_( -55.0 - E_L_ )
{
}

eprop_iaf_bsshslm_2020::State_::State_()
  : learning_signal_( 0.0 )
  , r_( 0 )
  , surrogate_gradient_( 0.0 )
  , i_in_( 0.0 )
  , v_m_( 0.0 )
  , z_( 0.0 )
  , z_in_( 0.0 )
{
}

eprop_iaf_bsshslm_2020::Buffers_::Buffers_( eprop_iaf_bsshslm_2020& n )
  : logger_( n )
{
}

eprop_iaf_bsshslm_2020::Buffers_::Buffers_( const Buffers_&, eprop_iaf_bsshslm_2020& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
eprop_iaf_bsshslm_2020::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::gamma, gamma_ );
  def< double >( d, names::I_e, I_e_ );
  def< bool >( d, names::regular_spike_arrival, regular_spike_arrival_ );
  def< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::V_th, V_th_ + E_L_ );
}

double
eprop_iaf_bsshslm_2020::Parameters_::set( const DictionaryDatum& d, Node* node )
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

  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< bool >( d, names::regular_spike_arrival, regular_spike_arrival_, node );
  updateValueParam< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );

  if ( C_m_ <= 0 )
  {
    throw BadProperty( "Membrane capacitance C_m > 0 required." );
  }

  if ( c_reg_ < 0 )
  {
    throw BadProperty( "Firing rate regularization prefactor c_reg ≥ 0 required." );
  }

  if ( f_target_ < 0 )
  {
    throw BadProperty( "Firing rate regularization target rate f_target ≥ 0 required." );
  }

  if ( gamma_ < 0.0 or 1.0 <= gamma_ )
  {
    throw BadProperty( "Surrogate gradient / pseudo-derivative scaling gamma from interval [0,1) required." );
  }

  if ( surrogate_gradient_function_ != "piecewise_linear" )
  {
    throw BadProperty(
      "Surrogate gradient / pseudo derivate function surrogate_gradient_function from [\"piecewise_linear\"] "
      "required." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant tau_m > 0 required." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time t_ref ≥ 0 required." );
  }

  if ( surrogate_gradient_function_ == "piecewise_linear" and fabs( V_th_ ) < 1e-6 )
  {
    throw BadProperty(
      "Relative threshold voltage V_th-E_L ≠ 0 required if surrogate_gradient_function is \"piecewise_linear\"." );
  }

  if ( V_th_ < V_min_ )
  {
    throw BadProperty( "Spike threshold voltage V_th ≥ minimal voltage V_min required." );
  }

  return delta_EL;
}

void
eprop_iaf_bsshslm_2020::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::surrogate_gradient, surrogate_gradient_ );
  def< double >( d, names::learning_signal, learning_signal_ );
}

void
eprop_iaf_bsshslm_2020::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  v_m_ -= updateValueParam< double >( d, names::V_m, v_m_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

eprop_iaf_bsshslm_2020::eprop_iaf_bsshslm_2020()
  : EpropArchivingNodeRecurrent()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

eprop_iaf_bsshslm_2020::eprop_iaf_bsshslm_2020( const eprop_iaf_bsshslm_2020& n )
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
eprop_iaf_bsshslm_2020::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_iaf_bsshslm_2020::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  if ( P_.surrogate_gradient_function_ == "piecewise_linear" )
  {
    compute_surrogate_gradient = &eprop_iaf_bsshslm_2020::compute_piecewise_linear_derivative;
  }

  // calculate the entries of the propagator matrix for the evolution of the state vector

  const double dt = Time::get_resolution().get_ms();

  V_.P_v_m_ = std::exp( -dt / P_.tau_m_ ); // called alpha in reference [1]
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P_v_m_ );
  V_.P_z_in_ = P_.regular_spike_arrival_ ? 1.0 : 1.0 - V_.P_v_m_;
}

long
eprop_iaf_bsshslm_2020::get_shift() const
{
  return offset_gen_ + delay_in_rec_;
}

bool
eprop_iaf_bsshslm_2020::is_eprop_recurrent_node() const
{
  return true;
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_iaf_bsshslm_2020::update( Time const& origin, const long from, const long to )
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
      erase_used_firing_rate_reg_history();
      erase_used_update_history();
      erase_used_eprop_history();

      if ( with_reset )
      {
        S_.v_m_ = 0.0;
        S_.r_ = 0;
        S_.z_ = 0.0;
      }
    }

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + V_.P_z_in_ * S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ -= P_.V_th_ * S_.z_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    S_.z_ = 0.0;

    S_.surrogate_gradient_ = ( this->*compute_surrogate_gradient )();

    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );

    if ( S_.v_m_ >= P_.V_th_ and S_.r_ == 0 )
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

    S_.learning_signal_ = get_learning_signal_from_history( t );

    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Surrogate gradient functions
 * ---------------------------------------------------------------- */

double
eprop_iaf_bsshslm_2020::compute_piecewise_linear_derivative()
{
  if ( S_.r_ > 0 )
  {
    return 0.0;
  }

  return P_.gamma_ * std::max( 0.0, 1.0 - std::fabs( ( S_.v_m_ - P_.V_th_ ) / P_.V_th_ ) ) / P_.V_th_;
}

/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
eprop_iaf_bsshslm_2020::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
eprop_iaf_bsshslm_2020::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
eprop_iaf_bsshslm_2020::handle( LearningSignalConnectionEvent& e )
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
eprop_iaf_bsshslm_2020::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

double
eprop_iaf_bsshslm_2020::compute_gradient( std::vector< long >& presyn_isis,
  const long t_previous_update,
  const long t_previous_trigger_spike,
  const double kappa,
  const bool average_gradient )
{
  auto eprop_hist_it = get_eprop_history( t_previous_trigger_spike );

  double e = 0.0;     // eligibility trace
  double e_bar = 0.0; // low-pass filtered eligibility trace
  double grad = 0.0;  // gradient value to be calculated
  double L = 0.0;     // learning signal
  double psi = 0.0;   // surrogate gradient
  double sum_e = 0.0; // sum of eligibility traces
  double z = 0.0;     // spiking variable
  double z_bar = 0.0; // low-pass filtered spiking variable

  for ( long presyn_isi : presyn_isis )
  {
    z = 1.0; // set spiking variable to 1 for each incoming spike

    for ( long t = 0; t < presyn_isi; ++t )
    {
      assert( eprop_hist_it != eprop_history_.end() );

      psi = eprop_hist_it->surrogate_gradient_;
      L = eprop_hist_it->learning_signal_;

      z_bar = V_.P_v_m_ * z_bar + V_.P_z_in_ * z;
      e = psi * z_bar;
      e_bar = kappa * e_bar + ( 1.0 - kappa ) * e;
      grad += L * e_bar;
      sum_e += e;
      z = 0.0; // set spiking variable to 0 between spikes

      ++eprop_hist_it;
    }
  }
  presyn_isis.clear();

  const long learning_window = kernel().simulation_manager.get_eprop_learning_window().get_steps();
  if ( average_gradient )
  {
    grad /= learning_window;
  }

  const long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();
  const auto it_reg_hist = get_firing_rate_reg_history( t_previous_update + get_shift() + update_interval );
  grad += it_reg_hist->firing_rate_reg_ * sum_e;

  return grad;
}

} // namespace nest
