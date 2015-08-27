#ifndef SION_LOGGER_H
#define SION_LOGGER_H

#include <fstream>
#include <algorithm>

#include "logger.h"

#include "mpi.h"
#include "sion.h"

namespace nest
{

class SIONLogger : public Logger
{
public:
  SIONLogger()
    : files_()
    , initialized_( false )
  {
  }

  ~SIONLogger() throw()
  {
  }

  void enroll( const int virtual_process, RecordingDevice& device );
  void initialize();
  void finalize();
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

private:
  const std::string build_filename_() const;

  class SIONBuffer
  {
  private:
    char* buffer;
    int ptr;
    int max_size;

  public:
    SIONBuffer()
      : buffer( NULL )
      , ptr( 0 )
      , max_size( 0 )
    {
    }

    SIONBuffer( int size )
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

    ~SIONBuffer()
    {
      if ( buffer != NULL )
        delete[] buffer;
    }

    void
    reserve( int size )
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
    write( const char* v, long unsigned int n )
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
    get_size()
    {
      return ptr;
    }

    int
    get_capacity()
    {
      return max_size;
    }

    void
    ensure_free_space( int size )
    {
      if ( !has_free_space( size ) )
      {
        // TODO: What? +10*size?
        int new_max_size = max_size + size * 10;
        reserve( max_size + size * 10 );
      }
    }

    bool
    has_free_space( int size )
    {
      return ( ptr + size < max_size );
    }

    int
    get_free()
    {
      return ( max_size - ptr );
    }

    void
    clear()
    {
      ptr = 0;
    }

    char*
    read()
    {
      return buffer;
    }

    template < typename T >
    SIONBuffer& operator<<( const T data )
    {
      write( ( const char* ) &data, sizeof( T ) );
      return *this;
    }
  };

  struct DeviceHeader
  {
    int device_id;
    int n_values;
    std::vector< std::string > value_names;
  };

  struct FileHeader
  {
    int n_devices;
    std::vector< DeviceHeader > device_headers;
  };

  struct DeviceEntry
  {
    DeviceEntry( RecordingDevice& device )
      : device( device )
      , n_rec( 0 )
    {
    }

    RecordingDevice& device;
    unsigned long n_rec;
  };

  struct VirtualProcessEntry
  {
    int sid;
    int body_blk, info_blk;
    sion_int64 body_pos, info_pos;

    double t_start, t_end, resolution;

    SIONBuffer buffer;
    typedef std::map< int, DeviceEntry > device_map;
    device_map devices;
  };

  struct Parameters_
  {
    std::string file_ext_;      //!< the file name extension to use, without .
    long buffer_size_;          //!< the size of the internal buffer .
    long sion_buffer_size_;     //!< the size of SIONlib's buffer .

    Parameters_();

    void get( const SIONLogger&, DictionaryDatum& ) const;
    void set( const SIONLogger&, const DictionaryDatum& );
  };

  Parameters_ P_;

  // one map for each virtual process,
  // in turn containing one ostream for everydevice
  typedef std::map< int, VirtualProcessEntry > file_map;
  file_map files_;

  bool initialized_;
};

} // namespace

#endif // SION_LOGGER_H
