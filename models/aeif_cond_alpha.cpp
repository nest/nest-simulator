/*
 *  aeif_cond_alpha.cpp
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

#include "aeif_cond_alpha.h"

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

nest::RecordablesMap< nest::aeif_cond_alpha > nest::aeif_cond_alpha::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< aeif_cond_alpha >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &aeif_cond_alpha::get_y_elem_< aeif_cond_alpha::State_::V_M > );
  insert_( names::g_ex, &aeif_cond_alpha::get_y_elem_< aeif_cond_alpha::State_::G_EXC > );
  insert_( names::g_in, &aeif_cond_alpha::get_y_elem_< aeif_cond_alpha::State_::G_INH > );
  insert_( names::w, &aeif_cond_alpha::get_y_elem_< aeif_cond_alpha::State_::W > );
}
}

extern "C" int
nest::aeif_cond_alpha_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::aeif_cond_alpha::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::aeif_cond_alpha& node = *( reinterpret_cast< nest::aeif_cond_alpha* >( pnode ) );

  const bool is_refractory = node.S_.r_ > 0;

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // Clamp membrane potential to V_reset while refractory, otherwise bound
  // it to V_peak. Do not use V_.V_peak_ here, since that is set to V_th if
  // Delta_T == 0.
  const double& V = is_refractory ? node.P_.V_reset_ : std::min( y[ S::V_M ], node.P_.V_peak_ );
  // shorthand for the other state variables
  const double& dg_ex = y[ S::DG_EXC ];
  const double& g_ex = y[ S::G_EXC ];
  const double& dg_in = y[ S::DG_INH ];
  const double& g_in = y[ S::G_INH ];
  const double& w = y[ S::W ];

  const double I_syn_exc = g_ex * ( V - node.P_.E_ex );
  const double I_syn_inh = g_in * ( V - node.P_.E_in );

  const double I_spike =
    node.P_.Delta_T == 0. ? 0. : ( node.P_.g_L * node.P_.Delta_T * std::exp( ( V - node.P_.V_th ) / node.P_.Delta_T ) );

  // dv/dt
  f[ S::V_M ] = is_refractory ? 0. : ( -node.P_.g_L * ( V - node.P_.E_L ) + I_spike - I_syn_exc - I_syn_inh - w
                                       + node.P_.I_e + node.B_.I_stim_ ) / node.P_.C_m;

  f[ S::DG_EXC ] = -dg_ex / node.P_.tau_syn_ex;
  // Synaptic Conductance (nS)
  f[ S::G_EXC ] = dg_ex - g_ex / node.P_.tau_syn_ex;

  f[ S::DG_INH ] = -dg_in / node.P_.tau_syn_in;
  // Synaptic Conductance (nS)
  f[ S::G_INH ] = dg_in - g_in / node.P_.tau_syn_in;

  // Adaptation current w.
  f[ S::W ] = ( node.P_.a * ( V - node.P_.E_L ) - w ) / node.P_.tau_w;

  return GSL_SUCCESS;
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::aeif_cond_alpha::Parameters_::Parameters_()
  : V_peak_( 0.0 )    // mV
  , V_reset_( -60.0 ) // mV
  , t_ref_( 0.0 )     // ms
  , g_L( 30.0 )       // nS
  , C_m( 281.0 )      // pF
  , E_ex( 0.0 )       // mV
  , E_in( -85.0 )     // mV
  , E_L( -70.6 )      // mV
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

nest::aeif_cond_alpha::State_::State_( const Parameters_& p )
  : r_( 0 )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0;
  }
}

nest::aeif_cond_alpha::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::aeif_cond_alpha::State_& nest::aeif_cond_alpha::State_::operator=( const State_& s )
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
nest::aeif_cond_alpha::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
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
nest::aeif_cond_alpha::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::V_th, V_th, node );
  updateValueParam< double >( d, names::V_peak, V_peak_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::E_L, E_L, node );
  updateValueParam< double >( d, names::V_reset, V_reset_, node );
  updateValueParam< double >( d, names::E_ex, E_ex, node );
  updateValueParam< double >( d, names::E_in, E_in, node );

  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::g_L, g_L, node );

  updateValueParam< double >( d, names::tau_syn_ex, tau_syn_ex, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_syn_in, node );

  updateValueParam< double >( d, names::a, a, node );
  updateValueParam< double >( d, names::b, b, node );
  updateValueParam< double >( d, names::Delta_T, Delta_T, node );
  updateValueParam< double >( d, names::tau_w, tau_w, node );

  updateValueParam< double >( d, names::I_e, I_e, node );

  updateValueParam< double >( d, names::gsl_error_tol, gsl_error_tol, node );

  if ( V_reset_ >= V_peak_ )
  {
    throw BadProperty( "Ensure that: V_reset < V_peak ." );
  }

  if ( Delta_T < 0. )
  {
    throw BadProperty( "Delta_T must be positive." );
  }
  else if ( Delta_T > 0. )
  {
    // check for possible numerical overflow with the exponential divergence at
    // spike time, keep a 1e20 margin for the subsequent calculations
    const double max_exp_arg = std::log( std::numeric_limits< double >::max() / 1e20 );
    if ( ( V_peak_ - V_th ) / Delta_T >= max_exp_arg )
    {
      throw BadProperty(
        "The current combination of V_peak, V_th and Delta_T"
        "will lead to numerical overflow at spike time; try"
        "for instance to increase Delta_T or to reduce V_peak"
        "to avoid this problem." );
    }
  }

  if ( V_peak_ < V_th )
  {
    throw BadProperty( "V_peak >= V_th required." );
  }

  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  if ( tau_syn_ex <= 0 || tau_syn_in <= 0 || tau_w <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  if ( gsl_error_tol <= 0. )
  {
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
  }
}

void
nest::aeif_cond_alpha::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::g_ex, y_[ G_EXC ] );
  def< double >( d, names::dg_ex, y_[ DG_EXC ] );
  def< double >( d, names::g_in, y_[ G_INH ] );
  def< double >( d, names::dg_in, y_[ DG_INH ] );
  def< double >( d, names::w, y_[ W ] );
}

void
nest::aeif_cond_alpha::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, y_[ V_M ], node );
  updateValueParam< double >( d, names::g_ex, y_[ G_EXC ], node );
  updateValueParam< double >( d, names::dg_ex, y_[ DG_EXC ], node );
  updateValueParam< double >( d, names::g_in, y_[ G_INH ], node );
  updateValueParam< double >( d, names::dg_in, y_[ DG_INH ], node );
  updateValueParam< double >( d, names::w, y_[ W ], node );
  if ( y_[ G_EXC ] < 0 || y_[ G_INH ] < 0 )
  {
    throw BadProperty( "Conductances must not be negative." );
  }
}

nest::aeif_cond_alpha::Buffers_::Buffers_( aeif_cond_alpha& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::aeif_cond_alpha::Buffers_::Buffers_( const Buffers_&, aeif_cond_alpha& n )
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

nest::aeif_cond_alpha::aeif_cond_alpha()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::aeif_cond_alpha::aeif_cond_alpha( const aeif_cond_alpha& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::aeif_cond_alpha::~aeif_cond_alpha()
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
nest::aeif_cond_alpha::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  ArchivingNode::clear_history();

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
  B_.sys_.function = aeif_cond_alpha_dynamics;

  B_.I_stim_ = 0.0;
}

void
nest::aeif_cond_alpha::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  // set the right threshold and GSL function depending on Delta_T
  if ( P_.Delta_T > 0. )
  {
    V_.V_peak = P_.V_peak_;
  }
  else
  {
    V_.V_peak = P_.V_th; // same as IAF dynamics for spikes if Delta_T == 0.
  }

  V_.g0_ex_ = 1.0 * numerics::e / P_.tau_syn_ex;
  V_.g0_in_ = 1.0 * numerics::e / P_.tau_syn_in;
  V_.refractory_counts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha::update( Time const& origin, const long from, const long to )
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

      // check for unreasonable values; we allow V_M to explode
      if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6 || S_.y_[ State_::W ] > 1e6 )
      {
        throw NumericalInstability( get_name() );
      }

      // spikes are handled inside the while-loop
      // due to spike-driven adaptation
      if ( S_.r_ > 0 )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
      }
      else if ( S_.y_[ State_::V_M ] >= V_.V_peak )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
        S_.y_[ State_::W ] += P_.b; // spike-driven adaptation

        /* Initialize refractory step counter.
         * - We need to add 1 to compensate for count-down immediately after
         *   while loop.
         * - If neuron has no refractory time, set to 0 to avoid refractory
         *   artifact inside while loop.
         */
        S_.r_ = V_.refractory_counts_ > 0 ? V_.refractory_counts_ + 1 : 0;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }

    // decrement refractory count
    if ( S_.r_ > 0 )
    {
      --S_.r_;
    }

    // apply spikes
    S_.y_[ State_::DG_EXC ] += B_.spike_exc_.get_value( lag ) * V_.g0_ex_;
    S_.y_[ State_::DG_INH ] += B_.spike_inh_.get_value( lag ) * V_.g0_in_;

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::aeif_cond_alpha::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::aeif_cond_alpha::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::aeif_cond_alpha::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
