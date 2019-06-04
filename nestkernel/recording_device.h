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
#include "device.h"
#include "nest_types.h"

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
    memory fills up with gigabytes of data:
      << /to_file true /to_memory false >>.
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

  /to_file   - If true, turn on recording to file. Similar to /record_to
               [/file], but does not affect settings for recording to memory and
               screen.
  /to_screen - If true, turn on recording to screen. Similar to /record_to
               [/screen], but does not affect settings for recording to memory
               and file.
  /to_memory - If true, turn on recording to memory Similar to /record_to
               [/memory], but does not affect settings for recording to file and
               screen.

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

  The following parameters control how output is formatted:
  /withtime      - boolean value which specifies whether the network time should
                   be recorded (default: true).
  /withgid       - boolean value which specifies whether the global id of the
                   observed node(s) should be recorded (default: false).
  /withweight    - boolean value which specifies whether the weight of the event
                   should be recorded (default: false).
  /time_in_steps - boolean value which specifies whether to print time in steps,
                   i.e., multiples of the resolution, rather than in ms. If
                   combined with /precise_times, each time is printed as a pair
                   of an integer step number and a double offset < 0.
  /precise_times - boolean value which specifies whether offsets describing the
                   precise timing of a spike within a time step should be taken
                   into account when computing the spike time. This is only
                   useful when recording from neurons that can emit spikes
                   off-grid (see module precise). Times are given in
                   milliseconds. If /time_in_steps is true, times are given as
                   steps and negative offset.
  /scientific    - if set to true, doubles are written in scientific format,
                   otherwise in fixed format; affects file output only, not
                   screen output (default: false)
  /precision     - number of digits to use in output of doubles to file
                   (default: 3)
  /binary        - if set to true, data is written in binary mode to files
                   instead of ASCII. This setting affects file output only, not
                   screen output (default: false)
  /fbuffer_size  - the size of the buffer to use for writing to files. Setting
                   this value to 0 will reduce buffering to a system-dependent
                   minimum. Set /flush_after_simulate to true to ensure that all
                   pending data is written to file before Simulate returns. A
                   value of -1 shows that the system default is in use. This
                   value can only be changed before Simulate is called.

  Data recorded in memory is available through the following parameter:
  /n_events      - Number of events collected or sampled. n_events can be set to
                   0, but no other value. Setting n_events to 0 will delete all
                   spikes recorded in memory. n_events will count events even
                   when not recording to memory.
  /events        - Dictionary with elements /senders (sender GID, only if
                   /withgid or /withpath are true), /times (spike times in ms or
                   steps, depending on /time_in_steps; only if /withtime is
                   true) and /offsets (only if /time_in_steps, /precise_times
                   and /withtime are true). All data stored in memory is erased
                   when /n_events is set to 0.

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
class RecordingDevice : public Device
{

public:
  /**
   * Device mode.
   */
  enum Mode
  {
    SPIKE_DETECTOR,
    MULTIMETER,
    SPIN_DETECTOR,
    WEIGHT_RECORDER
  };

  /**
   * Create recording device information.
   * @param Node of which the device is member.
   * @param Mode of recording device.
   * @param Default file name extension, excluding ".".
   * @param Default value for withtime property
   * @param Default value for withgid property
   * @param Default value for withweight property
   * @param Default value for withtargetgid property
   * @param Default value for withport property
   * @param Default value for withrport property
   */
  RecordingDevice( const Node&,
    Mode,
    const std::string&,
    bool,
    bool,
    bool = false,
    bool = false,
    bool = false,
    bool = false );

  /**
   * Copy from prototype member.
   * @param Node of which the device is member.
   * @param Prototype member to copy
   */
  RecordingDevice( const Node&, const RecordingDevice& );
  virtual ~RecordingDevice()
  {
  }

  using Device::init_parameters;
  void init_parameters( const RecordingDevice& );

  using Device::init_state;
  void init_state( const RecordingDevice& );

  /**
   * Close file stream.
   * @see Introductory comment for class, calibrate()
   */
  void init_buffers();

  /**
   * Ensure streams are open for writing.
   * @see Introductory comment for class.
   */
  void calibrate();

  /**
   * Flush output stream if requested.
   */
  void post_run_cleanup();

  /**
   * Close output stream if requested.
   */
  void finalize();

  /**
   * Record common information for one event.
   *
   * This function extracts sender and time information from the
   * given event and handles it as specified by the Recorder settings.
   * The following information is extracted:
   * - time stamp
   * - offset
   * - sender ID
   * @param endrecord pass false if more data is to come on same line
   */
  void record_event( const Event&, bool endrecord = true );

