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

nest::KernelManager* nest::KernelManager::kernel_manager_instance_ = nullptr;

void
nest::KernelManager::create_kernel_manager()
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
nest::KernelManager::destroy_kernel_manager()
{
  kernel_manager_instance_->logging_manager.set_logging_level( M_QUIET );
  delete kernel_manager_instance_;
}

nest::KernelManager::KernelManager()
  : fingerprint_( 0 )
  , logging_manager()
  , mpi_manager()
  , vp_manager()
  , module_manager()
  , random_manager()
  , simulation_manager()
  , modelrange_manager()
  , connection_manager()
  , sp_manager()
  , event_delivery_manager()
  , io_manager()
  , model_manager()
  , music_manager()
  , node_manager()
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

nest::KernelManager::~KernelManager()
{
}

void
nest::KernelManager::initialize()
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
nest::KernelManager::prepare()
{
  for ( auto& manager : managers )
  {
    manager->prepare();
  }
}

void
nest::KernelManager::cleanup()
{
  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->cleanup();
  }
}

void
nest::KernelManager::finalize()
{
  FULL_LOGGING_ONLY( dump_.close(); )

  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->finalize( /* adjust_number_of_threads_or_rng_only */ false );
  }
  initialized_ = false;
}

void
nest::KernelManager::reset()
{
  finalize();
  initialize();
}

void
nest::KernelManager::change_number_of_threads( size_t new_num_threads )
{
  // Inputs are checked in VPManager::set_status().
  // Just double check here that all values are legal.
  assert( node_manager.size() == 0 );
  assert( not connection_manager.get_user_set_delay_extrema() );
  assert( not simulation_manager.has_been_simulated() );
  assert( not sp_manager.is_structural_plasticity_enabled() or new_num_threads == 1 );

  // Finalize in reverse order of initialization with old thread number set
  for ( auto mgr_it = managers.rbegin(); mgr_it != managers.rend(); ++mgr_it )
  {
    ( *mgr_it )->finalize( /* adjust_number_of_threads_or_rng_only */ true );
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

  // Prepare timers
  kernel().simulation_manager.reset_timers_for_preparation();
  kernel().simulation_manager.reset_timers_for_dynamics();
  kernel().event_delivery_manager.reset_timers_for_preparation();
  kernel().event_delivery_manager.reset_timers_for_dynamics();
}

void
nest::KernelManager::set_status( const DictionaryDatum& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->set_status( dict );
  }
}

void
nest::KernelManager::get_status( DictionaryDatum& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->get_status( dict );
  }
}

void
nest::KernelManager::write_to_dump( const std::string& msg )
{
#pragma omp critical
  // In critical section to avoid any garbling of output.
  {
    dump_ << msg << std::endl << std::flush;
  }
}
