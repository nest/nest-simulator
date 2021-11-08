/*
 *  music_rate_in_proxy.h
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

#ifndef MUSIC_RATE_IN_PROXY_H
#define MUSIC_RATE_IN_PROXY_H

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <vector>

// External includes:
#include <music.hh>

// Includes from nestkernel:
#include "device_node.h"
#include "nest_types.h"
#include "node.h"

// Includes from sli:
#include "arraydatum.h"

/* BeginUserDocs: device, rate, MUSIC

Short description
+++++++++++++++++

A device which receives rate data from MUSIC

Description
+++++++++++

A music_rate_in_proxy can be used to receive rate data from
remote MUSIC applications in NEST.

It uses the MUSIC library to receive the data from other applications.
The music_rate_in_proxy represents a complete port to which MUSIC can
connect and send data. The music_rate_in_proxy can be queried using
GetStatus to retrieve the messages.

Parameters
++++++++++

The following properties are available in the status dictionary:

port_name      - The name of the MUSIC input port to listen to (default:
                 rate_in)
port_width     - The width of the MUSIC input port
data           - The data received on the port as vector of doubles
published      - A bool indicating if the port has been already published
                 with MUSIC

The parameter port_name can be set using SetStatus.

Examples
++++++++

/music_rate_in_proxy Create /mcip Set
10 Simulate
mcip GetStatus /data get /gaze_directions Set

Availability: Only when compiled with MUSIC

See also
++++++++

music_rate_out_proxy, music_cont_in_proxy

EndUserDocs*/

namespace nest
{
/**
 * Emit rate at times received from another application via a
 * MUSIC port.
 */
class music_rate_in_proxy : public DeviceNode
{

public:
  music_rate_in_proxy();
  music_rate_in_proxy( const music_rate_in_proxy& );

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

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( InstantaneousRateConnectionEvent& );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  using Node::sends_secondary_event;
  void
  sends_secondary_event( InstantaneousRateConnectionEvent& )
  {
  }
  void
  sends_secondary_event( DelayedRateConnectionEvent& )
  {
  }

private:
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // ------------------------------------------------------------

  struct State_;

  struct Parameters_
  {
    std::string port_name_; //!< the name of MUSIC port to connect to
    int channel_;           //!< the MUSIC channel of the port

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;          //!< Store current values in dictionary
    void set( const DictionaryDatum&, State_& ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct State_
  {
    bool registered_; //!< indicates whether this node has been published already
                      //!< with MUSIC

    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //! Set values from dictionary
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    double data_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
  Variables_ V_;
};

} // namespace

#endif

#endif /* #ifndef MUSIC_RATE_IN_PROXY_H */
