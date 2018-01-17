/*
 *  rate_neuron_ipn_impl.h
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

#ifndef RATE_NEURON_IPN_IMPL_H
#define RATE_NEURON_IPN_IMPL_H

#include "rate_neuron_ipn.h"

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

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

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

template < class TGainfunction >
RecordablesMap< rate_neuron_ipn< TGainfunction > >
  rate_neuron_ipn< TGainfunction >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::Parameters_::Parameters_()
  : tau_( 10.0 ) // ms
  , std_( 1.0 )
  , mean_( 0.0 )
  , linear_summation_( true )
{
  recordablesMap_.create();
}

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::State_::State_()
  : rate_( 0.0 )
  , noise_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::Parameters_::get(
  DictionaryDatum& d ) const
{
  def< double >( d, names::tau, tau_ );
  def< double >( d, names::std, std_ );
  def< double >( d, names::mean, mean_ );
  def< bool >( d, names::linear_summation, linear_summation_ );
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::Parameters_::set(
  const DictionaryDatum& d )
{
  updateValue< double >( d, names::tau, tau_ );
  updateValue< double >( d, names::mean, mean_ );
  updateValue< double >( d, names::std, std_ );
  updateValue< bool >( d, names::linear_summation, linear_summation_ );


  if ( tau_ <= 0 )
  {
    throw BadProperty( "Time constant must be > 0." );
  }
  if ( std_ < 0 )
  {
    throw BadProperty( "Standard deviation of noise must not be negative." );
  }
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ );   // Rate
  def< double >( d, names::noise, noise_ ); // Noise
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::rate, rate_ ); // Rate
}

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::Buffers_::Buffers_(
  rate_neuron_ipn< TGainfunction >& n )
  : logger_( n )
{
}

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::Buffers_::Buffers_( const Buffers_&,
  rate_neuron_ipn< TGainfunction >& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::rate_neuron_ipn()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

template < class TGainfunction >
nest::rate_neuron_ipn< TGainfunction >::rate_neuron_ipn(
  const rate_neuron_ipn& n )
  : Archiving_Node( n )
  , gain_( n.gain_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::init_state_( const Node& proto )
{
  const rate_neuron_ipn& pr = downcast< rate_neuron_ipn >( proto );
  S_ = pr.S_;
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::init_buffers_()
{
  B_.delayed_rates_.clear(); // includes resize

  // resize buffers
  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  B_.instant_rates_.resize( buffer_size, 0.0 );
  B_.last_y_values.resize( buffer_size, 0.0 );
  B_.random_numbers.resize( buffer_size, numerics::nan );

  // initialize random numbers
  for ( unsigned int i = 0; i < buffer_size; i++ )
  {
    B_.random_numbers[ i ] =
      V_.normal_dev_( kernel().rng_manager.get_rng( get_thread() ) );
  }

  B_.logger_.reset(); // includes resize
  Archiving_Node::clear_history();
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::calibrate()
{
  B_.logger_
    .init(); // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms();

  // propagators
  V_.P1_ = std::exp( -h / P_.tau_ );
  V_.P2_ = -numerics::expm1( -h / P_.tau_ );
  V_.input_noise_factor_ =
    std::sqrt( -0.5 * numerics::expm1( -2. * h / P_.tau_ ) );
}

/* ----------------------------------------------------------------
 * Update and event handling functions
 */

template < class TGainfunction >
bool
nest::rate_neuron_ipn< TGainfunction >::update_( Time const& origin,
  const long from,
  const long to,
  const bool called_from_wfr_update )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  const double wfr_tol = kernel().simulation_manager.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store rates to be sent by rate events
  std::vector< double > new_rates( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    // store rate
    new_rates[ lag ] = S_.rate_;
    // get noise
    S_.noise_ = P_.std_ * B_.random_numbers[ lag ];
    // propagate rate to new time step (exponential integration)
    S_.rate_ = V_.P1_ * S_.rate_ + V_.P2_ * P_.mean_
      + V_.input_noise_factor_ * S_.noise_;

    if ( called_from_wfr_update ) // use get_value_wfr_update to keep values in
                                  // buffer
    {
      if ( P_.linear_summation_ )
      {
        S_.rate_ += V_.P2_ * gain_( B_.delayed_rates_.get_value_wfr_update(
                                      lag ) + B_.instant_rates_[ lag ] );
      }
      else
      {
        S_.rate_ += V_.P2_ * ( B_.delayed_rates_.get_value_wfr_update( lag )
                               + B_.instant_rates_[ lag ] );
      }

      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded
        or fabs( S_.rate_ - B_.last_y_values[ lag ] ) > wfr_tol;
      // update last_y_values for next wfr iteration
      B_.last_y_values[ lag ] = S_.rate_;
    }
    else // use get_value to clear values in buffer after reading
    {
      if ( P_.linear_summation_ )
      {
        S_.rate_ += V_.P2_ * gain_( B_.delayed_rates_.get_value( lag )
                               + B_.instant_rates_[ lag ] );
      }
      else
      {
        S_.rate_ += V_.P2_
          * ( B_.delayed_rates_.get_value( lag ) + B_.instant_rates_[ lag ] );
      }
      // rate logging
      B_.logger_.record_data( origin.get_steps() + lag );
    }
  }

  if ( not called_from_wfr_update )
  {
    // Send delay-rate-neuron-event. This only happens in the final iteration
    // to avoid accumulation in the buffers of the receiving neurons.
    DelayedRateConnectionEvent drve;
    drve.set_coeffarray( new_rates );
    kernel().event_delivery_manager.send_secondary( *this, drve );

    // clear last_y_values
    std::vector< double >( buffer_size, 0.0 ).swap( B_.last_y_values );

    // modifiy new_rates for rate-neuron-event as proxy for next min_delay
    for ( long temp = from; temp < to; ++temp )
    {
      new_rates[ temp ] = S_.rate_;
    }

    // create new random numbers
    B_.random_numbers.resize( buffer_size, numerics::nan );
    for ( unsigned int i = 0; i < buffer_size; i++ )
    {
      B_.random_numbers[ i ] =
        V_.normal_dev_( kernel().rng_manager.get_rng( get_thread() ) );
    }
  }

  // Send rate-neuron-event
  InstantaneousRateConnectionEvent rve;
  rve.set_coeffarray( new_rates );
  kernel().event_delivery_manager.send_secondary( *this, rve );

  // Reset variables
  std::vector< double >( buffer_size, 0.0 ).swap( B_.instant_rates_ );

  return wfr_tol_exceeded;
}


template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::handle(
  InstantaneousRateConnectionEvent& e )
{
  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      B_.instant_rates_[ i ] += e.get_weight() * e.get_coeffvalue( it );
    }
    else
    {
      B_.instant_rates_[ i ] +=
        e.get_weight() * gain_( e.get_coeffvalue( it ) );
    }
    i++;
  }
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::handle( DelayedRateConnectionEvent& e )
{
  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      B_.delayed_rates_.add_value(
        e.get_delay() - kernel().connection_manager.get_min_delay() + i,
        e.get_weight() * e.get_coeffvalue( it ) );
    }
    else
    {
      B_.delayed_rates_.add_value(
        e.get_delay() - kernel().connection_manager.get_min_delay() + i,
        e.get_weight() * gain_( e.get_coeffvalue( it ) ) );
    }
    ++i;
  }
}

template < class TGainfunction >
void
nest::rate_neuron_ipn< TGainfunction >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif /* #ifndef RATE_NEURON_IPN_IMPL_H */
