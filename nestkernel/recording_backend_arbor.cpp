/*
 *  recording_backend_sionlib.cpp
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
#include "mpiutil.hpp"
#include "exceptions.h"

struct nest::ArborInternal {
  comm_info info;
  std::vector< std::vector< arb::spike > > spike_buffers;

  ArborInternal() = default;
  ArborInternal(const ArborInternal&) = delete;
  ArborInternal& operator = (const ArborInternal&) = delete;
  
  ArborInternal(ArborInternal&&) = default;
  ArborInternal& operator = (ArborInternal&&) = default;
};

nest::RecordingBackendArbor::RecordingBackendArbor()
  : prepared_(false)
  , cleanedup_(false)
  , steps_left_(0)
  , arbor_steps_(0)
  , num_arbor_cells_(0)
  , arbor_(new ArborInternal)

{
}

nest::RecordingBackendArbor::~RecordingBackendArbor() throw()
{
  finalize();
}

void
nest::RecordingBackendArbor::enroll( const RecordingDevice& device,
  const std::vector< Name >& value_names )
{
  throw UnsupportedEvent();
}

void
nest::RecordingBackendArbor::enroll( const RecordingDevice& device )
{
  const auto tid = device.get_thread();
  const auto gid = device.get_gid();

  auto device_it = devices_[ tid ].find( gid );
  if ( device_it  != devices_[ tid ].end() )
  {
    devices_[ tid ].erase( device_it );
  }
  
  devices_[ tid ].insert( std::make_pair( gid, &device ) );
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
    if (! cleanedup_ )
    {
        cleanup();
    }
}

void
nest::RecordingBackendArbor::prepare()
{
    if ( prepared_ ) {
      throw BackendPrepared( "RecordingBackendArbor" );
    }
    prepared_ = true;
    
    //  INITIALISE MPI
    arbor_->info = get_comm_info(false);

    //  MODEL SETUP
    DictionaryDatum dict_out;
    kernel().get_status(dict_out);
    const float nest_min_delay = (*dict_out)["min_delay"];
    const int num_nest_cells = (*dict_out)["network_size"];
    
    // HAND SHAKE ARBOR-NEST
    // hand shake #1: communicate cell populations
    num_arbor_cells_ = broadcast(0, MPI_COMM_WORLD, arbor_->info.arbor_root);
    broadcast(num_nest_cells, MPI_COMM_WORLD, arbor_->info.nest_root);

    // hand shake #2: min delay
    const float arb_comm_time = broadcast(0.f, MPI_COMM_WORLD, arbor_->info.arbor_root);
    const float nest_comm_time = nest_min_delay;
    broadcast(nest_comm_time, MPI_COMM_WORLD, arbor_->info.nest_root);
    const float min_delay = std::min(nest_comm_time, arb_comm_time);

    // hand shake #3: steps
    steps_left_ = arbor_steps_
        = broadcast(0u, MPI_COMM_WORLD, arbor_->info.arbor_root);

    DictionaryDatum dict_in;
    (*dict_in)["min_delay"] = min_delay;
    kernel().set_status(dict_in);
}

void
nest::RecordingBackendArbor::cleanup()
{
    if ( cleanedup_ ) {
        throw BackendCleanedUp( "RecordingBackendArbor" );
    }
    cleanedup_ = true;
    
    if (steps_left_ != 0)
    {
        throw UnmatchedSteps( steps_left_, arbor_steps_ );
    }

    MPI_Comm_free( &arbor_->info.comm );
}

void
nest::RecordingBackendArbor::synchronize()
{
  if (devices_.size() == 0)
  {
    return;
  }

#pragma omp single
  {
    auto& buffers = arbor_->spike_buffers;

    std::vector<arb::spike> local_spikes;
    std::size_t size = 0;
    for (const auto& spikes: buffers)
    {
        size += spikes.size();
    }    
    local_spikes.reserve(size);
    
    for (auto& spikes: buffers)
    {
      local_spikes.insert(local_spikes.end(), spikes.begin(), spikes.end());
      spikes.clear();
    }
    
    gather_spikes(local_spikes, arbor_->info.comm);
  }
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event )
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
  const auto time = static_cast<float>(step_time-offset);

  buffer.push_back({{num_arbor_cells_ + sender_gid, 0}, time});
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  throw UnsupportedEvent();
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendArbor::Parameters_::Parameters_()
{
}

void
nest::RecordingBackendArbor::Parameters_::get(
  const RecordingBackendArbor& al,
  DictionaryDatum& d ) const
{
}

void
nest::RecordingBackendArbor::Parameters_::set(
  const RecordingBackendArbor& al,
  const DictionaryDatum& d )
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
