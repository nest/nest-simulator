/*
 *  dynamicloader.h
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

#ifndef DYNAMICLOADER_H
#define DYNAMICLOADER_H

#include "config.h"

// DynamicLoaderModule defined only if libltdl is available

#ifdef HAVE_LIBLTDL

#include "slimodule.h"
#include "slifunction.h"

#include <vector>

class DynModule;

namespace nest
{

  // structure to store handles and pointers to modules
  struct sDynModule;

  typedef std::vector<sDynModule> vecDynModules;

  typedef std::vector<DynModule*> vecLinkedModules;
   
  class Network;

   
  /**
   * SLI interface of the Ddynamic module loader.
   * This class implements the SLI functions which allow for
   * loading dynamic modules into the kernel to extend its functionality.
   */

  class DynamicLoaderModule: public SLIModule
    {
    public:
     
      DynamicLoaderModule(Network *pNet, SLIInterpreter &interpreter);
      ~DynamicLoaderModule();

      void init(SLIInterpreter *);

      const std::string commandstring(void) const;
      const std::string name(void) const;


      /**
       * This static member is called by the constructor of a loadable module that was linked at compile time
       * into the application to circumvent dynamic loading problems. Typically, the constructor of the global
       * instance of the module calls this method to register itself.
       * Later, DynamicLoader will go through all registered modules and initialize them.
       */
      static int registerLinkedModule(DynModule *pModule);

      void initLinkedModules(SLIInterpreter &);
     
    public:
     
      class LoadModuleFunction : public SLIFunction
	{
	public:
    LoadModuleFunction(Network *pNet, vecDynModules &dyn_modules);

	private:
	  void execute(SLIInterpreter *) const;

	private:
	  Network *pNet_;
	  vecDynModules &dyn_modules_;
	};

      class UnloadModuleFunction : public SLIFunction
	{
	public:
	  UnloadModuleFunction(Network *pNet, vecDynModules &dyn_modules);

	private:
	  void execute(SLIInterpreter *) const;

	private:
	  Network *pNet_;
	  vecDynModules &dyn_modules_;
	  
	};



      /** @} */

      LoadModuleFunction          loadmodule_function;
      UnloadModuleFunction        unloadmodule_function;
      
    private:
      /**
       * Provide access to the list of linked modules managed DynamicLoader.
       * This function controls access to the list of linked modules managed
       * by DynamicLoaderModule via a Meyers' Singleton (Alexandrescu, ch 6.4).
       * The list is filled by calls to @c registerLinkedModule().
       */
       static vecLinkedModules& getLinkedModules();
     
      // vector to store handles and pointers to dynamic modules
      vecDynModules dyn_modules;

      Network *pNet_;
      static Dictionary* moduledict_;        //!< Dictionary for dynamically loaded modules.
    };

} // namespace

#endif  // HAVE_LIBLTDL

#endif
