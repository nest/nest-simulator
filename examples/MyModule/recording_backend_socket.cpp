/*
 *  recording_backend_socket.cpp
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

// Includes from C++
#include <sstream>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

#include "recording_backend_socket.h"

nest::RecordingBackendSocket::RecordingBackendSocket()
{
}

nest::RecordingBackendSocket::~RecordingBackendSocket() throw()
{
  finalize();
}

void
nest::RecordingBackendSocket::enroll( const RecordingDevice& device )
{
}

void
nest::RecordingBackendSocket::enroll( const RecordingDevice& device,
  const std::vector< Name >& /* value_names */ )
{
}

void
nest::RecordingBackendSocket::initialize()
{
}

void
nest::RecordingBackendSocket::post_run_cleanup()
{
}

void
nest::RecordingBackendSocket::finalize()
{
}

void
nest::RecordingBackendSocket::synchronize()
{
}

void
nest::RecordingBackendSocket::write( const RecordingDevice& device,
  const Event& event )
{
#pragma omp critical
  {
      addr_.sin_family = AF_INET;
      inet_aton("127.0.0.1", &addr_.sin_addr);
      addr_.sin_port = htons(50000);

      
    int s = socket(PF_INET, SOCK_DGRAM, 0);
    std::ostringstream msg;
    msg << "spike_detector " << device.get_gid()
	<< " got a spike by node " << event.get_sender_gid()
	<< std::endl;
    
    int x = sendto(s, msg.str().c_str(), msg.str().size(), 0, (struct sockaddr *)&addr_, sizeof(addr_));

    
    std::cout << msg.str() << x <<  std::endl;
  }
}

void
nest::RecordingBackendSocket::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
}

void
nest::RecordingBackendSocket::set_status( const DictionaryDatum& d )
{
}
