/*
 *  recording_backend_arbor.cpp
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

// C includes:
#include <mpi.h>
#include <memory>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_arbor.h"
#include "mpiutil_impl.h"
#include "exceptions.h"

// this is here to hide the arbor internal header info from nest
struct nest::ArborInternal
{
  arb::shadow::comm_info info;
  std::vector< std::vector< arb::shadow::spike > > spike_buffers;

  ArborInternal() = default;
  ArborInternal( const ArborInternal& ) = delete;
  ArborInternal& operator=( const ArborInternal& ) = delete;

  ArborInternal( ArborInternal&& ) = default;
  ArborInternal& operator=( ArborInternal&& ) = default;
};

// because we're only using C++11, but we don't want to pollute std::
// https://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
// https://isocpp.org/files/papers/N3656.txt
namespace nest
{
template < typename T, typename... Args >
std::unique_ptr< T >
make_unique( Args&&... args )
{
  return std::unique_ptr< T >( new T( std::forward< Args >( args )... ) );
}
}

nest::RecordingBackendArbor::RecordingBackendArbor()
  : enrolled_( false )
  , prepared_( false )
  , steps_left_( 0 )
  , arbor_steps_( 0 )
  , num_arbor_cells_( 0 )
  , arbor_( nest::make_unique< ArborInternal >() )
{
}

nest::RecordingBackendArbor::~RecordingBackendArbor() throw()
{
  cleanup();
}

void
nest::RecordingBackendArbor::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  device_map devices( nthreads );
  devices_.swap( devices );
  arbor_->spike_buffers.resize( nthreads );
}

void
nest::RecordingBackendArbor::finalize()
{
}

void
nest::RecordingBackendArbor::enroll( const RecordingDevice& device, const DictionaryDatum& )
{
  if ( device.get_type() == RecordingDevice::SPIKE_DETECTOR )
  {
    const auto tid = device.get_thread();
    const auto gid = device.get_gid();

    auto device_it = devices_[ tid ].find( gid );
    if ( device_it != devices_[ tid ].end() )
    {
      devices_[ tid ].erase( device_it );
    }

    devices_[ tid ].insert( std::make_pair( gid, &device ) );
  }
  else
  {
    throw BadProperty( "Only spike detectors can record to recording backend 'arbor'." );
  }
}

void
nest::RecordingBackendArbor::disenroll( const RecordingDevice& device )
{
  const auto tid = device.get_thread();
  const auto gid = device.get_gid();

  auto device_it = devices_[ tid ].find( gid );
  if ( device_it != devices_[ tid ].end() )
  {
    devices_[ tid ].erase( device_it );
  }
}

void
nest::RecordingBackendArbor::set_value_names( const RecordingDevice&,
  const std::vector< Name >&,
  const std::vector< Name >& )
{
  // nothing to do
}

void
nest::RecordingBackendArbor::pre_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendArbor::cleanup()
{
  if ( prepared_ )
  {
    if ( not enrolled_ )
    {
      return;
    }

    if ( not prepared_ )
    {
      throw BackendNotPrepared( "RecordingBackendArbor" );
    }
    prepared_ = false;

    if ( steps_left_ != 0 )
    {
      throw UnmatchedSteps( steps_left_, arbor_steps_ );
    }

    MPI_Comm_free( &arbor_->info.comm );
  }
}

void
nest::RecordingBackendArbor::exchange_( std::vector< arb::shadow::spike >& local_spikes )
{
  // static int step = 0;
  // std::cerr << "NEST: n: " << step++ << std::endl;
  // std::cerr << "NEST: Output spikes" << std::endl;
  arb::shadow::gather_spikes( local_spikes, MPI_COMM_WORLD );
  // std::cerr << "NEST: Output spikes done: " << steps_left_ << std::endl;
  steps_left_--;
}

void
nest::RecordingBackendArbor::prepare()
{
  if ( not enrolled_ )
  {
    return;
  }

  if ( prepared_ )
  {
    throw BackendPrepared( "RecordingBackendArbor" );
  }
  prepared_ = true;

  //  INITIALISE MPI
  arbor_->info = arb::shadow::get_comm_info( false, kernel().mpi_manager.get_communicator() );

  //  MODEL SETUP
  DictionaryDatum dict_out( new Dictionary );
  kernel().get_status( dict_out );
  const float nest_min_delay = ( *dict_out )[ "min_delay" ];
  const int num_nest_cells = ( long ) ( *dict_out )[ "network_size" ];

  // HAND SHAKE ARBOR-NEST
  // hand shake #1: communicate cell populations
  num_arbor_cells_ = arb::shadow::broadcast( 0, MPI_COMM_WORLD, arbor_->info.arbor_root );
  arb::shadow::broadcast( num_nest_cells, MPI_COMM_WORLD, arbor_->info.nest_root );

  // hand shake #2: communications step size synchronized
  const float arb_comm_time = arb::shadow::broadcast( 0.f, MPI_COMM_WORLD, arbor_->info.arbor_root );
  const float nest_comm_time = nest_min_delay;
  arb::shadow::broadcast( nest_comm_time, MPI_COMM_WORLD, arbor_->info.nest_root );
  const float min_delay = std::min( nest_comm_time, arb_comm_time );

  // hand shake #3: steps
  steps_left_ = arbor_steps_ =
    arb::shadow::broadcast( 0u, MPI_COMM_WORLD, arbor_->info.arbor_root ) + 1; // arbor has a pre-exchange

  // set min_delay
  DictionaryDatum dict_in( new Dictionary );
  ( *dict_in )[ "min_delay" ] = min_delay;
  ( *dict_in )[ "max_delay" ] = ( *dict_out )[ "max_delay" ];
  kernel().set_status( dict_in );

  // Arbor has an initial exchange before time 0
  std::vector< arb::shadow::spike > empty_spikes;
  exchange_( empty_spikes );
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >&,
  const std::vector< long >& )
{
  const thread t = device.get_thread();
  const auto device_gid = device.get_gid();

  if ( devices_[ t ].find( device_gid ) == devices_[ t ].end() )
  {
    return;
  }

  auto& buffer = arbor_->spike_buffers[ t ];

  const unsigned sender_gid = event.get_sender_gid();
  const auto step_time = event.get_stamp().get_ms();
  const auto offset = event.get_offset();
  const auto time = static_cast< float >( step_time - offset );

  buffer.push_back( { { num_arbor_cells_ + sender_gid, 0 }, time } );
}

void
nest::RecordingBackendArbor::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

void
nest::RecordingBackendArbor::post_run_hook()
{
#pragma omp single
  {
    auto& buffers = arbor_->spike_buffers;

    std::vector< arb::shadow::spike > local_spikes;
    std::size_t size = 0;
    for ( const auto& spikes : buffers )
    {
      size += spikes.size();
    }
    local_spikes.reserve( size );

    for ( auto& spikes : buffers )
    {
      local_spikes.insert( local_spikes.end(), spikes.begin(), spikes.end() );
      spikes.clear();
    }

    exchange_( local_spikes );
  }
}

void
nest::RecordingBackendArbor::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendArbor::check_device_status( const DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::RecordingBackendArbor::get_device_defaults( DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::RecordingBackendArbor::get_device_status( const nest::RecordingDevice& device,
  DictionaryDatum& params_dictionary ) const
{
  // nothing to do
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendArbor::Parameters_::Parameters_()
{
}

void
nest::RecordingBackendArbor::Parameters_::get( const RecordingBackendArbor& al, DictionaryDatum& d ) const
{
}

void
nest::RecordingBackendArbor::Parameters_::set( const RecordingBackendArbor& al, const DictionaryDatum& d )
{
}

void
nest::RecordingBackendArbor::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
