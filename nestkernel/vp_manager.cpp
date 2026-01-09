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

// C++ includes:
#include <cstdlib>

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "connection_manager.h"
#include "kernel_manager.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "mpi_manager.h"
#include "node_manager.h"
#include "simulation_manager.h"
#include "sp_manager.h"


// Includes from sli:
#include "dictutils.h"

nest::VPManager::VPManager()
#ifdef _OPENMP
  : force_singlethreading_( false )
#else
  : force_singlethreading_( true )
#endif
  , n_threads_( 1 )
{
}

void
nest::VPManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( adjust_number_of_threads_or_rng_only )
  {
    return;
  }

// When the VPManager is initialized, you will have 1 thread again.
// Setting more threads will be done via nest::set_kernel_status
#ifdef _OPENMP
  // The next line is required because we use the OpenMP
  // threadprivate() directive in the allocator, see OpenMP
  // API Specifications v 3.1, Ch 2.9.2, p 89, l 14f.
  // It keeps OpenMP from automagically changing the number
  // of threads used for parallel regions.
  omp_set_dynamic( false );
#endif

  if ( get_OMP_NUM_THREADS() > 1 )
  {
    std::string msg = "OMP_NUM_THREADS is set in your environment, but NEST ignores it.\n";
    msg += "For details, see the Guide to parallel computing in the NEST Documentation.";

    LOG( M_INFO, "VPManager::initialize()", msg );
  }

  set_num_threads( 1 );
}

void
nest::VPManager::finalize( const bool )
{
}

size_t
nest::VPManager::get_OMP_NUM_THREADS() const
{
  const char* const omp_num_threads = std::getenv( "OMP_NUM_THREADS" );
  if ( omp_num_threads )
  {
    return std::atoi( omp_num_threads );
  }
  else
  {
    return 0;
  }
}

void
nest::VPManager::set_status( const DictionaryDatum& d )
{
  size_t n_threads = get_num_threads();
  size_t n_vps = get_num_virtual_processes();

  bool n_threads_updated = updateValue< long >( d, names::local_num_threads, n_threads );
  bool n_vps_updated = updateValue< long >( d, names::total_num_virtual_procs, n_vps );

  if ( n_vps_updated )
  {
    if ( not n_threads_updated )
    {
      n_threads = n_vps / kernel::manager< MPIManager >.get_num_processes();
    }

    const bool n_threads_conflict = n_vps / kernel::manager< MPIManager >.get_num_processes() != n_threads;
    const bool n_procs_conflict = n_vps % kernel::manager< MPIManager >.get_num_processes() != 0;
    if ( n_threads_conflict or n_procs_conflict )
    {
      throw BadProperty(
        "Requested total_num_virtual_procs is incompatible with the number of processes and threads."
        "It must be an integer multiple of num_processes and equal to "
        "local_num_threads * num_processes. Value unchanged." );
    }
  }

  // We only want to act if new values differ from the old
  n_threads_updated = n_threads != get_num_threads();
  n_vps_updated = n_vps != get_num_virtual_processes();

  if ( n_threads_updated or n_vps_updated )
  {
    std::vector< std::string > errors;
    if ( kernel::manager< NodeManager >.size() > 0 )
    {
      errors.push_back( "Nodes exist" );
    }
    if ( kernel::manager< ConnectionManager >.get_user_set_delay_extrema() )
    {
      errors.push_back( "Delay extrema have been set" );
    }
    if ( kernel::manager< SimulationManager >.has_been_simulated() )
    {
      errors.push_back( "Network has been simulated" );
    }
    if ( kernel::manager< ModelManager >.are_model_defaults_modified() )
    {
      errors.push_back( "Model defaults were modified" );
    }
    if ( kernel::manager< SPManager >.is_structural_plasticity_enabled() and n_threads > 1 )
    {
      errors.push_back( "Structural plasticity enabled: multithreading cannot be enabled" );
    }
    if ( force_singlethreading_ and n_threads > 1 )
    {
      errors.push_back( "This installation of NEST does not support multiple threads" );
    }

    if ( not errors.empty() )
    {
      std::string msg = "Number of threads unchanged. Error conditions:";
      for ( auto& error : errors )
      {
        msg += " " + error + ".";
      }
      throw KernelException( msg );
    }

    if ( get_OMP_NUM_THREADS() > 0 and get_OMP_NUM_THREADS() != n_threads )
    {
      std::string msg = "OMP_NUM_THREADS is set in your environment, but NEST ignores it.\n";
      msg += "For details, see the Guide to parallel computing in the NEST Documentation.";
      LOG( M_WARNING, "VPManager::set_status()", msg );
    }

    kernel::manager< KernelManager >.change_number_of_threads( n_threads );
  }
}

