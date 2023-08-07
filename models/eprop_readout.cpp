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
#include "universal_data_logger_impl.h"

// sli
#include "dictutils.h"

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_readout > eprop_readout::recordablesMap_;

template <>
void
RecordablesMap< eprop_readout >::create()
{
  insert_( names::V_m, &eprop_readout::get_V_m_ );
  insert_( names::readout_signal, &eprop_readout::get_readout_signal_ );
  insert_( names::target_signal, &eprop_readout::get_target_signal_ );
  insert_( names::error_signal, &eprop_readout::get_error_signal_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

nest::eprop_readout::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                  // ms
  , C_m_( 250.0 )                                   // pF
  , E_L_( 0.0 )                                     // mV
  , I_e_( 0.0 )                                     // pA
  , V_min_( -std::numeric_limits< double >::max() ) // mV
  , start_learning_( 0.0 )                          // ms
  , regression_( true )
{
}

nest::eprop_readout::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , readout_signal_( 0.0 )
  , target_signal_( 0.0 )
  , error_signal_( 0.0 )
{
}

nest::eprop_readout::Buffers_::Buffers_( eprop_readout& n )
  : logger_( n )
{
}

nest::eprop_readout::Buffers_::Buffers_( const Buffers_&, eprop_readout& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
nest::eprop_readout::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::start_learning, start_learning_ );
  def< bool >( d, names::regression, regression_ );
}

double
nest::eprop_readout::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if leak potential is changed, adjust all variables defined relative to it
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  V_min_ -= updateValueParam< double >( d, names::V_min, V_min_, node ) ? E_L_ : delta_EL;

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::start_learning, start_learning_, node );
  updateValueParam< bool >( d, names::regression, regression_, node );

  if ( C_m_ <= 0 )
    throw BadProperty( "Capacitance must be > 0." );
  if ( tau_m_ <= 0 )
    throw BadProperty( "Membrane time constant must be > 0." );

  return delta_EL;
}

void
nest::eprop_readout::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ );
}

void
nest::eprop_readout::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  y3_ -= updateValueParam< double >( d, names::V_m, y3_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::eprop_readout::eprop_readout()
  : EpropArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::eprop_readout::eprop_readout( const eprop_readout& n )
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
nest::eprop_readout::init_state_( const Node& proto )
{
  const eprop_readout& pr = downcast< eprop_readout >( proto );
  S_ = pr.S_;
}

void
nest::eprop_readout::init_buffers_()
{
  B_.delayed_rates_.clear();       // includes resize
  B_.normalization_rates_.clear(); // includes resize
  B_.spikes_.clear();              // includes resize
  B_.currents_.clear();            // includes resize
  B_.logger_.reset();              // includes resize
  ArchivingNode::clear_history();
}

void
nest::eprop_readout::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  const double h = Time::get_resolution().get_ms();

  V_.P33_ = std::exp( -h / P_.tau_m_ );
  V_.P30_ = 1.0 / P_.C_m_ * ( 1.0 - V_.P33_ ) * P_.tau_m_;
  V_.P33_complement_ = 1.0 - V_.P33_;
  V_.start_learning_step_ = Time( Time::ms( std::max( P_.start_learning_, 0.0 ) ) ).get_steps();
  V_.target_signal_ = 0.0;
  V_.readout_signal_unnorm_ = 0.0;
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::eprop_readout::update_( Time const& origin, const long from, const long to )
{
  long update_interval_steps = kernel().simulation_manager.get_eprop_update_interval_steps();
  bool is_update_interval_reset = kernel().simulation_manager.get_eprop_update_interval_reset();
  long steps = origin.get_steps();
  const int shift = 3; // shift to synchronize factors of weight update

  const size_t buffer_size = kernel().connection_manager.get_min_delay();

  std::vector< double > error_signal_buffer( buffer_size, 0.0 );
  std::vector< double > readout_signal_unnorm_buffer( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    long t = steps + lag;
    int step_in_current_interval = ( t - shift ) % update_interval_steps;
    bool is_time_to_update = step_in_current_interval == 0;
    bool is_time_to_reset = is_update_interval_reset && is_time_to_update;
    bool is_learning_window =
      V_.start_learning_step_ <= step_in_current_interval && step_in_current_interval <= update_interval_steps - 1;
    bool is_extended_learning_window = step_in_current_interval == V_.start_learning_step_ - 1 || is_learning_window;

    if ( is_time_to_reset )
    {
      S_.y3_ = 0.0;
      B_.spikes_.clear(); // includes resize
    }

    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + V_.P33_complement_ * B_.spikes_.get_value( lag );
    S_.y3_ = S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_;

    double v_m = S_.y3_ + P_.E_L_;

    double norm_rate = P_.regression_ ? 1.0 : B_.normalization_rates_.get_value( lag ) + V_.readout_signal_unnorm_;
    S_.readout_signal_ = is_learning_window ? V_.readout_signal_unnorm_ / norm_rate : 0.0;
    S_.error_signal_ = is_learning_window ? S_.readout_signal_ - S_.target_signal_ : 0.0;
    S_.target_signal_ = is_extended_learning_window ? V_.target_signal_ : 0.0;

    V_.readout_signal_unnorm_ = is_extended_learning_window ? ( P_.regression_ ? v_m : std::exp( v_m ) ) : 0.0;
    if ( !P_.regression_ )
      readout_signal_unnorm_buffer[ lag ] = V_.readout_signal_unnorm_;

    error_signal_buffer[ lag ] = S_.error_signal_;

    write_error_signal_to_eprop_history( t, S_.error_signal_ );

    V_.target_signal_ = B_.delayed_rates_.get_value( lag );
    S_.y0_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( t );
  }

  LearningSignalConnectionEvent error_signal_event;
  error_signal_event.set_coeffarray( error_signal_buffer );
  kernel().event_delivery_manager.send_secondary( *this, error_signal_event );

  if ( !P_.regression_ )
  {
    // time is one time step longer than the final step_in_current_interval to enable sending the
    // unnormalized readout signal one time step in advance so that it is available
    // in the next times step for computing the normalized readout signal
    DelayedRateConnectionEvent readout_signal_unnorm_event;
    readout_signal_unnorm_event.set_coeffarray( readout_signal_unnorm_buffer );
    kernel().event_delivery_manager.send_secondary( *this, readout_signal_unnorm_event );
  }
  return;
}

/* ----------------------------------------------------------------
 * Getter functions for private variables and parameters
 *----------------------------------------------------------------- */

std::string
nest::eprop_readout::get_eprop_node_type() const
{
  return eprop_node_type_;
}

/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
nest::eprop_readout::handle( DelayedRateConnectionEvent& e )
{
  size_t rport = e.get_rport();
  const long delay = e.get_delay_steps();
  size_t i = 0;

  assert( rport < 3 );

  std::vector< unsigned int >::iterator it = e.begin();

  while ( it != e.end() )
  {
    double signal = e.get_weight() * e.get_coeffvalue( it ); // get_coeffvalue advances iterator
    size_t t = delay + i;

    if ( rport == READOUT_SIG - MIN_RATE_RECEPTOR )
    {
      B_.normalization_rates_.add_value( t, signal );
    }
    else if ( rport == TARGET_SIG - MIN_RATE_RECEPTOR )
    {
      B_.delayed_rates_.add_value( t, signal );
    }

    ++i;
  }
}

void
nest::eprop_readout::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::eprop_readout::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::eprop_readout::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace nest
