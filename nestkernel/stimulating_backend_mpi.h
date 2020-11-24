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
#include "stimulating_device.h"
#include "nest_types.h"
#include "nest_time.h"
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mpi.h>

/* BeginDocumentation

Read data from MPI
##################

When an input device is to be updated at the beginning of each step run, the 'mpi' backend
communicates with an external source via MPI to get the update.

Communication Protocol: (value,number,type,source/destination,tag)
+++++++++++++++++++++++
1) Connection of MPI port include in one file ( path+label+id+.txt )
2) Send at each beginning run of Nest (true, 1, CXX_BOOL, 0, 0)
3) Send the id of the device to update (id_device, 1, INT, 0, 0)
3) Receive shape of the data (shape, 1, INT, 0, 0)
4) Receive the data for updating the device (data, shape, DOUBLE, 0, 0)
5) Send at each ending of the run (true, 1, CXX_BOOL, 0, 1)
6) Send at this en of the simulation (true, 1, CXX_BOOL, 0, 2)


@author Lionel Kusch and Sandra Diaz
@ingroup NESTio

EndDocumentation */

namespace nest
{

// template < typename EmittedEvent >
// class StimulatingDevice;
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
  ~StimulatingBackendMPI() noexcept override = default;

  void initialize() override;

  void finalize() override;

  void enroll( StimulatingDevice< EmittedEvent >& device, const DictionaryDatum& params );

  void disenroll( const StimulatingDevice< EmittedEvent >& device );

  void cleanup() override;

  void prepare() override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;

  void set_value_names( const StimulatingDevice< EmittedEvent >& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  void get_device_defaults( DictionaryDatum& ) const override;

  void get_device_status( const StimulatingDevice< EmittedEvent >& device, DictionaryDatum& params_dictionary ) const;

private:
  bool enrolled_;
  bool prepared_;
  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with it's device. Only the master thread have a valid MPI communicator pointer.
  */
  typedef std::vector< std::map< index, std::pair< const MPI_Comm*, StimulatingDevice< EmittedEvent >* > > > device_map;
  device_map devices_;
  /**
   * A map of MPI communicator use by the master thread for the MPI communication.
   * This map contains also the number of the device by MPI communicator.
   */
  typedef std::map< std::string, std::tuple< MPI_Comm*, std::vector< int >*, int* > > comm_map;
  comm_map commMap_;

  /**
   *  Getting the port name for the MPI connection
   * @param device : input device for finding the file with the port
   * @param port_name : result of the port name
   */
  static void get_port( StimulatingDevice< EmittedEvent >* device, std::string* port_name );
  static void get_port( index index_node, const std::string& label, std::string* port_name );
  /**
   * MPI communication for receiving the data before each run. This function is use only by the master thread.
   * The allocation of this function is clean by the function clean_memory_input_data
   * @param comm : the MPI communicator for send and receive message
   * @param device_id : the list of ID which need to be updated
   * @return pair( size of data by device, the continuous array with all the data for the device )
   */
  static std::pair< int*, double* > receive_spike_train( const MPI_Comm& comm, std::vector< int >& device_id );
  /**
   * Update all the device with the data receiving
   * @param array_index : number of device by thread
   * @param devices_id : the devices id order by thread
   * @param data : the data receiving for update all the device
   */
  void update_device( int* array_index, std::vector< int >& devices_id, std::pair< int*, double* > data );
  /**
   * clean all the memory allocated for the updating device. The function is use only by the master thread
   * @param data
   */
  void clean_memory_input_data( std::pair< int*, double* >* data );
};

} // namespace

#endif // STIMULATING_BACKEND_MPI_H
