/*
 *  iaf_cond_alpha_mc_fixedca.cpp
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


#include "iaf_cond_alpha_mc_fixedca.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>

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
std::vector< Name > nest::iaf_cond_alpha_mc_fixedca::comp_names_( NCOMP );

/* ----------------------------------------------------------------
 * Receptor dictionary
 * ---------------------------------------------------------------- */

// leads to seg fault on exit, see #328
// DictionaryDatum nest::iaf_cond_alpha_mc_fixedca::receptor_dict_ = new
// Dictionary();

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_cond_alpha_mc_fixedca >
  nest::iaf_cond_alpha_mc_fixedca::recordablesMap_;

namespace nest
{
// specialization must be place in namespace

template <>
void
RecordablesMap< iaf_cond_alpha_mc_fixedca >::create()
{
  insert_( Name( "V_m.s" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
        iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "g_ex.s" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
        iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "g_in.s" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
        iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "i_ap.s" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
        iaf_cond_alpha_mc_fixedca::SOMA > );

  insert_( Name( "V_m.p" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
        iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "g_ex.p" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
        iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "g_in.p" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
        iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "i_ap.p" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
        iaf_cond_alpha_mc_fixedca::PROX > );

  insert_( Name( "V_m.d" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
        iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "g_ex.d" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
        iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "g_in.d" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
        iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "i_ap.d" ),
    &iaf_cond_alpha_mc_fixedca::
      get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
        iaf_cond_alpha_mc_fixedca::DIST > );

  insert_( names::t_ref_remaining, &iaf_cond_alpha_mc_fixedca::get_r_ );
  insert_( names::threshold, &iaf_cond_alpha_mc_fixedca::get_th_ );

  insert_( names::ca_spike_count, &iaf_cond_alpha_mc_fixedca::get_ca_ );
  insert_( names::t_refCa_remaining, &iaf_cond_alpha_mc_fixedca::get_rCa_ );
  insert_( names::ca_current, &iaf_cond_alpha_mc_fixedca::get_ICa_ );
  insert_( Name( "I_stim.s" ), &iaf_cond_alpha_mc_fixedca::get_CurrS_ );
  insert_( Name( "I_stim.p" ), &iaf_cond_alpha_mc_fixedca::get_CurrP_ );
  insert_( Name( "I_stim.d" ), &iaf_cond_alpha_mc_fixedca::get_CurrD_ );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" int
nest::iaf_cond_alpha_mc_fixedca_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // some shorthands
  typedef nest::iaf_cond_alpha_mc_fixedca N;
  typedef nest::iaf_cond_alpha_mc_fixedca::State_ S;

  // get access to node so we can work almost as in a member function
  assert( pnode );
  const nest::iaf_cond_alpha_mc_fixedca& node =
    *( reinterpret_cast< nest::iaf_cond_alpha_mc_fixedca* >( pnode ) );

  // compute dynamics for each compartment
  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...
  for ( size_t n = 0; n < N::NCOMP; ++n )
  {
    // membrane potential for current compartment
    const double V = y[ S::idx( n, S::V_M ) ];

    // excitatory synaptic current
    const double I_syn_exc =
      ( y[ S::idx( n, S::G_EXC ) ] ) * ( V - node.P_.E_ex[ n ] );
    // inhibitory synaptic current
    const double I_syn_inh =
      ( y[ S::idx( n, S::G_INH ) ] ) * ( V - node.P_.E_in[ n ] );

    // leak current
    const double I_L = ( y[ S::idx( n, S::G_L ) ] ) * ( V - node.P_.E_L[ n ] );

    const double ica = n == N::DIST ? node.S_.ICa_ : 0.0;

    // coupling currents
    const double I_conn =
      ( n > N::SOMA
          ? node.P_.g_conn[ n - 1 ]
            * ( ( V - node.P_.E_L[ n ] )
                - ( y[ S::idx( n - 1, S::V_M ) ] - node.P_.E_L[ n - 1 ] ) )
          : 0 )
      + ( n < N::NCOMP - 1
            ? node.P_.g_conn[ n ]
              * ( ( V - node.P_.E_L[ n ] )
                  - ( y[ S::idx( n + 1, S::V_M ) ] - node.P_.E_L[ n + 1 ] ) )
            : 0 );

    // derivatives
    // membrane potential
    f[ S::idx( n, S::V_M ) ] =
      ( -I_L - I_syn_exc - I_syn_inh - I_conn + node.B_.I_stim_[ n ]
        + node.P_.I_e[ n ] + ica + y[ S::idx( n, S::I_AP ) ] )
      / node.P_.C_m[ n ];

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

    // active current during AP
    f[ S::idx( n, S::DI_AP ) ] =
      -y[ S::idx( n, S::DI_AP ) ] / node.P_.tau_currAP[ n ];
    f[ S::idx( n, S::I_AP ) ] = y[ S::idx( n, S::DI_AP ) ]
      - y[ S::idx( n, S::I_AP ) ] / node.P_.tau_currAP[ n ];
  }

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc_fixedca::Parameters_::Parameters_()
  : V_th( -55.0 )       // mV
  , V_reset( -60.0 )    // mV
  , t_ref( 2.0 )        // ms
  , V_max( 30.0 )       // mV
  , V_thCa( -24.5 )     // mV
  , Ca_amplitude( 1.0 ) // pA
  , jump_Th( 3.0 )      // mV
  , tau_Th( 3.0 )       // ms
  , Ca_active( true )
  , reset_on_spike( true )

{
  // conductances between compartments
  g_conn[ SOMA ] = 2.5; // nS, soma-proximal
  g_conn[ PROX ] = 1.0; // nS, proximal-distal

  // soma parameters
  t_L[ SOMA ] = 500.0;      // nS
  nt_L[ SOMA ] = 10.0;      // nS
  C_m[ SOMA ] = 150.0;      // pF
  E_ex[ SOMA ] = 0.0;       // mV
  E_in[ SOMA ] = -85.0;     // mV
  E_L[ SOMA ] = -70.0;      // mV
  tau_synE[ SOMA ] = 0.5;   // ms
  tau_synI[ SOMA ] = 2.0;   // ms
  I_e[ SOMA ] = 0.0;        // pA
  tau_currAP[ SOMA ] = 1.0; // ms
  amp_currAP[ SOMA ] = 0.0; // pA

  // proximal parameters
  t_L[ PROX ] = 5.0;        // nS
  nt_L[ PROX ] = 5.0;       // nS
  C_m[ PROX ] = 75.0;       // pF
  E_ex[ PROX ] = 0.0;       // mV
  E_in[ PROX ] = -85.0;     // mV
  E_L[ PROX ] = -70.0;      // mV
  tau_synE[ PROX ] = 0.5;   // ms
  tau_synI[ PROX ] = 2.0;   // ms
  I_e[ PROX ] = 0.0;        // pA
  tau_currAP[ PROX ] = 1.0; // ms
  amp_currAP[ PROX ] = 0.0; // pA

  // distal parameters
  t_L[ DIST ] = 5.0;        // nS
  nt_L[ DIST ] = 10.0;      // nS
  C_m[ DIST ] = 150.0;      // pF
  E_ex[ DIST ] = 0.0;       // mV
  E_in[ DIST ] = -85.0;     // mV
  E_L[ DIST ] = -70.0;      // mV
  tau_synE[ DIST ] = 0.5;   // ms
  tau_synI[ DIST ] = 2.0;   // ms
  I_e[ DIST ] = 0.0;        // pA
  tau_currAP[ DIST ] = 1.0; // ms
  amp_currAP[ DIST ] = 0.0; // pA
}

nest::iaf_cond_alpha_mc_fixedca::Parameters_::Parameters_(
  const Parameters_& p )
  : V_th( p.V_th )
  , V_reset( p.V_reset )
  , t_ref( p.t_ref )
  , V_max( p.V_max )
  , V_thCa( p.V_thCa )
  , Ca_amplitude( p.Ca_amplitude )
  , jump_Th( p.jump_Th )
  , tau_Th( p.tau_Th )
  , Ca_active( p.Ca_active )
  , reset_on_spike( p.reset_on_spike )

{
  for ( size_t n = 0; n < NCOMP - 1; ++n )
    g_conn[ n ] = p.g_conn[ n ];

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    t_L[ n ] = p.t_L[ n ];
    nt_L[ n ] = p.nt_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
    tau_currAP[ n ] = p.tau_currAP[ n ];
    amp_currAP[ n ] = p.amp_currAP[ n ];
  }
}

nest::iaf_cond_alpha_mc_fixedca::Parameters_&
  nest::iaf_cond_alpha_mc_fixedca::Parameters_::
  operator=( const Parameters_& p )
{
  assert( this != &p ); // would be bad logical error in program

  V_th = p.V_th;
  V_reset = p.V_reset;
  t_ref = p.t_ref;
  V_max = p.V_max;
  V_thCa = p.V_thCa;
  Ca_amplitude = p.Ca_amplitude;
  jump_Th = p.jump_Th;
  tau_Th = p.tau_Th;
  Ca_active = p.Ca_active;
  reset_on_spike = p.reset_on_spike;

  // copy C-arrays
  for ( size_t n = 0; n < NCOMP - 1; ++n )
    g_conn[ n ] = p.g_conn[ n ];

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    t_L[ n ] = p.t_L[ n ];
    nt_L[ n ] = p.nt_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
    tau_currAP[ n ] = p.tau_currAP[ n ];
    amp_currAP[ n ] = p.amp_currAP[ n ];
  }

  return *this;
}


nest::iaf_cond_alpha_mc_fixedca::State_::State_( const Parameters_& p )
  : r_( 0 )
  , rCa_( 0 )
  , numCa_( 0 )
  , th_( p.V_th )
  , ICa_( 0.0 )

{
  // for simplicity, we first initialize all values to 0,
  // then set the membrane potentials for each compartment
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = 0;

  y_[ idx( SOMA, V_M ) ] = -70.;
  y_[ idx( PROX, V_M ) ] = -65.;
  y_[ idx( DIST, V_M ) ] = -60.;
  y_[ idx( SOMA, G_L ) ] = p.nt_L[ SOMA ];
  y_[ idx( PROX, G_L ) ] = p.nt_L[ PROX ];
  y_[ idx( DIST, G_L ) ] = p.nt_L[ DIST ];
}

nest::iaf_cond_alpha_mc_fixedca::State_::State_( const State_& s )
  : r_( s.r_ )
  , rCa_( s.rCa_ )
  , numCa_( s.numCa_ )
  , th_( s.th_ )
  , ICa_( s.ICa_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
}

nest::iaf_cond_alpha_mc_fixedca::State_&
  nest::iaf_cond_alpha_mc_fixedca::State_::
  operator=( const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
  r_ = s.r_;
  rCa_ = s.rCa_;
  numCa_ = s.numCa_;
  th_ = s.th_;
  ICa_ = s.ICa_;
  return *this;
}

nest::iaf_cond_alpha_mc_fixedca::Buffers_::Buffers_(
  iaf_cond_alpha_mc_fixedca& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_cond_alpha_mc_fixedca::Buffers_::Buffers_( const Buffers_&,
  iaf_cond_alpha_mc_fixedca& n )
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
nest::iaf_cond_alpha_mc_fixedca::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::V_max, V_max );
  def< double >( d, names::V_thCa, V_thCa );
  def< double >( d, names::Ca_amplitude, Ca_amplitude );
  def< double >( d, names::jump_Th, jump_Th );
  def< double >( d, names::tau_Th, tau_Th );
  def< bool >( d, names::Ca_active, Ca_active );
  def< bool >( d, names::reset_on_spike, reset_on_spike );
  def< double >( d, Name( "g_sp" ), g_conn[ SOMA ] );
  def< double >( d, Name( "g_pd" ), g_conn[ PROX ] );

  // create subdictionaries for per-compartment parameters
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    DictionaryDatum dd = new Dictionary();
    def< double >( dd, names::t_L, t_L[ n ] );
    def< double >( dd, names::nt_L, nt_L[ n ] );
    def< double >( dd, names::E_L, E_L[ n ] );
    def< double >( dd, names::E_ex, E_ex[ n ] );
    def< double >( dd, names::E_in, E_in[ n ] );
    def< double >( dd, names::C_m, C_m[ n ] );
    def< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
    def< double >( dd, names::tau_syn_in, tau_synI[ n ] );
    def< double >( dd, names::I_e, I_e[ n ] );
    def< double >( dd, names::tau_curr_AP, tau_currAP[ n ] );
    def< double >( dd, names::amp_curr_AP, amp_currAP[ n ] );
    ( *d )[ comp_names_[ n ] ] = dd;
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::Parameters_::set( const DictionaryDatum& d )
{
  // allow setting the membrane potential
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_reset, V_reset );
  updateValue< double >( d, names::t_ref, t_ref );
  updateValue< double >( d, names::V_max, V_max );
  updateValue< double >( d, names::V_thCa, V_thCa );
  updateValue< double >( d, names::Ca_amplitude, Ca_amplitude );
  updateValue< double >( d, names::jump_Th, jump_Th );
  updateValue< double >( d, names::tau_Th, tau_Th );
  updateValue< bool >( d, names::Ca_active, Ca_active );
  updateValue< bool >( d, names::reset_on_spike, reset_on_spike );
  updateValue< double >( d, Name( "g_sp" ), g_conn[ SOMA ] );
  updateValue< double >( d, Name( "g_pd" ), g_conn[ PROX ] );

  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

      updateValue< double >( dd, names::t_L, t_L[ n ] );
      updateValue< double >( dd, names::nt_L, nt_L[ n ] );
      updateValue< double >( dd, names::E_L, E_L[ n ] );
      updateValue< double >( dd, names::E_ex, E_ex[ n ] );
      updateValue< double >( dd, names::E_in, E_in[ n ] );
      updateValue< double >( dd, names::C_m, C_m[ n ] );
      updateValue< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
      updateValue< double >( dd, names::tau_syn_in, tau_synI[ n ] );
      updateValue< double >( dd, names::I_e, I_e[ n ] );
      updateValue< double >( dd, names::tau_curr_AP, tau_currAP[ n ] );
      updateValue< double >( dd, names::amp_curr_AP, amp_currAP[ n ] );
    }

