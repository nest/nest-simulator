/*
 *  poisson_generator.cpp
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

#include "poisson_generator.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::poisson_generator::Parameters_::Parameters_()
  : rate_( 0.0 ) // pA
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::poisson_generator::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ );
}

void
nest::poisson_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, rate_, node );
  if ( rate_ < 0 )
  {
    throw BadProperty( "The rate cannot be negative." );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::poisson_generator::poisson_generator()
  : DeviceNode()
  , device_()
  , P_()
{
}

nest::poisson_generator::poisson_generator( const poisson_generator& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::poisson_generator::init_state_( const Node& proto )
{
  const poisson_generator& pr = downcast< poisson_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::poisson_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::poisson_generator::calibrate()
{
  device_.calibrate();

  // rate_ is in Hz, dt in ms, so we have to convert from s to ms
  V_.poisson_dev_.set_lambda( Time::get_resolution().get_ms() * P_.rate_ * 1e-3 );
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
nest::poisson_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  if ( P_.rate_ <= 0 )
  {
    return;
  }

  for ( long lag = from; lag < to; ++lag )
  {
    if ( not device_.is_active( T + Time::step( lag ) ) )
    {
      continue; // no spike at this lag
    }

    DSSpikeEvent se;
    kernel().event_delivery_manager.send( *this, se, lag );
  }
}

void
nest::poisson_generator::event_hook( DSSpikeEvent& e )
{
  // Be careful of storing the rng as its own variable. Creation of sharedPTRs
  // lead to overhead.
  long n_spikes = V_.poisson_dev_.ldev( kernel().rng_manager.get_rng( get_thread() ) );

  if ( n_spikes > 0 ) // we must not send events with multiplicity 0
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }
}
