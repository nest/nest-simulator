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
#include "recording_backend_screen.h"
#include "recording_backend_ascii.h"
#ifdef HAVE_SIONLIB
  #include "recording_backend_sionlib.h"
#endif

// Includes from sli:
#include "dictutils.h"

nest::IOManager::IOManager()
  : overwrite_files_( false )
  , backend_( NULL )
{
  set_backend( names::RecordingBackendScreen );
}

void
nest::IOManager::set_data_path_prefix_( const DictionaryDatum& d )
{
  std::string tmp;
  if ( updateValue< std::string >( d, "data_path", tmp ) )
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

  if ( updateValue< std::string >( d, "data_prefix", tmp ) )
  {
    if ( tmp.find( '/' ) == std::string::npos )
      data_prefix_ = tmp;
    else
      LOG(
        M_ERROR, "SetStatus", "Data prefix must not contain path elements." );
  }
}

void
nest::IOManager::initialize()
{
  // data_path and data_prefix can be set via environment variables
  DictionaryDatum dict( new Dictionary );
  char* data_path = std::getenv( "NEST_DATA_PATH" );
  if ( data_path )
    ( *dict )[ "data_path" ] = std::string( data_path );
  char* data_prefix = std::getenv( "NEST_DATA_PREFIX" );
  if ( data_prefix )
    ( *dict )[ "data_prefix" ] = std::string( data_prefix );
  if ( !dict->empty() )
    set_data_path_prefix_( dict );
}

void
nest::IOManager::finalize()
{
  data_path_ = "";
  data_prefix_ = "";
  overwrite_files_ = false;
}

/*
     - set the data_path, data_prefix and overwrite_files properties
*/
void
nest::IOManager::set_status( const DictionaryDatum& d )
{
  set_data_path_prefix_( d );
  updateValue< bool >( d, "overwrite_files", overwrite_files_ );

  // Setup recording backend and its options
  DictionaryDatum dd;
  if ( updateValue< DictionaryDatum >( d, "recording", dd ) )
  {
    std::string recording_backend;
    if ( updateValue< std::string >( dd, names::recording_backend, recording_backend ) )
      set_backend( recording_backend );

    backend_->set_status( dd );
  }
}

void
nest::IOManager::get_status( DictionaryDatum& d )
{
  ( *d )[ "data_path" ] = data_path_;
  ( *d )[ "data_prefix" ] = data_prefix_;
  ( *d )[ "overwrite_files" ] = overwrite_files_;
}

bool
nest::IOManager::set_backend( Name name )
{
  if ( name == names::RecordingBackendScreen )
  {
    if ( backend_ != 0 )
    {
      delete backend_;
    }
    backend_ = new RecordingBackendScreen();
  }
  else if ( name == names::RecordingBackendASCII )
  {
    if ( backend_ != 0 )
    {
      delete backend_;
    }
    backend_ = new RecordingBackendASCII();
  }
#ifdef HAVE_SIONLIB
  else if ( name == names::RecordingBackendSIONlib )
  {
    if ( backend_ != 0 )
    {
      delete backend_;
    }
    backend_ = new RecordingBackendSIONlib();
  }
#endif // HAVE_SIONLIB
  else
  {
    std::string msg = String::compose( "Recording backend is not known: '%1'", name );
    LOG( M_WARNING, "IOManager::set_status", msg.c_str() );
    return false;
  }
  return true;
}
