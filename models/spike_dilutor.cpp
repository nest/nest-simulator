/*
 *  spike_dilutor.cpp
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

#include "spike_dilutor.h"

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

nest::spike_dilutor::Parameters_::Parameters_()
  : p_copy_( 1.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::spike_dilutor::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::p_copy ] = p_copy_;
}

void
nest::spike_dilutor::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::p_copy, p_copy_, node );
  if ( p_copy_ < 0 || p_copy_ > 1 )
  {
    throw BadProperty( "Copy probability must be in [0, 1]." );
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::spike_dilutor::spike_dilutor()
  : DeviceNode()
  , device_()
  , P_()
{
}

nest::spike_dilutor::spike_dilutor( const spike_dilutor& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::spike_dilutor::init_state_()
{
  device_.init_state();
}

void
nest::spike_dilutor::init_buffers_()
{
  B_.n_spikes_.clear(); // includes resize
  device_.init_buffers();
}

void
nest::spike_dilutor::calibrate()
{
  device_.calibrate();
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::spike_dilutor::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    if ( not device_.is_active( T ) )
    {
      return; // no spikes to be repeated
    }

    // generate spikes of mother process for each time slice
    unsigned long n_mother_spikes = static_cast< unsigned long >( B_.n_spikes_.get_value( lag ) );

    if ( n_mother_spikes )
    {
      DSSpikeEvent se;

      se.set_multiplicity( n_mother_spikes );
      kernel().event_delivery_manager.send( *this, se, lag );
    }
  }
}

void
nest::spike_dilutor::event_hook( DSSpikeEvent& e )
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

  unsigned long n_mother_spikes = e.get_multiplicity();
  unsigned long n_spikes = 0;

  for ( unsigned long n = 0; n < n_mother_spikes; n++ )
  {
    if ( get_vp_specific_rng( get_thread() )->drand() < P_.p_copy_ )
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

void
nest::spike_dilutor::handle( SpikeEvent& e )
{
  B_.n_spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    static_cast< double >( e.get_multiplicity() ) );
}
