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
  RecordingBackendScreen()
  {
  }

  ~RecordingBackendScreen() throw()
  {
  }

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

  void set_status( const DictionaryDatum& );

  void get_status( DictionaryDatum& ) const override;

  void get_device_status( const RecordingDevice& device, DictionaryDatum& ) const override;

  void set_device_status( const RecordingDevice& device, const DictionaryDatum& d ) override;

private:
  struct DeviceData
  {
    DeviceData();
    void get_status( DictionaryDatum& ) const;
    void set_status( const DictionaryDatum& );
    void write( const Event&, const std::vector< double >&, const std::vector< long >& );
  private:
    void prepare_cout_();
    void restore_cout_();
    std::ios::fmtflags old_fmtflags_;
    long old_precision_;
    long precision_;      //!< Number of decimal places used when writing decimal values
    bool time_in_steps_;  //!< Should time be recorded in steps (ms if false)
  };

  typedef std::vector< std::map< size_t, DeviceData > > device_data_map;
  device_data_map device_data_;
};

} // namespace

#endif // RECORDING_BACKEND_SCREEN_H
