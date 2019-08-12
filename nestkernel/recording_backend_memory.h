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
 * JME: Explain data life span
 *
 */
class RecordingBackendMemory : public RecordingBackend
{
public:
  RecordingBackendMemory();
  ~RecordingBackendMemory() throw();

  void initialize() override;
  void finalize() override;

  void enroll( const RecordingDevice& device ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names, const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void write( const RecordingDevice&, const Event&,
    const std::vector< double >&, const std::vector< long >& ) override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void set_device_status( const RecordingDevice& device, const DictionaryDatum& ) override;

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
    std::vector< long > senders_;                         //!< sender gids of the events
    std::vector< double > times_ms_;                      //!< times of registered events in ms
    std::vector< long > times_steps_;                     //!< times of registered events in steps
    std::vector< double > times_offset_;                  //!< offsets of registered events if time_in_steps_
    std::vector< Name > double_value_names_;              //!< names for values of type double
    std::vector< Name > long_value_names_;                //!< names for values of type long
    std::vector< std::vector< double > > double_values_;  //!< recorded values of type double, one vector per time
    std::vector< std::vector< long > > long_values_;      //!< recorded values of type long, one vector per time
    bool time_in_steps_;                                  //!< Should time be recorded in steps (ms if false)
  };

  typedef std::vector< std::map< size_t, DeviceData > > device_data_map;
  device_data_map device_data_;
};

} // namespace

#endif // RECORDING_BACKEND_MEMORY_H
