/*
 *  pp_cond_exp_mc_urbanczik.cpp
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


#include "pp_cond_exp_mc_urbanczik.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iostream>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"


std::vector< std::string > nest::pp_cond_exp_mc_urbanczik::comp_names_( NCOMP );
nest::RecordablesMap< nest::pp_cond_exp_mc_urbanczik > nest::pp_cond_exp_mc_urbanczik::recordablesMap_;

namespace nest
{
void
register_pp_cond_exp_mc_urbanczik( const std::string& name )
{
  register_node_model< pp_cond_exp_mc_urbanczik >( name );
}

// specialization must be place in namespace

template <>
void
RecordablesMap< pp_cond_exp_mc_urbanczik >::create()
{
  insert_( "V_m.s",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::V_M, pp_cond_exp_mc_urbanczik::SOMA > );
  insert_( "g_ex.s",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::G_EXC, pp_cond_exp_mc_urbanczik::SOMA > );
  insert_( "g_in.s",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::G_INH, pp_cond_exp_mc_urbanczik::SOMA > );
  insert_( "V_m.p",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::V_M, pp_cond_exp_mc_urbanczik::DEND > );
  insert_( "I_ex.p",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::I_EXC, pp_cond_exp_mc_urbanczik::DEND > );
  insert_( "I_in.p",
    &pp_cond_exp_mc_urbanczik::get_y_elem_< pp_cond_exp_mc_urbanczik::State_::I_INH, pp_cond_exp_mc_urbanczik::DEND > );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" int
nest::pp_cond_exp_mc_urbanczik_dynamics( double, const double y[], double f[], void* pnode )
{
  // some shorthands
  typedef nest::pp_cond_exp_mc_urbanczik N;
  typedef nest::pp_cond_exp_mc_urbanczik::State_ S;

  // get access to node so we can work almost as in a member function
  assert( pnode );
  const nest::pp_cond_exp_mc_urbanczik& node = *( reinterpret_cast< nest::pp_cond_exp_mc_urbanczik* >( pnode ) );

  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...

  // membrane potential of soma
  const double V = y[ S::idx( N::SOMA, S::V_M ) ];

  // leak current of soma
  const double I_L = node.P_.urbanczik_params.g_L[ N::SOMA ] * ( V - node.P_.urbanczik_params.E_L[ N::SOMA ] );

  // excitatory synaptic current soma
  const double I_syn_exc = y[ S::idx( N::SOMA, S::G_EXC ) ] * ( V - node.P_.E_ex[ N::SOMA ] );

  // inhibitory synaptic current soma
  const double I_syn_inh = y[ S::idx( N::SOMA, S::G_INH ) ] * ( V - node.P_.E_in[ N::SOMA ] );

  // coupling from dendrites to soma all summed up
  double I_conn_d_s = 0.0;

  // compute dynamics for each dendritic compartment
  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...
  for ( size_t n = 1; n < N::NCOMP; ++n )
  {
    // membrane potential of dendrite
    const double V_dnd = y[ S::idx( n, S::V_M ) ];

    // coupling current from dendrite to soma
    I_conn_d_s += node.P_.urbanczik_params.g_conn[ N::SOMA ] * ( V_dnd - V );

    // coupling current from soma to dendrite
    // not part of the main paper but an extension mentioned in the supplement
    const double I_conn_s_d = node.P_.urbanczik_params.g_conn[ n ] * ( V - V_dnd );

    // dendritic current due to input
    const double I_syn_ex = y[ S::idx( n, S::I_EXC ) ];
    const double I_syn_in = y[ S::idx( n, S::I_INH ) ];

    // derivative membrane potential
    // dendrite
    // In the paper the resting potential is set to zero and
    // the capacitance to one.
    f[ S::idx( n, S::V_M ) ] = ( -node.P_.urbanczik_params.g_L[ n ] * ( V_dnd - node.P_.urbanczik_params.E_L[ n ] )
                                 + I_syn_ex + I_syn_in + I_conn_s_d )
      / node.P_.urbanczik_params.C_m[ n ];

    // derivative dendritic current
    f[ S::idx( n, S::I_EXC ) ] = -I_syn_ex / node.P_.urbanczik_params.tau_syn_ex[ n ];
    f[ S::idx( n, S::I_INH ) ] = -I_syn_in / node.P_.urbanczik_params.tau_syn_in[ n ];

    // g_inh and g_exc are not used for the dendrites
    // therefore we set the corresponding derivatives to zero
    f[ S::idx( n, S::G_INH ) ] = 0.0;
    f[ S::idx( n, S::G_EXC ) ] = 0.0;
  }

  // derivative membrane potential
  // soma
  f[ S::idx( N::SOMA, S::V_M ) ] =
    ( -I_L - I_syn_exc - I_syn_inh + I_conn_d_s + node.B_.I_stim_[ N::SOMA ] + node.P_.I_e[ N::SOMA ] )
    / node.P_.urbanczik_params.C_m[ N::SOMA ]; // plus or minus I_conn_d_s?

  // excitatory conductance soma
  f[ S::idx( N::SOMA, S::G_EXC ) ] = -y[ S::idx( N::SOMA, S::G_EXC ) ] / node.P_.urbanczik_params.tau_syn_ex[ N::SOMA ];

  // inhibitory conductance soma
  f[ S::idx( N::SOMA, S::G_INH ) ] = -y[ S::idx( N::SOMA, S::G_INH ) ] / node.P_.urbanczik_params.tau_syn_in[ N::SOMA ];

  // I_EXC and I_INH are not used for the soma
  // therefore we set the corresponding derivatives to zero
  f[ S::idx( N::SOMA, S::I_EXC ) ] = 0.0;
  f[ S::idx( N::SOMA, S::I_INH ) ] = 0.0;

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::pp_cond_exp_mc_urbanczik::Parameters_::Parameters_()
  : t_ref( 3.0 ) // ms
{
  urbanczik_params.phi_max = 0.15;
  urbanczik_params.rate_slope = 0.5;
  urbanczik_params.beta = 1.0 / 3.0;
  urbanczik_params.theta = -55.0;
  // conductances between compartments
  urbanczik_params.g_conn[ SOMA ] = 600.0; // nS, soma-dendrite
  urbanczik_params.g_conn[ DEND ] = 0.0;   // nS, dendrite-soma

  // soma parameters
  urbanczik_params.g_L[ SOMA ] = 30.0;  // nS
  urbanczik_params.C_m[ SOMA ] = 300.0; // pF
  E_ex[ SOMA ] = 0.0;                   // mV
  E_in[ SOMA ] = -75;                   // mV
  urbanczik_params.E_L[ SOMA ] = -70.0; // mV
  urbanczik_params.tau_syn_ex[ SOMA ] = 3.0;
  urbanczik_params.tau_syn_in[ SOMA ] = 3.0;
  I_e[ SOMA ] = 0.0; // pA

  // dendritic parameters
  urbanczik_params.g_L[ DEND ] = 30.0;
  urbanczik_params.C_m[ DEND ] = 300.0; // pF
  E_ex[ DEND ] = 0.0;                   // mV
  E_in[ DEND ] = 0.0;                   // mV
  urbanczik_params.E_L[ DEND ] = -70.0; // mV
  urbanczik_params.tau_syn_ex[ DEND ] = 3.0;
  urbanczik_params.tau_syn_in[ DEND ] = 3.0;
  I_e[ DEND ] = 0.0; // pA
}

nest::pp_cond_exp_mc_urbanczik::Parameters_::Parameters_( const Parameters_& p )
  : t_ref( p.t_ref )
{
  urbanczik_params.phi_max = p.urbanczik_params.phi_max;
  urbanczik_params.rate_slope = p.urbanczik_params.rate_slope;
  urbanczik_params.beta = p.urbanczik_params.beta;
  urbanczik_params.theta = p.urbanczik_params.theta;
  // copy C-arrays
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    urbanczik_params.g_conn[ n ] = p.urbanczik_params.g_conn[ n ];
    urbanczik_params.g_L[ n ] = p.urbanczik_params.g_L[ n ];
    urbanczik_params.C_m[ n ] = p.urbanczik_params.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    urbanczik_params.E_L[ n ] = p.urbanczik_params.E_L[ n ];
    urbanczik_params.tau_syn_ex[ n ] = p.urbanczik_params.tau_syn_ex[ n ];
    urbanczik_params.tau_syn_in[ n ] = p.urbanczik_params.tau_syn_in[ n ];
    I_e[ n ] = p.I_e[ n ];
  }
}

nest::pp_cond_exp_mc_urbanczik::Parameters_&
nest::pp_cond_exp_mc_urbanczik::Parameters_::operator=( const Parameters_& p )
{
  assert( this != &p ); // would be bad logical error in program

  t_ref = p.t_ref;
  urbanczik_params.phi_max = p.urbanczik_params.phi_max;
  urbanczik_params.rate_slope = p.urbanczik_params.rate_slope;
  urbanczik_params.beta = p.urbanczik_params.beta;
  urbanczik_params.theta = p.urbanczik_params.theta;

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    urbanczik_params.g_conn[ n ] = p.urbanczik_params.g_conn[ n ];
    urbanczik_params.g_L[ n ] = p.urbanczik_params.g_L[ n ];
    urbanczik_params.C_m[ n ] = p.urbanczik_params.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    urbanczik_params.E_L[ n ] = p.urbanczik_params.E_L[ n ];
    urbanczik_params.tau_syn_ex[ n ] = p.urbanczik_params.tau_syn_ex[ n ];
    urbanczik_params.tau_syn_in[ n ] = p.urbanczik_params.tau_syn_in[ n ];
    I_e[ n ] = p.I_e[ n ];
  }

  return *this;
}


nest::pp_cond_exp_mc_urbanczik::State_::State_( const Parameters_& p )
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
    y_[ idx( n, V_M ) ] = p.urbanczik_params.E_L[ n ];
  }
}

nest::pp_cond_exp_mc_urbanczik::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::pp_cond_exp_mc_urbanczik::State_&
nest::pp_cond_exp_mc_urbanczik::State_::operator=( const State_& s )
{
  r_ = s.r_;
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  return *this;
}

nest::pp_cond_exp_mc_urbanczik::Buffers_::Buffers_( pp_cond_exp_mc_urbanczik& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::pp_cond_exp_mc_urbanczik::Buffers_::Buffers_( const Buffers_&, pp_cond_exp_mc_urbanczik& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::pp_cond_exp_mc_urbanczik::Parameters_::get( dictionary& d ) const
{
  d[ names::t_ref ] = t_ref;
  d[ names::phi_max ] = urbanczik_params.phi_max;
  d[ names::rate_slope ] = urbanczik_params.rate_slope;
  d[ names::beta ] = urbanczik_params.beta;
  d[ names::theta ] = urbanczik_params.theta;

  d[ names::g_sp ] = urbanczik_params.g_conn[ SOMA ];
  d[ names::g_ps ] = urbanczik_params.g_conn[ DEND ];

  // create subdictionaries for per-compartment parameters
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    dictionary dd;

    dd[ names::g_L ] = urbanczik_params.g_L[ n ];
    dd[ names::E_L ] = urbanczik_params.E_L[ n ];
    dd[ names::E_ex ] = E_ex[ n ];
    dd[ names::E_in ] = E_in[ n ];
    dd[ names::C_m ] = urbanczik_params.C_m[ n ];
    dd[ names::tau_syn_ex ] = urbanczik_params.tau_syn_ex[ n ];
    dd[ names::tau_syn_in ] = urbanczik_params.tau_syn_in[ n ];
    dd[ names::I_e ] = I_e[ n ];

    d[ comp_names_[ n ] ] = dd;
  }
}

void
nest::pp_cond_exp_mc_urbanczik::Parameters_::set( const dictionary& d )
{
  // allow setting the membrane potential
  d.update_value( names::t_ref, t_ref );
  d.update_value( names::phi_max, urbanczik_params.phi_max );
  d.update_value( names::rate_slope, urbanczik_params.rate_slope );
  d.update_value( names::beta, urbanczik_params.beta );
  d.update_value( names::theta, urbanczik_params.theta );

  d.update_value( names::g_sp, urbanczik_params.g_conn[ SOMA ] );
  d.update_value( names::g_ps, urbanczik_params.g_conn[ DEND ] );

  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( d.known( comp_names_[ n ] ) )
    {
      auto dd = d.get< dictionary >( comp_names_[ n ] );

      dd.update_value( names::E_L, urbanczik_params.E_L[ n ] );
      dd.update_value( names::E_ex, E_ex[ n ] );
      dd.update_value( names::E_in, E_in[ n ] );
      dd.update_value( names::C_m, urbanczik_params.C_m[ n ] );
      dd.update_value( names::g_L, urbanczik_params.g_L[ n ] );
      dd.update_value( names::tau_syn_ex, urbanczik_params.tau_syn_ex[ n ] );
      dd.update_value( names::tau_syn_in, urbanczik_params.tau_syn_in[ n ] );
      dd.update_value( names::I_e, I_e[ n ] );
    }
  }
  if ( urbanczik_params.rate_slope < 0 )
  {
    throw BadProperty( "Rate slope cannot be negative." );
  }

  if ( urbanczik_params.phi_max < 0 )
  {
    throw BadProperty( "Maximum rate cannot be negative." );
  }

  if ( t_ref < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  // apply checks compartment-wise
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( urbanczik_params.C_m[ n ] <= 0 )
    {
      throw BadProperty( "Capacitance (" + comp_names_[ n ] + ") must be strictly positive." );
    }

    if ( urbanczik_params.tau_syn_ex[ n ] <= 0 or urbanczik_params.tau_syn_in[ n ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }
}

void
nest::pp_cond_exp_mc_urbanczik::State_::get( dictionary& d ) const
{
  // we assume here that State_::get() always is called after
  // Parameters_::get(), so that the per-compartment dictionaries exist
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    assert( d.known( comp_names_[ n ] ) );
    auto dd = d.get< dictionary >( comp_names_[ n ] );

    dd[ names::V_m ] = y_[ idx( n, V_M ) ]; // Membrane potential
  }
}

void
nest::pp_cond_exp_mc_urbanczik::State_::set( const dictionary& d, const Parameters_& )
{
  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( d.known( comp_names_[ n ] ) )
    {
      auto dd = d.get< dictionary >( comp_names_[ n ] );
      dd.update_value( names::V_m, y_[ idx( n, V_M ) ] );
    }
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::pp_cond_exp_mc_urbanczik::pp_cond_exp_mc_urbanczik()
  : UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();

  // set up table of compartment names
  // comp_names_.resize(NCOMP); --- Fixed size, see comment on definition
  comp_names_[ SOMA ] = "soma";
  comp_names_[ DEND ] = "dendritic";
  UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >::urbanczik_params = &P_.urbanczik_params;
}

nest::pp_cond_exp_mc_urbanczik::pp_cond_exp_mc_urbanczik( const pp_cond_exp_mc_urbanczik& n )
  : UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >::urbanczik_params = &P_.urbanczik_params;
}

nest::pp_cond_exp_mc_urbanczik::~pp_cond_exp_mc_urbanczik()
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
nest::pp_cond_exp_mc_urbanczik::init_buffers_()
{
  B_.spikes_.resize( NUM_SPIKE_RECEPTORS );
  for ( size_t n = 0; n < NUM_SPIKE_RECEPTORS; ++n )
  {
    B_.spikes_[ n ].clear();
  } // includes resize

  B_.currents_.resize( NUM_CURR_RECEPTORS );
  for ( size_t n = 0; n < NUM_CURR_RECEPTORS; ++n )
  {
    B_.currents_[ n ].clear(); // includes resize
  }

  B_.logger_.reset();
  ArchivingNode::clear_history();

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

  B_.sys_.function = pp_cond_exp_mc_urbanczik_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    B_.I_stim_[ n ] = 0.0;
  }
}

void
nest::pp_cond_exp_mc_urbanczik::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
  V_.rng_ = get_vp_specific_rng( get_thread() );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  V_.h_ = Time::get_resolution().get_ms();
  // since t_ref >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );

  assert( ( int ) NCOMP == ( int ) pp_cond_exp_mc_urbanczik_parameters::NCOMP );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::pp_cond_exp_mc_urbanczik::update( Time const& origin, const long from, const long to )
{
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

    // add incoming spikes to soma
    S_.y_[ State_::G_EXC ] += B_.spikes_[ SOMA ].get_value( lag );
    S_.y_[ State_::G_INH ] += B_.spikes_[ SOMA + 1 ].get_value( lag );

    // add incoming spikes to dendrites
    for ( size_t n = 1; n < NCOMP; ++n )
    {
      S_.y_[ State_::idx( n, State_::I_EXC ) ] += B_.spikes_[ 2 * n ].get_value( lag );
      S_.y_[ State_::idx( n, State_::I_INH ) ] -= B_.spikes_[ 2 * n + 1 ].get_value( lag );
    }

    // Declaration outside if statement because we need it later
    unsigned long n_spikes = 0;

    if ( S_.r_ == 0 )
    {
      // Neuron not refractory

      // There is no reset of the membrane potential after a spike
      double rate = 1000.0 * P_.urbanczik_params.phi( S_.y_[ State_::V_M ] );

      if ( rate > 0.0 )
      {

        if ( P_.t_ref > 0.0 )
        {
          // Draw random number and compare to prob to have a spike
          if ( V_.rng_->drand() <= -numerics::expm1( -rate * V_.h_ * 1e-3 ) )
          {
            n_spikes = 1;
          }
        }
        else
        {
          // Draw Poisson random number of spikes
          poisson_distribution::param_type param( rate * V_.h_ * 1e-3 );
          n_spikes = V_.poisson_dist_( V_.rng_, param );
        }

        if ( n_spikes > 0 ) // Is there a spike? Then set the new dead time.
        {
          // Set dead time interval according to parameters
          S_.r_ = V_.RefractoryCounts_;

          // And send the spike event
          SpikeEvent se;
          se.set_multiplicity( n_spikes );
          kernel().event_delivery_manager.send( *this, se, lag );

          // Set spike time in order to make plasticity rules work
          for ( unsigned int i = 0; i < n_spikes; i++ )
          {
            set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
          }
        }
      } // if (rate > 0.0)
    }
    else // Neuron is within dead time
    {
      --S_.r_;
    }

    // Store dendritic membrane potential for Urbanczik-Senn plasticity
    write_urbanczik_history(
      Time::step( origin.get_steps() + lag + 1 ), S_.y_[ S_.idx( DEND, State_::V_M ) ], n_spikes, DEND );

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
nest::pp_cond_exp_mc_urbanczik::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() < 2 * NCOMP );

  B_.spikes_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::pp_cond_exp_mc_urbanczik::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  // not 100% clean, should look at MIN, SUP
  assert( e.get_rport() < NCOMP );

  // add weighted current; HEP 2002-10-04
  B_.currents_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::pp_cond_exp_mc_urbanczik::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
