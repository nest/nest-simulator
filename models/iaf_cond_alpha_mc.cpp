/*
 *  iaf_cond_alpha_mc.cpp
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


#include "iaf_cond_alpha_mc.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

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

/* ----------------------------------------------------------------
 * Compartment name list
 * ---------------------------------------------------------------- */

/* Harold Gutch reported some static destruction problems on OSX 10.4.
   He pointed out that the problem is avoided by defining the comp_names_
   vector with its final size. See also #348.
*/
std::vector< Name > nest::iaf_cond_alpha_mc::comp_names_( NCOMP );

/* ----------------------------------------------------------------
 * Receptor dictionary
 * ---------------------------------------------------------------- */

// leads to seg fault on exit, see #328
// DictionaryDatum nest::iaf_cond_alpha_mc::receptor_dict_ = new Dictionary();

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_cond_alpha_mc >
  nest::iaf_cond_alpha_mc::recordablesMap_;

namespace nest
{
// specialization must be place in namespace

template <>
void
RecordablesMap< iaf_cond_alpha_mc >::create()
{
  insert_( Name( "V_m.s" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::V_M,
      iaf_cond_alpha_mc::SOMA > );
  insert_( Name( "g_ex.s" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_EXC,
      iaf_cond_alpha_mc::SOMA > );
  insert_( Name( "g_in.s" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_INH,
      iaf_cond_alpha_mc::SOMA > );

  insert_( Name( "V_m.p" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::V_M,
      iaf_cond_alpha_mc::PROX > );
  insert_( Name( "g_ex.p" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_EXC,
      iaf_cond_alpha_mc::PROX > );
  insert_( Name( "g_in.p" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_INH,
      iaf_cond_alpha_mc::PROX > );

  insert_( Name( "V_m.d" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::V_M,
      iaf_cond_alpha_mc::DIST > );
  insert_( Name( "g_ex.d" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_EXC,
      iaf_cond_alpha_mc::DIST > );
  insert_( Name( "g_in.d" ),
    &iaf_cond_alpha_mc::get_y_elem_< iaf_cond_alpha_mc::State_::G_INH,
      iaf_cond_alpha_mc::DIST > );

  insert_( names::t_ref_remaining, &iaf_cond_alpha_mc::get_r_ );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" int
nest::iaf_cond_alpha_mc_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // some shorthands
  typedef nest::iaf_cond_alpha_mc N;
  typedef nest::iaf_cond_alpha_mc::State_ S;

  // get access to node so we can work almost as in a member function
  assert( pnode );
  const nest::iaf_cond_alpha_mc& node =
    *( reinterpret_cast< nest::iaf_cond_alpha_mc* >( pnode ) );

  // compute dynamics for each compartment
  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...
  for ( size_t n = 0; n < N::NCOMP; ++n )
  {
    // membrane potential for current compartment
    const double V = y[ S::idx( n, S::V_M ) ];

    // excitatory synaptic current
    const double I_syn_exc =
      y[ S::idx( n, S::G_EXC ) ] * ( V - node.P_.E_ex[ n ] );

    // inhibitory synaptic current
    const double I_syn_inh =
      y[ S::idx( n, S::G_INH ) ] * ( V - node.P_.E_in[ n ] );

    // leak current
    const double I_L = node.P_.g_L[ n ] * ( V - node.P_.E_L[ n ] );

    // coupling currents
    const double I_conn =
      ( n > N::SOMA
          ? node.P_.g_conn[ n - 1 ] * ( V - y[ S::idx( n - 1, S::V_M ) ] )
          : 0 )
      + ( n < N::NCOMP - 1
            ? node.P_.g_conn[ n ] * ( V - y[ S::idx( n + 1, S::V_M ) ] )
            : 0 );

    // derivatives
    // membrane potential
    f[ S::idx( n, S::V_M ) ] =
      ( -I_L - I_syn_exc - I_syn_inh - I_conn + node.B_.I_stim_[ n ]
        + node.P_.I_e[ n ] ) / node.P_.C_m[ n ];

    // excitatory conductance
    f[ S::idx( n, S::DG_EXC ) ] =
      -y[ S::idx( n, S::DG_EXC ) ] / node.P_.tau_synE[ n ];
    f[ S::idx( n, S::G_EXC ) ] = y[ S::idx( n, S::DG_EXC ) ]
      - y[ S::idx( n, S::G_EXC ) ] / node.P_.tau_synE[ n ];

    // inhibitory conductance
    f[ S::idx( n, S::DG_INH ) ] =
      -y[ S::idx( n, S::DG_INH ) ] / node.P_.tau_synI[ n ];
    f[ S::idx( n, S::G_INH ) ] = y[ S::idx( n, S::DG_INH ) ]
      - y[ S::idx( n, S::G_INH ) ] / node.P_.tau_synI[ n ];
  }

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc::Parameters_::Parameters_()
  : V_th( -55.0 )    // mV
  , V_reset( -60.0 ) // mV
  , t_ref( 2.0 )     // ms
{
  // conductances between compartments
  g_conn[ SOMA ] = 2.5; // nS, soma-proximal
  g_conn[ PROX ] = 1.0; // nS, proximal-distal

  // soma parameters
  g_L[ SOMA ] = 10.0;     // nS
  C_m[ SOMA ] = 150.0;    // pF
  E_ex[ SOMA ] = 0.0;     // mV
  E_in[ SOMA ] = -85.0;   // mV
  E_L[ SOMA ] = -70.0;    // mV
  tau_synE[ SOMA ] = 0.5; // ms
  tau_synI[ SOMA ] = 2.0; // ms
  I_e[ SOMA ] = 0.0;      // pA

  // proximal parameters
  g_L[ PROX ] = 5.0;      // nS
  C_m[ PROX ] = 75.0;     // pF
  E_ex[ PROX ] = 0.0;     // mV
  E_in[ PROX ] = -85.0;   // mV
  E_L[ PROX ] = -70.0;    // mV
  tau_synE[ PROX ] = 0.5; // ms
  tau_synI[ PROX ] = 2.0; // ms
  I_e[ PROX ] = 0.0;      // pA

  // distal parameters
  g_L[ DIST ] = 10.0;     // nS
  C_m[ DIST ] = 150.0;    // pF
  E_ex[ DIST ] = 0.0;     // mV
  E_in[ DIST ] = -85.0;   // mV
  E_L[ DIST ] = -70.0;    // mV
  tau_synE[ DIST ] = 0.5; // ms
  tau_synI[ DIST ] = 2.0; // ms
  I_e[ DIST ] = 0.0;      // pA
}

nest::iaf_cond_alpha_mc::Parameters_::Parameters_( const Parameters_& p )
  : V_th( p.V_th )
  , V_reset( p.V_reset )
  , t_ref( p.t_ref )
{
  // copy C-arrays
  for ( size_t n = 0; n < NCOMP - 1; ++n )
  {
    g_conn[ n ] = p.g_conn[ n ];
  }

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    g_L[ n ] = p.g_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
  }
}

nest::iaf_cond_alpha_mc::Parameters_& nest::iaf_cond_alpha_mc::Parameters_::
operator=( const Parameters_& p )
{
  assert( this != &p ); // would be bad logical error in program

  V_th = p.V_th;
  V_reset = p.V_reset;
  t_ref = p.t_ref;

  // copy C-arrays
  for ( size_t n = 0; n < NCOMP - 1; ++n )
  {
    g_conn[ n ] = p.g_conn[ n ];
  }

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    g_L[ n ] = p.g_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
  }

  return *this;
}


nest::iaf_cond_alpha_mc::State_::State_( const Parameters_& p )
  : r_( 0 )
{
  // for simplicity, we first initialize all values to 0,
  // then set the membrane potentials for each compartment
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0;
  }
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    y_[ idx( n, V_M ) ] = p.E_L[ n ];
  }
}

nest::iaf_cond_alpha_mc::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::iaf_cond_alpha_mc::State_& nest::iaf_cond_alpha_mc::State_::operator=(
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

nest::iaf_cond_alpha_mc::Buffers_::Buffers_( iaf_cond_alpha_mc& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_cond_alpha_mc::Buffers_::Buffers_( const Buffers_&,
  iaf_cond_alpha_mc& n )
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
nest::iaf_cond_alpha_mc::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );

  def< double >( d, names::g_sp, g_conn[ SOMA ] );
  def< double >( d, names::g_pd, g_conn[ PROX ] );

  // create subdictionaries for per-compartment parameters
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    DictionaryDatum dd = new Dictionary();

    def< double >( dd, names::g_L, g_L[ n ] );
    def< double >( dd, names::E_L, E_L[ n ] );
    def< double >( dd, names::E_ex, E_ex[ n ] );
    def< double >( dd, names::E_in, E_in[ n ] );
    def< double >( dd, names::C_m, C_m[ n ] );
    def< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
    def< double >( dd, names::tau_syn_in, tau_synI[ n ] );
    def< double >( dd, names::I_e, I_e[ n ] );

    ( *d )[ comp_names_[ n ] ] = dd;
  }
}

void
nest::iaf_cond_alpha_mc::Parameters_::set( const DictionaryDatum& d )
{
  // allow setting the membrane potential
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_reset, V_reset );
  updateValue< double >( d, names::t_ref, t_ref );

  updateValue< double >( d, Name( names::g_sp ), g_conn[ SOMA ] );
  updateValue< double >( d, Name( names::g_pd ), g_conn[ PROX ] );

  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

      updateValue< double >( dd, names::E_L, E_L[ n ] );
      updateValue< double >( dd, names::E_ex, E_ex[ n ] );
      updateValue< double >( dd, names::E_in, E_in[ n ] );
      updateValue< double >( dd, names::C_m, C_m[ n ] );
      updateValue< double >( dd, names::g_L, g_L[ n ] );
      updateValue< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
      updateValue< double >( dd, names::tau_syn_in, tau_synI[ n ] );
      updateValue< double >( dd, names::I_e, I_e[ n ] );
    }
  }
  if ( V_reset >= V_th )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( t_ref < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  // apply checks compartment-wise
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( C_m[ n ] <= 0 )
    {
      throw BadProperty( "Capacitance (" + comp_names_[ n ].toString()
        + ") must be strictly positive." );
    }
    if ( tau_synE[ n ] <= 0 || tau_synI[ n ] <= 0 )
    {
      throw BadProperty( "All time constants (" + comp_names_[ n ].toString()
        + ") must be strictly positive." );
    }
  }
}

void
nest::iaf_cond_alpha_mc::State_::get( DictionaryDatum& d ) const
{
  // we assume here that State_::get() always is called after
  // Parameters_::get(), so that the per-compartment dictionaries exist
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    assert( d->known( comp_names_[ n ] ) );
    DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

    def< double >( dd, names::V_m, y_[ idx( n, V_M ) ] ); // Membrane potential
  }
}

void
nest::iaf_cond_alpha_mc::State_::set( const DictionaryDatum& d,
  const Parameters_& )
{
  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );
      updateValue< double >( dd, names::V_m, y_[ idx( n, V_M ) ] );
    }
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc::iaf_cond_alpha_mc()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();

  // set up table of compartment names
  // comp_names_.resize(NCOMP); --- Fixed size, see comment on definition
  comp_names_[ SOMA ] = Name( "soma" );
  comp_names_[ PROX ] = Name( "proximal" );
  comp_names_[ DIST ] = Name( "distal" );
}

