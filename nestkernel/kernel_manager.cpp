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
#include "stopwatch_impl.h"

nest::KernelManager* nest::KernelManager::kernel_manager_instance_ = nullptr;


Dictionary
nest::KernelManager::get_build_info_()
{
  // Exit codes
  constexpr unsigned int EXITCODE_UNKNOWN_ERROR = 10;
  constexpr unsigned int EXITCODE_USERABORT = 15;
  constexpr unsigned int EXITCODE_EXCEPTION = 125;
  constexpr unsigned int EXITCODE_SCRIPTERROR = 126;
  constexpr unsigned int EXITCODE_FATAL = 127;

  // The range 200-215 is reserved for test skipping exitcodes. Any new codes must
  // also be added to testsuite/do_tests_sh.in.
  constexpr unsigned int EXITCODE_SKIPPED = 200;
  constexpr unsigned int EXITCODE_SKIPPED_NO_MPI = 201;
  constexpr unsigned int EXITCODE_SKIPPED_HAVE_MPI = 202;
  constexpr unsigned int EXITCODE_SKIPPED_NO_THREADING = 203;
  constexpr unsigned int EXITCODE_SKIPPED_NO_GSL = 204;
  constexpr unsigned int EXITCODE_SKIPPED_NO_MUSIC = 205;

  Dictionary build_info;

  build_info[ "version" ] = std::string( NEST_VERSION );
  build_info[ "exitcode" ] = EXIT_SUCCESS;
  build_info[ "built" ] = std::string( String::compose( "%1 %2", __DATE__, __TIME__ ) );
  build_info[ "datadir" ] = std::string( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DATADIR );
  build_info[ "docdir" ] = std::string( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DOCDIR );
  build_info[ "prefix" ] = std::string( NEST_INSTALL_PREFIX );
  build_info[ "host" ] = std::string( NEST_HOST );
  build_info[ "hostos" ] = std::string( NEST_HOSTOS );
  build_info[ "hostvendor" ] = std::string( NEST_HOSTVENDOR );
  build_info[ "hostcpu" ] = std::string( NEST_HOSTCPU );

#ifdef _OPENMP
  build_info[ "have_threads" ] = true;
  build_info[ "threads_model" ] = std::string( "openmp" );
#else
  build_info[ "have_threads" ] = false;
#endif

#ifdef HAVE_MPI
  build_info[ "have_mpi" ] = true;
  build_info[ "mpiexec" ] = std::string( MPIEXEC );
  build_info[ "mpiexec_numproc_flag" ] = std::string( MPIEXEC_NUMPROC_FLAG );
  build_info[ "mpiexec_max_numprocs" ] = std::string( MPIEXEC_MAX_NUMPROCS );
  build_info[ "mpiexec_preflags" ] = std::string( MPIEXEC_PREFLAGS );
  build_info[ "mpiexec_postflags" ] = std::string( MPIEXEC_POSTFLAGS );
#else
  build_info[ "have_mpi" ] = false;
#endif

#ifdef HAVE_GSL
  build_info[ "have_gsl" ] = true;
#else
  build_info[ "have_gsl" ] = false;
#endif

#ifdef HAVE_BOOST
  build_info[ "have_boost" ] = true;
#else
  build_info[ "have_boost" ] = false;
#endif

#ifdef HAVE_MUSIC
  build_info[ "have_music" ] = true;
#else
  build_info[ "have_music" ] = false;
#endif

#ifdef HAVE_LIBNEUROSIM
  build_info[ "have_libneurosim" ] = true;
#else
  build_info[ "have_libneurosim" ] = false;
#endif

#ifdef HAVE_SIONLIB
  build_info[ "have_sionlib" ] = true;
#else
  build_info[ "have_sionlib" ] = false;
#endif

#ifdef HAVE_HDF5
  build_info[ "have_hdf5" ] = true;
#else
  build_info[ "have_hdf5" ] = false;
#endif

#ifdef NDEBUG
  build_info[ "ndebug" ] = true;
#else
  build_info[ "ndebug" ] = false;
#endif

  Dictionary exitcodes;

  exitcodes[ "success" ] = EXIT_SUCCESS;
  exitcodes[ "skipped" ] = EXITCODE_SKIPPED;
  exitcodes[ "skipped_no_mpi" ] = EXITCODE_SKIPPED_NO_MPI;
  exitcodes[ "skipped_have_mpi" ] = EXITCODE_SKIPPED_HAVE_MPI;
  exitcodes[ "skipped_no_threading" ] = EXITCODE_SKIPPED_NO_THREADING;
  exitcodes[ "skipped_no_gsl" ] = EXITCODE_SKIPPED_NO_GSL;
  exitcodes[ "skipped_no_music" ] = EXITCODE_SKIPPED_NO_MUSIC;
  exitcodes[ "scripterror" ] = EXITCODE_SCRIPTERROR;
  exitcodes[ "abort" ] = NEST_EXITCODE_ABORT;
  exitcodes[ "userabort" ] = EXITCODE_USERABORT;
  exitcodes[ "segfault" ] = NEST_EXITCODE_SEGFAULT;
  exitcodes[ "exception" ] = EXITCODE_EXCEPTION;
  exitcodes[ "fatal" ] = EXITCODE_FATAL;
  exitcodes[ "unknownerror" ] = EXITCODE_UNKNOWN_ERROR;

  build_info[ "test_exitcodes" ] = exitcodes;

  return build_info;
}

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
  assert( false ); // Just to check if this ever gets called
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

  sw_omp_synchronization_construction_.reset();
  sw_omp_synchronization_simulation_.reset();
  sw_mpi_synchronization_.reset();

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

  sw_omp_synchronization_simulation_.reset();
  sw_mpi_synchronization_.reset();
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

  // Prepare timers and set the number of threads for multi-threaded timers
  kernel().simulation_manager.reset_timers_for_preparation();
  kernel().simulation_manager.reset_timers_for_dynamics();
  kernel().event_delivery_manager.reset_timers_for_preparation();
  kernel().event_delivery_manager.reset_timers_for_dynamics();

  sw_omp_synchronization_construction_.reset();
  sw_omp_synchronization_simulation_.reset();
  sw_mpi_synchronization_.reset();
}

