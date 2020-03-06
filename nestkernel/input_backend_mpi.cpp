/*
 *  recording_backend_screen.cpp
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
#include <fstream>


void
nest::InputBackendMPI::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  device_map devices( nthreads );
  devices_.swap( devices );
  comm_map comms(nthreads);
  commMap_.swap(comms);
}

void
nest::InputBackendMPI::finalize()
{
  // clear vector of map
  device_map::iterator it_device;
  for(it_device = devices_.begin();it_device!=devices_.end();it_device++){
    it_device->clear();
  }
  comm_map::iterator it_comm;
  for(it_comm = commMap_.begin();it_comm!=commMap_.end();it_comm++){
    it_comm->clear();
  }
  devices_.clear();
  commMap_.clear();
}

void
nest::InputBackendMPI::enroll( InputDevice& device,
  const DictionaryDatum& params )
{
  if ( device.get_type() == InputDevice::SPIKE_GENERATOR or
        device.get_type() == InputDevice::STEP_CURRENT_GENERATOR ){

	  thread tid = device.get_thread();
	  index node_id = device.get_node_id();

	  device_map::value_type::iterator device_it = devices_[ tid ].find( node_id );
	  if ( device_it != devices_[ tid ].end() )
	  {
	    devices_[ tid ].erase( device_it );
	  }
	  std::pair< MPI_Comm*, InputDevice*> pair = std::make_pair(nullptr,&device);
	  devices_[ tid ].insert( std::make_pair( node_id,  pair) );
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

  device_map::value_type::iterator device_it = devices_[ tid ].find( node_id );
  if ( device_it != devices_[ tid ].end() )
  {
    devices_[ tid ].erase( device_it );
  }
}

void
nest::InputBackendMPI::set_value_names( const InputDevice& device,
                                        const std::vector< Name >& double_value_names,
                                        const std::vector< Name >& long_value_names)
{
  // nothing to do
}

void
nest::InputBackendMPI::prepare()
{
  // Create the connection with MPI by thread ( I am not sure is the best)
  // 1) take all the port of the connections
  thread thread_id = kernel().vp_manager.get_thread_id();

  //get port and update the list of device
  device_map::value_type::iterator it_device;
  for(it_device=devices_[thread_id].begin();it_device != devices_[thread_id].end();it_device++){
    // add the link between MPI communicator and the device (device can share the same MPI communicator
	  std::string port_name;
	  get_port(it_device->second.second,&port_name);
	  comm_map::value_type::iterator comm_it = commMap_[ thread_id ].find(port_name);
	  MPI_Comm * comm;
	  if (comm_it != commMap_[ thread_id ].end())
    {
	    comm = comm_it->second.first;
	    comm_it->second.second+=1;
    } else {
      comm = new MPI_Comm;
      std::pair< MPI_Comm*, int> comm_count = std::make_pair(comm,1);
      commMap_[thread_id].insert(std::make_pair(port_name, comm_count));
	  }
	  it_device->second.first=comm;
  }

  // 2) connect the thread with MPI process it need to be connected
  // WARNING can be a bug if it's need all the thread to be connected in MPI
  comm_map::value_type::iterator it_comm;
  for ( it_comm = commMap_[thread_id].begin(); it_comm != commMap_[thread_id].end(); it_comm++){
    MPI_Comm_connect(it_comm->first.data(), MPI_INFO_NULL, 0, MPI_COMM_WORLD, it_comm->second.first); // should use the status for handle error
    printf("Connect to %s\n", it_comm->first.data());fflush(stdout);
  }
}

void
nest::InputBackendMPI::pre_run_hook()
{
  // connect take information of MPI process (for the moment only spike train)
  const thread thread_id = kernel().vp_manager.get_thread_id();
  device_map::value_type::iterator it_device;
  for (it_device = devices_[thread_id].begin(); it_device != devices_[thread_id].end(); it_device++) {
    receive_spike_train(*(it_device->second.first), *(it_device->second.second));
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
  // Question : 1 thread or multiple threads send this information ?
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all the thread to send ending connection of MPI
  comm_map::value_type::iterator it_comm;
  for (it_comm = commMap_[thread_id].begin(); it_comm != commMap_[thread_id].end(); it_comm++) {
    int value[1];
    value[0] = thread_id;
    MPI_Send(value, 1, MPI_INT, 0, 1, *it_comm->second.first);
  }
}

void
nest::InputBackendMPI::cleanup()
{
  //Disconnect all the MPI connection and send information about this disconnection
  // Clean all the list of map
  thread thread_id = kernel().vp_manager.get_thread_id();
  // WARNING can be a bug if all the thread to send ending connection of MPI
  //disconnect MPI
  comm_map::value_type::iterator it_comm;
  for (it_comm = commMap_[thread_id].begin(); it_comm != commMap_[thread_id].end(); it_comm++) {
    int value[1];
    value[0] = thread_id;
    MPI_Send(value, 1, MPI_INT, 0, 2, *it_comm->second.first);
    MPI_Comm_disconnect(it_comm->second.first);
    delete(it_comm->second.first);
  }
  // clear map of device
  commMap_[thread_id].clear();
  device_map::value_type::iterator it_device;
  for (it_device = devices_[thread_id].begin(); it_device != devices_[thread_id].end(); it_device++) {
    it_device->second.first= nullptr;
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
nest::InputBackendMPI::get_device_status( const nest::InputDevice& device,
                                          DictionaryDatum& params_dictionary ) const
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
nest::InputBackendMPI::get_port(InputDevice *device, std::string* port_name) {
  get_port(device->get_node_id(),device->get_label(),port_name);
}

void
nest::InputBackendMPI::get_port(const index index_node, const std::string& label, std::string* port_name){
  std::ostringstream basename; // path of the file : path+label+id+.txt (file contains only one line with name of the port
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
  else {
     //TODO take in count this case
  }
  char add_path[150];
  sprintf(add_path, "/%zu.txt", index_node);
  basename << add_path;
  std::cout << basename.rdbuf() << std::endl;
  std::ifstream file(basename.str());

  if (file.is_open()) {
    getline(file,*port_name);
  }
  file.close();
}

void
nest::InputBackendMPI::receive_spike_train(const MPI_Comm& comm, InputDevice& device){
  // Send the first message with id of device and thread id
  int message[2];
  message[0] = device.get_node_id();
  message[1] = kernel().vp_manager.get_thread_id();
  MPI_Status status_mpi;
  MPI_Send(message , 2, MPI_INT, 0, 0, comm);
  // Receive the size of data
  int shape[1];
  MPI_Recv(&shape, 1, MPI_INT, MPI_ANY_SOURCE,message[1] ,comm ,&status_mpi);
  // Receive the data ( for the moment only spike time )
  double spikes[shape[0]];
  MPI_Recv(&spikes, shape[0], MPI_DOUBLE,status_mpi.MPI_SOURCE,message[1],comm ,&status_mpi);
  std::vector<double> spikes_list(spikes, spikes + sizeof(spikes) / sizeof(spikes[0]));
  // Update the device with the data
  device.update_from_backend(spikes_list);
}