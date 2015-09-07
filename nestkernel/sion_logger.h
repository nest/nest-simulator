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
  void enroll( const int task,
    RecordingDevice& device,
    const std::vector< Name >& value_names );
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
    std::string file_ext_;  //!< the file name extension to use, without .
    long buffer_size_;      //!< the size of the internal buffer .
    long sion_buffer_size_; //!< the size of SIONlib's buffer .

    Parameters_();

    void get( const SIONLogger&, DictionaryDatum& ) const;
    void set( const SIONLogger&, const DictionaryDatum& );
  };

  Parameters_ P_;

  bool initialized_;
};

} // namespace

#endif // SION_LOGGER_H
