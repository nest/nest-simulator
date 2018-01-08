/*
 *  recording_backend_sionlib.cpp
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

// C includes:
#include <mpi.h>
#ifdef BG_MULTIFILE
#include <mpix.h>
#endif // BG_MULTIFILE

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_sionlib.h"

nest::RecordingBackendSIONlib::RecordingBackendSIONlib()
 : files_opened_( false )
{
}

nest::RecordingBackendSIONlib::~RecordingBackendSIONlib() throw()
{
  finalize();
}

void
nest::RecordingBackendSIONlib::enroll( const RecordingDevice& device )
{
  std::vector< Name > value_names;
  nest::RecordingBackendSIONlib::enroll( device, value_names );
}

void
nest::RecordingBackendSIONlib::enroll( const RecordingDevice& device,
  const std::vector< Name >& value_names )
{
  const thread t = device.get_thread();
  const thread gid = device.get_gid();

  device_map::value_type::iterator device_it = devices_[ t ].find( gid );
  if ( device_it  != devices_[ t ].end() )
  {
    devices_[ t ].erase( device_it );
  }
  
  DeviceEntry entry( device );
  DeviceInfo& info = entry.info;

  info.gid = gid;
  info.type = static_cast< unsigned int >( device.get_type() );
  info.name = device.get_name();
  info.label = device.get_label();

  info.value_names.reserve( value_names.size() );
  std::vector< Name >::const_iterator it;
  for ( it = value_names.begin(); it != value_names.end(); ++it )
  {
    info.value_names.push_back( it->toString() );
  }

  devices_[ t ].insert( std::make_pair( gid, entry ) );
  
  open_files_();
}

void
nest::RecordingBackendSIONlib::initialize()
{
  device_map devices( kernel().vp_manager.get_num_threads() );
  devices_.swap( devices );
}    

void
nest::RecordingBackendSIONlib::open_files_()
{
  if ( files_opened_ )
  {
    return;
  }
    
  local_comm_ = MPI_COMM_NULL;
#ifdef BG_MULTIFILE
  // MPIX calls not thread-safe; use only master thread here 
  // (omp single may be problematic as well)
#pragma omp master
  {
    MPIX_Pset_same_comm_create( &local_comm_ );
  }
#pragma omp barrier
#endif // BG_MULTIFILE
  // use additional local variable for local communicator to
  // avoid problems when calling sion_paropen_ompi(..)
  MPI_Comm local_comm = local_comm_;

  // we need to delay the throwing of exceptions to the end of the parallel
  // section
  WrappedThreadException* we = NULL;

  // This code is executed in a parallel region (opened in NodeManager::prepare_nodes())!
  const thread t = kernel().vp_manager.get_thread_id();
  const thread task = kernel().vp_manager.thread_to_vp( t );
  if ( !task )
  {
    t_start_ = kernel().simulation_manager.get_time().get_ms();
  }

  try
  {
#pragma omp critical
    {
      if ( files_.find( task ) == files_.end() )
      {
        files_.insert( std::make_pair( task, FileEntry() ) );
      }
    }
#pragma omp barrier

    FileEntry& file = files_[ task ];
    std::string filename = build_filename_();

    std::ifstream test( filename.c_str() );
    if ( test.good() & !kernel().io_manager.overwrite_files() )
    {
      std::string msg = String::compose(
        "The device file '%1' exists already and will not be overwritten. "
        "Please change data_path, or data_prefix, or set /overwrite_files "
        "to true in the root node.", filename );
      LOG( M_ERROR, "RecordingBackendSIONlib::open_files_()", msg );
      throw IOError();
    }
    test.close();

#ifdef BG_MULTIFILE
    int n_files = -1;
#else
    int n_files = P_.sion_n_files_;
#endif // BG_MULTIFILE
    sion_int32 fs_block_size = -1;
    sion_int64 sion_chunksize = P_.sion_chunksize_;
    int rank = kernel().mpi_manager.get_rank();

    file.sid = sion_paropen_ompi( filename.c_str(),
      P_.sion_collective_ ? "bw,cmerge,collsize=-1" : "bw",
      &n_files,
      MPI_COMM_WORLD,
      &local_comm,
      &sion_chunksize,
      &fs_block_size,
      &rank,
      NULL,
      NULL );

    file.buffer.reserve( P_.buffer_size_ );
    file.buffer.clear();

    filename_ = filename;
  }
  catch ( std::exception& e )
  {
#pragma omp critical
    if ( !we )
    {
      we = new WrappedThreadException( e );
    }
  }

  // check if any exceptions have been raised
  if ( we )
  {
    WrappedThreadException wec( *we );
    delete we;
    throw wec;
  }

  files_opened_ = true;
}

void
nest::RecordingBackendSIONlib::finalize()
{
  close_files_();
}

void
nest::RecordingBackendSIONlib::close_files_()
{
  if ( not files_opened_ )
  {
    return;
  }

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    const thread task = kernel().vp_manager.thread_to_vp( t );

    assert( ( files_.find( task ) != files_.end() )
      && "initialize() was not called before calling finalize()" );

    FileEntry& file = files_[ task ];
    SIONBuffer& buffer = file.buffer;

    if ( buffer.get_size() > 0 )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

#pragma omp master
    {
      device_map::value_type::iterator it;
      for ( it = devices_[ t ].begin(); it != devices_[ t ].end(); ++it )
      {
        const index gid = it->first;
        sion_uint64 n_rec = 0;

	device_map::iterator jj;
        for ( jj = devices_.begin(); jj != devices_.end(); ++jj )
        {
          n_rec += jj->find( gid )->second.info.n_rec;
        }

        unsigned long n_rec_total = 0;
        MPI_Reduce( &n_rec,
          &n_rec_total,
          1,
          MPI_UNSIGNED_LONG,
          MPI_SUM,
          0,
          MPI_COMM_WORLD );
        assert( sizeof( unsigned long ) <= sizeof( sion_uint64 ) );
        it->second.info.n_rec = static_cast< sion_uint64 >( n_rec_total );
      }
    }

    if ( task == 0 )
    {
      int mc;
      sion_int64* cs = NULL;
      int info_blk; // here int, other place sion_int64 due to sion api
      sion_int64 info_pos;

      sion_get_current_location( file.sid, &info_blk, &info_pos, &mc, &cs );
      struct
      {
        sion_int64 info_blk;
        sion_int64 info_pos;
      } data_end = { info_blk, info_pos };

      double t_end = kernel().simulation_manager.get_time().get_ms();
      double resolution = Time::get_resolution().get_ms();

      sion_fwrite( &t_start_, sizeof( double ), 1, file.sid );
      sion_fwrite( &t_end, sizeof( double ), 1, file.sid );
      sion_fwrite( &resolution, sizeof( double ), 1, file.sid );

      // write device info
      const sion_uint64 n_dev =
        static_cast< sion_uint64 >( devices_[ t ].size() );
      sion_fwrite( &n_dev, sizeof( sion_uint64 ), 1, file.sid );

      sion_uint64 gid;
      sion_uint32 type;
      sion_uint64 n_rec;
      sion_uint32 n_val;

      device_map::value_type::iterator it;
      for ( it = devices_[ t ].begin(); it != devices_[ t ].end(); ++it )
      {
        DeviceInfo& dev_info = it->second.info;

        gid = static_cast< sion_uint64 >( dev_info.gid );
        type = static_cast< sion_uint32 >( dev_info.type );

        sion_fwrite( &gid, sizeof( sion_uint64 ), 1, file.sid );
        sion_fwrite( &type, sizeof( sion_uint32 ), 1, file.sid );

        char name[ 16 ];
        strncpy( name, dev_info.name.c_str(), 16 );
        sion_fwrite( &name, sizeof( char ), 16, file.sid );

        char label[ 16 ];
        strncpy( label, dev_info.label.c_str(), 16 );
        sion_fwrite( &label, sizeof( char ), 16, file.sid );

        n_rec = static_cast< sion_uint64 >( dev_info.n_rec );
        sion_fwrite( &n_rec, sizeof( sion_uint64 ), 1, file.sid );

        // potentially dangerous downcasting from size_t assuming that we do
        // not have that many observables
        n_val = static_cast< sion_uint32 >( dev_info.value_names.size() );
        sion_fwrite( &n_val, sizeof( sion_uint32 ), 1, file.sid );

        for ( std::vector< std::string >::iterator it =
                dev_info.value_names.begin();
              it != dev_info.value_names.end();
              ++it )
        {
          char name[ 8 ];
          strncpy( name, it->c_str(), 8 );
          sion_fwrite( &name, sizeof( char ), 8, file.sid );
        }
      }

      // write tail to find beginning of meta data for task 0
      // write it as a single buffer to guarantee that it goes in one chunk
      sion_fwrite( &data_end, sizeof( data_end ), 1, file.sid );
    }

    sion_parclose_ompi( file.sid );
  }

  files_opened_ = false;
}

void
nest::RecordingBackendSIONlib::synchronize()
{
  if ( not files_opened_ or not P_.sion_collective_ )
  {
    return;
  }

  const thread t = kernel().vp_manager.get_thread_id();
  const thread task = kernel().vp_manager.thread_to_vp( t );

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  sion_coll_fwrite( buffer.read(), 1, buffer.get_size(), file.sid );
  buffer.clear();
}

void
nest::RecordingBackendSIONlib::write( const RecordingDevice& device,
  const Event& event )
{
  const thread t = device.get_thread();
  const sion_uint64 device_gid = static_cast< sion_uint64 >( device.get_gid() );

  if ( devices_[ t ].find( device_gid ) == devices_[ t ].end() )
  {
    return;
  }

  FileEntry& file = files_[ device.get_vp() ];
  SIONBuffer& buffer = file.buffer;

  devices_[ t ].find( device_gid )->second.info.n_rec++;
  const sion_uint32 n_values = 0;

  // 2 * GID (device, source) + time in steps + offset (double) + number of
  // values
  const unsigned int required_space = 2 * sizeof( sion_uint64 )
    + sizeof( sion_int64 ) + sizeof( double ) + sizeof( sion_uint32 );

  const sion_uint64 sender_gid = static_cast< sion_uint64 >( event.get_sender_gid() );
  const sion_int64 step = static_cast< sion_int64 >( event.get_stamp().get_steps() );
  const double offset = event.get_offset();
  
  if ( P_.sion_collective_ )
  {
    buffer.ensure_space( required_space );
    buffer << device_gid << sender_gid << step << offset << n_values;
    return;
  }

  if ( buffer.get_capacity() > required_space )
  {
    if ( buffer.get_free() < required_space )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    buffer << device_gid << sender_gid << step << offset << n_values;
  }
  else
  {
    if ( buffer.get_size() > 0 )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    sion_fwrite( &device_gid, sizeof( sion_uint64 ), 1, file.sid );
    sion_fwrite( &sender_gid, sizeof( sion_uint64 ), 1, file.sid );
    sion_fwrite( &step, sizeof( sion_int64 ), 1, file.sid );
    sion_fwrite( &offset, sizeof( double ), 1, file.sid );
    sion_fwrite( &n_values, sizeof( sion_uint32 ), 1, file.sid );
  }
}

void
nest::RecordingBackendSIONlib::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  const thread t = device.get_thread();
  const sion_uint64 device_gid = static_cast< sion_uint64 >( device.get_gid() );

  if ( devices_[ t ].find( device_gid ) == devices_[ t ].end() )
  {
    return;
  }
    
  FileEntry& file = files_[ device.get_vp() ];
  SIONBuffer& buffer = file.buffer;

  devices_[ t ].find( device_gid )->second.info.n_rec++;
  const sion_uint32 n_values = static_cast< sion_uint32 >( values.size() );

  // 2 * GID (device, source) + time in steps + offset (double) + number of
  // values + one double per value
  const unsigned int required_space = 2 * sizeof( sion_uint64 )
      + sizeof( sion_int64 ) + sizeof( double ) + sizeof( sion_uint32 )
      + n_values * sizeof( double );

  const sion_uint64 sender_gid = static_cast< sion_uint64 >( event.get_sender_gid() );
  const sion_int64 step = static_cast< sion_int64 >( event.get_stamp().get_steps() );
  const double offset = event.get_offset();
  
  if ( P_.sion_collective_ )
  {
    buffer.ensure_space( required_space );
    buffer << device_gid << sender_gid << step << offset << n_values;
    std::vector< double >::const_iterator val;
    for (val = values.begin(); val != values.end(); ++val )
    {
      buffer << *val;
    }
    return;
  }

  if ( buffer.get_capacity() > required_space )
  {
    if ( buffer.get_free() < required_space )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    buffer << device_gid << sender_gid << step << offset << n_values;
    std::vector< double >::const_iterator val;
    for ( val = values.begin(); val != values.end(); ++val )
    {
      buffer << *val;
    }
  }
  else
  {
    if ( buffer.get_size() > 0 )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    sion_fwrite( &device_gid, sizeof( sion_uint64 ), 1, file.sid );
    sion_fwrite( &sender_gid, sizeof( sion_uint64 ), 1, file.sid );
    sion_fwrite( &step, sizeof( sion_int64 ), 1, file.sid );
    sion_fwrite( &offset, sizeof( double ), 1, file.sid );
    sion_fwrite( &n_values, sizeof( sion_uint32 ), 1, file.sid );

    std::vector< double >::const_iterator val;
    for ( val = values.begin(); val != values.end(); ++val )
    {
      double value = *val;
      sion_fwrite( &value, sizeof( double ), 1, file.sid );
    }
  }
}

const std::string
nest::RecordingBackendSIONlib::build_filename_() const
{
  std::ostringstream basename;
  const std::string& path = kernel().io_manager.get_data_path();
  if ( !path.empty() )
    basename << path << '/';
  basename << kernel().io_manager.get_data_prefix();

  basename << "output";

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Buffer
 * ---------------------------------------------------------------- */

