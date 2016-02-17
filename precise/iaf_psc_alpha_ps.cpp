/*
 *  iaf_psc_alpha_ps.cpp
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

#include "iaf_psc_alpha_ps.h"

#ifdef HAVE_GSL_1_11

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_psc_alpha_ps > nest::iaf_psc_alpha_ps::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< nest::iaf_psc_alpha_ps >::create()
{
  insert_(
    names::V_m, &nest::iaf_psc_alpha_ps::get_y_elem_< nest::iaf_psc_alpha_ps::State_::V_M > );
  insert_(
    names::I_ex, &nest::iaf_psc_alpha_ps::get_y_elem_< nest::iaf_psc_alpha_ps::State_::I_EXC > );
  insert_(
    names::I_in, &nest::iaf_psc_alpha_ps::get_y_elem_< nest::iaf_psc_alpha_ps::State_::I_INH > );
}
}

/* ----------------------------------------------------------------
 * Dynamics for gsl_odeiv
 * ---------------------------------------------------------------- */

extern "C" int
nest::iaf_psc_alpha_ps_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::iaf_psc_alpha_ps::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::iaf_psc_alpha_ps& node = *( reinterpret_cast< nest::iaf_psc_alpha_ps* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double_t& V = y[ S::V_M ];
  const double_t& di_exc = y[ S::DI_EXC ];
  const double_t& i_exc = y[ S::I_EXC ];
  const double_t& di_inh = y[ S::DI_INH ];
  const double_t& i_inh = y[ S::I_INH ];

  // dv/dt
  f[ S::V_M ] = ( -node.P_.g_L * ( V - node.P_.E_L ) + i_exc - i_inh + node.P_.I_e
                  + node.B_.I_stim_ ) / node.P_.C_m;

  f[ S::DI_EXC ] = -di_exc / node.P_.tau_syn_exc;
  f[ S::I_EXC ] = di_exc - i_exc / node.P_.tau_syn_exc; // Synaptic Conductance (nS)

  f[ S::DI_INH ] = -di_inh / node.P_.tau_syn_inh;
  f[ S::I_INH ] = di_inh - i_inh / node.P_.tau_syn_inh; // Synaptic Conductance (nS)

  return GSL_SUCCESS;
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_ps::Parameters_::Parameters_()
  : V_reset_( -60.0 )  // mV
  , t_ref_( 2.0 )      // ms
  , g_L( 16.6667 )     // nS
  , C_m( 281.0 )       // pF
  , E_ex( 0.0 )        // mV
  , E_in( -85.0 )      // mV
  , E_L( -70. )        // mV
  , V_th( -55 )        // mV
  , tau_syn_exc( 0.2 ) // ms
  , tau_syn_inh( 2.0 ) // ms
  , I_e( 0.0 )         // pA
  , gsl_error_tol( 1e-6 )
{
}

nest::iaf_psc_alpha_ps::State_::State_( const Parameters_& p )
  : r_( 0 )
  , r_offset_( 0. )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = 0;
}

nest::iaf_psc_alpha_ps::State_::State_( const State_& s )
  : r_( s.r_ )
  , r_offset_( s.r_offset_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
}

nest::iaf_psc_alpha_ps::State_& nest::iaf_psc_alpha_ps::State_::operator=( const State_& s )
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
nest::iaf_psc_alpha_ps::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::tau_syn_ex, tau_syn_exc );
  def< double >( d, names::tau_syn_in, tau_syn_inh );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );
}

void
nest::iaf_psc_alpha_ps::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::E_L, E_L );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::E_ex, E_ex );
  updateValue< double >( d, names::E_in, E_in );

  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::g_L, g_L );

  updateValue< double >( d, names::tau_syn_ex, tau_syn_exc );
  updateValue< double >( d, names::tau_syn_in, tau_syn_inh );

  updateValue< double >( d, names::I_e, I_e );

  updateValue< double >( d, names::gsl_error_tol, gsl_error_tol );

  if ( V_reset_ >= V_th )
    throw BadProperty( "Reset potential must be smaller than threshold." );

  if ( C_m <= 0 )
    throw BadProperty( "Capacitance must be strictly positive." );

  if ( g_L <= 0 )
    throw BadProperty( "Leak conductance must be strictly positive." );

  if ( tau_syn_exc <= 0 || tau_syn_inh <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );

  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time cannot be negative." );

  if ( gsl_error_tol <= 0. )
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
}

void
nest::iaf_psc_alpha_ps::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::g_ex, y_[ I_EXC ] );
  def< double >( d, names::dg_ex, y_[ DI_EXC ] );
  def< double >( d, names::g_in, y_[ I_INH ] );
  def< double >( d, names::dg_in, y_[ DI_INH ] );
}

void
nest::iaf_psc_alpha_ps::State_::set( const DictionaryDatum& d, const Parameters_& )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );
  updateValue< double >( d, names::g_ex, y_[ I_EXC ] );
  updateValue< double >( d, names::dg_ex, y_[ DI_EXC ] );
  updateValue< double >( d, names::g_in, y_[ I_INH ] );
  updateValue< double >( d, names::dg_in, y_[ DI_INH ] );

  if ( y_[ I_EXC ] < 0 || y_[ I_INH ] < 0 )
    throw BadProperty( "Conductances must not be negative." );
}

