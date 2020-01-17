/*
 *  iaf_cond_alpha.cpp
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


#include "iaf_cond_alpha.h"

#ifdef HAVE_GSL

// C++ includes:
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
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_cond_alpha > nest::iaf_cond_alpha::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_cond_alpha >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_cond_alpha::get_y_elem_< iaf_cond_alpha::State_::V_M > );
  insert_( names::g_ex, &iaf_cond_alpha::get_y_elem_< iaf_cond_alpha::State_::G_EXC > );
  insert_( names::g_in, &iaf_cond_alpha::get_y_elem_< iaf_cond_alpha::State_::G_INH > );

  insert_( names::t_ref_remaining, &iaf_cond_alpha::get_r_ );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" inline int
nest::iaf_cond_alpha_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::iaf_cond_alpha::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::iaf_cond_alpha& node = *( reinterpret_cast< nest::iaf_cond_alpha* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...
  const double I_syn_exc = y[ S::G_EXC ] * ( y[ S::V_M ] - node.P_.E_ex );
  const double I_syn_inh = y[ S::G_INH ] * ( y[ S::V_M ] - node.P_.E_in );
  const double I_leak = node.P_.g_L * ( y[ S::V_M ] - node.P_.E_L );

  // dV_m/dt
  f[ 0 ] = ( -I_leak - I_syn_exc - I_syn_inh + node.B_.I_stim_ + node.P_.I_e ) / node.P_.C_m;

  // d dg_exc/dt, dg_exc/dt
  f[ 1 ] = -y[ S::DG_EXC ] / node.P_.tau_synE;
  f[ 2 ] = y[ S::DG_EXC ] - ( y[ S::G_EXC ] / node.P_.tau_synE );

  // d dg_exc/dt, dg_exc/dt
  f[ 3 ] = -y[ S::DG_INH ] / node.P_.tau_synI;
  f[ 4 ] = y[ S::DG_INH ] - ( y[ S::G_INH ] / node.P_.tau_synI );

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha::Parameters_::Parameters_()
  : V_th( -55.0 )    // mV
  , V_reset( -60.0 ) // mV
  , t_ref( 2.0 )     // ms
  , g_L( 16.6667 )   // nS
  , C_m( 250.0 )     // pF
  , E_ex( 0.0 )      // mV
  , E_in( -85.0 )    // mV
  , E_L( -70.0 )     // mV
  , tau_synE( 0.2 )  // ms
  , tau_synI( 2.0 )  // ms
  , I_e( 0.0 )       // pA
{
}

nest::iaf_cond_alpha::State_::State_( const Parameters_& p )
  : r( 0 )
{
  y[ V_M ] = p.E_L; // initialize to reversal potential
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = 0;
  }
}

nest::iaf_cond_alpha::State_::State_( const State_& s )
  : r( s.r )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = s.y[ i ];
  }
}

nest::iaf_cond_alpha::State_& nest::iaf_cond_alpha::State_::operator=( const State_& s )
{
  if ( this == &s ) // avoid assignment to self
  {
    return *this;
  }
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = s.y[ i ];
  }

  r = s.r;
  return *this;
}

nest::iaf_cond_alpha::Buffers_::Buffers_( iaf_cond_alpha& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_cond_alpha::Buffers_::Buffers_( const Buffers_&, iaf_cond_alpha& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::tau_syn_ex, tau_synE );
  def< double >( d, names::tau_syn_in, tau_synI );
  def< double >( d, names::I_e, I_e );
}

void
nest::iaf_cond_alpha::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // allow setting the membrane potential
  updateValueParam< double >( d, names::V_th, V_th, node );
  updateValueParam< double >( d, names::V_reset, V_reset, node );
  updateValueParam< double >( d, names::t_ref, t_ref, node );
  updateValueParam< double >( d, names::E_L, E_L, node );

  updateValueParam< double >( d, names::E_ex, E_ex, node );
  updateValueParam< double >( d, names::E_in, E_in, node );

  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::g_L, g_L, node );

  updateValueParam< double >( d, names::tau_syn_ex, tau_synE, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_synI, node );

  updateValueParam< double >( d, names::I_e, I_e, node );
  if ( V_reset >= V_th )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }
  if ( tau_synE <= 0 || tau_synI <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
nest::iaf_cond_alpha::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y[ V_M ] ); // Membrane potential
}

void
nest::iaf_cond_alpha::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, y[ V_M ], node );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha::iaf_cond_alpha()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_cond_alpha::iaf_cond_alpha( const iaf_cond_alpha& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::iaf_cond_alpha::~iaf_cond_alpha()
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
nest::iaf_cond_alpha::init_state_( const Node& proto )
{
  const iaf_cond_alpha& pr = downcast< iaf_cond_alpha >( proto );
  S_ = pr.S_;
}

void
nest::iaf_cond_alpha::init_buffers_()
{
  Archiving_Node::clear_history();

  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

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

  B_.sys_.function = iaf_cond_alpha_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::iaf_cond_alpha::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.PSConInit_E = 1.0 * numerics::e / P_.tau_synE;
  V_.PSConInit_I = 1.0 * numerics::e / P_.tau_synI;
  V_.RefractoryCounts = Time( Time::ms( P_.t_ref ) ).get_steps();

  // since t_ref >= 0, this can only fail in error
  assert( V_.RefractoryCounts >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha::update( Time const& origin, const long from, const long to )
{

  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

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
        S_.y );               // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    // refractoriness and spike generation
    if ( S_.r )
    { // neuron is absolute refractory
      --S_.r;
      S_.y[ State_::V_M ] = P_.V_reset; // clamp potential
    }
    else
      // neuron is not absolute refractory
      if ( S_.y[ State_::V_M ] >= P_.V_th )
    {
      S_.r = V_.RefractoryCounts;
      S_.y[ State_::V_M ] = P_.V_reset;

      // log spike with Archiving_Node
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // add incoming spikes
    S_.y[ State_::DG_EXC ] += B_.spike_exc_.get_value( lag ) * V_.PSConInit_E;
    S_.y[ State_::DG_INH ] += B_.spike_inh_.get_value( lag ) * V_.PSConInit_I;

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_cond_alpha::handle( SpikeEvent& e )
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
nest::iaf_cond_alpha::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::iaf_cond_alpha::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
