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
#include "universal_data_logger_impl.h"

// sli
#include "dictutils.h"

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf_psc_delta_adapt > eprop_iaf_psc_delta_adapt::recordablesMap_;

template <>
void
RecordablesMap< eprop_iaf_psc_delta_adapt >::create()
{
  insert_( names::V_m, &eprop_iaf_psc_delta_adapt::get_V_m_ );
  insert_( names::V_m_pseudo_deriv, &eprop_iaf_psc_delta_adapt::get_V_m_pseudo_deriv_ );
  insert_( names::learning_signal, &eprop_iaf_psc_delta_adapt::get_learning_signal_ );
  insert_( names::adapting_threshold, &eprop_iaf_psc_delta_adapt::get_adapting_threshold_ );
  insert_( names::adaptation, &eprop_iaf_psc_delta_adapt::get_adaptation_ );
}

/* ----------------------------------------------------------------
 * Default constructors for parameters, state, and buffers
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta_adapt::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                  // ms
  , C_m_( 250.0 )                                   // pF
  , t_ref_( 2.0 )                                   // ms
  , E_L_( -70.0 )                                   // mV
  , I_e_( 0.0 )                                     // pA
  , V_th_( -55.0 - E_L_ )                           // mV
  , V_min_( -std::numeric_limits< double >::max() ) // mV
  , adapt_beta_( 1.0 )
  , adapt_tau_( 10.0 ) // ms
  , regression_( true )
{
}

nest::eprop_iaf_psc_delta_adapt::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
  , adaptation_( 0.0 )
  , V_m_pseudo_deriv_( 0.0 )
  , learning_signal_( 0.0 )
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
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, V_th_ + E_L_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::adapt_beta, adapt_beta_ );
  def< double >( d, names::adapt_tau, adapt_tau_ );
  def< bool >( d, names::regression, regression_ );
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

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_m_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::adapt_beta, adapt_beta_, node );
  updateValueParam< double >( d, names::adapt_tau, adapt_tau_, node );
  updateValueParam< bool >( d, names::regression, regression_, node );

  if ( C_m_ <= 0 )
    throw BadProperty( "Capacitance must be > 0." );
  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time must not be negative." );
  if ( tau_m_ <= 0 )
    throw BadProperty( "Membrane time constant must be > 0." );
  if ( adapt_tau_ <= 0 )
    throw BadProperty( "Time constant of threshold adaptation must be > 0." );

  return delta_EL;
}

void
nest::eprop_iaf_psc_delta_adapt::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ );
  def< double >( d, names::adaptation, adaptation_ );
}

void
nest::eprop_iaf_psc_delta_adapt::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  double delta_EL,
  Node* node )
{
  y3_ -= updateValueParam< double >( d, names::V_m, y3_, node ) ? p.E_L_ : delta_EL;

  updateValueParam< double >( d, names::adaptation, adaptation_, node );
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
nest::eprop_iaf_psc_delta_adapt::init_state_( const Node& proto )
{
  const eprop_iaf_psc_delta_adapt& pr = downcast< eprop_iaf_psc_delta_adapt >( proto );
  S_ = pr.S_;
}

void
nest::eprop_iaf_psc_delta_adapt::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
  EpropArchivingNode::clear_history();
  V_.z_ = 0.0;
}

void
nest::eprop_iaf_psc_delta_adapt::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case multimeter connected after Simulate

  const double h = Time::get_resolution().get_ms();

  V_.P33_ = std::exp( -h / P_.tau_m_ ); // alpha
  V_.P30_ = P_.tau_m_ / P_.C_m_ * ( 1.0 - V_.P33_ );
  V_.Pa_ = std::exp( -h / P_.adapt_tau_ );
  V_.P33_complement_ = P_.regression_ ? 1.0 - V_.P33_ : 1.0;
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
}

/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta_adapt::update( Time const& origin, const long from, const long to )
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

    if ( is_time_to_reset )
    {
      S_.y3_ = 0.0;
      S_.adaptation_ = 0.0;
      S_.r_ = 0;
      B_.spikes_.clear(); // includes resize
      V_.z_ = 0.0;
    }

    S_.adaptation_ *= V_.Pa_;
    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + V_.P33_complement_ * B_.spikes_.get_value( lag );

    S_.adaptation_ += V_.z_;
    S_.y3_ -= V_.z_ * P_.V_th_;
    V_.z_ = 0.0;

    S_.y3_ = S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_;

    double thr = P_.V_th_ + P_.adapt_beta_ * S_.adaptation_;

    double v_m = S_.r_ > 0 ? 0.0 : S_.y3_;
    double v_th = S_.r_ > 0 ? P_.V_th_ : thr;
    S_.V_m_pseudo_deriv_ = calculate_v_m_pseudo_deriv( v_m, v_th, P_.V_th_ ); // psi
    write_v_m_pseudo_deriv_to_eprop_history( t + 1, S_.V_m_pseudo_deriv_ );

    if ( S_.y3_ >= thr && S_.r_ == 0 )
    {
      set_spiketime( Time::step( t + 1 ) );
      write_spike_history( t + 1 );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      V_.z_ = 1.0;

      if ( V_.RefractoryCounts_ > 0 )
        S_.r_ = V_.RefractoryCounts_;
    }

    long unsigned int transmission_shift = 3;
    S_.learning_signal_ = get_learning_signal_from_eprop_history( transmission_shift );

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
nest::eprop_iaf_psc_delta_adapt::get_leak_propagator() const
{
  return V_.P33_;
}

double
nest::eprop_iaf_psc_delta_adapt::get_leak_propagator_complement() const
{
  return V_.P33_complement_;
}

double
nest::eprop_iaf_psc_delta_adapt::get_adapt_propagator() const
{
  return V_.Pa_;
}

double
nest::eprop_iaf_psc_delta_adapt::get_adapt_beta() const
{
  return P_.adapt_beta_;
}

std::string
nest::eprop_iaf_psc_delta_adapt::get_eprop_node_type() const
{
  return eprop_node_type_;
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
  write_learning_signal_to_eprop_history( e );
}

void
nest::eprop_iaf_psc_delta_adapt::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace nest
