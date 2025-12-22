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
#include "io_manager_impl.h"
#include "kernel_manager.h"
#include "recording_backend_ascii.h"
#include "recording_backend_memory.h"
#include "recording_backend_screen.h"
#ifdef HAVE_MPI
#include "recording_backend_mpi.h"
#include "stimulation_backend_mpi.h"
#endif
#ifdef HAVE_SIONLIB
#include "recording_backend_sionlib.h"
#endif

#include <string>

namespace nest
{

IOManager::IOManager()
  : overwrite_files_( false )
{
}

IOManager::~IOManager()
{
}

void
IOManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    // Register backends again, since finalize cleans up
    // so backends from external modules are unloaded
    register_recording_backend< RecordingBackendASCII >( "ascii" );
    register_recording_backend< RecordingBackendMemory >( "memory" );
    register_recording_backend< RecordingBackendScreen >( "screen" );
#ifdef HAVE_MPI
    register_recording_backend< RecordingBackendMPI >( "mpi" );
    register_stimulation_backend< StimulationBackendMPI >( "mpi" );
#endif
#ifdef HAVE_SIONLIB
    register_recording_backend< RecordingBackendSIONlib >( "sionlib" );
#endif

    Dictionary dict;
    // The properties data_path and data_prefix can be set via environment variables
    char* data_path = std::getenv( "NEST_DATA_PATH" );
    if ( data_path )
    {
      dict[ names::data_path ] = std::string( data_path );
    }
    char* data_prefix = std::getenv( "NEST_DATA_PREFIX" );
    if ( data_prefix )
    {
      dict[ names::data_prefix ] = std::string( data_prefix );
    }

    set_data_path_prefix_( dict );

    overwrite_files_ = false;
  }

  for ( const auto& it : recording_backends_ )
  {
    it.second->initialize();
  }
  for ( const auto& it : stimulation_backends_ )
  {
    it.second->initialize();
  }
}

void
IOManager::finalize( const bool adjust_number_of_threads_or_rng_only )
{
  for ( const auto& it : recording_backends_ )
  {
    it.second->finalize();
  }
  for ( const auto& it : stimulation_backends_ )
  {
    it.second->finalize();
  }

  if ( not adjust_number_of_threads_or_rng_only )
  {
    for ( const auto& it : recording_backends_ )
    {
      delete it.second;
    }
    recording_backends_.clear();

    for ( const auto& it : stimulation_backends_ )
    {
      delete it.second;
    }
    stimulation_backends_.clear();
  }
}

void
IOManager::set_data_path_prefix_( const Dictionary& dict )
{
  std::string tmp;
  if ( dict.update_value( names::data_path, tmp ) )
  {
    DIR* testdir = opendir( tmp.c_str() );
    if ( testdir )
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
        msg = String::compose( "Errno %1 received when trying to open '%2'", errno, tmp );
        break;
      }

      LOG( VerbosityLevel::ERROR, "SetStatus", "Variable data_path not set: " + msg );
    }
  }

  if ( dict.update_value( names::data_prefix, tmp ) )
  {
    if ( tmp.find( '/' ) == std::string::npos )
    {
      data_prefix_ = tmp;
    }
    else
    {
      LOG( VerbosityLevel::ERROR, "SetStatus", "Data prefix must not contain path elements." );
    }
  }
}

void
IOManager::set_recording_backend_status( std::string recording_backend, const Dictionary& d )
{
  recording_backends_[ recording_backend ]->set_status( d );
}

void
IOManager::set_status( const Dictionary& d )
{
  set_data_path_prefix_( d );

  d.update_value( names::overwrite_files, overwrite_files_ );
}

Dictionary
IOManager::get_recording_backend_status( std::string recording_backend )
{
  Dictionary status;
  recording_backends_[ recording_backend ]->get_status( status );
  status[ names::element_type ] = std::string( "recording_backend" );
  return status;
}

