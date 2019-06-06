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

nest::RecordingBackendSoundClick::RecordingBackendSoundClick()
{
  // Load the raw sound data into the SFML sound buffer.
  sound_buffer_.loadFromMemory(
    sound_click_16bit_44_1khz_wav, sizeof( sound_click_16bit_44_1khz_wav ) );
  sound_.setBuffer( sound_buffer_ );
}

nest::RecordingBackendSoundClick::~RecordingBackendSoundClick() throw()
{
  cleanup();
}

// Register a device to record data using a certain backend
// Called by each multimeter during network calibration
void
nest::RecordingBackendSoundClick::enroll( const RecordingDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  if ( device.get_type() == RecordingDevice::SPIKE_DETECTOR )
  {
    // Start or resume, respectively, the stopwatch, which represents the
    // real time.
    stopwatch_.start();
  }
  else
  {
    throw BadProperty(
      "Only spike detectors can record to recording backend "
      ">SoundClick<" );
  }
}

// Initialize global backend-specific data structures
// Called during simulation startup
void
nest::RecordingBackendSoundClick::pre_run_hook()
{
  LOG( M_INFO,
    "Recording Backend",
    ( "Recording backend >SoundClick< successfully initialized." ) );
}

// Clean up the backend at the end of a call to Simulate
void
nest::RecordingBackendSoundClick::cleanup()
{
  // Halt the stopwatch, which represents the real time.
  // It continues when the simulation continues, that is with the next call
  // to Simulate.
  stopwatch_.stop();
}

// Write the data from the spike-event to the backend specific channel
void
nest::RecordingBackendSoundClick::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  // Calculate the time lag between real time (i.e., the stopwatch) and the
  // time of the spike event, and, if necessary, delay playing the sound.
  // This creates the illusion of a realistic sound from an
  // electrophysiological recording.

  if ( device.get_type() == RecordingDevice::SPIKE_DETECTOR )
  {
    int time_spike_event_us =
      static_cast< int >( floor( event.get_stamp().get_ms() * 1000.0 ) );
    int time_elapsed_us =
      static_cast< int >( floor( stopwatch_.elapsed_timestamp() ) );
    int time_lag_us = time_spike_event_us - time_elapsed_us;

    // Slow down the simulation to biological real time!
    if ( time_lag_us > 0 )
    {
      usleep( time_lag_us );
    }

    sound_.

      play();
  }
  else
  {
    throw;
  }
}

// Synchronize backend at the end of each simulation cycle
void
nest::RecordingBackendSoundClick::synchronize()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::prepare()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::post_run_hook()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::clear( const RecordingDevice& )
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::set_status( const DictionaryDatum& )
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::get_status( DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::set_device_status(
  const RecordingDevice& device,
  const DictionaryDatum& d )
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::get_device_status(
  const RecordingDevice& device,
  DictionaryDatum& d ) const
{
  // nothing to do
}
