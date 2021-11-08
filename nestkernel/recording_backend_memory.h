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

// Includes from nestkernel:
#include "recording_backend.h"

/* BeginUserDocs: NOINDEX

Recording backend `memory` - Store data in main memory
######################################################

Description
+++++++++++

When a recording device sends data to the ``memory`` backend, it is
stored internally in efficient vectors. These vectors are made
available to the user level in the device's status dictionary under
the key ``events``.

The ``events`` dictionary always contains the global IDs of the source
nodes of the recorded data in the field ``sender``. It also always
contains the time of the recording. Depending on the setting of the
property ``time_in_steps``, this time can be stored in two different
formats:

- If ``time_in_steps`` is `false` (which is the default), the time is
  stored as a single floating point number in the field ``times``,
  interpreted as the simulation time in ms

- If ``time_in_steps`` is `true`, the time is stored as a pair
  consisting of the integer number of simulation time steps in units
  of the simulation resolution in ``times`` and the negative offset from
  the next such grid point as a floating point number in ms in
  ``offset``.

All additional data collected or sampled by the recording device is
contained in the ``events`` dictionary in arrays. These data are named
based on the recordable they came from and with the appropriate data
type (either integer or floating point).

The number of events that were collected by the ``memory`` backend can
be read out of the `n_events` entry in the status dictionary of the
recording device. To delete data from memory, `n_events` can be set to
0. Other values cannot be set.

Parameter summary
+++++++++++++++++

events
    A dictionary containing the recorded data in the form of one numeric
    array for each quantity measured. It always has the sender global
    IDs of recorded events under the key ``senders`` and the time of the
    recording, the format of which depends on the setting of
    ``time_in_steps``.

n_events
    The number of events collected or sampled since the last reset of
    `n_events`. By setting `n_events` to 0, all events recorded so far
    will be discarded from memory.

time_in_steps
    A Boolean (default: *false*) specifying whether to store time in
    steps, i.e., in integer multiples of the simulation resolution
    (under the key ``times`` of the ``events`` dictionary) plus a
    floating point number for the negative offset from the next grid
    point in ms (under key ``offset``), or just the simulation time in
    ms under key ``times``. This property cannot be set after Simulate
    has been called.

EndUserDocs */

namespace nest
{

/**
 * Memory specialization of the RecordingBackend interface.
 *
 * Recorded data is stored in memory on a per-device-per-thread
 * basis. Setting the /n_events in the status dictionary of an
 * individual device will wipe the data for that device from memory.
 *
 * RecordingBackendMemory maintains a data structure mapping the data
 * vectors to every recording device instance on every thread. The
 * basic data structure is initialized during the initialize() call
 * and closed in finalize(). The concrete data vectors are added to
 * the basic data structure during the call to enroll(), when the
 * exact fields are known.
 *
 */
class RecordingBackendMemory : public RecordingBackend
{
public:
  RecordingBackendMemory();
  ~RecordingBackendMemory() throw();

  void initialize() override;
  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void write( const RecordingDevice&, const Event&, const std::vector< double >&, const std::vector< long >& ) override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void check_device_status( const DictionaryDatum& ) const override;
  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& ) const override;

private:
  struct DeviceData
  {
    DeviceData();
    void set_value_names( const std::vector< Name >&, const std::vector< Name >& );
    void push_back( const Event&, const std::vector< double >&, const std::vector< long >& );
    void get_status( DictionaryDatum& ) const;
    void set_status( const DictionaryDatum& );

  private:
    void clear();
    std::vector< long > senders_;                        //!< sender node IDs of the events
    std::vector< double > times_ms_;                     //!< times of registered events in ms
    std::vector< long > times_steps_;                    //!< times of registered events in steps
    std::vector< double > times_offset_;                 //!< offsets of registered events if time_in_steps_
    std::vector< Name > double_value_names_;             //!< names for values of type double
    std::vector< Name > long_value_names_;               //!< names for values of type long
    std::vector< std::vector< double > > double_values_; //!< recorded values of type double, one vector per value
    std::vector< std::vector< long > > long_values_;     //!< recorded values of type long, one vector per value
    bool time_in_steps_;                                 //!< Should time be recorded in steps (ms if false)
  };

  typedef std::vector< std::map< size_t, DeviceData > > device_data_map;
  device_data_map device_data_;
};

} // namespace

#endif // RECORDING_BACKEND_MEMORY_H
