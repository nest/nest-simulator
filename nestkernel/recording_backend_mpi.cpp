/*
 *  recording_backend_mpi.cpp
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
#include "recording_device.h"
#include "recording_backend_mpi.h"

void
nest::RecordingBackendMPI::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  device_map devices( nthreads );
  devices_.swap( devices );
  comm_map comms( nthreads );
  commMap_.swap( comms );
}

void
nest::RecordingBackendMPI::finalize()
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
nest::RecordingBackendMPI::enroll( const RecordingDevice& device, const DictionaryDatum& params )
{
  if ( device.get_type() == RecordingDevice::SPIKE_DETECTOR )
  {
    thread tid = device.get_thread();
    index node_id = device.get_node_id();

    auto device_it = devices_[ tid ].find( node_id );
    if ( device_it != devices_[ tid ].end() )
    {
      devices_[ tid ].erase( device_it );
    }

    std::pair< MPI_Comm*, const RecordingDevice* > pair = std::make_pair( nullptr, &device );
    devices_[ tid ].insert( std::make_pair( node_id, pair ) );
  }
  else
  {
    throw BadProperty( "Only spike detectors can record to recording backend 'mpi'." );
  }
}

void
nest::RecordingBackendMPI::disenroll( const RecordingDevice& device )
{
  const auto tid = device.get_thread();
  const auto node_id = device.get_node_id();

  auto device_it = devices_[ tid ].find( node_id );
  if ( device_it != devices_[ tid ].end() )
  {
    devices_[ tid ].erase( device_it );
  }
}

void
nest::RecordingBackendMPI::set_value_names( const RecordingDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  // nothing to do
}

void
nest::RecordingBackendMPI::prepare()
{
  // Create the connection with MPI per thread
  // TODO validate approach
  // 1) take all the ports of the connections
  thread thread_id = kernel().vp_manager.get_thread_id();
  // get port and update the list of devices
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
  // WARNING can be a bug if it's needed that all threads are to be connected with MPI
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    std::ostringstream msg;
    msg << "Connect to " << it_comm.first.data() << "\n";
    LOG( M_INFO, "MPI Record connect", msg.str() );
    MPI_Comm_connect( it_comm.first.data(),
      MPI_INFO_NULL,
      0,
      MPI_COMM_WORLD,
      it_comm.second.first ); // should use the status for handle error
  }
}

void
nest::RecordingBackendMPI::pre_run_hook()
{
  // Waiting until all the receptors are ready to receive information
  const thread thread_id = kernel().vp_manager.get_thread_id();
  for ( auto& it_device : devices_[ thread_id ] )
  {
    bool accept_starting[ 1 ];
    MPI_Status status_mpi;
    MPI_Recv( accept_starting, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, *it_device.second.first, &status_mpi );
  }
  // TODO future improvement is to save the source of the signal (meaning the MPI source can an option of the backend)
}


void
nest::RecordingBackendMPI::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendMPI::post_run_hook()
{
  // Send information about the end of the running part
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all the threads need to send ending MPI connection message
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    int value[ 1 ];
    value[ 0 ] = thread_id;
    MPI_Send( value, 1, MPI_INT, 0, 1, *it_comm.second.first );
  }
}

void
nest::RecordingBackendMPI::cleanup()
{
  // Disconnect all the MPI connections and send information about this disconnection
  // Clean all the elements in the map
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all the threads need to send ending MPI connection and
  // disconnect MPI
  for ( auto& it_comm : commMap_[ thread_id ] )
  {
    int value[ 1 ];
    value[ 0 ] = thread_id;
    MPI_Send( value, 1, MPI_INT, 0, 2, *it_comm.second.first );
    MPI_Comm_disconnect( it_comm.second.first );
    delete ( it_comm.second.first );
  }
  // clear map of device
  commMap_[ thread_id ].clear();
  for ( auto& it_device : devices_[ thread_id ] )
  {
    it_device.second.first = nullptr;
  }
}

void
nest::RecordingBackendMPI::check_device_status( const DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::get_device_defaults( DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::get_device_status( const nest::RecordingDevice& device,
  DictionaryDatum& params_dictionary ) const
{
  // nothing to do
}


void
nest::RecordingBackendMPI::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >&,
  const std::vector< long >& )
{
  // For each event send a message through the right MPI communicator
  const thread thread_id = kernel().get_kernel_manager().vp_manager.get_thread_id();
  const index sender = event.get_sender_node_id();
  const Time stamp = event.get_stamp();

  MPI_Comm* comm;
  auto it_devices = devices_[ thread_id ].find( device.get_node_id() );
  if ( it_devices != devices_[ thread_id ].end() )
  {
    comm = it_devices->second.first;
  }
  else
  {
    throw BackendPrepared( " Internal error " );
  }
  double passed_num[ 2 ] = { double( sender ), stamp.get_ms() };
  MPI_Send( &passed_num, 2, MPI_DOUBLE, 0, thread_id, *comm );
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */
void
nest::RecordingBackendMPI::get_status( DictionaryDatum& d ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::set_status( const DictionaryDatum& d )
{
  // nothing to do
}

void
nest::RecordingBackendMPI::get_port( const RecordingDevice* device, std::string* port_name )
{
  get_port( device->get_node_id(), device->get_label(), port_name );
}

void
nest::RecordingBackendMPI::get_port( const index index_node, const std::string& label, std::string* port_name )
{
  // path of the file : path+label+id+.txt (file contains only one line with name of the port
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
    throw MPIFilePortsUnknown(index_node);
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
