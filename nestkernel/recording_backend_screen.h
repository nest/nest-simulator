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
#include <set>

namespace nest
{

/**
 * A simple recording backend implementation that prints all recorded data to
 * screen.
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

  void enroll( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  /**
   * Finalization function. Nothing has to be finalized in case of the
   * RecordingBackendScreen.
   */
  void finalize();

  /**
   * Synchronization function called at the end of each time step.
   * Again, the RecordingBackendScreen is not doing anything in this function.
   */
  void synchronize();

  void write( const RecordingDevice&,
    const Event&,
    const std::vector< double >&,
    const std::vector< long >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  /**
   * Initialization function.
   */
  void initialize();

private:
  struct Parameters_
  {
    long precision_;

    Parameters_();

    void get( const RecordingBackendScreen&, DictionaryDatum& ) const;
    void set( const RecordingBackendScreen&, const DictionaryDatum& );
  };

  Parameters_ P_;

  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its recordings.
  */
  typedef std::vector< std::set< index > > enrollment_map;
  enrollment_map enrolled_devices_;

  void prepare_cout_();
  void restore_cout_();

  std::ios::fmtflags old_fmtflags_;
  long old_precision_;
};

inline void
RecordingBackendScreen::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}


inline void
RecordingBackendScreen::prepare_cout_()
{
  old_fmtflags_ = std::cout.flags( std::ios::fixed );
  old_precision_ = std::cout.precision( P_.precision_ );
}

inline void
RecordingBackendScreen::restore_cout_()
{
  std::cout.flags( old_fmtflags_ );
  std::cout.precision( old_precision_ );
}

} // namespace

#endif // RECORDING_BACKEND_SCREEN_H
