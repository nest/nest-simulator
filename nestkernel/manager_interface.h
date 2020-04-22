/*
 *  manager_interface.h
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

#ifndef MANAGER_INTERFACE_H
#define MANAGER_INTERFACE_H

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * Interface for kernel manager classes.
 *
 * This class defines the common interface for all manager classes
 * in the NEST kernel.
 *
 * @note Each manager shall be instantiated only once. Therefore, copy
 * constructor and assignment operator are declared private and not implemented.
 *
 * @ingroup KernelManagers
 */
class ManagerInterface
{
public:
  ManagerInterface( ManagerInterface const& ) = delete; // do not implement
  void operator=( ManagerInterface const& ) = delete;   // do not implement

public:
  ManagerInterface()
  {
  }

  virtual ~ManagerInterface()
  {
  }

  /**
   * Prepare manager for operation.
   *
   * After this method has completed, the manager should be completely
   * initialized and "ready for action".
   *
   * @note Initialization of any given manager may depend on other
   * managers having been initialized before. KernelManager::initialize()
   * is responsible for calling the initialization routines on the
   * specific managers in correct order.
   *
   * @see finalize()
   */
  virtual void initialize() = 0;

  /**
   * Take down manager after operation.
   *
   * After this method has completed, all dynamic data structures created by
   * the manager shall be deallocated and containers emptied. Plain variables
   * need not be reset.
   *
   * @note Finalization of any given manager may depend on other
   * managers not having been finalized yet. KernelManager::finalize()
   * is responsible for calling the initialization routines on the
   * specific managers in correct order, i.e., the opposite order of
   * initialize() calls.
   *
   * @see initialize()
   */
  virtual void finalize() = 0;

  /**
   * Change the number of threads
   *
   * Many data structures depend on the number of threads. This
   * function is called on each manager upon a change of that number
   * and allows the manager to re-allocate data structures
   * accordingly.
   */
  virtual void change_num_threads( thread ){};

  virtual void set_status( const DictionaryDatum& ) = 0;
  virtual void get_status( DictionaryDatum& ) = 0;

  virtual void prepare(){};
  virtual void cleanup(){};
};
}

#endif /* MANAGER_INTERFACE_H */
