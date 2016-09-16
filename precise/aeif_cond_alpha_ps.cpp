/*
 *  aeif_cond_alpha_ps.cpp
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

#include "aeif_cond_alpha_ps.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::aeif_cond_alpha_ps >
  nest::aeif_cond_alpha_ps::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< nest::aeif_cond_alpha_ps >::create()
{
  insert_( names::V_m,
    &nest::aeif_cond_alpha_ps::
      get_y_elem_< nest::aeif_cond_alpha_ps::State_::V_M > );
  insert_( names::g_ex,
    &nest::aeif_cond_alpha_ps::
      get_y_elem_< nest::aeif_cond_alpha_ps::State_::G_EXC > );
  insert_( names::g_in,
    &nest::aeif_cond_alpha_ps::
      get_y_elem_< nest::aeif_cond_alpha_ps::State_::G_INH > );
  insert_( names::w,
    &nest::aeif_cond_alpha_ps::
      get_y_elem_< nest::aeif_cond_alpha_ps::State_::W > );
}
}


/* ----------------------------------------------------------------
 * Dynamics for gsl_odeiv
 * ---------------------------------------------------------------- */

extern "C" int
nest::aeif_cond_alpha_ps_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // a shorthand
  typedef nest::aeif_cond_alpha_ps::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::aeif_cond_alpha_ps& node =
    *( reinterpret_cast< nest::aeif_cond_alpha_ps* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double_t& V = y[ S::V_M ];
  const double_t& dg_ex = y[ S::DG_EXC ];
  const double_t& g_ex = y[ S::G_EXC ];
  const double_t& dg_in = y[ S::DG_INH ];
  const double_t& g_in = y[ S::G_INH ];
  const double_t& w = y[ S::W ];

  const double_t I_syn_exc = g_ex * ( V - node.P_.E_ex );
  const double_t I_syn_inh = g_in * ( V - node.P_.E_in );

  // We pre-compute the argument of the exponential
  const double_t exp_arg = ( V - node.P_.V_th ) / node.P_.Delta_T;

  // Upper bound for exponential argument to avoid numerical instabilities
  const double_t MAX_EXP_ARG = 10.;

  // If the argument is too large, we clip it.
  const double_t I_spike =
    node.P_.Delta_T * std::exp( std::min( exp_arg, MAX_EXP_ARG ) );

  // dv/dt
  f[ S::V_M ] =
    ( -node.P_.g_L * ( ( V - node.P_.E_L ) - I_spike ) - I_syn_exc - I_syn_inh
      - w + node.P_.I_e + node.B_.I_stim_ ) / node.P_.C_m;

  f[ S::DG_EXC ] = -dg_ex / node.P_.tau_syn_ex;
  f[ S::G_EXC ] =
    dg_ex - g_ex / node.P_.tau_syn_ex; // Synaptic Conductance (nS)

  f[ S::DG_INH ] = -dg_in / node.P_.tau_syn_in;
  f[ S::G_INH ] =
    dg_in - g_in / node.P_.tau_syn_in; // Synaptic Conductance (nS)

  // Adaptation current w.
  f[ S::W ] = ( node.P_.a * ( V - node.P_.E_L ) - w ) / node.P_.tau_w;

  return GSL_SUCCESS;
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::aeif_cond_alpha_ps::Parameters_::Parameters_()
  : V_peak_( 0.0 )    // mV, should not be larger that V_th+10
  , V_reset_( -60.0 ) // mV
  , t_ref_( 0.0 )     // ms
  , g_L( 30.0 )       // nS
  , C_m( 281.0 )      // pF
  , E_L( -70.6 )      // mV
  , E_ex( 0.0 )       // mV
  , E_in( -85.0 )     // mV
  , Delta_T( 2.0 )    // mV
  , tau_w( 144.0 )    // ms
  , a( 4.0 )          // nS
  , b( 80.5 )         // pA
  , V_th( -50.4 )     // mV
  , tau_syn_ex( 0.2 ) // ms
  , tau_syn_in( 2.0 ) // ms
  , I_e( 0.0 )        // pA
  , gsl_error_tol( 1e-6 )
{
}

nest::aeif_cond_alpha_ps::State_::State_( const Parameters_& p )
  : r_( 0 )
  , r_offset_( 0. )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = 0;
}

nest::aeif_cond_alpha_ps::State_::State_( const State_& s )
  : r_( s.r_ )
  , r_offset_( s.r_offset_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
}

nest::aeif_cond_alpha_ps::State_& nest::aeif_cond_alpha_ps::State_::operator=(
  const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
  r_ = s.r_;
  r_offset_ = s.r_offset_;
  return *this;
}


/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha_ps::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::tau_syn_ex, tau_syn_ex );
  def< double >( d, names::tau_syn_in, tau_syn_in );
  def< double >( d, names::a, a );
  def< double >( d, names::b, b );
  def< double >( d, names::Delta_T, Delta_T );
  def< double >( d, names::tau_w, tau_w );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::V_peak, V_peak_ );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );
}

