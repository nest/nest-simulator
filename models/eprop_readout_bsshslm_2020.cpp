/*
 *  eprop_readout_bsshslm_2020.cpp
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
#include "eprop_readout_bsshslm_2020.h"

// C++
#include <limits>

// libnestutil
#include "dict_util.h"
#include "numerics.h"

// nestkernel
#include "eprop_archiving_node_readout_impl.h"
#include "eprop_archiving_node_recurrent_impl.h"
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"

// sli
#include "dictutils.h"

namespace nest
{

void
register_eprop_readout_bsshslm_2020( const std::string& name )
{
  register_node_model< eprop_readout_bsshslm_2020 >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_readout_bsshslm_2020 > eprop_readout_bsshslm_2020::recordablesMap_;

template <>
void
RecordablesMap< eprop_readout_bsshslm_2020 >::create()
{
  insert_( names::eprop_history_duration, &eprop_readout_bsshslm_2020::get_eprop_history_duration );
  insert_( names::error_signal, &eprop_readout_bsshslm_2020::get_error_signal_ );
  insert_( names::readout_signal, &eprop_readout_bsshslm_2020::get_readout_signal_ );
  insert_( names::readout_signal_unnorm, &eprop_readout_bsshslm_2020::get_readout_signal_unnorm_ );
  insert_( names::target_signal, &eprop_readout_bsshslm_2020::get_target_signal_ );
  insert_( names::V_m, &eprop_readout_bsshslm_2020::get_v_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

eprop_readout_bsshslm_2020::Parameters_::Parameters_()
  : C_m_( 250.0 )
  , E_L_( 0.0 )
  , I_e_( 0.0 )
  , loss_( "mean_squared_error" )
  , regular_spike_arrival_( true )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
{
}

eprop_readout_bsshslm_2020::State_::State_()
  : error_signal_( 0.0 )
  , readout_signal_( 0.0 )
  , readout_signal_unnorm_( 0.0 )
  , target_signal_( 0.0 )
  , i_in_( 0.0 )
  , v_m_( 0.0 )
  , z_in_( 0.0 )
{
}

eprop_readout_bsshslm_2020::Buffers_::Buffers_( eprop_readout_bsshslm_2020& n )
  : logger_( n )
{
}

eprop_readout_bsshslm_2020::Buffers_::Buffers_( const Buffers_&, eprop_readout_bsshslm_2020& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
eprop_readout_bsshslm_2020::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< std::string >( d, names::loss, loss_ );
  def< bool >( d, names::regular_spike_arrival, regular_spike_arrival_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
}

double
eprop_readout_bsshslm_2020::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if leak potential is changed, adjust all variables defined relative to it
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  V_min_ -= updateValueParam< double >( d, names::V_min, V_min_, node ) ? E_L_ : delta_EL;

  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< std::string >( d, names::loss, loss_, node );
  updateValueParam< bool >( d, names::regular_spike_arrival, regular_spike_arrival_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );

  if ( C_m_ <= 0 )
  {
    throw BadProperty( "Membrane capacitance C_m > 0 required." );
  }

  if ( loss_ != "mean_squared_error" and loss_ != "cross_entropy" )
  {
    throw BadProperty( "Loss function loss from [\"mean_squared_error\", \"cross_entropy\"] required." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant tau_m > 0 required." );
  }

  return delta_EL;
}

void
eprop_readout_bsshslm_2020::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::error_signal, error_signal_ );
  def< double >( d, names::readout_signal, readout_signal_ );
  def< double >( d, names::readout_signal_unnorm, readout_signal_unnorm_ );
  def< double >( d, names::target_signal, target_signal_ );
}

void
eprop_readout_bsshslm_2020::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  v_m_ -= updateValueParam< double >( d, names::V_m, v_m_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

eprop_readout_bsshslm_2020::eprop_readout_bsshslm_2020()
  : EpropArchivingNodeReadout()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

eprop_readout_bsshslm_2020::eprop_readout_bsshslm_2020( const eprop_readout_bsshslm_2020& n )
  : EpropArchivingNodeReadout( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
eprop_readout_bsshslm_2020::init_buffers_()
{
  B_.normalization_rate_ = 0;
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_readout_bsshslm_2020::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  if ( P_.loss_ == "mean_squared_error" )
  {
    compute_error_signal = &eprop_readout_bsshslm_2020::compute_error_signal_mean_squared_error;
    V_.signal_to_other_readouts_ = false;
  }
  else if ( P_.loss_ == "cross_entropy" )
  {
    compute_error_signal = &eprop_readout_bsshslm_2020::compute_error_signal_cross_entropy;
    V_.signal_to_other_readouts_ = true;
  }

  const double dt = Time::get_resolution().get_ms();

  V_.P_v_m_ = std::exp( -dt / P_.tau_m_ );
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P_v_m_ );
  V_.P_z_in_ = P_.regular_spike_arrival_ ? 1.0 : 1.0 - V_.P_v_m_;
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_readout_bsshslm_2020::update( Time const& origin, const long from, const long to )
{
  const long update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();
  const long learning_window = kernel::manager< SimulationManager >.get_eprop_learning_window().get_steps();
  const bool with_reset = kernel::manager< SimulationManager >.get_eprop_reset_neurons_on_update();
  const long shift = get_shift();

  const size_t buffer_size = kernel::manager< ConnectionManager >.get_min_delay();

  std::vector< double > error_signal_buffer( buffer_size, 0.0 );
  std::vector< double > readout_signal_unnorm_buffer( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;
    const long interval_step = ( t - shift ) % update_interval;
    const long interval_step_signals = ( t - shift - delay_out_norm_ ) % update_interval;

    if ( interval_step == 0 )
    {
      erase_used_eprop_history();

      if ( with_reset )
      {
        S_.v_m_ = 0.0;
      }
    }

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + V_.P_z_in_ * S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    ( this->*compute_error_signal )( lag );

    if ( interval_step_signals < update_interval - learning_window )
    {
      S_.target_signal_ = 0.0;
      S_.readout_signal_ = 0.0;
      S_.error_signal_ = 0.0;
    }

    B_.normalization_rate_ = 0.0;

    if ( V_.signal_to_other_readouts_ )
    {
      readout_signal_unnorm_buffer[ lag ] = S_.readout_signal_unnorm_;
    }

    error_signal_buffer[ lag ] = S_.error_signal_;

    append_new_eprop_history_entry( t );
    write_error_signal_to_history( t, S_.error_signal_ );

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }

  LearningSignalConnectionEvent error_signal_event;
  error_signal_event.set_coeffarray( error_signal_buffer );
  kernel::manager< EventDeliveryManager >.send_secondary( *this, error_signal_event );

  if ( V_.signal_to_other_readouts_ )
  {
    // time is one time step longer than the final interval_step to enable sending the
    // unnormalized readout signal one time step in advance so that it is available
    // in the next times step for computing the normalized readout signal
    DelayedRateConnectionEvent readout_signal_unnorm_event;
    readout_signal_unnorm_event.set_coeffarray( readout_signal_unnorm_buffer );
    kernel::manager< EventDeliveryManager >.send_secondary( *this, readout_signal_unnorm_event );
  }
  return;
}

/* ----------------------------------------------------------------
 * Error signal functions
 * ---------------------------------------------------------------- */

