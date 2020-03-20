/*
 *  music_rate_in_handler.h
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

#ifndef MUSIC_RATE_IN_HANDLER
#define MUSIC_RATE_IN_HANDLER

// Generated includes:
#include "config.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <music.hh>
#include <queue>

// Includes from nestkernel:
#include "nest_types.h"
#include "node.h"


namespace nest
{

/**
 * RateIn handler for all events of a MUSIC port received on this process.
 */
class MusicRateInHandler
{
public:
  MusicRateInHandler();
  MusicRateInHandler( std::string portname );

  virtual ~MusicRateInHandler();

  /**
   * Register a new node to a specific channel on this port.
   */
  void register_channel( int channel, nest::Node* mp );

  /**
   * Publish the MUSIC port.
   * This method has to be called once before the first simulation to
   * tell MUSIC which channels lie on which processor.
   */
  void publish_port();

  /**
   * This function is called by the scheduler and delivers the queued
   * events to the target music_in_proxies.
   */
  void update( Time const&, const long, const long );

private:
  MUSIC::ContInputPort* MP_;   //!< The MUSIC rate port for input of data
  std::vector< double > data_; //!< The buffer for incoming data

  bool published_;
  std::string port_name_;

  int port_width_; //!< the width of the MUSIC port
  //! Maps channel number to music_rate_in_proxy
  std::vector< nest::Node* > channelmap_;
};

} // namespace nest

#endif // HAVE_MUSIC

#endif // MUSIC_RATE_IN_HANDLER
