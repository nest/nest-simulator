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

<<<<<<< HEAD
=======

>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
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
<<<<<<< HEAD
  nt_L[ SOMA ] = 10.0;      // nS
=======
  nt_L[ SOMA ] = 15.0;      // nS
  g_L[ SOMA ] = 10.0;       // nS
>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
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
<<<<<<< HEAD
  nt_L[ PROX ] = 5.0;       // nS
=======
  nt_L[ PROX ] = 10.0;      // nS
  g_L[ PROX ] = 5.0;        // nS
>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
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
<<<<<<< HEAD
  nt_L[ DIST ] = 10.0;      // nS
=======
  nt_L[ DIST ] = 15.0;      // nS
  g_L[ DIST ] = 10.0;       // nS
>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
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

<<<<<<< HEAD
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
=======
  // clang-format off
  double tmp [] = {0.113805061345,2.57207665232,18.02752317,48.114851194,85.6436354848,128.220315409,173.216927443,218.423863108,262.925421389,306.489717242,349.112491753,390.843755892,431.733619358,471.823738396,511.150042808,549.738877611,587.609414528,624.775717584,661.244954807,697.018244723,732.094555251,766.472879664,800.153928882,833.134755215,865.400840475,896.94413699,927.753702162,957.824119589,987.161765639,1015.76828911,1043.64028973,1070.77212765,1097.15959686,1122.79624734,1147.67503772,1171.78927916,1195.13449965,1217.7113385,1239.5229808,1260.57216866,1280.85856267,1300.3823753,1319.15042263,1337.17144501,1354.45296787,1371.0056946,1386.84110397,1401.97297186,1416.41611455,1430.18572133,1443.30020284,1455.77646823,1467.62979231,1478.87540735,1489.53098862,1499.61782948,1509.15858215,1518.18231644,1526.71933347,1534.79386441,1542.42560126,1549.62754258,1556.41320802,1562.80746263,1568.83615284,1574.52140349,1579.87205151,1584.90454074,1589.64818584,1594.12204491,1598.34252662,1602.32154243,1606.06491015,1609.57660112,1612.86777674,1615.95719494,1618.86108026,1621.58734276,1624.14441992,1626.54333639,1628.80052582,1630.9262585,1632.92331089,1634.79507018,1636.54980257,1638.1963792,1639.74243747,1641.19678003,1642.56151916,1643.83507239,1645.01794436,1646.12227113,1647.15426479,1648.10980187,1648.98687404,1649.78430366,1650.506708,1651.1617283,1651.75765513,1652.30286116,1652.79955215,1653.24692827,1653.64575826,1653.99835872,1654.31053409,1654.58566801,1654.8259252,1655.02928695,1655.19449773,1655.3299823,1655.43907779,1655.52127023,1655.57987024,1655.61156327,1655.61431701,1655.59470485,1655.55412535,1655.49136896,1655.41100313,1655.31508708,1655.20870781,1655.09962685,1654.98805189,1654.87307229,1654.75139255,1654.61465196,1654.45770978,1654.29007684,1654.11257118,1653.91887835,1653.70425318,1653.45744893,1653.18186443,1652.8856851,1652.56911548,1652.2287645,1651.85646771,1651.44984549,1651.01563677,1650.56070426,1650.08573735,1649.58626069,1649.0607038,1648.50634931,1647.92782738,1647.32896996,1646.70424845,1646.05080423,1645.36962204,1644.67248825,1643.96631617,1643.25143872,1642.52926104,1641.80219514,1641.0717279,1640.33917833,1639.6093511,1638.87832674,1638.14136141,1637.39793101,1636.64628233,1635.88275776,1635.10838145,1634.32434556,1633.52506389,1632.71057285,1631.88582239,1631.04912535,1630.20501515,1629.34778326,1628.4693724,1627.5749934,1626.67293805,1625.76423134,1624.84081167,1623.90129996,1622.94719813,1621.96371022,1620.94872731,1619.91464871,1618.86877358,1617.81445818,1616.7536454,1615.68686916,1614.60677091,1613.51653525,1612.42279632,1611.32285649,1610.21734915,1609.10908765,1607.99625335,1606.87207165,1605.73367114,1604.58580718,1603.42745263,1602.2563389,1601.07405087,1599.8771964,1598.66725182,1597.45119976,1596.23192728,1595.00788329,1593.77635136,1592.53388949,1591.27629698,1590.008634,1588.73801819,1587.46355843,1586.1850879,1584.91018259,1583.64109198,1582.37847784,1581.12667289,1579.88719222,1578.65890834,1577.43698358,1576.2162045,1574.99217227,1573.76505085,1572.5375206,1571.31204754,1570.0878171,1568.86517202,1567.6474138,1566.43162693,1565.21423353,1563.99183704,1562.76240574,1561.5232335,1560.2751458,1559.01817771,1557.75002902,1556.46721769,1555.16738867,1553.85937197,1552.55589151,1551.26127734,1549.96145207,1548.64825828,1547.32689751,1546.00077877,1544.67944741,1543.36605585,1542.05691775,1540.75010129,1539.44722941,1538.15583329,1536.87985316,1535.61770495,1534.36896172,1533.13286062,1531.90356334,1530.67737625,1529.45525602,1528.23397786,1527.01091411,1525.78760429,1524.56412504,1523.34084405,1522.11892724,1520.89548514,1519.66927907,1518.44116777,1517.21063358,1515.97994774,1514.75098915,1513.52240878,1512.28645729,1511.04303186,1509.80350479,1508.57332243,1507.36125266,1506.17088331,1504.99810118,1503.83845515,1502.68693667,1501.54541383,1500.41326892,1499.28749465,1498.16453038,1497.04564003,1495.93024774,1494.81274416,1493.6893789,1492.5524788,1491.39832146,1490.23085431,1489.0538837,1487.86579307,1486.66487283,1485.45347166,1484.22891203,1482.98601166,1481.72458075,1480.44970584,1479.16851642,1477.88321317,1476.59342441,1475.30049486,1474.00590424,1472.70776447,1471.40315358,1470.08822392,1468.75969081,1467.41867341,1466.06814632,1464.71142785,1463.34992263,1461.98438729,1460.62094438,1459.26178099,1457.90429823,1456.54511135,1455.1801917,1453.80638424,1452.42156682,1451.03020819,1449.6356229,1448.24151434,1446.85217648,1445.47091509,1444.09740672,1442.72766693,1441.35912846,1439.9887615,1438.61352754,1437.2331003,1435.85064176,1434.46629022,1433.07871917,1431.6886702,1430.29137743,1428.88383948,1427.46880221,1426.05012411,1424.63112382,1423.20986972,1421.78204359,1420.3422579,1418.88326679,1417.40963166,1415.92758071,1414.44277891,1412.94930739,1411.44221209,1409.93356481,1408.43556623,1406.95473968,1405.48930326,1404.03388589,1402.58211218,1401.13246958,1399.68629963,1398.24342848,1396.80279203,1395.36001385,1393.91173814,1392.4590354,1391.00214462,1389.54010791,1388.07090953,1386.59221586,1385.10237303,1383.60182035,1382.09375947,1380.58218071,1379.07455417,1377.57672104,1376.086262,1374.59961093,1373.1167372,1371.63857108,1370.16297319,1368.69158863,1367.22781199,1365.76831096,1364.31151068,1362.8558221,1361.40016764,1359.94374674,1358.48544867,1357.02300401,1355.55682408,1354.08658711,1352.60968448,1351.1287602,1349.64512065,1348.15846282,1346.66616782,1345.16607855,1343.65835141,1342.14511175,1340.63014399,1339.09304742,1337.52637112,1335.95364894,1334.38223006,1332.81103138,1331.24772858,1329.69653854,1328.15433861,1326.61972993,1325.08796029,1323.55438224,1322.02219535,1320.49192177,1318.96169113,1317.432706,1315.90255946,1314.36452089,1312.81422679,1311.25332637,1309.6868102,1308.11747685,1306.54344907,1304.95907624,1303.3602152,1301.74390309,1300.10775443,1298.45263913,1296.78007612,1295.09234583,1293.39019508,1291.67421399,1289.94735243,1288.21191536,1286.46623999,1284.70920124,1282.9442224,1281.17476606,1279.40212508,1277.6261545,1275.84825096,1274.06887318,1272.28710816,1270.49928505,1268.70111065,1266.89434131,1265.08021394,1263.25626088,1261.42223417,1259.57976379,1257.72640146,1255.85583308,1253.96578733,1252.05619553,1250.12216168,1248.16184243,1246.18013248,1244.181028,1242.16320398,1240.12000691,1238.05043141,1235.95775879,1233.84414395,1231.6957699,1229.50713194,1227.28794078,1225.04307789,1222.77937855,1220.49908075,1218.2026686,1215.89380054,1213.57070931,1211.23033824,1208.87209207,1206.49299239,1204.09217185,1201.67079388,1199.22962621,1196.76561776,1194.27710631,1191.76417554,1189.22611967,1186.66364254,1184.08013788,1181.47942354,1178.86286818,1176.2289322,1173.57646846,1170.90327413,1168.20606748,1165.48449396,1162.73748258,1159.96248104,1157.15674466,1154.31525823,1151.43582179,1148.51534501,1145.54854552,1142.5327673,1139.46362641,1136.34208875,1133.17097716,1129.95146939,1126.68299937,1123.36541991,1120.00225004,1116.5937351,1113.13888039,1109.63564865,1106.07958083,1102.46792812,1098.79952731,1095.07299489,1091.28561738,1087.43493899,1083.51887658,1079.5348956,1075.48033446,1071.32732558,1067.06869205,1062.72930984,1058.32466592,1053.86115015,1049.33900677,1044.7574558,1040.11249238,1035.39958451,1030.61636236,1025.76354374,1020.84198402,1015.8476865,1010.77538298,1005.62473733,1000.40048663,995.104132885,989.73470897,984.29114096,978.773538198,973.183389307,967.521569932,961.78879934,955.986355001,950.115204937,944.176200299,938.171477924,932.103194002,925.973667667,919.784231302,913.534581861,907.225299968,900.856497155,894.428840483,887.943238631,881.401389467,874.805273935,868.157658364,861.464050917,854.72781196,847.950485999,841.133597625,834.279419612,827.389833812,820.465519764,813.508343419,806.518633912,799.497169344,792.446067886,785.365607854,778.256970907,771.123052361,763.967431609,756.793081666,749.601866199,742.398065448,735.187327767,727.974187436,720.760538633,713.547585987,706.338677913,699.135959944,691.940482755,684.754613364,677.577166216,670.40730965,663.247923726,656.100334501,648.966895917,641.849999549,634.749605354,627.666781095,620.603102944,613.559916268,606.538438814,599.542383908,592.576381439,585.642653631,578.74260529,571.878194927,565.051380612,558.263724021,551.516912156,544.81201153,538.150168265,531.532684345,524.960582134,518.434149438,511.954664338,505.524413869,498.938531257,492.737790076,486.537048896,480.336307715,474.135566535,468.081244968,462.026923401,455.972601834,449.918280267,444.087144368,438.256008468,432.424872569,426.59373667,420.986676969,415.379617269,409.772557569,404.165497869,398.771247521,393.376997172,387.982746824,382.588496475,377.407850654,372.227204832,367.04655901,361.865913188,356.901880739,351.937848291,346.973815842,342.009783394,337.258138801,332.506494208,327.754849615,323.003205022,318.454323491,313.90544196,309.35656043,304.807678899,300.464516029,296.12135316,291.77819029,287.435027421,283.387015958,279.339004494,275.290993031,271.242981568,267.194970105,263.146958641,259.098947178,255.050935715,251.417348255,247.783760795,244.150173335,240.516585876,236.882998416,233.249410956,229.615823496,225.982236036,222.689902822,219.397569607,216.105236392,212.812903178,209.520569963,206.228236748,202.935903534,199.957443589,196.978983644,194.000523698,191.022063753,188.043603808,185.065143863,182.086683918,179.108223973,176.452216715,173.796209457,171.140202199,168.484194942,165.828187684,163.172180426,160.516173168,157.86016591,155.490160205,153.1201545,150.750148795,148.38014309,146.010137384,143.640131679,141.270125974,138.900120269,136.775497915,134.650875561,132.526253208,130.401630854,128.2770085,126.152386146,124.027763793,122.062860176,120.09795656,118.133052943,116.168149326,114.313064442,112.457979557,110.602894672,108.747809787,107.049982604,105.352155421,103.654328238,101.956501055,100.258673872,98.5608466894,96.8630195064,95.1651923235,93.6718636897,92.1785350559,90.685206422,89.1918777882,87.6985491544,86.2052205206,84.7118918868,83.218563253,81.8922722108,80.5659811687,79.2396901266,77.9133990844,76.5871080423,75.2608170002,73.934525958,72.7129223239,71.4913186898,70.2697150557,69.0481114216,67.8979621885,66.7478129555,65.5976637225,64.4475144894,63.4001803863,62.3528462832,61.30551218,60.2581780769,59.2108439738,58.1635098707,57.1161757675,56.0688416644,55.1557095851,54.2425775059,53.3294454266,52.4163133473,51.5031812681,50.5900491888,49.6769171095,48.7637850303,47.9584066843,47.1530283384,46.3476499925,45.5422716466,44.7368933007,43.9315149548,43.1261366088,42.4160985878,41.7060605668,40.9960225458,40.2859845248,39.5759465038,38.8659084828,38.1558704618,37.4458324408,36.825016781,36.2042011213,35.5833854615,34.9625698017,34.3417541419,33.7209384822,33.1001228224,32.4793071626,31.9492444826,31.4191818027,30.8891191227,30.3590564427,29.8289937627,29.2989310828,28.7688684028,28.2388057228,27.7916491298,27.3444925367,26.8973359437,26.4501793506,26.0030227576,25.5558661645,25.1087095715,24.778410471,24.4481113705,24.11781227,23.7875131695,23.457214069,23.1269149685,22.796615868,22.4663167675,22.136017667,21.8057185665,21.475419466,21.1451203655,20.814821265,20.4845221645,20.154223064,19.8239239635,19.5823199452,19.340715927,19.0991119087,18.8575078905,18.6159038722,18.3742998539,18.1326958357,17.8910918174,17.6494877992,17.4078837809,17.1662797627,16.9246757444,16.6830717262,16.4414677079,16.1998636897,16.0215306927,15.8431976957,15.6648646987,15.4865317017,15.3081987047,15.1298657077,14.9515327107,14.7731997137,14.5948667167,14.4165337197,14.2382007227,14.0598677257,13.8815347287,13.7032017317,13.5248687347,13.3465357377,13.2033478434,13.0601599491,12.9169720549,12.7737841606,12.6305962663,12.487408372,12.3442204777,12.2010325835,12.0578446892,11.9146567949,11.7714689006,11.6282810064,11.4850931121,11.3419052178,11.1987173235,11.0555294292,10.912341535,10.7691536407,10.6259657464,10.4827778521,10.3395899578,10.1964020636,10.0532141693,9.91002627501,9.76683838073,9.62365048645,9.48046259217,9.33727469789,9.19408680361,9.05089890934,8.90771101506,8.76452312078,8.6213352265,8.47814733222,8.33495943794,8.19177154366,8.04858364938,7.90539575511,7.76220786083,7.61901996655,7.47583207227,7.33264417799,7.18945628371,7.04626838943,6.90308049515,6.75989260088,6.6167047066,6.47351681232,6.33032891804,6.18714102376,6.04395312948,5.9007652352,5.75757734092,5.61438944664,5.47120155237,5.32801365809,5.18482576381,5.04163786953,4.89844997525,4.75526208097,4.61207418669,4.46888629241,4.32569839814,4.18251050386,4.03932260958,3.8961347153,3.75294682102,3.60975892674,3.46657103246,3.32338313818,3.18019524391,3.03700734963,2.89381945535,2.75063156107,2.60744366679,2.46425577251,2.32106787823,2.17787998395,2.03469208967,1.8915041954,1.74831630112,1.60512840684,1.46194051256,1.31875261828,1.175564724,1.03237682972,0.889188935444,0.746001041165,0.602813146887,0.459625252608,0.316437358329,0.17324946405,0.0300615697713,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  //!< Waveform for calcium spike current
  // clang-format on
>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755

  V_.Ca_waveform_.assign( tmp, tmp + CA_SIZE );

  V_.AdaptThStep_ =
    numerics::expm1( -Time::get_resolution().get_ms() / P_.tau_Th );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::update( Time const& origin,
  const long_t from,
  const long_t to )
{

<<<<<<< HEAD
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
=======

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );

>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
  assert( from < to );

  for ( long_t lag = from; lag < to; ++lag )
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
<<<<<<< HEAD
      // Active current triggerd after a spike to be added first at
      // proximal, then distal compartment during refractory period
=======
>>>>>>> 78a21ddcb22b0e25dbce5d58d6e1c9451fb42755
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
      S_.G_L[ SOMA ] = P_.t_L[ SOMA ];
      S_.G_L[ PROX ] = P_.t_L[ PROX ];
      S_.G_L[ DIST ] = P_.t_L[ DIST ];
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }
    else
    {
      S_.G_L[ SOMA ] = P_.nt_L[ SOMA ];
      S_.G_L[ PROX ] = P_.nt_L[ PROX ];
      S_.G_L[ DIST ] = P_.nt_L[ DIST ];
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
