/*
 *  music_message_in_proxy.h
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

#ifndef MUSIC_MESSAGE_IN_PROXY_H
#define MUSIC_MESSAGE_IN_PROXY_H

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C includes:
#include <mpi.h>

// C++ includes:
#include <string>
#include <vector>

// External includes:
#include <music.hh>

// Includes from nestkernel:
#include "device_node.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"

namespace nest
{
/** @BeginDocumentation
Name: music_message_in_proxy - A device which receives message strings from
                              MUSIC.

Description:

A music_message_in_proxy can be used to receive message strings from
remote MUSIC applications in NEST.

It uses the MUSIC library to receive message strings from other
applications. The music_message_in_proxy represents an input port to
which MUSIC can connect a message source. The music_message_in_proxy
can queried using GetStatus to retrieve the messages.

Parameters:

The following properties are available in the status dictionary:

port_name      - The name of the MUSIC input port to listen to (default:
                 message_in)
port_width     - The width of the MUSIC input port
data           - A sub-dictionary that contains the string messages
                 in the form of two arrays:
                 messages      - The strings
                 message_times - The times the messages were sent (ms)
n_messages     - The number of messages.
published      - A bool indicating if the port has been already published
                 with MUSIC

The parameter port_name can be set using SetStatus. The field n_messages
can be set to 0 to clear the data arrays.

Examples:

/music_message_in_proxy Create /mmip Set
10 Simulate
mmip GetStatus /data get /messages get 0 get /command Set
(Executing command ') command join ('.) join =
command cvx exec

Author: Jochen Martin Eppler

FirstVersion: July 2010

Availability: Only when compiled with MUSIC

SeeAlso: music_event_out_proxy, music_event_in_proxy, music_cont_in_proxy
*/
class MsgHandler : public MUSIC::MessageHandler
{
  ArrayDatum messages;                 //!< The buffer for incoming message
  std::vector< double > message_times; //!< The buffer for incoming message

  void operator()( double t, void* msg, size_t size )
  {
    message_times.push_back( t * 1000.0 );
    messages.push_back( std::string( static_cast< char* >( msg ), size ) );
  }

public:
  void
  get_status( DictionaryDatum& d ) const
  {
    DictionaryDatum dict( new Dictionary );
    ( *dict )[ names::messages ] = messages;
    ( *dict )[ names::message_times ] =
      DoubleVectorDatum( new std::vector< double >( message_times ) );
    ( *d )[ names::n_messages ] = messages.size();
    ( *d )[ names::data ] = dict;
  }

  void
  clear()
  {
    message_times.clear();
    messages.clear();
  }
};

/**
 * Emit spikes at times received from another application via a
 * MUSIC port. The timestamps of the events also contain offsets,
 * which makes it also useful for precise spikes.
 */
class music_message_in_proxy : public DeviceNode
{

public:
  music_message_in_proxy();
  music_message_in_proxy( const music_message_in_proxy& );

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
    std::string port_name_;     //!< the name of MUSIC port to connect to
    double acceptable_latency_; //!< the acceptable latency of the port

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
    MsgHandler message_handler_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    MUSIC::MessageInputPort* MP_; //!< The MUSIC cont port for input of data
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
  Variables_ V_;
};

inline void
music_message_in_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  B_.message_handler_.get_status( d );
}

inline void
music_message_in_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_ );     // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  long nm = 0;
  if ( updateValue< long >( d, names::n_messages, nm ) )
  {
    if ( nm == 0 )
    {
      B_.message_handler_.clear();
    }
    else
    {
      throw BadProperty( "n_messaged can only be set to 0." );
    }
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif

#endif /* #ifndef MUSIC_MESSAGE_IN_PROXY_H */
