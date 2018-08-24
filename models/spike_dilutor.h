/*
 *  spike_dilutor.h
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

#ifndef SPIKE_DILUTOR_H
#define SPIKE_DILUTOR_H

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "stimulating_device.h"

namespace nest
{

/* BeginDocumentation
Name: spike_dilutor - repeats incoming spikes with a certain probability.

Description:
  The device repeats incoming spikes with a certain probability.
  Targets will receive diffenrent spike trains.

Remarks:
  In parallel simulations, a copy of the device is present on each process
  and spikes are collected only from local sources.

Parameters:
   The following parameters appear in the element's status dictionary:
   p_copy double - Copy probability

Sends: SpikeEvent

Author: Adapted from mip_generator by Kunkel, Oct 2011
ported to Nest 2.6 by: Setareh, April 2015

SeeAlso: mip_generator
*/

class spike_dilutor : public DeviceNode
{

public:
  spike_dilutor();
  spike_dilutor( const spike_dilutor& rhs );

  bool
  has_proxies() const
  {
    return false;
  }
  bool
  local_receiver() const
  {
    return true;
  }

  using Node::handles_test_event; // new
  using Node::handle;
  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool );
  port handles_test_event( SpikeEvent&, rport );
  void handle( SpikeEvent& );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  void event_hook( DSSpikeEvent& );

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double p_copy_; //!< copy probability for each incoming spike

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

  struct Buffers_
  {
    RingBuffer n_spikes_;
  };

  // ------------------------------------------------------------

  StimulatingDevice< SpikeEvent > device_;
  Parameters_ P_;
  Buffers_ B_;
};

inline port
spike_dilutor::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool )
{

  device_.enforce_single_syn_type( syn_id );

  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
spike_dilutor::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
spike_dilutor::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  device_.get_status( d );
}

inline void
spike_dilutor::set_status( const DictionaryDatum& d )
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

} // namespace nest

#endif
