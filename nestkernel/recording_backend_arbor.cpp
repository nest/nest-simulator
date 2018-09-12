/*
 *  recording_backend_sionlib.cpp
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

// C includes:
#include <mpi.h>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_arbor.h"

nest::RecordingBackendArbor::RecordingBackendArbor()
{
}

nest::RecordingBackendArbor::~RecordingBackendArbor() throw()
{
  finalize();
}

void
nest::RecordingBackendArbor::enroll( const RecordingDevice& device )
{
  std::vector< Name > value_names;
  nest::RecordingBackendArbor::enroll( device, value_names );
}

void
nest::RecordingBackendArbor::enroll( const RecordingDevice& device,
  const std::vector< Name >& value_names )
{
  const thread t = device.get_thread();
  const thread gid = device.get_gid();

  device_map::value_type::iterator device_it = devices_[ t ].find( gid );
  if ( device_it  != devices_[ t ].end() )
  {
    devices_[ t ].erase( device_it );
  }
  
  DeviceEntry entry( device );
  DeviceInfo& info = entry.info;

  info.gid = gid;
  info.type = static_cast< unsigned int >( device.get_type() );
  info.name = device.get_name();
  info.label = device.get_label();

  info.value_names.reserve( value_names.size() );
  std::vector< Name >::const_iterator it;
  for ( it = value_names.begin(); it != value_names.end(); ++it )
  {
    info.value_names.push_back( it->toString() );
  }

  devices_[ t ].insert( std::make_pair( gid, entry ) );
}

void
nest::RecordingBackendArbor::initialize()
{
  device_map devices( kernel().vp_manager.get_num_threads() );
  devices_.swap( devices );
}    

void
nest::RecordingBackendArbor::finalize()
{
}

void
nest::RecordingBackendArbor::synchronize()
{
  // Why?
  // if ( not files_opened_ or not P_.sion_collective_ )
  // {
  //   return;
  // }

  const thread t = kernel().vp_manager.get_thread_id();
  const thread task = kernel().vp_manager.thread_to_vp( t );

  //ArborBuffer& buffer = file.buffer;
  // write buffer
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event )
{
  const thread t = device.get_thread();
  const auto device_gid = device.get_gid();

  if ( devices_[ t ].find( device_gid ) == devices_[ t ].end() )
  {
    return;
  }

  //ArborBuffer& buffer = file.buffer;

  devices_[ t ].find( device_gid )->second.info.n_rec++;
  // const sion_uint32 n_values = 0;

  // // 2 * GID (device, source) + time in steps + offset (double) + number of
  // // values
  // const unsigned int required_space = 2 * sizeof( sion_uint64 )
  //   + sizeof( sion_int64 ) + sizeof( double ) + sizeof( sion_uint32 );

  // const sion_uint64 sender_gid = static_cast< sion_uint64 >( event.get_sender_gid() );
  // const sion_int64 step = static_cast< sion_int64 >( event.get_stamp().get_steps() );
  const double offset = event.get_offset();
  
  // if ( P_.sion_collective_ )
  // {
  //   buffer.ensure_space( required_space );
  //   buffer << device_gid << sender_gid << step << offset << n_values;
  //   return;
  // }

  // if ( buffer.get_capacity() > required_space )
  // {
  //   if ( buffer.get_free() < required_space )
  //   {
  //     sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
  //     buffer.clear();
  //   }

  //   buffer << device_gid << sender_gid << step << offset << n_values;
  // }
  // else
  // {
  //   if ( buffer.get_size() > 0 )
  //   {
  //     sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
  //     buffer.clear();
  //   }

  //   sion_fwrite( &device_gid, sizeof( sion_uint64 ), 1, file.sid );
  //   sion_fwrite( &sender_gid, sizeof( sion_uint64 ), 1, file.sid );
  //   sion_fwrite( &step, sizeof( sion_int64 ), 1, file.sid );
  //   sion_fwrite( &offset, sizeof( double ), 1, file.sid );
  //   sion_fwrite( &n_values, sizeof( sion_uint32 ), 1, file.sid );
  // }
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  const thread t = device.get_thread();
  const auto device_gid = device.get_gid();

  if ( devices_[ t ].find( device_gid ) == devices_[ t ].end() )
  {
    return;
  }
    
  //ArborBuffer& buffer = file.buffer;

  devices_[ t ].find( device_gid )->second.info.n_rec++;
  // const sion_uint32 n_values = static_cast< sion_uint32 >( values.size() );

  // // 2 * GID (device, source) + time in steps + offset (double) + number of
  // // values + one double per value
  // const unsigned int required_space = 2 * sizeof( sion_uint64 )
  //     + sizeof( sion_int64 ) + sizeof( double ) + sizeof( sion_uint32 )
  //     + n_values * sizeof( double );

  // const sion_uint64 sender_gid = static_cast< sion_uint64 >( event.get_sender_gid() );
  // const sion_int64 step = static_cast< sion_int64 >( event.get_stamp().get_steps() );
  const double offset = event.get_offset();
  
  // if ( P_.sion_collective_ )
  // {
  //   buffer.ensure_space( required_space );
  //   buffer << device_gid << sender_gid << step << offset << n_values;
  //   std::vector< double >::const_iterator val;
  //   for (val = values.begin(); val != values.end(); ++val )
  //   {
  //     buffer << *val;
  //   }
  //   return;
  // }

  // if ( buffer.get_capacity() > required_space )
  // {
  //   if ( buffer.get_free() < required_space )
  //   {
  //     sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
  //     buffer.clear();
  //   }

  //   buffer << device_gid << sender_gid << step << offset << n_values;
  //   std::vector< double >::const_iterator val;
  //   for ( val = values.begin(); val != values.end(); ++val )
  //   {
  //     buffer << *val;
  //   }
  // }
  // else
  // {
  //   if ( buffer.get_size() > 0 )
  //   {
  //     sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
  //     buffer.clear();
  //   }

  //   sion_fwrite( &device_gid, sizeof( sion_uint64 ), 1, file.sid );
  //   sion_fwrite( &sender_gid, sizeof( sion_uint64 ), 1, file.sid );
  //   sion_fwrite( &step, sizeof( sion_int64 ), 1, file.sid );
  //   sion_fwrite( &offset, sizeof( double ), 1, file.sid );
  //   sion_fwrite( &n_values, sizeof( sion_uint32 ), 1, file.sid );

  //   std::vector< double >::const_iterator val;
  //   for ( val = values.begin(); val != values.end(); ++val )
  //   {
  //     double value = *val;
  //     sion_fwrite( &value, sizeof( double ), 1, file.sid );
  //   }
  // }
}

/* ----------------------------------------------------------------
 * Buffer
 * ---------------------------------------------------------------- */

nest::RecordingBackendArbor::ArborBuffer::ArborBuffer()
{
}

nest::RecordingBackendArbor::ArborBuffer::~ArborBuffer()
{
}

void
nest::RecordingBackendArbor::ArborBuffer::write( /*something*/ )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendArbor::Parameters_::Parameters_()
{
}

void
nest::RecordingBackendArbor::Parameters_::get(
  const RecordingBackendArbor& al,
  DictionaryDatum& d ) const
{
}

void
nest::RecordingBackendArbor::Parameters_::set(
  const RecordingBackendArbor& al,
  const DictionaryDatum& d )
{
}

void
nest::RecordingBackendArbor::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
