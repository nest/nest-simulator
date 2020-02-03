/*
 *  music_event_handler.cpp
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

#include "music_event_handler.h"

#ifdef HAVE_MUSIC

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event.h"
#include "kernel_manager.h"
#include "nest_types.h"

namespace nest
{
MusicEventHandler::MusicEventHandler()
  : music_port_( 0 )
  , music_perm_ind_( 0 )
  , published_( false )
  , portname_( "" )
  , acceptable_latency_( 0.0 )
  , max_buffered_( -1 )
{
}

MusicEventHandler::MusicEventHandler( std::string portname, double acceptable_latency, int max_buffered )
  : music_port_( 0 )
  , music_perm_ind_( 0 )
  , published_( false )
  , portname_( portname )
  , acceptable_latency_( acceptable_latency )
  , max_buffered_( max_buffered )
{
}

MusicEventHandler::~MusicEventHandler()
{
  if ( published_ )
  {
    if ( music_perm_ind_ != 0 )
    {
      delete music_perm_ind_;
    }
    if ( music_port_ != 0 )
    {
      delete music_port_;
    }
  }
}

void
MusicEventHandler::register_channel( size_t channel, nest::Node* mp )
{
  if ( channel >= channelmap_.size() )
  {
    // all entries not explicitly set will be 0
    channelmap_.resize( channel + 1, 0 );
    eventqueue_.resize( channel + 1 );
  }
  if ( channelmap_[ channel ] != 0 )
  {
    throw MUSICChannelAlreadyMapped( "MusicEventHandler", portname_, channel );
  }

  channelmap_[ channel ] = mp;
  indexmap_.push_back( channel );
}

void
MusicEventHandler::publish_port()
{
  if ( not published_ )
  {
    music_port_ = kernel().music_manager.get_music_setup()->publishEventInput( portname_ );

    // MUSIC wants seconds, NEST has miliseconds
    const double acceptable_latency_s = 0.001 * acceptable_latency_;

    if ( not music_port_->isConnected() )
    {
      throw MUSICPortUnconnected( "MusicEventHandler", portname_ );
    }

    if ( not music_port_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( "MusicEventHandler", portname_ );
    }

    unsigned int music_port_width = music_port_->width();

    // check, if all mappings are within the valid range of port width
    // the maximum channel mapped - 1 == size of channelmap
    if ( channelmap_.size() > music_port_width )
    {
      throw MUSICChannelUnknown( "MusicEventHandler", portname_, channelmap_.size() - 1 );
    }

    // create the permutation index mapping
    music_perm_ind_ = new MUSIC::PermutationIndex( &indexmap_.front(), indexmap_.size() );
    // map the port
    if ( max_buffered_ >= 0 )
    {
      music_port_->map( music_perm_ind_, this, acceptable_latency_s, max_buffered_ );
    }
    else
    {
      music_port_->map( music_perm_ind_, this, acceptable_latency_s );
    }

    std::string msg = String::compose( "Mapping MUSIC input port '%1' with width=%2 , acceptable latency=%3 ms",
      portname_,
      music_port_width,
      acceptable_latency_ );
    if ( max_buffered_ > 0 )
    {
      msg += String::compose( " and max buffered=%1 ticks", max_buffered_ );
    }
    msg += ".";
    LOG( M_INFO, "MusicEventHandler::publish_port()", msg.c_str() );
  }
}

void MusicEventHandler::operator()( double t, MUSIC::GlobalIndex channel )
{
  assert( channelmap_[ channel ] != 0 );
  eventqueue_[ channel ].push( t * 1e3 ); // MUSIC uses seconds as time unit
}

void
MusicEventHandler::update( Time const& origin, const long from, const long to )
{
  for ( size_t channel = 0; channel < channelmap_.size(); ++channel )
  {
    if ( channelmap_[ channel ] != 0 )
    {
      while ( not eventqueue_[ channel ].empty() )
      {
        Time T = Time::ms( eventqueue_[ channel ].top() );

        if ( T > origin + Time::step( from ) - Time::ms( acceptable_latency_ )
          and T <= origin + Time::step( from + to ) )
        {
          nest::SpikeEvent se;
          se.set_offset( Time( Time::step( T.get_steps() ) ).get_ms() - T.get_ms() );
          se.set_stamp( T );

          // deliver to the proxy for this channel
          channelmap_[ channel ]->handle( se );
          eventqueue_[ channel ].pop(); // remove the sent event from the queue
        }
        else
        {
          break;
        }
      }
    }
  }
}

} // namespace nest

#endif // #ifdef HAVE_MUSIC
