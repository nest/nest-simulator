/*
 *  music_event_handler.h
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

#ifndef MUSIC_EVENT_HANDLER
#define MUSIC_EVENT_HANDLER

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
 * Event handler for all events of a MUSIC port received on this process.
 */
class MusicEventHandler : public MUSIC::EventHandlerGlobalIndex
{
public:
  MusicEventHandler();
  MusicEventHandler( std::string portname, double acceptable_latency, int max_buffered );

  virtual ~MusicEventHandler();

  /**
   * Register a new node to a specific channel on this port.
   */
  void register_channel( size_t channel, nest::Node* mp );

  /**
   * Publish the MUSIC port.
   * This method has to be called once before the first simulation to
   * tell MUSIC which channels lie on which processor.
   */
  void publish_port();

  /**
   * Called by MUSIC from within tick() to deliver events to
   * NEST. This function only queues the events. Delivery to the
   * targets takes place in update().
   */
  void operator()( double t, MUSIC::GlobalIndex channel );

  /**
   * This function is called by the scheduler and delivers the queued
   * events to the target music_in_proxies.
   */
  void update( Time const&, const long, const long );

private:
  MUSIC::EventInputPort* music_port_;
  MUSIC::PermutationIndex* music_perm_ind_;
  bool published_;
  std::string portname_;
  //! Maps channel number to music_event_in_proxy
  std::vector< nest::Node* > channelmap_;
  //! Maps local index to global MUSIC index (channel)
  std::vector< MUSIC::GlobalIndex > indexmap_;
  double acceptable_latency_; //!< The acceptable latency of the port in ms
  int max_buffered_;

  /**
   * Buffers incoming spike events until they are due. The vector has
   * one entry per channel. The priority queues used within the vector
   * implement min-heaps stored in vectors.
   */
  std::vector< std::priority_queue< double, std::vector< double >, std::greater< double > > > eventqueue_;
};

} // namespace nest

#endif // HAVE_MUSIC

#endif // MUSIC_EVENT_HANDLER
