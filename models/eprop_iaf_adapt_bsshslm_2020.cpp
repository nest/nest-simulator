/*
 *  eprop_iaf_adapt_bsshslm_2020.cpp
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
#include "eprop_iaf_adapt_bsshslm_2020.h"

// C++
#include <limits>

// libnestutil
#include "dict_util.h"
#include "numerics.h"

// nestkernel
#include "eprop_archiving_node_recurrent_impl.h"
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"

// sli
#include "dictutils.h"

namespace nest
{

void
register_eprop_iaf_adapt_bsshslm_2020( const std::string& name )
{
  register_node_model< eprop_iaf_adapt_bsshslm_2020 >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf_adapt_bsshslm_2020 > eprop_iaf_adapt_bsshslm_2020::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf_adapt_bsshslm_2020 >::create()
{
  insert_( names::eprop_history_duration, &eprop_iaf_adapt_bsshslm_2020::get_eprop_history_duration );
  insert_( names::adaptation, &eprop_iaf_adapt_bsshslm_2020::get_adaptation_ );
  insert_( names::V_th_adapt, &eprop_iaf_adapt_bsshslm_2020::get_v_th_adapt_ );
  insert_( names::learning_signal, &eprop_iaf_adapt_bsshslm_2020::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf_adapt_bsshslm_2020::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf_adapt_bsshslm_2020::get_v_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

eprop_iaf_adapt_bsshslm_2020::Parameters_::Parameters_()
  : adapt_beta_( 1.0 )
  , adapt_tau_( 10.0 )
  , C_m_( 250.0 )
  , c_reg_( 0.0 )
  , E_L_( -70.0 )
  , f_target_( 0.01 )
  , beta_( 1.0 )
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

eprop_iaf_adapt_bsshslm_2020::State_::State_()
  : adapt_( 0.0 )
  , v_th_adapt_( 15.0 )
  , learning_signal_( 0.0 )
  , r_( 0 )
  , surrogate_gradient_( 0.0 )
  , i_in_( 0.0 )
  , v_m_( 0.0 )
  , z_( 0.0 )
  , z_in_( 0.0 )
{
}

eprop_iaf_adapt_bsshslm_2020::Buffers_::Buffers_( eprop_iaf_adapt_bsshslm_2020& n )
  : logger_( n )
{
}

eprop_iaf_adapt_bsshslm_2020::Buffers_::Buffers_( const Buffers_&, eprop_iaf_adapt_bsshslm_2020& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
eprop_iaf_adapt_bsshslm_2020::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::adapt_beta, adapt_beta_ );
  def< double >( d, names::adapt_tau, adapt_tau_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::beta, beta_ );
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
eprop_iaf_adapt_bsshslm_2020::Parameters_::set( const DictionaryDatum& d, Node* node )
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

  if ( updateValueParam< double >( d, names::f_target, f_target_, node ) )
  {
    f_target_ /= 1000.0; // convert from spikes/s to spikes/ms
  }

  updateValueParam< double >( d, names::beta, beta_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< bool >( d, names::regular_spike_arrival, regular_spike_arrival_, node );

  if ( updateValueParam< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_, node ) )
  {
    eprop_iaf_adapt_bsshslm_2020* nrn = dynamic_cast< eprop_iaf_adapt_bsshslm_2020* >( node );
    assert( nrn );
    nrn->compute_surrogate_gradient_ = nrn->find_surrogate_gradient( surrogate_gradient_function_ );
  }

  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );

  if ( adapt_beta_ < 0 )
  {
    throw BadProperty( "Threshold adaptation prefactor adapt_beta ≥ 0 required." );
  }

  if ( adapt_tau_ <= 0 )
  {
    throw BadProperty( "Threshold adaptation time constant adapt_tau > 0 required." );
  }

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

  return delta_EL;
}

void
eprop_iaf_adapt_bsshslm_2020::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::adaptation, adapt_ );
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::V_th_adapt, v_th_adapt_ + p.E_L_ );
  def< double >( d, names::surrogate_gradient, surrogate_gradient_ );
  def< double >( d, names::learning_signal, learning_signal_ );
}

void
eprop_iaf_adapt_bsshslm_2020::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  v_m_ -= updateValueParam< double >( d, names::V_m, v_m_, node ) ? p.E_L_ : delta_EL;

  // adaptive threshold can only be set indirectly via the adaptation variable
  if ( updateValueParam< double >( d, names::adaptation, adapt_, node ) )
  {
    // if E_L changed in this SetStatus call, p.V_th_ has been adjusted and no further action is needed
    v_th_adapt_ = p.V_th_ + p.adapt_beta_ * adapt_;
  }
  else
  {
    // adjust voltage to change in E_L
    v_th_adapt_ -= delta_EL;
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

eprop_iaf_adapt_bsshslm_2020::eprop_iaf_adapt_bsshslm_2020()
  : EpropArchivingNodeRecurrent()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

eprop_iaf_adapt_bsshslm_2020::eprop_iaf_adapt_bsshslm_2020( const eprop_iaf_adapt_bsshslm_2020& n )
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
eprop_iaf_adapt_bsshslm_2020::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_iaf_adapt_bsshslm_2020::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  // calculate the entries of the propagator matrix for the evolution of the state vector

  const double dt = Time::get_resolution().get_ms();

  V_.P_v_m_ = std::exp( -dt / P_.tau_m_ );
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P_v_m_ );
  V_.P_z_in_ = P_.regular_spike_arrival_ ? 1.0 : 1.0 - V_.P_v_m_;
  V_.P_adapt_ = std::exp( -dt / P_.adapt_tau_ );
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_iaf_adapt_bsshslm_2020::update( Time const& origin, const long from, const long to )
{
  const long update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();
  const bool with_reset = kernel::manager< SimulationManager >.get_eprop_reset_neurons_on_update();
  const long shift = get_shift();

  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;
    const long interval_step = ( t - shift ) % update_interval;

    if ( interval_step == 0 )
    {
      erase_used_firing_rate_reg_history();
      erase_used_eprop_history();

      if ( with_reset )
      {
        S_.v_m_ = 0.0;
        S_.adapt_ = 0.0;
        S_.r_ = 0;
        S_.z_ = 0.0;
      }
    }

    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + V_.P_z_in_ * S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ -= P_.V_th_ * S_.z_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    S_.adapt_ = V_.P_adapt_ * S_.adapt_ + S_.z_;
    S_.v_th_adapt_ = P_.V_th_ + P_.adapt_beta_ * S_.adapt_;

    S_.z_ = 0.0;

    S_.surrogate_gradient_ =
      ( this->*compute_surrogate_gradient_ )( S_.r_, S_.v_m_, S_.v_th_adapt_, P_.beta_, P_.gamma_ );

    if ( S_.v_m_ >= S_.v_th_adapt_ and S_.r_ == 0 )
    {
      count_spike();

      SpikeEvent se;
      kernel::manager< EventDeliveryManager >.send( *this, se, lag );

      S_.z_ = 1.0;
      S_.r_ = V_.RefractoryCounts_;
    }

    append_new_eprop_history_entry( t );
    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );

    if ( interval_step == update_interval - 1 )
    {
      write_firing_rate_reg_to_history( t, P_.f_target_, P_.c_reg_ );
      reset_spike_count();
    }

    S_.learning_signal_ = get_learning_signal_from_history( t );

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
eprop_iaf_adapt_bsshslm_2020::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
eprop_iaf_adapt_bsshslm_2020::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
eprop_iaf_adapt_bsshslm_2020::handle( LearningSignalConnectionEvent& e )
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
eprop_iaf_adapt_bsshslm_2020::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

double
eprop_iaf_adapt_bsshslm_2020::compute_gradient( std::vector< long >& presyn_isis,
  const long t_previous_update,
  const long t_previous_trigger_spike,
  const double kappa,
  const bool average_gradient )
{
  auto eprop_hist_it = get_eprop_history( t_previous_trigger_spike );

  double e = 0.0;       // eligibility trace
  double e_bar = 0.0;   // low-pass filtered eligibility trace
  double epsilon = 0.0; // adaptive component of eligibility vector
  double grad = 0.0;    // gradient value to be calculated
  double L = 0.0;       // learning signal
  double psi = 0.0;     // surrogate gradient
  double sum_e = 0.0;   // sum of eligibility traces
  double z = 0.0;       // spiking variable
  double z_bar = 0.0;   // low-pass filtered spiking variable

  for ( long presyn_isi : presyn_isis )
  {
    z = 1.0; // set spiking variable to 1 for each incoming spike

    for ( long t = 0; t < presyn_isi; ++t )
    {
      assert( eprop_hist_it != eprop_history_.end() );

      psi = eprop_hist_it->surrogate_gradient_;
      L = eprop_hist_it->learning_signal_;

      z_bar = V_.P_v_m_ * z_bar + V_.P_z_in_ * z;
      e = psi * ( z_bar - P_.adapt_beta_ * epsilon );
      epsilon = V_.P_adapt_ * epsilon + e;
      e_bar = kappa * e_bar + ( 1.0 - kappa ) * e;
      grad += L * e_bar;
      sum_e += e;
      z = 0.0; // set spiking variable to 0 between spikes

      ++eprop_hist_it;
    }
  }
  presyn_isis.clear();

  const long update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();
  const long learning_window = kernel::manager< SimulationManager >.get_eprop_learning_window().get_steps();
  const auto firing_rate_reg = get_firing_rate_reg_history( t_previous_update + get_shift() + update_interval );

  grad += firing_rate_reg * sum_e;

  if ( average_gradient )
  {
    grad /= learning_window;
  }

  return grad;
}

} // namespace nest
