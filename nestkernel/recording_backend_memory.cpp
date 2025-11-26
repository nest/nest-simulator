/*
 *  recording_backend_memory.cpp
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

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_memory.h"

nest::RecordingBackendMemory::RecordingBackendMemory()
{
}

nest::RecordingBackendMemory::~RecordingBackendMemory() throw()
{
}

void
nest::RecordingBackendMemory::initialize()
{
  device_data_map tmp( kernel().vp_manager.get_num_threads() );
  device_data_.swap( tmp );
}

void
nest::RecordingBackendMemory::finalize()
{
}

void
nest::RecordingBackendMemory::enroll( const RecordingDevice& device, const dictionary& params )
{
  size_t t = device.get_thread();
  size_t node_id = device.get_node_id();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data == device_data_[ t ].end() )
  {
    auto p = device_data_[ t ].insert( std::make_pair( node_id, DeviceData() ) );
    device_data = p.first;
  }

  device_data->second.set_status( params );
}

void
nest::RecordingBackendMemory::disenroll( const RecordingDevice& device )
{
  size_t t = device.get_thread();
  size_t node_id = device.get_node_id();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data_[ t ].erase( device_data );
  }
}

void
nest::RecordingBackendMemory::set_value_names( const RecordingDevice& device,
  const std::vector< std::string >& double_value_names,
  const std::vector< std::string >& long_value_names )
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  device_data_map::value_type::iterator device_data = device_data_[ t ].find( node_id );
  assert( device_data != device_data_[ t ].end() );
  device_data->second.set_value_names( double_value_names, long_value_names );
}

void
nest::RecordingBackendMemory::pre_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendMemory::cleanup()
{
  // nothing to do
}

void
nest::RecordingBackendMemory::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  size_t t = device.get_thread();
  size_t node_id = device.get_node_id();

  device_data_[ t ][ node_id ].push_back( event, double_values, long_values );
}

void
nest::RecordingBackendMemory::check_device_status( const dictionary& params ) const
{
  DeviceData dd;
  dd.set_status( params ); // throws if params contains invalid entries
}

void
nest::RecordingBackendMemory::get_device_defaults( dictionary& params ) const
{
  DeviceData dd;
  dd.get_status( params );
}

void
nest::RecordingBackendMemory::get_device_status( const RecordingDevice& device, dictionary& d ) const
{
  const size_t t = device.get_thread();
  const size_t node_id = device.get_node_id();

  const auto device_data = device_data_[ t ].find( node_id );
  if ( device_data != device_data_[ t ].end() )
  {
    device_data->second.get_status( d );
  }
}

void
nest::RecordingBackendMemory::post_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendMemory::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendMemory::get_status( dictionary& ) const
{
  // nothing to do
}

void
nest::RecordingBackendMemory::set_status( const dictionary& )
{
  // nothing to do
}

void
nest::RecordingBackendMemory::prepare()
{
  // nothing to do
}

/* ******************* Device meta data class DeviceInfo ******************* */

nest::RecordingBackendMemory::DeviceData::DeviceData()
  : time_in_steps_( false )
{
}

void
nest::RecordingBackendMemory::DeviceData::set_value_names( const std::vector< std::string >& double_value_names,
  const std::vector< std::string >& long_value_names )
{
  double_value_names_ = double_value_names;
  double_values_.resize( double_value_names.size() );

  long_value_names_ = long_value_names;
  long_values_.resize( long_value_names.size() );
}

void
nest::RecordingBackendMemory::DeviceData::push_back( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  senders_.push_back( event.get_sender_node_id() );

  if ( time_in_steps_ )
  {
    times_steps_.push_back( event.get_stamp().get_steps() );
    times_offset_.push_back( event.get_offset() );
  }
  else
  {
    times_ms_.push_back( event.get_stamp().get_ms() - event.get_offset() );
  }

  for ( size_t i = 0; i < double_values.size(); ++i )
  {
    double_values_[ i ].push_back( double_values[ i ] );
  }
  for ( size_t i = 0; i < long_values.size(); ++i )
  {
    long_values_[ i ].push_back( long_values[ i ] );
  }
}

void
nest::RecordingBackendMemory::DeviceData::get_status( dictionary& d ) const
{
  dictionary events;

  if ( d.known( names::events ) )
  {
    events = d.get< dictionary >( names::events );
  }

  auto init_intvector = [ &events ]( std::string key ) -> std::vector< int >&
  {
    if ( not events.known( key ) )
    {
      events[ key ] = std::vector< int >();
    }
    return boost::any_cast< std::vector< int >& >( events[ key ] );
  };
  auto init_doublevector = [ &events ]( std::string key ) -> std::vector< double >&
  {
    if ( not events.known( key ) )
    {
      events[ key ] = std::vector< double >();
    }
    return boost::any_cast< std::vector< double >& >( events[ key ] );
  };

  auto& senders = init_intvector( names::senders );
  senders.insert( senders.end(), senders_.begin(), senders_.end() );

  if ( time_in_steps_ )
  {
    auto& times = init_intvector( names::times );
    times.insert( times.end(), times_steps_.begin(), times_steps_.end() );

    auto& offsets = init_doublevector( names::offsets );
    offsets.insert( offsets.end(), times_offset_.begin(), times_offset_.end() );
  }
  else
  {
    auto& times = init_doublevector( names::times );
    times.insert( times.end(), times_ms_.begin(), times_ms_.end() );
  }

  for ( size_t i = 0; i < double_values_.size(); ++i )
  {
    auto& double_name = init_doublevector( double_value_names_[ i ] );
    double_name.insert( double_name.end(), double_values_[ i ].begin(), double_values_[ i ].end() );
  }
  for ( size_t i = 0; i < long_values_.size(); ++i )
  {
    auto& long_name = init_intvector( long_value_names_[ i ] );
    long_name.insert( long_name.end(), long_values_[ i ].begin(), long_values_[ i ].end() );
  }

  d[ names::events ] = events;
  d[ names::time_in_steps ] = time_in_steps_;
}

void
nest::RecordingBackendMemory::DeviceData::set_status( const dictionary& d )
{
  bool time_in_steps = false;
  if ( d.update_value( names::time_in_steps, time_in_steps ) )
  {
    if ( kernel().simulation_manager.has_been_simulated() )
    {
      throw BadProperty( "Property time_in_steps cannot be set after Simulate has been called." );
    }

    time_in_steps_ = time_in_steps;
  }

  long n_events = 1;
  if ( d.update_value( names::n_events, n_events ) and n_events == 0 )
  {
    clear();
  }
}

void
nest::RecordingBackendMemory::DeviceData::clear()
{
  senders_.clear();
  times_ms_.clear();
  times_steps_.clear();
  times_offset_.clear();

  for ( size_t i = 0; i < double_values_.size(); ++i )
  {
    double_values_[ i ].clear();
  }
  for ( size_t i = 0; i < long_values_.size(); ++i )
  {
    long_values_[ i ].clear();
  }
}
