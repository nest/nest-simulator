/*
 *  recording_backend_soundclick.cpp
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

// Includes from nestkernel
#include "recording_device.h"

// Own includes
#include "recording_backend_soundclick.h"


// Constructor
nest::RecordingBackendSoundClick::RecordingBackendSoundClick()
{
  // Load the raw sound data into the SFML sound buffer.
  sound_buffer_.loadFromMemory(
    sound_click_16bit_44_1khz_wav, sizeof( sound_click_16bit_44_1khz_wav ) );
  sound_.setBuffer( sound_buffer_ );
}

// Destructor
nest::RecordingBackendSoundClick::~RecordingBackendSoundClick() throw()
{
  finalize();
}

// Called by each spike detector during network calibration before
// simulation run
void
nest::RecordingBackendSoundClick::enroll( const RecordingDevice& device )
{
  // Start or resume, repectively, the stopwatch, which represents the
  // real time.
  stopwatch_.start();
}

// Called by each multimeter during network calibration
void nest::RecordingBackendSoundClick::enroll( const RecordingDevice& device,
  const std::vector< Name >& ) // value_names, i.e. recordables
{
  throw BadProperty(
    "Only spike detectors can record to recording backend "
    ">SoundClick<" );
}

// Called on simulation startup
void
nest::RecordingBackendSoundClick::initialize()
{
  LOG( M_INFO,
    "Recording Backend",
    ( "recording backend >SoundClick< successfully initialized" ) );
}

// Called at the end of a call to Simulate
void
nest::RecordingBackendSoundClick::finalize()
{
  // Halt the stopwatch, which represents the real time.
  // It continues when the simulation continues, that is with the next call
  // to Simulate.
  stopwatch_.stop();
}

// Synchronize backend at the end of each simulation cycle
void
nest::RecordingBackendSoundClick::synchronize()
{
  // nothing to do
}

// Called by the spike detectors on every spike event
void
nest::RecordingBackendSoundClick::write( const RecordingDevice& device,
  const Event& event )
{
  // Calculate the time lag between real time (i.e., the stopwatch) and the
  // time of the spike event, and, if necessary, delay playing the sound.
  // This creates the illusion of a realistic sound from an
  // electrophysiological recording.

  // NOTE: This slows down the simulation to biological real time!

  int time_spike_event_us =
    static_cast< int >( floor( event.get_stamp().get_ms() * 1000.0 ) );
  int time_elapsed_us =
    static_cast< int >( floor( stopwatch_.elapsed_timestamp() ) );
  int time_lag_us = time_spike_event_us - time_elapsed_us;

  if ( time_lag_us > 0 )
  {
    usleep( time_lag_us );
  }

  sound_.play();
}

// Called by each multimeter on every event
void
nest::RecordingBackendSoundClick::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  // Must not happen !
  // Only spike detectors are allowed to connect to the SoundClick backend.
  throw;
}
