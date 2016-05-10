/*
 *  iaf_cond_alpha_mc_kinetics.cpp
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


#include "iaf_cond_alpha_mc_kinetics.h"

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
std::vector< Name > nest::iaf_cond_alpha_mc_kinetics::comp_names_( NCOMP );

/* ----------------------------------------------------------------
 * Receptor dictionary
 * ---------------------------------------------------------------- */

// leads to seg fault on exit, see #328
// DictionaryDatum nest::iaf_cond_alpha_mc_kinetics::receptor_dict_ = new Dictionary();

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_cond_alpha_mc_kinetics >
  nest::iaf_cond_alpha_mc_kinetics::recordablesMap_;

namespace nest
{
// specialization must be place in namespace

template <>
void
RecordablesMap< iaf_cond_alpha_mc_kinetics >::create()
{
  insert_( Name( "V_m.s" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::V_M,
      iaf_cond_alpha_mc_kinetics::SOMA > );
  insert_( Name( "g_ex.s" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_EXC,
      iaf_cond_alpha_mc_kinetics::SOMA > );
  insert_( Name( "g_in.s" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_INH,
      iaf_cond_alpha_mc_kinetics::SOMA > );
  insert_( Name( "i_ap.s" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::I_AP,
      iaf_cond_alpha_mc_kinetics::SOMA > );

  insert_( Name( "V_m.p" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::V_M,
      iaf_cond_alpha_mc_kinetics::PROX > );
  insert_( Name( "g_ex.p" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_EXC,
      iaf_cond_alpha_mc_kinetics::PROX > );
  insert_( Name( "g_in.p" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_INH,
      iaf_cond_alpha_mc_kinetics::PROX > );
  insert_( Name( "i_ap.p" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::I_AP,
      iaf_cond_alpha_mc_kinetics::PROX > );

  insert_( Name( "V_m.d" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::V_M,
      iaf_cond_alpha_mc_kinetics::DIST > );
  insert_( Name( "g_ex.d" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_EXC,
      iaf_cond_alpha_mc_kinetics::DIST > );
  insert_( Name( "g_in.d" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::G_INH,
      iaf_cond_alpha_mc_kinetics::DIST > );
  insert_( Name( "i_ap.d" ),
    &iaf_cond_alpha_mc_kinetics::get_y_elem_< iaf_cond_alpha_mc_kinetics::State_::I_AP,
      iaf_cond_alpha_mc_kinetics::DIST > );

  
  insert_( names::t_ref_remaining, &iaf_cond_alpha_mc_kinetics::get_r_ );
  insert_( names::threshold, &iaf_cond_alpha_mc_kinetics::get_th_ );
  insert_( names::ca_current, &iaf_cond_alpha_mc_kinetics::get_ica_ );
  
 
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" int
nest::iaf_cond_alpha_mc_kinetics_dynamics( double, const double y[], double f[], void* pnode )
{
  // some shorthands
  typedef nest::iaf_cond_alpha_mc_kinetics N;
  typedef nest::iaf_cond_alpha_mc_kinetics::State_ S;

  // get access to node so we can work almost as in a member function
  assert( pnode );
  const nest::iaf_cond_alpha_mc_kinetics& node =
    *( reinterpret_cast< nest::iaf_cond_alpha_mc_kinetics* >( pnode ) );

  // compute dynamics for each compartment
  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...
  for ( size_t n = 0; n < N::NCOMP; ++n )
  {
    // membrane potential for current compartment
    const double V = y[ S::idx( n, S::V_M ) ];

    // excitatory synaptic current
    const double I_syn_exc = ( y[ S::idx( n, S::G_EXC ) ] ) * ( V - node.P_.E_ex[ n ] );
    // inhibitory synaptic current
    const double I_syn_inh = ( y[ S::idx( n, S::G_INH ) ] ) * ( V - node.P_.E_in[ n ] );

    // leak current
    const double I_L = node.P_.g_L[ n ] * ( V - node.P_.E_L[ n ] );

    double minf = 0.0;
    double hinf = 0.0;
    if ( n == N::DIST )
    {
      if ( node.P_.Ca_active )
      {
        minf =
          1.0 / ( 1.0 + std::exp( ( node.P_.half_m - V ) * node.P_.slope_m ) );
        hinf =
          1.0 / ( 1.0 + std::exp( ( node.P_.half_h - V ) * node.P_.slope_h ) );
      }
    }


    const double ica =
      y[ S::idx( n, S::M_CA ) ] * y[ S::idx( n, S::H_CA ) ] * node.P_.G_Ca * ( node.P_.E_Ca - V );

    // coupling currents
    const double I_conn =
      ( n > N::SOMA
          ? node.P_.g_conn[ n - 1 ]
            * ( ( V - node.P_.E_L[ n ] ) - ( y[ S::idx( n - 1, S::V_M ) ] - node.P_.E_L[ n - 1 ] ) )
          : 0 )
      + ( n < N::NCOMP - 1
            ? node.P_.g_conn[ n ] * ( ( V - node.P_.E_L[ n ] )
                                      - ( y[ S::idx( n + 1, S::V_M ) ] - node.P_.E_L[ n + 1 ] ) )
            : 0 );

    // derivatives
    // membrane potential
    f[ S::idx( n, S::V_M ) ] =
      ( -I_L - I_syn_exc - I_syn_inh - I_conn + node.B_.I_stim_[ n ] + node.P_.I_e[ n ] + ica
        + y[ S::idx( n, S::I_AP ) ] ) / node.P_.C_m[ n ];

    // excitatory conductance
    f[ S::idx( n, S::DG_EXC ) ] = -y[ S::idx( n, S::DG_EXC ) ] / node.P_.tau_synE[ n ];
    f[ S::idx( n, S::G_EXC ) ] =
      y[ S::idx( n, S::DG_EXC ) ] - y[ S::idx( n, S::G_EXC ) ] / node.P_.tau_synE[ n ];

    // inhibitory conductance
    f[ S::idx( n, S::DG_INH ) ] = -y[ S::idx( n, S::DG_INH ) ] / node.P_.tau_synI[ n ];
    f[ S::idx( n, S::G_INH ) ] =
      y[ S::idx( n, S::DG_INH ) ] - y[ S::idx( n, S::G_INH ) ] / node.P_.tau_synI[ n ];

    // active current during AP
    f[ S::idx( n, S::DI_AP ) ] = -y[ S::idx( n, S::DI_AP ) ] / node.P_.tau_currAP[ n ];
    f[ S::idx( n, S::I_AP ) ] =
      y[ S::idx( n, S::DI_AP ) ] - y[ S::idx( n, S::I_AP ) ] / node.P_.tau_currAP[ n ];

    if ( ( n == N::DIST ) && ( node.P_.Ca_active ) )
    {
      f[ S::idx( n, S::M_CA ) ] = ( minf - y[ S::idx( n, S::M_CA ) ] ) / node.P_.tau_m;
      f[ S::idx( n, S::H_CA ) ] = ( hinf - y[ S::idx( n, S::H_CA ) ] ) / node.P_.tau_h;
    }
    else
    {
      f[ S::idx( n, S::M_CA ) ] = 0.0;
      f[ S::idx( n, S::H_CA ) ] = 0.0;
    }	

  }

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc_kinetics::Parameters_::Parameters_()
  : V_th( -55.0 )    // mV
  , V_reset( -60.0 ) // mV
  , t_ref( 2.0 ) // ms
  , V_max( 30.0 ) // mV
  , E_Ca( 50.0 ) // mV
  , G_Ca( 70.0 ) // nS
  , tau_m( 15.0 ) // ms
  , tau_h( 80.0 ) // ms
  , half_m( -9.0 ) // mV
  , half_h( -21.0 ) // mV
  , slope_m( 0.5 ) // mV-1
  , slope_h( -0.5 ) // mV-1
  , jump_Th( 25.0 ) // mV
  , tau_Th( 7.0 ) // ms
  , Ca_active( true )
  , reset_on_spike( true )

{
  // conductances between compartments
  g_conn[ SOMA ] = 2.5; // nS, soma-proximal
  g_conn[ PROX ] = 1.0; // nS, proximal-distal

  // soma parameters
  t_L[ SOMA ] = 500.0;    // nS
  nt_L[ SOMA ] = 15.0;    // nS
  g_L[ SOMA ] = 10.0;     // nS
  C_m[ SOMA ] = 150.0;    // pF
  E_ex[ SOMA ] = 0.0;     // mV
  E_in[ SOMA ] = -85.0;   // mV
  E_L[ SOMA ] = -70.0;    // mV
  tau_synE[ SOMA ] = 0.5; // ms
  tau_synI[ SOMA ] = 2.0; // ms
  I_e[ SOMA ] = 0.0;      // pA
  tau_currAP[ SOMA ] = 1.0; // ms
  amp_currAP[ SOMA ] = 0.0; // pA

  // proximal parameters
  t_L[ PROX ] = 5.0;      // nS
  nt_L[ PROX ] = 10.0;    // nS
  g_L[ PROX ] = 5.0;      // nS
  C_m[ PROX ] = 75.0;     // pF
  E_ex[ PROX ] = 0.0;     // mV
  E_in[ PROX ] = -85.0;   // mV
  E_L[ PROX ] = -70.0;    // mV
  tau_synE[ PROX ] = 0.5; // ms
  tau_synI[ PROX ] = 2.0; // ms
  I_e[ PROX ] = 0.0;      // pA
  tau_currAP[ PROX ] = 1.0; // ms
  amp_currAP[ PROX ] = 0.0; // pA

  // distal parameters
  t_L[ DIST ] = 5.0;      // nS
  nt_L[ DIST ] = 15.0;    // nS
  g_L[ DIST ] = 10.0;     // nS
  C_m[ DIST ] = 150.0;    // pF
  E_ex[ DIST ] = 0.0;     // mV
  E_in[ DIST ] = -85.0;   // mV
  E_L[ DIST ] = -70.0;    // mV
  tau_synE[ DIST ] = 0.5; // ms
  tau_synI[ DIST ] = 2.0; // ms
  I_e[ DIST ] = 0.0;      // pA
  tau_currAP[ DIST ] = 1.0; // ms
  amp_currAP[ DIST ] = 0.0; // pA
}

nest::iaf_cond_alpha_mc_kinetics::Parameters_::Parameters_( const Parameters_& p )
  : V_th( p.V_th )
  , V_reset( p.V_reset )
  , t_ref( p.t_ref )
  , V_max( p.V_max )
  , E_Ca( p.E_Ca )
  , G_Ca( p.G_Ca )
  , tau_m( p.tau_m )
  , tau_h( p.tau_h )
  , half_m( p.half_m )
  , half_h( p.half_h )
  , slope_m( p.slope_m )
  , slope_h( p.slope_h )
  , jump_Th( p.jump_Th )
  , tau_Th( p.tau_Th )
  , Ca_active( p.Ca_active )
  , reset_on_spike( p.reset_on_spike )

{
  // copy C-arrays
  for ( size_t n = 0; n < NCOMP - 1; ++n )
    g_conn[ n ] = p.g_conn[ n ];

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    t_L[ n ] = p.t_L[ n ];
    nt_L[ n ] = p.nt_L[ n ];
    g_L[ n ] = p.g_L[ n ];
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

nest::iaf_cond_alpha_mc_kinetics::Parameters_& nest::iaf_cond_alpha_mc_kinetics::Parameters_::
operator=( const Parameters_& p )
{
  assert( this != &p ); // would be bad logical error in program

  V_th = p.V_th;
  V_reset = p.V_reset;
  t_ref = p.t_ref;
  V_max = p.V_max;
  E_Ca = p.E_Ca;
  G_Ca = p.G_Ca;
  tau_m = p.tau_m;
  tau_h = p.tau_h;
  half_m = p.half_m;
  half_h = p.half_h;
  slope_m = p.slope_m;
  slope_h = p.slope_h;
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
    g_L[ n ] = p.g_L[ n ];
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


nest::iaf_cond_alpha_mc_kinetics::State_::State_( const Parameters_& p )
  : r_( 0 )
  , th_( p.V_th )

{
  // for simplicity, we first initialize all values to 0,
  // then set the membrane potentials for each compartment
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = 0;

  y_[ idx( SOMA, V_M ) ] = -70.;
  y_[ idx( PROX, V_M ) ] = -65.;
  y_[ idx( DIST, V_M ) ] = -60.;

  const double_t minf =
    1.0 / ( 1.0 + std::exp( ( p.half_m - y_[ idx( DIST, V_M ) ] ) * p.slope_m ) );

  const double_t hinf =
    1.0 / ( 1.0 + std::exp( ( p.half_h - y_[ idx( DIST, V_M ) ] ) * p.slope_h ) );
  y_[ idx( DIST, M_CA ) ] = minf;
  y_[ idx( DIST, H_CA ) ] = hinf;
  ICa_ = y_[ idx( DIST, M_CA ) ] * y_[ idx( DIST, H_CA ) ] * p.G_Ca * ( p.E_Ca - y_[ idx( DIST, V_M ) ] );
}

nest::iaf_cond_alpha_mc_kinetics::State_::State_( const State_& s )
  : r_( s.r_ )
  , th_( s.th_ )
  , ICa_( s.ICa_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
}

nest::iaf_cond_alpha_mc_kinetics::State_& nest::iaf_cond_alpha_mc_kinetics::State_::operator=(
  const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
  r_ = s.r_;
  th_ = s.th_;
  ICa_ = s.ICa_;
  return *this;
}

nest::iaf_cond_alpha_mc_kinetics::Buffers_::Buffers_( iaf_cond_alpha_mc_kinetics& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_cond_alpha_mc_kinetics::Buffers_::Buffers_( const Buffers_&,
  iaf_cond_alpha_mc_kinetics& n )
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
nest::iaf_cond_alpha_mc_kinetics::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::V_max, V_max );
  def< double >( d, names::E_Ca, E_Ca );
  def< double >( d, names::G_Ca, G_Ca );
  def< double >( d, names::tau_m, tau_m );
  def< double >( d, names::tau_h, tau_h );
  def< double >( d, names::half_m, half_m );
  def< double >( d, names::half_h, half_h );
  def< double >( d, names::slope_m, slope_m );
  def< double >( d, names::slope_h, slope_h );
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
    def< double >( dd, names::g_L, g_L[ n ] );
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
nest::iaf_cond_alpha_mc_kinetics::Parameters_::set( const DictionaryDatum& d )
{
  // allow setting the membrane potential
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_reset, V_reset );
  updateValue< double >( d, names::t_ref, t_ref );
  updateValue< double >( d, names::V_max, V_max );
  updateValue< double >( d, names::E_Ca, E_Ca );
  updateValue< double >( d, names::G_Ca, G_Ca );
  updateValue< double >( d, names::tau_m, tau_m );
  updateValue< double >( d, names::tau_h, tau_h );
  updateValue< double >( d, names::half_m, half_m );
  updateValue< double >( d, names::half_h, half_h );
  updateValue< double >( d, names::slope_m, slope_m );
  updateValue< double >( d, names::slope_h, slope_h );
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
      updateValue< double >( dd, names::g_L, g_L[ n ] );
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

  if ( tau_m <= 0 || tau_h <= 0 || tau_Th <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );

  // apply checks compartment-wise
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( C_m[ n ] <= 0 )
      throw BadProperty(
        "Capacitance (" + comp_names_[ n ].toString() + ") must be strictly positive." );

    if ( tau_synE[ n ] <= 0 || tau_synI[ n ] <= 0 || tau_currAP[ n ] <= 0 )
      throw BadProperty(
        "All time constants (" + comp_names_[ n ].toString() + ") must be strictly positive." );
  }
}

void
nest::iaf_cond_alpha_mc_kinetics::State_::get( DictionaryDatum& d ) const
{
  // we assume here that State_::get() always is called after Parameters_::get(),
  // so that the per-compartment dictionaries exist
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    assert( d->known( comp_names_[ n ] ) );
    DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

    def< double >( dd, names::V_m, y_[ idx( n, V_M ) ] ); // Membrane potential
  }
}

void
nest::iaf_cond_alpha_mc_kinetics::State_::set( const DictionaryDatum& d, const Parameters_& )
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

nest::iaf_cond_alpha_mc_kinetics::iaf_cond_alpha_mc_kinetics()
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

nest::iaf_cond_alpha_mc_kinetics::iaf_cond_alpha_mc_kinetics( const iaf_cond_alpha_mc_kinetics& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::iaf_cond_alpha_mc_kinetics::~iaf_cond_alpha_mc_kinetics()
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
nest::iaf_cond_alpha_mc_kinetics::init_state_( const Node& proto )
{
  const iaf_cond_alpha_mc_kinetics& pr = downcast< iaf_cond_alpha_mc_kinetics >( proto );
  S_ = pr.S_;
}

void
nest::iaf_cond_alpha_mc_kinetics::init_buffers_()
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
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
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

  B_.sys_.function = iaf_cond_alpha_mc_kinetics_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  for ( size_t n = 0; n < NCOMP; ++n )
    B_.I_stim_[ n ] = 0.0;
}

void
nest::iaf_cond_alpha_mc_kinetics::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    V_.PSConInit_E_[ n ] = 1.0 * numerics::e / P_.tau_synE[ n ];
    V_.PSConInit_I_[ n ] = 1.0 * numerics::e / P_.tau_synI[ n ];
    V_.PSConInit_AP_[ n ] = 1.0 * numerics::e / P_.tau_currAP[ n ];
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  assert( V_.RefractoryCounts_ >= 0 ); // since t_ref >= 0, this can only fail in error

  V_.AdaptThStep_ = numerics::expm1( -Time::get_resolution().get_ms() / P_.tau_Th );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_kinetics::update( Time const& origin, const long_t from, const long_t to )
{

  assert( to >= 0 && ( delay ) from < kernel().connection_builder_manager.get_min_delay() );
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

    S_.ICa_ = S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::M_CA ]
      * S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::H_CA ] * P_.G_Ca
      * ( P_.E_Ca - S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::V_M ] );

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


    // spike handling
    if ( S_.r_ )
    { // neuron is absolute refractory
      --S_.r_;
      if ( S_.r_ == int(0.5*V_.RefractoryCounts_) )
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
      P_.g_L[ SOMA ] = P_.t_L[ SOMA ];
      P_.g_L[ PROX ] = P_.t_L[ PROX ];
      P_.g_L[ DIST ] = P_.t_L[ DIST ];
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }
    else
    {
      P_.g_L[ SOMA ] = P_.nt_L[ SOMA ];
      P_.g_L[ PROX ] = P_.nt_L[ PROX ];
      P_.g_L[ DIST ] = P_.nt_L[ DIST ];
    }


    // set new input currents
    for ( size_t n = 0; n < NCOMP; ++n )
      B_.I_stim_[ n ] = B_.currents_[ n ].get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_cond_alpha_mc_kinetics::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < 2 * NCOMP );

  B_.spikes_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_cond_alpha_mc_kinetics::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < NCOMP ); // not 100% clean, should look at MIN, SUP

  // add weighted current; HEP 2002-10-04
  B_.currents_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
nest::iaf_cond_alpha_mc_kinetics::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