void
eprop_readout_bsshslm_2020::compute_error_signal_mean_squared_error( const long lag )
{
  S_.readout_signal_ = S_.readout_signal_unnorm_;
  S_.readout_signal_unnorm_ = S_.v_m_ + P_.E_L_;
  S_.error_signal_ = S_.readout_signal_ - S_.target_signal_;
}

void
eprop_readout_bsshslm_2020::compute_error_signal_cross_entropy( const long lag )
{
  const double norm_rate = B_.normalization_rate_ + S_.readout_signal_unnorm_;
  S_.readout_signal_ = S_.readout_signal_unnorm_ / norm_rate;
  S_.readout_signal_unnorm_ = std::exp( S_.v_m_ + P_.E_L_ );
  S_.error_signal_ = S_.readout_signal_ - S_.target_signal_;
}

/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
eprop_readout_bsshslm_2020::handle( DelayedRateConnectionEvent& e )
{
  const size_t rport = e.get_rport();
  assert( rport < SUP_RATE_RECEPTOR );

  auto it = e.begin();
  assert( it != e.end() );

  const double signal = e.get_weight() * e.get_coeffvalue( it );
  if ( rport == READOUT_SIG )
  {
    B_.normalization_rate_ += signal;
  }
  else if ( rport == TARGET_SIG )
  {
    S_.target_signal_ = signal;
  }

  assert( it == e.end() );
}

void
eprop_readout_bsshslm_2020::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
eprop_readout_bsshslm_2020::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
eprop_readout_bsshslm_2020::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

double
eprop_readout_bsshslm_2020::compute_gradient( std::vector< long >& presyn_isis,
  const long,
  const long t_previous_trigger_spike,
  const double kappa,
  const bool average_gradient )
{
  auto eprop_hist_it = get_eprop_history( t_previous_trigger_spike );

  double grad = 0.0;  // gradient value to be calculated
  double L = 0.0;     // error signal
  double z = 0.0;     // spiking variable
  double z_bar = 0.0; // low-pass filtered spiking variable

  for ( long presyn_isi : presyn_isis )
  {
    z = 1.0; // set spiking variable to 1 for each incoming spike

    for ( long t = 0; t < presyn_isi; ++t )
    {
      assert( eprop_hist_it != eprop_history_.end() );

      L = eprop_hist_it->error_signal_;

      z_bar = V_.P_v_m_ * z_bar + V_.P_z_in_ * z;
      grad += L * z_bar;
      z = 0.0; // set spiking variable to 0 between spikes

      ++eprop_hist_it;
    }
  }
  presyn_isis.clear();

  const long learning_window = kernel::manager< SimulationManager >.get_eprop_learning_window().get_steps();
  if ( average_gradient )
  {
    grad /= learning_window;
  }

  return grad;
}

} // namespace nest