void
nest::aeif_cond_alpha_ps::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_peak, V_peak_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::E_L, E_L );
  updateValue< double >( d, names::E_ex, E_ex );
  updateValue< double >( d, names::E_in, E_in );
  updateValue< double >( d, names::V_reset, V_reset_ );

  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::g_L, g_L );

  updateValue< double >( d, names::tau_syn_ex, tau_syn_ex );
  updateValue< double >( d, names::tau_syn_in, tau_syn_in );

  updateValue< double >( d, names::a, a );
  updateValue< double >( d, names::b, b );
  updateValue< double >( d, names::Delta_T, Delta_T );
  updateValue< double >( d, names::tau_w, tau_w );

  updateValue< double >( d, names::I_e, I_e );

  updateValue< double >( d, names::gsl_error_tol, gsl_error_tol );

  if ( V_peak_ <= V_th )
    throw BadProperty( "V_peak must be larger than threshold." );

  if ( V_reset_ >= V_peak_ )
    throw BadProperty( "Ensure that: V_reset < V_peak ." );

  if ( C_m <= 0 )
    throw BadProperty( "Capacitance must be strictly positive." );

  if ( g_L <= 0 )
    throw BadProperty( "Leak conductance must be strictly positive." );

  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time cannot be negative." );

  if ( tau_syn_ex <= 0 || tau_syn_in <= 0 || tau_w <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );

  if ( gsl_error_tol <= 0. )
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
}

void
nest::aeif_cond_alpha_ps::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::g_ex, y_[ G_EXC ] );
  def< double >( d, names::dg_ex, y_[ DG_EXC ] );
  def< double >( d, names::g_in, y_[ G_INH ] );
  def< double >( d, names::dg_in, y_[ DG_INH ] );
  def< double >( d, names::w, y_[ W ] );
}

void
nest::aeif_cond_alpha_ps::State_::set( const DictionaryDatum& d,
  const Parameters_& )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );
  updateValue< double >( d, names::g_ex, y_[ G_EXC ] );
  updateValue< double >( d, names::dg_ex, y_[ DG_EXC ] );
  updateValue< double >( d, names::g_in, y_[ G_INH ] );
  updateValue< double >( d, names::dg_in, y_[ DG_INH ] );
  updateValue< double >( d, names::w, y_[ W ] );

  if ( y_[ G_EXC ] < 0 || y_[ G_INH ] < 0 )
    throw BadProperty( "Conductances must not be negative." );
}

