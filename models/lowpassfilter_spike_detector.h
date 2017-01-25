/*
 *  lowpassfilter_spike_detector.h
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

#ifndef LOWPASSFILTER_SPIKE_DETECTOR_H
#define LOWPASSFILTER_SPIKE_DETECTOR_H


// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"
#include "node.h"
#include "recording_device.h"

/* BeginDocumentation

  Name: lowpassfilter_spike_detector - Device for detecting single
  spikes and efficiently generating a continuous trace of the activity
  for each node connected to it.

  Description:
  There are 2 parts to this device, it acts as a normal spike_detector, but it
  also generates a continuous trace for
  each node connected to it based on spiking activity. It can be used to record
  spikes from a single neuron or
  spike-generating device, or from many of them at once. Spikes and traces can
  be recorded to memory or file.
  spike events can be accessed from "events". By default, GID and time of each
  spike is recorded.
  filtered spike values can be accessed from "filter_events". GID and time are
  reported as well.

  The decayed value from time "t1" to "t2" is calculated as follows:
  new_trace_value = exp((t1 - t2) / tau_filter) * old_trace_value
  And in response to a spike, the trace is updated as follows:
  new_trace_value = old_trace_value + 1 / tau_filter

  The lowpassfilter_spike_detector can also record spike times and calculate
  traces with full precision
  from neurons emitting precisely timed spikes. Set /precise_times to achieve
  this. The filtering of
  precise models will come at a later version.

  Any node from which spikes are to be recorded (and filtered traces
  calculated), must be connected to
  this device using a normal connect command. Any connection weight and delay
  will be ignored for that connection.
  Recording of spikes can be disabled by setting the \record_spikes parameter to
  false, the trace calculation and
  recording will be unaffected by this.

  Remarks:
  - Filtered traces and spikes are recorded (stored in memory or written to
  screen/file) at multiples of minimum
  delay in the network (+ one resolution increment). For example, if the minimum
  delay in the network is 1.5 ms,
  but simulation resolution and filter_report_interval are 0.1, The results for
  the first 1.5 ms will be recorded
  once the simulation has exceeded 1.5 ms (i.e., nest.Simulate(x) where x is >=
  1.6).
  - Filtered traces and/or spikes are not necessarily written to file in
  chronological order.
  - To get the most performance when only needing the trace at certain times,
  set start_times and stop_times
  with only 1 filter_report_interval intervals. For example, if the trace is
  needed at 50ms and 100ms, specify
  filter_start_times = [49.0, 99.0] , filter_stop_times = [50.0, 100.0] , and
  filter_report_interval = 1.0.
  - When writing to file, spikes and filtered traces are stored in separate
  files. The files that include
  the traces include "filtered" in their name and by default are stored with
  "dat" extension.
  - Setting \n_events to 0 clears both the "events" and "filter_events" on
  memory.

  Parameters:
  /filter_start_times - The start time(s) for reporting the continous trace
  /filter_stop_times - The stop time(s) for reporting the continous trace
  /filter_report_interval - The interval to report the trace in each start, stop
  block.
  /tau_filter - Tau for calculating the trace value.
  /record_spikes - If true, it would also record spikes just as a spike_detector
  would.

  Receives: SpikeEvent

  SeeAlso: Spike_detector, Device, RecordingDevice
*/


namespace nest
{

/**
 * lowpassfilter_spike_detector.
 *
 * Just like a normal spike_detector, this class manages spike recording for
 * normal and precise spikes. In addition to that, it calculates and records
 * low-pass filtered traces for each node connected to it. It receives spikes
 * via its handle(SpikeEvent&) method, buffers them, and records them via its
 * RecordingDevices in the update() method. Recording of spikes can be disabled
 * by setting the "record_spikes" parameter to false, the trace calculation and
 * recording will be unaffected by this.
 *
 * Spikes are buffered in a two-segment buffer. We need to distinguish between
 * two types of spikes: those delivered from the global event queue (almost all
 * spikes) and spikes delivered locally from devices that are replicated on VPs
 * (has_proxies() == false).
 * - Spikes from the global queue are delivered by deliver_events() at the
 *   beginning of each update cycle and are stored only until update() is called
 *   during the same update cycle. Global queue spikes are thus written to the
 *   read_toggle() segment of the buffer, from which update() reads.
 * - Spikes delivered locally may be delivered before or after
 *   spike_detector::update() is executed. These spikes are therefore buffered
 *in
 *   the write_toggle() segment of the buffer and output during the next cycle.
 * - After all spikes are recorded, update() clears the read_toggle() segment
 *   of the buffer.
 *
 * @ingroup Devices
 *
 * @author Sepehr Mahmoudian 2016-08-01
 */

class lowpassfilter_spike_detector : public Node
{

public:
  lowpassfilter_spike_detector();
  lowpassfilter_spike_detector( const lowpassfilter_spike_detector& );

