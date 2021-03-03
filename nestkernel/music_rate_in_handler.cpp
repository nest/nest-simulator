/*
 *  music_rate_in_handler.cpp
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

#include "music_rate_in_handler.h"

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
MusicRateInHandler::MusicRateInHandler()
  : MP_( 0 )
  , published_( false )
  , port_name_( "" )
{
}

MusicRateInHandler::MusicRateInHandler( std::string portname )
  : MP_( 0 )
  , published_( false )
  , port_name_( portname )
{
}

MusicRateInHandler::~MusicRateInHandler()
{
  if ( published_ )
  {
    if ( MP_ != 0 )
    {
      delete MP_;
    }
  }
}

void
MusicRateInHandler::register_channel( int channel, nest::Node* mp )
{
  if ( static_cast< size_t >( channel ) >= channelmap_.size() )
  {
    // all entries not explicitly set will be 0
    channelmap_.resize( channel + 1, 0 );
  }

  if ( channelmap_[ channel ] != 0 )
  {
    throw MUSICChannelAlreadyMapped( "MusicRateInHandler", port_name_, channel );
  }

  channelmap_[ channel ] = mp;
}

void
MusicRateInHandler::publish_port()
{

  if ( not published_ )
  {
    MUSIC::Setup* s = kernel().music_manager.get_music_setup();
    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( "" );
    }

    MP_ = s->publishContInput( port_name_ );

    if ( not MP_->isConnected() )
    {
      throw MUSICPortUnconnected( "", port_name_ );
    }

    if ( not MP_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( "", port_name_ );
    }

    port_width_ = MP_->width();

    data_ = std::vector< double >( port_width_ );

    for ( std::vector< double >::iterator it = data_.begin(); it != data_.end(); ++it )
    {
      *it = 0;
    }

    MUSIC::ArrayData data_map( static_cast< void* >( &( data_[ 0 ] ) ), MPI::DOUBLE, 0, port_width_ );

    MP_->map( &data_map );
    published_ = true;

    std::string msg = String::compose( "Mapping MUSIC input port '%1' with width=%2.", port_name_, port_width_ );
    LOG( M_INFO, "music_rate_in_handler::publish_port()", msg.c_str() );
  }
}


void
MusicRateInHandler::update( Time const&, const long, const long )
{
  const size_t buffer_size = kernel().connection_manager.get_min_delay();
  std::vector< double > new_rates( buffer_size, 0.0 );

  for ( size_t channel = 0; channel < channelmap_.size(); ++channel )
  {
    if ( channelmap_[ channel ] != 0 )
    {
      std::fill( new_rates.begin(), new_rates.end(), data_[ channel ] );

      InstantaneousRateConnectionEvent rate_event;
      rate_event.set_coeffarray( new_rates );
      channelmap_[ channel ]->handle( rate_event );
    }
  }
}

} // namespace nest

#endif // #ifdef HAVE_MUSIC
