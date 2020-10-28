/*
 *  stimulating_backend_mpi.cpp
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
#include <fstream>

// Includes from nestkernel:
#include "stimulating_backend.h"
#include "stimulating_backend_mpi.h"
//#include "kernel_manager.h"

nest::StimulatingBackendMPI::StimulatingBackendMPI()
  : enrolled_( false )
  , prepared_( false )
{
}

nest::StimulatingBackendMPI::~StimulatingBackendMPI()
{
}


void
nest::StimulatingBackendMPI::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  device_map devices( nthreads );
  devices_.swap( devices );
}

void
nest::StimulatingBackendMPI::finalize()
{
  // clear vector of map
  for ( auto& it_device : devices_ )
  {
    it_device.clear();
  }
  devices_.clear();
  commMap_.clear();
}

void
nest::StimulatingBackendMPI::enroll( StimulatingDevice< EmittedEvent >& device, const DictionaryDatum& params )
{
  if ( device.get_type() == StimulatingDevice< EmittedEvent >::SPIKE_GENERATOR or device.get_type() == StimulatingDevice< EmittedEvent >::STEP_CURRENT_GENERATOR )
  {
    thread tid = device.get_thread();
    index node_id = device.get_node_id();

    auto device_it = devices_[ tid ].find( node_id );
    if ( device_it != devices_[ tid ].end() )
    {
      devices_[ tid ].erase( device_it );
    }
    std::pair< MPI_Comm*, StimulatingDevice< EmittedEvent >* > pair = std::make_pair( nullptr, &device );
    devices_[ tid ].insert( std::make_pair( node_id, pair ) );
    enrolled_ = true;
  }
  else
  {
    throw BadProperty( "Currently only spike generators and step current generators can have input backend 'mpi'." );
  }
}

void
nest::StimulatingBackendMPI::disenroll( const nest::StimulatingDevice< EmittedEvent >& device )
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
nest::StimulatingBackendMPI::set_value_names( const StimulatingDevice< EmittedEvent >& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  // nothing to do
}

void
nest::StimulatingBackendMPI::prepare()
{
  printf("In stimulating backend MPI prepare %b", enrolled_);
  if ( not enrolled_ )
  {
    return;
  }

  if ( prepared_ )
  {
    throw BackendPrepared( "RecordingBackendArbor" );
  }
  prepared_ = true;

  // need to be run only by the master thread : it is the case because it's not run in parallel
  thread thread_id_master = kernel().vp_manager.get_thread_id();
  // Create the connection with MPI
  // 1) take all the ports of the connections
  // get port and update the list of device only for master
  for ( auto& it_device : devices_[ thread_id_master ] )
  {
    // add the link between MPI communicator and the device (devices can share the same MPI communicator)
    std::string port_name;
    get_port( it_device.second.second, &port_name );
    auto comm_it = commMap_.find( port_name );
    MPI_Comm* comm;
    if ( comm_it != commMap_.end() )
    {
      comm = comm_it->second.first;
      comm_it->second.second += 1;
    }
    else
    {
      comm = new MPI_Comm;
      std::pair< MPI_Comm*, int > comm_count = std::make_pair( comm, 1 );
      commMap_.insert( std::make_pair( port_name, comm_count ) );
    }
    it_device.second.first = comm;
  }

  // 2) connect the master thread to the MPI process it needs to be connected to
  for ( auto& it_comm : commMap_ )
  {
    MPI_Comm_connect( it_comm.first.data(),
      MPI_INFO_NULL,
      0,
      MPI_COMM_WORLD,
      it_comm.second.first ); // should use the status for handle error
    std::ostringstream msg;
    msg << "Connect to " << it_comm.first.data() << "\n";
    LOG( M_INFO, "MPI Input connect", msg.str() );
  }
}

void
nest::StimulatingBackendMPI::pre_run_hook()
{
  printf("In stimulating backend MPI pre_run_hook %b", enrolled_);
#pragma omp master
  {
    for ( auto& it_comm : commMap_ )
    {
      bool value[ 1 ] = { true };
      MPI_Send( value, 1, MPI_CXX_BOOL, 0, 0, *it_comm.second.first );
    }
    // Receive information of MPI process
    for ( auto& it_device : devices_[ 0 ] )
    {
      receive_spike_train( *( it_device.second.first ), *( it_device.second.second ) );
    }
  }
#pragma omp barrier
}

void
nest::StimulatingBackendMPI::post_step_hook()
{
  // nothing to do
}

void
nest::StimulatingBackendMPI::post_run_hook()
{
#pragma omp master
  {
    // Send information about the end of the running part
    for ( auto& it_comm : commMap_ )
    {
      bool value[ 1 ] = { true };
      MPI_Send( value, 1, MPI_CXX_BOOL, 0, 1, *it_comm.second.first );
    }
  }
#pragma omp barrier
}

void
nest::StimulatingBackendMPI::cleanup()
{
// Disconnect all the MPI connection and send information about this disconnection
// Clean all the elements in the map
// disconnect MPI message
#pragma omp master
  {
    for ( auto& it_comm : commMap_ )
    {
      bool value[ 1 ] = { true };
      MPI_Send( value, 1, MPI_CXX_BOOL, 0, 2, *it_comm.second.first );
      MPI_Comm_disconnect( it_comm.second.first );
      delete it_comm.second.first;
    }
    // clear map of devices
    commMap_.clear();
    thread thread_id_master = kernel().vp_manager.get_thread_id();
    for ( auto& it_device : devices_[ thread_id_master ] )
    {
      it_device.second.first = nullptr;
    }
  }
#pragma omp barrier
}

void
nest::StimulatingBackendMPI::check_device_status( const DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::StimulatingBackendMPI::get_device_defaults( DictionaryDatum& params ) const
{
  // nothing to do
}

void
nest::StimulatingBackendMPI::get_device_status( const nest::StimulatingDevice< EmittedEvent >& device,
  DictionaryDatum& params_dictionary ) const
{
  // nothing to do
}


void
nest::StimulatingBackendMPI::get_status( lockPTRDatum< Dictionary, &SLIInterpreter::Dictionarytype >& ) const
{
  // nothing to do
}

void
nest::StimulatingBackendMPI::set_status( const DictionaryDatum& d )
{
  // nothing to do
}


void
nest::StimulatingBackendMPI::get_port( StimulatingDevice< EmittedEvent >* device, std::string* port_name )
{
  get_port( device->get_node_id(), device->get_label(), port_name );
}

void
nest::StimulatingBackendMPI::get_port( const index index_node, const std::string& label, std::string* port_name )
{
  // path of the file : path+label+id+.txt
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
nest::StimulatingBackendMPI::receive_spike_train( const MPI_Comm& comm, StimulatingDevice< EmittedEvent >& device )
{
  // Send the first message with id of device
  int message[ 1 ];
  message[ 0 ] = device.get_node_id();
  MPI_Send( &message, 1, MPI_INT, 0, 0, comm );
  // Receive the size of data
  MPI_Status status_mpi;
  int shape[ 1 ];
  MPI_Recv( &shape, 1, MPI_INT, MPI_ANY_SOURCE, message[ 0 ], comm, &status_mpi );
  // Receive the data ( for the moment only spike time )
  double* spikes{ new double[ shape[ 0 ] ]{} };
  MPI_Recv( spikes, shape[ 0 ], MPI_DOUBLE, status_mpi.MPI_SOURCE, message[ 0 ], comm, &status_mpi );
  std::vector< double > spikes_list( &spikes[ 0 ], &spikes[ shape[ 0 ] ] );
  // Update the device with the data in all the thread
  for ( auto& thread_device : devices_ )
  {
    thread_device.find( device.get_node_id() )->second.second->update_from_backend( spikes_list );
  }
  delete[] spikes;
  spikes = nullptr;
}
