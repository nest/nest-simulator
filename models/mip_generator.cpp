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


// Includes from libnestutil:
#include "dict_util.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::mip_generator::Parameters_::Parameters_()
  : rate_( 0.0 ) // Hz
  , p_copy_( 1.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_;
  ( *d )[ names::p_copy ] = p_copy_;
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

  if ( p_copy_ < 0 or p_copy_ > 1 )
  {
    throw BadProperty( "Copy probability must be in [0, 1]." );
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::mip_generator::mip_generator()
  : StimulationDevice()
  , P_()
{
}

nest::mip_generator::mip_generator( const mip_generator& n )
  : StimulationDevice( n )
  , P_( n.P_ ) // also causes deep copy of random nnumber generator
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::mip_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
}

void
nest::mip_generator::calibrate()
{
  StimulationDevice::calibrate();

  // rate_ is in Hz, dt in ms, so we have to convert from s to ms
  poisson_distribution::param_type param( Time::get_resolution().get_ms() * P_.rate_ * 1e-3 );
  V_.poisson_dist_.param( param );
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::mip_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 and static_cast< delay >( from ) < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    if ( not StimulationDevice::is_active( T ) || P_.rate_ <= 0 )
    {
      return; // no spikes to be generated
    }

    // generate spikes of parent process for each time slice
    const unsigned long n_parent_spikes = V_.poisson_dist_( get_vp_synced_rng( get_thread() ) );

    if ( n_parent_spikes )
    {
      DSSpikeEvent se;

      se.set_multiplicity( n_parent_spikes );
      kernel().event_delivery_manager.send( *this, se, lag );
    }
  }
}

void
nest::mip_generator::event_hook( DSSpikeEvent& e )
{
  /*
     We temporarily set the spike multiplicity here to the number of
     spikes selected by the copy process. After spike delivery, the
     multiplicity is reset to the number of parent spikes, so that this
     value is available for delivery to the next target.

     This is thread-safe because mip_generator is replicated on each thread.
   */

  RngPtr rng = get_vp_specific_rng( get_thread() );
  const unsigned long n_parent_spikes = e.get_multiplicity();

  // TODO: draw n_spikes from binomial distribution
  unsigned long n_spikes = 0;
  for ( unsigned long n = 0; n < n_parent_spikes; n++ )
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

  e.set_multiplicity( n_parent_spikes );
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */
void
nest::mip_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 2 )
    {
      throw BadParameterValue( "The size of the data for the mip_generator needs to be 2 [rate, p_copy]." );
    }
    else
    {
      DictionaryDatum d = DictionaryDatum( new Dictionary );
      ( *d )[ names::rate ] = DoubleDatum( input_param[ 0 ] );
      ( *d )[ names::p_copy ] = DoubleDatum( input_param[ 1 ] );
      ptmp.set( d, this );
    }
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