void
nest::KernelManager::set_status( const Dictionary& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->set_status( dict );
  }
}

void
nest::KernelManager::get_status( Dictionary& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->get_status( dict );
  }

  dict[ "build_info" ] = get_build_info_();
  if ( NEST_HOSTOS == std::string( "linux" ) )
  {
    dict[ "memory_size" ] = get_memsize_linux_();
  }
  else if ( NEST_HOSTOS == std::string( "darwin" ) )
  {
    dict[ "memory_size" ] = get_memsize_darwin_();
  }
  else
  {
    // Not available for this OS.
    dict[ "memory_size" ] = -1;
  }

  sw_omp_synchronization_construction_.get_status(
    dict, names::time_omp_synchronization_construction, names::time_omp_synchronization_construction_cpu );
  sw_omp_synchronization_simulation_.get_status(
    dict, names::time_omp_synchronization_simulation, names::time_omp_synchronization_simulation_cpu );
  sw_mpi_synchronization_.get_status( dict, names::time_mpi_synchronization, names::time_mpi_synchronization_cpu );
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

#ifdef __linux__

#include <fstream>
#include <sstream>
size_t
nest::KernelManager::get_memsize_linux_() const
{
  // code based on mistral.ai
  std::ifstream file( "/proc/self/status" );
  if ( not file.is_open() )
  {
    throw std::runtime_error( "Could not open /proc/self/status" );
  }

  std::string line;
  while ( std::getline( file, line ) )
  {
    if ( line.rfind( "VmSize:", 0 ) == 0 )
    {
      std::istringstream stream( line );
      std::string key;
      size_t value;
      std::string unit;
      stream >> key >> value >> unit;
      file.close();
      if ( unit != "kB" )
      {
        throw std::runtime_error( "VmSize not reported in kB" );
      }
      return value;
    }
  }

  file.close();
  throw std::runtime_error( "VmSize not found in /proc/self/status" );
}

#else

size_t
nest::KernelManager::get_memsize_linux_() const
{
  assert( false || "Only implemented on Linux systems." );
  return 0;
}

#endif


#if defined __APPLE__

#include <mach/mach.h>
size_t
nest::KernelManager::get_memsize_darwin_() const
{
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

  kern_return_t result = task_info( mach_task_self(), TASK_BASIC_INFO, ( task_info_t ) &t_info, &t_info_count );
  assert( result == KERN_SUCCESS || "Problem occurred during getting of task_info." );

  // For macOS, vmsize is not informative, it is an extremly large address range, usually O(2^40).
  // resident_size gives the most reasonable information. Information is in bytes, thus divide.
  return t_info.resident_size / 1024;
}

#else

size_t
nest::KernelManager::get_memsize_darwin_() const
{
  assert( false || "Only implemented on macOS." );
  return 0;
}

#endif
