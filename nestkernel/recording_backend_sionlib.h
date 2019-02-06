/*
 *  recording_backend_sionlib.h
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

#ifndef RECORDING_BACKEND_SIONLIB_H
#define RECORDING_BACKEND_SIONLIB_H

// C includes:
#include <mpi.h>
#include <sion.h>

#include "recording_backend.h"

namespace nest
{

class RecordingBackendSIONlib : public RecordingBackend
{
public:
  const static unsigned int SIONLIB_REC_BACKEND_VERSION;
  const static unsigned int DEV_NAME_BUFFERSIZE;
  const static unsigned int DEV_LABEL_BUFFERSIZE;
  const static unsigned int VALUE_NAME_BUFFERSIZE;
  const static unsigned int NEST_VERSION_BUFFERSIZE;

  RecordingBackendSIONlib();

  ~RecordingBackendSIONlib() throw();

  void enroll( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  void finalize();
  void synchronize();

  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& double_values,
    const std::vector< long >& long_values );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  void initialize();
  void calibrate();

private:
  void open_files_();
  void close_files_();
  const std::string build_filename_() const;

  bool files_opened_;

  class SIONBuffer
  {
  private:
    char* buffer_;
    size_t ptr_;
    size_t max_size_;

  public:
    SIONBuffer();
    SIONBuffer( size_t size );
    ~SIONBuffer();

    void reserve( size_t size );
    void ensure_space( size_t size );
    void write( const char* v, size_t n );

    size_t
    get_capacity()
    {
      return max_size_;
    };

    size_t
    get_size()
    {
      return ptr_;
    };

    size_t
    get_free()
    {
      return max_size_ - ptr_;
    };

    void
    clear()
    {
      ptr_ = 0;
    };

    char*
    read()
    {
      return buffer_;
    };

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
    std::vector< std::string > double_value_names;
    std::vector< std::string > long_value_names;
  };

  struct DeviceEntry
  {
    DeviceEntry( const RecordingDevice& device )
      : device( device )
      , info()
    {
    }

    const RecordingDevice& device;
    DeviceInfo info;
  };

  struct FileEntry
  {
    int sid;
    SIONBuffer buffer;
  };

  typedef std::vector< std::map< index, DeviceEntry > > device_map;
  device_map devices_;

  typedef std::map< thread, FileEntry > file_map;
  file_map files_;

  std::string filename_;
  MPI_Comm local_comm_;    // single copy of local MPI communicator
                           // for all threads using the sionlib
                           // recording backend in parallel (for broadcasting
                           // the results of MPIX..(..) in open_files_(..))

  double t_start_; // simulation start time for storing

  struct Parameters_
  {
    std::string file_ext_; //!< the file name extension to use, without .
    bool sion_collective_; //!< use SIONlib's collective mode.
    long sion_chunksize_;  //!< the size of SIONlib's buffer.
    int sion_n_files_;     //!< the number of SIONLIB container files automatically used.
    long buffer_size_;     //!< the size of the internal buffer.

    Parameters_();

    void get( const RecordingBackendSIONlib&, DictionaryDatum& ) const;
    void set( const RecordingBackendSIONlib&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline void
RecordingBackendSIONlib::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );

  ( *d )[ names::filename ] = filename_;
}

} // namespace

#endif // RECORDING_BACKEND_SIONLIB_H
