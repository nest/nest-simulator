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
#include "exceptions.h"

nest::RecordingBackendMPI::RecordingBackendMPI()
  : enrolled_( false )
  , prepared_( false )
{
}

nest::RecordingBackendMPI::~RecordingBackendMPI() throw()
{
}


void
nest::RecordingBackendMPI::initialize()
{
  auto nthreads = kernel().vp_manager.get_num_threads();
  std::vector< std::vector< std::vector< std::array< double, 3 > > > > empty_vector( nthreads );
  buffer_.swap( empty_vector );
  device_map devices( nthreads );
  devices_.swap( devices );
}

void
nest::RecordingBackendMPI::finalize()
{
  // clear vector of buffer
  for ( auto& it_buffer : buffer_ )
  {
    it_buffer.clear();
  }
  buffer_.clear();
  // clear vector of map
  for ( auto& it_device : devices_ )
  {
    it_device.clear();
  }
  devices_.clear();
  commMap_.clear();
}

void
nest::RecordingBackendMPI::enroll( const RecordingDevice& device, const DictionaryDatum& )
{
  if ( device.get_type() == RecordingDevice::SPIKE_RECORDER )
  {
    thread tid = device.get_thread();
    index node_id = device.get_node_id();

    auto device_it = devices_[ tid ].find( node_id );
    if ( device_it != devices_[ tid ].end() )
    {
      devices_[ tid ].erase( device_it );
    }

    std::tuple< int, MPI_Comm*, const RecordingDevice* > tuple = std::make_tuple( -1, nullptr, &device );
    devices_[ tid ].insert( std::make_pair( node_id, tuple ) );
    enrolled_ = true;
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
nest::RecordingBackendMPI::set_value_names( const RecordingDevice&,
  const std::vector< Name >&,
  const std::vector< Name >& )
{
  // nothing to do
}

void
nest::RecordingBackendMPI::prepare()
{
  if ( not enrolled_ )
  {
    return;
  }

  if ( prepared_ )
  {
    throw BackendPrepared( "RecordingBackendMPI" );
  }
  prepared_ = true;
  thread thread_id_master = 0;
#pragma omp parallel default( none ) shared( thread_id_master )
  {
#pragma omp master
    {
      // Create the connection with MPI
      // 1) take all the ports of the connections
      // get port and update the list of devices
      thread_id_master = kernel().vp_manager.get_thread_id();
    }
  }
  int count_max = 0;
  for ( auto& it_device : devices_[ thread_id_master ] )
  {
    // add the link between MPI communicator and the device (devices can share the same MPI communicator)
    std::string port_name;
    get_port( std::get< 2 >( it_device.second ), &port_name );
    auto comm_it = commMap_.find( port_name );
    MPI_Comm* comm;
    int index_mpi;
    if ( comm_it != commMap_.end() )
    {
      comm = std::get< 1 >( comm_it->second );
      std::get< 2 >( comm_it->second ) += 1;
      index_mpi = std::get< 0 >( comm_it->second );
    }
    else
    {
      comm = new MPI_Comm;
      std::tuple< int, MPI_Comm*, int > comm_count = std::make_tuple( count_max, comm, 1 );
      commMap_.insert( std::make_pair( port_name, comm_count ) );
      index_mpi = count_max;
      count_max += 1;
    }
    std::get< 0 >( it_device.second ) = index_mpi;
    std::get< 1 >( it_device.second ) = comm;
  }

  // initialize the buffer
  for ( auto& thread_data : buffer_ )
  {
    std::vector< std::vector< std::array< double, 3 > > > data_comm( count_max );
    thread_data.swap( data_comm );
  }

  // 2) connect the thread to the MPI process it needs to be connected to
  for ( auto& it_comm : commMap_ )
  {
    MPI_Comm_connect( it_comm.first.data(),
      MPI_INFO_NULL,
      0,
      MPI_COMM_WORLD,
      std::get< 1 >( it_comm.second ) ); // should use the status for handle error
    std::ostringstream msg;
    msg << "Connect to " << it_comm.first.data() << "\n";
    LOG( M_INFO, "MPI Record connect", msg.str() );
  }
#pragma omp parallel default( none ) shared( thread_id_master )
  {
    // Update all the threads
    thread thread_id = kernel().vp_manager.get_thread_id();
    if ( thread_id != thread_id_master )
    {
      for ( auto& it_device : devices_[ thread_id ] )
      {
        auto device_data = devices_[ thread_id_master ].find( it_device.first );
        std::get< 0 >( it_device.second ) = std::get< 0 >( device_data->second );
        std::get< 1 >( it_device.second ) = std::get< 1 >( device_data->second );
      }
    }
  }
}

void
nest::RecordingBackendMPI::pre_run_hook()
{
#pragma omp master
  {
    for ( auto& it_comm : commMap_ )
    {
      bool value[ 1 ] = { true };
      MPI_Send( &value, 1, MPI_CXX_BOOL, 0, 0, *std::get< 1 >( it_comm.second ) );
    }
  }
#pragma omp barrier
}


void
nest::RecordingBackendMPI::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendMPI::post_run_hook()
{
#pragma omp master
  {
    // Receive information of MPI process
    for ( auto& it_comm : commMap_ )
    {
      // available to receive information
      bool value[ 1 ];
      MPI_Status status = MPI_Status();
      MPI_Recv( &value, 1, MPI_CXX_BOOL, 0, 0, *std::get< 1 >( it_comm.second ), &status );
      int index_comm = std::get< 0 >( it_comm.second );

      std::vector< double > data;
      for ( auto& data_thread : buffer_ )
      {
        for ( auto& data_sample : data_thread[ index_comm ] )
        {
          data.push_back( data_sample[ 0 ] );
          data.push_back( data_sample[ 1 ] );
          data.push_back( data_sample[ 2 ] );
        }
      }
      send_data( std::get< 1 >( it_comm.second ), data.data(), data.size() );
    }
    // clear the buffer
    for ( auto& data_thread : buffer_ )
    {
      for ( auto& data_comm : data_thread )
      {
        data_comm.clear();
      }
    }
    // Send information about the end of the running part
    for ( auto& it_comm : commMap_ )
    {
      int value[ 1 ] = { true };
      MPI_Send( value, 1, MPI_CXX_BOOL, 0, 1, *std::get< 1 >( it_comm.second ) );
    }
  }
#pragma omp barrier
}

void
nest::RecordingBackendMPI::cleanup()
{
// Disconnect all the MPI connections and send information about this disconnection
// Clean all the elements in the map
// disconnect MPI
#pragma omp master
  {
    for ( auto& it_comm : commMap_ )
    {
      bool value[ 1 ] = { true };
      MPI_Send( value, 1, MPI_CXX_BOOL, 0, 2, *std::get< 1 >( it_comm.second ) );
      MPI_Comm_disconnect( std::get< 1 >( it_comm.second ) );
      delete ( std::get< 1 >( it_comm.second ) );
    }
    // clear the buffer
    for ( auto& data_thread : buffer_ )
    {
      data_thread.clear();
    }
    // clear map of device
    commMap_.clear();
    thread thread_id_master = kernel().vp_manager.get_thread_id();
    for ( auto& it_device : devices_[ thread_id_master ] )
    {
      std::get< 0 >( it_device.second ) = -1;
      std::get< 1 >( it_device.second ) = nullptr;
    }
  }
#pragma omp barrier
}

void
nest::RecordingBackendMPI::check_device_status( const DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::get_device_defaults( DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::get_device_status( const nest::RecordingDevice&, DictionaryDatum& ) const
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
  const index recorder = device.get_node_id();
  const Time stamp = event.get_stamp();

  auto it_devices = devices_[ thread_id ].find( recorder );
  if ( it_devices != devices_[ thread_id ].end() )
  {
    std::array< double, 3 > data{ double( recorder ), double( sender ), stamp.get_ms() };
    buffer_[ thread_id ][ std::get< 0 >( it_devices->second ) ].push_back( data );
  }
  else
  {
    throw BackendPrepared( " Internal error " );
  }
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */
void
nest::RecordingBackendMPI::get_status( DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendMPI::set_status( const DictionaryDatum& )
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
  // path of the file : path+label+id+.txt
  // (file contains only one line with name of the port )
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
    throw MPIPortsFileUnknown( index_node );
  }

  basename << "/" << index_node << ".txt";
  std::cout << basename.rdbuf() << std::endl;
  std::ifstream file( basename.str() );
  if ( file.is_open() )
  {
    getline( file, *port_name );
  }
  file.close();
}

void
nest::RecordingBackendMPI::send_data( const MPI_Comm* comm, const double data[], const int size )
{
  // Send the size of data
  int shape = { size };
  MPI_Send( &shape, 1, MPI_INT, 0, 0, *comm );
  // Receive the data ( for the moment only spike time )
  MPI_Send( data, shape, MPI_DOUBLE, 0, 0, *comm );
}
