/*
 *  pulsepacket_generator.h
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

#ifndef PULSEPACKET_GENERATOR_H
#define PULSEPACKET_GENERATOR_H

// C++ includes:
#include <deque>
#include <vector>

// Includes from librandom:
#include "normal_randomdev.h"

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "stimulating_device.h"

namespace nest
{

/** @BeginDocumentation
Name: pulsepacket_generator - Generate sequence of Gaussian pulse packets.

Description:

The pulsepacket_generator produces a spike train contains Gaussian pulse
packets centered about given  times.  A Gaussian pulse packet is
a given number of spikes with normal distributed random displacements
from the center time of the pulse.
It resembles the output of synfire groups of neurons.

Parameters:

pulse_times  double - Times of the centers of pulses in ms
activity     int    - Number of spikes per pulse
sdev         double - Standard deviation of spike times in each pulse in ms

Remarks:

- All targets receive identical spike trains.
- New pulse packets are generated when activity or sdev are changed.
- Gaussian pulse are independently generated for each given
  pulse-center time.
- Both standard deviation and number of spikes may be set at any time.
  Pulses are then re-generated with the new values.

Sends: SpikeEvent

SeeAlso: spike_generator, StimulatingDevice
*/
class pulsepacket_generator : public Node
{

public:
  pulsepacket_generator();
  pulsepacket_generator( pulsepacket_generator const& );

  // behaves like normal node, since it must provide identical
  // output to all targets
  bool
  has_proxies() const
  {
    return true;
  }

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void create_pulse();
  void update( Time const&, const long, const long );

  struct Buffers_;

  // ------------------------------------------------------------

  struct Parameters_
  {

    std::vector< double > pulse_times_; //!< times of pulses
    long a_;                            //!< number of pulses in a packet
    double sdev_;                       //!< standard deviation of the packet

    double sdev_tolerance_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dicitonary.
     * @note Buffer is passed so that the position etc can be reset
     *       parameters have been changed.
     */
    void set( const DictionaryDatum&, pulsepacket_generator& );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    std::deque< long > spiketimes_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {

    librandom::NormalRandomDev norm_dev_; //!< random deviate generator

    /** Indices into sorted vector of sorted pulse-center times
     *  (P_.pulse_times_). Spike times to be sent are calculated from
     *  pulse-center times between 'start' and 'stop'. Times before 'start' are
     *  outdated, times after 'stop' are not touched yet.
     *
     *  Must be index, not iterator, since we copy pulse times
     *  out of temporary parameter set.
     */
    size_t start_center_idx_;
    size_t stop_center_idx_;
    double tolerance;

    Variables_();
  };

  // ------------------------------------------------------------

  StimulatingDevice< SpikeEvent > device_;

  Parameters_ P_;
  Buffers_ B_;
  Variables_ V_;
};

inline port
pulsepacket_generator::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool )
{
  device_.enforce_single_syn_type( syn_id );

  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline void
pulsepacket_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  device_.get_status( d );
}

inline void
pulsepacket_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, *this );  // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace nest

#endif
