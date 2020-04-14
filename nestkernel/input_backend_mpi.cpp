/*
 *  input_backend_mpi.cpp
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

// C++ includes:
#include <iostream>


// Includes from nestkernel:
#include "input_backend_mpi.h"
#include "input_device.h"


void
nest::InputBackendMPI::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  device_map devices( nthreads );
  devices_.swap( devices );
  comm_map comms( nthreads );
  commMap_.swap( comms );
}

void
nest::InputBackendMPI::finalize()
{
  // clear vector of map
  for ( auto& it_device : devices_ )
  {
    it_device.clear();
  }
  for ( auto& it_comm : commMap_ )
  {
    it_comm.clear();
  }
  devices_.clear();
  commMap_.clear();
}

void
nest::InputBackendMPI::enroll( InputDevice& device, const DictionaryDatum& params )
{
  if ( device.get_type() == InputDevice::SPIKE_GENERATOR or device.get_type() == InputDevice::STEP_CURRENT_GENERATOR )
  {

    thread tid = device.get_thread();
    index node_id = device.get_node_id();

    auto device_it = devices_[ tid ].find( node_id );
    if ( device_it != devices_[ tid ].end() )
    {
      devices_[ tid ].erase( device_it );
    }
    std::pair< MPI_Comm*, InputDevice* > pair = std::make_pair( nullptr, &device );
    devices_[ tid ].insert( std::make_pair( node_id, pair ) );
  }
  else
  {
    throw BadProperty( "Only spike generators can have input backend 'mpi'." );
  }
}

void
nest::InputBackendMPI::disenroll( InputDevice& device )
{
  thread tid = device.get_thread();
  index node_id = device.get_node_id();

  auto device_it = devices_[ tid ].find( node_id );
  if ( device_it != devices_[ tid ].end() )
  {
    devices_[ tid ].erase( device_it );
  }
}

void
nest::InputBackendMPI::set_value_names( const InputDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  // nothing to do
}

void
nest::InputBackendMPI::prepare()
{
  // Create the connection with MPI per thread
  // TODO Validate approach
  // 1) take all the ports of the connections
  thread thread_id = kernel().vp_manager.get_thread_id();

  // get port and update the list of device
  for ( auto& it_device : devices_[ thread_id ] )
  {
    // add the link between MPI communicator and the device (devices can share the same MPI communicator)
    std::string port_name;
    get_port( it_device.second.second, &port_name );
    auto comm_it = commMap_[ thread_id ].find( port_name );
    MPI_Comm* comm;
    if ( comm_it != commMap_[ thread_id ].end() )
    {
      comm = comm_it->second.first;
      comm_it->second.second += 1;
    }
    else
    {
      comm = new MPI_Comm;
      std::pair< MPI_Comm*, int > comm_count = std::make_pair( comm, 1 );
      commMap_[ thread_id ].insert( std::make_pair( port_name, comm_count ) );
    }
    it_device.second.first = comm;
  }

  // 2) connect the thread to the MPI process it needs to be connected to
  // WARNING can be a bug if it's needed that all threads are to be connected via MPI
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    MPI_Comm_connect( it_comm.first.data(),
      MPI_INFO_NULL,
      0,
      MPI_COMM_WORLD,
      it_comm.second.first ); // should use the status for handle error
    std::ostringstream msg;
    msg << "Connect to " << it_comm.first.data() << "\n";
    LOG( M_INFO, "MPI Input connect", msg.str() );
    fflush( stdout );
  }
}

void
nest::InputBackendMPI::pre_run_hook()
{
  // Receive information of MPI process
  // TODO extend, for the moment it only deals with spike trains
  const thread thread_id = kernel().vp_manager.get_thread_id();
  for ( auto& it_device : devices_[ thread_id ] )
  {
    receive_spike_train( *( it_device.second.first ), *( it_device.second.second ) );
  }
}

void
nest::InputBackendMPI::post_step_hook()
{
  // nothing to do
}

void
nest::InputBackendMPI::post_run_hook()
{
  // Send information about the end of the running part
  // TODO Solve question : 1 thread or multiple threads send this information ?
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all threads are to send ending MPI connection message
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    int value[ 1 ];
    value[ 0 ] = thread_id;
    MPI_Send( value, 1, MPI_INT, 0, 1, *it_comm.second.first );
  }
}

void
nest::InputBackendMPI::cleanup()
{
  // Disconnect all the MPI connection and send information about this disconnection
  // Clean all the elements in the map
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all threads send ending MPI connection and
  // disconnect MPI message
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    int value[ 1 ];
    value[ 0 ] = thread_id;
    MPI_Send( value, 1, MPI_INT, 0, 2, *it_comm.second.first );
    MPI_Comm_disconnect( it_comm.second.first );
    delete ( it_comm.second.first );
  }
  // clear map of devices
  commMap_[ thread_id ].clear();
  for ( auto& it_device : devices_[ thread_id ] )
  {
    it_device.second.first = nullptr;
  }
}

void
nest::InputBackendMPI::check_device_status( const DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::InputBackendMPI::get_device_defaults( DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::InputBackendMPI::get_device_status( const nest::InputDevice& device, DictionaryDatum& params_dictionary ) const
{
  // nothing to do
}


void
nest::InputBackendMPI::get_status( lockPTRDatum< Dictionary, &SLIInterpreter::Dictionarytype >& ) const
{
  // nothing to do
}

void
nest::InputBackendMPI::set_status( const DictionaryDatum& d )
{
  // nothing to do
}


void
nest::InputBackendMPI::get_port( InputDevice* device, std::string* port_name )
{
  get_port( device->get_node_id(), device->get_label(), port_name );
}

void
nest::InputBackendMPI::get_port( const index index_node, const std::string& label, std::string* port_name )
{
  // path of the file : path+label+id+.tx
  // (file contains only one line with name of the port)
  std::ostringstream basename;
  const std::string& path = kernel().io_manager.get_data_path();
  if ( not path.empty() )
  {
    basename << path << '/';
  }
  basename << kernel().io_manager.get_data_prefix();

  if ( not label.empty() )
  {
    basename << label;
  }
  else
  {
    throw MPIFilePortsUnknown( index_node );
  }
  char add_path[ 150 ];
  sprintf( add_path, "/%zu.txt", index_node );
  basename << add_path;
  std::cout << basename.rdbuf() << std::endl;
  std::ifstream file( basename.str() );

  if ( file.is_open() )
  {
    getline( file, *port_name );
  }
  file.close();
}

void
nest::InputBackendMPI::receive_spike_train( const MPI_Comm& comm, InputDevice& device )
{
  // Send the first message with id of device and thread id
  int message[ 2 ];
  message[ 0 ] = device.get_node_id();
  message[ 1 ] = kernel().vp_manager.get_thread_id();
  MPI_Status status_mpi;
  MPI_Send( message, 2, MPI_INT, 0, 0, comm );
  // Receive the size of data
  int shape[ 1 ];
  MPI_Recv( &shape, 1, MPI_INT, MPI_ANY_SOURCE, message[ 1 ], comm, &status_mpi );
  // Receive the data ( for the moment only spike time )
  double* spikes{ new double[ shape[ 0 ] ] {} };
  MPI_Recv( spikes, shape[ 0 ], MPI_DOUBLE, status_mpi.MPI_SOURCE, message[ 1 ], comm, &status_mpi );
  std::vector< double > spikes_list( &spikes[0], &spikes[ shape [ 0 ] ]);
  // Update the device with the data
  device.update_from_backend( spikes_list );
  delete[] spikes;
  spikes = nullptr;
}
