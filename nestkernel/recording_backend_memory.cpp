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

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_memory.h"

nest::RecordingBackendMemory::RecordingBackendMemory()
{
}

nest::RecordingBackendMemory::~RecordingBackendMemory() throw()
{
  delete_data_();
}

void
nest::RecordingBackendMemory::enroll( const RecordingDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  thread t = device.get_thread();
  index gid = device.get_gid();

  // If the device is not already enrolled, enroll it
  if ( data_[ t ].find( gid ) == data_[ t ].end() )
  {
    data_[ t ].insert( std::make_pair(
      gid, new Recordings( double_value_names, long_value_names ) ) );
  }

  bool time_in_steps = device.get_time_in_steps();
  data_[ t ].find( gid )->second->set_time_in_steps( time_in_steps );
}

void
nest::RecordingBackendMemory::pre_run_hook()
{
  delete_data_();
  data_map tmp( kernel().vp_manager.get_num_threads() );
  data_.swap( tmp );
}

void
nest::RecordingBackendMemory::delete_data_()
{
  for ( size_t t = 0; t < data_.size(); ++t )
  {
    data_map::value_type& inner = data_[ t ];
    data_map::value_type::iterator d;
    for ( d = inner.begin(); d != inner.end(); ++d )
    {
      delete d->second;
    }
  }
}


void
nest::RecordingBackendMemory::cleanup()
{
}

void
nest::RecordingBackendMemory::synchronize()
{
}

void
nest::RecordingBackendMemory::clear( const RecordingDevice& device )
{
  if ( data_.size() != 0 )
  {
    const thread t = device.get_thread();
    const index gid = device.get_gid();

    data_map::value_type::const_iterator device_data = data_[ t ].find( gid );
    if ( device_data != data_[ t ].end() )
    {
      device_data->second->clear();
    }
  }
}

void
nest::RecordingBackendMemory::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  thread t = device.get_thread();
  index gid = device.get_gid();

  if ( data_[ t ].find( gid ) == data_[ t ].end() )
  {
    return;
  }

  index sender = event.get_sender_gid();
  data_[ t ][ gid ]->push_back( sender, event, double_values, long_values );
}

void
nest::RecordingBackendMemory::get_device_status( const RecordingDevice& device,
  DictionaryDatum& d ) const
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  data_map::value_type::const_iterator device_data = data_[ t ].find( gid );
  if ( device_data != data_[ t ].end() )
  {
    device_data->second->get_status( d );
  }
}

void
nest::RecordingBackendMemory::set_device_status( const RecordingDevice& device,
  const DictionaryDatum& d )
{
  const int t = device.get_thread();
  const int gid = device.get_gid();

  if ( data_[ t ].find( gid ) != data_[ t ].end() )
  {
    bool time_in_steps = device.get_time_in_steps();
    data_[ t ].find( gid )->second->set_time_in_steps( time_in_steps );
  }
}

void
nest::RecordingBackendMemory::post_run_hook()
{
}
void
nest::RecordingBackendMemory::get_status(
  lockPTRDatum< Dictionary, &SLIInterpreter::Dictionarytype >& ) const
{
}
void
nest::RecordingBackendMemory::set_status(
  lockPTRDatum< Dictionary, &SLIInterpreter::Dictionarytype > const& )
{
}
void
nest::RecordingBackendMemory::prepare()
{
}