void
IOManager::get_status( Dictionary& d )
{
  d[ names::data_path ] = data_path_;
  d[ names::data_prefix ] = data_prefix_;
  d[ names::overwrite_files ] = overwrite_files_;

  std::vector< std::string > recording_backends;
  for ( const auto& it : recording_backends_ )
  {
    recording_backends.push_back( it.first );
  }
  d[ names::recording_backends ] = recording_backends;

  std::vector< std::string > stimulation_backends;
  for ( const auto& it : stimulation_backends_ )
  {
    stimulation_backends.push_back( it.first );
  }
  d[ names::stimulation_backends ] = stimulation_backends;
}

void
IOManager::pre_run_hook()
{
  for ( auto& it : recording_backends_ )
  {
    it.second->pre_run_hook();
  }
  for ( auto& it : stimulation_backends_ )
  {
    it.second->pre_run_hook();
  }
}

void
IOManager::post_run_hook()
{
  for ( auto& it : recording_backends_ )
  {
    it.second->post_run_hook();
  }
  for ( auto& it : stimulation_backends_ )
  {
    it.second->post_run_hook();
  }
}

void
IOManager::post_step_hook()
{
  for ( auto& it : recording_backends_ )
  {
    it.second->post_step_hook();
  }
}

void
IOManager::prepare()
{
  for ( auto& it : recording_backends_ )
  {
    it.second->prepare();
  }
  for ( auto& it : stimulation_backends_ )
  {
    it.second->prepare();
  }
}

void
IOManager::cleanup()
{
  for ( auto& it : recording_backends_ )
  {
    it.second->cleanup();
  }
  for ( auto& it : stimulation_backends_ )
  {
    it.second->cleanup();
  }
}

bool
IOManager::is_valid_recording_backend( const std::string& backend_name ) const
{
  return recording_backends_.find( backend_name ) != recording_backends_.end();
}

bool
IOManager::is_valid_stimulation_backend( const std::string& backend_name ) const
{
  return stimulation_backends_.find( backend_name ) != stimulation_backends_.end();
}

void
IOManager::write( const std::string& backend_name,
  const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  recording_backends_[ backend_name ]->write( device, event, double_values, long_values );
}

void
IOManager::enroll_recorder( const std::string& backend_name, const RecordingDevice& device, const Dictionary& params )
{
  for ( auto& it : recording_backends_ )
  {
    if ( it.first == backend_name )
    {
      it.second->enroll( device, params );
    }
    else
    {
      it.second->disenroll( device );
    }
  }
}

void
IOManager::enroll_stimulator( const std::string& backend_name, StimulationDevice& device, const Dictionary& params )
{
  if ( not is_valid_stimulation_backend( backend_name ) and not backend_name.empty() )
  {
    return;
  }

  if ( backend_name.empty() )
  {
    for ( auto& it : stimulation_backends_ )
    {
      it.second->disenroll( device );
    }
  }
  else
  {
    for ( auto& it : stimulation_backends_ )
    {
      if ( it.first == backend_name )
      {
        ( it.second )->enroll( device, params );
      }
      else
      {
        it.second->disenroll( device );
      }
    }
  }
}

void
IOManager::set_recording_value_names( const std::string& backend_name,
  const RecordingDevice& device,
  const std::vector< std::string >& double_value_names,
  const std::vector< std::string >& long_value_names )
{
  recording_backends_[ backend_name ]->set_value_names( device, double_value_names, long_value_names );
}

void
IOManager::check_recording_backend_device_status( const std::string& backend_name, const Dictionary& params )
{
  recording_backends_[ backend_name ]->check_device_status( params );
}

void
IOManager::get_recording_backend_device_defaults( const std::string& backend_name, Dictionary& params )
{
  recording_backends_[ backend_name ]->get_device_defaults( params );
}

void
IOManager::get_recording_backend_device_status( const std::string& backend_name,
  const RecordingDevice& device,
  Dictionary& d )
{
  recording_backends_[ backend_name ]->get_device_status( device, d );
}

} // namespace nest
