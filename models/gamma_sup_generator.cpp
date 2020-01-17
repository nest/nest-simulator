/*
 *  gamma_sup_generator.cpp
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

#include "gamma_sup_generator.h"

// C++ includes:
#include <algorithm>
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from sli:
#include "datum.h"
#include "dict.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Constructor of internal states class
 * ---------------------------------------------------------------- */

nest::gamma_sup_generator::Internal_states_::Internal_states_( size_t num_bins,
  unsigned long ini_occ_ref,
  unsigned long ini_occ_act )
{
  occ_.resize( num_bins, ini_occ_ref );
  occ_.back() += ini_occ_act;
}

/* ----------------------------------------------------------------
 * Propagate internal states one time step and generate spikes
 * ---------------------------------------------------------------- */

unsigned long
nest::gamma_sup_generator::Internal_states_::update( double transition_prob, librandom::RngPtr rng )
{
  std::vector< unsigned long > n_trans; // only set from poisson_dev_ og bino_dev_ or 0, thus >= 0
  n_trans.resize( occ_.size() );

  // go through all states and draw number of transitioning components
  for ( unsigned long i = 0; i < occ_.size(); i++ )
  {
    if ( occ_[ i ] > 0 )
    {
      /*The binomial distribution converges towards the Poisson distribution as
       the number of trials goes to infinity while the product np remains fixed.
       Therefore the Poisson distribution with parameter \lambda = np can be
       used as an approximation to B(n, p) of the binomial distribution if n is
       sufficiently large and p is sufficiently small. According to two rules
       of thumb, this approximation is good if n >= 20 and p <= 0.05, or if
       n >= 100 and np <= 10. Source:
       http://en.wikipedia.org/wiki/Binomial_distribution#Poisson_approximation
       */
      if ( ( occ_[ i ] >= 100 && transition_prob <= 0.01 )
        || ( occ_[ i ] >= 500 && transition_prob * occ_[ i ] <= 0.1 ) )
      {
        poisson_dev_.set_lambda( transition_prob * occ_[ i ] );
        n_trans[ i ] = poisson_dev_.ldev( rng );
        if ( n_trans[ i ] > occ_[ i ] )
        {
          n_trans[ i ] = occ_[ i ];
        }
      }
      else
      {
        bino_dev_.set_p_n( transition_prob, occ_[ i ] );
        n_trans[ i ] = bino_dev_.ldev( rng );
      }
    }
    else
    {
      n_trans[ i ] = 0;
    }
  }

  // according to above numbers, change the occupation vector
  for ( unsigned long i = 0; i < occ_.size(); i++ )
  {
    if ( n_trans[ i ] > 0 )
    {
      occ_[ i ] -= n_trans[ i ];
      if ( i == occ_.size() - 1 )
      {
        occ_.front() += n_trans[ i ];
      }
      else
      {
        occ_[ i + 1 ] += n_trans[ i ];
      }
    }
  }
  return n_trans.back();
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::gamma_sup_generator::Parameters_::Parameters_()
  : rate_( 0.0 ) // Hz
  , gamma_shape_( 1 )
  , n_proc_( 1 )
  , num_targets_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gamma_sup_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_;
  ( *d )[ names::gamma_shape ] = gamma_shape_;
  ( *d )[ names::n_proc ] = n_proc_;
}

void
nest::gamma_sup_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< long >( d, names::gamma_shape, gamma_shape_, node );
  if ( gamma_shape_ < 1 )
  {
    throw BadProperty( "The shape must be larger or equal 1" );
  }

  updateValueParam< double >( d, names::rate, rate_, node );
  if ( rate_ < 0.0 )
  {
    throw BadProperty( "The rate must be larger than 0." );
  }

  long n_proc_l = n_proc_;
  updateValueParam< long >( d, names::n_proc, n_proc_l, node );
  if ( n_proc_l < 1 )
  {
    throw BadProperty( "The number of component processes cannot be smaller than one" );
  }
  else
  {
    n_proc_ = static_cast< unsigned long >( n_proc_l );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::gamma_sup_generator::gamma_sup_generator()
  : DeviceNode()
  , device_()
  , P_()
{
}

nest::gamma_sup_generator::gamma_sup_generator( const gamma_sup_generator& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::gamma_sup_generator::init_state_( const Node& proto )
{
  const gamma_sup_generator& pr = downcast< gamma_sup_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::gamma_sup_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::gamma_sup_generator::calibrate()
{
  device_.calibrate();

  double h = Time::get_resolution().get_ms();

  // transition probability in each time step
  V_.transition_prob_ = P_.rate_ * P_.gamma_shape_ * h / 1000.0;

  // approximate equilibrium occupation to initialize to
  unsigned long ini_occ_0 = static_cast< unsigned long >( P_.n_proc_ / P_.gamma_shape_ );

  // If new targets have been added during a simulation break, we
  // initialize the new elements in Internal_states with the initial dist. The
  // existing elements are unchanged.
  Internal_states_ internal_states0( P_.gamma_shape_, ini_occ_0, P_.n_proc_ - ini_occ_0 * P_.gamma_shape_ );
  B_.internal_states_.resize( P_.num_targets_, internal_states0 );
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
nest::gamma_sup_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  if ( P_.rate_ <= 0 || P_.num_targets_ == 0 )
  {
    return;
  }

  for ( long lag = from; lag < to; ++lag )
  {
    Time t = T + Time::step( lag );

    if ( not device_.is_active( t ) )
    {
      continue; // no spike at this lag
    }

    DSSpikeEvent se;
    kernel().event_delivery_manager.send( *this, se, lag );
  }
}


void
nest::gamma_sup_generator::event_hook( DSSpikeEvent& e )
{

  // get port number
  const port prt = e.get_port();

  // we handle only one port here, get reference to vector elem
  assert( 0 <= prt && static_cast< size_t >( prt ) < B_.internal_states_.size() );

  // age_distribution object propagates one time step and returns number of
  // spikes
  unsigned long n_spikes =
    B_.internal_states_[ prt ].update( V_.transition_prob_, kernel().rng_manager.get_rng( get_thread() ) );

  if ( n_spikes > 0 ) // we must not send events with multiplicity 0
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }
}
