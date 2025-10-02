/*
 *  kernel_manager.cpp
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

#include "kernel_manager.h"

// Include concrete manager headers only in the implementation.
#include "connection_manager.h"
#include "event_delivery_manager.h"
#include "io_manager.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "modelrange_manager.h"
#include "module_manager.h"
#include "mpi_manager.h"
#include "music_manager.h"
#include "node_manager.h"
#include "random_manager.h"
#include "simulation_manager.h"
#include "sp_manager.h"
#include "vp_manager.h"

namespace nest
{

KernelManager::KernelManager()
  : fingerprint_( 0 )
  , managers( { &kernel::manager< LoggingManager >,
      &kernel::manager< MPIManager >,
      &kernel::manager< VPManager >,
      &kernel::manager< ModuleManager >,
      &kernel::manager< RandomManager >,
      &kernel::manager< SimulationManager >,
      &kernel::manager< ModelRangeManager >,
      &kernel::manager< ConnectionManager >,
      &kernel::manager< SPManager >,
      &kernel::manager< EventDeliveryManager >,
      &kernel::manager< IOManager >,
      &kernel::manager< ModelManager >,
      &kernel::manager< MUSICManager >,
      &kernel::manager< NodeManager > } )
  , initialized_( false )
{
}

KernelManager::~KernelManager()
{
  if ( initialized_ )
  {
    KernelManager::finalize();
  }
}

void
KernelManager::initialize( const bool )
{
  for ( auto& manager : managers )
  {
    manager->initialize( /* adjust_number_of_threads_or_rng_only */ false );
  }

  ++fingerprint_;
  initialized_ = true;
  FULL_LOGGING_ONLY( dump_.open(
    String::compose( "dump_%1_%2.log", mpi_manager.get_num_processes(), mpi_manager.get_rank() ).c_str() ); )
}

void
KernelManager::prepare()
{
  for ( auto manager : managers )
  {
    manager->prepare();
  }
}

void
KernelManager::cleanup()
{
  for ( auto it = managers.rbegin(); it != managers.rend(); ++it )
  {
    ( *it )->cleanup();
  }
}

void
KernelManager::finalize( const bool )
{
  FULL_LOGGING_ONLY( dump_.close(); )

  for ( auto it = managers.rbegin(); it != managers.rend(); ++it )
  {
    ( *it )->finalize( /* adjust_number_of_threads_or_rng_only */ false );
  }
  initialized_ = false;
}

void
KernelManager::reset()
{
  finalize();
  initialize();
}

void
KernelManager::change_number_of_threads( size_t new_num_threads )
{
  // Inputs are checked in VPManager::set_status().
  // Just double check here that all values are legal.
  assert( kernel::manager< NodeManager >.size() == 0 );
  assert( not kernel::manager< ConnectionManager >.get_user_set_delay_extrema() );
  assert( not kernel::manager< SimulationManager >.has_been_simulated() );
  assert( not kernel::manager< SPManager >.is_structural_plasticity_enabled() or new_num_threads == 1 );

  for ( auto it = managers.rbegin(); it != managers.rend(); ++it )
  {
    ( *it )->finalize( /* adjust_number_of_threads_or_rng_only */ true );
  }

  kernel::manager< VPManager >.set_num_threads( new_num_threads );

  // Initialize in original order with new number of threads set
  for ( auto& manager : managers )
  {
    manager->initialize( /* adjust_number_of_threads_or_rng_only */ true );
  }

  // Finalizing deleted all register components. Now that all infrastructure
  // is in place again, we can tell modules to re-register the components
  // they provide.
  kernel::manager< ModuleManager >.reinitialize_dynamic_modules();

  // Prepare timers and set the number of threads for multi-threaded timers
  kernel::manager< SimulationManager >.reset_timers_for_preparation();
  kernel::manager< SimulationManager >.reset_timers_for_dynamics();
  kernel::manager< EventDeliveryManager >.reset_timers_for_preparation();
  kernel::manager< EventDeliveryManager >.reset_timers_for_dynamics();
}

void
KernelManager::set_status( const DictionaryDatum& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->set_status( dict );
  }
}

void
KernelManager::get_status( DictionaryDatum& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->get_status( dict );
  }
}

void
KernelManager::write_to_dump( const std::string& msg )
{
#pragma omp critical
  // In critical section to avoid any garbling of output.
  {
    dump_ << msg << std::endl << std::flush;
  }
}

bool
KernelManager::is_initialized() const
{
  return initialized_;
}

unsigned long
KernelManager::get_fingerprint() const
{
  return fingerprint_;
}

} // namespace nest
