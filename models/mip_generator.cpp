/*
 *  mip_generator.cpp
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

#include "mip_generator.h"

// Includes from librandom:
#include "gslrandomgen.h"
#include "random_datums.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::mip_generator::Parameters_::Parameters_()
  : rate_( 0.0 ) // Hz
  , p_copy_( 1.0 )
  , mother_seed_( 0 )
{
  rng_ = librandom::RandomGen::create_knuthlfg_rng( mother_seed_ );
}

nest::mip_generator::Parameters_::Parameters_( const Parameters_& p )
  : rate_( p.rate_ )
  , p_copy_( p.p_copy_ )
  , mother_seed_( p.mother_seed_ )
{
  // deep copy of random number generator
  rng_ = p.rng_->clone( p.mother_seed_ );
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_;
  ( *d )[ names::p_copy ] = p_copy_;
  ( *d )[ names::mother_seed ] = mother_seed_;
}

void
nest::mip_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, rate_, node );
  updateValueParam< double >( d, names::p_copy, p_copy_, node );
  if ( rate_ < 0 )
  {
    throw BadProperty( "Rate must be non-negative." );
  }
  if ( p_copy_ < 0 || p_copy_ > 1 )
  {
    throw BadProperty( "Copy probability must be in [0, 1]." );
  }

  bool reset_rng = updateValue< librandom::RngPtr >( d, names::mother_rng, rng_ );

  // order important to avoid short-circuitung
  reset_rng = updateValue< long >( d, names::mother_seed, mother_seed_ ) || reset_rng;
  if ( reset_rng )
  {
    rng_->seed( mother_seed_ );
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::mip_generator::mip_generator()
  : DeviceNode()
  , device_()
  , P_()
{
}

nest::mip_generator::mip_generator( const mip_generator& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ ) // also causes deep copy of random nnumber generator
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::init_state_( const Node& proto )
{
  const mip_generator& pr = downcast< mip_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::mip_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::mip_generator::calibrate()
{
  device_.calibrate();

  // rate_ is in Hz, dt in ms, so we have to convert from s to ms
  V_.poisson_dev_.set_lambda( Time::get_resolution().get_ms() * P_.rate_ * 1e-3 );
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    if ( not device_.is_active( T ) || P_.rate_ <= 0 )
    {
      return; // no spikes to be generated
    }

    // generate spikes of mother process for each time slice
    long n_mother_spikes = V_.poisson_dev_.ldev( P_.rng_ );

    if ( n_mother_spikes )
    {
      DSSpikeEvent se;

      se.set_multiplicity( n_mother_spikes );
      kernel().event_delivery_manager.send( *this, se, lag );
    }
  }
}

void
nest::mip_generator::event_hook( DSSpikeEvent& e )
{
  // note: event_hook() receives a reference of the spike event that
  // was originally created in the update function. there we set
  // the multiplicty to store the number of mother spikes. the *same*
  // reference will be delivered multiple times to the event hook,
  // once for every receiver. when calling handle() of the receiver
  // above, we need to change the multiplicty to the number of copied
  // child process spikes, so afterwards it needs to be reset to correctly
  // store the number of mother spikes again during the next call of
  // event_hook().
  // reichert

  librandom::RngPtr rng = kernel().rng_manager.get_rng( get_thread() );
  unsigned long n_mother_spikes = e.get_multiplicity();
  unsigned long n_spikes = 0;

  for ( unsigned long n = 0; n < n_mother_spikes; n++ )
  {
    if ( rng->drand() < P_.p_copy_ )
    {
      n_spikes++;
    }
  }

  if ( n_spikes > 0 )
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }

  e.set_multiplicity( n_mother_spikes );
}
