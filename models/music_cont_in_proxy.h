/*
 *  music_cont_in_proxy.h
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

#ifndef MUSIC_CONT_IN_PROXY_H
#define MUSIC_CONT_IN_PROXY_H

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C includes:
#include <mpi.h>

// C++ includes:
#include <vector>

// External includes:
#include <music.hh>

// Includes from nestkernel:
#include "device_node.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"

namespace nest
{

/** @BeginDocumentation

Name: music_cont_in_proxy - A device which receives continuous data from MUSIC.

Description:

A music_cont_in_proxy can be used to receive continuous data from
remote MUSIC applications in NEST.

It uses the MUSIC library to receive the data from other applications.
The music_cont_in_proxy represents a complete port to which MUSIC can
connect and send data. The music_cont_in_proxy can queried using
GetStatus to retrieve the messages.

Parameters:

The following properties are available in the status dictionary:

port_name      - The name of the MUSIC input port to listen to (default:
                 cont_in)
port_width     - The width of the MUSIC input port
data           - The data received on the port as vector of doubles
published      - A bool indicating if the port has been already published
                 with MUSIC

The parameter port_name can be set using SetStatus.

Examples:

/music_cont_in_proxy Create /mcip Set
10 Simulate
mcip GetStatus /data get /gaze_directions Set

Author: Jochen Martin Eppler

FirstVersion: July 2010

Availability: Only when compiled with MUSIC

SeeAlso: music_event_out_proxy, music_event_in_proxy, music_message_in_proxy
*/
class music_cont_in_proxy : public DeviceNode
{

public:
  music_cont_in_proxy();
  music_cont_in_proxy( const music_cont_in_proxy& );

  bool
  has_proxies() const
  {
    return false;
  }
  bool
  one_node_per_process() const
  {
    return true;
  }

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
    //! Set values from dictionary
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    std::vector< double > data_; //!< The buffer for incoming data
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    MUSIC::ContInputPort* MP_; //!< The MUSIC cont port for input of data
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
  Variables_ V_;
};

} // namespace

#endif

#endif /* #ifndef MUSIC_CONT_IN_PROXY_H */
