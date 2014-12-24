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

#include "config.h"

#include "neststartup.h"

#include <fstream>

#include "network.h"
#include "interpret.h"
#include "communicator.h"

#include "dict.h"
#include "dictdatum.h"
#include "random_numbers.h"

#ifndef _IS_PYNEST
#include "gnureadline.h"
#endif

#include "slistartup.h"
#include "sliarray.h"
#include "oosupport.h"
#include "processes.h"
#include "nestmodule.h"
#include "sliregexp.h"
#include "specialfunctionsmodule.h"
#include "sligraphics.h"
#include "dynamicloader.h"
#include "filesystem.h"

#include "static_modules.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef _IS_PYNEST
int neststartup(int argc, char** argv, SLIInterpreter &engine, nest::Network* &pNet)
#else
int neststartup(int argc, char** argv, SLIInterpreter &engine, nest::Network* &pNet, std::string modulepath)
#endif
{

#ifdef HAVE_MPI
  nest::Communicator::init(&argc, &argv);
#endif

#ifdef _OPENMP
  /* The next line is required because we use the OpenMP
     threadprivate() directive in the allocator, see OpenMP
     API Specifications v 3.1, Ch 2.9.2, p 89, l 14f. 
     It keeps OpenMP from automagically changing the number
     of threads used for parallel regions. 
  */
  omp_set_dynamic(false); 
  omp_set_num_threads(1);
#endif

  // We disable synchronization between stdio and istd::ostreams
  // this has to be done before any in- or output has been done.
/*
 * TODO: This block looks to me as if it would evaluate to the same stuff
 *       in all cases. Can it be removed (or simplified, if I'm wrong ;-)
 */
#ifdef __GNUC__
#if  __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
  // Broken with GCC 3.1 and higher. 
  // cin.get() never returns, or leaves cin in a broken state.
  std::ios::sync_with_stdio(false);
#endif
#else
  // This is for all other compilers
  std::ios::sync_with_stdio(false);
#endif

  addmodule<OOSupportModule>(engine);   
  addmodule<RandomNumbers>(engine);

#if defined(HAVE_READLINE) && !defined(_IS_PYNEST)
  addmodule<GNUReadline>(engine);
#endif

  addmodule<SLIArrayModule>(engine);
  addmodule<SpecialFunctionsModule>(engine);   // safe without GSL
  addmodule<SLIgraphics>(engine);
  engine.addmodule(new SLIStartup(argc,argv));
  addmodule<Processes>(engine);
  addmodule<RegexpModule>(engine);
  addmodule<FilesystemModule>(engine);

  // create the network and register with NestModule class
  pNet = new nest::Network(engine);
  assert(pNet != 0);
  nest::NestModule::register_network(*pNet);
  addmodule<nest::NestModule>(engine);

  // now add static modules providing models
  add_static_modules(engine, *pNet);

#ifdef HAVE_LIBLTDL
  //dynamic loader module for managing linked and dynamically loaded extension modules
  nest::DynamicLoaderModule *pDynLoader = new nest::DynamicLoaderModule(pNet, engine);

  // initialize all modules that were linked into at compile time
  // these modules have registered via calling DynamicLoader::registerLinkedModule
  // from their constructor
  pDynLoader->initLinkedModules(engine);

  // interpreter will delete module on destruction
  engine.addmodule(pDynLoader);
#endif

#ifdef _IS_PYNEST
  // add the init-script to the list of module initializers
  ArrayDatum *ad = dynamic_cast<ArrayDatum *>(engine.baselookup(engine.commandstring_name).datum());
  assert(ad != NULL);
  ad->push_back(new StringDatum("(" + modulepath + "/pynest-init.sli) run"));
#endif

  return engine.startup();
}

void nestshutdown(void)
{
#ifdef HAVE_MPI
  nest::Communicator::finalize();
#endif
}

#if defined(HAVE_LIBNEUROSIM) && defined(_IS_PYNEST)
Datum* CYTHON_unpackConnectionGeneratorDatum(PyObject* obj)
{
  Datum* ret = NULL;
  ConnectionGenerator* cg = NULL;

  cg = PNS::unpackConnectionGenerator(obj);

  if (cg != NULL)
    ret = static_cast<Datum*>(new nest::ConnectionGeneratorDatum(cg));

  return ret;
}
#endif
