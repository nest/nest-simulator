/*
 *  spike_generator.h
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

#ifndef SPIKE_GENERATOR_H
#define SPIKE_GENERATOR_H


// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
#include "stimulating_device.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Devices
@ingroup generator

Name: spike_generator - A device which generates spikes from an array with
                        spike-times.

Synopsis: spike_generator Create -> gid

Description:

A spike generator can be used to generate spikes at specific times
which are given to the spike generator as an array.

Spike times are given in milliseconds, and must be sorted with the
earliest spike first. All spike times must be strictly in the future.
Trying to set a spike time in the past or at the current time step,
will cause a NEST error. Setting a spike time of 0.0 will also result
in an error.

Spike times may not coincide with a time step, i.e., are not a multiple
of the simulation resolution. Three options control how spike times that
do not coincide with a step are handled (see examples below):

Multiple occurrences of the same time indicate that more than one
event is to be generated at this particular time.

Additionally, spike_weights can be set. This also is an array,
which contains one weight value per spike time. If set, the spikes
are delivered with the respective weight multiplied with the
weight of the connection. To disable this functionality, the
spike_weights array can be set to an empty array.

    /precise_times   default: false

If false, spike times will be rounded to simulation steps, i.e., multiples
of the resolution. The rounding is controlled by the two other flags.
If true, spike times will not be rounded but represented exactly as a
combination of step and offset. This should only be used if all neurons
receiving the spike train can handle precise timing information. In this
case, the other two options are ignored.

    /allow_offgrid_times   default: false

If false, spike times will be rounded to the nearest step if they are
less than tic/2 from the step, otherwise NEST reports an error.
If true, spike times are rounded to the nearest step if within tic/2
from the step, otherwise they are rounded up to the *end* of the step.

    /shift_now_spikes   default: false

This option is mainly for use by the PyNN-NEST interface.
If false, spike times rounded down to the current point in time will
be considered in the past and ignored.
If true, spike times that are rounded down to the current time step
are shifted one time step into the future.

Note that GetStatus will report the spike times that the spike_generator
will actually use, i.e., for grid-based simulation the spike times rounded
to the appropriate point on the time grid. This means that GetStatus may
return different /spike_times values at different resolutions.

Example:

    spike_generator << /spike_times [1.0 2.0 3.0] >> SetStatus

    Instructs the spike generator to generate events at 1.0, 2.0, and
    3.0 milliseconds, relative to the device-timer origin.

Example:

Assume that NEST works with default resolution (step size) of 0.1ms
and default tic length of 0.001ms. Then, spikes times not falling
onto the grid will be handled as follows for different option settings:

    /spike_generator << /spike_times [1.0 1.9999 3.0001] >> Create
    ---> spikes at steps 10 (==1.0ms), 20 (==2.0ms) and 30 (==3.0ms)

    /spike_generator << /spike_times [1.0 1.05 3.0001] >> Create
    ---> error, spike time 1.05 not within tic/2 of step

    /spike_generator << /spike_times [1.0 1.05 3.0001]
                        /allow_offgrid_times true >> Create
    ---> spikes at steps 10, 11 (mid-step time rounded up),
         30 (time within tic/2 of step moved to step)

    /spike_generator << /spike_times [1.0 1.05 3.0001]
                        /precise_times true >> Create
    ---> spikes at step 10, offset 0.0; step 11, offset -0.05;
         step 31, offset -0.0999

    Assume we have simulated 10.0ms and simulation times is thus 10.0 (step
100).
    Then, any spike times set, at this time, must be later than step 100.

    /spike_generator << /spike_times [10.0001] >> Create
    ---> spike time is within tic/2 of step 100, rounded down to 100 thus
         not in the future, spike will not be emitted

    /spike_generator << /spike_times [10.0001] /precise_times true >> Create
    ---> spike at step 101, offset -0.0999 is in the future

    /spike_generator
      << /spike_times [10.0001 11.0001] /shift_now_spikes true >>
    Create
    ---> spike at step 101, spike shifted into the future, and spike at step
110,
         not shifted, since it is in the future anyways


Example:

    spike_generator
      << /spike_times [1.0 2.0] /spike_weights [5.0 -8.0] >>
    SetStatus

    Instructs the spike generator to generate an event with weight 5.0
    at 1.0 ms, and an event with weight -8.0 at 2.0 ms, relative to
    the device-timer origin.

    spike_generator << /spike_weights [] >> SetStatus

    Instructs the spike generator to generate events at 1.0, 2.0, and
    3.0 milliseconds, and use the weight of the connection.

Parameters:
The following properties can be set in the status dictionary.

\verbatim embed:rst
===================== ============= ==========================================
 origin               ms            Time origin for device timer
 start                ms            Earliest possible time stamp of a spike to
                                    be emitted
 stop                 ms            Earliest time stamp of a potential spike
                                    event that is not emitted
 spike_times          ms            Spike-times
 spike_weights        synaptic      Corresponding spike-weights, the unit
                      weights       depends on the receiver
 spike_multiplicities integer       Multiplicities of spikes, same length
                                    as spike_times; mostly for debugging
 precise_times        boolean       see above
 allow_offgrid_times  boolean       see above
 shift_now_spikes     boolean       see above
===================== ============= ==========================================
\endverbatim

Sends: SpikeEvent

Author: Gewaltig, Diesmann, Eppler

SeeAlso: Device, StimulatingDevice, testsuite::test_spike_generator
*/
class spike_generator : public DeviceNode
{

public:
  spike_generator();
  spike_generator( const spike_generator& );

  bool
  has_proxies() const
  {
    return false;
  }

  Name
  get_element_type() const
  {
    return names::stimulator;
  }

  port send_test_event( Node&, rport, synindex, bool );
  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::event_hook;
  using Node::sends_signal;

  void event_hook( DSSpikeEvent& );

  SignalType
  sends_signal() const
  {
    return ALL;
  }

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // ------------------------------------------------------------

  struct State_
  {
    size_t position_; //!< index of next spike to deliver

    State_(); //!< Sets default state value
  };

  // ------------------------------------------------------------

  struct Parameters_
  {
    //! Spike time stamp as Time, rel to origin_
    std::vector< Time > spike_stamps_;

    //! Spike time offset, if using precise_times_
    std::vector< double > spike_offsets_;

    std::vector< double > spike_weights_; //!< Spike weights as double

    std::vector< long > spike_multiplicities_; //!< Spike multiplicity

    //! Interpret spike times as precise, i.e. send as step and offset
    bool precise_times_;

    //! Allow and round up spikes not on steps; irrelevant if precise_times_
    bool allow_offgrid_times_;

    //! Shift spike times at present to next step
    bool shift_now_spikes_;

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dictionary.
     * @note State is passed so that the position can be reset if the
     *       spike_times_ or spike_weights_ vector has been filled with
     *       new data, or if the origin was reset.
     */
    void set( const DictionaryDatum&, State_&, const Time&, const Time&, Node* node );

    /**
     * Insert spike time to arrays, throw BadProperty for invalid spike times.
     *
     * @param spike time, ms
     * @param origin
     * @param current simulation time
     */
    void assert_valid_spike_time_and_insert_( double, const Time&, const Time& );
  };

  // ------------------------------------------------------------

  StimulatingDevice< SpikeEvent > device_;

  Parameters_ P_;
  State_ S_;
};

inline port
spike_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  device_.enforce_single_syn_type( syn_id );

  if ( dummy_target )
  {
    DSSpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
  else
  {
    SpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
}

inline void
spike_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  device_.get_status( d );
}

} // namespace

#endif /* #ifndef SPIKE_GENERATOR_H */
