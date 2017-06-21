/*
 *  recording_device.h
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

#ifndef RECORDING_DEVICE_H
#define RECORDING_DEVICE_H

// C++ includes:
#include <fstream>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from nestkernel:
#include "node.h"
#include "device.h"
#include "recording_backend.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

class RecordingDevice : public Node, public Device
{
public:
  void
  init_parameters( const RecordingDevice& pr )
  {
    Device::init_parameters( pr );

    P_ = pr.P_;
    S_ = pr.S_;
  }

  void
  init_state( const RecordingDevice& pr )
  {
    Device::init_state( pr );
    S_ = pr.S_;
  }

  void
  init_buffers()
  {
    Device::init_buffers();
  }

  void
  calibrate()
  {
    Device::calibrate();
  }

  bool is_active( Time const& T ) const;

  bool get_time_in_steps() const;

  /**
   * Device type.
   */
  enum Type
  {
    SPIKE_DETECTOR,
    MULTIMETER,
    SPIN_DETECTOR
  };
  virtual Type get_type() const = 0;

  const std::string&
  get_label() const
  {
    return P_.label_;
  }

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

private:
  struct Parameters_
  {
    std::string label_;    //!< A user-defined label for symbolic device names.
    bool time_in_steps_;   //!< Flag indicating if time is recorded in steps or ms

    Parameters_();
    void get( const RecordingDevice&, DictionaryDatum& ) const;
    void set( const RecordingDevice&, const DictionaryDatum&, long );
  };

  struct State_
  {
    size_t n_events_;

    State_();
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const RecordingDevice& );
  };

  Parameters_ P_;
  State_ S_;
};

inline void
RecordingDevice::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
  S_.get( d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::recorder );
}


inline bool
RecordingDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return get_t_min_() < stamp && stamp <= get_t_max_();
}

inline bool
RecordingDevice::get_time_in_steps() const
{
  return P_.time_in_steps_;
}

} // namespace

#endif // RECORDING_DEVICE_H
