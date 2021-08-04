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

/* BeginUserDocs: recording backend

Recording backend `mpi` - Send data with MPI
############################################

.. admonition:: Availability

   This stimulation backend is only available if NEST was compiled with
   :ref:`support for MPI <compile-with-mpi>`.

The `mpi` recording backend sends collected data to a remote process
using MPI.

The name of the MPI port to send data to is read from a file for each
device configured to use this backend. The file needs to be named
according to the following pattern:

::

   {data_path}/{data_prefix}{label}/{node_id}.txt

The ``data_path`` and ``data_prefix`` are global kernel properties,
while `label` is a property of the device in question and `node_id`
its node ID. This path can only be set outside of a `Run` context
(i.e. after ``Prepare()`` has been called, but ``Cleanup()`` has not).

Communication Protocol
++++++++++++++++++++++

The following protocol is used to exchange information between both
MPI processes. The protocol is described using the following format
for the MPI messages: (value, number, type, source/destination, tag)

1) ``Prepare``  : Connection of MPI port included in the port_file (see below)
2) ``Run`` begin: Send at each beginning of the run (true, 1, CXX_BOOL, 0, 0)
3) ``Run`` end  : Receive at each ending of the run (true, 1, CXX_BOOL, 0, 0)
4) ``Run`` end  : Send shape of the data of the run (shape, 1,INT, 0, 0)
5) ``Run`` end  : Send data of the data of the run (data, shape, DOUBLE, 0, 0)
6) ``Run`` end  : Send at each ending of the run (true, 1, CXX_BOOL, 0, 1)
7) ``Cleanup``  : Send at this en of the simulation (true, 1, CXX_BOOL, 0, 2)

Data format
+++++++++++

The format of the data sent is an array consisting of (id device, id node, time
is ms).

EndUserDocs */

namespace nest
{

/**
 * A recording backend for sending information with MPI.
 * Communication protocol diagram:
 * \image html MPI_backend_protocol_of_communication.svg
 * General state machine diagram of NEST communication with the MPI backend:
 * \image html MPI_backend_state_Nest.svg
 * Example of state machine diagram for the communication with NEST:
 * \image html MPI_backend_example_state_machine_communication_with_Nest.svg
 */
class RecordingBackendMPI : public RecordingBackend
{
public:
  RecordingBackendMPI();
  ~RecordingBackendMPI() throw();

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
  bool enrolled_;
  bool prepared_;

  /**
   * Buffer for saving events before they are sent. The buffer has 3
   * dimensions: thread_id, MPI_communicator_index and number of
   * events elements. The events elements are described as an array
   * with three components: id of device, id of neurons and data ( one
   * double )
   */
  std::vector< std::vector< std::vector< std::array< double, 3 > > > > buffer_;

  /**
   * A map for the enrolled devices. We have a vector with one map per
   * local thread. The map associates the node ID of a device on a
   * given thread with its MPI index and device. Only the master
   * thread has a valid MPI communicator pointer.
  */
  typedef std::vector< std::map< index, std::tuple< int, MPI_Comm*, const RecordingDevice* > > > device_map;
  device_map devices_;

  /**
   * A map of MPI communicators used by the master thread for the MPI
   * communication.  The values of the map are tuples containing the
   * index of the MPI communicator, the MPI communicator itself, and
   * the number of devices linked to that MPI communicator.
   */
  typedef std::map< std::string, std::tuple< int, MPI_Comm*, int > > comm_map;
  comm_map commMap_;

  static void get_port( const RecordingDevice* device, std::string* port_name );
  static void get_port( index index_node, const std::string& label, std::string* port_name );
  static void send_data( const MPI_Comm* comm, const double data[], int size );
};

} // namespace

#endif // RECORDING_BACKEND_MPI_H
