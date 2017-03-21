/*
 *  recording_backend_memory.h
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

#ifndef RECORDING_BACKEND_MEMORY_H
#define RECORDING_BACKEND_MEMORY_H

#include "recording_backend.h"

namespace nest
{

/**
 * Memory specialization of the RecordingBackend interface.
 * Recorded data is written to plain text files on a per-device-per-thread basis.
 * Some formatting options are available to allow some compatibility to legacy NEST output files.
 *
 * RecordingBackendMemory maintains a data structure mapping one file stream to every recording device instance
 * on every thread. Files are opened and inserted into the map during the initialize() call and
 * closed in finalize().
 */
class RecordingBackendMemory : public RecordingBackend
{
public:
  /**
   * RecordingBackendMemory constructor.
   * The actual setup is done in initialize().
   */
  RecordingBackendMemory()
    : data_()
  {
  }

  /**
   * RecordingBackendMemory descructor.
   */
  ~RecordingBackendMemory() throw()
  {
    // remaining files are not closed here but should be handled gracefully on NEST shutdown.
  }

  /**
   * Functions called by all instantiated recording devices to register themselves with their
   * metadata.
   */
  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  /**
   * Initialize the RecordingBackendMemory during simulation preparation.
   */
  void initialize();

  /**
   * Finalize the RecordingBackendMemory after the simulation has finished.
   */
  void finalize();

  /**
   * Trivial synchronization function. The RecordingBackendMemory does not need explicit synchronization after
   * each time step.
   */
  void synchronize();

  /**
   * Functions to write data to file.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

private:

  // one map for each virtual process,
  // in turn containing one ostream for everydevice
  // vp -> (gid -> [device, filestream])
  typedef std::map< int, std::map< int, std::pair< RecordingDevice*, std::vector< double > > > > data_map;
  data_map data_;
};

inline void
RecordingBackendMemory::get_status( DictionaryDatum& d ) const
{
//  P_.get( *this, d );
}

} // namespace

#endif // RECORDING_BACKEND_MEMORY_H
