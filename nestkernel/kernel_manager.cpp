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
  kernel_manager_instance_->finalize();
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
  , managers({
      &logging_manager,
      &mpi_manager,
      &vp_manager,
      &rng_manager,
      &simulation_manager,
      &modelrange_manager,
      &connection_manager,
      &sp_manager,
      &event_delivery_manager,
      &model_manager,
      &music_manager,
      &node_manager,
      &io_manager
    })
  , initialized_( false )
{
}

nest::KernelManager::~KernelManager()
{
}

  // logging_manager.initialize(); // must come first so others can log
  // io_manager.initialize();      // independent of others

  // mpi_manager.initialize(); // set up inter-process communication
  // vp_manager.initialize();  // set up threads

  // // invariant: process infrastructure (MPI, threads) in place

  // rng_manager.initialize(); // depends on number of VPs

  // // invariant: supporting managers set up

  // // "Core kernel managers" follow
  // simulation_manager.initialize(); // independent of others
  // modelrange_manager.initialize(); // independent of others
  // connection_manager.initialize(); // depends only on num of threads
  // sp_manager.initialize();

  // // prerequisites:
  // //   - min_delay/max_delay available (connection_manager)
  // //   - clock initialized (simulation_manager)
  // event_delivery_manager.initialize();

  // model_manager.initialize(); // depends on number of threads

  // music_manager.initialize();

  // // prerequisites:
  // //   - modelrange_manager initialized
  // //   - model_manager for pristine models
  // //   - vp_manager for number of threads
  // node_manager.initialize(); // must come last

void
nest::KernelManager::initialize()
{
  for (auto& m: managers)
  {
    m->initialize();
  }

  initialized_ = true;
}

void
nest::KernelManager::prepare()
{
  for (auto& m: managers)
  {
    m->prepare();
  }

  std::cerr << "Starting kernelmanager prepare" << std::endl;
}

void
nest::KernelManager::cleanup()
{
  for (auto&& m_it = managers.rbegin();
       m_it != managers.rend();
       ++m_it)
  {
    (*m_it)->cleanup();
  }
}

void
nest::KernelManager::finalize()
{
  initialized_ = false;

  for (auto&& m_it = managers.rbegin();
       m_it != managers.rend();
       ++m_it)
  {
    (*m_it)->finalize();
  }
}

void
nest::KernelManager::reset()
{
  finalize();
  initialize();
}

void
nest::KernelManager::num_threads_changed_reset()
{
  io_manager.finalize();
  node_manager.finalize();
  model_manager.finalize();
  connection_manager.finalize();
  modelrange_manager.finalize();
  rng_manager.finalize();

  rng_manager.initialize();
  // independent of threads, but node_manager needs it reset
  modelrange_manager.initialize();
  connection_manager.initialize();
  model_manager.initialize();
  music_manager.initialize();
  node_manager.initialize();
  io_manager.initialize();
}

void
nest::KernelManager::set_status( const DictionaryDatum& dict )
{
  assert( is_initialized() );
  logging_manager.set_status( dict );
  io_manager.set_status( dict );

  mpi_manager.set_status( dict );
  vp_manager.set_status( dict );

  // set RNGs --- MUST come after n_threads_ is updated
  rng_manager.set_status( dict );
  simulation_manager.set_status( dict );
  modelrange_manager.set_status( dict );
  connection_manager.set_status( dict );
  sp_manager.set_status( dict );

  event_delivery_manager.set_status( dict );
  model_manager.set_status( dict );
  music_manager.set_status( dict );

  node_manager.set_status( dict ); // has to be called last
}

void
nest::KernelManager::get_status( DictionaryDatum& dict )
{
  assert( is_initialized() );
  logging_manager.get_status( dict );
  io_manager.get_status( dict );

  mpi_manager.get_status( dict );
  vp_manager.get_status( dict );

  rng_manager.get_status( dict );
  simulation_manager.get_status( dict );
  modelrange_manager.get_status( dict );
  connection_manager.get_status( dict );
  sp_manager.get_status( dict );

  event_delivery_manager.get_status( dict );
  model_manager.get_status( dict );
  music_manager.get_status( dict );

  node_manager.get_status( dict );
}
