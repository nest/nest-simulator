/*
 *  recording_backend_ascii.cpp
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

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

// includes from sli:
#include "dictutils.h"

#include "recording_backend_ascii.h"

namespace nest
{

const unsigned int RecordingBackendASCII::ASCII_REC_BACKEND_VERSION = 2;

RecordingBackendASCII::RecordingBackendASCII()
{
}

RecordingBackendASCII::~RecordingBackendASCII() throw()
{
}

void
RecordingBackendASCII::initialize()
{
  data_map tmp( kernel().vp_manager.get_num_threads() );
  device_data_.swap( tmp );
}

void
RecordingBackendASCII::finalize()
{
  // nothing to do
}

void
RecordingBackendASCII::enroll( const RecordingDevice& device, const DictionaryDatum& params )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data == device_data_[ t ].end() )
  {
    std::string vp_node_id_string = compute_vp_node_id_string_( device );
    std::string modelname = device.get_name();
    auto p = device_data_[ t ].insert( std::make_pair( node_id, DeviceData( modelname, vp_node_id_string ) ) );
    device_data = p.first;
  }

  device_data->second.set_status( params );
}

void
RecordingBackendASCII::disenroll( const RecordingDevice& device )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data_[ t ].erase( device_data );
  }
}

void
RecordingBackendASCII::set_value_names( const RecordingDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  assert( device_data != device_data_[ t ].end() );
  device_data->second.set_value_names( double_value_names, long_value_names );
}

void
RecordingBackendASCII::pre_run_hook()
{
  // nothing to do
}

void
RecordingBackendASCII::post_run_hook()
{
  for ( auto& inner : device_data_ )
  {
    for ( auto& device_data : inner )
    {
      device_data.second.flush_file();
    }
  }
}

void
RecordingBackendASCII::post_step_hook()
{
  // nothing to do
}

void
RecordingBackendASCII::cleanup()
{
  for ( auto& inner : device_data_ )
  {
    for ( auto& device_data : inner )
    {
      device_data.second.close_file();
    }
  }
}

void
RecordingBackendASCII::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data == device_data_[ t ].end() )
  {
    return;
  }

  device_data->second.write( event, double_values, long_values );
}

const std::string
RecordingBackendASCII::compute_vp_node_id_string_( const RecordingDevice& device ) const
{
  const double num_vps = kernel().vp_manager.get_num_virtual_processes();
  const double num_nodes = kernel().node_manager.size();
  const int vp_digits = static_cast< int >( std::floor( std::log10( num_vps ) ) + 1 );
  const int node_id_digits = static_cast< int >( std::floor( std::log10( num_nodes ) ) + 1 );

  std::ostringstream vp_node_id_string;
  vp_node_id_string << "-" << std::setfill( '0' ) << std::setw( node_id_digits ) << device.get_node_id() << "-"
                    << std::setfill( '0' ) << std::setw( vp_digits ) << device.get_vp();

  return vp_node_id_string.str();
}

void
RecordingBackendASCII::prepare()
{
  for ( auto& inner : device_data_ )
  {
    for ( auto& device_info : inner )
    {
      device_info.second.open_file();
    }
  }
}

void
RecordingBackendASCII::set_status( const DictionaryDatum& )
{
  // nothing to do
}

void
RecordingBackendASCII::get_status( DictionaryDatum& ) const
{
  // nothing to do
}

void
RecordingBackendASCII::check_device_status( const DictionaryDatum& params ) const
{
  DeviceData dd( "", "" );
  dd.set_status( params ); // throws if params contains invalid entries
}

void
RecordingBackendASCII::get_device_defaults( DictionaryDatum& params ) const
{
  DeviceData dd( "", "" );
  dd.get_status( params );
}

void
RecordingBackendASCII::get_device_status( const RecordingDevice& device, DictionaryDatum& d ) const
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  data_map::value_type::const_iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data->second.get_status( d );
  }
}

/* ******************* Device meta data class DeviceData ******************* */

