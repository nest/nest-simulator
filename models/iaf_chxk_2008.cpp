/*
 *  iaf_chxk_2008.cpp
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

#include "iaf_chxk_2008.h"

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

nest::RecordablesMap< nest::iaf_chxk_2008 > nest::iaf_chxk_2008::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_chxk_2008 >::create()
{
  insert_( names::V_m, &iaf_chxk_2008::get_y_elem_< iaf_chxk_2008::State_::V_M > );
  insert_( names::g_ex, &iaf_chxk_2008::get_y_elem_< iaf_chxk_2008::State_::G_EXC > );
  insert_( names::g_in, &iaf_chxk_2008::get_y_elem_< iaf_chxk_2008::State_::G_INH > );
  insert_( names::g_ahp, &iaf_chxk_2008::get_y_elem_< iaf_chxk_2008::State_::G_AHP > );

  insert_( names::I_syn_ex, &iaf_chxk_2008::get_I_syn_exc_ );
  insert_( names::I_syn_in, &iaf_chxk_2008::get_I_syn_inh_ );
  insert_( names::I_ahp, &iaf_chxk_2008::get_I_ahp_ );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" inline int
nest::iaf_chxk_2008_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::iaf_chxk_2008::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::iaf_chxk_2008& node = *( reinterpret_cast< nest::iaf_chxk_2008* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...
  const double I_syn_exc = y[ S::G_EXC ] * ( y[ S::V_M ] - node.P_.E_ex );
  const double I_syn_inh = y[ S::G_INH ] * ( y[ S::V_M ] - node.P_.E_in );
  const double I_ahp = y[ S::G_AHP ] * ( y[ S::V_M ] - node.P_.E_ahp );
  const double I_leak = node.P_.g_L * ( y[ S::V_M ] - node.P_.E_L );

  // dV_m/dt
  f[ S::V_M ] = ( -I_leak - I_syn_exc - I_syn_inh - I_ahp + node.B_.I_stim_ + node.P_.I_e ) / node.P_.C_m;

  // d dg_exc/dt, dg_exc/dt
  f[ S::DG_EXC ] = -y[ S::DG_EXC ] / node.P_.tau_synE;
  f[ S::G_EXC ] = y[ S::DG_EXC ] - ( y[ S::G_EXC ] / node.P_.tau_synE );

  // d dg_inh/dt, dg_inh/dt
  f[ S::DG_INH ] = -y[ S::DG_INH ] / node.P_.tau_synI;
  f[ S::G_INH ] = y[ S::DG_INH ] - ( y[ S::G_INH ] / node.P_.tau_synI );

  // d dg_ahp/dt, dg_ahp/dt
  f[ S::DG_AHP ] = -y[ S::DG_AHP ] / node.P_.tau_ahp;
  f[ S::G_AHP ] = y[ S::DG_AHP ] - ( y[ S::G_AHP ] / node.P_.tau_ahp );

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_chxk_2008::Parameters_::Parameters_()
  : // Default values chosen based on values found in
  // Alex Casti's simulator
  V_th( -45.0 )     // mV
  , g_L( 100.0 )    // nS
  , C_m( 1000.0 )   // pF
  , E_ex( 20.0 )    // mV
  , E_in( -90.0 )   // mV
  , E_L( -60.0 )    // mV
  , tau_synE( 1.0 ) // ms
  , tau_synI( 1.0 ) // ms
  , I_e( 0.0 )      // pA
  , tau_ahp( 0.5 )  // ms
  , g_ahp( 443.8 )  // nS
  , E_ahp( -95.0 )  // mV
  , ahp_bug( false )

{
  recordablesMap_.create();
}

nest::iaf_chxk_2008::State_::State_( const Parameters_& p )
  : r( 0 )
{
  y[ V_M ] = p.E_L; // initialize to reversal potential
  for ( size_t i = 2; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = 0;
  }
}

nest::iaf_chxk_2008::State_::State_( const State_& s )
  : r( s.r )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = s.y[ i ];
  }
}

nest::iaf_chxk_2008::State_& nest::iaf_chxk_2008::State_::operator=( const State_& s )
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

nest::iaf_chxk_2008::Buffers_::Buffers_( iaf_chxk_2008& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_chxk_2008::Buffers_::Buffers_( const Buffers_&, iaf_chxk_2008& n )
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
nest::iaf_chxk_2008::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::tau_syn_ex, tau_synE );
  def< double >( d, names::tau_syn_in, tau_synI );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::tau_ahp, tau_ahp );
  def< double >( d, names::E_ahp, E_ahp );
  def< double >( d, names::g_ahp, g_ahp );
  def< bool >( d, names::ahp_bug, ahp_bug );
}

void
nest::iaf_chxk_2008::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // allow setting the membrane potential
  updateValueParam< double >( d, names::V_th, V_th, node );
  updateValueParam< double >( d, names::g_L, g_L, node );
  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::E_ex, E_ex, node );
  updateValueParam< double >( d, names::E_in, E_in, node );
  updateValueParam< double >( d, names::E_L, E_L, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_synE, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_synI, node );
  updateValueParam< double >( d, names::I_e, I_e, node );
  updateValueParam< double >( d, names::tau_ahp, tau_ahp, node );
  updateValueParam< double >( d, names::E_ahp, E_ahp, node );
  updateValueParam< double >( d, names::g_ahp, g_ahp, node );
  updateValueParam< bool >( d, names::ahp_bug, ahp_bug, node );
  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( tau_synE <= 0 or tau_synI <= 0 or tau_ahp <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
nest::iaf_chxk_2008::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y[ V_M ] ); // Membrane potential
}

void
nest::iaf_chxk_2008::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, y[ V_M ], node );
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::iaf_chxk_2008::iaf_chxk_2008()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
}

nest::iaf_chxk_2008::iaf_chxk_2008( const iaf_chxk_2008& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::iaf_chxk_2008::~iaf_chxk_2008()
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
nest::iaf_chxk_2008::init_state_( const Node& proto )
{
  const iaf_chxk_2008& pr = downcast< iaf_chxk_2008 >( proto );
  S_ = pr.S_;
}

void
nest::iaf_chxk_2008::init_buffers_()
{
  ArchivingNode::clear_history();

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

  B_.sys_.function = iaf_chxk_2008_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::iaf_chxk_2008::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.PSConInit_E = 1.0 * numerics::e / P_.tau_synE;
  V_.PSConInit_I = 1.0 * numerics::e / P_.tau_synI;
  V_.PSConInit_AHP = P_.g_ahp * numerics::e / P_.tau_ahp;
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_chxk_2008::update( Time const& origin, const long from, const long to )
{

  assert( to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    double t = 0.0;

    // remember membrane potential at beginning of step
    // to check for *crossing*
    const double vm_prev = S_.y[ State_::V_M ];

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
    // neuron should spike on threshold crossing only.
    if ( vm_prev < P_.V_th and S_.y[ State_::V_M ] >= P_.V_th )
    {
      // Find precise spike time by linear interpolation.
      // dt is time from spike to end of time step.
      const double dt = B_.step_ * ( S_.y[ State_::V_M ] - P_.V_th ) / ( S_.y[ State_::V_M ] - vm_prev );

      // Evolve AHP conductance from time of spike to end of step
      const double delta_dg = V_.PSConInit_AHP * std::exp( -dt / P_.tau_ahp );
      const double delta_g = delta_dg * dt;

      if ( P_.ahp_bug )
      {
        // Bug in original Fortran implementation by Casti et al. ignores
        // AHP conductance from previous spikes: at any time, only the
        // alpha function "kicked off" by the most recent spike is included
        S_.y[ State_::G_AHP ] = delta_g;
        S_.y[ State_::DG_AHP ] = delta_dg;
      }
      else
      {
        // Correct implementation adds initial values for new AHP to AHP history
        S_.y[ State_::G_AHP ] += delta_g;
        S_.y[ State_::DG_AHP ] += delta_dg;
      }

      // log spike with ArchivingNode
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      se.set_offset( dt );
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
nest::iaf_chxk_2008::handle( SpikeEvent& e )
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
nest::iaf_chxk_2008::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::iaf_chxk_2008::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
#endif // HAVE_GSL
