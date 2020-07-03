/*
 *  rate_transformer_node_impl.h
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

#ifndef RATE_TRANSFORMER_NODE_IMPL_H
#define RATE_TRANSFORMER_NODE_IMPL_H

#include "rate_transformer_node.h"

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

// Includes from libnestutil:
#include "numerics.h"
#include "dict_util.h"


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

template < class TNonlinearities >
RecordablesMap< rate_transformer_node< TNonlinearities > > rate_transformer_node< TNonlinearities >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Parameters_::Parameters_()
  : linear_summation_( true )
{
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::State_::State_()
  : rate_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::Parameters_::get( DictionaryDatum& d ) const
{
  def< bool >( d, names::linear_summation, linear_summation_ );
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< bool >( d, names::linear_summation, linear_summation_, node );
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ ); // Rate
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::State_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, rate_, node ); // Rate
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Buffers_::Buffers_( rate_transformer_node< TNonlinearities >& n )
  : logger_( n )
{
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Buffers_::Buffers_( const Buffers_&,
  rate_transformer_node< TNonlinearities >& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::rate_transformer_node()
  : Archiving_Node()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::rate_transformer_node( const rate_transformer_node& n )
  : Archiving_Node( n )
  , nonlinearities_( n.nonlinearities_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::init_state_( const Node& proto )
{
  const rate_transformer_node& pr = downcast< rate_transformer_node >( proto );
  S_ = pr.S_;
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::init_buffers_()
{
  B_.delayed_rates_.clear(); // includes resize

  // resize buffers
  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  B_.instant_rates_.resize( buffer_size, 0.0 );
  B_.last_y_values.resize( buffer_size, 0.0 );

  B_.logger_.reset(); // includes resize
  Archiving_Node::clear_history();
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate
}

/* ----------------------------------------------------------------
 * Update and event handling functions
 */

template < class TNonlinearities >
bool
nest::rate_transformer_node< TNonlinearities >::update_( Time const& origin,
  const long from,
  const long to,
  const bool called_from_wfr_update )
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
    // store rate
    new_rates[ lag ] = S_.rate_;
    // reinitialize output rate
    S_.rate_ = 0.0;

    double delayed_rates = 0;
    if ( called_from_wfr_update )
    {
      // use get_value_wfr_update to keep values in buffer
      delayed_rates = B_.delayed_rates_.get_value_wfr_update( lag );
    }
    else
    {
      // use get_value to clear values in buffer after reading
      delayed_rates = B_.delayed_rates_.get_value( lag );
    }

    if ( P_.linear_summation_ )
    {
      S_.rate_ += nonlinearities_.input( delayed_rates + B_.instant_rates_[ lag ] );
    }
    else
    {
      S_.rate_ += delayed_rates + B_.instant_rates_[ lag ];
    }

    if ( called_from_wfr_update )
    {
      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded or fabs( S_.rate_ - B_.last_y_values[ lag ] ) > wfr_tol;
      // update last_y_values for next wfr iteration
      B_.last_y_values[ lag ] = S_.rate_;
    }
    else
    {
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
  }

  // Send rate-neuron-event
  InstantaneousRateConnectionEvent rve;
  rve.set_coeffarray( new_rates );
  kernel().event_delivery_manager.send_secondary( *this, rve );

  // Reset variables
  std::vector< double >( buffer_size, 0.0 ).swap( B_.instant_rates_ );

  return wfr_tol_exceeded;
}


template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( InstantaneousRateConnectionEvent& e )
{
  const double weight = e.get_weight();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      B_.instant_rates_[ i ] += weight * e.get_coeffvalue( it );
    }
    else
    {
      B_.instant_rates_[ i ] += weight * nonlinearities_.input( e.get_coeffvalue( it ) );
    }
    ++i;
  }
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( DelayedRateConnectionEvent& e )
{
  const double weight = e.get_weight();
  const long delay = e.get_delay_steps();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      B_.delayed_rates_.add_value( delay + i, weight * e.get_coeffvalue( it ) );
    }
    else
    {
      B_.delayed_rates_.add_value( delay + i, weight * nonlinearities_.input( e.get_coeffvalue( it ) ) );
    }
    ++i;
  }
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif /* #ifndef RATE_TRANSFORMER_NODE_IMPL_H */
