/*
 *  eprop_iaf_psc_delta.cpp
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
#include "eprop_iaf_psc_delta.h"

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

RecordablesMap< eprop_iaf_psc_delta > eprop_iaf_psc_delta::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf_psc_delta >::create()
{
  insert_( names::learning_signal, &eprop_iaf_psc_delta::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf_psc_delta::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf_psc_delta::get_V_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta::Parameters_::Parameters_()
  : C_m_( 250.0 )
  , c_reg_( 0.0 )
  , E_L_( -70.0 )
  , f_target_( 10.0 )
  , gamma_( 0.3 )
  , I_e_( 0.0 )
  , propagator_idx_( 0 )
  , surrogate_gradient_( "pseudo_derivative" )
  , t_ref_( 2.0 )
  , tau_m_( 10.0 )
  , V_min_( -std::numeric_limits< double >::max() )
  , V_th_( -55.0 - E_L_ )
{
}

nest::eprop_iaf_psc_delta::State_::State_()
  : learning_signal_( 0.0 )
  , r_( 0 )
  , surrogate_gradient_( 0.0 )
  , y0_( 0.0 )
  , y3_( 0.0 )
{
}

nest::eprop_iaf_psc_delta::Buffers_::Buffers_( eprop_iaf_psc_delta& n )
  : logger_( n )
{
}

nest::eprop_iaf_psc_delta::Buffers_::Buffers_( const Buffers_&, eprop_iaf_psc_delta& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Getter and setter functions for parameters and state
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::Parameters_::get( DictionaryDatum& d ) const
{
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
nest::eprop_iaf_psc_delta::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if leak potential is changed, adjust all variables defined relative to it
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  V_th_ -= updateValueParam< double >( d, names::V_th, V_th_, node ) ? E_L_ : delta_EL;
  V_min_ -= updateValueParam< double >( d, names::V_min, V_min_, node ) ? E_L_ : delta_EL;

  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::c_reg, c_reg_, node );
  updateValueParam< double >( d, names::f_target, f_target_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< long >( d, names::propagator_idx, propagator_idx_, node );
  updateValueParam< std::string >( d, names::surrogate_gradient, surrogate_gradient_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );

  if ( C_m_ <= 0 )
    throw BadProperty( "Capacitance must be > 0." );

  if ( propagator_idx_ != 0 and propagator_idx_ != 1 )
    throw BadProperty( "One of two available propagators indexed by 0 and 1 must be selected." );

  if ( surrogate_gradient_ != "pseudo_derivative" )
    throw BadProperty( "One of the available surrogate gradients \"pseudo_derivative\" needs to be selected." );

  if ( tau_m_ <= 0 )
    throw BadProperty( "Membrane time constant must be > 0." );

  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time must not be negative." );

  return delta_EL;
}

void
nest::eprop_iaf_psc_delta::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ );
}

void
nest::eprop_iaf_psc_delta::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  y3_ -= updateValueParam< double >( d, names::V_m, y3_, node ) ? p.E_L_ : delta_EL;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta::eprop_iaf_psc_delta()
  : EpropArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::eprop_iaf_psc_delta::eprop_iaf_psc_delta( const eprop_iaf_psc_delta& n )
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
nest::eprop_iaf_psc_delta::init_state_( const Node& proto )
{
  const eprop_iaf_psc_delta& pr = downcast< eprop_iaf_psc_delta >( proto );
  S_ = pr.S_;
}

void
nest::eprop_iaf_psc_delta::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize

  S_.z_ = 0.0;
}

void
nest::eprop_iaf_psc_delta::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  const double dt = Time::get_resolution().get_ms();

  V_.P33_ = std::exp( -dt / P_.tau_m_ ); // alpha
  V_.P30_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P33_ );

  double propagators[] = { 1.0 - V_.P33_, 1.0 };
  V_.P33_complement_ = propagators[ P_.propagator_idx_ ];

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  write_eprop_parameter_to_map( "leak_propagator", V_.P33_ );
  write_eprop_parameter_to_map( "leak_propagator_complement", V_.P33_complement_ );

  if ( P_.surrogate_gradient_ == "pseudo_derivative" )
    compute_surrogate_gradient = &eprop_iaf_psc_delta::compute_pseudo_derivative;
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::update( Time const& origin, const long from, const long to )
{
  long update_interval_steps = kernel().simulation_manager.get_eprop_update_interval().get_steps();
  bool with_reset = kernel().simulation_manager.get_eprop_reset_neurons_on_update();
  const long shift = static_cast< long >( get_shift() );

  for ( long lag = from; lag < to; ++lag )
  {
    long t = origin.get_steps() + lag;
    bool is_time_to_eprop_update = t % update_interval_steps == shift - 1;

    if ( is_time_to_eprop_update )
    {
      write_firing_rate_reg_to_history( t + 1, P_.f_target_, P_.c_reg_ );
      erase_unneeded_firing_rate_reg_history();
      erase_unneeded_update_history();
      erase_unneeded_eprop_history();
      reset_spike_counter();

      if ( with_reset )
      {
        S_.y3_ = 0.0;
        S_.r_ = 0;
        B_.spikes_.clear(); // includes resize
        S_.z_ = 0.0;
      }
    }

    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + V_.P33_complement_ * B_.spikes_.get_value( lag );

    S_.y3_ -= S_.z_ * P_.V_th_;
    S_.z_ = 0.0;

    S_.y3_ = S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_;

    S_.surrogate_gradient_ = ( this->*compute_surrogate_gradient )();

    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );

    if ( S_.y3_ >= P_.V_th_ and S_.r_ == 0 )
    {
      add_spike_to_counter();

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      S_.z_ = 1.0;

      if ( V_.RefractoryCounts_ > 0 )
        S_.r_ = V_.RefractoryCounts_;
    }

    std::deque< HistEntryEpropArchive >::iterator it_eprop_hist;
    get_eprop_history( t - shift, &it_eprop_hist );
    S_.learning_signal_ = it_eprop_hist->learning_signal_;

    if ( S_.r_ > 0 )
      --S_.r_;

    S_.y0_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Surrogate gradient functions
 * ---------------------------------------------------------------- */

double
nest::eprop_iaf_psc_delta::compute_pseudo_derivative()
{
  double v_m = S_.r_ > 0 ? 0.0 : S_.y3_;
  double psi = P_.gamma_ * std::max( 0.0, 1.0 - std::fabs( ( v_m - P_.V_th_ ) / P_.V_th_ ) ) / P_.V_th_;
  return psi;
}


/* ----------------------------------------------------------------
 * Event handling functions
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::eprop_iaf_psc_delta::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::eprop_iaf_psc_delta::handle( LearningSignalConnectionEvent& e )
{
  std::vector< unsigned int >::iterator it_event = e.begin();
  std::vector< unsigned int >::iterator it_event_end = e.end();

  if ( it_event != it_event_end )
  {
    double time_point = e.get_stamp().get_ms();
    double delay = Time::delay_steps_to_ms( e.get_delay_steps() );
    double weight = e.get_weight();
    double error_signal = e.get_coeffvalue( it_event ); // implicitely decrease access counter

    write_learning_signal_to_history( time_point, delay, weight, error_signal );
  }
}

void
nest::eprop_iaf_psc_delta::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace nest
