/*
 *  siegert_neuron.cpp
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


#include "siegert_neuron.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

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

struct my_params
{
  double a;
  double b;
};

/* ----------------------------------------------------------------
 * Integrands for evaluation of siegert formula
 * ---------------------------------------------------------------- */

double
integrand1( double x, void* p )
{
  struct my_params* params = ( struct my_params* ) p;
  double y_th = ( params->a );
  double y_r = ( params->b );
  if ( x == 0 )
  {
    return exp( -y_th * y_th ) * 2 * ( y_th - y_r );
  }
  else
  {
    return exp( -( x - y_th ) * ( x - y_th ) ) * ( 1. - exp( 2 * ( y_r - y_th ) * x ) ) / x;
  }
}

double
integrand2( double x, void* p )
{
  struct my_params* params = ( struct my_params* ) p;
  double y_th = ( params->a );
  double y_r = ( params->b );
  if ( x == 0 )
  {
    return 2 * ( y_th - y_r );
  }
  else
  {
    return ( exp( 2 * y_th * x - x * x ) - exp( 2 * y_r * x - x * x ) ) / x;
  }
}

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< siegert_neuron > siegert_neuron::recordablesMap_;

// // Override the create() method with one call to RecordablesMap::insert_()
// // for each quantity to be recorded.
template <>
void
RecordablesMap< siegert_neuron >::create()
{
  insert_( names::rate, &siegert_neuron::get_rate_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::siegert_neuron::Parameters_::Parameters_()
  : tau_( 1.0 )     // ms
  , tau_m_( 5.0 )   // ms
  , tau_syn_( 0.0 ) // ms
  , t_ref_( 2.0 )   // ms
  , mean_( 0.0 )    // 1/ms
  , theta_( 15.0 )  // mV, rel to E_L_
  , V_reset_( 0.0 ) // mV, rel to E_L_
{
}

nest::siegert_neuron::State_::State_()
  : r_( 0.0 )
{
}


/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::siegert_neuron::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::mean, mean_ );
  def< double >( d, names::theta, theta_ );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::tau, tau_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::tau_syn, tau_syn_ );
  def< double >( d, names::t_ref, t_ref_ );
}

void
nest::siegert_neuron::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::mean, mean_, node );
  updateValueParam< double >( d, names::theta, theta_, node );
  updateValueParam< double >( d, names::V_reset, V_reset_, node );
  updateValueParam< double >( d, names::tau, tau_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::tau_syn, tau_syn_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );

  if ( V_reset_ >= theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  if ( tau_ <= 0 )
  {
    throw BadProperty( "time constant must be > 0." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant must be > 0." );
  }

  if ( tau_syn_ < 0 )
  {
    throw BadProperty( "Membrane time constant must not be negative." );
  }
}

void
nest::siegert_neuron::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, r_ ); // Rate
}

void
nest::siegert_neuron::State_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, r_, node ); // Rate
}

nest::siegert_neuron::Buffers_::Buffers_( siegert_neuron& n )
  : logger_( n )
{
}

nest::siegert_neuron::Buffers_::Buffers_( const Buffers_&, siegert_neuron& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::siegert_neuron::siegert_neuron()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
  gsl_w_ = gsl_integration_workspace_alloc( 1000 );
}

nest::siegert_neuron::siegert_neuron( const siegert_neuron& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
  gsl_w_ = gsl_integration_workspace_alloc( 1000 );
}

nest::siegert_neuron::~siegert_neuron()
{
  gsl_integration_workspace_free( gsl_w_ );
}

/* ----------------------------------------------------------------
 * Siegert function and helpers
 * ---------------------------------------------------------------- */

double
nest::siegert_neuron::siegert1( double theta_shift, double V_reset_shift, double mu, double sigma )
{
  double y_th;
  y_th = ( theta_shift - mu ) / sigma;
  double y_r;
  y_r = ( V_reset_shift - mu ) / sigma;

  double result, error;

  gsl_function F;
  F.function = &integrand1;

  struct my_params alpha = { y_th, y_r };
  F.params = &alpha;

  double lower_bound = y_th;
  double err = 1.;
  while ( err >= 1e-12 )
  {
    err = integrand1( lower_bound, &alpha );
    if ( err > 1e-12 )
    {
      lower_bound /= 2.;
    }
  }

  double upper_bound = y_th;
  err = 1.;
  while ( err >= 1e-12 )
  {
    err = integrand1( upper_bound, &alpha );
    if ( err > 1e-12 )
    {
      upper_bound *= 2.;
    }
  }

  gsl_integration_qags( &F, lower_bound, upper_bound, 0.0, 1.49e-8, 1000, gsl_w_, &result, &error );

  // factor 1e3 due to conversion from kHz to Hz, as time constant in ms.
  return 1e3 * 1. / ( P_.t_ref_ + exp( y_th * y_th ) * result * P_.tau_m_ );
}

