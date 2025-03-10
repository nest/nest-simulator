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

namespace nest
{

void
RecordingBackendScreen::initialize()
{
  device_data_map tmp( kernel().vp_manager.get_num_threads() );
  device_data_.swap( tmp );
}

void
RecordingBackendScreen::finalize()
{
}

void
RecordingBackendScreen::enroll( const RecordingDevice& device, const DictionaryDatum& params )
{
  const size_t node_id = device.get_node_id();
  const size_t t = device.get_thread();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data == device_data_[ t ].end() )
  {
    auto p = device_data_[ t ].insert( std::make_pair( node_id, DeviceData() ) );
    device_data = p.first;
  }

  device_data->second.set_status( params );
}

void
RecordingBackendScreen::disenroll( const RecordingDevice& device )
{
  const size_t node_id = device.get_node_id();
  const size_t t = device.get_thread();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data_[ t ].erase( device_data );
  }
}

void
RecordingBackendScreen::set_value_names( const RecordingDevice&,
  const std::vector< Name >&,
  const std::vector< Name >& )
{
  // nothing to do
}

void
RecordingBackendScreen::pre_run_hook()
{
  // nothing to do
}

void
RecordingBackendScreen::cleanup()
{
  // nothing to do
}

void
RecordingBackendScreen::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  if ( device_data_[ t ].find( node_id ) == device_data_[ t ].end() )
  {
    return;
  }

  device_data_[ t ][ node_id ].write( event, double_values, long_values );
}

void
RecordingBackendScreen::check_device_status( const DictionaryDatum& params ) const
{
  DeviceData dd;
  dd.set_status( params ); // throws if params contains invalid entries
}

void
RecordingBackendScreen::get_device_defaults( DictionaryDatum& params ) const
{
  DeviceData dd;
  dd.get_status( params );
}

void
RecordingBackendScreen::get_device_status( const RecordingDevice& device, DictionaryDatum& d ) const
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  device_data_map::value_type::const_iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data->second.get_status( d );
  }
}


void
RecordingBackendScreen::prepare()
{
  // nothing to do
}

void
RecordingBackendScreen::post_run_hook()
{
  // nothing to do
}

void
RecordingBackendScreen::post_step_hook()
{
  // nothing to do
}

void
RecordingBackendScreen::set_status( const DictionaryDatum& )
{
  // nothing to do
}

void
RecordingBackendScreen::get_status( DictionaryDatum& ) const
{
  // nothing to do
}

/* ******************* Device meta data class DeviceInfo ******************* */

RecordingBackendScreen::DeviceData::DeviceData()
  : precision_( 3 )
  , time_in_steps_( false )
{
}

void
RecordingBackendScreen::DeviceData::get_status( DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
}

void
RecordingBackendScreen::DeviceData::set_status( const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< bool >( d, names::time_in_steps, time_in_steps_ );
}

void
RecordingBackendScreen::DeviceData::write( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
#pragma omp critical
  {
    prepare_cout_();

    std::cout << event.get_sender_node_id() << "\t";

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
RecordingBackendScreen::DeviceData::prepare_cout_()
{
  old_fmtflags_ = std::cout.flags( std::ios::fixed );
  old_precision_ = std::cout.precision( precision_ );
}

void
RecordingBackendScreen::DeviceData::restore_cout_()
{
  std::cout.flags( old_fmtflags_ );
  std::cout.precision( old_precision_ );
}

}