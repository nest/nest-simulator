/*
 *  neststartup.cpp
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

#include "neststartup.h"

// C++ includes:
#include <fstream>

// Generated includes:
#include "config.h"
#include "static_modules.h"

// Includes from libnestutil:
#include "logging.h"
#include "logging_event.h"

// Includes from librandom:
#include "random_numbers.h"

// Includes from nestkernel:
#include "dynamicloader.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest.h"
#include "nestmodule.h"
#include "model_manager_impl.h"
#include "exceptions.h"

// Includes from sli:
#include "dict.h"
#include "dictdatum.h"
#include "filesystem.h"
#include "interpret.h"
#include "oosupport.h"
#include "processes.h"
#include "sliarray.h"
#include "sligraphics.h"
#include "sliregexp.h"
#include "slistartup.h"
#include "specialfunctionsmodule.h"

#if defined( _BUILD_NEST_CLI ) && defined( HAVE_READLINE )
#include <gnureadline.h>
#endif

SLIInterpreter* sli_engine;

SLIInterpreter&
get_engine()
{
  assert( sli_engine );
  return *sli_engine;
}

void
sli_logging( const nest::LoggingEvent& e )
{
  sli_engine->message( static_cast< int >( e.severity ), e.function.c_str(), e.message.c_str() );
}

int
#ifndef _IS_PYNEST
neststartup( int* argc, char*** argv, SLIInterpreter& engine )
#else
neststartup( int* argc, char*** argv, SLIInterpreter& engine, std::string modulepath )
#endif
{
  nest::init_nest( argc, argv );

  sli_engine = &engine;
  register_logger_client( sli_logging );

// We disable synchronization between stdio and istd::ostreams
// this has to be done before any in- or output has been done.
/*
 * TODO: This block looks to me as if it would evaluate to the same stuff
 *       in all cases. Can it be removed (or simplified, if I'm wrong ;-)
 */
#ifdef __GNUC__
#if __GNUC__ < 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ < 1 )
  // Broken with GCC 3.1 and higher.
  // cin.get() never returns, or leaves cin in a broken state.
  std::ios::sync_with_stdio( false );
#endif
#else
  // This is for all other compilers
  std::ios::sync_with_stdio( false );
#endif

  addmodule< OOSupportModule >( engine );
  addmodule< RandomNumbers >( engine );

#if defined( _BUILD_NEST_CLI ) && defined( HAVE_READLINE )
  addmodule< GNUReadline >( engine );
#endif

  addmodule< SLIArrayModule >( engine );
  addmodule< SpecialFunctionsModule >( engine ); // safe without GSL
  addmodule< SLIgraphics >( engine );
  engine.addmodule( new SLIStartup( *argc, *argv ) );
  addmodule< Processes >( engine );
  addmodule< RegexpModule >( engine );
  addmodule< FilesystemModule >( engine );

  // register NestModule class
  addmodule< nest::NestModule >( engine );

  // this can make problems with reference counting, if
  // the intepreter decides cleans up memory before NEST is ready
  engine.def( "modeldict", nest::kernel().model_manager.get_modeldict() );
  engine.def( "synapsedict", nest::kernel().model_manager.get_synapsedict() );
  engine.def( "connruledict", nest::kernel().connection_manager.get_connruledict() );
  engine.def( "growthcurvedict", nest::kernel().sp_manager.get_growthcurvedict() );

  // now add static modules providing models
  add_static_modules( engine );

/*
 * The following section concerns shared user modules and is thus only
 * included if we built with libtool and libltdl.
 *
 * On BlueGene, we need to link user modules statically, but for convenience
 * they still register themselves with the DyamicLoadModule during static
 * initialization. On BlueGene, we then need to prevent that the modules are
 * loaded a second time, therefore the guard below. At the same time, we
 * need to create the DynamicLoaderModule, since the compiler might otherwise
 * optimize DynamicLoaderModule::registerLinkedModule() away.
 */
#ifdef HAVE_LIBLTDL
  // dynamic loader module for managing linked and dynamically loaded extension
  // modules
  nest::DynamicLoaderModule* pDynLoader = new nest::DynamicLoaderModule( engine );

// initialize all modules that were linked into at compile time
// these modules have registered via calling DynamicLoader::registerLinkedModule
// from their constructor
#ifndef IS_BLUEGENE
  pDynLoader->initLinkedModules( engine );

  // interpreter will delete module on destruction
  engine.addmodule( pDynLoader );
#endif
#endif

#ifdef _IS_PYNEST
  // add the init-script to the list of module initializers
  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( engine.baselookup( engine.commandstring_name ).datum() );
  assert( ad != NULL );
  ad->push_back( new StringDatum( "(" + modulepath + "/pynest-init.sli) run" ) );
#endif

  return engine.startup();
}

void
nestshutdown( int exitcode )
{
  nest::kernel().finalize();
  nest::kernel().mpi_manager.mpi_finalize( exitcode );
  nest::KernelManager::destroy_kernel_manager();
}

#if defined( HAVE_LIBNEUROSIM ) && defined( _IS_PYNEST )
Datum*
CYTHON_unpackConnectionGeneratorDatum( PyObject* obj )
{
  Datum* ret = NULL;
  ConnectionGenerator* cg = NULL;

  cg = PNS::unpackConnectionGenerator( obj );
  if ( cg != NULL )
  {
    ret = static_cast< Datum* >( new ConnectionGeneratorDatum( cg ) );
  }

  return ret;
}
#endif

#ifdef _IS_PYNEST
#ifdef HAVE_MPI4PY

#include <mpi4py/mpi4py.h>

void
set_communicator( PyObject* pyobj )
{
  import_mpi4py();

  // If object is not a mpi4py communicator, bail
  if ( not PyObject_TypeCheck( pyobj, &PyMPIComm_Type ) )
  {
    throw nest::KernelException( "set_communicator: argument is not a mpi4py communicator" );
  }

  nest::kernel().mpi_manager.set_communicator( *PyMPIComm_Get( pyobj ) );
}

#else // ! HAVE_MPI4PY

void
set_communicator( PyObject* )
{
  throw nest::KernelException( "set_communicator: NEST not compiled with MPI4PY" );
}

#endif
#endif //_IS_PYNEST
