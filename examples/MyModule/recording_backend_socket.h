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
 * Recorded data is sent via UDP to a server. A minimal receiving
 * server in Python looks like this:
 *
 * import socket
 * s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
 * s.bind(('', 50000)) # '' means all available interfaces
 * while True:
 *     print s.recv(1024)
 *
 * RecordingBackendSocket only works for spike data. It uses a single
 * socket connection to send the data of all recording devices. In
 * order to not open the socket if no device is connected, the socket
 * is opened upon the first call to enroll() (issued by the recorder's
 * calibrate() function) and closed in finalize(), which is called on
 * all registered recording backends by IOManager::cleanup().
 */

class RecordingBackendSocket : public RecordingBackend
{
public:
  RecordingBackendSocket();

  ~RecordingBackendSocket() throw();

  /**
   * Function called by spike detectors using this recording
   * backend. This function opens the socket.
   */
  void enroll( const RecordingDevice&, // device
    const std::vector< Name >&,        // double value names
    const std::vector< Name >& );      // long value names

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
  void write( const RecordingDevice&, // device
    const Event&,                     // event
    const std::vector< double >&,     // double values
    const std::vector< long >& );     // long values

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  /**
   * Initialize the RecordingBackendSocket during simulation preparation.
   */
  void initialize();

private:
  struct Parameters_
  {
    std::string ip_; //!< The IP address the socket binds to
    long port_;      //!< The port the socket binds to

    Parameters_();

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  struct Buffers_
  {
    struct sockaddr_in addr_;
    int socket_;
  };

  Parameters_ P_;
  Buffers_ B_;
};

inline void
RecordingBackendSocket::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
}

} // namespace

#endif // RECORDING_BACKEND_SOCKET_H
