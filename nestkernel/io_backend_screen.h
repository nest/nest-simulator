/*
 *  io_backend_screen.h
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

#ifndef IO_BACKEND_SCREEN_H
#define IO_BACKEND_SCREEN_H

#include "io_backend.h"

namespace nest
{

/**
 * A simple IO backend implementation that prints all recorded data to screen.
 */
class IOBackendScreen : public IOBackend
{
public:
  /**
   * IOBackendScreen constructor
   * The actual initialization is happening in IOBackend::initialize()
   */
  IOBackendScreen()
  {
  }

  /**
   * IOBackendScreen destructor
   * The actual finalization is happening in IOBackend::finalize()
   */
  ~IOBackendScreen() throw()
  {
  }

  /**
   * Functions called by all instantiated recording devices to register themselves with their
   * metadata.
   * Both functions are implemented trivially, since the IOBackendScreen does not handle metadata.
   */
  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  /**
   * Initialization function. In this case, only floating point precision of the standard output
   * stream is configured according to the parameters.
   */
  void initialize();
  /**
   * Finalization function. Nothing has to be finalized in case of the IOBackendScreen.
   */
  void finalize();
  /**
   * Synchronization function called at the end of each time step.
   * Again, the IOBackendScreen is not doing anything in this function.
   */
  void synchronize();

  /**
   * Write functions simply dumping all recorded data to standard output.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

private:
  struct Parameters_
  {
    long precision_;

    Parameters_();

    void get( const IOBackendScreen&, DictionaryDatum& ) const;
    void set( const IOBackendScreen&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline void
IOBackendScreen::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // IO_BACKEND_SCREEN_H
