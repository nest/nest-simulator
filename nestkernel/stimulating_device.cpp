/*
 *  stimulating_device.cpp
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

// Includes from libnestutil:
#include "compose.hpp"
#include "kernel_manager.h"
#include "stimulating_device.h"


template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::Parameters_::Parameters_()
  : label_()
  , time_in_steps_( false )
  , input_from_( names::internal )
{
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::Parameters_::Parameters_( const Parameters_& p )
  : label_( p.label_ )
  , time_in_steps_( p.time_in_steps_ )
  , input_from_( p.input_from_ )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
  ( *d )[ names::input_from ] = LiteralDatum( input_from_ );
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::Parameters_::set( const DictionaryDatum& d ) const
{
  updateValue< std::string >( d, names::label, label_ );

  bool time_in_steps = time_in_steps_;
  updateValue< bool >( d, names::time_in_steps, time_in_steps );

  if ( time_in_steps != time_in_steps_ )
  {
    throw BadProperty(
      "Property /time_in_steps cannot be set if recordings exist. "
      "Please clear the events first by setting /n_events to 0." );
  }
  time_in_steps_ = time_in_steps;
  std::string input_from;
  if ( updateValue< std::string >( d, names::input_from, input_from ) )
  {
    if ( not kernel().io_manager.is_valid_input_backend( input_from ) )
    {
      std::string msg = String::compose( "Unknown input backend '%1'", input_from );
      throw BadProperty( msg );
    }
    input_from_ = input_from;
  }
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::State_::State_()
  : n_events_( 0 )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::State_::get( DictionaryDatum& d ) const
{
  // if we already have the n_events entry, we add to it, otherwise we create it
  if ( d->known( names::n_events ) )
  {
    long n_events = getValue< long >( d, names::n_events );
    ( *d )[ names::n_events ] = n_events + n_events_;
  }
  else
  {
    ( *d )[ names::n_events ] = n_events_;
  }
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::State_::set( const DictionaryDatum& d ) const
{
  size_t n_events = n_events_;
  if ( updateValue< long >( d, names::n_events, n_events ) )
  {
    if ( n_events == 0 )
    {
      n_events_ = n_events;
    }
    else
    {
      throw BadProperty(
        "Property /n_events can only be set "
        "to 0 (which clears all stored events)." );
    }
  }
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::set_status( const DictionaryDatum& d ) const
{
  if ( kernel().simulation_manager.has_been_prepared() )
  {
    throw BadProperty( "Input parameters cannot be changed while inside a Prepare/Run/Cleanup context." );
  }

  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  State_ stmp = S_; // temporary copy in case of errors
  stmp.set( d );    // throws if BadProperty

  Device::set_status( d );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    DictionaryDatum backend_params = DictionaryDatum( new Dictionary );

    // copy all properties not previously accessed from d to backend_params
    for ( auto& kv_pair : *d )
    {
      if ( not kv_pair.second.accessed() )
      {
        ( *backend_params )[ kv_pair.first ] = kv_pair.second;
      }
    }

    kernel().io_manager.check_input_backend_device_status( ptmp.input_from_, backend_params );

    // cache all properties accessed by the backend in private member
    backend_params_->clear();
    for ( auto& kv_pair : *backend_params )
    {
      if ( kv_pair.second.accessed() )
      {
        ( *backend_params_ )[ kv_pair.first ] = kv_pair.second;
        d->lookup( kv_pair.first ).set_access_flag();
      }
    }
  }
  else
  {
    kernel().io_manager.enroll_input( ptmp.input_from_, *this, d );
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}


template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::stimulator );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    // first get the defaults from the backend
    kernel().io_manager.get_stimulating_backend_device_defaults( P_.input_from_, d );

    // then overwrite with cached parameters
    for ( auto& kv_pair : *backend_params_ )
    {
      ( *d )[ kv_pair.first ] = kv_pair.second;
    }
  }
  else
  {
    kernel().io_manager.get_stimulating_backend_device_status( P_.input_from_, *this, d );
  }
}