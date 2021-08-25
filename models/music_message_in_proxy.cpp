/*
 *  music_message_in_proxy.cpp
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

#include "music_message_in_proxy.h"

#ifdef HAVE_MUSIC

// External includes:
#include <music.hh>

// Includes from sli:
#include "arraydatum.h"
#include "doubledatum.h"
#include "integerdatum.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_message_in_proxy::Parameters_::Parameters_()
  : port_name_( "message_in" )
  , acceptable_latency_( 0.0 )
{
}

nest::music_message_in_proxy::State_::State_()
  : published_( false )
  , port_width_( -1 )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_message_in_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
  ( *d )[ names::acceptable_latency ] = acceptable_latency_;
}

void
nest::music_message_in_proxy::Parameters_::set( const DictionaryDatum& d, State_& s, Node* node )
{
  if ( not s.published_ )
  {
    updateValue< string >( d, names::port_name, port_name_ );
    updateValueParam< double >( d, names::acceptable_latency, acceptable_latency_, node );
  }
}

void
nest::music_message_in_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

void
nest::music_message_in_proxy::State_::set( const DictionaryDatum&, const Parameters_&, Node* )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_message_in_proxy::music_message_in_proxy()
  : DeviceNode()
  , P_()
  , S_()
{
}

nest::music_message_in_proxy::music_message_in_proxy( const music_message_in_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::music_message_in_proxy::init_buffers_()
{
}

void
nest::music_message_in_proxy::calibrate()
{
  // only publish the port once,
  if ( not S_.published_ )
  {
    MUSIC::Setup* s = kernel().music_manager.get_music_setup();
    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( get_name() );
    }

    V_.MP_ = s->publishMessageInput( P_.port_name_ );

    if ( not V_.MP_->isConnected() )
    {
      throw MUSICPortUnconnected( get_name(), P_.port_name_ );
    }

    if ( not V_.MP_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( get_name(), P_.port_name_ );
    }

    S_.port_width_ = V_.MP_->width();

    // MUSIC wants seconds, NEST has miliseconds
    double acceptable_latency = P_.acceptable_latency_ / 1000.0;

    V_.MP_->map( &B_.message_handler_, acceptable_latency );
    S_.published_ = true;

    std::string msg = String::compose(
      "Mapping MUSIC input port '%1' with width=%2 and acceptable latency=%3 "
      "ms.",
      P_.port_name_,
      S_.port_width_,
      P_.acceptable_latency_ );
    LOG( M_INFO, "music_message_in_proxy::calibrate()", msg.c_str() );
  }
}

#endif
