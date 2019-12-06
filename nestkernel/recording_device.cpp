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

#include "recording_device.h"

nest::RecordingDevice::RecordingDevice()
  : DeviceNode()
  , Device()
  , P_()
  , backend_params_( new Dictionary )
{
}

nest::RecordingDevice::RecordingDevice( const RecordingDevice& rd )
  : DeviceNode( rd )
  , Device( rd )
  , P_( rd.P_ )
  , backend_params_( new Dictionary( *rd.backend_params_ ) )
{
}

void
nest::RecordingDevice::set_initialized_()
{
  kernel().io_manager.enroll_recorder( P_.record_to_, *this, backend_params_ );
}

void
nest::RecordingDevice::calibrate( const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  Device::calibrate();
  kernel().io_manager.set_recording_value_names( P_.record_to_, *this, double_value_names, long_value_names );
}

const std::string&
nest::RecordingDevice::get_label() const
{
  return P_.label_;
}

nest::RecordingDevice::Parameters_::Parameters_()
  : label_()
  , record_to_( names::memory )
{
}

nest::RecordingDevice::Parameters_::Parameters_( const Parameters_& p )
  : label_( p.label_ )
  , record_to_( p.record_to_ )
{
}

void
nest::RecordingDevice::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::record_to ] = LiteralDatum( record_to_ );
}

void
nest::RecordingDevice::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );

  std::string record_to;
  if ( updateValue< std::string >( d, names::record_to, record_to ) )
  {
    if ( not kernel().io_manager.is_valid_recording_backend( record_to ) )
    {
      std::string msg = String::compose( "Unknown recording backend '%1'", record_to );
      throw BadProperty( msg );
    }

    record_to_ = record_to;
  }
}

nest::RecordingDevice::State_::State_()
  : n_events_( 0 )
{
}

void
nest::RecordingDevice::State_::get( DictionaryDatum& d ) const
{
  size_t n_events = 0;
  updateValue< long >( d, names::n_events, n_events );
  ( *d )[ names::n_events ] = n_events + n_events_;
}

void
nest::RecordingDevice::State_::set( const DictionaryDatum& d )
{
  size_t n_events = 0;
  if ( updateValue< long >( d, names::n_events, n_events ) )
  {
    if ( n_events != 0 )
    {
      throw BadProperty( "Property n_events can only be set to 0 (which clears all stored events)." );
    }

    n_events_ = 0;
  }
}

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  if ( kernel().simulation_manager.has_been_prepared() )
  {
    throw BadProperty( "Recorder parameters cannot be changed while inside a Prepare/Run/Cleanup context." );
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
    for ( auto kv_pair = d->begin(); kv_pair != d->end(); ++kv_pair )
    {
      if ( not kv_pair->second.accessed() )
      {
        ( *backend_params )[ kv_pair->first ] = kv_pair->second;
      }
    }

    kernel().io_manager.check_recording_backend_device_status( ptmp.record_to_, backend_params );

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
    kernel().io_manager.enroll_recorder( ptmp.record_to_, *this, d );
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

void
nest::RecordingDevice::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::recorder );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    // first get the defaults from the backend
    kernel().io_manager.get_recording_backend_device_defaults( P_.record_to_, d );

    // then overwrite with cached parameters
    for ( auto kv_pair = backend_params_->begin(); kv_pair != backend_params_->end(); ++kv_pair )
    {
      ( *d )[ kv_pair->first ] = kv_pair->second;
    }
  }
  else
  {
    kernel().io_manager.get_recording_backend_device_status( P_.record_to_, *this, d );
  }
}

bool
nest::RecordingDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return get_t_min_() < stamp && stamp <= get_t_max_();
}

void
nest::RecordingDevice::write( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  kernel().io_manager.write( P_.record_to_, *this, event, double_values, long_values );
  S_.n_events_++;
}