void
nest::VPManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::local_num_threads, get_num_threads() );
  def< long >( d, names::total_num_virtual_procs, get_num_virtual_processes() );
}

void
nest::VPManager::set_num_threads( size_t n_threads )
{
  assert( not( kernel::manager< SPManager >.is_structural_plasticity_enabled() and n_threads > 1 ) );
  n_threads_ = n_threads;

#ifdef _OPENMP
  omp_set_num_threads( n_threads_ );
#endif
}

size_t
nest::VPManager::get_thread_id() const
{
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

size_t
nest::VPManager::get_num_threads() const
{
  return n_threads_;
}

void
nest::VPManager::assert_single_threaded() const
{
#ifdef _OPENMP
  assert( omp_get_num_threads() == 1 );
#endif
}

void
nest::VPManager::assert_thread_parallel() const
{
#ifdef _OPENMP
  // omp_get_num_threads() returns int
  assert( omp_get_num_threads() == static_cast< int >( n_threads_ ) );
#endif
}

size_t
nest::VPManager::get_vp() const
{
  return kernel::manager< MPIManager >.get_rank() + get_thread_id() * kernel::manager< MPIManager >.get_num_processes();
}

size_t
nest::VPManager::node_id_to_vp( const size_t node_id ) const
{
  return node_id % get_num_virtual_processes();
}

size_t
nest::VPManager::vp_to_thread( const size_t vp ) const
{
  return vp / kernel::manager< MPIManager >.get_num_processes();
}

size_t
nest::VPManager::get_num_virtual_processes() const
{
  return get_num_threads() * kernel::manager< MPIManager >.get_num_processes();
}

bool
nest::VPManager::is_local_vp( const size_t vp ) const
{
  return kernel::manager< MPIManager >.get_process_id_of_vp( vp ) == kernel::manager< MPIManager >.get_rank();
}

size_t
nest::VPManager::thread_to_vp( const size_t tid ) const
{
  return tid * kernel::manager< MPIManager >.get_num_processes() + kernel::manager< MPIManager >.get_rank();
}

bool
nest::VPManager::is_node_id_vp_local( const size_t node_id ) const
{
  return ( node_id % get_num_virtual_processes() == static_cast< size_t >( get_vp() ) );
}

size_t
nest::VPManager::node_id_to_lid( const size_t node_id ) const
{
  // starts at lid 0 for node_ids >= 1 (expected value for neurons, excl. node ID 0)
  return std::ceil( static_cast< double >( node_id ) / get_num_virtual_processes() ) - 1;
}

size_t
nest::VPManager::lid_to_node_id( const size_t lid ) const
{
  const size_t vp = get_vp();
  return ( lid + static_cast< size_t >( vp == 0 ) ) * get_num_virtual_processes() + vp;
}

size_t
nest::VPManager::get_num_assigned_ranks_per_thread() const
{
  return std::ceil( static_cast< double >( kernel::manager< MPIManager >.get_num_processes() ) / n_threads_ );
}

size_t
nest::VPManager::get_start_rank_per_thread( const size_t tid ) const
{
  return tid * get_num_assigned_ranks_per_thread();
}

size_t
nest::VPManager::get_end_rank_per_thread( const size_t rank_start, const size_t num_assigned_ranks_per_thread ) const
{
  size_t rank_end = rank_start + num_assigned_ranks_per_thread;

  // if we have more threads than ranks, or if ranks can not be
  // distributed evenly on threads, we need to make sure, that all
  // threads care only about existing ranks
  if ( rank_end > kernel::manager< MPIManager >.get_num_processes() )
  {
    rank_end = std::max( rank_start, kernel::manager< MPIManager >.get_num_processes() );
  }

  return rank_end;
}

nest::AssignedRanks
nest::VPManager::get_assigned_ranks( const size_t tid )
{
  AssignedRanks assigned_ranks;
  assigned_ranks.begin = get_start_rank_per_thread( tid );
  assigned_ranks.max_size = get_num_assigned_ranks_per_thread();
  assigned_ranks.end = get_end_rank_per_thread( assigned_ranks.begin, assigned_ranks.max_size );
  assigned_ranks.size = assigned_ranks.end - assigned_ranks.begin;
  return assigned_ranks;
}
