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

// Includes from sli:
#include "dictutils.h"

nest::IOManager::IOManager()
  : overwrite_files_( false )
{
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
        msg = String::compose( "Errno %1 received when trying to open '%2'", errno, tmp );
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
      LOG( M_ERROR, "SetStatus", "Data prefix must not contain path elements." );
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
  updateValue< bool >( d, names::overwrite_files, overwrite_files_ );
}

void
nest::IOManager::get_status( DictionaryDatum& d )
{
  ( *d )[ names::data_path ] = data_path_;
  ( *d )[ names::data_prefix ] = data_prefix_;
  ( *d )[ names::overwrite_files ] = overwrite_files_;
}