nest::iaf_psc_alpha_ps::Buffers_::Buffers_( iaf_psc_alpha_ps& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_psc_alpha_ps::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha_ps& n )
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

nest::iaf_psc_alpha_ps::iaf_psc_alpha_ps()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_alpha_ps::iaf_psc_alpha_ps( const iaf_psc_alpha_ps& n )
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
nest::iaf_psc_alpha_ps::init_state_( const Node& proto )
{
  const iaf_psc_alpha_ps& pr = downcast< iaf_psc_alpha_ps >( proto );
  S_ = pr.S_;
}

void
nest::iaf_psc_alpha_ps::init_buffers_()
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
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_step_reset( B_.s_ );

  if ( B_.c_ == 0 )
    B_.c_ = gsl_odeiv_control_yp_new( P_.gsl_error_tol, P_.gsl_error_tol );
  else
    gsl_odeiv_control_init( B_.c_, P_.gsl_error_tol, P_.gsl_error_tol, 0.0, 1.0 );

  if ( B_.e_ == 0 )
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_evolve_reset( B_.e_ );

  B_.sys_.function = iaf_psc_alpha_ps_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::iaf_psc_alpha_ps::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  V_.I0_ex_ = 1.0 * numerics::e / P_.tau_syn_exc;
  V_.I0_in_ = 1.0 * numerics::e / P_.tau_syn_inh;
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps() + 1;
  V_.RefractoryOffset_ = P_.t_ref_ - ( V_.RefractoryCounts_ - 1 ) * Time::get_resolution().get_ms();
  assert( V_.RefractoryCounts_ >= 0 ); // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryOffset_ >= 0. );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_alpha_ps::interpolate_( double& t, double t_old )
{
  // find the exact time when the threshold was crossed
  double dt_crossing = ( P_.V_th - S_.y_old_[ State_::V_M ] ) * ( t - t_old )
    / ( S_.y_[ State_::V_M ] - S_.y_old_[ State_::V_M ] );

  // reset V_m and set the other variables correctly
  S_.y_[ State_::V_M ] = P_.V_reset_;
  for ( int i = 1; i < State_::STATE_VEC_SIZE; ++i )
  {
    S_.y_[ i ] = S_.y_old_[ i ] + ( S_.y_[ i ] - S_.y_old_[ i ] ) / ( t - t_old ) * dt_crossing;
  }

  t = t_old + dt_crossing;
}

void
nest::iaf_psc_alpha_ps::spiking_( const long_t T, const long_t lag, const double t )
{
  // spike event
  const double_t spike_offset = B_.step_ - t;
  SpikeEvent se;
  se.set_offset( spike_offset );
  kernel().event_delivery_manager.send( *this, se, lag );

  // refractoriness
  if ( P_.t_ref_ > 0. )
  {
    S_.r_ = V_.RefractoryCounts_;
    S_.r_offset_ = V_.RefractoryOffset_ - ( B_.step_ - t );
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
nest::iaf_psc_alpha_ps::update( const Time& origin, const long_t from, const long_t to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_builder_manager.get_min_delay() );
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
  if ( S_.y_[ State_::V_M ] >= P_.V_th )
  {
    S_.y_[ State_::V_M ] = P_.V_reset_;
    SpikeEvent se;
    se.set_offset( B_.step_ * ( 1 - std::numeric_limits< double_t >::epsilon() ) );
    kernel().event_delivery_manager.send( *this, se, from );
  }

  for ( long_t lag = from; lag < to; ++lag )
  {
    gsl_odeiv_step_reset( B_.s_ );
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
      // store the previous values of V_m, g_exc, g_inh, and t
      std::copy( S_.y_, S_.y_ + sizeof( S_.y_ ) / sizeof( S_.y_[ 0 ] ), S_.y_old_ );
      t_old = t;
      B_.events_.get_next_event( T, t_next_event, spike_in, spike_ex, B_.step_ );

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

        if ( status != GSL_SUCCESS )
          throw GSLSolverFailure( get_name(), status );

        // check for unreasonable values; we allow V_M to explode
        if ( S_.y_[ State_::V_M ] < -1e3 )
          throw NumericalInstability( get_name() );
      }

      // check refractoriness
      if ( S_.r_ > 0 || S_.r_offset_ > 0. )
        S_.y_[ State_::V_M ] = P_.V_reset_; // only V_m is frozen
      else if ( S_.y_[ State_::V_M ] >= P_.V_th )
      {
        // find the exact time when the threshold was crossed
        interpolate_( t, t_old );
        spiking_( T, lag, t );
      }

      if ( S_.r_ == 0 && std::abs( t - S_.r_offset_ ) < std::numeric_limits< double >::epsilon() )
        S_.r_offset_ = 0.;

      if ( t == t_next_event )
      {
        S_.y_[ State_::DI_EXC ] += spike_ex * V_.I0_ex_;
        S_.y_[ State_::DI_INH ] += spike_in * V_.I0_in_;
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
nest::iaf_psc_alpha_ps::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  const long_t Tdeliver = e.get_stamp().get_steps() + e.get_delay() - 1;
  B_.events_.add_spike( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    Tdeliver,
    e.get_offset(),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_psc_alpha_ps::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::iaf_psc_alpha_ps::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL_1_11
