/*
 *  stimulating_backend_mpi.h
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

#ifndef STIMULATING_BACKEND_MPI_H
#define STIMULATING_BACKEND_MPI_H

#include "stimulating_backend.h"
#include "nest_types.h"
#include "nest_time.h"
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mpi.h>

/* BeginUserDocs: stimulating backend

.. _stimulating_backend_mpi:

Collect data from MPI communication for updating device
#######################################################

.. admonition:: Availability

   This stimulating backend is only available if NEST was compiled with
   :ref:`support for MPI <compile-with-mpi>`.

The `mpi` stimulating backend collects data from MPI channels and
updates stimulating device just before each run. This is useful for
co-simulation or for receiving data from an external software.

The creation of this MPI communication is based on the functions
'MPI_Comm_connect' and 'MPI_Comm_disconnect'. The port name is 
read from a file for each device with this backend.
The name of the file needs to be specified according to the following 
pattern:

::

   data_path/label/id_device.txt

The properties ``data_path`` and ``data_prefix`` are global kernel properties.
This path is only used during the ``Prepare`` call. There is not
functionality for changing connection during the ``Run``.
If you want to change the connections during the simulation, you need first 
to close the previous one by using calling ``Cleanup`` of the simulator.


Communication Protocol
++++++++++++++++++++++
The following protocol is used to exchange information between
both MPI processes. The protocol is described using the
following format for the MPI messages:
(value, number, type, source/destination, tag)

1) ``Prepare``  : Connection of MPI port include in one file (see below)
2) ``Run`` begin: Send start run (true, 1, CXX_BOOL, 0, 0)
3) ``Run`` begin: Send the id of the device to update (id_device, 1, INT, 0, 0)
3) ``Run`` begin: Receive shape of the data (shape, 1, INT, 0, 0)
4) ``Run`` begin: Receive the data for updating the device (data, shape, DOUBLE, 0, 0)
5) ``Run`` end  : Send at each ending of the run (true, 1, CXX_BOOL, 0, 1)
6) ``Cleanup``  : Send at this en of the simulation (true, 1, CXX_BOOL, 0, 2)

Data format
+++++++++++

The format of the data is depend of the stimulating device. 

Parameter summary
+++++++++++++++++
.. glossary::

 label
   Shared file with ports with the format path+label+id+.txt
   Where path = kernel().io_manager.get_data_path() and the
   label and id are the ones provided for the device.

@author Lionel Kusch and Sandra Diaz
@ingroup NESTio

EndUserDocs */

namespace nest
{


/**
 * A simple input backend MPI implementation
 */
class StimulatingBackendMPI : public StimulatingBackend
{
public:
  /**
   * InputBackend constructor
   * The actual initialization is happening in InputBackend::initialize()
   */
  StimulatingBackendMPI();

  /**
   * InputBackend destructor
   * The actual finalization is happening in InputBackend::finalize()
   */
  ~StimulatingBackendMPI() noexcept override;

  void initialize() override;

  void finalize() override;

  void enroll( StimulatingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( StimulatingDevice& device ) override;

  void cleanup() override;

  void prepare() override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

private:
  bool enrolled_;
  bool prepared_;
  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the node ID of a device on a given thread
   * with its device. Only the master thread has a valid MPI communicator pointer.
  */
  using device_map = std::vector< std::map< index, std::pair< const MPI_Comm*, StimulatingDevice* > > >;
  device_map devices_;
  /**
   * A map of MPI communicators used by the master thread for the MPI communication.
   * This map contains also the number of the devices linked to each MPI communicator.
   */
  typedef std::map< std::string, std::tuple< MPI_Comm*, std::vector< int >*, int* > > comm_map;
  comm_map commMap_;

  /**
   *  Getting the port name for the MPI connection
   * @param device : input device for finding the file with the port
   * @param port_name : result of the port name
   */
  static void get_port( StimulatingDevice* device, std::string* port_name );
  static void get_port( index index_node, const std::string& label, std::string* port_name );
  /**
   * MPI communication for receiving the data before each run. This function is used only by the master thread.
   * The allocation of this function is cleaned by the function clean_memory_input_data
   * @param comm : the MPI communicator for send and receive message
   * @param device_id : the list of IDs which needs to be updated
   * @return pair( size of data by device, the continuous array with all the data for the device )
   */
  static std::pair< int*, double* > receive_spike_train( const MPI_Comm& comm, std::vector< int >& device_id );
  /**
   * Update all the devices with the data received
   * @param array_index : number of devices by thread
   * @param devices_id : the devices' ID ordered by thread
   * @param data : the data received for updating all the devices
   */
  void update_device( int* array_index, std::vector< int >& devices_id, std::pair< int*, double* > data );
  /**
   * clean all the memory allocated for the updating device. The function is used only by the master thread
   * @param data
   */
  void clean_memory_input_data( std::pair< int*, double* >* data );
};

} // namespace

#endif // STIMULATING_BACKEND_MPI_H
