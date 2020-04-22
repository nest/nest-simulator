/*
 *  recording_backend_mpi.h
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

#ifndef RECORDING_BACKEND_MPI_H
#define RECORDING_BACKEND_MPI_H

#include "recording_backend.h"
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mpi.h>

/* BeginDocumentation

Send data with MPI
##################

When a recording device sends data to the ``mpi`` backend, it sends the
event using MPI. Events are sent with the GID and the time stamp.

Communication Protocol: (value,number,type,source/destination,tag)
+++++++++++++++++++++++
1) Connection of MPI port include in one file ( path+label+id+.txt )
2) Send at each beginning of the run (true, 1, CXX_BOOL, 0, 0)
3) Receive at each ending of the run (true, 1, CXX_BOOL, 0, 0)
4) Send shape of the data of the run (shape, 1,I NT, 0, 0)
5) Send data of the data of the run (data, shape, DOUBLE, 0, 0)
6) Send at each ending of the run (true, 1, CXX_BOOL, 0, 1)
7) Send at this en of the simulation (true, 1, CXX_BOOL, 0, 2)

@author Lionel Kusch and Sandra Diaz
@ingroup NESTio

EndDocumentation */


namespace nest
{

/**
 * A recording backend for sending information with MPI.
 */
class RecordingBackendMPI : public RecordingBackend
{
public:
  RecordingBackendMPI() = default;
  ~RecordingBackendMPI() noexcept override = default;

  void initialize() override;
  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void cleanup() override;

  void prepare() override;

  void write( const RecordingDevice&, const Event&, const std::vector< double >&, const std::vector< long >& ) override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;
  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& params_dictionary ) const override;

private:
  /**
   * Buffer for saving events before to send it.
   * The buffer sa 3 dimensions : thread_id, MPI_communicator_index and number of events
   * elements : id of device, id of neurons, data ( one double )
   */
  std::vector< std::vector< std::vector< std::array< double, 3 > > > > buffer_;
  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its MPI index and device. Only the master thread have the have a valid MPI communicator pointer.
  */
  typedef std::vector< std::map< index, std::tuple< int, MPI_Comm*, const RecordingDevice* > > > device_map;
  device_map devices_;
  /**
   * A map of MPI communicator use by the master thread for the MPI communication.
   * This map contains also the number of the device by MPI communicator.
   */
  typedef std::map< std::string, std::tuple< int, MPI_Comm*, int > > comm_map;
  comm_map commMap_;

  static void get_port( const RecordingDevice* device, std::string* port_name );
  static void get_port( index index_node, const std::string& label, std::string* port_name );
  static void send_data( const MPI_Comm* comm, const double data[], int size);
};

} // namespace

#endif // RECORDING_BACKEND_MPI_H
