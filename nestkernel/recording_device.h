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
#include "device_node.h"
#include "recording_backend.h"
#include "nest_types.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/** @BeginDocumentation
  Name: RecordingDevice - Common properties of all recording devices.

  Description:

  Recording devices are used to measure properties of or signals emitted
  by network nodes, e.g., using a multimeter, voltmeter or a spike detector.

  Recording devices can collect data in memory, display it on the terminal
  output or write it to file in any combination. The output format can be
  controlled by device parameters as discussed below.

  Recording devices can be subdivided into two groups: collectors and samplers.
  - Collectors collect events sent to them; the spike detector is the
    archetypical example. Nodes are connected to collectors and the collector
    then collects all spikes emitted by the nodes connected to it and records
    them.
  - Samplers actively interrogate their targets at given time intervals
    (default: 1ms), and record the data they obtain. This means that the sampler
    must be connected to the node (not the node to the sampler), and that the
    node must support the particular type of sampling; see device specific
    documentation for details.

  Recording devices share the start, stop, and origin parameters global to
  devices. Start and stop have the following meaning for stimulating devices
  (origin is just a global offset):
  - Collectors collect all events with timestamps T that fulfill
      start < T <= stop.
    Note that events with timestamp T == start are NOT recorded.
  - Sampling devices sample at times t = nh with
      start < t <= stop
      (t-start) mod interval == 0

  Remarks:
  - Recording devices can only reliably record data generated during the
    previous min_delay interval. This means that in order to ensure consistent
    results, you should always set a stop time for a recording device that is at
    least one min_delay before the end of the simulation time.
  - By default, devices record to memory. If you want to record to file, it may
    be a good idea to turn off recording to memory, to avoid that you computer's
    memory fills up with gigabytes of data: << /record_to [/ascii] >>.
  - Events are not necessarily recorded in chronological order.
  - The device will not open an existing file, since that would erase the
    existing data in the file. If you want existing files to be overwritten
    automatically, you must set /overwrite_files in the root node.

  Parameters:
  The following parameters are shared with all devices:
  /start  - Activation time, relative to origin.
  /stop   - Inactivation time, relative to origin.
  /origin - Reference time for start and stop.

  The following parameter is only relevant for sampling devices:
  /interval - Sampling interval in ms (default: 1ms).


JME: Update for NESTIO!! All of this documentation is kind of outdated


  The following parameters control where output is sent/data collected:
  /record_to - An array containing any combination of /file, /memory, /screen,
               indicating whether to write to file, record in memory or write
               to the console window. An empty array turns all recording of
               individual events off, only an event count is kept. You can also
               pass strings (file), (memory), (screen), mainly for compatibility
               with Python.

               The name of the output file is
                 data_path/data_prefix(label|model_name)-gid-vp.file_extension

               See /label and /file_extension for how to change the name.
               /data_prefix is changed in the root node. If you change any part
               of the name, an open file will be closed and a new file opened.

               To close the file, pass a /record_to array without /file, or pass
               /to_file false. If you later turn recording to file on again, the
               file will be overwritten, unless you have changed data_prefix,
               label, or file_extension.

  /filenames - Array containing the filenames where data is recorded to. This
               array has one entry per local thread and is only available if
               /to_file is set to true, or if /record_to contains /to_file.

  /label     - String specifying an arbitrary label for the device. It is used
               instead of model_name in the output file name.
  /file_extension - String specifying the file name extension, without leading
                    dot. The default depends on the specific device.
  /close_after_simulate - Close output stream before Simulate returns. If set to
                          false, any output streams will remain open when
                          Simulate returns. (Default: false).
  /flush_after_simulate - Flush output stream before Simulate returns. If set to
                          false, any output streams will be in an undefined
                          state when Simulate returns. (Default: true).
  /flush_records - Flush output stream whenever new data has been written to the
                   stream. This may impede performance (Default: false).
  /close_on_reset - Close output file stream upon ResetNetwork. Upon the next
                    call to Simulate, the file is reopened, overwriting its
                    contents. If set to false, the file will remain open after
                    ResetNetwork, so you can record continuously. NB:
                    the file is always closed upon ResetKernel. (Default: true).
  /use_gid_in_filename - Determines if the GID is used in the file name of the
  recording device. Setting this to false can lead to conflicting file names.

//JME: document the flags for different backends (e.g. precision for ascii) and
// add an info message to the sionlib backend explaining that time_in_steps is
// not working there

  The following parameters control how output is formatted:
  /time_in_steps - boolean value which specifies whether to print time in steps,
                   i.e., multiples of the resolution, rather than in ms. If
                   combined with /precise_times, each time is printed as a pair
                   of an integer step number and a double offset < 0.

  Data recorded in memory is available through the following parameter:
  /n_events      - Number of events collected or sampled. n_events can be set to
                   0, but no other value. Setting n_events to 0 will delete all
                   spikes recorded in memory. n_events will count events even
                   when not recording to memory.
  /events        - Dictionary with elements /senders (sender GID), /times (spike
                   times in ms or steps, depending on /time_in_steps) and
                   /offsets (only if /time_in_steps is true). All data stored in memory
                   is erased when /n_events is set to 0.

  SeeAlso: Device, StimulatingDevice
*/

/**
 * Base class for all recording devices.
 *
 * Recording devices collect data and output it to the screen,
 * store it internally or write it to files. This class provides
 * for time windowing of data registration, temporary storage of
 * data and output of data to files.
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

  bool get_record_targets() const;
  void set_record_targets( bool );

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
    bool record_targets_;  //!< Should the targets of events be recorded?
    ArrayDatum record_to_; //!< Array of recording backends to use

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

inline bool
RecordingDevice::get_record_targets() const
{
  return P_.record_targets_;
}

inline void
RecordingDevice::set_record_targets( bool record_targets )
{
  P_.record_targets_ = record_targets;
}

inline void
RecordingDevice::write( const Event& event,
			const std::vector< double >& double_values,
			const std::vector< long >& long_values)
{
  //JME: The number of events needs to be stored on a per-backend basis
  ++S_.n_events_;
  for ( auto& backend_token : P_.record_to_ )
  {
    Name backend_name( getValue< std::string >( backend_token ) );
    kernel().io_manager.write( backend_name, *this, event, double_values, long_values );
  }
}

inline void
RecordingDevice::enroll( const std::vector< Name >& double_value_names,
			 const std::vector< Name >& long_value_names)
{
  //JME: also handle disenroll
  for ( auto& backend_token : P_.record_to_ )
  {
    Name backend_name( getValue< std::string >( backend_token ) );
    kernel().io_manager.enroll_recorder( backend_name, *this, double_value_names, long_value_names );
  }
}

} // namespace

#endif // RECORDING_DEVICE_H
