/*
 *  hh_cond_beta_gap_traub.cpp
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


#include "hh_cond_beta_gap_traub.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iostream>

// External includes:
#include <gsl/gsl_errno.h>

// Includes from libnestutil:
#include "beta_normalization_factor.h"
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"

nest::RecordablesMap< nest::hh_cond_beta_gap_traub > nest::hh_cond_beta_gap_traub::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< hh_cond_beta_gap_traub >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::V_M > );
  insert_( names::g_ex, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::G_EXC > );
  insert_( names::g_in, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::G_INH > );
  insert_( names::Act_m, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::HH_M > );
  insert_( names::Inact_h, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::HH_H > );
  insert_( names::Act_n, &hh_cond_beta_gap_traub::get_y_elem_< hh_cond_beta_gap_traub::State_::HH_N > );
}

extern "C" int
hh_cond_beta_gap_traub_dynamics( double time, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::hh_cond_beta_gap_traub::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::hh_cond_beta_gap_traub& node = *( reinterpret_cast< nest::hh_cond_beta_gap_traub* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // ionic currents
  const double I_Na =
    node.P_.g_Na * y[ S::HH_M ] * y[ S::HH_M ] * y[ S::HH_M ] * y[ S::HH_H ] * ( y[ S::V_M ] - node.P_.E_Na );
  const double I_K =
    node.P_.g_K * y[ S::HH_N ] * y[ S::HH_N ] * y[ S::HH_N ] * y[ S::HH_N ] * ( y[ S::V_M ] - node.P_.E_K );
  const double I_L = node.P_.g_L * ( y[ S::V_M ] - node.P_.E_L );

  // chemical synaptic currents
  const double I_syn_exc = y[ S::G_EXC ] * ( y[ S::V_M ] - node.P_.E_ex );
  const double I_syn_inh = y[ S::G_INH ] * ( y[ S::V_M ] - node.P_.E_in );

  // gap junction currents
  // set I_gap depending on interpolation order
  double gap = 0.0;

  const double t = time / node.B_.step_;

  switch ( kernel().simulation_manager.get_wfr_interpolation_order() )
  {
  case 0:
    gap = -node.B_.sumj_g_ij_ * y[ S::V_M ] + node.B_.interpolation_coefficients[ node.B_.lag_ ];
    break;

  case 1:
    gap = -node.B_.sumj_g_ij_ * y[ S::V_M ] + node.B_.interpolation_coefficients[ node.B_.lag_ * 2 + 0 ]
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 2 + 1 ] * t;
    break;

  case 3:
    gap = -node.B_.sumj_g_ij_ * y[ S::V_M ] + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 0 ]
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 1 ] * t
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 2 ] * t * t
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 3 ] * t * t * t;
    break;

  default:
    throw BadProperty( "Interpolation order must be 0, 1, or 3." );
  }

  const double I_gap = gap;

  // membrane potential
  f[ S::V_M ] = ( -I_Na - I_K - I_L - I_syn_exc - I_syn_inh + node.B_.I_stim_ + I_gap + node.P_.I_e ) / node.P_.C_m;

  // channel dynamics
  const double V = y[ S::V_M ] - node.P_.V_T;

  const double alpha_n = 0.032 * ( 15. - V ) / ( std::exp( ( 15. - V ) / 5. ) - 1. );
  const double beta_n = 0.5 * std::exp( ( 10. - V ) / 40. );
  const double alpha_m = 0.32 * ( 13. - V ) / ( std::exp( ( 13. - V ) / 4. ) - 1. );
  const double beta_m = 0.28 * ( V - 40. ) / ( std::exp( ( V - 40. ) / 5. ) - 1. );
  const double alpha_h = 0.128 * std::exp( ( 17. - V ) / 18. );
  const double beta_h = 4. / ( 1. + std::exp( ( 40. - V ) / 5. ) );

  f[ S::HH_M ] = alpha_m - ( alpha_m + beta_m ) * y[ S::HH_M ]; // m-variable
  f[ S::HH_H ] = alpha_h - ( alpha_h + beta_h ) * y[ S::HH_H ]; // h-variable
  f[ S::HH_N ] = alpha_n - ( alpha_n + beta_n ) * y[ S::HH_N ]; // n-variable

  // synapses: beta function
  // d^2g_exc/dt^2, dg_exc/dt
  f[ S::DG_EXC ] = -y[ S::DG_EXC ] / node.P_.tau_decay_ex;
  f[ S::G_EXC ] = y[ S::DG_EXC ] - ( y[ S::G_EXC ] / node.P_.tau_rise_ex );

  // d^2g_inh/dt^2, dg_inh/dt
  f[ S::DG_INH ] = -y[ S::DG_INH ] / node.P_.tau_decay_in;
  f[ S::G_INH ] = y[ S::DG_INH ] - ( y[ S::G_INH ] / node.P_.tau_rise_in );

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::hh_cond_beta_gap_traub::Parameters_::Parameters_()
  : g_Na( 20000.0 )      // Sodium Conductance                      (nS)
  , g_K( 6000.0 )        // Potassium Conductance                   (nS)
  , g_L( 10.0 )          // Leak Conductance                        (nS)
  , C_m( 200.0 )         // Membrane Capacitance                    (pF)
  , E_Na( 50.0 )         // Sodium Reversal potential               (mV)
  , E_K( -90.0 )         // Potassium Reversal potential            (mV)
  , E_L( -60.0 )         // Leak Reversal potential                 (mV)
  , V_T( -50.0 )         // adjusts firing threshold                (mV)
  , E_ex( 0.0 )          // Excitatory reversal potential           (mV)
  , E_in( -80.0 )        // Inhibitory reversal potential           (mV)
  , tau_rise_ex( 0.5 )   // Excitatory Synaptic Rise Time Constant  (ms)
  , tau_decay_ex( 5.0 )  // Excitatory Synaptic Decay Time Constant (ms)
  , tau_rise_in( 0.5 )   // Inhibitory Synaptic Rise Time Constant  (ms)
  , tau_decay_in( 10.0 ) // Inhibitory Synaptic Decay Time Constant (ms)
  , t_ref_( 2.0 )        // Refractory time in ms                   (ms)
  , I_e( 0.0 )           // Stimulus Current                        (pA)
{
}

nest::hh_cond_beta_gap_traub::State_::State_( const Parameters_& p )
  : r_( 0 )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0.0;
  }

  // equilibrium values for (in)activation variables
  const double alpha_n = 0.032 * ( 15. - y_[ 0 ] ) / ( std::exp( ( 15. - y_[ 0 ] ) / 5. ) - 1. );
  const double beta_n = 0.5 * std::exp( ( 10. - y_[ 0 ] ) / 40. );
  const double alpha_m = 0.32 * ( 13. - y_[ 0 ] ) / ( std::exp( ( 13. - y_[ 0 ] ) / 4. ) - 1. );
  const double beta_m = 0.28 * ( y_[ 0 ] - 40. ) / ( std::exp( ( y_[ 0 ] - 40. ) / 5. ) - 1. );
  const double alpha_h = 0.128 * std::exp( ( 17. - y_[ 0 ] ) / 18. );
  const double beta_h = 4. / ( 1. + std::exp( ( 40. - y_[ 0 ] ) / 5. ) );

  y_[ HH_H ] = alpha_h / ( alpha_h + beta_h );
  y_[ HH_N ] = alpha_n / ( alpha_n + beta_n );
  y_[ HH_M ] = alpha_m / ( alpha_m + beta_m );
}

nest::hh_cond_beta_gap_traub::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::hh_cond_beta_gap_traub::State_&
nest::hh_cond_beta_gap_traub::State_::operator=( const State_& s )
{
  r_ = s.r_;
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::hh_cond_beta_gap_traub::Parameters_::get( DictionaryDatum& d ) const
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
  def< double >( d, names::tau_rise_ex, tau_rise_ex );
  def< double >( d, names::tau_decay_ex, tau_decay_ex );
  def< double >( d, names::tau_rise_in, tau_rise_in );
  def< double >( d, names::tau_decay_in, tau_decay_in );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::I_e, I_e );
}

void
nest::hh_cond_beta_gap_traub::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::g_Na, g_Na, node );
  updateValueParam< double >( d, names::g_K, g_K, node );
  updateValueParam< double >( d, names::g_L, g_L, node );
  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::E_Na, E_Na, node );
  updateValueParam< double >( d, names::E_K, E_K, node );
  updateValueParam< double >( d, names::E_L, E_L, node );
  updateValueParam< double >( d, names::V_T, V_T, node );
  updateValueParam< double >( d, names::E_ex, E_ex, node );
  updateValueParam< double >( d, names::E_in, E_in, node );
  updateValueParam< double >( d, names::tau_rise_ex, tau_rise_ex, node );
  updateValueParam< double >( d, names::tau_decay_ex, tau_decay_ex, node );
  updateValueParam< double >( d, names::tau_rise_in, tau_rise_in, node );
  updateValueParam< double >( d, names::tau_decay_in, tau_decay_in, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::I_e, I_e, node );

  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  if ( tau_rise_ex <= 0 or tau_decay_ex <= 0 or tau_rise_in <= 0 or tau_decay_in <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  if ( g_K < 0 or g_Na < 0 or g_L < 0 )
  {
    throw BadProperty( "All conductances must be non-negative." );
  }
}

void
nest::hh_cond_beta_gap_traub::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] ); // Membrane potential
  def< double >( d, names::Act_m, y_[ HH_M ] );
  def< double >( d, names::Inact_h, y_[ HH_H ] );
  def< double >( d, names::Act_n, y_[ HH_N ] );
}

void
nest::hh_cond_beta_gap_traub::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, y_[ V_M ], node );
  updateValueParam< double >( d, names::Act_m, y_[ HH_M ], node );
  updateValueParam< double >( d, names::Inact_h, y_[ HH_H ], node );
  updateValueParam< double >( d, names::Act_n, y_[ HH_N ], node );
  if ( y_[ HH_M ] < 0 or y_[ HH_H ] < 0 or y_[ HH_N ] < 0 )
  {
    throw BadProperty( "All (in)activation variables must be non-negative." );
  }
}

nest::hh_cond_beta_gap_traub::Buffers_::Buffers_( hh_cond_beta_gap_traub& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::hh_cond_beta_gap_traub::Buffers_::Buffers_( const Buffers_&, hh_cond_beta_gap_traub& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::hh_cond_beta_gap_traub::hh_cond_beta_gap_traub()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::hh_cond_beta_gap_traub::hh_cond_beta_gap_traub( const hh_cond_beta_gap_traub& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::hh_cond_beta_gap_traub::~hh_cond_beta_gap_traub()
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
nest::hh_cond_beta_gap_traub::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  // allocate strucure for gap events here
  // function is called from Scheduler::prepare_nodes() before the
  // first call to update
  // so we already know which interpolation scheme to use according
  // to the properties of this neurons
  // determine size of structure depending on interpolation scheme
  // and unsigned int Scheduler::min_delay() (number of simulation time steps
  // per min_delay step)

  // resize interpolation_coefficients depending on interpolation order
  const size_t buffer_size =
    kernel().connection_manager.get_min_delay() * ( kernel().simulation_manager.get_wfr_interpolation_order() + 1 );

  B_.interpolation_coefficients.resize( buffer_size, 0.0 );

  B_.last_y_values.resize( kernel().connection_manager.get_min_delay(), 0.0 );

  B_.sumj_g_ij_ = 0.0;

  ArchivingNode::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( not B_.s_ )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( not B_.c_ )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  if ( not B_.e_ )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = hh_cond_beta_gap_traub_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

double
nest::hh_cond_beta_gap_traub::get_normalisation_factor( double tau_rise, double tau_decay )
{
  return nest::beta_normalization_factor( tau_rise, tau_decay );
}

void
nest::hh_cond_beta_gap_traub::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.PSConInit_E = nest::hh_cond_beta_gap_traub::get_normalisation_factor( P_.tau_rise_ex, P_.tau_decay_ex );
  V_.PSConInit_I = nest::hh_cond_beta_gap_traub::get_normalisation_factor( P_.tau_rise_in, P_.tau_decay_in );

  V_.refractory_counts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  V_.U_old_ = S_.y_[ State_::V_M ];

  // since t_ref_ >= 0, this can only fail in error
  assert( V_.refractory_counts_ >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

bool
nest::hh_cond_beta_gap_traub::update_( Time const& origin,
  const long from,
  const long to,
  const bool called_from_wfr_update )
{

  assert( to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const size_t interpolation_order = kernel().simulation_manager.get_wfr_interpolation_order();
  const double wfr_tol = kernel().simulation_manager.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store the new interpolation coefficients
  // to be sent by gap event
  const size_t buffer_size = kernel().connection_manager.get_min_delay() * ( interpolation_order + 1 );
  std::vector< double > new_coefficients( buffer_size, 0.0 );

  // parameters needed for piecewise interpolation
  double y_i = 0.0, y_ip1 = 0.0, hf_i = 0.0, hf_ip1 = 0.0;
  double f_temp[ State_::STATE_VEC_SIZE ];

  for ( long lag = from; lag < to; ++lag )
  {

    // B_.lag is needed by hh_cond_beta_gap_traub_dynamics to
    // determine the current section
    B_.lag_ = lag;

    if ( called_from_wfr_update )
    {
      y_i = S_.y_[ State_::V_M ];
      if ( interpolation_order == 3 )
      {
        hh_cond_beta_gap_traub_dynamics( 0, S_.y_, f_temp, reinterpret_cast< void* >( this ) );
        hf_i = B_.step_ * f_temp[ State_::V_M ];
      }
    }

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

    if ( not called_from_wfr_update )
    {
      S_.y_[ State_::DG_EXC ] += B_.spike_exc_.get_value( lag ) * V_.PSConInit_E;
      S_.y_[ State_::DG_INH ] += B_.spike_inh_.get_value( lag ) * V_.PSConInit_I;
      // sending spikes: crossing 0 mV, pseudo-refractoriness and local
      // maximum...
      // refractory?
      if ( S_.r_ > 0 )
      {
        --S_.r_;
      }
      else if ( S_.y_[ State_::V_M ] >= P_.V_T + 30. and U_old > S_.y_[ State_::V_M ] ) // ( threshold and maximum )
      {
        S_.r_ = V_.refractory_counts_;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }

      // log state data
      B_.logger_.record_data( origin.get_steps() + lag );

      // set new input current
      B_.I_stim_ = B_.currents_.get_value( lag );
    }
    else // if(called_from_wfr_update)
    {
      S_.y_[ State_::DG_EXC ] += B_.spike_exc_.get_value_wfr_update( lag ) * V_.PSConInit_E;
      S_.y_[ State_::DG_INH ] += B_.spike_inh_.get_value_wfr_update( lag ) * V_.PSConInit_I;
      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded or fabs( S_.y_[ State_::V_M ] - B_.last_y_values[ lag ] ) > wfr_tol;
      B_.last_y_values[ lag ] = S_.y_[ State_::V_M ];

      // update different interpolations

      // constant term is the same for each interpolation order
      new_coefficients[ lag * ( interpolation_order + 1 ) + 0 ] = y_i;

      switch ( interpolation_order )
      {
      case 0:
        break;

      case 1:
        y_ip1 = S_.y_[ State_::V_M ];

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = y_ip1 - y_i;
        break;

      case 3:
        y_ip1 = S_.y_[ State_::V_M ];
        hh_cond_beta_gap_traub_dynamics( B_.step_, S_.y_, f_temp, reinterpret_cast< void* >( this ) );
        hf_ip1 = B_.step_ * f_temp[ State_::V_M ];

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = hf_i;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 2 ] = -3 * y_i + 3 * y_ip1 - 2 * hf_i - hf_ip1;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 3 ] = 2 * y_i - 2 * y_ip1 + hf_i + hf_ip1;
        break;

      default:
        throw BadProperty( "Interpolation order must be 0, 1, or 3." );
      }
    }


  } // end for-loop

  // if not called_from_wfr_update perform constant extrapolation
  // and reset last_y_values
  if ( not called_from_wfr_update )
  {
    for ( long temp = from; temp < to; ++temp )
    {
      new_coefficients[ temp * ( interpolation_order + 1 ) + 0 ] = S_.y_[ State_::V_M ];
    }

    std::vector< double >( kernel().connection_manager.get_min_delay(), 0.0 ).swap( B_.last_y_values );
  }

  // Send gap-event
  GapJunctionEvent ge;
  ge.set_coeffarray( new_coefficients );
  kernel().event_delivery_manager.send_secondary( *this, ge );

  // Reset variables
  B_.sumj_g_ij_ = 0.0;
  std::vector< double >( buffer_size, 0.0 ).swap( B_.interpolation_coefficients );

  return wfr_tol_exceeded;
}

void
nest::hh_cond_beta_gap_traub::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    // add with negative weight, ie positive value, since we are changing a
    // conductance
    B_.spike_inh_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::hh_cond_beta_gap_traub::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::hh_cond_beta_gap_traub::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::hh_cond_beta_gap_traub::handle( GapJunctionEvent& e )
{
  const double weight = e.get_weight();

  B_.sumj_g_ij_ += weight;

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    B_.interpolation_coefficients[ i ] += weight * e.get_coeffvalue( it );
    ++i;
  }
}

} // namespace nest

#endif // HAVE_GSL
