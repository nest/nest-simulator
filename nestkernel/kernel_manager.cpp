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
#pragma omp critical( create_kernel_manager )
  {
    if ( kernel_manager_instance_ == nullptr )
    {
      kernel_manager_instance_ = new KernelManager();
      assert( kernel_manager_instance_ );
    }
  }
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
  , random_manager()
  , simulation_manager()
  , modelrange_manager()
  , connection_manager()
  , sp_manager()
  , event_delivery_manager()
  , model_manager()
  , music_manager()
  , node_manager()
  , io_manager()
  , managers( { &logging_manager,
      &mpi_manager,
      &vp_manager,
      &random_manager,
      &simulation_manager,
      &modelrange_manager,
      &model_manager,
      &connection_manager,
      &sp_manager,
      &event_delivery_manager,
      &music_manager,
      &io_manager,
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
    manager->initialize();
  }

  ++fingerprint_;
  initialized_ = true;
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
  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->finalize();
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
nest::KernelManager::change_number_of_threads( thread new_num_threads )
{
  // Inputs are checked in VPManager::set_status().
  // Just double check here that all values are legal.
  assert( node_manager.size() == 0 );
  assert( not connection_manager.get_user_set_delay_extrema() );
  assert( not simulation_manager.has_been_simulated() );
  assert( not sp_manager.is_structural_plasticity_enabled() or new_num_threads == 1 );

  vp_manager.set_num_threads( new_num_threads );
  for ( auto& manager : managers )
  {
    manager->change_number_of_threads();
  }
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
