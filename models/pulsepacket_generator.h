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

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "random_generators.h"
#include "stimulation_device.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Generate sequence of Gaussian pulse packets

Description
+++++++++++

The pulsepacket_generator produces a spike train contains Gaussian pulse
packets centered about given  times.  A Gaussian pulse packet is
a given number of spikes with normal distributed random displacements
from the center time of the pulse.
It resembles the output of synfire groups of neurons.

Remarks
+++++++

- All targets receive identical spike trains.
- New pulse packets are generated when activity or sdev are changed.
- Gaussian pulse are independently generated for each given
  pulse-center time.
- Both standard deviation and number of spikes may be set at any time.
  Pulses are then re-generated with the new values.

.. include:: ../models/stimulation_device.rst

pulse_times
    Times of the centers of pulses (ms)

activity
    Number of spikes per pulse

sdev
    Standard deviation of spike times in each pulse (ms)

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. activity
 1. sdev
 2. pulse_times

Sends
+++++

SpikeEvent

See also
++++++++

spike_generator

EndUserDocs */

class pulsepacket_generator : public StimulationDevice
{

public:
  pulsepacket_generator();
  pulsepacket_generator( pulsepacket_generator const& );

  // behaves like normal node, since it must provide identical
  // output to all targets

  port send_test_event( Node&, rport, synindex, bool ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;

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
    void set( const DictionaryDatum&, pulsepacket_generator&, Node* );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    std::deque< long > spiketimes_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    normal_distribution normal_dist_; //!< normal distribution

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

  Parameters_ P_;
  Buffers_ B_;
  Variables_ V_;
};

inline port
pulsepacket_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline void
pulsepacket_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );
}

inline void
pulsepacket_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;      // temporary copy in case of errors
  ptmp.set( d, *this, this ); // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline StimulationDevice::Type
pulsepacket_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}

} // namespace nest

#endif
