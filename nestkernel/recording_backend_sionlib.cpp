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

void
nest::RecordingBackendSIONlib::enroll( RecordingDevice& device )
{
  std::vector< Name > value_names;
  nest::RecordingBackendSIONlib::enroll( device, value_names );
}

void
nest::RecordingBackendSIONlib::enroll( RecordingDevice& device, const std::vector< Name >& value_names )
{
  const thread task = device.get_vp();
  const thread gid = device.get_gid();

#pragma omp critical
  {
    if ( devices_.find( task ) == devices_.end() )
    {
      devices_.insert( std::make_pair( task, device_map::mapped_type() ) );
    }
  }
#pragma omp barrier

  // guarantee that the device has not been enrolled previously
  if ( devices_[ task ].find( gid ) == devices_[ task ].end() )
  {
    DeviceEntry entry( device );
    DeviceInfo& info = entry.info;

    info.gid = gid;
    info.type = static_cast< unsigned int >( device.get_type() );
    info.name = device.get_name();
    info.label = device.get_label();

    info.value_names.reserve( value_names.size() );
    for ( std::vector< Name >::const_iterator it = value_names.begin(); it != value_names.end();
          ++it )
    {
      info.value_names.push_back( it->toString() );
    }

    devices_[ task ].insert( std::make_pair( gid, entry ) );
  }
}

void
nest::RecordingBackendSIONlib::initialize()
{
  int rank = kernel().mpi_manager.get_rank();

  MPI_Comm local_comm = MPI_COMM_NULL;
#ifdef BG_MULTIFILE
  MPIX_Pset_same_comm_create( &local_comm );
#endif // BG_MULTIFILE

  // we need to delay the throwing of exceptions to the end of the parallel section
  WrappedThreadException* we = NULL;

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    const thread task = kernel().vp_manager.thread_to_vp( t );
    if (! task) {
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
      char* filename_c = strdup( filename.c_str() );

      std::ifstream test( filename.c_str() );
      if ( test.good() & !kernel().io_manager.overwrite_files() )
      {
#ifndef NESTIO
        std::string msg = String::compose(
          "The device file '%1' exists already and will not be overwritten. "
          "Please change data_path, or data_prefix, or set /overwrite_files "
          "to true in the root node.",
          filename );
        LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
        throw IOError();
#endif // NESTIO
      }
      else
        test.close();

// SIONlib parameters
#ifdef BG_MULTIFILE
      int n_files = -1;
#else
      int n_files = 1;
#endif // BG_MULTIFILE
      sion_int32 fs_block_size = -1;
      sion_int64 sion_chunksize = P_.sion_chunksize_;

      file.sid = sion_paropen_ompi( filename_c,
        P_.sion_collective_ ? "bw,cmerge" : "bw",
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
        we = new WrappedThreadException( e );
    }
  } // parallel

  // check if any exceptions have been raised
  if ( we )
  {
    WrappedThreadException wec( *we );
    delete we;
    throw wec;
  }
}

void
nest::RecordingBackendSIONlib::finalize()
{
  close_files_();
}

