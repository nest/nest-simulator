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

  void enroll( const int task, RecordingDevice& device );
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
    SIONBuffer();
    SIONBuffer( int size );
    ~SIONBuffer();
    void reserve( int size );
    void write( const char* v, long unsigned int n );
    int get_capacity();
    int get_size();
    int get_free();
    void clear();
    char* read();
    template < typename T >
    SIONBuffer& operator<<( const T data );
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
    std::string file_ext_;  //!< the file name extension to use, without .
    long buffer_size_;      //!< the size of the internal buffer .
    long sion_buffer_size_; //!< the size of SIONlib's buffer .

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
