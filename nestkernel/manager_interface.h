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
   * @param adjust_number_of_threads_or_rng_only  Pass true if calling from kernel_manager::change_number_of_threads()
   * or RandomManager::get_status() to limit operations to those necessary for thread adjustment or switch or re-seeding
   * of RNG.
   *
   * @see finalize()
   */
  virtual void initialize( const bool adjust_number_of_threads_or_rng_only ) = 0;

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
   * @param adjust_number_of_threads_or_rng_only  Pass true if calling from kernel_manager::change_number_of_threads()
   * to limit operations to those necessary for thread adjustment.
   *
   * @see initialize()
   */
  virtual void finalize( const bool adjust_number_of_threads_or_rng_only ) = 0;

  /**
   * Set the status of the manager
   *
   * @see get_status()
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Retrieve the status of the manager
   *
   * @note This would ideally be a const function. However, some
   * managers delay the update of internal variables up to the point
   * where they are needed (e.g., before reporting their values to the
   * user, or before simulate is called). An example for this pattern
   * is the call to update_delay_extrema_() right at the beginning of
   * ConnectionManager::get_status().
   *
   * @see set_status()
   */
  virtual void get_status( DictionaryDatum& ) = 0;

  virtual void prepare() {};
  virtual void cleanup() {};
};
}

#endif /* MANAGER_INTERFACE_H */
