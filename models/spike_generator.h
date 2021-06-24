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
#include "stimulation_device.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Generate spikes from an array with spike-times

Description
+++++++++++

A spike generator can be used to generate spikes at specific times
which are given to the spike generator as an array.

Spike times are given in milliseconds, and must be sorted with the
earliest spike first. All spike times must be strictly in the future.
Trying to set a spike time in the past or at the current time step
will cause a NEST error. Setting a spike time of 0.0 will also result
in an error.

Spike times may not coincide with a time step, i.e., are not a multiple
of the simulation resolution. Three options control how spike times that
do not coincide with a step are handled (see examples below):

Multiple occurrences of the same time indicate that more than one
event is to be generated at this particular time.

Additionally, `spike_weights` can be set. This is an array as well.
It contains one weight value per spike time. If set, the spikes
are delivered with the respective weight multiplied with the
weight of the connection. To disable this functionality, the
spike_weights array can be set to an empty array.

    `precise_times`  default: false

If false, spike times will be rounded to simulation steps, i.e., multiples
of the resolution. The rounding is controlled by the two other flags.
If true, spike times will not be rounded but represented exactly as a
combination of step and offset. This should only be used if all neurons
receiving the spike train can handle precise timing information. In this
case, the other two options are ignored.

    `allow_offgrid_times`   default: false

If false, spike times will be rounded to the nearest step if they are
less than tic/2 from the step, otherwise NEST reports an error.
If true, spike times are rounded to the nearest step if within tic/2
from the step, otherwise they are rounded up to the *end* of the step.

    `shift_now_spikes`   default: false

This option is mainly for use by the PyNN-NEST interface.
If false, spike times rounded down to the current point in time will
be considered in the past and ignored.
If true, spike times that are rounded down to the current time step
are shifted one time step into the future.

Note that ``GetStatus`` will report the spike times that the spike_generator
will actually use, i.e., for grid-based simulation the spike times rounded
to the appropriate point on the time grid. This means that ``GetStatus`` may
return different `spike_times` values at different resolutions.

Example:

  ::

     nest.Create("spike_generator",
                 params={"spike_times": [1.0, 2.0, 3.0]})

  Instructs the spike generator to generate events at 1.0, 2.0, and
  3.0 milliseconds, relative to the device-timer origin.

Example:

Assume that NEST works with default resolution (step size) of 0.1 ms
and default tic length of 0.001 ms. Then, spikes times not falling
onto the grid will be handled as follows for different option settings:

  ::

    nest.Create("spike_generator",
               params={"spike_times": [1.0, 1.9999, 3.0001]})

  ---> spikes at steps 10 (==1.0 ms), 20 (==2.0 ms) and 30 (==3.0 ms)

  ::

    nest.Create("spike_generator",
               params={"spike_times": [1.0, 1.05, 3.0001]})

  ---> **Error!** Spike time 1.05 not within tic/2 of step


  ::

    nest.Create("spike_generator",
               params={"spike_times": [1.0, 1.05, 3.0001],
               "allow_offgrid_times": True})

  ---> spikes at steps 10, 11 (mid-step time rounded up),
         30 (time within tic/2 of step moved to step)

  ::

    nest.Create("spike_generator",
               params={"spike_times": [1.0, 1.05, 3.0001],
               "precise_times": True})

  ---> spikes at step 10, offset 0.0; step 11, offset -0.05;
         step 31, offset -0.0999

Assume we have simulated 10.0 ms and simulation time is thus 10.0 (step
100). Then, any spike times set at this time must be later than step 100.

  ::

    nest.Create("spike_generator",
               params={"spike_times": [10.0001]})

  ---> spike time is within tic/2 of step 100, rounded down to 100 thus
         not in the future; **spike will not be emitted**

  ::

    nest.Create("spike_generator",
               params={"spike_times": [10.0001],
               "precise_times": True})

  ---> spike at step 101, offset -0.0999 is in the future

  ::

    nest.Create("spike_generator",
               params={"spike_times": [10.0001, 11.0001],
               "shift_now_spikes": True})

  ---> spike at step 101, spike shifted into the future, and spike at step
        110, not shifted, since it is in the future anyways

.. include:: ../models/stimulation_device.rst

spike_times
    List of spike times in ms

spike_weights
    Corresponding spike-weights, the unit depends on the receiver

spike_multiplicities
    Multiplicities of spikes, same length as spike_times; mostly for debugging

precise_times
    See above

allow_offgrid_times
    See above

shift_now_spikes
    See above

Set spike times from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The spike times for this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds just an array of spike times in ms.

Sends
+++++

SpikeEvent

See also
++++++++

poisson_generator

EndUserDocs
*/
class spike_generator : public StimulationDevice
{

public:
  spike_generator();
  spike_generator( const spike_generator& );

  port send_test_event( Node&, rport, synindex, bool ) override;
  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_spikes ) override;


  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::event_hook;
  using Node::sends_signal;

  void event_hook( DSSpikeEvent& ) override;

  SignalType
  sends_signal() const override
  {
    return ALL;
  }

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;

  // ------------------------------------------------------------

  struct State_
  {
    State_();
    size_t position_; //!< index of next spike to deliver
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

    Parameters_();                               //!< Sets default parameter values
    Parameters_( const Parameters_& ) = default; //!< Recalibrate all times

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

  Parameters_ P_;
  State_ S_;
};

inline port
spike_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  enforce_single_syn_type( syn_id );

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
  StimulationDevice::get_status( d );
}

inline void
nest::spike_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // To detect "now" spikes and shift them, we need the origin. In case
  // it is set in this call, we need to extract it explicitly here.
  Time origin;
  double v;
  if ( updateValue< double >( d, names::origin, v ) )
  {
    origin = Time::ms( v );
  }
  else
  {
    origin = StimulationDevice::get_origin();
  }

  // throws if BadProperty
  ptmp.set( d, S_, origin, kernel().simulation_manager.get_time(), this );

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}

inline StimulationDevice::Type
spike_generator::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace nest

#endif /* #ifndef SPIKE_GENERATOR_H */
