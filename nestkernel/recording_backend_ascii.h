/*
 *  recording_backend_ascii.h
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

#ifndef RECORDING_BACKEND_ASCII_H
#define RECORDING_BACKEND_ASCII_H

#include "recording_backend.h"

namespace nest
{

/**
 * ASCII specialization of the RecordingBackend interface.
 *
 * Recorded data is written to plain text files on a
 * per-device-per-thread basis.
 *
 * RecordingBackendASCII maintains a data structure mapping one file
 * stream to every recording device instance on every thread. Files
 * are opened and inserted into the map during the enroll() call
 * (issued by the recorder's calibrate() function) and closed in
 * finalize(), which is called on all registered recording backends by
 * IOManager::cleanup().
 */
class RecordingBackendASCII : public RecordingBackend
{
public:
  RecordingBackendASCII();

  ~RecordingBackendASCII() throw();

  void enroll( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  /**
   * Flush files after a single call to Run
   */
  void post_run_cleanup();

  /**
   * Finalize the RecordingBackendASCII after the simulation has finished.
   */
  void finalize();

  /**
   * Trivial synchronization function. The RecordingBackendASCII does
   * not need explicit synchronization after each time step.
   */
  void synchronize();

  void write( const RecordingDevice&,
    const Event&,
    const std::vector< double >&,
    const std::vector< long >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  /**
   * Initialize the RecordingBackendASCII during simulation preparation.
   */
  void initialize();

  void get_device_status( const RecordingDevice& device,
    DictionaryDatum& ) const;

private:
  /**
   * Build device filename.
   * The filename consists of the data path set in IOManager, the
   * device's label (or name as a fallback if no label is given),
   * the device GID, and the virtual process ID, all separated by
   * dashes, followed by the filename extension file_ext.
   */
  const std::string build_filename_( const RecordingDevice& device ) const;

  struct Parameters_
  {
    long precision_;       //!< Number of decimal places to use for values
    std::string file_ext_; //!< File name extension to use, without leading "."

    Parameters_();

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  Parameters_ P_;

  /**
   * A map for the data files.  We have a vector with one map per
   * local thread. The map associates the gid of a device on a given
   * thread with the file name and a pointer to the file stream
   *
   * vp -> ( gid -> [ file_name, file_stream ] )
  */
  typedef std::vector< std::map< size_t,
    std::pair< std::string, std::ofstream* > > > file_map;
  file_map files_;
};

inline void
RecordingBackendASCII::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
}

} // namespace

#endif // RECORDING_BACKEND_ASCII_H