void
nest::RecordingBackendSIONlib::close_files_()
{
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
      // This is potentially dangerous, since we assume that the same set of devices exists on every
      // MPI rank. This should be safe in most situations, excluding e.g. the global spike detector.
      for ( device_map::mapped_type::iterator it = devices_[ task ].begin();
            it != devices_[ task ].end();
            ++it )
      {
        const index gid = it->first;
        sion_uint64 n_rec = 0;

        for ( device_map::iterator jj = devices_.begin(); jj != devices_.end(); ++jj )
        {
          n_rec += jj->second.find( gid )->second.info.n_rec;
        }

        unsigned long n_rec_total = 0;
        MPI_Reduce( &n_rec, &n_rec_total, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD );
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
      struct {
	sion_int64 info_blk;
	sion_int64 info_pos;
      } data_end = {info_blk, info_pos};
      
      double t_end = kernel().simulation_manager.get_time().get_ms();
      double resolution = Time::get_resolution().get_ms();

      sion_fwrite( &t_start_, sizeof( double ), 1, file.sid );
      sion_fwrite( &t_end, sizeof( double ), 1, file.sid );
      sion_fwrite( &resolution, sizeof( double ), 1, file.sid );

      // write device info
      const sion_uint64 n_dev = static_cast< sion_uint64 >( devices_[ task ].size() );
      sion_fwrite( &n_dev, sizeof( sion_uint64 ), 1, file.sid );

      sion_uint64 gid;
      sion_uint32 type;
      sion_uint64 n_rec;
      sion_uint32 n_val;

      for ( device_map::mapped_type::iterator it = devices_[ task ].begin();
            it != devices_[ task ].end();
            ++it )
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

        for ( std::vector< std::string >::iterator it = dev_info.value_names.begin();
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
}

void
nest::RecordingBackendSIONlib::synchronize()
{
  if ( !P_.sion_collective_ )
    return;

  const thread t = kernel().vp_manager.get_thread_id();
  const thread task = kernel().vp_manager.thread_to_vp( t );

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  sion_coll_fwrite( buffer.read(), 1, buffer.get_size(), file.sid );
  buffer.clear();
}

void
nest::RecordingBackendSIONlib::write( const RecordingDevice& device, const Event& event )
{
  const thread task = device.get_vp();
  const index device_gid = device.get_gid();

  const index sender_gid = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const delay steps = stamp.get_steps();
  const double offset = event.get_offset();

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  // increase number of recored entries for the device in question on the corresponding thread
  devices_.find( task )->second.find( device_gid )->second.info.n_rec++;

  const sion_int64 step = static_cast< sion_int64 >( steps );
  const sion_uint32 n_values = 0;

  // 2 * GID (device, source) + time in steps + offset (double) + number of values
  const unsigned int required_space =
    2 * sizeof( sion_uint64 ) + sizeof( sion_int64 ) + sizeof( double ) + sizeof( sion_uint32 );

  if ( P_.sion_collective_ )
  {
    buffer.ensure_space( required_space );
    buffer << device_gid << sender_gid << step << offset << n_values;
    return;
  }

  // we only continue if we are not in collective mode

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
  const thread task = device.get_vp();
  const sion_uint64 device_gid = static_cast< sion_uint64 >( device.get_gid() );

  const sion_uint64 sender_gid = static_cast< sion_uint64 >( event.get_sender_gid() );
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  devices_.find( task )->second.find( device_gid )->second.info.n_rec++;

  const sion_int64 step = static_cast< sion_int64 >( stamp.get_steps() );
  const sion_uint32 n_values = static_cast< sion_uint32 >( values.size() );

  const unsigned int required_space = 2 * sizeof( sion_uint64 ) + sizeof( sion_int64 )
    + ( 1 + n_values ) * sizeof( double ) + sizeof( sion_uint32 );

  if ( P_.sion_collective_ )
  {
    buffer.ensure_space( required_space );
    buffer << device_gid << sender_gid << step << offset << n_values;
    for ( std::vector< double >::const_iterator val = values.begin(); val != values.end(); ++val )
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
    for ( std::vector< double >::const_iterator val = values.begin(); val != values.end(); ++val )
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

    for ( std::vector< double >::const_iterator val = values.begin(); val != values.end(); ++val )
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
  : buffer( NULL )
  , ptr( 0 )
  , max_size( 0 )
{
}

nest::RecordingBackendSIONlib::SIONBuffer::SIONBuffer( size_t size )
  : buffer( NULL )
  , ptr( 0 )
  , max_size( 0 )
{
  reserve( size );
}

nest::RecordingBackendSIONlib::SIONBuffer::~SIONBuffer()
{
  if ( buffer != NULL )
    delete[] buffer;
}

void
nest::RecordingBackendSIONlib::SIONBuffer::reserve( size_t size )
{
  char* new_buffer = new char[ size ];

  if ( buffer != NULL )
  {
    ptr = std::min( ptr, size );
    memcpy( new_buffer, buffer, ptr );
    delete[] buffer;
  }
  buffer = new_buffer;
  max_size = size;
}

void
nest::RecordingBackendSIONlib::SIONBuffer::ensure_space( size_t size )
{
  if ( get_free() < size )
  {
    reserve( max_size + 10 * size );
  }
}

void
nest::RecordingBackendSIONlib::SIONBuffer::write( const char* v, size_t n )
{
  // TODO: replace by get_free()
  if ( ptr + n <= max_size )
  {
    memcpy( buffer + ptr, v, n );
    ptr += n;
  }
  else
  {
	// TODO: use NEST logging framework here
    std::cerr << "SIONBuffer: buffer overflow: ptr=" << ptr << " n=" << n
              << " max_size=" << max_size << std::endl;
  }
}

template < typename T >
nest::RecordingBackendSIONlib::SIONBuffer&
nest::RecordingBackendSIONlib::SIONBuffer::operator<<( const T data )
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
  , buffer_size_( 1024 )
{
}

void
nest::RecordingBackendSIONlib::Parameters_::get( const RecordingBackendSIONlib& al, DictionaryDatum& d ) const
{
  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::buffer_size ] = buffer_size_;
  ( *d )[ names::sion_chunksize ] = sion_chunksize_;
  ( *d )[ names::sion_collective ] = sion_collective_;
}

void
nest::RecordingBackendSIONlib::Parameters_::set( const RecordingBackendSIONlib& al, const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  updateValue< long >( d, names::buffer_size, buffer_size_ );
  updateValue< long >( d, names::sion_chunksize, sion_chunksize_ );
  updateValue< bool >( d, names::sion_collective, sion_collective_ );
}

void
nest::RecordingBackendSIONlib::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
