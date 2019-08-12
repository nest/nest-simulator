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
  , P_ ()
{
}

nest::RecordingDevice::RecordingDevice( const RecordingDevice& rd )
  : DeviceNode( rd )
  , Device( rd )
  , P_ ( rd.P_ )
{
}

void
nest::RecordingDevice::set_initialized_()
{
  kernel().io_manager.enroll_recorder( P_.record_to_, *this );
}

void
nest::RecordingDevice::calibrate(
  const std::vector< Name >& double_value_names, const std::vector< Name >& long_value_names )
{
  Device::calibrate();
  kernel().io_manager.set_recording_value_names( P_.record_to_, *this , double_value_names, long_value_names );
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
nest::RecordingDevice::Parameters_::get( const RecordingDevice&, DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::record_to ] = LiteralDatum( record_to_ );
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice&, const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );

  Name record_to;
  if ( updateValue< Name >( d, names::record_to, record_to ) )
  {
    if ( not kernel().io_manager.is_valid_recording_backend( record_to ) )
    {
      std::string msg = String::compose( "Unknown recording backend '%1'", record_to.toString() );
      throw BadProperty( msg );
    }

    record_to_ = record_to;
  }
}

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  //JME: make sure we're outside of Prepare/Run/Cleanup context

  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set( *this, d );   // throws if BadProperty

  Device::set_status( d );

  kernel().io_manager.set_recording_device_status( P_.record_to_, *this, d );

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;

  kernel().io_manager.enroll_recorder( P_.record_to_, *this );
}

void
nest::RecordingDevice::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::recorder );

  kernel().io_manager.get_recording_device_status( P_.record_to_, *this, d );
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
}