  /**
   * Print single item of type ValueT.
   *
   * @param endrecord pass false if more data is to come on same line.
   */
  template < typename ValueT >
  void print_value( const ValueT&, bool endrecord = true );

  /** Indicate if recording device is active.
   *  The argument is the time stamp of the event, and the
   *  device is active if start_ < T <= stop_.
   */
  bool is_active( Time const& T ) const;

  void get_status( DictionaryDatum& ) const;

  /**
   * Set properties of recording device.
   * Setting properties of recording devices is special:
   * - Changing /label will close the current stream and open a new stream
   *   on next call to Simulate (if recording to file)
   * - The state cannot be set, except that passing "/n_events 0" will clear
   *   all stored data from memory (no effect on file streams)
   * - Passing "/close_stream true" will close the output stream if writing
   *   to a file. NB: The next call to Simulate will re-open the file, over-
   *   writing it.
   * - Modifying /data_prefix in the root node will close the current stream
   *   and open a new stream on next call to Simulate (if recording to file).
   */
  void set_status( const DictionaryDatum& );

  /**
   * Special version for recorders that need to have their data cleared.
   * This version of set_status() should be used by recording devices that
   * need to clear their own data upon setting n_events to 0.
   * @param dictionary with parameters to set
   * @param t is pointer to the data that is to be cleared, must support
   * t->clear()
   * @todo This breaks encapsulation. Can be find a better solution, short of a
   *       huge mess with pointers to owners and an extended owner interface?
   */
  template < typename DataT >
  void set_status( const DictionaryDatum&, DataT& t );

  bool
  to_screen() const
  {
    return P_.to_screen_;
  }
  bool
  to_file() const
  {
    return P_.to_file_;
  }
  bool
  to_memory() const
  {
    return P_.to_memory_;
  }
  bool
  to_accumulator() const
  {
    return P_.to_accumulator_;
  }

  inline void set_precise_times( bool precise_times );

  inline void set_precision( long precision );

  inline bool records_precise_times() const;

  inline bool is_precision_user_set() const;

  inline bool is_precise_times_user_set() const;


private:
  /**
   * Print the time-stamp according to the recorder's flags.
   *
   * The following combinations are possible:
   * time_in_steps & precise_times:  give steps and offsets separately
   * time_in_steps                :  give steps, ignore offsets
   *                 precise_times:  give time in ms, take into account offsets
   * none set                     :  give time in ms, ignore offsets
   */
  void print_time_( std::ostream&, const Time&, double offset );

  /**
   * Print a node's global ID and/or address, according to the recorder's flags.
   */
  void print_id_( std::ostream&, index );

  /**
   * Print the weight of an event.
   */
  void print_weight_( std::ostream&, double );

  /**
   * Print the target gid of an event.
   */
  void print_target_( std::ostream&, index );

  /**
   * Print the port of an event.
   */
  void print_port_( std::ostream&, long );

  /**
   * Print the rport of an event.
   */
  void print_rport_( std::ostream&, long );

  /**
   * Store data in internal structure.
   * @param store sender gid of event
   * @param store timestamp of event
   * @param store offset of event
   * @param store weight of event
   * @param store target gid of event
   * @param store port of event
   * @param store rport of event
   */
  void store_data_( index, const Time&, double, double, index, long, long );

  /**
   * Clear data in internal structure, and call clear_data_hook().
   * @see clear_data_hook().
   */
  void clear_data_();

  /**
   * Flush output stream (not std::cout).
   */
  void flush_stream_();

  /**
   * Build filename from parts.
   * @note This function returns the filename, it does not manipulate
   *       any data member.
   */
  const std::string build_filename_() const;

  // ------------------------------------------------------------------

  struct Buffers_
  {
    std::ofstream fs_; //!< the file to write the recorded data to

    /**
     * Manually managed output buffer.
     *
     * This pointer is zero unless the user explicitly sets fbuffer_size_
     * to a value greater than zero.
     */
    char* fbuffer_;
    long fbuffer_size_; //!< size of fbuffer_; -1: not yet set

    Buffers_();
    ~Buffers_();
  };

  // ------------------------------------------------------------------

