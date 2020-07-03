/*
 *  dynamicloader.cpp
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

/*
   This file is part of NEST.

   dynamicloader.cpp -- Implements the class DynamicLoaderModule
   to allow for dymanically loaded modules for extending the kernel.

   Author(s):
   Moritz Helias

   First Version: November 2005
*/

#include "dynamicloader.h"

#ifdef HAVE_LIBLTDL

// External includes:
#include <ltdl.h>

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model.h"

// Includes from sli:
#include "integerdatum.h"
#include "interpret.h"
#include "stringdatum.h"


namespace nest
{

struct sDynModule
{
  std::string name;
  lt_dlhandle handle;
  SLIModule* pModule;

  bool operator==( const sDynModule& rhs ) const
  {
    return name == rhs.name;
  }

  // operator!= must be implemented explicitly, not all compilers
  // generate it automatically from operator==
  bool operator!=( const sDynModule& rhs ) const
  {
    return not( *this == rhs );
  }
};

// static member initialization
Dictionary* DynamicLoaderModule::moduledict_ = new Dictionary();

vecLinkedModules&
DynamicLoaderModule::getLinkedModules()
{
  static vecLinkedModules lm; // initialized empty on first call
  return lm;
}


/*! At the time when DynamicLoaderModule is constructed, the SLI Interpreter
  and NestModule must be already constructed and initialized.
  DynamicLoaderModule relies on the presence of
  the following SLI datastructures: Name, Dictionary.
*/
DynamicLoaderModule::DynamicLoaderModule( SLIInterpreter& interpreter )
  : loadmodule_function( dyn_modules )
{
  interpreter.def( "moduledict", new DictionaryDatum( moduledict_ ) );
}

DynamicLoaderModule::~DynamicLoaderModule()
{
  // unload all loaded modules
  for ( vecDynModules::iterator it = dyn_modules.begin(); it != dyn_modules.end(); ++it )
  {
    if ( it->handle != NULL )
    {
      lt_dlclose( it->handle );
      it->handle = NULL;
    }
  }

  lt_dlexit();
}

// The following concerns the new module: -----------------------

const std::string
DynamicLoaderModule::name( void ) const
{
  return std::string( "NEST-Dynamic Loader" ); // Return name of the module
}

const std::string
DynamicLoaderModule::commandstring( void ) const
{
  return std::string( "" ); // Run associated SLI startup script
}


// auxiliary function to check name of module via its pointer
// we cannot use a & for the second argument, as std::bind2nd() then
// becomes confused, at least with g++ 4.0.1.
bool
has_name( SLIModule const* const m, const std::string n )
{
  return m->name() == n;
}


/** @BeginDocumentation
  Name: Install - Load a dynamic module to extend the functionality.

  Description:

  Synopsis: (module_name) Install -> handle
*/
DynamicLoaderModule::LoadModuleFunction::LoadModuleFunction( vecDynModules& dyn_modules )
  : dyn_modules_( dyn_modules )
{
}

void
DynamicLoaderModule::LoadModuleFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  if ( kernel().model_manager.has_user_models() or kernel().model_manager.has_user_prototypes() )
  {
    throw DynamicModuleManagementError( "Modules cannot be installed after CopyModel has been called" );
  }

  sDynModule new_module;

  new_module.name = getValue< std::string >( i->OStack.top() );
  if ( new_module.name.empty() )
  {
    throw DynamicModuleManagementError( "Module name must not be empty." );
  }

  // check if module already loaded
  // this check can happen here, since we are comparing dynamically loaded
  // modules based on the name given to the Install command
  if ( std::find( dyn_modules_.begin(), dyn_modules_.end(), new_module ) != dyn_modules_.end() )
  {
    throw DynamicModuleManagementError( "Module '" + new_module.name + "' is loaded already." );
  }

  // call lt_dlerror() to reset any error messages hanging around
  lt_dlerror();
  // try to open the module
  const lt_dlhandle hModule = lt_dlopenext( new_module.name.c_str() );

  if ( not hModule )
  {
    char* errstr = ( char* ) lt_dlerror();
    std::string msg = "Module '" + new_module.name + "' could not be opened.";
    if ( errstr )
    {
      msg += "\nThe dynamic loader returned the following error: '" + std::string( errstr ) + "'.";
    }
    msg += "\n\nPlease check LD_LIBRARY_PATH (OSX: DYLD_LIBRARY_PATH)!";
    throw DynamicModuleManagementError( msg );
  }