  if ( V_reset >= V_th )
    throw BadProperty( "Reset potential must be smaller than threshold." );

  if ( t_ref < 0 )
    throw BadProperty( "Refractory time cannot be negative." );

  if ( tau_Th <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );

  // apply checks compartment-wise
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( C_m[ n ] <= 0 )
      throw BadProperty( "Capacitance (" + comp_names_[ n ].toString()
        + ") must be strictly positive." );

    if ( tau_synE[ n ] <= 0 || tau_synI[ n ] <= 0 || tau_currAP[ n ] <= 0 )
      throw BadProperty( "All time constants (" + comp_names_[ n ].toString()
        + ") must be strictly positive." );
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::State_::get( DictionaryDatum& d ) const
{
  // we assume here that State_::get() always is called after
  // Parameters_::get(),
  // so that the per-compartment dictionaries exist
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    assert( d->known( comp_names_[ n ] ) );
    DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

    def< double >( dd, names::V_m, y_[ idx( n, V_M ) ] ); // Membrane potential
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::State_::set( const DictionaryDatum& d,
  const Parameters_& )
{
  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );
      updateValue< double >( dd, names::V_m, y_[ idx( n, V_M ) ] );
    }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc_fixedca::iaf_cond_alpha_mc_fixedca()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  if ( B_.step_ != 0.1 )
  {
    throw InvalidSimulationResolution( get_name() );
  }
  recordablesMap_.create();