nest::aeif_cond_alpha_ps::Buffers_::Buffers_( aeif_cond_alpha_ps& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::aeif_cond_alpha_ps::Buffers_::Buffers_( const Buffers_&,
  aeif_cond_alpha_ps& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::aeif_cond_alpha_ps::aeif_cond_alpha_ps()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::aeif_cond_alpha_ps::aeif_cond_alpha_ps( const aeif_cond_alpha_ps& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha_ps::init_state_( const Node& proto )
{
  const aeif_cond_alpha_ps& pr = downcast< aeif_cond_alpha_ps >( proto );
  S_ = pr.S_;
}

void
nest::aeif_cond_alpha_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  B_.currents_.clear(); // includes resize
  Archiving_Node::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();

  // We must integrate this model with high-precision to obtain decent results
  B_.IntegrationStep_ = std::min( 0.01, B_.step_ );

  if ( B_.s_ == 0 )
    B_.s_ =
      gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_step_reset( B_.s_ );

  if ( B_.c_ == 0 )
    B_.c_ = gsl_odeiv_control_yp_new( P_.gsl_error_tol, P_.gsl_error_tol );
  else
    gsl_odeiv_control_init(
      B_.c_, P_.gsl_error_tol, P_.gsl_error_tol, 0.0, 1.0 );

  if ( B_.e_ == 0 )
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_evolve_reset( B_.e_ );

  B_.sys_.function = aeif_cond_alpha_ps_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.;
}

void
nest::aeif_cond_alpha_ps::calibrate()
{
  B_.logger_
    .init(); // ensures initialization in case mm connected after Simulate

  V_.g0_ex_ = 1.0 * numerics::e / P_.tau_syn_ex;
  V_.g0_in_ = 1.0 * numerics::e / P_.tau_syn_in;
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  V_.RefractoryOffset_ =
    P_.t_ref_ - V_.RefractoryCounts_ * Time::get_resolution().get_ms();
  assert( V_.RefractoryCounts_
    >= 0 ); // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryOffset_ >= 0. );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha_ps::interpolate_( double& t, double t_old )
{
  // find the exact time when the threshold was crossed
  double dt_crossing = ( P_.V_peak_ - S_.y_old_[ State_::V_M ] ) * ( t - t_old )
    / ( S_.y_[ State_::V_M ] - S_.y_old_[ State_::V_M ] );

  // reset V_m and set the other variables correctly
  S_.y_[ State_::V_M ] = P_.V_reset_;
  for ( int i = 1; i < State_::STATE_VEC_SIZE; ++i )
  {
    S_.y_[ i ] = S_.y_old_[ i ]
      + ( S_.y_[ i ] - S_.y_old_[ i ] ) / ( t - t_old ) * dt_crossing;
  }
  S_.y_[ State_::W ] += P_.b; // spike-driven adaptation

  t = t_old + dt_crossing;
}

void
nest::aeif_cond_alpha_ps::spiking_( const long_t T,
  const long_t lag,
  const double t )
{
  // spike event
  const double_t offset = B_.step_ - t;
  set_spiketime( Time::step( T + 1 ), offset );
  SpikeEvent se;
  se.set_offset( offset );
  kernel().event_delivery_manager.send( *this, se, lag );

  // refractoriness
  if ( P_.t_ref_ > 0. )
  {
    S_.r_ = V_.RefractoryCounts_;
    S_.r_offset_ = V_.RefractoryOffset_ - offset;
    if ( S_.r_offset_ < 0. )
    {
      if ( S_.r_ > 0 )
      {
        --S_.r_;
        S_.r_offset_ = B_.step_ + S_.r_offset_;
      }
      else
        S_.r_offset_ = t + V_.RefractoryOffset_;
    }
    B_.events_.set_refractory( T + S_.r_, B_.step_ - S_.r_offset_ );
  }
}

void
nest::aeif_cond_alpha_ps::update( const Time& origin,
  const long_t from,
  const long_t to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );
  assert( State_::V_M == 0 );

  double t, t_old, t_next_event, spike_in( 0. ), spike_ex( 0. );

  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
    B_.events_.prepare_delivery();

  /* Neurons may have been initialized to superthreshold potentials.
     We need to check for this here and issue spikes at the beginning of
     the interval.
  */
  if ( S_.y_[ State_::V_M ] >= P_.V_peak_ )
  {
    S_.y_[ State_::V_M ] = P_.V_reset_;
    S_.y_[ State_::W ] += P_.b;
    const double_t init_offset =
      B_.step_ * ( 1 - std::numeric_limits< double_t >::epsilon() );
    set_spiketime( Time::step( origin.get_steps() + from + 1 ), init_offset );
    SpikeEvent se;
    se.set_offset(
      B_.step_ * ( 1 - std::numeric_limits< double_t >::epsilon() ) );
    kernel().event_delivery_manager.send( *this, se, from );
  }

  for ( long_t lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long_t T = origin.get_steps() + lag;
    t = 0.;
    t_next_event = 0.;

    if ( S_.r_ > 0 )
      --S_.r_;

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // gsl_odeiv_evolve_apply performs only a single numerical
    // integration step, starting from t and bounded by step;
    // the while-loop ensures integration over the whole simulation
    // step (0, step] if more than one integration step is needed due
    // to a small integration step size;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t

    while ( t < B_.step_ )
    {
      // store the previous values of the state variables, and t
      std::copy(
        S_.y_, S_.y_ + sizeof( S_.y_ ) / sizeof( S_.y_[ 0 ] ), S_.y_old_ );
      t_old = t;
      B_.events_.get_next_event(
        T, t_next_event, spike_in, spike_ex, B_.step_ );

      while ( t < t_next_event )
      {
        const int status = gsl_odeiv_evolve_apply( B_.e_,
          B_.c_,
          B_.s_,
          &B_.sys_,             // system of ODE
          &t,                   // from t
          t_next_event,         // to t <= t_next_event
          &B_.IntegrationStep_, // integration step size
          S_.y_ );              // neuronal state

        // checks
        if ( status != GSL_SUCCESS )
          throw GSLSolverFailure( get_name(), status );
        if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6
          || S_.y_[ State_::W ] > 1e6 )
          throw NumericalInstability( get_name() );
      }

      // check refractoriness
      if ( S_.r_ > 0 || S_.r_offset_ > 0. )
        S_.y_[ State_::V_M ] = P_.V_reset_; // only V_m is frozen
      else if ( S_.y_[ State_::V_M ] >= P_.V_peak_ )
      {
        // spiking: find the exact threshpassing, then emit the spike
        interpolate_( t, t_old );
        spiking_( T, lag, t );
      }

      // reset refractory offset once refractory period is elapsed
      if ( S_.r_ == 0
        && std::abs( t - S_.r_offset_ )
          < std::numeric_limits< double >::epsilon() )
        S_.r_offset_ = 0.;

      if ( t == t_next_event )
      {
        S_.y_[ State_::DG_EXC ] += spike_ex * V_.g0_ex_;
        S_.y_[ State_::DG_INH ] += spike_in * V_.g0_in_;
        spike_ex = 0.;
        spike_in = 0.;
      }
    }

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::aeif_cond_alpha_ps::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  const long_t Tdeliver = e.get_stamp().get_steps() + e.get_delay() - 1;
  B_.events_.add_spike(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    Tdeliver,
    e.get_offset(),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::aeif_cond_alpha_ps::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::aeif_cond_alpha_ps::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