RecordingBackendASCII::DeviceData::DeviceData( std::string modelname, std::string vp_node_id_string )
  : precision_( 3 )
  , time_in_steps_( false )
  , modelname_( modelname )
  , vp_node_id_string_( vp_node_id_string )
  , file_extension_( "dat" )
  , label_( "" )
{
}

void
RecordingBackendASCII::DeviceData::set_value_names( const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  double_value_names_ = double_value_names;
  long_value_names_ = long_value_names;
}

void
RecordingBackendASCII::DeviceData::flush_file()
{
  file_.flush();
}

void
RecordingBackendASCII::DeviceData::open_file()
{
  std::string filename = compute_filename_();

  std::ifstream test( filename.c_str() );
  if ( test.good() and not kernel().io_manager.overwrite_files() )
  {
    std::string msg = String::compose(
      "The file '%1' already exists and overwriting files is disabled. To overwrite files, set "
      "the kernel property overwrite_files to true. To change the name or location of the file, "
      "change the kernel properties data_path or data_prefix, or the device property label.",
      filename );
    LOG( M_ERROR, "RecordingBackendASCII::enroll()", msg );
    throw IOError();
  }
  test.close();

  file_ = std::ofstream( filename.c_str() );

  if ( not file_.good() )
  {
    std::string msg = String::compose( "I/O error while opening file '%1'.", filename );
    LOG( M_ERROR, "RecordingBackendASCII::prepare()", msg );
    throw IOError();
  }

  file_ << "# NEST version: " << NEST_VERSION << std::endl
        << "# RecordingBackendASCII version: " << ASCII_REC_BACKEND_VERSION << std::endl;

  const std::string timehead = ( time_in_steps_ ) ? "\ttime_step\ttime_offset" : "\ttime_ms";
  file_ << std::fixed << std::setprecision( precision_ ) << "sender" << timehead;
  for ( auto& val : double_value_names_ )
  {
    file_ << "\t" << val;
  }
  for ( auto& val : long_value_names_ )
  {
    file_ << "\t" << val;
  }
  file_ << std::endl;
}

void
RecordingBackendASCII::DeviceData::close_file()
{
  file_.close();
}

void
RecordingBackendASCII::DeviceData::write( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  file_ << event.get_sender_node_id() << "\t";

  if ( time_in_steps_ )
  {
    file_ << event.get_stamp().get_steps() << "\t" << event.get_offset();
  }
  else
  {
    file_ << ( event.get_stamp().get_ms() - event.get_offset() );
  }

  for ( auto& val : double_values )
  {
    file_ << "\t" << val;
  }
  for ( auto& val : long_values )
  {
    file_ << "\t" << val;
  }

  file_ << "\n";
}

void
RecordingBackendASCII::DeviceData::get_status( DictionaryDatum& d ) const
{
  ( *d )[ names::file_extension ] = file_extension_;
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;

  std::string filename = compute_filename_();
  initialize_property_array( d, names::filenames );
  append_property( d, names::filenames, filename );
}

void
RecordingBackendASCII::DeviceData::set_status( const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::file_extension, file_extension_ );
  updateValue< long >( d, names::precision, precision_ );
  updateValue< std::string >( d, names::label, label_ );

  bool time_in_steps = false;
  if ( updateValue< bool >( d, names::time_in_steps, time_in_steps ) )
  {
    if ( kernel().simulation_manager.has_been_simulated() )
    {
      throw BadProperty( "Property time_in_steps cannot be set after Simulate has been called." );
    }

    time_in_steps_ = time_in_steps;
  }
}

std::string
RecordingBackendASCII::DeviceData::compute_filename_() const
{
  std::string data_path = kernel().io_manager.get_data_path();
  if ( not data_path.empty() and not( data_path[ data_path.size() - 1 ] == '/' ) )
  {
    data_path += '/';
  }

  std::string label = label_;
  if ( label.empty() )
  {
    label = modelname_;
  }

  std::string data_prefix = kernel().io_manager.get_data_prefix();

  return data_path + data_prefix + label + vp_node_id_string_ + "." + file_extension_;
}

} // namespace nest
