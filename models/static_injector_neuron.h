/*
 *  static_injector_neuron.h
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

#ifndef STATIC_INJECTOR_NEURON_H
#define STATIC_INJECTOR_NEURON_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

namespace nest
{

/* BeginUserDocs: neuron, static injector

Short description
+++++++++++++++++

Neuron that emits prescribed spikes

Description
+++++++++++

The static injector neuron simply emits spikes at prescribed spike times.
Incoming spikes will be ignored. The static injector neuron behaves similarly
to a spike generator, but is treated internally as a neuron and not a device.
Unlike a spike generator which is replicated at each virtual process, the
static injector neuron resides on a single virtual process. Spikes emitted
by the static injector neuron will be counted by the local spike count.

Receives
++++++++

None

Sends
+++++

SpikeEvent

EndUserDocs */

class static_injector_neuron : public Node
{

public:
  static_injector_neuron();
  static_injector_neuron( const static_injector_neuron& );

  port send_test_event( Node&, rport, synindex, bool ) override;
  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding,
   * Overloading, and Hiding
   */
  using Node::event_hook;
  using Node::receives_signal;
  using Node::sends_signal;

  void event_hook( DSSpikeEvent& ) override;

  SignalType
  sends_signal() const override
  {
    return ALL; // TODO: should this be ALL or SPIKE?
  }

  SignalType
  receives_signal() const override
  {
    return NONE;
  }

  void set_data( std::vector< double >& input_spikes );

private:
  void init_state_() override;
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  Time const& get_origin() const;
  Time const& get_start() const;
  Time const& get_stop() const;
  long get_t_min_() const;
  long get_t_max_() const;

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

    std::vector< double > spike_weights_; //!< Spike weights as double

    std::vector< long > spike_multiplicities_; //!< Spike multiplicity

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

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    //! Origin of device time axis, relative to network time. Defaults to 0.
    Time origin_;

    //!< Start time, relative to origin. Defaults to 0.
    Time start_;

    //!< Stop time, relative to origin. Defaults to "infinity".
    Time stop_;

    /**
     * Time step of device activation.
     * t_min_ = origin_ + start_, in steps.
     * @note This is an auxiliary variable that is initialized to -1 in the
     * constructor and set to its proper value by calibrate. It should NOT
     * be returned by get_parameters().
     */
    long t_min_;

    /**
     * Time step of device deactivation.
     * t_max_ = origin_ + stop_, in steps.
     * @note This is an auxiliary variable that is initialized to -1 in the
     * constructor and set to its proper value by calibrate. It should NOT
     * be returned by get_parameters().
     */
    long t_max_;
  };

  State_ S_;
  Parameters_ P_;
  Variables_ V_;

  /**
   * Synapse type of the first outgoing connection made by the Device.
   *
   * Used to check that devices connect using only a single synapse type,
   * see #481 and #737. Since this value must survive resets, it is
   * stored here, even though it is an implementation detail.
   */
  synindex first_syn_id_;

  void enforce_single_syn_type( synindex syn_id );
};


inline port
static_injector_neuron::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
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
static_injector_neuron::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  // Node::get_status( d );
}


inline void
static_injector_neuron::set_status( const DictionaryDatum& d )
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
    origin = get_origin();
  }

  // throws if BadProperty
  ptmp.set( d, S_, origin, kernel().simulation_manager.get_time(), this );

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  // Node::set_status( d );

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}


inline Time const&
static_injector_neuron::get_origin() const
{
  return V_.origin_;
}

inline Time const&
static_injector_neuron::get_start() const
{
  return V_.start_;
}

inline Time const&
static_injector_neuron::get_stop() const
{
  return V_.stop_;
}

inline long
static_injector_neuron::get_t_min_() const
{
  return V_.t_min_;
}

inline long
static_injector_neuron::get_t_max_() const
{
  return V_.t_max_;
}

} // namespace


/* Need to set this somewhere if precise_times = true

// activate off-grid communication only after nodes have been created
  // successfully
  if ( model->is_off_grid() )
  {
    kernel().event_delivery_manager.set_off_grid_communication( true );
    LOG( M_INFO,
      "NodeManager::add_node",
      "Neuron models emitting precisely timed spikes exist: "
      "the kernel property off_grid_spiking has been set to true.\n\n"
      "NOTE: Mixing precise-spiking and normal neuron models may "
      "lead to inconsistent results." );
  }
*/

#endif // STATIC_INJECTOR_NEURON_H
