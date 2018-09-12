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

#ifndef RECORDING_BACKEND_ARBOR_H
#define RECORDING_BACKEND_ARBOR_H

// C includes:
#include <mpi.h>

#include "recording_backend.h"

namespace nest
{

class RecordingBackendArbor : public RecordingBackend
{
public:
  RecordingBackendArbor();

  ~RecordingBackendArbor() throw();

  void enroll( const RecordingDevice& device );
  void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names );

  void finalize();
  void synchronize();

  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  void initialize();
  void calibrate();

private:
  void open_files_();
  void close_files_();
  const std::string build_filename_() const;

  bool files_opened_;

  class ArborBuffer
  {
  private:

  public:
    ArborBuffer();
    ~ArborBuffer();

    void write(/*something*/);
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
    DeviceEntry( const RecordingDevice& device )
      : device( device )
      , info()
    {
    }

    const RecordingDevice& device;
    DeviceInfo info;
  };

  typedef std::vector< std::map< index, DeviceEntry > > device_map;
  device_map devices_;

  MPI_Comm local_comm_;    // single copy of local MPI communicator
                           // for all threads using the sionlib
                           // recording backend in parallel (for broadcasting
                           // the results of MPIX..(..) in open_files_(..))

  double t_start_; // simulation start time for storing

  struct Parameters_
  {
    Parameters_();

    void get( const RecordingBackendArbor&, DictionaryDatum& ) const;
    void set( const RecordingBackendArbor&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline void
RecordingBackendArbor::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // RECORDING_BACKEND_ARBOR_H
