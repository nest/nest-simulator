/*
 *  recording_backend_screen.h
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

#ifndef RECORDING_BACKEND_SCREEN_H
#define RECORDING_BACKEND_SCREEN_H

#include "recording_backend.h"

namespace nest
{

/**
 * A simple recording backend implementation that prints all recorded data to screen.
 */
class RecordingBackendScreen : public RecordingBackend
{
public:
  /**
   * RecordingBackendScreen constructor
   * The actual initialization is happening in RecordingBackend::initialize()
   */
  RecordingBackendScreen()
  {
  }

  /**
   * RecordingBackendScreen destructor
   * The actual finalization is happening in RecordingBackend::finalize()
   */
  ~RecordingBackendScreen() throw()
  {
  }

  /**
   * Functions called by all instantiated recording devices to register themselves with their
   * metadata.
   * Both functions are implemented trivially, since the RecordingBackendScreen does not handle metadata.
   */
  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  /**
   * Initialization function. In this case, only floating point precision of the standard output
   * stream is configured according to the parameters.
   */
  void initialize();
  /**
   * Finalization function. Nothing has to be finalized in case of the RecordingBackendScreen.
   */
  void finalize();
  /**
   * Synchronization function called at the end of each time step.
   * Again, the RecordingBackendScreen is not doing anything in this function.
   */
  void synchronize();

  /**
   * Write functions simply dumping all recorded data to standard output.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

private:
  struct Parameters_
  {
    long precision_;

    Parameters_();

    void get( const RecordingBackendScreen&, DictionaryDatum& ) const;
    void set( const RecordingBackendScreen&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline void
RecordingBackendScreen::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // RECORDING_BACKEND_SCREEN_H
