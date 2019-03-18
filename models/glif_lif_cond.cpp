/*
 *  glif_lif_cond.cpp
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

#include "glif_lif_cond.h"

// C++ includes:
#include <limits>
#include <iostream>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"
#include "name.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "lockptrdatum.h"


using namespace nest;

nest::RecordablesMap< nest::glif_lif_cond >
  nest::glif_lif_cond::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< nest::glif_lif_cond >::create()
{
  insert_( names::V_m,
    &nest::glif_lif_cond::get_y_elem_< nest::glif_lif_cond::State_::V_M > );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" inline int
nest::glif_lif_cond_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // a shorthand
  typedef nest::glif_lif_cond::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::glif_lif_cond& node =
    *( reinterpret_cast< nest::glif_lif_cond* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  double I_syn = 0.0;
  for ( size_t i = 0; i < node.P_.n_receptors_(); ++i )
  {
    const size_t j = i * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR;
    I_syn += y[ S::G_SYN + j ] * ( y[ S::V_M ] - node.P_.E_rev_[ i ] );
  }

  const double I_leak = node.P_.G_ * ( y[ S::V_M ] - node.P_.E_L_ );

  // dV_m/dt
  f[ 0 ] = ( -I_leak - I_syn + node.B_.I_stim_ ) / node.P_.C_m_;

  // d dg_exc/dt, dg_exc/dt
  for ( size_t i = 0; i < node.P_.n_receptors_(); ++i )
  {
    const size_t j = i * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR;
    // Synaptic conductance derivative dG/dt
    f[ S::DG_SYN + j ] = -y[ S::DG_SYN + j ] / node.P_.tau_syn_[ i ];
    f[ S::G_SYN + j ] =
      y[ S::DG_SYN + j ] - ( y[ S::G_SYN + j ] / node.P_.tau_syn_[ i ] );
  }

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::glif_lif_cond::Parameters_::Parameters_()
  : th_inf_( 26.5 )    // mV
  , G_( 4.6951 )       // nS (1/Gohm)
  , E_L_( -77.4 )      // mV
  , C_m_( 99.182 )     // pF
  , t_ref_( 0.5 )      // ms
  , V_reset_( -77.4 )  // mV
  , tau_syn_( 1, 2.0 ) // ms
  , E_rev_( 1, -70.0 ) // mV
  , has_connections_( false )

{
}

nest::glif_lif_cond::State_::State_( const Parameters_& p )
  : y_( STATE_VECTOR_MIN_SIZE, 0.0 )

{
  y_[ V_M ] = p.E_L_; // initialize to membrane potential
}

nest::glif_lif_cond::State_::State_( const State_& s )

{
  y_ = s.y_;
}

nest::glif_lif_cond::State_& nest::glif_lif_cond::State_::operator=(
  const State_& s )
{

  if ( this == &s ) // avoid assignment to self
  {
    return *this;
  }

  y_ = s.y_;

  return *this;
}


/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::glif_lif_cond::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, th_inf_ );
  def< double >( d, Name( "g_m" ), G_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::V_reset, V_reset_ );
  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::tau_syn, tau_syn_ad );
  ArrayDatum E_rev_ad( E_rev_ );
  def< ArrayDatum >( d, names::E_rev, E_rev_ad );
  def< bool >( d, names::has_connections, has_connections_ );
}

void
nest::glif_lif_cond::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, th_inf_ );
  updateValue< double >( d, Name( "g_m" ), G_ );
  updateValue< double >( d, names::E_L, E_L_ );
  updateValue< double >( d, names::C_m, C_m_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::V_reset, V_reset_ );

  if ( V_reset_ >= th_inf_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  if ( C_m_ <= 0.0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( G_ <= 0.0 )
  {
    throw BadProperty( "Membrane conductance must be strictly positive." );
  }

  if ( t_ref_ <= 0.0 )
  {
    throw BadProperty( "Refractory time constant must be strictly positive." );
  }

  const size_t old_n_receptors = this->n_receptors_();
  bool tau_flag =
    updateValue< std::vector< double > >( d, "tau_syn", tau_syn_ );
  bool Erev_flag = updateValue< std::vector< double > >( d, "E_rev", E_rev_ );

  if ( tau_flag || Erev_flag )
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

    if ( this->n_receptors_() != old_n_receptors && has_connections_ == true )
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
          "All synaptic time constants must be strictly positive." );
      }
    }
  }
}

void
nest::glif_lif_cond::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );

  std::vector< double >* dg = new std::vector< double >();
  std::vector< double >* g = new std::vector< double >();

  for ( size_t i = 0;
        i < ( ( y_.size() - State_::NUMBER_OF_FIXED_STATES_ELEMENTS )
              / State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR );
        ++i )
  {
    dg->push_back( y_[ State_::DG_SYN
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
    g->push_back( y_[ State_::G_SYN
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
  }

  ( *d )[ names::dg ] = DoubleVectorDatum( dg );
  ( *d )[ names::g ] = DoubleVectorDatum( g );
}

void
nest::glif_lif_cond::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
  // Only the membrane potential can be set; one could also make other state
  // variables settable.
  updateValue< double >( d, names::V_m, y_[ V_M ] );
}

nest::glif_lif_cond::Buffers_::Buffers_( glif_lif_cond& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( Time::get_resolution().get_ms() )
  , IntegrationStep_( std::min( 0.01, step_ ) )
  , I_stim_( 0.0 )
{
}

nest::glif_lif_cond::Buffers_::Buffers_( const Buffers_& b, glif_lif_cond& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( b.step_ )
  , IntegrationStep_( b.IntegrationStep_ )
  , I_stim_( b.I_stim_ )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::glif_lif_cond::glif_lif_cond()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::glif_lif_cond::glif_lif_cond( const glif_lif_cond& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::glif_lif_cond::~glif_lif_cond()
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
nest::glif_lif_cond::init_state_( const Node& proto )
{
  const glif_lif_cond& pr = downcast< glif_lif_cond >( proto );
  S_ = pr.S_;
}

void
nest::glif_lif_cond::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // include resize
  B_.logger_.reset();   // includes resize

  B_.step_ = Time::get_resolution().get_ms();
  // We must integrate this model with high-precision to obtain decent results
  B_.IntegrationStep_ = std::min( 0.01, B_.step_ );

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  B_.sys_.function = glif_lif_cond_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::glif_lif_cond::calibrate()
{
  B_.logger_.init();

  V_.t_ref_remaining_ = 0.0;
  V_.t_ref_total_ = P_.t_ref_; // in ms

  V_.CondInitialValues_.resize( P_.n_receptors_() );

  B_.spikes_.resize( P_.n_receptors_() );

  S_.y_.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * P_.n_receptors_() ),
    0.0 );

  for ( size_t i = 0; i < P_.n_receptors_(); i++ )
  {
    V_.CondInitialValues_[ i ] = 1.0 * numerics::e / P_.tau_syn_[ i ];
    B_.spikes_[ i ].resize();
  }

  // reallocate instance of stepping function for ODE GSL solver
  if ( B_.s_ != 0 )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
  B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, S_.y_.size() );

  // reallocate instance of evolution function for ODE GSL solver
  if ( B_.e_ != 0 )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
  B_.e_ = gsl_odeiv_evolve_alloc( S_.y_.size() );

  B_.sys_.dimension = S_.y_.size();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::glif_lif_cond::update( Time const& origin,
  const long from,
  const long to )
{

  const double dt = Time::get_resolution().get_ms(); // in ms
  double v_old = S_.y_[ State_::V_M ];

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
        &S_.y_[ 0 ] );        // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    if ( V_.t_ref_remaining_ > 0.0 )
    {
      // While neuron is in refractory period count-down in time steps (since dt
      // may change while in refractory) while holding the voltage at last peak.
      V_.t_ref_remaining_ -= dt;
      if ( V_.t_ref_remaining_ <= 0.0 )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
      }
      else
      {
        S_.y_[ State_::V_M ] = v_old;
      }
    }
    else
    {

      if ( S_.y_[ State_::V_M ] >= P_.th_inf_ )
      {

        V_.t_ref_remaining_ = V_.t_ref_total_;

        // Determine spike offset and send spike event
        double spike_offset =
          ( 1 - ( P_.th_inf_ - v_old ) / ( S_.y_[ State_::V_M ] - v_old ) )
          * Time::get_resolution().get_ms();
        set_spiketime(
          Time::step( origin.get_steps() + lag + 1 ), spike_offset );
        SpikeEvent se;
        se.set_offset( spike_offset );
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }

    // add incoming spike
    for ( size_t i = 0; i < P_.n_receptors_(); i++ )
    {
      // Apply spikes delivered in this step: The spikes arriving at T+1 have an
      // immediate effect on the state of the neuron
      S_.y_[ State_::DG_SYN + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                                * i ) ] += B_.spikes_[ i ].get_value( lag )
        * V_.CondInitialValues_[ i ]; // add incoming spike
    }

    B_.I_stim_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( origin.get_steps() + lag );

    v_old = S_.y_[ State_::V_M ];
  }
}


nest::port
nest::glif_lif_cond::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type <= 0
    || receptor_type > static_cast< port >( P_.n_receptors_() ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }

  P_.has_connections_ = true;
  return receptor_type;
}


void
nest::glif_lif_cond::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::glif_lif_cond::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::glif_lif_cond::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e ); // the logger does this for us
}