  // set up table of compartment names
  // comp_names_.resize(NCOMP); --- Fixed size, see comment on definition
  comp_names_[ SOMA ] = Name( "soma" );
  comp_names_[ PROX ] = Name( "proximal" );
  comp_names_[ DIST ] = Name( "distal" );
}

nest::iaf_cond_alpha_mc_fixedca::iaf_cond_alpha_mc_fixedca(
  const iaf_cond_alpha_mc_fixedca& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  if ( B_.step_ != 0.1 )
  {
    throw InvalidSimulationResolution( get_name() );
  }
}

nest::iaf_cond_alpha_mc_fixedca::~iaf_cond_alpha_mc_fixedca()
{
  // GSL structs may not have been allocated, so we need to protect destruction
  if ( B_.s_ )
    gsl_odeiv_step_free( B_.s_ );
  if ( B_.c_ )
    gsl_odeiv_control_free( B_.c_ );
  if ( B_.e_ )
    gsl_odeiv_evolve_free( B_.e_ );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::init_state_( const Node& proto )
{
  const iaf_cond_alpha_mc_fixedca& pr =
    downcast< iaf_cond_alpha_mc_fixedca >( proto );
  S_ = pr.S_;
}

void
nest::iaf_cond_alpha_mc_fixedca::init_buffers_()
{
  B_.spikes_.resize( NUM_SPIKE_RECEPTORS );
  for ( size_t n = 0; n < NUM_SPIKE_RECEPTORS; ++n )
    B_.spikes_[ n ].clear(); // includes resize

  B_.currents_.resize( NUM_CURR_RECEPTORS );
  for ( size_t n = 0; n < NUM_CURR_RECEPTORS; ++n )
    B_.currents_[ n ].clear(); // includes resize

  B_.logger_.reset();
  Archiving_Node::clear_history();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
    B_.s_ =
      gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_step_reset( B_.s_ );

  if ( B_.c_ == 0 )
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  else
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );

  if ( B_.e_ == 0 )
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_evolve_reset( B_.e_ );

  B_.sys_.function = iaf_cond_alpha_mc_fixedca_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  for ( size_t n = 0; n < NCOMP; ++n )
    B_.I_stim_[ n ] = 0.0;
}

