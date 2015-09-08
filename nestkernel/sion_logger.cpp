#include <sstream>

#include "recording_device.h"

#include "sion.h"

#include "sion_logger.h"

void
nest::SIONLogger::enroll( const int task, RecordingDevice& device )
{
  std::vector< Name > value_names;
  nest::SIONLogger::enroll( task, device, value_names );
}

void
nest::SIONLogger::enroll( const int task,
  RecordingDevice& device,
  const std::vector< Name >& value_names )
{
  const int gid = device.get_gid();
  
#pragma omp critical
  {
    if ( devices_.find( task ) == devices_.end() )
    {
      devices_.insert( std::make_pair( task, device_map::mapped_type() ) );
    }
  }

  if ( devices_[ task ].find( gid ) == devices_[ task ].end() )
  {
    DeviceEntry entry( device );
    DeviceInfo& info = entry.info;

    info.gid = device.get_gid();
    info.type = device.get_type();
    info.name = device.get_name();

    info.value_names.reserve( value_names.size() );
    for ( std::vector< Name >::const_iterator it = value_names.begin(); it != value_names.end();
          ++it )
    {
      info.value_names.push_back( it->toString() );
    }

    devices_[ task ].insert( std::make_pair( gid, entry ) );
  }
  else
  {
    // TODO: device already enrolled! error handling!
  }
}

void
nest::SIONLogger::initialize()
{
  MPI_Comm local_comm;
  int rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );

#pragma omp parallel
  {
    Network& network = *( Node::network() );
    thread t = network.get_thread_id();
    int task = network.thread_to_vp( t );

#pragma omp critical
    {
      if ( files_.find( task ) == files_.end() )
      {
        files_.insert( std::make_pair( task, FileEntry() ) );
      }
    }

    FileEntry& file = files_[ task ];
    FileInfo& info = file.info;

    std::string filename = build_filename_();
    char* filename_c = strdup( filename.c_str() );

    std::ifstream test( filename.c_str() );
    if ( test.good() & !Node::network()->overwrite_files() )
    {
#ifndef NESTIO
      std::string msg = String::compose(
        "The device file '%1' exists already and will not be overwritten. "
        "Please change data_path, or data_prefix, or set /overwrite_files "
        "to true in the root node.",
        filename );
      Node::network()->message( SLIInterpreter::M_ERROR, "RecordingDevice::calibrate()", msg );
#endif // NESTIO
      throw IOError();
    }
    else
      test.close();

    // SIONlib parameters
    int n_files = 1;
    sion_int32 fs_block_size = -1;

    sion_int64 sion_buffer_size = P_.sion_buffer_size_;

    file.sid = sion_paropen_ompi( filename_c,
      "bw",
      &n_files,
      MPI_COMM_WORLD,
      &local_comm, // FIXME: does it do anything when not on JUQUEEN?
      &sion_buffer_size,
      &fs_block_size,
      &rank,
      NULL,
      NULL ); // FIXME: nullptr allowed here? Readback filename?

    int mc;
    sion_int64* cs;
    sion_get_current_location( file.sid, &( info.body_blk ), &( info.body_pos ), &mc, &cs );

    info.t_start = Node::network()->get_time().get_ms();

    file.buffer.reserve( P_.buffer_size_ );
    file.buffer.clear();

    for ( device_map::mapped_type::iterator it = devices_[ task ].begin();
          it != devices_[ task ].end();
          ++it )
    {
      RecordingDevice& device = it->second.device;
      device.set_filename( filename );
    }
  }
}

void
nest::SIONLogger::finalize()
{
#pragma omp parallel
  {
    Network& network = *( Node::network() );
    thread t = network.get_thread_id();
    int task = network.thread_to_vp( t );

    if ( files_.find( task ) == files_.end() )
    {
      // TODO: error handling
    }

    FileEntry& file = files_[ task ];
    FileInfo& info = file.info;
    SIONBuffer& buffer = file.buffer;

    if ( buffer.get_size() > 0 )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    info.t_end = Node::network()->get_time().get_ms();

    int mc;
    sion_int64* cs;
    sion_get_current_location( file.sid, &( info.info_blk ), &( info.info_pos ), &mc, &cs );

	// write device info
    int n_dev = devices_[ task ].size();
    sion_fwrite( &n_dev, sizeof( int ), 1, file.sid );

    for ( device_map::mapped_type::iterator it = devices_[ task ].begin();
          it != devices_[ task ].end();
          ++it )
    {
      DeviceInfo& dev_info = it->second.info;

      sion_fwrite( &( dev_info.gid ), sizeof( int ), 1, file.sid );
      sion_fwrite( &( dev_info.type ), sizeof( int ), 1, file.sid );

      char name[ 16 ];
      strncpy( name, dev_info.name.c_str(), 16 );
      sion_fwrite( &name, sizeof( char ), 16, file.sid );

      sion_fwrite( &( dev_info.n_rec ), sizeof( unsigned long ), 1, file.sid );

      int n_val = dev_info.value_names.size();
      sion_fwrite( &n_val, sizeof( int ), 1, file.sid );

      for ( std::vector< std::string >::iterator it = dev_info.value_names.begin();
            it != dev_info.value_names.end();
            ++it )
      {

        char name[ 8 ];
        strncpy( name, it->c_str(), 8 );
        sion_fwrite( &name, sizeof( char ), 8, file.sid );
      }
    }

    // write tail
    sion_fwrite( &( info.body_blk ), sizeof( int ), 1, file.sid );
    sion_fwrite( &( info.body_pos ), sizeof( sion_int64 ), 1, file.sid );

    sion_fwrite( &( info.info_blk ), sizeof( int ), 1, file.sid );
    sion_fwrite( &( info.info_pos ), sizeof( sion_int64 ), 1, file.sid );

    sion_fwrite( &( info.t_start ), sizeof( double ), 1, file.sid );
    sion_fwrite( &( info.t_end ), sizeof( double ), 1, file.sid );
    sion_fwrite( &( info.resolution ), sizeof( double ), 1, file.sid );

    sion_parclose_ompi( file.sid );
  }
}

