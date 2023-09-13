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
#include "kernel_manager.h"
#include "mpi_manager.h"
#include "mpi_manager_impl.h"
#include "vp_manager_impl.h"

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
nest::VPManager::initialize()
{
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
  set_num_threads( 1 );
}

void
nest::VPManager::finalize()
{
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
      n_threads = n_vps / kernel().mpi_manager.get_num_processes();
    }

    const bool n_threads_conflict = n_vps / kernel().mpi_manager.get_num_processes() != n_threads;
    const bool n_procs_conflict = n_vps % kernel().mpi_manager.get_num_processes() != 0;
    if ( n_threads_conflict or n_procs_conflict )
    {
      throw BadProperty(
        "Requested total_num_virtual_procs is incompatible with the number of processes and threads."
        "It must be an integer multiple of num_processes and equal to "
        "local_num_threads * num_processes. Value unchanged." );
    }
  }

  if ( force_singlethreading_ and n_threads > 1 )
  {
    throw BadProperty( "This installation of NEST was built without support for multiple threads." );
  }

  // We only want to act if new values differ from the old
  n_threads_updated = n_threads != get_num_threads();
  n_vps_updated = n_vps != get_num_virtual_processes();

  if ( n_threads_updated or n_vps_updated )
  {
    if ( kernel().sp_manager.is_structural_plasticity_enabled() and n_threads > 1 )
    {
      throw KernelException( "Structural plasticity enabled: multithreading cannot be enabled." );
    }

    std::vector< std::string > errors;
    if ( kernel().node_manager.size() > 0 )
    {
      errors.push_back( "Nodes exist" );
    }
    if ( kernel().connection_manager.get_user_set_delay_extrema() )
    {
      errors.push_back( "Delay extrema have been set" );
    }
    if ( kernel().simulation_manager.has_been_simulated() )
    {
      errors.push_back( "Network has been simulated" );
    }
    if ( kernel().model_manager.are_model_defaults_modified() )
    {
      errors.push_back( "Model defaults were modified" );
    }

    if ( errors.size() == 1 )
    {
      throw KernelException( errors[ 0 ] + ": number of threads cannot be changed." );
    }
    if ( errors.size() > 1 )
    {
      std::string msg = "Number of threads unchanged. Error conditions:";
      for ( auto& error : errors )
      {
        msg += " " + error + ".";
      }
      throw KernelException( msg );
    }

    kernel().change_number_of_threads( n_threads );
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
  if ( kernel().sp_manager.is_structural_plasticity_enabled() and n_threads > 1 )
  {
    throw KernelException( "Multiple threads can not be used if structural plasticity is enabled" );
  }

  char* omp_num_threads = std::getenv( "OMP_NUM_THREADS" );
  if ( omp_num_threads and static_cast< size_t >( std::atoi( omp_num_threads ) ) != n_threads )
  {
    const std::string tstr = ( n_threads > 1 ) ? "threads" : "thread";
    const int ONT = std::atoi( omp_num_threads );
    std::string msg = "The new number of threads disagrees with the environment variable OMP_NUM_THREADS.\n";
    msg += "NEST only respects the kernel attributes /local_num_threads or /total_num_virtual_procs\n";
    msg += String::compose( "and will now use %1 %2 and ignore OMP_NUM_THREADS (set to %3).", n_threads, tstr, ONT );

    LOG( M_WARNING, "MPIManager::init_mpi()", msg );
  }

  n_threads_ = n_threads;

#ifdef _OPENMP
  omp_set_num_threads( n_threads_ );
#endif
}

void
nest::VPManager::assert_single_threaded()
{
#ifdef _OPENMP
  assert( omp_get_num_threads() == 1 );
#endif
}