void
nest::iaf_cond_alpha_mc_fixedca::calibrate()
{
  B_.logger_
    .init(); // ensures initialization in case mm connected after Simulate

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    V_.PSConInit_E_[ n ] = 1.0 * numerics::e / P_.tau_synE[ n ];
    V_.PSConInit_I_[ n ] = 1.0 * numerics::e / P_.tau_synI[ n ];
    V_.PSConInit_AP_[ n ] = 1.0 * numerics::e / P_.tau_currAP[ n ];
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  assert( V_.RefractoryCounts_
    >= 0 ); // since t_ref >= 0, this can only fail in error

  V_.RefractoryCountsCa_ = CA_SIZE - 1;
  assert( V_.RefractoryCountsCa_
    >= 0 ); // since CA_SIZE > 0, this can only fail in error

  // Waveform for calcium spike current, obtained from kinetics model
  // so as to reproduce Fig.8 in [3]
  double tmp[] = { 0.101505459544,
    0.294432926197,
    0.827341319615,
    2.23613332778,
    5.7513806152,
    13.8254306961,
    30.2196601894,
    58.2886444295,
    97.7872982964,
    144.371168629,
    192.824575186,
    239.570320787,
    282.844159137,
    322.013557065,
    357.015741847,
    388.04918845,
    415.425498549,
    439.500548182,
    460.641738122,
    479.211474696,
    495.558181061,
    510.011061996,
    522.876949273,
    534.438448268,
    544.952989434,
    554.652556157,
    563.743937612,
    572.409393555,
    580.807639574,
    589.075075601,
    597.327191366,
    605.660091706,
    614.152092872,
    622.865348531,
    631.847471041,
    641.133119907,
    650.745535012,
    660.697997329,
    670.995204329,
    681.63455122,
    692.607312538,
    703.899721445,
    715.493946501,
    727.368967545,
    739.501353916,
    751.865949385,
    764.436469041,
    777.186014007,
    790.087510186,
    803.114077472,
    816.239335851,
    829.437654729,
    842.684351629,
    855.955846112,
    869.229774451,
    882.485070221,
    895.702015568,
    908.862267523,
    921.948863326,
    934.946208322,
    947.840049621,
    960.617438329,
    973.266682847,
    985.777295381,
    998.139933537,
    1010.3463386,
    1022.38927185,
    1034.26245001,
    1045.96048091,
    1057.47879985,
    1068.81360763,
    1079.96181048,
    1090.92096225,
    1101.6892094,
    1112.26523857,
    1122.64822722,
    1132.83779717,
    1142.83397113,
    1152.63713214,
    1162.24798593,
    1171.66752609,
    1180.89700185,
    1189.93788852,
    1198.79186028,
    1207.46076537,
    1215.94660336,
    1224.25150448,
    1232.37771088,
    1240.3275596,
    1248.10346716,
    1255.70791574,
    1263.14344059,
    1270.41261892,
    1277.51805973,
    1284.4623949,
    1291.24827115,
    1297.87834288,
    1304.35526586,
    1310.68169166,
    1316.86026264,
    1322.89360768,
    1328.78433836,
    1334.53504561,
    1340.14829688,
    1345.6266336,
    1350.9725691,
    1356.18858677,
    1361.27713851,
    1366.24064347,
    1371.08148695,
    1375.80201959,
    1380.40455666,
    1384.89137754,
    1389.26472534,
    1393.52680663,
    1397.67979128,
    1401.72581245,
    1405.66696656,
    1409.50531342,
    1413.2428764,
    1416.88164266,
    1420.42356339,
    1423.87055416,
    1427.22449526,
    1430.48723206,
    1433.66057545,
    1436.74630225,
    1439.74615567,
    1442.66184578,
    1445.49504996,
    1448.24741341,
    1450.92054964,
    1453.51604096,
    1456.035439,
    1458.48026519,
    1460.85201132,
    1463.15213997,
    1465.38208508,
    1467.54325241,
    1469.63702008,
    1471.66473903,
    1473.62773354,
    1475.5273017,
    1477.36471589,
    1479.14122328,
    1480.85804627,
    1482.51638298,
    1484.11740771,
    1485.66227136,
    1487.1521019,
    1488.58800483,
    1489.97106356,
    1491.30233988,
    1492.58287435,
    1493.81368673,
    1494.99577637,
    1496.13012261,
    1497.2176852,
    1498.25940462,
    1499.25620252,
    1500.20898206,
    1501.11862827,
    1501.98600843,
    1502.81197239,
    1503.59735292,
    1504.34296609,
    1505.04961152,
    1505.7180728,
    1506.34911773,
    1506.94349866,
    1507.50195282,
    1508.02520257,
    1508.51395576,
    1508.96890596,
    1509.39073278,
    1509.78010211,
    1510.13766647,
    1510.46406517,
    1510.75992466,
    1511.02585877,
    1511.2624689,
    1511.47034436,
    1511.65006256,
    1511.80218923,
    1511.92727871,
    1512.02587413,
    1512.00582174,
    1511.79792323,
    1511.43327988,
    1510.9384065,
    1510.33586422,
    1509.64480888,
    1508.88146608,
    1508.05954259,
    1507.19058236,
    1506.28427452,
    1505.34871965,
    1504.39065985,
    1503.41567737,
    1502.42836609,
    1501.43247926,
    1500.43105682,
    1499.42653497,
    1498.42084041,
    1497.4154712,
    1496.4115662,
    1495.40996451,
    1494.4112563,
    1493.41582628,
    1492.42389065,
    1491.43552866,
    1490.45070929,
    1489.46931384,
    1488.49115511,
    1487.51599338,
    1486.54354993,
    1485.57351824,
    1484.60557334,
    1483.6393795,
    1482.67459654,
    1481.71088496,
    1480.74791005,
    1479.78534521,
    1478.82287444,
    1477.86019432,
    1476.89701546,
    1475.93306351,
    1474.96807983,
    1474.00182185,
    1473.03406322,
    1472.06459375,
    1471.0932192,
    1470.11976093,
    1469.14405545,
    1468.16595396,
    1467.18532172,
    1466.20203748,
    1465.21599285,
    1464.22709163,
    1463.23524918,
    1462.24039175,
    1461.24245583,
    1460.24138756,
    1459.23714204,
    1458.22968281,
    1457.21898123,
    1456.20501593,
    1455.18777228,
    1454.16724189,
    1453.14342214,
    1452.11631569,
    1451.08593007,
    1450.05227728,
    1449.0153734,
    1447.97523818,
    1446.93189479,
    1445.88536941,
    1444.83569101,
    1443.78289101,
    1442.72700304,
    1441.66806271,
    1440.60610737,
    1439.54117589,
    1438.47330849,
    1437.40254655,
    1436.32893241,
    1435.25250927,
    1434.17332101,
    1433.09141205,
    1432.00682726,
    1430.91961182,
    1429.82981114,
    1428.73747074,
    1427.64263618,
    1426.54535298,
    1425.44566653,
    1424.34362205,
    1423.23926452,
    1422.1326386,
    1421.02378864,
    1419.91275857,
    1418.7995919,
    1417.68433167,
    1416.56702043,
    1415.44770018,
    1414.32641239,
    1413.20319794,
    1412.07809713,
    1410.95114964,
    1409.82239451,
    1408.69187016,
    1407.55961437,
    1406.42566423,
    1405.29005618,
    1404.15282602,
    1403.01400882,
    1401.87363903,
    1400.73175038,
    1399.58837595,
    1398.44354813,
    1397.29729863,
    1396.14965851,
    1395.00065812,
    1393.85032719,
    1392.69869474,
    1391.54578919,
    1390.39163824,
    1389.23626901,
    1388.07970793,
    1386.92198082,
    1385.76311287,
    1384.60312865,
    1383.4420521,
    1382.27990658,
    1381.11671484,
    1379.95249902,
    1378.78728071,
    1377.6210809,
    1376.45392002,
    1375.28581794,
    1374.11679397,
    1372.94686688,
    1371.7760549,
    1370.60437573,
    1369.43184656,
    1368.25848403,
    1367.0843043,
    1365.90932303,
    1364.73355538,
    1363.55701601,
    1362.37971912,
    1361.20167844,
    1360.0229072,
    1358.84341821,
    1357.6632238,
    1356.48233588,
    1355.30076588,
    1354.11852483,
    1352.93562331,
    1351.75207149,
    1350.56787911,
    1349.38305551,
    1348.19760961,
    1347.01154994,
    1345.82488461,
    1344.63762136,
    1343.44976754,
    1342.2613301,
    1341.07231563,
    1339.88273031,
    1338.69257999,
    1337.50187012,
    1336.3106058,
    1335.11879176,
    1333.92643238,
    1332.73353168,
    1331.54009331,
    1330.3461206,
    1329.15161651,
    1327.95658366,
    1326.76102432,
    1325.56494043,
    1324.36833358,
    1323.17120502,
    1321.97355567,
    1320.7753861,
    1319.57669656,
    1318.37748695,
    1317.17775685,
    1315.97750549,
    1314.77673178,
    1313.57543429,
    1312.37361124,
    1311.17126056,
    1309.96837979,
    1308.76496617,
    1307.56101659,
    1306.3565276,
    1305.15149543,
    1303.94591594,
    1302.73978466,
    1301.53309679,
    1300.32584715,
    1299.11803024,
    1297.9096402,
    1296.7006708,
    1295.49111548,
    1294.28096729,
    1293.07021893,
    1291.85886274,
    1290.64689066,
    1289.43429429,
    1288.22106481,
    1287.00719305,
    1285.79266943,
    1284.57748397,
    1283.3616263,
    1282.14508563,
    1280.92785078,
    1279.70991013,
    1278.49125165,
    1277.27186284,
    1276.05173082,
    1274.83084221,
    1273.6091832,
    1272.38673951,
    1271.1634964,
    1269.93943863,
    1268.71455049,
    1267.48881576,
    1266.26221771,
    1265.0347391,
    1263.80636215,
    1262.57706856,
    1261.34683946,
    1260.11565542,
    1258.88349643,
    1257.65034192,
    1256.41617068,
    1255.18096091,
    1253.94469018,
    1252.70733541,
    1251.46887287,
    1250.22927815,
    1248.98852616,
    1247.74659109,
    1246.50344642,
    1245.25906489,
    1244.01341846,
    1242.76647834,
    1241.51821493,
    1240.2685978,
    1239.01759571,
    1237.76517652,
    1236.51130725,
    1235.25595397,
    1233.99908185,
    1232.7406551,
    1231.48063692,
    1230.21898954,
    1228.95567413,
    1227.69065081,
    1226.42387857,
    1225.15531532,
    1223.88491779,
    1222.61264152,
    1221.33844081,
    1220.06226873,
    1218.78407704,
    1217.50381615,
    1216.22143511,
    1214.93688157,
    1213.65010168,
    1212.36104012,
    1211.06964001,
    1209.77584289,
    1208.47958864,
    1207.18081544,
    1205.87945974,
    1204.57545618,
    1203.26873753,
    1201.95923466,
    1200.64687645,
    1199.33158973,
    1198.01329922,
    1196.69192746,
    1195.36739477,
    1194.03961909,
    1192.70851599,
    1191.37399855,
    1190.03597728,
    1188.69436004,
    1187.34905193,
    1185.99995523,
    1184.64696925,
    1183.28999028,
    1181.92891145,
    1180.56362264,
    1179.19401034,
    1177.81995754,
    1176.4413436,
    1175.05804414,
    1173.66993087,
    1172.27687146,
    1170.87872939,
    1169.47536379,
    1168.0742591,
    1166.6666682,
    1165.25253464,
    1163.83178706,
    1162.40433998,
    1160.97009446,
    1159.52893873,
    1158.08074864,
    1156.62538808,
    1155.16270935,
    1153.69255338,
    1152.21474993,
    1150.72911777,
    1149.23546471,
    1147.7335877,
    1146.2232727,
    1144.70429474,
    1143.1764177,
    1141.63939421,
    1140.09296547,
    1138.53686094,
    1136.97079813,
    1135.39448227,
    1133.80760593,
    1132.20984865,
    1130.60087649,
    1128.98034158,
    1127.34788161,
    1125.70311924,
    1124.04566156,
    1122.37509943,
    1120.6910068,
    1118.99293998,
    1117.28043689,
    1115.55301623,
    1113.8101766,
    1112.05139561,
    1110.27612885,
    1108.48380889,
    1106.67384416,
    1104.8456178,
    1102.99848642,
    1101.1317788,
    1099.24479453,
    1097.33680253,
    1095.40703953,
    1093.45470844,
    1091.47897664,
    1089.47897419,
    1087.4537919,
    1085.40247937,
    1083.3240428,
    1081.21744286,
    1079.08159227,
    1076.9153534,
    1074.71753565,
    1072.48689279,
    1070.2221201,
    1067.92185142,
    1065.58465606,
    1063.20903559,
    1060.79342049,
    1058.33616673,
    1055.83555214,
    1053.28977277,
    1050.69693911,
    1048.05507227,
    1045.36210009,
    1042.6158532,
    1039.81406118,
    1036.95434871,
    1034.03423183,
    1031.05111446,
    1028.00228505,
    1024.88491372,
    1021.69604973,
    1018.43261963,
    1015.09142614,
    1011.66914785,
    1008.16234018,
    1004.56743758,
    1000.88075738,
    997.09850556,
    993.216784707,
    989.231604632,
    985.138895952,
    980.934527119,
    976.614325326,
    972.174101767,
    967.609681729,
    962.916939954,
    958.0918417,
    953.130489838,
    948.02917823,
    942.784451451,
    937.393170752,
    931.852585864,
    926.160411929,
    920.31491046,
    914.314972793,
    908.160203984,
    901.851004642,
    895.388647639,
    888.775346238,
    882.014309821,
    875.109783201,
    868.067065595,
    860.892505612,
    853.593469278,
    846.178279118,
    838.656123616,
    831.036937984,
    823.331258907,
    815.550057768,
    807.70455847,
    799.806047337,
    791.865683389,
    783.894317517,
    775.902328618,
    767.89948358,
    759.89482634,
    751.89659914,
    743.912196843,
    735.948153032,
    728.010154732,
    720.103081199,
    712.231061359,
    704.397544175,
    696.605376435,
    688.856883036,
    681.153945681,
    673.4980769,
    665.890487262,
    658.332144558,
    650.823824487,
    643.366152963,
    635.959640578,
    628.604710038,
    621.3017175,
    614.050968791,
    606.852731449,
    599.707243445,
    592.614719329,
    585.575354439,
    578.589327695,
    571.656803376,
    564.777932216,
    557.952852049,
    551.181688178,
    544.464553593,
    537.801549118,
    531.192763548,
    524.638273793,
    518.138145053,
    511.692431037,
    505.3011742,
    498.964406018,
    492.68214728,
    486.454408386,
    480.281189658,
    474.162481636,
    468.09826538,
    462.088512743,
    456.133186635,
    450.23224127,
    444.38562239,
    438.593267464,
    432.855105876,
    427.171059089,
    421.541040788,
    415.964957016,
    410.442706282,
    404.974179672,
    399.559260935,
    394.197826568,
    388.889745893,
    383.634881126,
    378.433087445,
    373.284213052,
    368.188099236,
    363.144580436,
    358.153484304,
    353.214631768,
    348.327837104,
    343.492908001,
    338.709645638,
    333.977844759,
    329.297293759,
    324.667774763,
    320.089063726,
    315.560930518,
    311.08313903,
    306.655447277,
    302.277607504,
    297.949366301,
    293.670464715,
    289.440638379,
    285.259617627,
    281.127127625,
    277.042888506,
    273.006615494,
    269.01801905,
    265.076805006,
    261.182674705,
    257.335325146,
    253.53444913,
    249.779735401,
    246.070868799,
    242.407530404,
    238.789397687,
    235.216144658,
    231.687442016,
    228.202957302,
    224.762355043,
    221.365296906,
    218.011441844,
    214.700446247,
    211.431964088,
    208.205647068,
    205.021144765,
    201.878104774,
    198.776172855,
    195.714993069,
    192.69420792,
    189.713458496,
    186.7723846,
    183.870624887,
    181.007816998,
    178.183597686,
    175.39760295,
    172.649468155,
    169.938828161,
    167.265317442,
    164.628570205,
    162.028220508,
    159.463902374,
    156.935249902,
    154.441897379,
    151.983479385,
    149.559630898,
    147.169987396,
    144.814184957,
    142.491860356,
    140.202651158,
    137.946195811,
    135.722133737,
    133.530105415,
    131.369752467,
    129.240717743,
    127.142645393,
    125.075180951,
    123.037971404,
    121.030665269,
    119.052912657,
    117.104365342,
    115.184676828,
    113.293502406,
    111.430499221,
    109.595326321,
    107.787644721,
    106.007117448,
    104.253409599,
    102.526188383,
    100.825123174,
    99.1498855488,
    97.5001493351,
    95.875590648,
    94.2758879298,
    92.7007219858,
    91.1497760185,
    89.6227356605,
    88.1192890047,
    86.6391266331,
    85.1819416437,
    83.747429676,
    82.3352889339,
    80.9452202083,
    79.5769268966,
    78.2301150217,
    76.9044932488,
    75.5997729011,
    74.315667974,
    73.051895147,
    71.8081737958,
    70.5842260014,
    69.379776559,
    68.1945529852,
    67.0282855234,
    65.8807071494,
    64.7515535741,
    63.6405632463,
    62.5474773539,
    61.4720398239,
    60.4139973219,
    59.3730992498,
    58.3490977429,
    57.3417476666,
    56.3508066109,
    55.3760348855,
    54.4171955128,
    53.4740542206,
    52.5463794343,
    51.6339422678,
    50.7365165137,
    49.8538786335,
    48.985807746,
    48.132085616,
    47.2924966418,
    46.4668278423,
    45.654868844,
    44.8564118661,
    44.0712517066,
    43.2991857273,
    42.5400138377,
    41.7935384795,
    41.05956461,
    40.3378996854,
    39.6283536431,
    38.9307388849,
    38.2448702583,
    37.5705650387,
    36.9076429105,
    36.2559259486,
    35.6152385987,
    34.9854076586,
    34.3662622578,
    33.7576338383,
    33.159356134,
    32.5712651509,
    31.9931991463,
    31.4249986083,
    30.8665062353,
    30.3175669149,
    29.7780277027,
    29.2477378019,
    28.7265485413,
    28.2143133548,
    27.7108877595,
    27.2161293347,
    26.7298977001,
    26.2520544949,
    25.7824633557,
    25.3209898955,
    24.8675016821,
    24.4218682162,
    23.9839609107,
    23.5536530686,
    23.130819862,
    22.7153383102,
    22.3070872593,
    21.9059473598,
    21.5118010465,
    21.1245325166,
    20.7440277088,
    20.3701742826,
    20.0028615973,
    19.6419806909,
    19.2874242599,
    18.9390866385,
    18.5968637779,
    18.2606532263,
    17.9303541086,
    17.6058671059,
    17.2870944362,
    16.9739398339,
    16.6663085303,
    16.3641072342,
    16.0672441121,
    15.7756287695,
    15.4891722308,
    15.2077869212,
    14.9313866474,
    14.6598865791,
    14.3932032301,
    14.1312544406,
    13.8739593581,
    13.6212384202,
    13.373013336,
    13.129207069,
    12.889743819,
    12.654549005,
    12.4235492479,
    12.1966723536,
    11.9738472959,
    11.7550041997,
    11.5400743249,
    11.3289900496,
    11.1216848541,
    10.918093305,
    10.7181510389,
    10.5217947474,
    10.3289621608,
    10.1395920336,
    9.95362412882,
    9.77099920301,
    9.59165899172,
    9.4155461947,
    9.24260446144,
    9.07277837691,
    8.90601344742,
    8.74225608669,
    8.58145360204,
    8.42355418079,
    8.26850687683,
    8.11626159733,
    7.96676908962,
    7.81998092828,
    7.6758495023,
    7.53432800254,
    7.39537040921,
    7.25893147963,
    7.12496673606,
    6.99343245374,
    6.86428564909,
    6.73748406805,
    6.61298617456,
    6.49075113925,
    6.37073882821,
    6.25290979197,
    6.13722525464,
    6.02364710311,
    5.91213787651,
    5.80266075574,
    5.69517955319,
    5.58965870257,
    5.48606324894,
    5.38435883878,
    5.2845117103,
    5.18648868386,
    5.09025715249,
    4.99578507259,
    4.90304095475,
    4.81199385471,
    4.72261336441,
    4.63486960325,
    4.54873320941,
    4.4641753313,
    4.38116761922,
    4.299682217,
    4.21969175392,
    4.14116933662,
    4.06408854124,
    3.98842340557,
    3.91414842141,
    3.84123852702,
    3.76966909963,
    3.69941594816,
    3.63045530596,
    3.56276382371,
    3.49631856246,
    3.43109698666,
    3.36707695746,
    3.30423672596,
    3.24255492668,
    3.18201057106,
    3.12258304108,
    3.06425208303,
    3.00699780128,
    2.95080065226,
    2.8956414384,
    2.84150130233,
    2.78836172102,
    2.73620450011,
    2.6850117683,
    2.63476597179,
    2.5854498689,
    2.53704652469,
    2.48953930571,
    2.44291187481,
    2.39714818607,
    2.35223247976,
    2.30814927745,
    2.26488337712,
    2.22241984841,
    2.18074402793,
    2.13984151462,
    2.09969816524,
    2.06030008989,
    2.02163364761,
    1.98368544205,
    1.94644231724,
    1.90989135342,
    1.87401986287,
    1.83881538594,
    1.80426568703,
    1.77035875068,
    1.73708277774,
    1.70442618157,
    1.67237758434,
    1.64092581337,
    1.61005989751,
    1.57976906365,
    1.55004273319,
    1.52087051867,
    1.49224222037,
    1.46414782303,
    1.4365774926,
    1.40952157304,
    1.38297058318,
    1.35691521364,
    1.33134632378,
    1.30625493873,
    1.28163224648,
    1.25746959493,
    1.23375848915,
    1.21049058849,
    1.18765770395,
    1.1652517954,
    1.14326496898,
    1.1216894745,
    1.10051770288,
    1.07974218361,
    1.05935558234,
    1.0393506984,
    1.01972046244,
    1.00045793409,
    0.98155629966,
    0.963008869849,
    0.944809077556,
    0.926950475678,
    0.909426734964,
    0.892231641905,
    0.875359096664,
    0.858803111035,
    0.842557806438,
    0.82661741196,
    0.810976262414,
    0.795628796442,
    0.780569554652,
    0.76579317778,
    0.751294404887,
    0.737068071596,
    0.723109108344,
    0.70941253868,
    0.695973477583,
    0.682787129811,
    0.669848788285,
    0.657153832494,
    0.64469772693,
    0.632476019555,
    0.620484340287,
    0.608718399522,
    0.597173986675,
    0.585846968746,
    0.574733288921,
    0.563828965185,
    0.553130088969,
    0.542632823816,
    0.532333404071,
    0.522228133599,
    0.512313384519,
    0.502585595965,
    0.493041272867,
    0.483676984752,
    0.474489364574,
    0.465475107554,
    0.456630970048,
    0.447953768435,
    0.439440378019,
    0.431087731957,
    0.422892820205,
    0.414852688479,
    0.406964437236,
    0.399225220678,
    0.391632245769,
    0.384182771268,
    0.376874106785,
    0.369703611849,
    0.362668694998,
    0.355766812876,
    0.348995469358,
    0.34235221468,
    0.335834644595,
    0.329440399533,
    0.323167163786,
    0.3170126647,
    0.310974671889,
    0.305050996455,
    0.299239490229,
    0.293538045022,
    0.28794459189,
    0.282457100413,
    0.277073577987,
    0.271792069129,
    0.266610654791,
    0.261527451694,
    0.256540611665,
    0.251648320996,
    0.246848799801,
    0.242140301401,
    0.237521111708,
    0.232989548623,
    0.228543961449,
    0.224182730308,
    0.219904265576,
    0.215707007323,
    0.211589424763,
    0.207550015716,
    0.203587306085,
    0.199699849327,
    0.195886225953,
    0.192145043021,
    0.18847493365,
    0.184874556533,
    0.181342595468,
    0.17787775889,
    0.174478779417,
    0.171144413401,
    0.167873440487,
    0.164664663183,
    0.161516906436,
    0.158429017217,
    0.155399864109,
    0.152428336909,
    0.149513346234,
    0.146653823131,
    0.143848718705,
    0.141097003739,
    0.138397668333,
    0.135749721542,
    0.133152191027,
    0.130604122706,
    0.128104580416,
    0.125652645581,
    0.123247416882,
    0.120888009936,
    0.118573556986,
    0.116303206582,
    0.114076123288,
    0.111891487375,
    0.109748494533,
    0.107646355582,
    0.105584296191,
    0.103561556601,
    0.101577391351,
    0.0 };

  V_.Ca_waveform_.assign( tmp, tmp + CA_SIZE );

  V_.AdaptThStep_ =
    numerics::expm1( -Time::get_resolution().get_ms() / P_.tau_Th );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::update( Time const& origin,
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
        throw GSLSolverFailure( get_name(), status );
    }

    S_.th_ += ( S_.th_ - P_.V_th ) * ( V_.AdaptThStep_ );

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
      // Active current triggerd after a spike to be added first at
      // proximal, then distal compartment during refractory period
      if ( S_.r_ == int( 0.5 * V_.RefractoryCounts_ ) )
      {
        S_.y_[ PROX * State_::STATE_VEC_COMPS + State_::DI_AP ] +=
          P_.amp_currAP[ PROX ] * V_.PSConInit_AP_[ PROX ];
      }
      if ( S_.r_ == 0 )
      {
        if ( P_.reset_on_spike )
          S_.y_[ State_::V_M ] = P_.V_reset;
        S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::DI_AP ] +=
          P_.amp_currAP[ DIST ] * V_.PSConInit_AP_[ DIST ];
      }
    }
    else if ( S_.y_[ State_::V_M ] >= S_.th_ )
    { // neuron fires spike
      S_.r_ = V_.RefractoryCounts_;
      S_.y_[ State_::V_M ] = P_.V_max;
      S_.th_ += P_.jump_Th;
      S_.y_[ SOMA * State_::STATE_VEC_COMPS + State_::G_L ] = P_.t_L[ SOMA ];
      S_.y_[ PROX * State_::STATE_VEC_COMPS + State_::G_L ] = P_.t_L[ PROX ];
      S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::G_L ] = P_.t_L[ DIST ];
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }
    else
    {
      S_.y_[ SOMA * State_::STATE_VEC_COMPS + State_::G_L ] = P_.nt_L[ SOMA ];
      S_.y_[ PROX * State_::STATE_VEC_COMPS + State_::G_L ] = P_.nt_L[ PROX ];
      S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::G_L ] = P_.nt_L[ DIST ];
    }

    if ( S_.rCa_ )
    { // calcium spike is absolute refractory
      --S_.rCa_;
      if ( S_.rCa_ >= 0 )
      {
        S_.ICa_ =
          V_.Ca_waveform_[ V_.RefractoryCountsCa_ - S_.rCa_ ] * P_.Ca_amplitude;
      }
      else
      {
        S_.ICa_ = 0.0;
      }
    }
    else if ( P_.Ca_active )
    { // calcium spike
      if ( ( S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::V_M ]
             >= P_.V_thCa ) && ( P_.Ca_active ) )
      {
        S_.rCa_ = V_.RefractoryCountsCa_;
        S_.ICa_ = V_.Ca_waveform_[ 0 ] * P_.Ca_amplitude;
        S_.numCa_ += 1.0;
      }
    }

    // set new input currents
    for ( size_t n = 0; n < NCOMP; ++n )
      B_.I_stim_[ n ] = B_.currents_[ n ].get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < 2 * NCOMP );

  B_.spikes_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport()
    && e.get_rport() < NCOMP ); // not 100% clean, should look at MIN, SUP

  // add weighted current; HEP 2002-10-04
  B_.currents_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
