/*
 *  spike_train_injector.h
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

#ifndef SPIKE_TRAIN_INJECTOR_H
#define SPIKE_TRAIN_INJECTOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device.h"
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

namespace nest
{

/* BeginUserDocs: neuron, device, spike, generator

Short description
+++++++++++++++++

Neuron that emits prescribed spike trains.

Description
+++++++++++

The spike train injector neuron emits spikes at prescribed spike times
which are given as an array. The neuron does not allow incoming connections
and is thus not able to process incoming spikes or currents.

.. note::

   ``spike_train_injector`` is recommended if the spike trains have a similar
   rate to regular neurons. For very high rates, use
   :doc:`spike generator </models/spike_generator>`.

Spike times are given in milliseconds as an array. The `spike_times` array
must be sorted with the earliest spike first. All spike times must be strictly
in the future. Trying to set a spike time in the past or at the current time
step will cause a NEST error. Setting a spike time of 0.0 will also result
in an error.

Multiple occurrences of the same time indicate that more than one event is to
be generated at this particular time.

The spike generator supports spike times that do not coincide with a time
step, that is, are not falling on the grid defined by the simulation resolution.
Spike times that do not coincide with a step are handled with one of three
options (see examples below):

Option 1:   ``precise_times``   default: false

If false, spike times will be rounded to simulation steps, i.e., multiples
of the resolution. The rounding is controlled by the two other flags. If true,
spike times will not be rounded but represented exactly as a combination of
step and offset. This should only be used if all neurons receiving the spike
train can handle precise timing information. In this case, the other two
options are ignored.

Option 2:   ``allow_offgrid_times``   default: false

If false, spike times will be rounded to the nearest step if they are less
than tic/2 from the step, otherwise NEST reports an error. If true, spike
times are rounded to the nearest step if within tic/2 from the step, otherwise
they are rounded up to the *end* of the step. This setting has no effect if
``precise_times`` is `true`.

Option 3:   ``shift_now_spikes``   default: false

This option is mainly for use by the PyNN-NEST interface. If false, spike
times rounded down to the current point in time will be considered in the past
and ignored. If true, spike times that are rounded down to the current time
step are shifted one time step into the future.

Note that ``GetStatus`` will report the spike times that the spike_generator
will actually use, i.e., for grid-based simulation the spike times rounded
to the appropriate point on the time grid. This means that ``GetStatus`` may
return different `spike_times` values at different resolutions.

Example:

::

     nest.Create("spike_train_injector",
                 params={"spike_times": [1.0, 2.0, 3.0]})

Instructs the spike train injector neuron to emit events at 1.0, 2.0,
and 3.0 milliseconds, relative to the timer origin.

Example:

Assume that NEST works with default resolution (step size) of 0.1 ms
and default tic length of 0.001 ms. Then, spikes times not falling
onto the grid will be handled as follows for different option settings:

::

    nest.Create("spike_train_injector",
               params={"spike_times": [1.0, 1.9999, 3.0001]})

---> spikes at steps 10 (==1.0 ms), 20 (==2.0 ms) and 30 (==3.0 ms)

::

    nest.Create("spike_train_injector",
               params={"spike_times": [1.0, 1.05, 3.0001]})

---> **Error!** Spike time 1.05 not within tic/2 of step


::

    nest.Create("spike_train_injector",
               params={"spike_times": [1.0, 1.05, 3.0001],
               "allow_offgrid_times": True})

---> spikes at steps 10, 11 (mid-step time rounded up),
      30 (time within tic/2 of step moved to step)

::

    nest.Create("spike_train_injector",
               params={"spike_times": [1.0, 1.05, 3.0001],
               "precise_times": True})

---> spikes at step 10, offset 0.0; step 11, offset -0.05;
      step 31, offset -0.0999

Assume we have simulated 10.0 ms and simulation time is thus 10.0 (step
100). Then, any spike times set at this time must be later than step 100.

::

    nest.Create("spike_train_injector",
               params={"spike_times": [10.0001]})

---> spike time is within tic/2 of step 100, rounded down to 100 thus
      not in the future; **spike will not be emitted**

::

    nest.Create("spike_train_injector",
               params={"spike_times": [10.0001],
               "precise_times": True})

---> spike at step 101, offset -0.0999 is in the future

::

    nest.Create("spike_train_injector",
               params={"spike_times": [10.0001, 11.0001],
               "shift_now_spikes": True})

---> spike at step 101, spike shifted into the future, and spike at step
      110, not shifted, since it is in the future anyways


Parameters
++++++++++

origin
    A positive floating point number (default : `0.0`) used as the
    reference time in ms for `start` and `stop`.

start
    A positive floating point number (default: `0.0`) specifying the
    activation time in ms, relative to `origin`.

stop
    A floating point number (default: `infinity`) specifying the
    deactivation time in ms, relative to `origin`. The value of `stop`
    must be greater than or equal to `start`.

spike_times
    List of spike times in ms.

spike_multiplicities
    List of multiplicities of spikes, same length as spike_times; mostly
    for debugging.

precise_times
    See above.

allow_offgrid_times
    See above.

shift_now_spikes
    See above.

Receives
++++++++

None

Sends
+++++

SpikeEvent

See also
++++++++

spike_generator

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: spike_train_injector

EndUserDocs */

/**
 * @brief Spike train injector node.
 *
 * See UserDocs for details.
 *
 * @note Spikes emitted by a spike train injector neuron will be counted by
 * the local spike count.
 */
void register_spike_train_injector( const std::string& name );

class spike_train_injector : public Node, public Device
{

public:
  spike_train_injector();
  spike_train_injector( const spike_train_injector& );

  size_t send_test_event( Node&, size_t, synindex, bool ) override;
  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;
  bool is_active( const Time& ) const override;

  bool
  is_off_grid() const override
  {
    return P_.precise_times_;
  }

  void set_data( std::vector< double >& input_spikes );

private:
  void init_state_() override;
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  /**
   * State variables of the model.
   */
  struct State_
  {
    State_();
    size_t position_; //!< index of next spike to deliver
  };

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    //! Spike time stamp as Time, rel to origin_
    std::vector< Time > spike_stamps_;

    //! Spike time offset, if using precise_times_
    std::vector< double > spike_offsets_;

    //! Spike multiplicity
    std::vector< long > spike_multiplicities_;

    //! Interpret spike times as precise, i.e. send as step and offset
    bool precise_times_;

    //! Allow and round up spikes not on steps; irrelevant if precise_times_
    bool allow_offgrid_times_;

    //! Shift spike times at present to next step
    bool shift_now_spikes_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& ) = default;
    Parameters_& operator=( const Parameters_& ) = default;

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dictionary.
     * @note State is passed so that the position can be reset if the
     *       spike_times_ vector has been filled with new data, or if
     *       the origin was reset.
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

  State_ S_;
  Parameters_ P_;
};


inline size_t
spike_train_injector::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}


inline void
spike_train_injector::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  Device::get_status( d );
}


inline void
spike_train_injector::set_status( const DictionaryDatum& d )
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
    origin = Device::get_origin();
  }

  // throws if BadProperty
  ptmp.set( d, S_, origin, kernel::manager< SimulationManager >.get_time(), this );

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  Device::set_status( d );

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}

} // namespace
#endif // SPIKE_TRAIN_INJECTOR_H
