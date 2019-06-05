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

nest::KernelManager* nest::KernelManager::kernel_manager_instance_ = 0;

void
nest::KernelManager::create_kernel_manager()
{
#pragma omp critical( create_kernel_manager )
  {
    if ( kernel_manager_instance_ == 0 )
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
  : logging_manager()
  , mpi_manager()
  , vp_manager()
  , rng_manager()
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
      &rng_manager,
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
  for ( auto& m : managers )
  {
    m->initialize();
  }

  initialized_ = true;
}

void
nest::KernelManager::prepare()
{
  for ( auto& m : managers )
  {
    m->prepare();
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
  initialized_ = false;

  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->finalize();
  }
}

void
nest::KernelManager::reset()
{
  finalize();
  initialize();
}

void
nest::KernelManager::change_num_threads( thread num_threads )
{
  // JME: TODO check all managers for a dependency on the number of
  // threads and implement change_num_threads() for the ones that have
  // one. The sequence below is really dangerous, as this function is
  // called by VPManager::set_status(), which is in turn called from
  // the UI. Finalizing and re-initializing all managers will discard
  // all changes to the corresponding manager's status dictionaries.

  node_manager.finalize();
  connection_manager.finalize();
  model_manager.finalize();
  modelrange_manager.finalize();
  rng_manager.finalize();

  // JME: this should go to where KernelManager::change_num_threads()
  // is called in VPManager. apeyser/nestio did not have this line. It
  // was added in master in 18acd78aa1c00
  vp_manager.set_num_threads( num_threads );

  rng_manager.initialize();
  // independent of threads, but node_manager needs it reset
  modelrange_manager.initialize();
  model_manager.initialize();
  connection_manager.initialize();
  event_delivery_manager.initialize();
  music_manager.initialize();
  node_manager.initialize();

  for ( auto& manager : managers )
  {
    manager->change_num_threads( num_threads );
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
