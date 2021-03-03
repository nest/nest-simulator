/*
 *  aeif_psc_delta_clopath.cpp
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

#include "aeif_psc_delta_clopath.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
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

nest::RecordablesMap< nest::aeif_psc_delta_clopath > nest::aeif_psc_delta_clopath::recordablesMap_;

namespace nest
{
/*
 * template specialization must be placed in namespace
 *
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< aeif_psc_delta_clopath >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::V_M > );
  insert_( names::w, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::W > );
  insert_( names::z, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::Z > );
  insert_( names::V_th, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::V_TH > );
  insert_( names::u_bar_plus, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::U_BAR_PLUS > );
  insert_( names::u_bar_minus, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::U_BAR_MINUS > );
  insert_( names::u_bar_bar, &aeif_psc_delta_clopath::get_y_elem_< aeif_psc_delta_clopath::State_::U_BAR_BAR > );
}
}


extern "C" int
nest::aeif_psc_delta_clopath_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::aeif_psc_delta_clopath::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::aeif_psc_delta_clopath& node = *( reinterpret_cast< nest::aeif_psc_delta_clopath* >( pnode ) );

  const bool is_refractory = node.S_.r_ > 0;
  const bool is_clamped = node.S_.clamp_r_ > 0;

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away...

  // Clamp membrane potential to V_reset while refractory, otherwise bound
  // it to V_peak.
  const double& V = ( is_refractory || is_clamped ) ? ( is_clamped ? node.P_.V_clamp_ : node.P_.V_reset_ )
                                                    : std::min( y[ S::V_M ], node.P_.V_peak_ );
  // shorthand for the other state variables
  const double& w = y[ S::W ];
  const double& z = y[ S::Z ];
  const double& V_th = y[ S::V_TH ];
  const double& u_bar_plus = y[ S::U_BAR_PLUS ];
  const double& u_bar_minus = y[ S::U_BAR_MINUS ];
  const double& u_bar_bar = y[ S::U_BAR_BAR ];

  const double I_spike =
    node.P_.Delta_T == 0. ? 0. : ( node.P_.g_L * node.P_.Delta_T * std::exp( ( V - V_th ) / node.P_.Delta_T ) );

  // dv/dt
  f[ S::V_M ] = ( is_refractory || is_clamped ) ? 0.0 : ( -node.P_.g_L * ( V - node.P_.E_L ) + I_spike - w + z
                                                          + node.P_.I_e + node.B_.I_stim_ ) / node.P_.C_m;

  // Adaptation current w.
  f[ S::W ] = is_clamped ? 0.0 : ( node.P_.a * ( V - node.P_.E_L ) - w ) / node.P_.tau_w;

  f[ S::Z ] = -z / node.P_.tau_z;

  f[ S::V_TH ] = -( V_th - node.P_.V_th_rest ) / node.P_.tau_V_th;

  f[ S::U_BAR_PLUS ] = ( -u_bar_plus + V ) / node.P_.tau_plus;

  f[ S::U_BAR_MINUS ] = ( -u_bar_minus + V ) / node.P_.tau_minus;

  f[ S::U_BAR_BAR ] = ( -u_bar_bar + u_bar_minus ) / node.P_.tau_bar_bar;

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::aeif_psc_delta_clopath::Parameters_::Parameters_()
  : V_peak_( 33.0 )      // mV
  , V_reset_( -60.0 )    // mV
  , t_ref_( 0.0 )        // ms
  , g_L( 30.0 )          // nS
  , C_m( 281.0 )         // pF
  , E_L( -70.6 )         // mV
  , Delta_T( 2.0 )       // mV
  , tau_w( 144.0 )       // ms
  , tau_z( 40.0 )        // ms
  , tau_V_th( 50.0 )     // ms
  , V_th_max( 30.4 )     // mV
  , V_th_rest( -50.4 )   // mV
  , tau_plus( 7.0 )      // ms
  , tau_minus( 10.0 )    // ms
  , tau_bar_bar( 500.0 ) // ms
  , a( 4.0 )             // nS
  , b( 80.5 )            // pA
  , I_sp( 400.0 )        // pA
  , I_e( 0.0 )           // pA
  , gsl_error_tol( 1e-6 )
  , t_clamp_( 2.0 )  // ms
  , V_clamp_( 33.0 ) // mV
{
}

nest::aeif_psc_delta_clopath::State_::State_( const Parameters_& p )
  : r_( 0 )
  , clamp_r_( 0 )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0;
  }
  y_[ V_M ] = p.E_L;
  y_[ V_TH ] = p.V_th_rest;
  y_[ U_BAR_PLUS ] = p.E_L;
  y_[ U_BAR_MINUS ] = p.E_L;
  y_[ U_BAR_BAR ] = p.E_L;
}

nest::aeif_psc_delta_clopath::State_::State_( const State_& s )
  : r_( s.r_ )
  , clamp_r_( s.clamp_r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::aeif_psc_delta_clopath::State_& nest::aeif_psc_delta_clopath::State_::operator=( const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  r_ = s.r_;
  clamp_r_ = s.clamp_r_;
  return *this;
}

/* ----------------------------------------------------------------
 * Paramater and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::aeif_psc_delta_clopath::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th_max, V_th_max );
  def< double >( d, names::V_th_rest, V_th_rest );
  def< double >( d, names::tau_V_th, tau_V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::a, a );
  def< double >( d, names::b, b );
  def< double >( d, names::I_sp, I_sp );
  def< double >( d, names::Delta_T, Delta_T );
  def< double >( d, names::tau_w, tau_w );
  def< double >( d, names::tau_z, tau_z );
  def< double >( d, names::tau_plus, tau_plus );
  def< double >( d, names::tau_minus, tau_minus );
  def< double >( d, names::tau_bar_bar, tau_bar_bar );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::V_peak, V_peak_ );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );
  def< double >( d, names::V_clamp, V_clamp_ );
  def< double >( d, names::t_clamp, t_clamp_ );
}

void
nest::aeif_psc_delta_clopath::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::V_th_max, V_th_max, node );
  updateValueParam< double >( d, names::V_th_rest, V_th_rest, node );
  updateValueParam< double >( d, names::tau_V_th, tau_V_th, node );
  updateValueParam< double >( d, names::V_peak, V_peak_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::E_L, E_L, node );
  updateValueParam< double >( d, names::V_reset, V_reset_, node );

  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::g_L, g_L, node );

  updateValueParam< double >( d, names::a, a, node );
  updateValueParam< double >( d, names::b, b, node );
  updateValueParam< double >( d, names::I_sp, I_sp, node );
  updateValueParam< double >( d, names::Delta_T, Delta_T, node );
  updateValueParam< double >( d, names::tau_w, tau_w, node );
  updateValueParam< double >( d, names::tau_z, tau_z, node );
  updateValueParam< double >( d, names::tau_plus, tau_plus, node );
  updateValueParam< double >( d, names::tau_minus, tau_minus, node );
  updateValueParam< double >( d, names::tau_bar_bar, tau_bar_bar, node );

  updateValueParam< double >( d, names::I_e, I_e, node );

  updateValueParam< double >( d, names::gsl_error_tol, gsl_error_tol, node );

  updateValueParam< double >( d, names::V_clamp, V_clamp_, node );
  updateValueParam< double >( d, names::t_clamp, t_clamp_, node );

  if ( V_reset_ >= V_peak_ )
  {
    throw BadProperty( "Ensure that V_reset < V_peak ." );
  }

  if ( Delta_T < 0. )
  {
    throw BadProperty( "Delta_T must be greater than or equal to zero." );
  }
  else if ( Delta_T > 0. )
  {
    // check for possible numerical overflow with the exponential divergence at
    // spike time, keep a 1e20 margin for the subsequent calculations
    const double max_delta_arg = std::log( std::numeric_limits< double >::max() / 1e20 );
    if ( ( V_peak_ - V_th_rest ) / Delta_T >= max_delta_arg )
    {
      throw BadProperty(
        "The current combination of V_peak, V_th_rest and Delta_T "
        "will lead to numerical overflow at spike time; try"
        "for instance to increase Delta_T or to reduce V_peak"
        "to avoid this problem." );
    }
  }

  if ( V_th_max < V_th_rest )
  {
    throw BadProperty( "V_th_max >= V_th_rest required." );
  }

  if ( V_peak_ < V_th_rest )
  {
    throw BadProperty( "V_peak >= V_th_rest required." );
  }

  if ( C_m <= 0 )
  {
    throw BadProperty( "Ensure that C_m > 0" );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Ensure that t_ref >= 0" );
  }

  if ( t_clamp_ < 0 )
  {
    throw BadProperty( "Ensure that t_clamp >= 0" );
  }

  if ( tau_w <= 0 or tau_V_th <= 0 or tau_w <= 0 or tau_z <= 0 or tau_plus <= 0 or tau_minus <= 0 or tau_bar_bar <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  if ( gsl_error_tol <= 0. )
  {
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
  }
}

void
nest::aeif_psc_delta_clopath::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::w, y_[ W ] );
  def< double >( d, names::u_bar_plus, y_[ U_BAR_PLUS ] );
  def< double >( d, names::u_bar_minus, y_[ U_BAR_MINUS ] );
  def< double >( d, names::u_bar_bar, y_[ U_BAR_BAR ] );
}

void
nest::aeif_psc_delta_clopath::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, y_[ V_M ], node );
  updateValueParam< double >( d, names::w, y_[ W ], node );
  updateValueParam< double >( d, names::u_bar_plus, y_[ U_BAR_PLUS ], node );
  updateValueParam< double >( d, names::u_bar_minus, y_[ U_BAR_MINUS ], node );
  updateValueParam< double >( d, names::u_bar_bar, y_[ U_BAR_BAR ], node );
}

nest::aeif_psc_delta_clopath::Buffers_::Buffers_( aeif_psc_delta_clopath& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::aeif_psc_delta_clopath::Buffers_::Buffers_( const Buffers_&, aeif_psc_delta_clopath& n )
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

nest::aeif_psc_delta_clopath::aeif_psc_delta_clopath()
  : ClopathArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::aeif_psc_delta_clopath::aeif_psc_delta_clopath( const aeif_psc_delta_clopath& n )
  : ClopathArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::aeif_psc_delta_clopath::~aeif_psc_delta_clopath()
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
nest::aeif_psc_delta_clopath::init_state_( const Node& proto )
{
  const aeif_psc_delta_clopath& pr = downcast< aeif_psc_delta_clopath >( proto );
  S_ = pr.S_;
}

void
nest::aeif_psc_delta_clopath::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  ClopathArchivingNode::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();

  // We must integrate this model with high-precision to obtain decent results
  B_.IntegrationStep_ = std::min( 0.01, B_.step_ );

  if ( B_.s_ == 0 )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }
  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_yp_new( P_.gsl_error_tol, P_.gsl_error_tol );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, P_.gsl_error_tol, P_.gsl_error_tol, 0.0, 1.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
  B_.sys_.function = aeif_psc_delta_clopath_dynamics;

  B_.I_stim_ = 0.0;

  init_clopath_buffers();
}

void
nest::aeif_psc_delta_clopath::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.V_peak_ = P_.V_peak_;

  V_.refractory_counts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  // implementation of the clamping after a spike
  V_.clamp_counts_ = Time( Time::ms( P_.t_clamp_ ) ).get_steps();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::aeif_psc_delta_clopath::update( const Time& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );
  assert( State_::V_M == 0 );

  for ( long lag = from; lag < to; ++lag )
  {
    double t = 0.0;

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
      // check for unreasonable values; we allow V_M to explode
      if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6 || S_.y_[ State_::W ] > 1e6 )
      {
        throw NumericalInstability( get_name() );
      }

      // spikes are handled inside the while-loop
      // due to spike-driven adaptation
      if ( S_.r_ == 0 && S_.clamp_r_ == 0 )
      {
        // neuron not refractory
        S_.y_[ State_::V_M ] = S_.y_[ State_::V_M ] + B_.spikes_.get_value( lag );
      }
      else // neuron is absolute refractory
      {
        B_.spikes_.get_value( lag ); // clear buffer entry, ignore spike
      }

      // set the right threshold depending on Delta_T
      if ( P_.Delta_T == 0. )
      {
        V_.V_peak_ = S_.y_[ State_::V_TH ]; // same as IAF dynamics for spikes if
                                            // Delta_T == 0.
      }

      if ( S_.y_[ State_::V_M ] >= V_.V_peak_ && S_.clamp_r_ == 0 )
      {
        S_.y_[ State_::V_M ] = P_.V_clamp_;
        S_.y_[ State_::W ] += P_.b; // spike-driven adaptation
        S_.y_[ State_::Z ] = P_.I_sp;
        S_.y_[ State_::V_TH ] = P_.V_th_max;

        /* Initialize clamping step counter.
        * - We need to add 1 to compensate for count-down immediately after
        *   while loop.
        * - If neuron does not use clamping, set to 0
        */
        S_.clamp_r_ = V_.clamp_counts_ > 0 ? V_.clamp_counts_ + 1 : 0;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
      else if ( S_.clamp_r_ == 1 )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
        S_.clamp_r_ = 0;

        /* Initialize refractory step counter.
        * - We need to add 1 to compensate for count-down immediately after
        *   while loop.
        * - If neuron has no refractory time, set to 0 to avoid refractory
        *   artifact inside while loop.
        */
        S_.r_ = V_.refractory_counts_ > 0 ? V_.refractory_counts_ + 1 : 0;
      }

      if ( S_.r_ > 0 )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
      }
    }

    // save data for Clopath synapses
    write_clopath_history( Time::step( origin.get_steps() + lag + 1 ),
      S_.y_[ State_::V_M ],
      S_.y_[ State_::U_BAR_PLUS ],
      S_.y_[ State_::U_BAR_MINUS ],
      S_.y_[ State_::U_BAR_BAR ] );

    // decrement clamp count
    if ( S_.clamp_r_ > 0 )
    {
      --S_.clamp_r_;
    }
    // decrement refractory count
    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::aeif_psc_delta_clopath::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::aeif_psc_delta_clopath::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::aeif_psc_delta_clopath::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
