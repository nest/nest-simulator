#ifndef COLLECTIVE_SION_LOGGER_H
#define COLLECTIVE_SION_LOGGER_H

#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

#include "logger.h"

#include "mpi.h"
#include "sion.h"

namespace nest
{

class CollectiveSIONLogger : public Logger
{
public:
  CollectiveSIONLogger()
    : files_()
    , initialized_( false )
  {
  }

  CollectiveSIONLogger( size_t chunksize )
    : files_()
    , initialized_( false )
  {
    P_.sion_chunksize_ = chunksize;
  }

  ~CollectiveSIONLogger() throw()
  {
  }

  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  void initialize();
  void finalize();
  void synchronize();

  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

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
    void ensure_space( int size );
    void write( const char* v, long unsigned int n );
    int get_capacity();
    int get_size();
    int get_free();
    void clear();
    char* read();
    template < typename T >
    SIONBuffer& operator<<( const T data );
  };

  struct DeviceInfo
  {
    DeviceInfo()
      : n_rec( 0 )
    {
    }

    int gid;
    int type;
    std::string name;
    unsigned long n_rec;
    std::vector< std::string > value_names;
  };

  struct DeviceEntry
  {
    DeviceEntry( RecordingDevice& device )
      : device( device )
      , info()
    {
    }

    RecordingDevice& device;
    DeviceInfo info;
  };

  struct FileInfo
  {
    int body_blk, info_blk;
    sion_int64 body_pos, info_pos;

    double t_start, t_end, resolution;

    SIONBuffer buffer;
  };

  struct FileEntry
  {
    int sid;
    SIONBuffer buffer;
    FileInfo info;
  };

  typedef std::map< int, std::map< int, DeviceEntry > > device_map;
  device_map devices_;

  typedef std::map< int, FileEntry > file_map;
  file_map files_;

  struct Parameters_
  {
    std::string file_ext_; //!< the file name extension to use, without .
    long buffer_size_;     //!< the size of the internal buffer .
    long sion_chunksize_;  //!< the size of SIONlib's buffer .

    Parameters_();

    void get( const CollectiveSIONLogger&, DictionaryDatum& ) const;
    void set( const CollectiveSIONLogger&, const DictionaryDatum& );
  };

  Parameters_ P_;

  bool initialized_;
};

inline void
CollectiveSIONLogger::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // COLLECTIVE_SION_LOGGER_H
