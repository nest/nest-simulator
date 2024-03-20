/*
 *  eprop_readout.cpp
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
#include "eprop_readout.h"

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
register_eprop_readout( const std::string& name )
{
  register_node_model< eprop_readout >( name );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_readout > eprop_readout::recordablesMap_;

template <>
void
RecordablesMap< eprop_readout >::create()
{
  insert_( names::error_signal, &eprop_readout::get_error_signal_ );
  insert_( names::readout_signal, &eprop_readout::get_readout_signal_ );
  insert_( names::readout_signal_unnorm, &eprop_readout::get_readout_signal_unnorm_ );
  insert_( names::target_signal, &eprop_readout::get_target_signal_ );
  insert_( names::V_m, &eprop_readout::get_v_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

eprop_readout::Parameters_::Parameters_()
  : C_m_( 250.0 )
  , E_L_( 0.0 )
  , I_e_( 0.0 )
  , loss_( "mean_squared_error" )
  , regular_spike_arrival_( true )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , eprop_isi_trace_cutoff_( std::numeric_limits< long >::max() )
  , eta_( 0.0 )
{
}

eprop_readout::State_::State_()
  : error_signal_( 0.0 )
  , readout_signal_( 0.0 )
  , readout_signal_unnorm_( 0.0 )
  , target_signal_( 0.0 )
  , i_in_( 0.0 )
  , v_m_( 0.0 )
  , z_in_( 0.0 )
{
}

eprop_readout::Buffers_::Buffers_( eprop_readout& n )
  : logger_( n )
{
}

eprop_readout::Buffers_::Buffers_( const Buffers_&, eprop_readout& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
eprop_readout::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< std::string >( d, names::loss, loss_ );
  def< bool >( d, names::regular_spike_arrival, regular_spike_arrival_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< long >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );
  def< double >( d, names::eta, eta_ );
}

double
eprop_readout::Parameters_::set( const DictionaryDatum& d, Node* node )
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
  updateValueParam< long >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_, node );
  updateValueParam< double >( d, names::eta, eta_, node );

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

  if ( eprop_isi_trace_cutoff_ < 0 )
  {
    throw BadProperty( "Cutoff of integration of eprop trace between spikes eprop_isi_trace_cutoff ≥ 0 required." );
  }

  if ( eta_ < 0 )
  {
    throw BadProperty( "Learning rate eta ≥ 0 required." );
  }

  return delta_EL;
}

void
eprop_readout::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::error_signal, error_signal_ );
  def< double >( d, names::readout_signal, readout_signal_ );
  def< double >( d, names::readout_signal_unnorm, readout_signal_unnorm_ );
  def< double >( d, names::target_signal, target_signal_ );
}

void
eprop_readout::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  v_m_ -= updateValueParam< double >( d, names::V_m, v_m_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

eprop_readout::eprop_readout()
  : EpropArchivingNodeReadout()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

eprop_readout::eprop_readout( const eprop_readout& n )
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
eprop_readout::init_buffers_()
{
  B_.normalization_rate_ = 0;
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_readout::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  if ( P_.loss_ == "mean_squared_error" )
  {
    compute_error_signal = &eprop_readout::compute_error_signal_mean_squared_error;
    V_.signal_to_other_readouts_ = false;
  }
  else if ( P_.loss_ == "cross_entropy" )
  {
    compute_error_signal = &eprop_readout::compute_error_signal_cross_entropy;
    V_.signal_to_other_readouts_ = true;
  }

  const double dt = Time::get_resolution().get_ms();

  const double kappa = std::exp( -dt / P_.tau_m_ );

  V_.P_v_m_ = kappa;
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - kappa );
  V_.P_z_in_ = P_.regular_spike_arrival_ ? 1.0 : 1.0 - V_.P_v_m_;
}

bool
eprop_readout::is_eprop_recurrent_node() const
{
  return false;
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_readout::update( Time const& origin, const long from, const long to )
{
  const size_t buffer_size = kernel().connection_manager.get_min_delay();

  std::vector< double > error_signal_buffer( buffer_size, 0.0 );
  std::vector< double > readout_signal_unnorm_buffer( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + V_.P_z_in_ * S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    ( this->*compute_error_signal )( lag );

    if ( not S_.learning_window_flag_ )
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

    write_error_signal_to_history( t, S_.error_signal_ );

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }

  LearningSignalConnectionEvent error_signal_event;
  error_signal_event.set_coeffarray( error_signal_buffer );
  kernel().event_delivery_manager.send_secondary( *this, error_signal_event );

  if ( V_.signal_to_other_readouts_ )
  {
    // time is one time step longer than the final interval_step to enable sending the
    // unnormalized readout signal one time step in advance so that it is available
    // in the next times step for computing the normalized readout signal
    DelayedRateConnectionEvent readout_signal_unnorm_event;
    readout_signal_unnorm_event.set_coeffarray( readout_signal_unnorm_buffer );
    kernel().event_delivery_manager.send_secondary( *this, readout_signal_unnorm_event );
  }
  return;
}

/* ----------------------------------------------------------------
 * Error signal functions
 * ---------------------------------------------------------------- */

