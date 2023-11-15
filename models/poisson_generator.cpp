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
#include "nest_impl.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"


namespace nest
{

void
register_poisson_generator( const std::string& name )
{
  register_node_model< poisson_generator >( name );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

poisson_generator::Parameters_::Parameters_()
  : rate_( 0.0 ) // Hz
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
poisson_generator::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ );
}

void
poisson_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
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

poisson_generator::poisson_generator()
  : StimulationDevice()
  , P_()
{
}

poisson_generator::poisson_generator( const poisson_generator& n )
  : StimulationDevice( n )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
poisson_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
poisson_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
}

void
poisson_generator::pre_run_hook()
{
  StimulationDevice::pre_run_hook();

  // rate_ is in Hz, dt in ms, so we have to convert from s to ms
  poisson_distribution::param_type param( Time::get_resolution().get_ms() * P_.rate_ * 1e-3 );
  V_.poisson_dist_.param( param );
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
poisson_generator::update( Time const& T, const long from, const long to )
{
  if ( P_.rate_ <= 0 )
  {
    return;
  }

  for ( long lag = from; lag < to; ++lag )
  {
    if ( not StimulationDevice::is_active( T + Time::step( lag ) ) )
    {
      continue; // no spike at this lag
    }

    DSSpikeEvent se;
    kernel().event_delivery_manager.send( *this, se, lag );
  }
}

void
poisson_generator::event_hook( DSSpikeEvent& e )
{
  long n_spikes = V_.poisson_dist_( get_vp_specific_rng( get_thread() ) );

  if ( n_spikes > 0 ) // we must not send events with multiplicity 0
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
poisson_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 1 )
    {
      throw BadParameterValue( "The size of the data for the poisson generator needs to be 1 [rate]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::rate ] = DoubleDatum( input_param[ 0 ] );
    ptmp.set( d, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}

}