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
#ifdef HAVE_RECORDINGBACKEND_ARBOR
#include "recording_backend_arbor.h"
#endif
#ifdef HAVE_MPI
#include "recording_backend_mpi.h"
#include "stimulation_backend_mpi.h"
#endif
#ifdef HAVE_SIONLIB
#include "recording_backend_sionlib.h"
#endif

// Includes from sli:
#include "dictutils.h"
#include <string>

namespace nest
{

IOManager::IOManager()
  : overwrite_files_( false )
{
  register_recording_backends_();
  register_stimulation_backends_();
}

IOManager::~IOManager()
{
  for ( auto& it : recording_backends_ )
  {
    delete it.second;
  }
  for ( auto& it : stimulation_backends_ )
  {
    delete it.second;
  }
}

void
IOManager::set_data_path_prefix_( const DictionaryDatum& dict )
{
  std::string tmp;
  if ( updateValue< std::string >( dict, names::data_path, tmp ) )
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
        msg = String::compose( "Errno %1 received when trying to open '%2'", errno, tmp );
        break;
      }

      LOG( M_ERROR, "SetStatus", "Variable data_path not set: " + msg );
    }
  }

  if ( updateValue< std::string >( dict, names::data_prefix, tmp ) )
  {
    if ( tmp.find( '/' ) == std::string::npos )
    {
      data_prefix_ = tmp;
    }
    else
    {
      LOG( M_ERROR, "SetStatus", "Data prefix must not contain path elements." );
    }
  }
}

void
IOManager::initialize()
{
  DictionaryDatum dict( new Dictionary );
  // The properties data_path and data_prefix can be set via environment variables
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

  set_data_path_prefix_( dict );

  overwrite_files_ = false;

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
IOManager::finalize()
{
  for ( const auto& it : recording_backends_ )
  {
    it.second->finalize();
  }
  for ( const auto& it : stimulation_backends_ )
  {
    it.second->finalize();
  }
}

void IOManager::change_num_threads( thread )
{
  for ( const auto& it : recording_backends_ )
  {
    it.second->finalize();
    it.second->initialize();
  }
  for ( const auto& it : stimulation_backends_ )
  {
    it.second->finalize();
    it.second->initialize();
  }
}

void
IOManager::set_status( const DictionaryDatum& d )
{
  set_data_path_prefix_( d );

  updateValue< bool >( d, names::overwrite_files, overwrite_files_ );

  DictionaryDatum recording_backends;
  if ( updateValue< DictionaryDatum >( d, names::recording_backends, recording_backends ) )
  {
    for ( const auto& it : recording_backends_ )
    {
      DictionaryDatum recording_backend_status;
      if ( updateValue< DictionaryDatum >( recording_backends, it.first, recording_backend_status ) )
      {
        it.second->set_status( recording_backend_status );
      }
    }
  }
}

void
IOManager::get_status( DictionaryDatum& d )
{
  ( *d )[ names::data_path ] = data_path_;
  ( *d )[ names::data_prefix ] = data_prefix_;
  ( *d )[ names::overwrite_files ] = overwrite_files_;

  DictionaryDatum recording_backends( new Dictionary );
  for ( const auto& it : recording_backends_ )
  {
    DictionaryDatum recording_backend_status( new Dictionary );
    it.second->get_status( recording_backend_status );
    ( *recording_backends )[ it.first ] = recording_backend_status;
  }
  ( *d )[ names::recording_backends ] = recording_backends;
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
IOManager::is_valid_recording_backend( const Name& backend_name ) const
{
  std::map< Name, RecordingBackend* >::const_iterator backend;
  backend = recording_backends_.find( backend_name );
  return backend != recording_backends_.end();
}

bool
IOManager::is_valid_stimulation_backend( const Name& backend_name ) const
{
  auto backend = stimulation_backends_.find( backend_name );
  return backend != stimulation_backends_.end();
}

void
IOManager::write( const Name& backend_name,
  const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  recording_backends_[ backend_name ]->write( device, event, double_values, long_values );
}

void
IOManager::enroll_recorder( const Name& backend_name, const RecordingDevice& device, const DictionaryDatum& params )
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
nest::IOManager::enroll_stimulator( const Name& backend_name, StimulationDevice& device, const DictionaryDatum& params )
{

  if ( not is_valid_stimulation_backend( backend_name ) and not backend_name.toString().empty() )
  {
    return;
  }
  if ( backend_name.toString().empty() )
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
IOManager::set_recording_value_names( const Name& backend_name,
  const RecordingDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  recording_backends_[ backend_name ]->set_value_names( device, double_value_names, long_value_names );
}

void
IOManager::check_recording_backend_device_status( const Name& backend_name, const DictionaryDatum& params )
{
  recording_backends_[ backend_name ]->check_device_status( params );
}

void
IOManager::get_recording_backend_device_defaults( const Name& backend_name, DictionaryDatum& params )
{
  recording_backends_[ backend_name ]->get_device_defaults( params );
}

void
IOManager::get_recording_backend_device_status( const Name& backend_name,
  const RecordingDevice& device,
  DictionaryDatum& d )
{
  recording_backends_[ backend_name ]->get_device_status( device, d );
}

void
IOManager::register_recording_backends_()
{
  recording_backends_.insert( std::make_pair( "ascii", new RecordingBackendASCII() ) );
  recording_backends_.insert( std::make_pair( "memory", new RecordingBackendMemory() ) );
  recording_backends_.insert( std::make_pair( "screen", new RecordingBackendScreen() ) );
#ifdef HAVE_RECORDINGBACKEND_ARBOR
  recording_backends_.insert( std::make_pair( "arbor", new RecordingBackendArbor() ) );
#endif
#ifdef HAVE_MPI
  recording_backends_.insert( std::make_pair( "mpi", new RecordingBackendMPI() ) );
#endif
#ifdef HAVE_SIONLIB
  recording_backends_.insert( std::make_pair( "sionlib", new RecordingBackendSIONlib() ) );
#endif
}

void
IOManager::register_stimulation_backends_()
{
#ifdef HAVE_MPI
  stimulation_backends_.insert( std::make_pair( "mpi", new StimulationBackendMPI() ) );
#endif
}

} // namespace nest
