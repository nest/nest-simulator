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
  insert_( names::V_m, &eprop_iaf_psc_delta::get_V_m_ );
  insert_( names::V_m_pseudo_deriv, &eprop_iaf_psc_delta::get_V_m_pseudo_deriv_ );
  insert_( names::learning_signal, &eprop_iaf_psc_delta::get_learning_signal_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta::Parameters_::Parameters_()
  : tau_m_( 10.0 )
  , C_m_( 250.0 )
  , c_reg_( 0.0 )
  , t_ref_( 2.0 )
  , f_target_( 10.0 )
  , E_L_( -70.0 )
  , I_e_( 0.0 )
  , V_th_( -55.0 - E_L_ )
  , V_min_( -std::numeric_limits< double >::max() )
  , gamma_( 0.3 )
{
}

nest::eprop_iaf_psc_delta::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
  , V_m_pseudo_deriv_( 0.0 )
  , learning_signal_( 0.0 )
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
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, V_th_ + E_L_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::gamma, gamma_ );
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

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::c_reg, c_reg_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::f_target, f_target_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );

  if ( C_m_ <= 0 )
    throw BadProperty( "Capacitance must be > 0." );
  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time must not be negative." );
  if ( tau_m_ <= 0 )
    throw BadProperty( "Membrane time constant must be > 0." );

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
  EpropArchivingNode::clear_history();

  V_.z_ = 0.0;
}

void
nest::eprop_iaf_psc_delta::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  const double h = Time::get_resolution().get_ms();
  const bool is_regression = kernel().simulation_manager.get_eprop_regression();

  V_.P33_ = std::exp( -h / P_.tau_m_ ); // alpha
  V_.P30_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P33_ );
  V_.P33_complement_ = is_regression ? 1.0 - V_.P33_ : 1.0;
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::update( Time const& origin, const long from, const long to )
{
  long update_interval_steps = kernel().simulation_manager.get_eprop_update_interval_steps();
  bool is_update_interval_reset = kernel().simulation_manager.get_eprop_update_interval_reset();
  long steps = origin.get_steps();
  const int shift = 2; // shift to synchronize factors of weight update

  for ( long lag = from; lag < to; ++lag )
  {
    long t = steps + lag;
    int step_in_current_interval = ( t - shift ) % update_interval_steps;
    bool is_time_to_update = step_in_current_interval == update_interval_steps - 1;
    bool is_time_to_reset = is_update_interval_reset && is_time_to_update;

    if ( is_time_to_update )
    {
      write_firing_rate_reg_to_history( t + 1, P_.f_target_, P_.c_reg_ );
      erase_unneeded_firing_rate_reg_history();
      erase_unneeded_update_history();
      erase_unneeded_eprop_history();
      reset_spike_counter();
    }

    if ( is_time_to_reset )
    {
      S_.y3_ = 0.0;
      S_.r_ = 0;
      B_.spikes_.clear(); // includes resize
      V_.z_ = 0.0;
    }

    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + V_.P33_complement_ * B_.spikes_.get_value( lag );

    S_.y3_ -= V_.z_ * P_.V_th_;
    V_.z_ = 0.0;

    S_.y3_ = S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_;

    double v_m = S_.r_ > 0 ? 0.0 : S_.y3_;
    double psi = P_.gamma_ * std::max( 0.0, 1.0 - std::fabs( ( v_m - P_.V_th_ ) / P_.V_th_ ) ) / P_.V_th_;

    S_.V_m_pseudo_deriv_ = psi;
    write_v_m_pseudo_deriv_to_history( t + 1, psi );

    if ( S_.y3_ >= P_.V_th_ && S_.r_ == 0 )
    {
      set_spiketime( Time::step( t + 1 ) );
      add_spike_to_counter();

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      V_.z_ = 1.0;

      if ( V_.RefractoryCounts_ > 0 )
        S_.r_ = V_.RefractoryCounts_;
    }

    std::deque< histentry_eprop_archive >::iterator it_eprop_hist;
    get_eprop_history( t - shift, &it_eprop_hist );
    S_.learning_signal_ = it_eprop_hist->learning_signal_;

    if ( S_.r_ > 0 )
      --S_.r_;

    S_.y0_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( t );
  }
}

/* ----------------------------------------------------------------
 * Getter functions for private variables and parameters
 *----------------------------------------------------------------- */

double
nest::eprop_iaf_psc_delta::get_leak_propagator() const
{
  return V_.P33_;
}

double
nest::eprop_iaf_psc_delta::get_leak_propagator_complement() const
{
  return V_.P33_complement_;
}

std::string
nest::eprop_iaf_psc_delta::get_eprop_node_type() const
{
  return eprop_node_type_;
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
  write_learning_signal_to_history( e );
}

void
nest::eprop_iaf_psc_delta::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace nest
