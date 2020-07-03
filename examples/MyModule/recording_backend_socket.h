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

  void initialize() override;

  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void write( const RecordingDevice&, const Event&, const std::vector< double >&, const std::vector< long >& ) override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void check_device_status( const DictionaryDatum& ) const override;

  void get_device_defaults( DictionaryDatum& ) const override;

  void get_device_status( const RecordingDevice&, DictionaryDatum& ) const override;

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

} // namespace

#endif // RECORDING_BACKEND_SOCKET_H
