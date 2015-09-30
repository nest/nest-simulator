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

#include "dictdatum.h"

namespace nest
{

class VPManager;

class KernelManager
{
private:
  KernelManager();
  ~KernelManager();
  static KernelManager* kernel_manager_instance_;

  KernelManager( KernelManager const& );  // Don't Implement
  void operator=( KernelManager const& ); // Don't Implement

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

  VPManager vp_manager;
  LoggingManager logging_manager;
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


#endif /* KERNEL_MANAGER_H */
