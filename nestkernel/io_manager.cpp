/*
 *  io_manager.cpp
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

#include "io_manager.h"

// Generated includes:
#include "config.h"

// C includes:
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>

// C++ includes:
#include <cstdlib>

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "recording_backend_ascii.h"
#include "recording_backend_memory.h"
#include "recording_backend_screen.h"
#include "recording_backend_arbor.h"
#ifdef HAVE_SIONLIB
#include "recording_backend_sionlib.h"
#endif

// Includes from sli:
#include "dictutils.h"

nest::IOManager::IOManager()
  : overwrite_files_( false )
{
  recording_backends_.insert(std::make_pair( "ascii", new RecordingBackendASCII() ) );
  recording_backends_.insert(std::make_pair( "memory", new RecordingBackendMemory() ) );
  recording_backends_.insert(std::make_pair( "screen", new RecordingBackendScreen() ) );
  recording_backends_.insert(std::make_pair( "arbor", new RecordingBackendArbor() ) );
#ifdef HAVE_SIONLIB
  recording_backends_.insert(std::make_pair( "sionlib", new RecordingBackendSIONlib() ) );
#endif
}

nest::IOManager::~IOManager()
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( auto it = recording_backends_.begin(); it != recording_backends_.end();
        ++it )
  {
    delete it->second;
  }
}

void
nest::IOManager::set_data_path_prefix_( const DictionaryDatum& d )
{
  std::string tmp;
  if ( updateValue< std::string >( d, names::data_path, tmp ) )
  {
    DIR* testdir = opendir( tmp.c_str() );
    if ( testdir != NULL )
    {
      data_path_ = tmp;    // absolute path & directory exists
      closedir( testdir ); // we only opened it to check it exists
    }
    else
    {
      std::string msg;

      switch ( errno )
      {
      case ENOTDIR:
        msg = String::compose( "'%1' is not a directory.", tmp );
        break;
      case ENOENT:
        msg = String::compose( "Directory '%1' does not exist.", tmp );
        break;
      default:
        msg = String::compose(
          "Errno %1 received when trying to open '%2'", errno, tmp );
        break;
      }

      LOG( M_ERROR, "SetStatus", "Variable data_path not set: " + msg );
    }
  }

  if ( updateValue< std::string >( d, names::data_prefix, tmp ) )
  {
    if ( tmp.find( '/' ) == std::string::npos )
    {
      data_prefix_ = tmp;
    }
    else
    {
      LOG(
        M_ERROR, "SetStatus", "Data prefix must not contain path elements." );
    }
  }
}

void
nest::IOManager::initialize()
{
  // data_path and data_prefix can be set via environment variables
  DictionaryDatum dict( new Dictionary );
  char* data_path = std::getenv( "NEST_DATA_PATH" );
  if ( data_path )
  {
    ( *dict )[ names::data_path ] = std::string( data_path );
  }
  char* data_prefix = std::getenv( "NEST_DATA_PREFIX" );
  if ( data_prefix )
  {
    ( *dict )[ names::data_prefix ] = std::string( data_prefix );
  }
  if ( not dict->empty() )
  {
    set_data_path_prefix_( dict );
  }
  
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->initialize();
  }
}

void
nest::IOManager::finalize()
{
  data_path_ = "";
  data_prefix_ = "";
  overwrite_files_ = false;

  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->finalize();
  }
}

/*
     - set the data_path, data_prefix and overwrite_files properties
*/
void
nest::IOManager::set_status( const DictionaryDatum& d )
{
  set_data_path_prefix_( d );
  updateValue< bool >( d, names::overwrite_files, overwrite_files_ );

  DictionaryDatum recording_backends;
  if ( updateValue< DictionaryDatum >( d, names::recording_backends, recording_backends ) )
  {
    std::map< Name, RecordingBackend* >::const_iterator it;
    for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
    {
      DictionaryDatum recording_backend_status;
      if ( updateValue< DictionaryDatum >(recording_backends, it->first, recording_backend_status ) )
      {
	it->second->set_status( recording_backend_status );
      }
    }
  }
}

void
nest::IOManager::get_status( DictionaryDatum& d )
{
  ( *d )[ names::data_path ] = data_path_;
  ( *d )[ names::data_prefix ] = data_prefix_;
  ( *d )[ names::overwrite_files ] = overwrite_files_;

  DictionaryDatum recording_backends( new Dictionary );
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    DictionaryDatum recording_backend_status( new Dictionary );
    it->second->get_status( recording_backend_status );
    ( *recording_backends )[ it->first ] = recording_backend_status;
  }
  ( *d )[ names::recording_backends ] = recording_backends;
}

void
nest::IOManager::post_run_cleanup()
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->post_run_cleanup();
  }
}

void
nest::IOManager::cleanup()
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->finalize();
  }
}

void
nest::IOManager::synchronize()
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->synchronize();
  }
}

bool
nest::IOManager::is_valid_recording_backend( Name backend_name )
{
  std::map< Name, RecordingBackend* >::const_iterator backend;
  backend = recording_backends_.find( backend_name );
  return backend != recording_backends_.end();
}

nest::RecordingBackend*
nest::IOManager::get_recording_backend_( Name backend_name )
{
  std::map< Name, RecordingBackend* >::const_iterator backend;
  backend = recording_backends_.find( backend_name );
  assert( backend != recording_backends_.end() );
  return ( backend->second );
}

void
nest::IOManager::clear_recording_backends( const RecordingDevice& device )
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->clear( device );
  }
}

void
nest::IOManager::write( const RecordingDevice& device,
			const Event& event )
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->write( device, event );
  }
}

void
nest::IOManager::write( const RecordingDevice& device,
			const Event& event,
			const std::vector< double >& data )
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->write( device, event, data );
  }
}

void
nest::IOManager::enroll_recorder( Name backend_name,
				  const RecordingDevice& device )
{
  get_recording_backend_( backend_name )->enroll( device );
}

void
nest::IOManager::enroll_recorder( Name backend_name,
				  const RecordingDevice& device,
				  const std::vector< Name >& value_names )
{
  get_recording_backend_( backend_name )->enroll( device, value_names );
}

void
nest::IOManager::get_recording_device_status( const RecordingDevice& device,
					      DictionaryDatum& d )
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->get_device_status( device, d );
  }
}

void
nest::IOManager::set_recording_device_status( const RecordingDevice& device,
					      const DictionaryDatum& d )
{
  std::map< Name, RecordingBackend* >::const_iterator it;
  for ( it = recording_backends_.begin(); it != recording_backends_.end(); ++it )
  {
    it->second->set_device_status( device, d );
  }
}
