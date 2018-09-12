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

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_arbor.h"

nest::RecordingBackendArbor::RecordingBackendArbor()
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
  const auto t = device.get_thread();
  const auto gid = device.get_gid();

  auto device_it = devices_[ t ].find( gid );
  if ( device_it  != devices_[ t ].end() )
  {
    devices_[ t ].erase( device_it );
  }
  
  devices_[ t ].insert( std::make_pair( gid, &device ) );
}

void
nest::RecordingBackendArbor::initialize()
{
  device_map devices( kernel().vp_manager.get_num_threads() );
  devices_.swap( devices );
}

void
nest::RecordingBackendArbor::finalize()
{
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
    ArborSpikes sbuf;
    for (auto buf: buffers_) {
      auto& spikes = buf.get_spikes();
      sbuf.insert(sbuf.end(), spikes.begin(), spikes.end());
      buf.clear();
    }
    transmit_(sbuf);
  }  
}

#include <numeric>

void
nest::RecordingBackendArbor::transmit_(ArborSpikes& sbuf)
{
  // ArborSpikes should pack without padding
  static_assert((sizeof(ArborSpike) % alignof(ArborSpike)) == 0);

  int sbuf_length = sbuf.size()*sizeof(sbuf[0]);
  
  int global_size;
  int global_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &global_size);

  std::vector< int > rbuf_lengths(global_size);
  // send and receive: receive length is the length of every send (??)
  MPI_Allgather(&sbuf_length,     1, MPI_INT,
		&rbuf_lengths[0], 1, MPI_INT,
		MPI_COMM_WORLD);

  auto total_chars = std::accumulate(rbuf_lengths.begin(),
				     rbuf_lengths.end(),
				     0);
  ArborSpikes rbuf(total_chars/sizeof(ArborSpike));
  
  std::vector< int > rbuf_offset;
  rbuf_offset.reserve(global_size);
  for (int i = 0, c_offset = 0; i < global_size; i++) {
    const auto c_size = rbuf_lengths[i];
    rbuf_offset.push_back(c_offset);
    c_offset += c_size;
  }

  // spikes
  MPI_Allgatherv(
    &sbuf[0], sbuf_length, MPI_CHAR,
    &rbuf[0], &rbuf_lengths[0], &rbuf_offset[0], MPI_CHAR,
    MPI_COMM_WORLD);
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

  ArborBuffer& buffer = buffers_[ t ];
 
  const auto sender_gid = event.get_sender_gid();
  const auto step_time = event.get_stamp().get_ms();
  const auto offset = event.get_offset();
  const auto time = static_cast<float>(step_time-offset);

  buffer.write(sender_gid, time);  
}

void
nest::RecordingBackendArbor::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  throw UnsupportedEvent();
}

/* ----------------------------------------------------------------
 * Buffer
 * ---------------------------------------------------------------- */


void
nest::RecordingBackendArbor::ArborBuffer::write(index gid, float time)
{
  spikes_.push_back({gid, time});
}

void
nest::RecordingBackendArbor::ArborBuffer::clear()
{
  spikes_.clear();
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
