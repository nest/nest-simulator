/*
 *  hh_cond_exp_traub.cpp
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


#include "hh_cond_exp_traub.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_sf_exp.h>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

nest::RecordablesMap< nest::hh_cond_exp_traub >
  nest::hh_cond_exp_traub::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< hh_cond_exp_traub >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::V_M > );
  insert_( names::g_ex,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::G_EXC > );
  insert_( names::g_in,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::G_INH > );
  insert_( names::Act_m,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::HH_M > );
  insert_( names::Act_h,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::HH_H > );
  insert_( names::Inact_n,
    &hh_cond_exp_traub::get_y_elem_< hh_cond_exp_traub::State_::HH_N > );
}

extern "C" int
hh_cond_exp_traub_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::hh_cond_exp_traub::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::hh_cond_exp_traub& node =
    *( reinterpret_cast< nest::hh_cond_exp_traub* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // ionic currents
  const double I_Na = node.P_.g_Na * y[ S::HH_M ] * y[ S::HH_M ] * y[ S::HH_M ]
    * y[ S::HH_H ] * ( y[ S::V_M ] - node.P_.E_Na );
  const double I_K = node.P_.g_K * y[ S::HH_N ] * y[ S::HH_N ] * y[ S::HH_N ]
    * y[ S::HH_N ] * ( y[ S::V_M ] - node.P_.E_K );
  const double I_L = node.P_.g_L * ( y[ S::V_M ] - node.P_.E_L );

  const double I_syn_exc = y[ S::G_EXC ] * ( y[ S::V_M ] - node.P_.E_ex );
  const double I_syn_inh = y[ S::G_INH ] * ( y[ S::V_M ] - node.P_.E_in );

  // membrane potential
  f[ S::V_M ] = ( -I_Na - I_K - I_L - I_syn_exc - I_syn_inh + node.B_.I_stim_
                  + node.P_.I_e ) / node.P_.C_m;

  // channel dynamics
  const double V = y[ S::V_M ] - node.P_.V_T;

  const double alpha_n =
    0.032 * ( 15. - V ) / ( std::exp( ( 15. - V ) / 5. ) - 1. );
  const double beta_n = 0.5 * std::exp( ( 10. - V ) / 40. );
  const double alpha_m =
    0.32 * ( 13. - V ) / ( std::exp( ( 13. - V ) / 4. ) - 1. );
  const double beta_m =
    0.28 * ( V - 40. ) / ( std::exp( ( V - 40. ) / 5. ) - 1. );
  const double alpha_h = 0.128 * std::exp( ( 17. - V ) / 18. );
  const double beta_h = 4. / ( 1. + std::exp( ( 40. - V ) / 5. ) );

  f[ S::HH_M ] = alpha_m - ( alpha_m + beta_m ) * y[ S::HH_M ]; // m-variable
  f[ S::HH_H ] = alpha_h - ( alpha_h + beta_h ) * y[ S::HH_H ]; // h-variable
  f[ S::HH_N ] = alpha_n - ( alpha_n + beta_n ) * y[ S::HH_N ]; // n-variable

  // synapses: exponential conductance
  f[ S::G_EXC ] = -y[ S::G_EXC ] / node.P_.tau_synE;
  f[ S::G_INH ] = -y[ S::G_INH ] / node.P_.tau_synI;

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::hh_cond_exp_traub::Parameters_::Parameters_()
  : g_Na( 20000.0 ) // Sodium Conductance (nS)
  , g_K( 6000.0 )   // K Conductance      (nS)
  , g_L( 10.0 )     // Leak Conductance   (nS)
  , C_m( 200.0 )    // Membrane Capacitance (pF)
  , E_Na( 50.0 )    // Reversal potentials (mV)
  , E_K( -90.0 )
  , E_L( -60.0 )
  , V_T( -63.0 ) // adjusts threshold to around -50 mV
  , E_ex( 0.0 )
  , E_in( -80.0 )
  , tau_synE( 5.0 )  // Synaptic Time Constant Excitatory Synapse (ms)
  , tau_synI( 10.0 ) // Synaptic Time Constant Excitatory Synapse (ms)
  , t_ref_( 2.0 )    // Refractory time in ms
  , I_e( 0.0 )       // Stimulus Current (pA)
{
}

nest::hh_cond_exp_traub::State_::State_( const Parameters_& p )
  : r_( 0 )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0.0;
  }

  // equilibrium values for (in)activation variables
  const double alpha_n =
    0.032 * ( 15. - y_[ 0 ] ) / ( std::exp( ( 15. - y_[ 0 ] ) / 5. ) - 1. );
  const double beta_n = 0.5 * std::exp( ( 10. - y_[ 0 ] ) / 40. );
  const double alpha_m =
    0.32 * ( 13. - y_[ 0 ] ) / ( std::exp( ( 13. - y_[ 0 ] ) / 4. ) - 1. );
  const double beta_m =
    0.28 * ( y_[ 0 ] - 40. ) / ( std::exp( ( y_[ 0 ] - 40. ) / 5. ) - 1. );
  const double alpha_h = 0.128 * std::exp( ( 17. - y_[ 0 ] ) / 18. );
  const double beta_h = 4. / ( 1. + std::exp( ( 40. - y_[ 0 ] ) / 5. ) );

  y_[ HH_H ] = alpha_h / ( alpha_h + beta_h );
  y_[ HH_N ] = alpha_n / ( alpha_n + beta_n );
  y_[ HH_M ] = alpha_m / ( alpha_m + beta_m );
}

nest::hh_cond_exp_traub::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::hh_cond_exp_traub::State_& nest::hh_cond_exp_traub::State_::operator=(
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
nest::hh_cond_exp_traub::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::g_Na, g_Na );
  def< double >( d, names::g_K, g_K );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::E_Na, E_Na );
  def< double >( d, names::E_K, E_K );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_T, V_T );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::tau_syn_ex, tau_synE );
  def< double >( d, names::tau_syn_in, tau_synI );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::I_e, I_e );
}

void
nest::hh_cond_exp_traub::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::g_Na, g_Na );
  updateValue< double >( d, names::g_K, g_K );
  updateValue< double >( d, names::g_L, g_L );
  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::E_Na, E_Na );
  updateValue< double >( d, names::E_K, E_K );
  updateValue< double >( d, names::E_L, E_L );
  updateValue< double >( d, names::V_T, V_T );
  updateValue< double >( d, names::E_ex, E_ex );
  updateValue< double >( d, names::E_in, E_in );
  updateValue< double >( d, names::tau_syn_ex, tau_synE );
  updateValue< double >( d, names::tau_syn_in, tau_synI );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::I_e, I_e );

  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( tau_synE <= 0 || tau_synI <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }
}

void
nest::hh_cond_exp_traub::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] ); // Membrane potential
  def< double >( d, names::Act_m, y_[ HH_M ] );
  def< double >( d, names::Act_h, y_[ HH_H ] );
  def< double >( d, names::Inact_n, y_[ HH_N ] );
}

void
nest::hh_cond_exp_traub::State_::set( const DictionaryDatum& d,
  const Parameters_& )
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

nest::hh_cond_exp_traub::Buffers_::Buffers_( hh_cond_exp_traub& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::hh_cond_exp_traub::Buffers_::Buffers_( const Buffers_&,
  hh_cond_exp_traub& n )
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

nest::hh_cond_exp_traub::hh_cond_exp_traub()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::hh_cond_exp_traub::hh_cond_exp_traub( const hh_cond_exp_traub& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::hh_cond_exp_traub::~hh_cond_exp_traub()
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
nest::hh_cond_exp_traub::init_state_( const Node& proto )
{
  const hh_cond_exp_traub& pr = downcast< hh_cond_exp_traub >( proto );
  S_ = pr.S_;
}

void
nest::hh_cond_exp_traub::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  Archiving_Node::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  B_.I_stim_ = 0.0;

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

  B_.sys_.function = hh_cond_exp_traub_dynamics;
  B_.sys_.jacobian = 0;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
}

void
nest::hh_cond_exp_traub::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
  V_.refractory_counts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  V_.U_old_ = S_.y_[ State_::V_M ];
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */
void
nest::hh_cond_exp_traub::update( Time const& origin,
  const long from,
  const long to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    double tt = 0.0; // it's all relative!
    V_.U_old_ = S_.y_[ State_::V_M ];


    // adaptive step integration
    while ( tt < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &tt,                  // from t...
        B_.step_,             // ...to t=t+h
        &B_.IntegrationStep_, // integration window (written on!)
        S_.y_ );              // neuron state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    S_.y_[ State_::G_EXC ] += B_.spike_exc_.get_value( lag );
    S_.y_[ State_::G_INH ] += B_.spike_inh_.get_value( lag );

    // sending spikes: crossing 0 mV, pseudo-refractoriness and local maximum...
    // refractory?
    if ( S_.r_ )
    {
      --S_.r_;
    }
    else
    {
      // (threshold   &&    maximum    )
      if ( S_.y_[ State_::V_M ] >= P_.V_T + 30.
        && V_.U_old_ > S_.y_[ State_::V_M ] )
      {
        S_.r_ = V_.refractory_counts_;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::hh_cond_exp_traub::handle( SpikeEvent& e )
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
    // add with negative weight, ie positive value, since we are changing a
    // conductance
    B_.spike_inh_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::hh_cond_exp_traub::handle( CurrentEvent& e )
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
nest::hh_cond_exp_traub::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace nest

#endif // HAVE_GSL
