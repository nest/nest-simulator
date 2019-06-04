/*
 *  music_event_in_proxy.h
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

#ifndef MUSIC_EVENT_IN_PROXY_H
#define MUSIC_EVENT_IN_PROXY_H

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"

namespace nest
{

/** @BeginDocumentation
Name: music_event_in_proxy - A device which receives spikes from MUSIC.

Description:

A music_event_in_proxy can be used to pass spikes to nodes within NEST
which are received from another application.

It uses the MUSIC library to receive spike events from other
applications. The music_event_in_proxy represents one channel on a port
to which MUSIC can connect an event source. The music_event_in_proxy can
be connected to local neurons or devices within NEST to receive
the events. Multiple music_in_proxies can be configured to listen
on the same port, but each channel can only listened to by a
single proxy.

Parameters:

The following properties are available in the status dictionary:

port_name      - The name of the MUSIC input port to listen to (default:
                 event_in)
music_channel  - The MUSIC global index on the input port to listen to
registered     - A bool indicating if the port has been already registered
                 with the corresponding MUSIC event handler

The parameters port_name and music_channel can be set using SetStatus.
The acceptable latency of the MUSIC input port can be set using the
command SetAcceptableLatency.

Examples:

/music_event_in_proxy Create /meip Set
meip << /music_channel 2 >> SetStatus
/iaf_psc_alpha Create /n Set
(event_in) 0.2 SetAcceptableLatency
meip n Connect

Author: Moritz Helias, Jochen Martin Eppler

FirstVersion: October 2008

Availability: Only when compiled with MUSIC

SeeAlso: SetAcceptableLatency, music_event_out_proxy, music_cont_in_proxy,
music_message_in_proxy
*/
class music_event_in_proxy : public DeviceNode
{

public:
  music_event_in_proxy();
  music_event_in_proxy( const music_event_in_proxy& );

  bool
  has_proxies() const
  {
    return false;
  } // a copy on each process
  bool
  one_node_per_process() const
  {
    return true;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& );
  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void
  update( Time const&, const long, const long )
  {
  }

  // ------------------------------------------------------------
  struct State_;

  struct Parameters_
  {
    std::string port_name_; //!< the name of MUSIC port to connect to
    int channel_;           //!< the MUSIC channel of the port

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum& ) const;

    /**
     * Set values from dicitonary.
     */
    void set( const DictionaryDatum&, State_& );
  };

  // ------------------------------------------------------------

  struct State_
  {
    bool registered_; //!< indicates whether this node has been registered
                      //!< already with MUSIC

    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //!< Set values from dictionary
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
};

inline port
music_event_in_proxy::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

} // namespace

#endif

#endif /* #ifndef MUSIC_EVENT_IN_PROXY_H */
