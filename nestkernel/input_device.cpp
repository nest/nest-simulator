/*
 *  recording_device.cpp
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
#include "input_device.h"

nest::InputDevice::InputDevice()
  : DeviceNode()
  , Device()
  , P_()
  , backend_params_( new Dictionary )
{
}

nest::InputDevice::InputDevice( const InputDevice& id )
  : DeviceNode( id )
  , Device( id )
  , P_( id.P_ )
  , backend_params_( new Dictionary( *id.backend_params_ ) )
{
}

void
nest::InputDevice::set_initialized_() {
  kernel().io_manager.enroll_input(P_.input_from_, *this, backend_params_);
}

void
nest::InputDevice::calibrate( const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  Device::calibrate();
  kernel().io_manager.set_input_value_names( P_.input_from_, *this, double_value_names, long_value_names );
}

const std::string&
nest::InputDevice::get_label() const
{
  return P_.label_;
}

nest::InputDevice::Parameters_::Parameters_()
  : label_()
  , time_in_steps_( false )
  , input_from_(names::internal)
{

}

nest::InputDevice::Parameters_::Parameters_( const Parameters_& p )
        : label_( p.label_ )
        , time_in_steps_(p.time_in_steps_)
        , input_from_( p.input_from_ )
{
}

void
nest::InputDevice::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
  ( *d )[ names::input_from ] = LiteralDatum(input_from_);
}

void
nest::InputDevice::Parameters_::set(
					 const DictionaryDatum& d)
{
  updateValue< std::string >( d, names::label, label_ );

  bool time_in_steps = time_in_steps_;
  updateValue< bool >( d, names::time_in_steps, time_in_steps );

//  if ( time_in_steps != time_in_steps_ and n_events != 0 ) I don't understand n_events
// probabily link with S_.n_events_
  if ( time_in_steps != time_in_steps_)
  {
    throw BadProperty("Property /time_in_steps cannot be set if recordings exist. "
		      "Please clear the events first by setting /n_events to 0.");
  }
  time_in_steps_ = time_in_steps;
  std::string input_from;
  if ( updateValue< std::string >( d, names::input_from, input_from ) ) {
    if (not kernel().io_manager.is_valid_input_backend(input_from)) {
      std::string msg = String::compose("Unknown input backend '%1'", input_from);
      throw BadProperty(msg);
    }
    input_from_ = input_from;
  }
}

nest::InputDevice::State_::State_()
  : n_events_( 0 )
{
}

void
nest::InputDevice::State_::get( DictionaryDatum& d ) const
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

void
nest::InputDevice::State_::set( const DictionaryDatum& d)
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
      throw BadProperty( "Property /n_events can only be set "
        "to 0 (which clears all stored events)." );
    }
  }
}

void
nest::InputDevice::set_status( const DictionaryDatum& d )
{
  if ( kernel().simulation_manager.has_been_prepared() )
  {
    throw BadProperty( "Input parameters cannot be changed while inside a Prepare/Run/Cleanup context." );
  }

  Parameters_ ptmp = P_;    // temporary copy in case of errors
  ptmp.set( d); // throws if BadProperty

  State_ stmp = S_;         // temporary copy in case of errors
  stmp.set( d );     // throws if BadProperty

  Device::set_status( d );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    DictionaryDatum backend_params = DictionaryDatum( new Dictionary );

    // copy all properties not previously accessed from d to backend_params
    for ( auto kv_pair = d->begin(); kv_pair != d->end(); ++kv_pair )
    {
      if ( not kv_pair->second.accessed() )
      {
        ( *backend_params )[ kv_pair->first ] = kv_pair->second;
      }
    }

    kernel().io_manager.check_input_backend_device_status( ptmp.input_from_, backend_params );

    // cache all properties accessed by the backend in private member
    backend_params_->clear();
    for ( auto kv_pair = backend_params->begin(); kv_pair != backend_params->end(); ++kv_pair )
    {
      if ( kv_pair->second.accessed() )
      {
        ( *backend_params_ )[ kv_pair->first ] = kv_pair->second;
        d->lookup( kv_pair->first ).set_access_flag();
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


void
nest::InputDevice::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::stimulator );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    // first get the defaults from the backend
    kernel().io_manager.get_input_backend_device_defaults( P_.input_from_, d );

    // then overwrite with cached parameters
    for ( auto kv_pair = backend_params_->begin(); kv_pair != backend_params_->end(); ++kv_pair )
    {
      ( *d )[ kv_pair->first ] = kv_pair->second;
    }
  }
  else
  {
    kernel().io_manager.get_input_backend_device_status( P_.input_from_, *this, d );
  }
}

bool
nest::InputDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return get_t_min_() < stamp && stamp <= get_t_max_();
}
