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
  insert_( names::eprop_history_duration, &eprop_readout::get_eprop_history_duration );
  insert_( names::error_signal, &eprop_readout::get_error_signal_ );
  insert_( names::readout_signal, &eprop_readout::get_readout_signal_ );
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
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , eprop_isi_trace_cutoff_( 1000.0 )
{
}

eprop_readout::State_::State_()
  : error_signal_( 0.0 )
  , readout_signal_( 0.0 )
  , target_signal_( 0.0 )
  , learning_window_signal_( 0.0 )
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
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );
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
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_, node );

  if ( C_m_ <= 0 )
  {
    throw BadProperty( "Membrane capacitance C_m > 0 required." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant tau_m > 0 required." );
  }

  if ( eprop_isi_trace_cutoff_ < 0.0 )
  {
    throw BadProperty( "Cutoff of integration of eprop trace between spikes eprop_isi_trace_cutoff ≥ 0 required." );
  }

  return delta_EL;
}

void
eprop_readout::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, v_m_ + p.E_L_ );
  def< double >( d, names::error_signal, error_signal_ );
  def< double >( d, names::readout_signal, readout_signal_ );
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
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
eprop_readout::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  V_.eprop_isi_trace_cutoff_steps_ = Time( Time::ms( P_.eprop_isi_trace_cutoff_ ) ).get_steps();

  const double dt = Time::get_resolution().get_ms();

  V_.P_v_m_ = std::exp( -dt / P_.tau_m_ );
  V_.P_i_in_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P_v_m_ );
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
eprop_readout::update( Time const& origin, const long from, const long to )
{
  const size_t buffer_size = kernel::manager< ConnectionManager >.get_min_delay();

  std::vector< double > error_signal_buffer( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;

    S_.z_in_ = B_.spikes_.get_value( lag );

    S_.v_m_ = V_.P_i_in_ * S_.i_in_ + S_.z_in_ + V_.P_v_m_ * S_.v_m_;
    S_.v_m_ = std::max( S_.v_m_, P_.V_min_ );

    S_.readout_signal_ = S_.v_m_ + P_.E_L_;
    S_.error_signal_ = S_.readout_signal_ - S_.target_signal_;

    S_.target_signal_ *= S_.learning_window_signal_;
    S_.readout_signal_ *= S_.learning_window_signal_;
    S_.error_signal_ *= S_.learning_window_signal_;

    error_signal_buffer[ lag ] = S_.error_signal_;

    append_new_eprop_history_entry( t );
    write_error_signal_to_history( t, S_.error_signal_ );

    S_.i_in_ = B_.currents_.get_value( lag ) + P_.I_e_;

    B_.logger_.record_data( t );
  }

  LearningSignalConnectionEvent error_signal_event;
  error_signal_event.set_coeffarray( error_signal_buffer );
  kernel::manager< EventDeliveryManager >.send_secondary( *this, error_signal_event );

  return;
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
  if ( rport == LEARNING_WINDOW_SIG )
  {
    S_.learning_window_signal_ = signal;
  }
  else if ( rport == TARGET_SIG )
  {
    S_.target_signal_ = signal;
  }

  assert( it == e.end() );
}

void
eprop_readout::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
eprop_readout::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
eprop_readout::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
eprop_readout::compute_gradient( const long t_spike,
  const long t_spike_previous,
  double& z_previous_buffer,
  double& z_bar,
  double& e_bar,
  double& e_bar_reg,
  double& epsilon,
  double& weight,
  const CommonSynapseProperties& cp,
  WeightOptimizer* optimizer )
{
  double z = 0.0;                // spiking variable
  double z_current_buffer = 1.0; // buffer containing the spike that triggered the current integration
  double L = 0.0;                // error signal
  double grad = 0.0;             // gradient

  const EpropSynapseCommonProperties& ecp = static_cast< const EpropSynapseCommonProperties& >( cp );
  const auto optimize_each_step = ( *ecp.optimizer_cp_ ).optimize_each_step_;

  auto eprop_hist_it = get_eprop_history( t_spike_previous - 1 );

  const long t_compute_until = std::min( t_spike_previous + V_.eprop_isi_trace_cutoff_steps_, t_spike );

  for ( long t = t_spike_previous; t < t_compute_until; ++t, ++eprop_hist_it )
  {
    z = z_previous_buffer;
    z_previous_buffer = z_current_buffer;
    z_current_buffer = 0.0;

    L = eprop_hist_it->error_signal_;

    z_bar = V_.P_v_m_ * z_bar + z;

    if ( optimize_each_step )
    {
      grad = L * z_bar;
      weight = optimizer->optimized_weight( *ecp.optimizer_cp_, t, grad, weight );
    }
    else
    {
      grad += L * z_bar;
    }
  }

  if ( not optimize_each_step )
  {
    weight = optimizer->optimized_weight( *ecp.optimizer_cp_, t_compute_until, grad, weight );
  }

  const long cutoff_to_spike_interval = t_spike - t_compute_until;

  if ( cutoff_to_spike_interval > 0 )
  {
    z_bar *= std::pow( V_.P_v_m_, cutoff_to_spike_interval );
  }
}

} // namespace nest
