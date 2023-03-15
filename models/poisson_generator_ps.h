/*
 *  poisson_generator_ps.h
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

#ifndef POISSON_GENERATOR_PS_H
#define POISSON_GENERATOR_PS_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "random_generators.h"
#include "stimulation_device.h"

namespace nest
{

/* BeginUserDocs: device, generator, precise

Short description
+++++++++++++++++

Simulated neuron firing with Poisson process statistics - precise
spike timing version with arbitrary dead times

Description
+++++++++++

The ``poisson_generator_ps`` simulates a neuron firing with Poisson
statistics (with dead time), that is, exponentially distributed interspike
intervals plus constant dead time, spike events have exact timing,
that is, they are not constrained to the simulation time grid.

.. note::
   This generator must be connected to all its targets using the
   same synapse model. Failure to do so will only be detected at
   runtime.

.. include:: ../models/stimulation_device.rst

rate
    Mean firing rate (spikes/s)

dead_time
    Minimal time between two spikes (ms)

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. dead_time
 1. rate

Sends
+++++

SpikeEvent

See also
++++++++

poisson_generator, parrot_neuron_ps

EndUserDocs */

class poisson_generator_ps : public StimulationDevice
{

public:
  poisson_generator_ps();
  poisson_generator_ps( const poisson_generator_ps& );

  bool is_off_grid() const override;

  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void calibrate_time( const TimeConverter& tc ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void pre_run_hook() override;

  /**
   * Update state.
   * Update cannot send spikes directly, since we need to identify each
   * target to know the time of the most recent spike sent. Since target
   * information is in the Connectors, we send a DSSpikeEvent to all
   * targets, which is reflected to this->event_hook() with target
   * information.
   * @see event_hook, DSSpikeEvent
   */
  void update( Time const&, const long, const long ) override;

  /**
   * Send out spikes.
   * Called once per target to dispatch actual output spikes.
   * @param contains target information.
   */
  void event_hook( DSSpikeEvent& ) override;

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double rate_;      //!< process rate [Hz]
    double dead_time_; //!< dead time [ms]

    /**
     * Number of targets.
     * This is a hidden parameter; must be placed in parameters,
     * even though it is an implementation detail, since it
     * concerns the connections and must not be affected by resets.
     */
    size_t num_targets_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    typedef std::pair< Time, double > SpikeTime;

    /**
     * Time of next spike represented as time stamp and offset, for each target.
     *   - first: time stamp
     *   - second: offset (<=0)
     * @note first == Time::neg_inf() marks that no spike has been generated yet
     *   and that an initial interval needs to be drawn.
     */
    std::vector< SpikeTime > next_spike_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    double inv_rate_ms_;               //!< 1000.0 / Parameters_.rate_
    exponential_distribution exp_dev_; //!< random deviate generator

    /**
     * @name update-hook communication.
     * The following variables are used for direct communication from
     * update() to event_hook(). They rely on the fact that event_hook()
     * is called instantaneously from update().
     * Spikes are sent at times t that fulfill
     *
     *   t_min_active_ < t <= t_max_active_
     */
    //@{
    Time t_min_active_; //!< start of generator activity in slice
    Time t_max_active_; //!< end of generator activity in slice
    //@}
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
poisson_generator_ps::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

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
    const port p = target.handles_test_event( e, receptor_type );
    if ( p != invalid_port and not is_model_prototype() )
    {
      ++P_.num_targets_; // count number of targets
    }
    return p;
  }
}

inline void
poisson_generator_ps::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );
}

inline void
poisson_generator_ps::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // If the rate is changed, the event_hook must handle the interval from
  // the rate change to the first subsequent spike.
  if ( d->known( names::rate ) )
  {
    B_.next_spike_.assign( P_.num_targets_, Buffers_::SpikeTime( Time::neg_inf(), 0 ) );
  }

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline void
poisson_generator_ps::calibrate_time( const TimeConverter& tc )
{
  V_.t_min_active_ = tc.from_old_tics( V_.t_min_active_.get_tics() );
  V_.t_max_active_ = tc.from_old_tics( V_.t_max_active_.get_tics() );
}

inline bool
poisson_generator_ps::is_off_grid() const
{
  return true;
}

inline StimulationDevice::Type
poisson_generator_ps::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace

#endif // POISSON_GENERATOR_PS_H
