/*
 *  recording_backend_socket.h
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

#ifndef RECORDING_BACKEND_SOCKET_H
#define RECORDING_BACKEND_SOCKET_H

// Includes for network access:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "recording_backend.h"

namespace nest
{

/**
 * Socket specialization of the RecordingBackend interface.
 *
 * Recorded data is sent via UDP to server. A minimal server in Python
 * looks like this:
 *
 * import socket
 * s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
 * s.bind(("localhost", 50000))
 * while True:
 *     print s.recvfrom(1024)
 *
 * RecordingBackendSocket maintains a data structure mapping a socket
 * connection to a specific IP address and port to every recording
 * device instance on every thread. Socket connections are opened and
 * inserted into the map during the enroll() call (issued by the
 * recorder's calibrate() function) and closed in finalize(), which is
 * called on all registered recording backends by
 * IOManager::cleanup().
 */
class RecordingBackendSocket : public RecordingBackend
{
public:
  RecordingBackendSocket();

   ~RecordingBackendSocket() throw();

  /**
   * Functions called by all instantiated recording devices to register
   * themselves with their
   * metadata. Here, files are opened.
   */
  void enroll( const RecordingDevice& device );
  void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names );

  /**
   * Flush files after a single call to Run
   */
  void post_run_cleanup();

  /**
   * Finalize the RecordingBackendSocket after the simulation has finished.
   */
  void finalize();

  /**
   * Trivial synchronization function. The RecordingBackendSocket does
   * not need explicit synchronization after each time step.
   */
  void synchronize();

  /**
   * Functions to write data to file.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  /**
   * Initialize the RecordingBackendSocket during simulation preparation.
   */
  void initialize();

private:

  struct sockaddr_in addr_;
};

inline void
RecordingBackendSocket::get_status( DictionaryDatum& d ) const
{
//  ( *d )[ "IP" ] = addr_.sin_addr;
//  ( *d )[ "Port" ] = addr_.sin_port;
}

} // namespace

#endif // RECORDING_BACKEND_SOCKET_H
