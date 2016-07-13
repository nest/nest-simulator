/*
 *  io_backend_sion.h
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

#ifndef IO_BACKEND_SION_H
#define IO_BACKEND_SION_H

#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

#include "io_backend.h"

#include "mpi.h"
#include "sion.h"

namespace nest
{
class IOBackendSION : public IOBackend
{
public:
  IOBackendSION()
    : files_()
    , initialized_( false )
  {
  }
  
  IOBackendSION( std::string file_ext,
    long buffer_size,
    long sion_chunksize,
    bool sion_collective,
    bool close_after_simulate )
    : files_()
    , initialized_( false )
  {
    P_.file_ext_ = file_ext;
    P_.buffer_size_ = buffer_size;
    P_.sion_chunksize_ = sion_chunksize;
    P_.sion_collective_ = sion_collective;
    P_.close_after_simulate_ = close_after_simulate;
  }

  ~IOBackendSION() throw()
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
  void close_files_();
  const std::string build_filename_() const;

  class SIONBuffer
  {
  private:
	// TODO: add underscores
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

    index gid;
    unsigned int type;
    std::string name;
    std::string label;
    unsigned long int n_rec;
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
    sion_int64 body_blk, info_blk;
    sion_int64 body_pos, info_pos;

    double t_start, t_end, resolution;
  };

  struct FileEntry
  {
    int sid;
    SIONBuffer buffer;
    FileInfo info;
  };

  typedef std::map< thread, std::map< index, DeviceEntry > > device_map;
  device_map devices_;

  typedef std::map< thread, FileEntry > file_map;
  file_map files_;

  struct Parameters_
  {
    std::string file_ext_;      //!< the file name extension to use, without .
    long buffer_size_;          //!< the size of the internal buffer .
    long sion_chunksize_;       //!< the size of SIONlib's buffer .
    bool sion_collective_;      //!< use SIONlib's collective mode .
    bool close_after_simulate_; //!< if true, finalize() shall close the stream

    Parameters_();

    void get( const IOBackendSION&, DictionaryDatum& ) const;
    void set( const IOBackendSION&, const DictionaryDatum& );
  };

  Parameters_ P_;

  bool initialized_;
};

inline void
IOBackendSION::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // IO_BACKEND_SION_H
