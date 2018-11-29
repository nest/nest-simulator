/*
 *  gif_cond_exp_multisynapse.cpp
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

#include "gif_cond_exp_multisynapse.h"

#ifdef HAVE_GSL

// C++ includes:
#include <limits>
#include <iomanip>
#include <iostream>
#include <cstdio>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "integerdatum.h"
#include "doubledatum.h"

#include "compose.hpp"
#include "propagator_stability.h"
#include "event.h"

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< gif_cond_exp_multisynapse >
  gif_cond_exp_multisynapse::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< gif_cond_exp_multisynapse >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m,
    &gif_cond_exp_multisynapse::
      get_y_elem_< gif_cond_exp_multisynapse::State_::V_M > );
  insert_( names::E_sfa, &gif_cond_exp_multisynapse::get_E_sfa_ );
  insert_( names::I_stc, &gif_cond_exp_multisynapse::get_I_stc_ );
}
} // namespace

extern "C" int
nest::gif_cond_exp_multisynapse_dynamics( double,
  const double* y,
  double* f,
  void* pnode )
{
  // a shorthand
  typedef nest::gif_cond_exp_multisynapse::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::gif_cond_exp_multisynapse& node =
    *( reinterpret_cast< nest::gif_cond_exp_multisynapse* >( pnode ) );

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...
  const bool is_refractory = node.S_.r_ref_ > 0;
  const double& I_L = -node.P_.g_L_ * ( y[ S::V_M ] - node.P_.E_L_ );
  const double& stc = node.S_.stc_;
  const double& V = is_refractory ? node.P_.V_reset_ : y[ S::V_M ];

  // I_syn = - sum_k g_k (V - E_rev_k).
  double I_syn = 0.0;
  for ( size_t i = 0; i < node.P_.n_receptors(); ++i )
  {
    const size_t j = i * S::NUM_STATE_ELEMENTS_PER_RECEPTOR;
    I_syn += -y[ S::G + j ] * ( V - node.P_.E_rev_[ i ] );
  }

  // output: dv/dt
  f[ S::V_M ] = is_refractory ? 0.0 : ( I_L + node.S_.I_stim_ + node.P_.I_e_
                                        + I_syn - stc ) / node.P_.c_m_;

  // outputs: dg/dt
  for ( size_t i = 0; i < node.P_.n_receptors(); i++ )
  {
    const size_t j = i * S::NUM_STATE_ELEMENTS_PER_RECEPTOR;
    f[ S::G + j ] = -y[ S::G + j ] / node.P_.tau_syn_[ i ];
  }

  return GSL_SUCCESS;
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_cond_exp_multisynapse::Parameters_::Parameters_()
  : g_L_( 4.0 )        // nS
  , E_L_( -70.0 )      // mV
  , V_reset_( -55.0 )  // mV
  , Delta_V_( 0.5 )    // mV
  , V_T_star_( -35 )   // mV
  , lambda_0_( 0.001 ) // 1/ms
  , t_ref_( 4.0 )      // ms
  , c_m_( 80.0 )       // pF
  , tau_stc_()         // ms
  , q_stc_()           // pA
  , tau_sfa_()         // ms
  , q_sfa_()           // mV
  , tau_syn_( 1, 2.0 ) // ms
  , E_rev_( 1, 0.0 )   // mV
  , I_e_( 0.0 )        // pA
  , has_connections_( false )
  , gsl_error_tol( 1e-3 )
{
}

nest::gif_cond_exp_multisynapse::State_::State_( const Parameters_& p )
  : y_( STATE_VEC_SIZE + NUM_STATE_ELEMENTS_PER_RECEPTOR, 0.0 )
  , I_stim_( 0.0 )
  , sfa_( 0.0 )
  , stc_( 0.0 )
  , sfa_elems_()
  , stc_elems_()
  , r_ref_( 0 )
{
  y_[ V_M ] = p.E_L_;
}

nest::gif_cond_exp_multisynapse::State_::State_( const State_& s )
  : I_stim_( s.I_stim_ )
  , sfa_( s.sfa_ )
  , stc_( s.stc_ )
  , r_ref_( s.r_ref_ )
{
  sfa_elems_.resize( s.sfa_elems_.size(), 0.0 );
  for ( size_t i = 0; i < sfa_elems_.size(); ++i )
  {
    sfa_elems_[ i ] = s.sfa_elems_[ i ];
  }

  stc_elems_.resize( s.stc_elems_.size(), 0.0 );
  for ( size_t i = 0; i < stc_elems_.size(); ++i )
  {
    stc_elems_[ i ] = s.stc_elems_[ i ];
  }

  y_ = s.y_;
}

nest::gif_cond_exp_multisynapse::State_&
  nest::gif_cond_exp_multisynapse::State_::
  operator=( const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  sfa_elems_.resize( s.sfa_elems_.size(), 0.0 );
  for ( size_t i = 0; i < sfa_elems_.size(); ++i )
  {
    sfa_elems_[ i ] = s.sfa_elems_[ i ];
  }

  stc_elems_.resize( s.stc_elems_.size(), 0.0 );
  for ( size_t i = 0; i < stc_elems_.size(); ++i )
  {
    stc_elems_[ i ] = s.stc_elems_[ i ];
  }

  y_ = s.y_;

  I_stim_ = s.I_stim_;
  sfa_ = s.sfa_;
  r_ref_ = s.r_ref_;
  stc_ = s.stc_;

  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gif_cond_exp_multisynapse::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::g_L, g_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::Delta_V, Delta_V_ );
  def< double >( d, names::V_T_star, V_T_star_ );
  def< double >( d, names::lambda_0, lambda_0_ * 1000.0 ); // convert to 1/s
  def< double >( d, names::t_ref, t_ref_ );
  def< size_t >( d, names::n_receptors, n_receptors() );
  ArrayDatum E_rev_ad( E_rev_ );
  def< ArrayDatum >( d, names::E_rev, E_rev_ad );
  def< bool >( d, names::has_connections, has_connections_ );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );

  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::tau_syn, tau_syn_ad );

  ArrayDatum tau_sfa_list_ad( tau_sfa_ );
  def< ArrayDatum >( d, names::tau_sfa, tau_sfa_list_ad );

  ArrayDatum q_sfa_list_ad( q_sfa_ );
  def< ArrayDatum >( d, names::q_sfa, q_sfa_list_ad );

  ArrayDatum tau_stc_list_ad( tau_stc_ );
  def< ArrayDatum >( d, names::tau_stc, tau_stc_list_ad );

  ArrayDatum q_stc_list_ad( q_stc_ );
  def< ArrayDatum >( d, names::q_stc, q_stc_list_ad );
}

void
nest::gif_cond_exp_multisynapse::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::E_L, E_L_ );
  updateValue< double >( d, names::g_L, g_L_ );
  updateValue< double >( d, names::C_m, c_m_ );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::Delta_V, Delta_V_ );
  updateValue< double >( d, names::V_T_star, V_T_star_ );

  if ( updateValue< double >( d, names::lambda_0, lambda_0_ ) )
  {
    lambda_0_ /= 1000.0; // convert to 1/ms
  }

  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::gsl_error_tol, gsl_error_tol );

  updateValue< std::vector< double > >( d, names::tau_sfa, tau_sfa_ );
  updateValue< std::vector< double > >( d, names::q_sfa, q_sfa_ );
  updateValue< std::vector< double > >( d, names::tau_stc, tau_stc_ );
  updateValue< std::vector< double > >( d, names::q_stc, q_stc_ );

  const size_t old_n_receptors = n_receptors();
  bool Erev_flag =
    updateValue< std::vector< double > >( d, names::E_rev, E_rev_ );
  bool tau_flag =
    updateValue< std::vector< double > >( d, names::tau_syn, tau_syn_ );
  if ( Erev_flag || tau_flag )
  { // receptor arrays have been modified
    if ( ( E_rev_.size() != old_n_receptors
           || tau_syn_.size() != old_n_receptors )
      and ( not Erev_flag || not tau_flag ) )
    {
      throw BadProperty(
        "If the number of receptor ports is changed, both arrays "
        "E_rev and tau_syn must be provided." );
    }
    if ( E_rev_.size() != tau_syn_.size() )
    {
      throw BadProperty(
        "The reversal potential, and synaptic time constant arrays "
        "must have the same size." );
    }
    if ( tau_syn_.size() < old_n_receptors && has_connections_ )
    {
      throw BadProperty(
        "The neuron has connections, therefore the number of ports cannot be "
        "reduced." );
    }
    for ( size_t i = 0; i < tau_syn_.size(); ++i )
    {
      if ( tau_syn_[ i ] <= 0 )
      {
        throw BadProperty(
          "All synaptic time constants must be strictly positive" );
      }
    }
  }

  if ( tau_sfa_.size() != q_sfa_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_sfa' and 'q_sfa' need to have the same dimensions.\nSize of "
      "tau_sfa: %1\nSize of q_sfa: %2",
      tau_sfa_.size(),
      q_sfa_.size() ) );
  }

  if ( tau_stc_.size() != q_stc_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_stc' and 'q_stc' need to have the same dimensions.\nSize of "
      "tau_stc: %1\nSize of q_stc: %2",
      tau_stc_.size(),
      q_stc_.size() ) );
  }

  if ( g_L_ <= 0 )
  {
    throw BadProperty( "Membrane conductance must be strictly positive." );
  }

  if ( Delta_V_ <= 0 )
  {
    throw BadProperty( "Delta_V must be strictly positive." );
  }

  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  if ( lambda_0_ < 0 )
  {
    throw BadProperty( "lambda_0 must not be negative." );
  }

  for ( size_t i = 0; i < tau_sfa_.size(); i++ )
  {
    if ( tau_sfa_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }

  for ( size_t i = 0; i < tau_stc_.size(); i++ )
  {
    if ( tau_stc_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }
}

void
nest::gif_cond_exp_multisynapse::State_::get( DictionaryDatum& d,
  const Parameters_& p ) const
{
  def< double >( d, names::V_m, y_[ V_M ] ); // Membrane potential
  def< double >( d, names::E_sfa, sfa_ );    // Adaptive threshold potential
  def< double >( d, names::I_stc, stc_ );    // Spike-triggered current


  std::vector< double >* g = new std::vector< double >();

  for ( size_t i = 0;
        i < ( y_.size() - State_::NUMBER_OF_FIXED_STATES_ELEMENTS );
        ++i )
  {
    g->push_back(
      y_[ State_::G + State_::NUM_STATE_ELEMENTS_PER_RECEPTOR * i ] );
  }

  ( *d )[ names::g ] = DoubleVectorDatum( g );
}

void
nest::gif_cond_exp_multisynapse::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );
  y_.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
      + State_::NUM_STATE_ELEMENTS_PER_RECEPTOR * p.n_receptors(),
    0.0 );

  sfa_elems_.resize( p.tau_sfa_.size(), 0.0 );
  stc_elems_.resize( p.tau_stc_.size(), 0.0 );
}

nest::gif_cond_exp_multisynapse::Buffers_::Buffers_(
  gif_cond_exp_multisynapse& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( Time::get_resolution().get_ms() )
  , IntegrationStep_( step_ )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::gif_cond_exp_multisynapse::Buffers_::Buffers_( const Buffers_& b,
  gif_cond_exp_multisynapse& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( b.step_ )
  , IntegrationStep_( b.IntegrationStep_ )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::gif_cond_exp_multisynapse::gif_cond_exp_multisynapse()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::gif_cond_exp_multisynapse::gif_cond_exp_multisynapse(
  const gif_cond_exp_multisynapse& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::gif_cond_exp_multisynapse::~gif_cond_exp_multisynapse()
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
nest::gif_cond_exp_multisynapse::init_state_( const Node& proto )
{
  const gif_cond_exp_multisynapse& pr =
    downcast< gif_cond_exp_multisynapse >( proto );
  S_ = pr.S_;
}

void
nest::gif_cond_exp_multisynapse::init_buffers_()
{
  B_.spikes_.resize( P_.n_receptors() );
  for ( size_t i = 0; i < P_.n_receptors(); ++i )
  {
    B_.spikes_[ i ].clear(); // includes resize
  }

  B_.currents_.clear(); //!< includes resize
  B_.logger_.reset();   //!< includes resize
  Archiving_Node::clear_history();

  const int state_size = 1 + ( State_::STATE_VEC_SIZE - 1 ) * P_.n_receptors();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, state_size );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_y_new( P_.gsl_error_tol, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, P_.gsl_error_tol, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( state_size );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = gif_cond_exp_multisynapse_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = state_size;
  B_.sys_.params = reinterpret_cast< void* >( this );
}

void
nest::gif_cond_exp_multisynapse::calibrate()
{
  B_.sys_.dimension = S_.y_.size();

  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );

  // initializing adaptation (stc/sfa) variables
  V_.P_sfa_.resize( P_.tau_sfa_.size(), 0.0 );
  V_.P_stc_.resize( P_.tau_stc_.size(), 0.0 );

  for ( size_t i = 0; i < P_.tau_sfa_.size(); i++ )
  {
    V_.P_sfa_[ i ] = std::exp( -h / P_.tau_sfa_[ i ] );
  }

  for ( size_t i = 0; i < P_.tau_stc_.size(); i++ )
  {
    V_.P_stc_[ i ] = std::exp( -h / P_.tau_stc_[ i ] );
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::gif_cond_exp_multisynapse::update( Time const& origin,
  const long from,
  const long to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    // exponential decaying stc and sfa elements
    S_.stc_ = 0.0;
    for ( size_t i = 0; i < S_.stc_elems_.size(); i++ )
    {
      S_.stc_ += S_.stc_elems_[ i ];
      S_.stc_elems_[ i ] = V_.P_stc_[ i ] * S_.stc_elems_[ i ];
    }

    S_.sfa_ = P_.V_T_star_;
    for ( size_t i = 0; i < S_.sfa_elems_.size(); i++ )
    {
      S_.sfa_ += S_.sfa_elems_[ i ];
      S_.sfa_elems_[ i ] = V_.P_sfa_[ i ] * S_.sfa_elems_[ i ];
    }

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

    double t = 0.0;

    while ( t < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &t,                   // from t
        B_.step_,             // to t <= step
        &B_.IntegrationStep_, // integration step size
        &S_.y_[ 0 ] );        // neuronal state converted to double[]

      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    for ( size_t i = 0; i < P_.n_receptors(); i++ )
    {
      S_.y_[ State_::G + ( State_::NUM_STATE_ELEMENTS_PER_RECEPTOR * i ) ] +=
        B_.spikes_[ i ].get_value( lag );
    }

    if ( S_.r_ref_ == 0 ) // neuron is not in refractory period
    {

      const double lambda = P_.lambda_0_
        * std::exp( ( S_.y_[ State_::V_M ] - S_.sfa_ ) / P_.Delta_V_ );

      if ( lambda > 0.0 )
      {

        // Draw random number and compare to prob to have a spike
        // hazard function is computed by 1 - exp(- lambda * dt)
        if ( V_.rng_->drand()
          < -numerics::expm1( -lambda * Time::get_resolution().get_ms() ) )
        {

          for ( size_t i = 0; i < S_.stc_elems_.size(); i++ )
          {
            S_.stc_elems_[ i ] += P_.q_stc_[ i ];
          }

          for ( size_t i = 0; i < S_.sfa_elems_.size(); i++ )
          {
            S_.sfa_elems_[ i ] += P_.q_sfa_[ i ];
          }

          S_.r_ref_ = V_.RefractoryCounts_;

          // And send the spike event
          set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
          SpikeEvent se;
          kernel().event_delivery_manager.send( *this, se, lag );
        }
      }
    }
    else
    { // neuron is absolute refractory
      --S_.r_ref_;
      S_.y_[ State_::V_M ] = P_.V_reset_;
    }


    // Set new input current
    S_.I_stim_ = B_.currents_.get_value( lag );

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::gif_cond_exp_multisynapse::handle( SpikeEvent& e )
{
  if ( e.get_weight() < 0 )
  {
    throw BadProperty(
      "Synaptic weights for conductance based models "
      "must be positive." );
  }
  assert( e.get_delay_steps() > 0 );
  assert(
    ( e.get_rport() > 0 ) && ( ( size_t ) e.get_rport() <= P_.n_receptors() ) );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::gif_cond_exp_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * I );
}

void
nest::gif_cond_exp_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