  struct Parameters_
  {
    bool to_file_;   //!< true if recorder writes its output to a file
    bool to_screen_; //!< true if recorder writes its output to stdout
    bool to_memory_; //!< true if data should be recorded in memory, default
    bool to_accumulator_; //!< true if data is to be accumulated; exclusive to
                          //!< all other to_*
    bool time_in_steps_;  //!< true if time is printed in steps, not ms.
    bool precise_times_;  //!< true if time is computed including offset
    bool withgid_;        //!< true if element GID is to be printed, default
    bool withtime_;       //!< true if time of event is to be printed, default
    bool withweight_;     //!< true if weight of event is to be printed
    bool withtargetgid_;  //!< true if target GID is to be printed, default
    bool withport_;       //!< true if port is to be printed, default
    bool withrport_;      //!< true if rport is to be printed, default

    long precision_;  //!< precision of doubles written to file
    bool scientific_; //!< use scientific format if true, else fixed

    bool user_set_precise_times_; //!< true if user set precise_times
    bool user_set_precision_;     //!< true if user set precision

    bool binary_; //!< true if to write files in binary mode instead of ASCII
    long fbuffer_size_; //!< output buffer size; -1 until set by user

    std::string label_;    //!< a user-defined label for symbolic device names.
    std::string file_ext_; //!< the file name extension to use, without .
    std::string filename_; //!< the filename, if recording to a file (read-only)
    bool close_after_simulate_; //!< if true, finalize() shall close the stream
    bool flush_after_simulate_; //!< if true, post_run_cleanup() flushes stream
    bool flush_records_;        //!< if true, flush stream after each output
    bool close_on_reset_;       //!< if true, close stream in init_buffers()

    bool use_gid_in_filename_;

    /**
     * Set default parameter values.
     * @param Default file name extension, excluding ".".
     * @param Default value for withtime property
     * @param Default value for withgid property
     * @param Default value for withweight property
     * @param Default value for withtargetgid property
     * @param Default value for withport property
     * @param Default value for withrport property
     */
    Parameters_( const std::string&, bool, bool, bool, bool, bool, bool );

    //! Store current values in dictionary
    void get( const RecordingDevice&, DictionaryDatum& ) const;

    /**
     * Set values from dictionary.
     *
     * @note `Buffers_&` cannot be `const` because `basic_ofstream::is_open()`
     * is not `const` in C++98  (cf C++ Standard §27.8.1.10).
     */
    void set( const RecordingDevice&, Buffers_&, const DictionaryDatum& );
  };

  // ------------------------------------------------------------------

  struct State_
  {
    size_t events_;                         //!< Event counter
    std::vector< long > event_senders_;     //!< List of event sender ids
    std::vector< long > event_targets_;     //!< List of event targets ids
    std::vector< long > event_ports_;       //!< List of event ports
    std::vector< long > event_rports_;      //!< List of event rports
    std::vector< double > event_times_ms_;  //!< List of event times in ms
    std::vector< long > event_times_steps_; //!< List of event times in steps
    //! List of event time offsets
    std::vector< double > event_times_offsets_;
    std::vector< double > event_weights_; //!< List of event weights

    State_(); //!< Sets default parameter values

    void clear_events(); //!< clear all data
    //! Store current values in dictionary
    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum& ); //!< Get values from dictionary
  };

  // ------------------------------------------------------------------

  const Node& node_; //!< node to which device instance belongs
  const Mode mode_;  //!< operating mode, depends on owning node
  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
};

inline bool
RecordingDevice::is_precision_user_set() const
{
  return P_.user_set_precision_;
}

inline bool
RecordingDevice::is_precise_times_user_set() const
{
  return P_.user_set_precise_times_;
}

inline bool
RecordingDevice::records_precise_times() const
{
  return P_.precise_times_;
}

inline bool
RecordingDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return get_t_min_() < stamp and stamp <= get_t_max_();
}

inline void
RecordingDevice::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
  S_.get( d, P_ );
  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::recorder );
}

inline void
RecordingDevice::set_precise_times( bool use_precise )
{
  P_.precise_times_ = use_precise;
}

inline void
RecordingDevice::set_precision( long precision )
{
  P_.precision_ = precision;
}


template < typename ValueT >
void
RecordingDevice::print_value( const ValueT& value, bool endrecord )
{
  if ( P_.to_screen_ )
  {
    std::cout << value << '\t';
    if ( endrecord )
    {
      std::cout << '\n';
    }
  }

  if ( P_.to_file_ )
  {
    B_.fs_ << value << '\t';
    if ( endrecord )
    {
      B_.fs_ << '\n';
    }
  }
}

template < typename DataT >
void
RecordingDevice::set_status( const DictionaryDatum& d, DataT& data )
{
  // plain set_status does most of the work
  set_status( d );

  // if n_events is 0, also clear event data
  if ( S_.events_ == 0 )
  {
    data.clear();
  }
}

} // namespace

#endif // RECORDING_DEVICE_H
