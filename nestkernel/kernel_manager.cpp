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
KernelManager* KernelManager::kernel_manager_instance_ = nullptr;

void
KernelManager::create_kernel_manager()
{
#pragma omp master
  {
    if ( not kernel_manager_instance_ )
    {
      kernel_manager_instance_ = new KernelManager();
      assert( kernel_manager_instance_ );
    }
  }
#pragma omp barrier
}

void
KernelManager::destroy_kernel_manager()
{
  kernel_manager_instance_->logging_manager.set_logging_level( M_QUIET );
  delete kernel_manager_instance_;
}

KernelManager::KernelManager()
  : fingerprint_( 0 )
  , logging_manager( *new LoggingManager() )
  , mpi_manager( *new MPIManager() )
  , vp_manager( *new VPManager() )
  , module_manager( *new ModuleManager() )
  , random_manager( *new RandomManager() )
  , simulation_manager( *new SimulationManager() )
  , modelrange_manager( *new ModelRangeManager() )
  , connection_manager( *new ConnectionManager() )
  , sp_manager( *new SPManager() )
  , event_delivery_manager( *new EventDeliveryManager() )
  , io_manager( *new IOManager() )
  , model_manager( *new ModelManager() )
  , music_manager( *new MUSICManager() )
  , node_manager( *new NodeManager() )
  , managers( { &logging_manager,
      &mpi_manager,
      &vp_manager,
      &module_manager,
      &random_manager,
      &simulation_manager,
      &modelrange_manager,
      &connection_manager,
      &sp_manager,
      &event_delivery_manager,
      &io_manager,
      &model_manager,
      &music_manager,
      &node_manager } )
  , initialized_( false )
{
}

KernelManager::~KernelManager()
{
  if ( initialized_ )
  {
    finalize();
  }

  for ( auto manager : managers )
  {
    delete manager;
  }
}

void
KernelManager::initialize()
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
KernelManager::finalize()
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
  assert( node_manager.size() == 0 );
  assert( not connection_manager.get_user_set_delay_extrema() );
  assert( not simulation_manager.has_been_simulated() );
  assert( not sp_manager.is_structural_plasticity_enabled() or new_num_threads == 1 );

  for ( auto it = managers.rbegin(); it != managers.rend(); ++it )
  {
    ( *it )->finalize( /* adjust_number_of_threads_or_rng_only */ true );
  }

  vp_manager.set_num_threads( new_num_threads );

  // Initialize in original order with new number of threads set
  for ( auto& manager : managers )
  {
    manager->initialize( /* adjust_number_of_threads_or_rng_only */ true );
  }

  // Finalizing deleted all register components. Now that all infrastructure
  // is in place again, we can tell modules to re-register the components
  // they provide.
  module_manager.reinitialize_dynamic_modules();

  // Prepare timers and set the number of threads for multi-threaded timers
  kernel().simulation_manager.reset_timers_for_preparation();
  kernel().simulation_manager.reset_timers_for_dynamics();
  kernel().event_delivery_manager.reset_timers_for_preparation();
  kernel().event_delivery_manager.reset_timers_for_dynamics();
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

} // namespace nest