void
nest::SIONLogger::write( const RecordingDevice& device, const Event& event )
{
  int task = device.get_vp();
  int gid = device.get_gid();

  // FIXME: use proper type for sender (was: index)
  const int sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  devices_.find( task )->second.find( gid )->second.info.n_rec++;

  double time = stamp.get_ms() - offset;
  int n_values = 0;

  unsigned int required_space = 3 * sizeof( int ) + sizeof( double );
  if ( buffer.get_capacity() > required_space )
  {
    if ( buffer.get_free() < required_space )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    buffer << gid << sender << time << n_values;
  }
  else
  {
    if ( buffer.get_size() > 0 )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    sion_fwrite( &gid, sizeof( int ), 1, file.sid );
    sion_fwrite( &sender, sizeof( int ), 1, file.sid );
    sion_fwrite( &time, sizeof( double ), 1, file.sid );
    sion_fwrite( &n_values, sizeof( int ), 1, file.sid );
  }
}

void
nest::SIONLogger::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double_t >& values )
{
  int task = device.get_vp();
  int gid = device.get_gid();

  const int sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  FileEntry& file = files_[ task ];
  SIONBuffer& buffer = file.buffer;

  devices_.find( task )->second.find( gid )->second.info.n_rec++;

  double time = stamp.get_ms() - offset;
  int n_values = values.size();

  unsigned int required_space = 3 * sizeof( int ) + ( 1 + n_values ) * sizeof( double );
  if ( buffer.get_capacity() > required_space )
  {
    if ( buffer.get_free() < required_space )
    {
      sion_fwrite( buffer.read(), buffer.get_size(), 1, file.sid );
      buffer.clear();
    }

    buffer << gid << sender << time << n_values;
    for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
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

    sion_fwrite( &gid, sizeof( int ), 1, file.sid );
    sion_fwrite( &sender, sizeof( int ), 1, file.sid );
    sion_fwrite( &time, sizeof( double ), 1, file.sid );
    sion_fwrite( &n_values, sizeof( int ), 1, file.sid );

    for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
    {
      double value = *val;
      sion_fwrite( &value, sizeof( double ), 1, file.sid );
    }
  }
}

const std::string
nest::SIONLogger::build_filename_() const
{
  std::ostringstream basename;
  const std::string& path = Node::network()->get_data_path();
  if ( !path.empty() )
    basename << path << '/';
  basename << Node::network()->get_data_prefix();

  basename << "output";

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Buffer
 * ---------------------------------------------------------------- */


nest::SIONLogger::SIONBuffer::SIONBuffer()
  : buffer( NULL )
  , ptr( 0 )
  , max_size( 0 )
{
}

nest::SIONLogger::SIONBuffer::SIONBuffer( int size )
  : buffer( NULL )
  , ptr( 0 )
{
  if ( size > 0 )
  {
    buffer = new char[ size ];
    max_size = size;
  }
  max_size = 0;
}

nest::SIONLogger::SIONBuffer::~SIONBuffer()
{
  if ( buffer != NULL )
    delete[] buffer;
}

void
nest::SIONLogger::SIONBuffer::reserve( int size )
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
nest::SIONLogger::SIONBuffer::write( const char* v, long unsigned int n )
{
  if ( ptr + n <= max_size )
  {
    memcpy( buffer + ptr, v, n );
    ptr += n;
  }
  else
  {
    std::cerr << "SIONBuffer: buffer overflow: ptr=" << ptr << " n=" << n
              << " max_size=" << max_size << std::endl;
  }
}

int
nest::SIONLogger::SIONBuffer::get_size()
{
  return ptr;
}

int
nest::SIONLogger::SIONBuffer::get_capacity()
{
  return max_size;
}

int
nest::SIONLogger::SIONBuffer::get_free()
{
  return ( max_size - ptr );
}

void
nest::SIONLogger::SIONBuffer::clear()
{
  ptr = 0;
}

char*
nest::SIONLogger::SIONBuffer::read()
{
  return buffer;
}

template < typename T >
nest::SIONLogger::SIONBuffer& nest::SIONLogger::SIONBuffer::operator<<( const T data )
{
  write( ( const char* ) &data, sizeof( T ) );
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::SIONLogger::Parameters_::Parameters_()
  : file_ext_( "dat" )
  , sion_buffer_size_( 2400 )
  , buffer_size_( 1024 )
{
}

void
nest::SIONLogger::Parameters_::get( const SIONLogger& al, DictionaryDatum& d ) const
{
  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::buffer_size ] = buffer_size_;
  ( *d )[ names::sion_buffer_size ] = sion_buffer_size_;
}

void
nest::SIONLogger::Parameters_::set( const SIONLogger& al, const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  updateValue< long >( d, names::sion_buffer_size, sion_buffer_size_ );
  updateValue< long >( d, names::buffer_size, buffer_size_ );
}

void
nest::SIONLogger::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