nest::iaf_cond_alpha_mc::iaf_cond_alpha_mc( const iaf_cond_alpha_mc& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::iaf_cond_alpha_mc::~iaf_cond_alpha_mc()
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
nest::iaf_cond_alpha_mc::init_state_( const Node& proto )
{
  const iaf_cond_alpha_mc& pr = downcast< iaf_cond_alpha_mc >( proto );
  S_ = pr.S_;
}

void
nest::iaf_cond_alpha_mc::init_buffers_()
{
  B_.spikes_.resize( NUM_SPIKE_RECEPTORS );
  for ( size_t n = 0; n < NUM_SPIKE_RECEPTORS; ++n )
  {
    B_.spikes_[ n ].clear();
  } // includes resize

  B_.currents_.resize( NUM_CURR_RECEPTORS );
  for ( size_t n = 0; n < NUM_CURR_RECEPTORS; ++n )
  {
    B_.currents_[ n ].clear();
  } // includes resize

  B_.logger_.reset();
  Archiving_Node::clear_history();

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

  B_.sys_.function = iaf_cond_alpha_mc_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    B_.I_stim_[ n ] = 0.0;
  }
}

void
nest::iaf_cond_alpha_mc::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    V_.PSConInit_E_[ n ] = 1.0 * numerics::e / P_.tau_synE[ n ];
    V_.PSConInit_I_[ n ] = 1.0 * numerics::e / P_.tau_synI[ n ];
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  // since t_ref >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc::update( Time const& origin,
  const long from,
  const long to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
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
        S_.y_ );              // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    // add incoming spikes at end of interval
    // exploit here that spike buffers are compartment for compartment,
    // alternating between excitatory and inhibitory
    for ( size_t n = 0; n < NCOMP; ++n )
    {
      S_.y_[ n * State_::STATE_VEC_COMPS + State_::DG_EXC ] +=
        B_.spikes_[ 2 * n ].get_value( lag ) * V_.PSConInit_E_[ n ];
      S_.y_[ n * State_::STATE_VEC_COMPS + State_::DG_INH ] +=
        B_.spikes_[ 2 * n + 1 ].get_value( lag ) * V_.PSConInit_I_[ n ];
    }

    // refractoriness and spiking
    // exploit here that plain offset enum value V_M indexes soma V_M
    if ( S_.r_ )
    { // neuron is absolute refractory
      --S_.r_;
      S_.y_[ State_::V_M ] = P_.V_reset;
    }
    else if ( S_.y_[ State_::V_M ] >= P_.V_th )
    { // neuron fires spike
      S_.r_ = V_.RefractoryCounts_;
      S_.y_[ State_::V_M ] = P_.V_reset;

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input currents
    for ( size_t n = 0; n < NCOMP; ++n )
    {
      B_.I_stim_[ n ] = B_.currents_[ n ].get_value( lag );
    }

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_cond_alpha_mc::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < 2 * NCOMP );

  B_.spikes_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_cond_alpha_mc::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  // not 100% clean, should look at MIN, SUP
  assert( 0 <= e.get_rport() && e.get_rport() < NCOMP );

  // add weighted current; HEP 2002-10-04
  B_.currents_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
nest::iaf_cond_alpha_mc::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
