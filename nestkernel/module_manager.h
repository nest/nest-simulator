/*
 *  module_manager.h
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

#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <map>
#include <string>

// Includes from nestkernel:
#include "manager_interface.h"

// Includes from sli:
#include "dictutils.h"

// DynamicLoaderModule defined only if libltdl is available
#ifdef HAVE_LIBLTDL
#include <ltdl.h>

namespace nest
{
class NESTExtensionInterface;

class ModuleManager : public ManagerInterface
{
public:
  ModuleManager();
  ~ModuleManager() override;

  void initialize( const bool ) override;

  //! Unload modules but only on full ResetKernel(), not when just changing then number of threads
  void finalize( const bool adjust_number_of_threads_or_rng_only ) override;

  //! To be called after change of number of threads to re-register components provided by modules
  void reinitialize_dynamic_modules();

  void get_status( DictionaryDatum& ) override;
  void set_status( const DictionaryDatum& ) override;

  void install( const std::string& name );

private:
  struct ModuleMapEntry_
  {
    ModuleMapEntry_() = default;
    ModuleMapEntry_( lt_dlhandle hndl, NESTExtensionInterface* ext )
      : handle( hndl )
      , extension( ext )
    {
    }

    lt_dlhandle handle;                //!< required for unloading
    NESTExtensionInterface* extension; //!< required for re-initizliation(), avoid re-casting handle
  };

  std::map< std::string, ModuleMapEntry_ > modules_;
};
}

#else

#include "exceptions.h"

namespace nest
{
class ModuleManager : public ManagerInterface
{
public:
  ModuleManager()
  {
  }
  ~ModuleManager() override
  {
  }

  void
  initialize( const bool ) override
  {
  }
  void
  finalize( const bool ) override
  {
  }

  void
  reinitialize_dynamic_modules()
  {
  }

  void
  get_status( DictionaryDatum& ) override
  {
  }
  void
  set_status( const DictionaryDatum& ) override
  {
  }

  void
  install( const std::string& name )
  {
    throw KernelException( "Dynamic modules not supported without libltdl." );
  }
};
}

#endif // HAVE_LIBLTDL

#endif // #ifndef MODULE_MANAGER_H