  void set_has_proxies( const bool hp );
  bool
  has_proxies() const
  {
    return has_proxies_;
  }
  bool
  potential_global_receiver() const
  {
    return true;
  }
  void set_local_receiver( const bool lr );
  bool
  local_receiver() const
  {
    return local_receiver_;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void finalize();
  void print_value( long sender, double time, double value );
  long filter_step_( long update_start ); // Returns the first step to
                                          // report based on the
                                          // current filter block.
  double add_impulse_(
    long node ); // Adds to and returns the current trace value for the node.
  double calculate_decay_( long node,
    double
      to_time ); // calculates and returns the decayed trace value for the node.

  /**
   * Perform filtering and update device by recording spikes.
   *
   * All spikes in the read_toggle() half of the spike buffer are
   * processed and recorded by passing them to the RecordingDevice, which then
   * stores them in memory or outputs them as desired.
   *
   * @see RecordingDevice
   */
  void update( Time const&, const long, const long );

  /**
   * Buffer for incoming spikes.
   *
   * This data structure buffers all incoming spikes until they are
   * passed to the RecordingDevice for storage or output during update().
   * update() always reads from spikes_[network()->read_toggle()] and
   * deletes all events that have been read.
   *
   * Events arriving from locally sending nodes, i.e., devices without
   * proxies, are stored in spikes_[network()->write_toggle()], to ensure
   * order-independent results.
   *
   * Events arriving from globally sending nodes are delivered from the
   * global event queue by Scheduler::deliver_events() at the beginning
   * of the time slice. They are therefore written to
   * spikes_[network()->read_toggle()]
   * so that they can be recorded by the subsequent call to update().
   * This does not violate order-independence, since all spikes are delivered
   * from the global queue before any node is updated.
   */

  struct Parameters_
  {
    // If set to true, will act as a spike_detector. This does not affect the
    // behavior of the low-pass filter.
    // Default value is false.
    bool record_spikes_;
    double tau_;

    std::vector< double > filter_start_times_;
    std::vector< double > filter_stop_times_;
    Time filter_report_interval_;


    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };

  struct State_
  {
    // stores the gids of the nodes connected to this device.
    std::vector< long > node_gids_;
    // stores traces for each node if to_memory == true.
    std::vector< std::vector< double > > node_traces_;
    // stores the times for trace calculations for each node.
    std::vector< double > filter_times_;

    State_();
  };

  struct Variables_
  {

    size_t filter_block_index_; // stores the current index of filter block

    Variables_();
  };

  struct Buffers_
  {
    // Stores the trace (only the last value) for each node.
    std::vector< double > traces_;
    // Stores the last simulation time trace was calculated for each node.
    std::vector< double > trace_times_;
    // The steps to report in each min_delay interval.
    std::vector< long > steps_to_filter_;
    // stores the spike events for each node.
    std::vector< std::vector< std::vector< Event* > > > node_spikes_;
  };

  bool has_proxies_;
  bool local_receiver_;

  RecordingDevice spikes_device_;   // For recording spiking activity
  RecordingDevice filtered_device_; // For recording filtered information
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline void
lowpassfilter_spike_detector::set_has_proxies( const bool hp )
{
  has_proxies_ = hp;
}

inline void
lowpassfilter_spike_detector::set_local_receiver( const bool lr )
{
  local_receiver_ = lr;
}

inline void
lowpassfilter_spike_detector::finalize()
{
  spikes_device_.finalize();
  filtered_device_.finalize();
}

// For keeping track of the nodes connected to this device.

inline port
lowpassfilter_spike_detector::handles_test_event( SpikeEvent& e,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  S_.node_gids_.push_back( e.get_sender().get_gid() );
  return S_.node_gids_.size() - 1; // -1 because we want rports to start from 0
}

} // namespace

#endif /* #ifndef LOWPASSFILTER_SPIKE_DETECTOR_H */
