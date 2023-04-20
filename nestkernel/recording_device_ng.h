/*
 *  recording_device_ng.h
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

#ifndef RECORDING_DEVICE_NG_H
#define RECORDING_DEVICE_NG_H

// C++ includes:
#include <fstream>
#include <vector>

// Includes from nestkernel:
#include "device.h"

#include "kernel_manager.h"
#include "nest_object_interface.h"
#include "nest_types.h"
#include "recording_backend.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{


class RecordingDeviceNG : public NESTObjectInterface, public Device
{
public:

  RecordingDeviceNG();
  RecordingDeviceNG( const RecordingDevice& );

  using Device::pre_run_hook;

  void pre_run_hook( const std::vector< Name >&, const std::vector< Name >& );

  bool is_active( Time const& T ) const override;

  const std::string& get_label() const;

  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) const override;

protected:
  void write( const Event&, const std::vector< double >&, const std::vector< long >& );
  void set_initialized_();

private:
  struct Parameters_
  {
    std::string label_; //!< A user-defined label for symbolic device names.
    Name record_to_;    //!< The name of the recording backend to use

    Parameters_();
    Parameters_( const Parameters_& ) = default;
    Parameters_& operator=( const Parameters_& ) = default;
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  } P_;

  struct State_
  {
    size_t n_events_; //!< The number of events recorded by the device.

    State_();
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  } S_;

  DictionaryDatum backend_params_;
};

} // namespace

#endif // RECORDING_DEVICE_H
