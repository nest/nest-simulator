/*
 *  gif_cond_exp.cpp
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

#include "gif_cond_exp.h"

#ifdef HAVE_GSL

// C++ includes:
#include <limits>
#include <iomanip>
#include <iostream>
#include <cstdio>

// Includes from libnestutil:
#include "compose.hpp"
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


namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< gif_cond_exp > gif_cond_exp::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< gif_cond_exp >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &gif_cond_exp::get_y_elem_< gif_cond_exp::State_::V_M > );
  insert_( names::E_sfa, &gif_cond_exp::get_E_sfa_ );
  insert_( names::I_stc, &gif_cond_exp::get_I_stc_ );
  insert_( names::g_ex, &gif_cond_exp::get_y_elem_< gif_cond_exp::State_::G_EXC > );
  insert_( names::g_in, &gif_cond_exp::get_y_elem_< gif_cond_exp::State_::G_INH > );
}
} // namespace

extern "C" int
nest::gif_cond_exp_dynamics( double, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::gif_cond_exp::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::gif_cond_exp& node = *( reinterpret_cast< nest::gif_cond_exp* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  const double I_syn_exc = y[ S::G_EXC ] * ( y[ S::V_M ] - node.P_.E_ex_ );
  const double I_syn_inh = y[ S::G_INH ] * ( y[ S::V_M ] - node.P_.E_in_ );
  const double I_L = node.P_.g_L_ * ( y[ S::V_M ] - node.P_.E_L_ );
  const double stc = node.S_.stc_;

  // V dot
  f[ 0 ] = ( -I_L + node.S_.I_stim_ + node.P_.I_e_ - I_syn_exc - I_syn_inh - stc ) / node.P_.c_m_;

  f[ 1 ] = -y[ S::G_EXC ] / node.P_.tau_synE_;
  f[ 2 ] = -y[ S::G_INH ] / node.P_.tau_synI_;

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_cond_exp::Parameters_::Parameters_()
  : g_L_( 4.0 )        // nS
  , E_L_( -70.0 )      // mV
  , V_reset_( -55.0 )  // mV
  , Delta_V_( 0.5 )    // mV
  , V_T_star_( -35 )   // mV
  , lambda_0_( 0.001 ) // 1/ms
  , E_ex_( 0.0 )       // mV
  , E_in_( -85.0 )     // mV
  , tau_synE_( 2.0 )   // ms
  , tau_synI_( 2.0 )   // ms
  , t_ref_( 4.0 )      // ms
  , c_m_( 80.0 )       // pF
  , tau_stc_()         // ms
  , q_stc_()           // nA
  , tau_sfa_()         // ms
  , q_sfa_()           // mV
  , I_e_( 0.0 )        // pA
  , gsl_error_tol( 1e-3 )
{
}

nest::gif_cond_exp::State_::State_( const Parameters_& p )
  : I_stim_( 0.0 )
  , sfa_( 0.0 )
  , stc_( 0.0 )
  , sfa_elems_()
  , stc_elems_()
  , r_ref_( 0 )
{
  neuron_state_[ V_M ] = p.E_L_;
  neuron_state_[ G_EXC ] = neuron_state_[ G_INH ] = 0;
}

nest::gif_cond_exp::State_::State_( const State_& s )
  : I_stim_( s.I_stim_ )
  , sfa_( s.sfa_ )
  , stc_( s.stc_ )
  , sfa_elems_()
  , stc_elems_()
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
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    neuron_state_[ i ] = s.neuron_state_[ i ];
  }
}

nest::gif_cond_exp::State_& nest::gif_cond_exp::State_::operator=( const State_& s )
{
  I_stim_ = s.I_stim_;
  sfa_ = s.sfa_;
  stc_ = s.stc_;
  r_ref_ = s.r_ref_;

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
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    neuron_state_[ i ] = s.neuron_state_[ i ];
  }
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gif_cond_exp::Parameters_::get( DictionaryDatum& d ) const
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
  def< double >( d, names::tau_syn_ex, tau_synE_ );
  def< double >( d, names::tau_syn_in, tau_synI_ );
  def< double >( d, names::E_ex, E_ex_ );
  def< double >( d, names::E_in, E_in_ );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );

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
nest::gif_cond_exp::Parameters_::set( const DictionaryDatum& d, Node* node )
{

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::E_L, E_L_, node );
  updateValueParam< double >( d, names::g_L, g_L_, node );
  updateValueParam< double >( d, names::C_m, c_m_, node );
  updateValueParam< double >( d, names::V_reset, V_reset_, node );
  updateValueParam< double >( d, names::Delta_V, Delta_V_, node );
  updateValueParam< double >( d, names::V_T_star, V_T_star_, node );

  if ( updateValueParam< double >( d, names::lambda_0, lambda_0_, node ) )
  {
    lambda_0_ /= 1000.0; // convert to 1/ms
  }

  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_synE_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_synI_, node );
  updateValueParam< double >( d, names::E_ex, E_ex_, node );
  updateValueParam< double >( d, names::E_in, E_in_, node );
  updateValueParam< double >( d, names::gsl_error_tol, gsl_error_tol, node );

  updateValue< std::vector< double > >( d, names::tau_sfa, tau_sfa_ );
  updateValue< std::vector< double > >( d, names::q_sfa, q_sfa_ );
  updateValue< std::vector< double > >( d, names::tau_stc, tau_stc_ );
  updateValue< std::vector< double > >( d, names::q_stc, q_stc_ );

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
  if ( tau_synE_ <= 0 || tau_synI_ <= 0 )
  {
    throw BadProperty( "Synapse time constants must be strictly positive." );
  }
}

void
nest::gif_cond_exp::State_::get( DictionaryDatum& d, const Parameters_& ) const
{
  def< double >( d, names::V_m, neuron_state_[ V_M ] ); // Membrane potential
  def< double >( d, names::g_ex, neuron_state_[ G_EXC ] );
  def< double >( d, names::g_in, neuron_state_[ G_INH ] );
  def< double >( d, names::E_sfa, sfa_ ); // Adaptive threshold potential
  def< double >( d, names::I_stc, stc_ ); // Spike-triggered current
}

void
nest::gif_cond_exp::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, neuron_state_[ V_M ], node );
  updateValueParam< double >( d, names::g_ex, neuron_state_[ G_EXC ], node );
  updateValueParam< double >( d, names::g_in, neuron_state_[ G_INH ], node );
}

nest::gif_cond_exp::Buffers_::Buffers_( gif_cond_exp& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::gif_cond_exp::Buffers_::Buffers_( const Buffers_&, gif_cond_exp& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::gif_cond_exp::gif_cond_exp()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::gif_cond_exp::gif_cond_exp( const gif_cond_exp& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::gif_cond_exp::~gif_cond_exp()
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
nest::gif_cond_exp::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  //!< includes resize
  B_.logger_.reset();    //!< includes resize
  ArchivingNode::clear_history();

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
    B_.c_ = gsl_odeiv_control_y_new( P_.gsl_error_tol, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, P_.gsl_error_tol, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = gif_cond_exp_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
}

void
nest::gif_cond_exp::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();
  V_.rng_ = get_vp_specific_rng( get_thread() );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();

  // initializing adaptation (stc/sfa) variables
  V_.P_sfa_.resize( P_.tau_sfa_.size(), 0.0 );
  V_.P_stc_.resize( P_.tau_stc_.size(), 0.0 );

  for ( size_t i = 0; i < P_.tau_sfa_.size(); i++ )
  {
    V_.P_sfa_[ i ] = std::exp( -h / P_.tau_sfa_[ i ] );
  }
  S_.sfa_elems_.resize( P_.tau_sfa_.size(), 0.0 );

  for ( size_t i = 0; i < P_.tau_stc_.size(); i++ )
  {
    V_.P_stc_[ i ] = std::exp( -h / P_.tau_stc_[ i ] );
  }
  S_.stc_elems_.resize( P_.tau_stc_.size(), 0.0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::gif_cond_exp::update( Time const& origin, const long from, const long to )
{

  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
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
        S_.neuron_state_ );   // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    S_.neuron_state_[ State_::G_EXC ] += B_.spike_exc_.get_value( lag );
    S_.neuron_state_[ State_::G_INH ] += B_.spike_inh_.get_value( lag );

    if ( S_.r_ref_ == 0 ) // neuron is not in refractory period
    {

      const double lambda = P_.lambda_0_ * std::exp( ( S_.neuron_state_[ State_::V_M ] - S_.sfa_ ) / P_.Delta_V_ );

      if ( lambda > 0.0 )
      {

        // Draw random number and compare to prob to have a spike
        // hazard function is computed by 1 - exp(- lambda * dt)
        if ( V_.rng_->drand() < -numerics::expm1( -lambda * Time::get_resolution().get_ms() ) )
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
      S_.neuron_state_[ State_::V_M ] = P_.V_reset_;
    }

    // Set new input current
    S_.I_stim_ = B_.currents_.get_value( lag );

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::gif_cond_exp::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // EX: We must compute the arrival time of the incoming spike
  //     explicitly, since it depends on delay and offset within
  //     the update cycle.  The way it is done here works, but
  //     is clumsy and should be improved.
  if ( e.get_weight() >= 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  } // keep conductance positive
}

void
nest::gif_cond_exp::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::gif_cond_exp::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
