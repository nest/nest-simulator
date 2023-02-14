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

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding,
   * Overloading, and Hiding
   */
  using Node::receives_signal;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool ) override;
  SignalType sends_signal() const override;
  SignalType receives_signal() const override;


  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void set_data( std::vector< double >& input_spikes );

private:
  void init_buffers_() override;
  void
  pre_run_hook() override
  {
  } // no variables

  void update( Time const&, const long, const long ) override;

  /**
   * Synapse type of the first outgoing connection made by the Device.
   *
   * Used to check that devices connect using only a single synapse type,
   * see #481 and #737. Since this value must survive resets, it is
   * stored here, even though it is an implementation detail.
   */
  synindex first_syn_id_;

  void enforce_single_syn_type( synindex syn_id );

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

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
};


inline port
static_injector_neuron::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  enforce_single_syn_type( syn_id );


  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

void
static_injector_neuron::enforce_single_syn_type( synindex syn_id )
{
  if ( first_syn_id_ == invalid_synindex )
  {
    first_syn_id_ = syn_id;
  }
  if ( syn_id != first_syn_id_ )
  {
    throw IllegalConnection( "All outgoing connections from a device must use the same synapse type." );
  }
}


void
static_injector_neuron::set_data( std::vector< double >& input_spikes )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  if ( ptmp.precise_times_ and not input_spikes.empty() )
  {
    throw BadProperty( "Option precise_times is not supported with an stimulation backend\n" );
  }

  // TODO: get_origin must be implemented in static_injector_neuron
  // const Time& origin = StimulationDevice::get_origin();
  // Tempororary placeholder:
  Time origin = Time::step( 0 );

  // For the input backend
  if ( not input_spikes.empty() )
  {

    DictionaryDatum d = DictionaryDatum( new Dictionary );
    std::vector< double > times_ms;
    const size_t n_spikes = P_.spike_stamps_.size();
    times_ms.reserve( n_spikes + input_spikes.size() );
    for ( size_t n = 0; n < n_spikes; ++n )
    {
      times_ms.push_back( P_.spike_stamps_[ n ].get_ms() );
    }
    std::copy( input_spikes.begin(), input_spikes.end(), std::back_inserter( times_ms ) );
    ( *d )[ names::spike_times ] = DoubleVectorDatum( times_ms );

    ptmp.set( d, S_, origin, Time::step( times_ms[ times_ms.size() - 1 ] ), this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
// -------------------------------


inline SignalType
static_injector_neuron::sends_signal() const
{
  return SPIKE;
}

inline SignalType
static_injector_neuron::receives_signal() const
{
  return NONE;
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