  // see if we can find the mod symbol in the module
  SLIModule* pModule = ( SLIModule* ) lt_dlsym( hModule, "mod" );
  char* errstr = ( char* ) lt_dlerror();
  if ( errstr )
  {
    lt_dlclose( hModule ); // close module again
    lt_dlerror();          // remove any error caused by lt_dlclose()
    throw DynamicModuleManagementError(
            "Module '" + new_module.name + "' could not be loaded.\n"
            "The dynamic loader returned the following error: '"
            + std::string(errstr) + "'.");
  }

  // check if module is linked in. This test is based on the module name
  // returned by DynModule::name(), since we have no file names for linked
  // modules. We can only perform it after we have loaded the module.
  if ( std::find_if( DynamicLoaderModule::getLinkedModules().begin(),
         DynamicLoaderModule::getLinkedModules().end(),
         std::bind( has_name, std::placeholders::_1, pModule->name() ) )
    != DynamicLoaderModule::getLinkedModules().end() )
  {
    lt_dlclose( hModule ); // close module again
    lt_dlerror();          // remove any error caused by lt_dlclose()
    throw DynamicModuleManagementError(
            "Module '" + new_module.name + "' is linked into NEST.\n"
            "You neither need nor may load it dynamically in addition.");
  }

  // all is well an we can register the module with the interpreter
  try
  {
    pModule->install( std::cerr, i );
  }
  catch ( std::exception& e )
  {
    // We should uninstall the partially installed module here, but
    // this must wait for #152.
    // For now, we just close the module file and rethrow the exception.

    lt_dlclose( hModule );
    lt_dlerror(); // remove any error caused by lt_dlclose()
    throw;        // no arg re-throws entire exception, see Stroustrup 14.3.1
  }

  // add the handle to list of loaded modules
  new_module.handle = hModule;
  new_module.pModule = pModule;
  dyn_modules_.push_back( new_module );

  LOG( M_INFO, "Install", ( "loaded module " + pModule->name() ).c_str() );

  // remove operand and operator from stack
  i->OStack.pop();
  i->EStack.pop();

  // put handle to module onto stack
  int moduleid = dyn_modules_.size() - 1;
  i->OStack.push( moduleid );
  ( *moduledict_ )[ new_module.name ] = moduleid;

  // now we can run the module initializer, after we have cleared the EStack
  if ( not pModule->commandstring().empty() )
  {
    Token t = new StringDatum( pModule->commandstring() );
    i->OStack.push_move( t );
    Token c = new NameDatum( "initialize_module" );
    i->EStack.push_move( c );
  }
}

void
DynamicLoaderModule::init( SLIInterpreter* i )
{
  // bind functions to terminal names
  i->createcommand( "Install", &loadmodule_function );

  // the ld_* functions return 0 on success and an int > 0 on failure
  if ( lt_dlinit() )
  {
    LOG( M_ERROR, "DynamicLoaderModule::init", "Could not initialize libltdl. No dynamic modules will be available." );
  }

  if ( lt_dladdsearchdir( NEST_INSTALL_PREFIX "/" NEST_INSTALL_LIBDIR ) )
  {
    LOG( M_ERROR, "DynamicLoaderModule::init", "Could not add dynamic module search directory." );
  }
}


int
DynamicLoaderModule::registerLinkedModule( SLIModule* pModule )
{
  assert( pModule != 0 );
  getLinkedModules().push_back( pModule );
  return getLinkedModules().size();
}

void
DynamicLoaderModule::initLinkedModules( SLIInterpreter& interpreter )
{

  for ( vecLinkedModules::iterator it = getLinkedModules().begin(); it != getLinkedModules().end(); ++it )
  {
    interpreter.message( SLIInterpreter::M_STATUS, "DynamicLoaderModule::initLinkedModules", "adding linked module" );
    interpreter.message( SLIInterpreter::M_STATUS, "DynamicLoaderModule::initLinkedModules", ( *it )->name().c_str() );
    interpreter.addlinkedusermodule( *it );
  }
}


} // namespace nest

#endif // HAVE_LIBLTDL