nest::RecordingBackendSIONlib::SIONBuffer::SIONBuffer()
  : buffer_( NULL )
  , ptr_( 0 )
  , max_size_( 0 )
{
}

nest::RecordingBackendSIONlib::SIONBuffer::SIONBuffer( size_t size )
  : buffer_( NULL )
  , ptr_( 0 )
  , max_size_( 0 )
{
  reserve( size );
}

nest::RecordingBackendSIONlib::SIONBuffer::~SIONBuffer()
{
  if ( buffer_ != NULL )
    delete[] buffer_;
}

void
nest::RecordingBackendSIONlib::SIONBuffer::reserve( size_t size )
{
  char* new_buffer = new char[ size ];

  if ( buffer_ != NULL )
  {
    ptr_ = std::min( ptr_, size );
    memcpy( new_buffer, buffer_, ptr_ );
    delete[] buffer_;
  }
  buffer_ = new_buffer;
  max_size_ = size;
}

void
nest::RecordingBackendSIONlib::SIONBuffer::ensure_space( size_t size )
{
  if ( get_free() < size )
  {
    reserve( max_size_ + 10 * size );
  }
}

void
nest::RecordingBackendSIONlib::SIONBuffer::write( const char* v, size_t n )
{
  if ( n <= get_free() )
  {
    memcpy( buffer_ + ptr_, v, n );
    ptr_ += n;
  }
  else
  {
    std::string msg = String::compose(
      "SIONBuffer: buffer overflow: ptr=%1, n=%2, max_size=%3.",
     ptr_, n, max_size_ );
    LOG( M_ERROR, "RecordingBackendSIONlib::write()", msg );
    throw IOError();
  }
}

