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
  sound_buffer_.loadFromMemory( sound_click_16bit_44_1khz_wav, sizeof( sound_click_16bit_44_1khz_wav ) );
  sound_.setBuffer( sound_buffer_ );
}

nest::RecordingBackendSoundClick::~RecordingBackendSoundClick() throw()
{
  cleanup();
}

void
nest::RecordingBackendSoundClick::initialize()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::finalize()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::enroll( const RecordingDevice& device const DictionaryDatum& )
{
  if ( device.get_type() != RecordingDevice::SPIKE_DETECTOR )
  {
    throw BadProperty( "Only spike detectors can record to recording backend 'SoundClick'" );
  }
}

void
nest::RecordingBackendSoundClick::disenroll( const RecordingDevice& device )
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::set_value_names( const RecordingDevice&,
  const std::vector< Name >&,
  const std::vector< Name >& )
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::pre_run_hook()
{
  // nothing to do
}


void
nest::RecordingBackendSoundClick::post_run_hook()
{
  // nothing to do
}


void
nest::RecordingBackendSoundClick::post_step_hook()
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::prepare()
{
  stopwatch_.start();
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

void
nest::RecordingBackendSoundClick::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  assert( device.get_type() == RecordingDevice::SPIKE_DETECTOR );

  // Calculate the time lag between real time (i.e., the stopwatch) and the
  // time of the spike event, and, if necessary, delay playing the sound.
  // This creates the illusion of a realistic sound from an
  // electrophysiological recording.

  int time_spike_event_us = static_cast< int >( floor( event.get_stamp().get_ms() * 1000.0 ) );
  int time_elapsed_us = static_cast< int >( floor( stopwatch_.elapsed_timestamp() ) );
  int time_lag_us = time_spike_event_us - time_elapsed_us;

  // Slow down the simulation to biological real time!
  if ( time_lag_us > 0 )
  {
    usleep( time_lag_us );
  }

  sound_.play();
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
nest::RecordingBackendSoundClick::check_device_status( const DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::get_device_defaults( DictionaryDatum& ) const
{
  // nothing to do
}

void
nest::RecordingBackendSoundClick::get_device_status( const RecordingDevice&, DictionaryDatum& ) const
{
  // nothing to do
}
