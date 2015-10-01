/*
 *  kernel_manager.h
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

#ifndef KERNEL_MANAGER_H
#define KERNEL_MANAGER_H

#include "vp_manager.h"
#include "logging_manager.h"
#include "io_manager.h"
#include "simulation_manager.h"

#include "dictdatum.h"

namespace nest
{

class KernelManager
{
private:
  KernelManager();
  ~KernelManager();
  static KernelManager* kernel_manager_instance_;

  KernelManager( KernelManager const& );  // do not implement
  void operator=( KernelManager const& ); // do not implement

public:
  /**
   * Create/destroy and access the KernelManager singleton.
   */
  static void create_kernel_manager();
  static void destroy_kernel_manager();
  static KernelManager& get_kernel_manager();

  void init();
  void reset();

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& );

  //! Returns true if kernel is initialized
  bool is_initialized() const;

  VPManager vp_manager;
  LoggingManager logging_manager;
  IOManager io_manager;
  SimulationManager simulation_manager;

private:
  bool initialized_;   //!< true if all sub-managers initialized
};

KernelManager& kernel();

} // namespace nest

inline nest::KernelManager&
nest::KernelManager::get_kernel_manager()
{
  assert( kernel_manager_instance_ );
  return *kernel_manager_instance_;
}

inline nest::KernelManager&
nest::kernel()
{
  return KernelManager::get_kernel_manager();
}

inline bool
nest::KernelManager::is_initialized() const
{
  return initialized_;
}

#endif /* KERNEL_MANAGER_H */