template < typename T >
nest::RecordingBackendSIONlib::SIONBuffer&
  nest::RecordingBackendSIONlib::SIONBuffer::
  operator<<( const T data )
{
  write( ( const char* ) &data, sizeof( T ) );
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendSIONlib::Parameters_::Parameters_()
  : file_ext_( "sion" )
  , sion_collective_( false )
  , sion_chunksize_( 1 << 18 )
  , sion_n_files_( 1 )
  , buffer_size_( 1024 )
{
}

void
nest::RecordingBackendSIONlib::Parameters_::get(
  const RecordingBackendSIONlib& al,
  DictionaryDatum& d ) const
{
  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::buffer_size ] = buffer_size_;
  ( *d )[ names::sion_chunksize ] = sion_chunksize_;
  ( *d )[ names::sion_collective ] = sion_collective_;
  ( *d )[ names::sion_n_files ] = sion_n_files_;
}

void
nest::RecordingBackendSIONlib::Parameters_::set(
  const RecordingBackendSIONlib& al,
  const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  updateValue< long >( d, names::buffer_size, buffer_size_ );
  updateValue< long >( d, names::sion_chunksize, sion_chunksize_ );
  updateValue< bool >( d, names::sion_collective, sion_collective_ );
  updateValue< long >( d, names::sion_n_files, sion_n_files_ );
}

void
nest::RecordingBackendSIONlib::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