void
eprop_readout::compute_error_signal_mean_squared_error( const long lag )
{
  S_.readout_signal_ = S_.v_m_ + P_.E_L_;
  S_.error_signal_ = S_.readout_signal_ - S_.target_signal_;
}

void
eprop_readout::compute_error_signal_cross_entropy( const long lag )
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
eprop_readout::handle( DelayedRateConnectionEvent& e )
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
  else if ( rport == LEARNING_WINDOW_SIG )
  {
    S_.learning_window_flag_ = true;
    if ( std::abs( signal ) < 1.0 )
    {
      S_.learning_window_flag_ = false;
    }
  }

  assert( it == e.end() );
}

void
eprop_readout::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
eprop_readout::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
eprop_readout::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
eprop_readout::compute_gradient( const long t_spike,
  const long t_previous_spike,
  long& t,
  double& previous_z_buffer,
  double& z_bar,
  double& e_bar,
  double& epsilon,
  double& avg_e,
  double& weight,
  const double kappa,
  const CommonSynapseProperties& cp,
  WeightOptimizer* optimizer )
{
  double z = 0.0;    // spiking variable
  double L = 0.0;    // error signal
  double grad = 0.0; // gradient

  const EpropSynapseCommonProperties& ecp = static_cast< const EpropSynapseCommonProperties& >( cp );

  const long learning_window =
    ecp.average_gradient_ ? kernel().simulation_manager.get_eprop_learning_window().get_steps() : 1;

  auto eprop_hist_it = get_eprop_history( t_previous_spike - 1 );

  bool pre = true;

  while ( t < std::min( t_spike, t_previous_spike + P_.eprop_isi_trace_cutoff_ ) )
  {
    if ( pre )
    {
      z = previous_z_buffer;
      previous_z_buffer = 1.0;
      pre = false;
    }
    else
    {
      if ( previous_z_buffer == 1.0 )
      {
        z = 1.0;
        previous_z_buffer = 0.0;
      }
      else
      {
        z = 0.0;
      }
    }

    L = eprop_hist_it->error_signal_;

    z_bar = V_.P_v_m_ * z_bar + V_.P_z_in_ * z;
    grad = L * z_bar;
    grad /= learning_window;

    weight = optimizer->optimized_weight( *ecp.optimizer_cp_, t, grad, weight );

    ++eprop_hist_it;
    ++t;
  }

  int power = t_spike - ( t_previous_spike + P_.eprop_isi_trace_cutoff_ );

  if ( power > 0 )
  {
    z_bar *= std::pow( V_.P_v_m_, power );
  }
}

} // namespace nest