double
nest::siegert_neuron::siegert2( double theta_shift, double V_reset_shift, double mu, double sigma )
{
  double y_th;
  y_th = ( theta_shift - mu ) / sigma;
  double y_r;
  y_r = ( V_reset_shift - mu ) / sigma;

  double result, error;

  gsl_function F;
  F.function = &integrand2;

  struct my_params alpha = { y_th, y_r };
  F.params = &alpha;

  double lower_bound = 0.;
  double upper_bound = 1.;
  double err = 1.;
  while ( err >= 1e-12 )
  {
    err = integrand2( upper_bound, &alpha );
    if ( err > 1e-12 )
    {
      upper_bound *= 2.;
    }
  }

  gsl_integration_qags( &F, lower_bound, upper_bound, 0.0, 1.49e-8, 1000, gsl_w_, &result, &error );

  // factor 1e3 due to conversion from kHz to Hz, as time constant in ms.
  return 1e3 * 1. / ( P_.t_ref_ + result * P_.tau_m_ );
}

double
nest::siegert_neuron::siegert( double mu, double sigma_square )
{
  double sigma = std::sqrt( sigma_square );

  // Effective shift of threshold and reset due to colored noise:
  // alpha = |zeta(1/2)|/sqrt(2) with zeta being the Riemann zeta
  // function (Fourcaud & Brunel, 2002)
  double alpha = 2.0652531522312172;

  double theta_shift = P_.theta_ + sigma * alpha / 2. * sqrt( P_.tau_syn_ / P_.tau_m_ );
  double V_r_shift = P_.V_reset_ + sigma * alpha / 2. * sqrt( P_.tau_syn_ / P_.tau_m_ );

  // Catch cases where neurons get no input.
  // Use (Brunel, 2000) eq. (22) to estimate
  // firing rate to be ~ 1e-16
  if ( ( theta_shift - mu ) > 6. * sigma )
  {
    return 0.;
  }
  if ( mu <= theta_shift - 0.05 * std::abs( theta_shift ) )
  {
    return siegert1( theta_shift, V_r_shift, mu, sigma );
  }
  else
  {
    return siegert2( theta_shift, V_r_shift, mu, sigma );
  }
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::siegert_neuron::init_state_( const Node& proto )
{
  const siegert_neuron& pr = downcast< siegert_neuron >( proto );
  S_ = pr.S_;
}

void
nest::siegert_neuron::init_buffers_()
{
  // resize buffers
  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  B_.drift_input_.resize( buffer_size, 0.0 );
  B_.diffusion_input_.resize( buffer_size, 0.0 );
  B_.last_y_values.resize( buffer_size, 0.0 );

  B_.logger_.reset(); // includes resize
  Archiving_Node::clear_history();
}

void
nest::siegert_neuron::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms();

  // propagators
  V_.P1_ = std::exp( -h / P_.tau_ );
  V_.P2_ = -numerics::expm1( -h / P_.tau_ );
}

/* ----------------------------------------------------------------
 * Update and event handling functions
 */

bool
nest::siegert_neuron::update_( Time const& origin, const long from, const long to, const bool called_from_wfr_update )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  const double wfr_tol = kernel().simulation_manager.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store rates to be sent by rate events
  std::vector< double > new_rates( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    // register rate in buffer
    new_rates[ lag ] = S_.r_;

    // propagate rate to new time step (exponential integration)
    double drive = siegert( B_.drift_input_[ lag ], B_.diffusion_input_[ lag ] );
    S_.r_ = V_.P1_ * ( S_.r_ ) + V_.P2_ * ( P_.mean_ + drive );

    if ( not called_from_wfr_update )
    {
      // rate logging
      B_.logger_.record_data( origin.get_steps() + lag );
    }
    else // check convergence of waveform relaxation
    {
      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded or fabs( S_.r_ - B_.last_y_values[ lag ] ) > wfr_tol;
      // update last_y_values for next wfr_update iteration
      B_.last_y_values[ lag ] = S_.r_;
    }
  }

  if ( not called_from_wfr_update )
  {
    // clear last_y_values
    std::vector< double >( buffer_size, 0.0 ).swap( B_.last_y_values );

    // modifiy new_rates for diffusion-event as proxy for next min_delay
    for ( long temp = from; temp < to; ++temp )
    {
      new_rates[ temp ] = S_.r_;
    }
  }

  // Send diffusion-event
  DiffusionConnectionEvent rve;
  rve.set_coeffarray( new_rates );
  kernel().event_delivery_manager.send_secondary( *this, rve );

  // Reset variables
  std::vector< double >( buffer_size, 0.0 ).swap( B_.drift_input_ );
  std::vector< double >( buffer_size, 0.0 ).swap( B_.diffusion_input_ );

  return wfr_tol_exceeded;
}

void
nest::siegert_neuron::handle( DiffusionConnectionEvent& e )
{
  const double drift = e.get_drift_factor();
  const double diffusion = e.get_diffusion_factor();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    const double value = e.get_coeffvalue( it );
    B_.drift_input_[ i ] += drift * value;
    B_.diffusion_input_[ i ] += diffusion * value;
    ++i;
  }
}

void
nest::siegert_neuron::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif // HAVE_GSL
