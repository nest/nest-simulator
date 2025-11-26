/*
 *  module_manager.cpp
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

#include "module_manager.h"

#ifdef HAVE_LIBLTDL

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_extension_interface.h"

// Includes from sli:
#include "arraydatum.h"

// Includes from thirdparty:
#include "compose.hpp"

#include <node_manager.h>


namespace nest
{

ModuleManager::ModuleManager()
  : modules_()
{
  lt_dlinit();

  // Add NEST lib install dir to ltdl search path
  // To avoid problems due to string substitution in NEST binaries during
  // Conda installation, we need to convert the literal to string, cstr and back,
  // see #2237 and https://github.com/conda/conda-build/issues/1674#issuecomment-280378336
  const std::string module_dir = std::string( NEST_INSTALL_PREFIX ).c_str() + std::string( "/" NEST_INSTALL_LIBDIR );
  if ( lt_dladdsearchdir( module_dir.c_str() ) )
  {
    LOG( M_ERROR,
      "ModuleManager::ModuleManager",
      String::compose( "Could not add dynamic module search directory '%1'.", module_dir ) );
  }
}

ModuleManager::~ModuleManager()
{
  finalize( /* adjust_number_of_threads_or_rng_only */ false ); // closes dynamically loaded modules
  lt_dlexit();
}

void
ModuleManager::initialize( const bool )
{
}

void
ModuleManager::finalize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( adjust_number_of_threads_or_rng_only )
  {
    return;
  }

  // unload all loaded modules
  for ( const auto& [ name, module_info ] : modules_ )
  {
    lt_dlclose( module_info.handle );
  }
  modules_.clear();
}

void
ModuleManager::reinitialize_dynamic_modules()
{
  for ( const auto& [ name, module_info ] : modules_ )
  {
    module_info.extension->initialize();
  }
}

void
ModuleManager::get_status( DictionaryDatum& d )
{
  ArrayDatum loaded;
  for ( const auto& [ name, module_info ] : modules_ )
  {
    loaded.push_back( new LiteralDatum( name ) );
  }
  ( *d )[ names::modules ] = loaded;
}

void
ModuleManager::set_status( const DictionaryDatum& d )
{
}

void
ModuleManager::install( const std::string& name )
{
  // We cannot have connections without network elements, so we only need to check nodes.
  // Simulating an empty network causes no problems, so we don't have to check for that.
  if ( kernel::manager< NodeManager >.size() > 0 )
  {
    throw KernelException(
      "Network elements have been created, so external modules can no longer be imported. "
      "Call ResetKernel() first." );
  }

  if ( name.empty() )
  {
    throw DynamicModuleManagementError( "Module name must not be empty." );
  }

  if ( modules_.find( name ) != modules_.end() )
  {
    throw DynamicModuleManagementError( "Module '" + name + "' is loaded already." );
  }

  // call lt_dlerror() to reset any error messages hanging around
  lt_dlerror();

  // try to open the module
  const lt_dlhandle hModule = lt_dlopenext( name.c_str() );

  if ( not hModule )
  {
    char* errstr = ( char* ) lt_dlerror();
    std::string msg = "Module '" + name + "' could not be opened.";
    if ( errstr )
    {
      msg += "\nThe dynamic loader returned the following error: '" + std::string( errstr ) + "'.";
    }
    msg += "\n\nPlease check LD_LIBRARY_PATH (OSX: DYLD_LIBRARY_PATH)!";
    throw DynamicModuleManagementError( msg );
  }

  // see if we can find the "module" symbol in the module
  NESTExtensionInterface* extension = reinterpret_cast< NESTExtensionInterface* >( lt_dlsym( hModule, "module" ) );
  char* errstr = ( char* ) lt_dlerror();
  if ( errstr )
  {
    lt_dlclose( hModule ); // close module again
    lt_dlerror();          // remove any error caused by lt_dlclose()
    throw DynamicModuleManagementError(
            "Module '" + name + "' could not be loaded.\n"
            "The dynamic loader returned the following error: '"
            + std::string(errstr) + "'.");
  }

  // all is well and we can register module components
  try
  {
    extension->initialize();
  }
  catch ( std::exception& e )
  {
    lt_dlclose( hModule );
    lt_dlerror(); // remove any error caused by lt_dlclose()
    throw;        // no arg re-throws entire exception, see Stroustrup 14.3.1
  }

  // add the handle to list of loaded modules
  modules_[ name ] = ModuleMapEntry_( hModule, extension );

  LOG( M_INFO, "Install", ( "loaded module " + name ).c_str() );
}

} // namespace nest

#endif /* HAVE_LIBLTDL */
