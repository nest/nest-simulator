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

// Includes from librandom:
#include "exp_randomdev.h"

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "stimulating_device.h"

/*BeginDocumentation
Name: poisson_generator_ps - simulate neuron firing with Poisson processes
(with arbitrary dead time) statistics and exact timing
Description:

  The poisson_generator_ps generator simulates a neuron firing with Poisson
  statistics (with dead time), ie, exponentially distributed interspike
  intervals plus constant dead time, spike events have exact timing
  (i.e. not binned).

Parameters:
   The following parameters appear in the element's status dictionary:

   rate     - mean firing rate. (double, var)
   dead_time - minimal time between two spikes. (double, var)

Remarks:
   - This generator must be connected to all its targets using the
     same synapse model. Failure to do so will only be detected at
     runtime.
   - This generator has only been validated in a very basic manner.

   Sends: SpikeEvent

SeeAlso: poisson_generator, spike_generator, Device, StimulatingDevice
*/

namespace nest
{

/**
 * Poisson generator (with dead time) with precisely timed spikes.
 *
 * This Poisson process (with dead time) generator sends different spike
 * trains to all its targets.
 * All spikes are sent individually with offsets identifying their precise
 * times.
 *
 * @ingroup Devices
 */
class poisson_generator_ps : public Node
{

public:
  poisson_generator_ps();
  poisson_generator_ps( const poisson_generator_ps& );

  bool
  has_proxies() const
  {
    return false;
  }
  bool
  is_off_grid() const
  {
    return true;
  } // uses off_grid events

  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  /**
   * Update state.
   * Update cannot send spikes directly, since we need to identify each
   * target to know the time of the most recent spike sent. Since target
   * information is in the Connectors, we send a DSSpikeEvent to all
   * targets, which is reflected to this->event_hook() with target
   * information.
   * @see event_hook, DSSpikeEvent
   */
  void update( Time const&, const long, const long );

  /**
   * Send out spikes.
   * Called once per target to dispatch actual output spikes.
   * @param contains target information.
   */
  void event_hook( DSSpikeEvent& );

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

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
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
    double inv_rate_ms_;              //!< 1000.0 / Parameters_.rate_
    librandom::ExpRandomDev exp_dev_; //!< random deviate generator

    /**
     * @name update-hook communication.
     * The following variables are used for direct communication from
     * update() to event_hook(). They rely on the fact that event_hook()
     * is called instantaneuously from update().
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

  StimulatingDevice< CurrentEvent > device_;
  Parameters_ P_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
poisson_generator_ps::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool dummy_target )
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
    const port p = target.handles_test_event( e, receptor_type );
    if ( p != invalid_port_ and not is_model_prototype() )
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
  device_.get_status( d );
}

inline void
poisson_generator_ps::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace

#endif // POISSON_GENERATOR_PS_H
