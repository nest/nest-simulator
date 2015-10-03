/*
 *  vp_manager.cpp
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

#include "vp_manager.h"
#include "kernel_manager.h"
#include "dictutils.h"
#include "network.h"
#include "logging.h"
#include "vp_manager_impl.h"

nest::VPManager::VPManager()
  : force_singlethreading_( false )
  , n_threads_( 1 )
{
}

void
nest::VPManager::init()
{
#ifndef _OPENMP
  if ( n_threads_ > 1 )
  {
    LOG( M_ERROR, "Network::reset", "No multithreading available, using single threading" );
    n_threads_ = 1;
    force_singlethreading_ = true;
  }
#endif

  set_num_threads( get_num_threads() );
}

void
nest::VPManager::reset()
{
}

void
nest::VPManager::set_status( const DictionaryDatum& d )
{
  long n_threads;
  bool n_threads_updated = updateValue< long >( d, "local_num_threads", n_threads );
  if ( n_threads_updated )
  {
    if ( Network::get_network().size() > 1 )
      throw KernelException( "Nodes exist: Thread/process number cannot be changed." );
    if ( Network::get_network().models_.size() > Network::get_network().pristine_models_.size() )
      throw KernelException(
        "Custom neuron models exist: Thread/process number cannot be changed." );
    if ( Network::get_network().connection_manager_.has_user_prototypes() )
      throw KernelException(
        "Custom synapse types exist: Thread/process number cannot be changed." );
    if ( kernel().connection_builder_manager.get_user_set_delay_extrema() )
      throw KernelException(
        "Delay extrema have been set: Thread/process number cannot be changed." );
    if ( kernel().simulation_manager.has_been_simulated() )
      throw KernelException(
        "The network has been simulated: Thread/process number cannot be changed." );
    if ( not Time::resolution_is_default() )
      throw KernelException(
        "The resolution has been set: Thread/process number cannot be changed." );
    if ( Network::get_network().model_defaults_modified() )
      throw KernelException(
        "Model defaults have been modified: Thread/process number cannot be changed." );

    if ( n_threads > 1 && force_singlethreading_ )
    {
      LOG(
        M_WARNING, "Network::set_status", "No multithreading available, using single threading" );
      n_threads_ = 1;
    }

    // it is essential to call reset() here to adapt memory pools and more
    // to the new number of threads and VPs.
    n_threads_ = n_threads;
    Network::get_network().reset();
  }

  long n_vps;
  bool n_vps_updated = updateValue< long >( d, "total_num_virtual_procs", n_vps );
  if ( n_vps_updated )
  {
    if ( Network::get_network().size() > 1 )
      throw KernelException( "Nodes exist: Thread/process number cannot be changed." );
    if ( Network::get_network().models_.size() > Network::get_network().pristine_models_.size() )
      throw KernelException(
        "Custom neuron models exist: Thread/process number cannot be changed." );
    if ( Network::get_network().connection_manager_.has_user_prototypes() )
      throw KernelException(
        "Custom synapse types exist: Thread/process number cannot be changed." );
    if ( kernel().connection_builder_manager.get_user_set_delay_extrema() )
      throw KernelException(
        "Delay extrema have been set: Thread/process number cannot be changed." );
    if ( kernel().simulation_manager.has_been_simulated() )
      throw KernelException(
        "The network has been simulated: Thread/process number cannot be changed." );
    if ( not Time::resolution_is_default() )
      throw KernelException(
        "The resolution has been set: Thread/process number cannot be changed." );
    if ( Network::get_network().model_defaults_modified() )
      throw KernelException(
        "Model defaults have been modified: Thread/process number cannot be changed." );

    if ( n_vps % kernel().mpi_manager.get_num_processes() != 0 )
      throw BadProperty(
        "Number of virtual processes (threads*processes) must be an integer "
        "multiple of the number of processes. Value unchanged." );

    n_threads_ = n_vps / kernel().mpi_manager.get_num_processes();
    if ( ( n_threads > 1 ) && ( force_singlethreading_ ) )
    {
      LOG(
        M_WARNING, "Network::set_status", "No multithreading available, using single threading" );
      n_threads_ = 1;
    }

    // it is essential to call reset() here to adapt memory pools and more
    // to the new number of threads and VPs
    set_num_threads( n_threads_ );
    Network::get_network().reset();
  }
}

void
nest::VPManager::get_status( DictionaryDatum& d )
{
  def< long >( d, "local_num_threads", n_threads_ );
  def< long >( d, "total_num_virtual_procs", kernel().vp_manager.get_num_virtual_processes() );
}

void
nest::VPManager::set_num_threads( nest::thread n_threads )
{
  n_threads_ = n_threads;
  Network::get_network().nodes_vec_.resize( n_threads_ );

#ifdef _OPENMP
  omp_set_num_threads( n_threads_ );

#ifdef USE_PMA
// initialize the memory pools
#ifdef IS_K
  assert( n_threads <= MAX_THREAD && "MAX_THREAD is a constant defined in allocator.h" );

#pragma omp parallel
  poormansallocpool[ omp_get_thread_num() ].init();
#else
#pragma omp parallel
  poormansallocpool.init();
#endif
#endif

#endif
}

// TODO: put those functions as inlines in the header, as soon as all
//       references to Network are gone.

bool
nest::VPManager::is_local_vp( nest::thread vp ) const
{
  return Network::get_network().get_process_id( vp ) == kernel().mpi_manager.get_rank();
}

nest::thread
nest::VPManager::suggest_vp( nest::index gid ) const
{
  return gid % ( kernel().mpi_manager.get_num_sim_processes() * n_threads_ );
}

nest::thread
nest::VPManager::suggest_rec_vp( nest::index gid ) const
{
  return gid % ( kernel().mpi_manager.get_num_rec_processes() * n_threads_ )
    + kernel().mpi_manager.get_num_sim_processes() * n_threads_;
}

nest::thread
nest::VPManager::vp_to_thread( nest::thread vp ) const
{
  if ( vp >= static_cast< thread >( kernel().mpi_manager.get_num_sim_processes() * n_threads_ ) )
  {
    return ( vp + kernel().mpi_manager.get_num_sim_processes() * ( 1 - n_threads_ )
             - kernel().mpi_manager.get_rank() ) / kernel().mpi_manager.get_num_rec_processes();
  }
  else
  {
    return vp / kernel().mpi_manager.get_num_sim_processes();
  }
}

nest::thread
nest::VPManager::thread_to_vp( nest::thread t ) const
{
  if ( kernel().mpi_manager.get_rank() >= static_cast< int >( kernel().mpi_manager.get_num_sim_processes() ) )
  {
    // Rank is a recording process
    return t * kernel().mpi_manager.get_num_rec_processes() + kernel().mpi_manager.get_rank()
      - kernel().mpi_manager.get_num_sim_processes() + kernel().mpi_manager.get_num_sim_processes() * n_threads_;
  }
  else
  {
    // Rank is a simulating process
    return t * kernel().mpi_manager.get_num_sim_processes() + kernel().mpi_manager.get_rank();
  }
}
