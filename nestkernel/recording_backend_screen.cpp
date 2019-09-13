/*
 *  recording_backend_screen.cpp
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

// C++ includes:
#include <iostream>

// Includes from nestkernel:
#include "recording_device.h"

#include "recording_backend_screen.h"

void
nest::RecordingBackendScreen::initialize()
{
  device_data_map tmp( kernel().vp_manager.get_num_threads() );
  device_data_.swap( tmp );
}

void
nest::RecordingBackendScreen::finalize()
{
}

void
nest::RecordingBackendScreen::enroll( const RecordingDevice& device, const DictionaryDatum& params )
{
  const index gid = device.get_gid();
  const thread t = device.get_thread();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( gid );
  if ( device_data == device_data_[ t ].end() )
  {
    auto p = device_data_[ t ].insert( std::make_pair( gid, DeviceData() ) );
    device_data = p.first;
  }

  device_data->second.set_status( params );
}

void
nest::RecordingBackendScreen::disenroll( const RecordingDevice& device )
{
  const index gid = device.get_gid();
  const thread t = device.get_thread();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( gid );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data_[ t ].erase( device_data );
  }
}

void
nest::RecordingBackendScreen::set_value_names( const RecordingDevice&,
  const std::vector< Name >&,
  const std::vector< Name >& )
{
  // nothing to do
}

void
nest::RecordingBackendScreen::pre_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendScreen::cleanup()
{
  // nothing to do
}

void
nest::RecordingBackendScreen::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  if ( device_data_[ t ].find( gid ) == device_data_[ t ].end() )
  {
    return;
  }

  device_data_[ t ][ gid ].write( event, double_values, long_values );
}

void
nest::RecordingBackendScreen::check_device_status( const DictionaryDatum& params ) const
{
  DeviceData dd;
  dd.set_status( params ); // throws if params contains invalid entries
}

void
nest::RecordingBackendScreen::get_device_defaults( DictionaryDatum& params ) const
{
  DeviceData dd;
  dd.get_status( params );
}

void
nest::RecordingBackendScreen::get_device_status( const nest::RecordingDevice& device, DictionaryDatum& d ) const
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  device_data_map::value_type::const_iterator device_data = device_data_[ t ].find( gid );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data->second.get_status( d );
  }
}


void
nest::RecordingBackendScreen::prepare()
{
  // nothing to do
}

void
nest::RecordingBackendScreen::post_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendScreen::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendScreen::set_status( const DictionaryDatum& d )
{
  // nothing to do
}

void
nest::RecordingBackendScreen::get_status( DictionaryDatum& d ) const
{
  // nothing to do
}

/* ******************* Device meta data class DeviceInfo ******************* */

nest::RecordingBackendScreen::DeviceData::DeviceData()
  : precision_( 3 )
  , time_in_steps_( false )
{
}

void
nest::RecordingBackendScreen::DeviceData::get_status( DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
}

void
nest::RecordingBackendScreen::DeviceData::set_status( const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< bool >( d, names::time_in_steps, time_in_steps_ );
}

void
nest::RecordingBackendScreen::DeviceData::write( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
#pragma omp critical
  {
    prepare_cout_();

    std::cout << event.get_sender_gid() << "\t";

    if ( time_in_steps_ )
    {
      std::cout << event.get_stamp().get_steps() << "\t" << event.get_offset();
    }
    else
    {
      std::cout << event.get_stamp().get_ms() - event.get_offset();
    }

    for ( auto& val : double_values )
    {
      std::cout << "\t" << val;
    }
    for ( auto& val : long_values )
    {
      std::cout << "\t" << val;
    }
    std::cout << std::endl;

    restore_cout_();
  }
}

void
nest::RecordingBackendScreen::DeviceData::prepare_cout_()
{
  old_fmtflags_ = std::cout.flags( std::ios::fixed );
  old_precision_ = std::cout.precision( precision_ );
}

void
nest::RecordingBackendScreen::DeviceData::restore_cout_()
{
  std::cout.flags( old_fmtflags_ );
  std::cout.precision( old_precision_ );
}
