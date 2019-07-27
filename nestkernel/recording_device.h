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

// Includes from nestkernel:
#include "node.h"
#include "device.h"
#include "device_node.h"
#include "recording_backend.h"
#include "nest_types.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/**
 * Base class for all recording devices.
 *
 * Recording devices collect data and output it to one or more
 * recording backends. This class provides for time windowing of data
 * registration, temporary storage of data and output of data to
 * files.
 *
 * If the device is configured to record from start to stop, this
 * is interpreted as (start, stop], i.e., the earliest recorded
 * event will have time stamp start+1, as it was generated during
 * the update step (start, start+1].
 *
 * Class RecordingDevice by itself manages the identity and time
 * of the recorded events, including precise event times, but not
 * any additional data about the events. Use class AnalogRecordingDevice
 * if you need to store additional data; that class also provides
 * sampling at a given interval.
 *
 * @note The RecordingDevice class breaks the general persistence rules
 *       with respect to Parameters, State, Buffers and Variables. The
 *       central problem is that changes to /data_prefix in the root node
 *       require us to close output file streams and re-open them under
 *       new names. The only way to detect such changes is by comparing the
 *       current file name with a filename constructed anew upon each
 *       call to calibrate(). We cannot use init_buffers() here, since it
 *       is called only once after ResetNetwork. Thus, even though the file
 *       stream is a Buffer, we need to place all file opening in calibrate().
 *       init_buffers() merely closes the stream if close_on_reset_ is true.
 *
 *  @todo Some aspects of RecordingDevice behavior depend on the type of device:
 *        Multimeter needs to have its data cleared on n_events==0 and provides
 *        an accumulator mode which is administered by RecordingDevice. To tell
 *        recording device about this deviating behavior, we mark the type of
 *        "owning device" with an enum flag on construction. This is not very
 *        clean and should probably be solved by subclassing instead.
 *
 * @ingroup Devices
 *
 * @author HEP 2002-07-22, 2008-03-21, 2011-02-11
 */

class RecordingDevice : public DeviceNode, public Device
{
public:

  RecordingDevice( );
  RecordingDevice( const RecordingDevice& );
    
  void
  init_state( const RecordingDevice& pr )
  {
    Device::init_state( pr );
    S_ = pr.S_;
  }


    // remove, as this anyway happens due to the inheritance RD->D
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
    MULTIMETER,
    SPIKE_DETECTOR,
    SPIN_DETECTOR,
    WEIGHT_RECORDER
  };
  virtual Type get_type() const = 0;

  const std::string&
  get_label() const
  {
    return P_.label_;
  }

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

protected:
  void write( const Event&, const std::vector< double >&, const std::vector< long >& );
  void enroll( const std::vector< Name >&, const std::vector< Name >& );

private:
  struct Parameters_
  {
    std::string label_;    //!< A user-defined label for symbolic device names.
    bool time_in_steps_;   //!< Should time be recorded in steps (ms if false)
    ArrayDatum record_to_; //!< Array of recording backends to use

    Parameters_();
    Parameters_( const Parameters_& );
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

  for ( auto& backend_token : P_.record_to_ )
  {
    Name backend_name( getValue< std::string >( backend_token ) );
    kernel().io_manager.get_recording_device_status( backend_name, *this, d );
  }
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

inline void
RecordingDevice::write( const Event& event,
  const std::vector< double >& double_values,
  const std::vector< long >& long_values )
{
  // JME: The number of events needs to be stored and handled on a
  // per-recording_device basis only in the memory backend
  ++S_.n_events_;
  for ( auto& backend_token : P_.record_to_ )
  {
    Name backend_name( getValue< std::string >( backend_token ) );
    kernel().io_manager.write( backend_name, *this, event, double_values, long_values );
  }
}

inline void
RecordingDevice::enroll( const std::vector< Name >& double_value_names, const std::vector< Name >& long_value_names )
{
  // JME: also handle disenroll here by running a loop over all backends
  // and calling enroll_recorder() for the ones that are in the
  // record_to_ vector and disenroll_recorder() for all that aren't.
  for ( auto& backend_token : P_.record_to_ )
  {
    Name backend_name( getValue< std::string >( backend_token ) );
    kernel().io_manager.enroll_recorder( backend_name, *this, double_value_names, long_value_names );
  }
}

} // namespace

#endif // RECORDING_DEVICE_H
