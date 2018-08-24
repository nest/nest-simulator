/*
 *  music_event_out_proxy.h
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

#ifndef MUSIC_EVENT_OUT_PROXY_H
#define MUSIC_EVENT_OUT_PROXY_H

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <vector>

// External includes:
#include <music.hh>

// Includes from nestkernel:
#include "device_node.h"
#include "event.h"
#include "exceptions.h"
#include "nest_types.h"

/* BeginDocumentation

Name: music_event_out_proxy - Device to forward spikes to remote applications
                              using MUSIC.

Description:
A music_event_out_proxy is used to send spikes to a remote application that
also uses MUSIC.

The music_event_out_proxy represents a complete MUSIC event output
port. The channel on the port to which a source node forwards its
events is determined during connection setup by using the parameter
music_channel of the connection. The name of the port is set via
SetStatus (see Parameters section below).

Parameters:
The following properties are available in the status dictionary:

port_name      - The name of the MUSIC output_port to forward events to
                 (default: event_out)
port_width     - The width of the MUSIC input port
published      - A bool indicating if the port has been already published
                 with MUSIC

The parameter port_name can be set using SetStatus.

Examples:
/iaf_psc_alpha Create /n Set
/music_event_out_proxy Create /meop Set
n meop << /music_channel 2 >> Connect

Author: Moritz Helias, Jochen Martin Eppler
FirstVersion: March 2009
Availability: Only when compiled with MUSIC

SeeAlso: music_event_in_proxy, music_cont_in_proxy, music_message_in_proxy
*/

namespace nest
{
class music_event_out_proxy : public DeviceNode
{

public:
  music_event_out_proxy();
  music_event_out_proxy( const music_event_out_proxy& );
  ~music_event_out_proxy();

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

  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
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

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum&, State_& ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct State_
  {
    bool published_; //!< indicates whether this node has been published already
                     //!< with MUSIC
    int port_width_; //!< the width of the MUSIC port

    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //!< Set values from dictionary
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    MUSIC::EventOutputPort* MP_; //!< The MUSIC event port for output of spikes
    std::vector< MUSIC::GlobalIndex > index_map_;
    MUSIC::PermutationIndex* music_perm_ind_; //!< The permutation index needed
                                              //!< to map the ports of MUSIC.
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
};

inline port
music_event_out_proxy::handles_test_event( SpikeEvent&, rport receptor_type )
{
  // receptor_type i is mapped to channel i of the MUSIC port so we
  // have to generate the index map here, that assigns the channel
  // number to the local index of this connection the local index
  // equals the number of connection

  if ( not S_.published_ )
  {
    V_.index_map_.push_back( static_cast< int >( receptor_type ) );
  }
  else
  {
    throw MUSICPortAlreadyPublished( get_name(), P_.port_name_ );
  }

  return receptor_type;
}

} // namespace

#endif /* #ifndef MUSIC_EVENT_OUT_PROXY_H */

#endif
