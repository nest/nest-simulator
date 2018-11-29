/*
 *  hh_psc_alpha.cpp
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


#include "hh_psc_alpha.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


nest::RecordablesMap< nest::hh_psc_alpha > nest::hh_psc_alpha::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< hh_psc_alpha >::create()
{
  // use standard names whereever you can for consistency!
  insert_(
    names::V_m, &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::V_M > );
  insert_( names::I_syn_ex,
    &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::I_EXC > );
  insert_( names::I_syn_in,
    &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::I_INH > );
  insert_(
    names::Act_m, &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::HH_M > );
  insert_(
    names::Act_h, &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::HH_H > );
  insert_(
    names::Inact_n, &hh_psc_alpha::get_y_elem_< hh_psc_alpha::State_::HH_N > );
}

extern "C" int
hh_psc_alpha_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::hh_psc_alpha::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::hh_psc_alpha& node =
    *( reinterpret_cast< nest::hh_psc_alpha* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double& V = y[ S::V_M ];
  const double& m = y[ S::HH_M ];
  const double& h = y[ S::HH_H ];
  const double& n = y[ S::HH_N ];
  const double& dI_ex = y[ S::DI_EXC ];
  const double& I_ex = y[ S::I_EXC ];
  const double& dI_in = y[ S::DI_INH ];
  const double& I_in = y[ S::I_INH ];

  const double alpha_n =
    ( 0.01 * ( V + 55. ) ) / ( 1. - std::exp( -( V + 55. ) / 10. ) );
  const double beta_n = 0.125 * std::exp( -( V + 65. ) / 80. );
  const double alpha_m =
    ( 0.1 * ( V + 40. ) ) / ( 1. - std::exp( -( V + 40. ) / 10. ) );
  const double beta_m = 4. * std::exp( -( V + 65. ) / 18. );
  const double alpha_h = 0.07 * std::exp( -( V + 65. ) / 20. );
  const double beta_h = 1. / ( 1. + std::exp( -( V + 35. ) / 10. ) );

  const double I_Na = node.P_.g_Na * m * m * m * h * ( V - node.P_.E_Na );
  const double I_K = node.P_.g_K * n * n * n * n * ( V - node.P_.E_K );
  const double I_L = node.P_.g_L * ( V - node.P_.E_L );

  // V dot -- synaptic input are currents, inhib current is negative
  f[ S::V_M ] = ( -( I_Na + I_K + I_L ) + node.B_.I_stim_ + node.P_.I_e + I_ex
                  + I_in ) / node.P_.C_m;

  // channel dynamics
  f[ S::HH_M ] =
    alpha_m * ( 1 - y[ S::HH_M ] ) - beta_m * y[ S::HH_M ]; // m-variable
  f[ S::HH_H ] =
    alpha_h * ( 1 - y[ S::HH_H ] ) - beta_h * y[ S::HH_H ]; // h-variable
  f[ S::HH_N ] =
    alpha_n * ( 1 - y[ S::HH_N ] ) - beta_n * y[ S::HH_N ]; // n-variable

  // synapses: alpha functions
  f[ S::DI_EXC ] = -dI_ex / node.P_.tau_synE;
  f[ S::I_EXC ] = dI_ex - ( I_ex / node.P_.tau_synE );
  f[ S::DI_INH ] = -dI_in / node.P_.tau_synI;
  f[ S::I_INH ] = dI_in - ( I_in / node.P_.tau_synI );

  return GSL_SUCCESS;
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::hh_psc_alpha::Parameters_::Parameters_()
  : t_ref_( 2.0 )   // ms
  , g_Na( 12000.0 ) // nS
  , g_K( 3600.0 )   // nS
  , g_L( 30.0 )     // nS
  , C_m( 100.0 )    // pF
  , E_Na( 50.0 )    // mV
  , E_K( -77.0 )    // mV
  , E_L( -54.402 )  // mV
  , tau_synE( 0.2 ) // ms
  , tau_synI( 2.0 ) // ms
  , I_e( 0.0 )      // pA
{
}

nest::hh_psc_alpha::State_::State_( const Parameters_& )
  : r_( 0 )
{
  y_[ 0 ] = -65; // p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0;
  }

  // equilibrium values for (in)activation variables
  const double alpha_n = ( 0.01 * ( y_[ 0 ] + 55. ) )
    / ( 1. - std::exp( -( y_[ 0 ] + 55. ) / 10. ) );
  const double beta_n = 0.125 * std::exp( -( y_[ 0 ] + 65. ) / 80. );
  const double alpha_m =
    ( 0.1 * ( y_[ 0 ] + 40. ) ) / ( 1. - std::exp( -( y_[ 0 ] + 40. ) / 10. ) );
  const double beta_m = 4. * std::exp( -( y_[ 0 ] + 65. ) / 18. );
  const double alpha_h = 0.07 * std::exp( -( y_[ 0 ] + 65. ) / 20. );
  const double beta_h = 1. / ( 1. + std::exp( -( y_[ 0 ] + 35. ) / 10. ) );

  y_[ HH_H ] = alpha_h / ( alpha_h + beta_h );
  y_[ HH_N ] = alpha_n / ( alpha_n + beta_n );
  y_[ HH_M ] = alpha_m / ( alpha_m + beta_m );
}

nest::hh_psc_alpha::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::hh_psc_alpha::State_& nest::hh_psc_alpha::State_::operator=(
  const State_& s )
{
  assert( this != &s ); // would be bad logical error in program
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  r_ = s.r_;
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::hh_psc_alpha::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_Na, g_Na );
  def< double >( d, names::g_K, g_K );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_Na, E_Na );
  def< double >( d, names::E_K, E_K );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::tau_syn_ex, tau_synE );
  def< double >( d, names::tau_syn_in, tau_synI );
  def< double >( d, names::I_e, I_e );
}

void
nest::hh_psc_alpha::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::g_Na, g_Na );
  updateValue< double >( d, names::E_Na, E_Na );
  updateValue< double >( d, names::g_K, g_K );
  updateValue< double >( d, names::E_K, E_K );
  updateValue< double >( d, names::g_L, g_L );
  updateValue< double >( d, names::E_L, E_L );

  updateValue< double >( d, names::tau_syn_ex, tau_synE );
  updateValue< double >( d, names::tau_syn_in, tau_synI );

  updateValue< double >( d, names::I_e, I_e );
  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }
  if ( tau_synE <= 0 || tau_synI <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  if ( g_K < 0 || g_Na < 0 || g_L < 0 )
  {
    throw BadProperty( "All conductances must be non-negative." );
  }
}

void
nest::hh_psc_alpha::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::Act_m, y_[ HH_M ] );
  def< double >( d, names::Act_h, y_[ HH_H ] );
  def< double >( d, names::Inact_n, y_[ HH_N ] );
}

void
nest::hh_psc_alpha::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );
  updateValue< double >( d, names::Act_m, y_[ HH_M ] );
  updateValue< double >( d, names::Act_h, y_[ HH_H ] );
  updateValue< double >( d, names::Inact_n, y_[ HH_N ] );
  if ( y_[ HH_M ] < 0 || y_[ HH_H ] < 0 || y_[ HH_N ] < 0 )
  {
    throw BadProperty( "All (in)activation variables must be non-negative." );
  }
}

nest::hh_psc_alpha::Buffers_::Buffers_( hh_psc_alpha& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::hh_psc_alpha::Buffers_::Buffers_( const Buffers_&, hh_psc_alpha& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::hh_psc_alpha::hh_psc_alpha()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::hh_psc_alpha::hh_psc_alpha( const hh_psc_alpha& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::hh_psc_alpha::~hh_psc_alpha()
{
  // GSL structs may not have been allocated, so we need to protect destruction
  if ( B_.s_ )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
  if ( B_.c_ )
  {
    gsl_odeiv_control_free( B_.c_ );
  }
  if ( B_.e_ )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::hh_psc_alpha::init_state_( const Node& proto )
{
  const hh_psc_alpha& pr = downcast< hh_psc_alpha >( proto );
  S_ = pr.S_;
}

void
nest::hh_psc_alpha::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  Archiving_Node::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
  {
    B_.s_ =
      gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = hh_psc_alpha_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::hh_psc_alpha::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.PSCurrInit_E_ = 1.0 * numerics::e / P_.tau_synE;
  V_.PSCurrInit_I_ = 1.0 * numerics::e / P_.tau_synI;
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::hh_psc_alpha::update( Time const& origin, const long from, const long to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    double t = 0.0;
    const double U_old = S_.y_[ State_::V_M ];

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // gsl_odeiv_evolve_apply performs only a single numerical
    // integration step, starting from t and bounded by step;
    // the while-loop ensures integration over the whole simulation
    // step (0, step] if more than one integration step is needed due
    // to a small integration step size;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t; this is of advantage
    // for a consistent and efficient integration across subsequent
    // simulation intervals
    while ( t < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &t,                   // from t
        B_.step_,             // to t <= step
        &B_.IntegrationStep_, // integration step size
        S_.y_ );              // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    S_.y_[ State_::DI_EXC ] +=
      B_.spike_exc_.get_value( lag ) * V_.PSCurrInit_E_;
    S_.y_[ State_::DI_INH ] +=
      B_.spike_inh_.get_value( lag ) * V_.PSCurrInit_I_;

    // sending spikes: crossing 0 mV, pseudo-refractoriness and local maximum...
    // refractory?
    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }
    else
      // (    threshold    &&     maximum       )
      if ( S_.y_[ State_::V_M ] >= 0 && U_old > S_.y_[ State_::V_M ] )
    {
      S_.r_ = V_.RefractoryCounts_;

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );
  }
}

void
nest::hh_psc_alpha::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  } // current input, keep negative weight
}

void
nest::hh_psc_alpha::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::hh_psc_alpha::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
