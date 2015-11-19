/*
 *  music_event_in_proxy.cpp
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

#include "config.h"

#ifdef HAVE_MUSIC

#include "music_event_in_proxy.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "arraydatum.h"
#include "dictutils.h"
#include "music.hh"


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_event_in_proxy::Parameters_::Parameters_()
  : port_name_( "event_in" )
  , channel_( 0 )
{
}

nest::music_event_in_proxy::Parameters_::Parameters_( const Parameters_& op )
  : port_name_( op.port_name_ )
  , channel_( op.channel_ )
{
}

nest::music_event_in_proxy::State_::State_()
{
}


/* ----------------------------------------------------------------
 * Paramater extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_event_in_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::music_channel ] = channel_;
  ( *d )[ names::port_name ] = port_name_;
}

void
nest::music_event_in_proxy::Parameters_::set( const DictionaryDatum& d, State_& s )
{
  if ( !s.registered_ )
  {
    updateValue< long >( d, names::music_channel, channel_ );
    updateValue< string >( d, names::port_name, port_name_ );
  }
}

void
nest::music_event_in_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::registered ] = registered_;
}

void
nest::music_event_in_proxy::State_::set( const DictionaryDatum&, const Parameters_& )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_event_in_proxy::music_event_in_proxy()
  : Node()
  , P_()
  , S_()
{
}

nest::music_event_in_proxy::music_event_in_proxy( const music_event_in_proxy& n )
  : Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
  network()->register_music_in_port( P_.port_name_, true );
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::music_event_in_proxy::init_state_( const Node& proto )
{
  const music_event_in_proxy& pr = downcast< music_event_in_proxy >( proto );

  S_ = pr.S_;
}

void
nest::music_event_in_proxy::init_buffers_()
{
}

void
nest::music_event_in_proxy::calibrate()
{
  // register my port and my channel at the scheduler
  if ( !S_.registered_ )
  {
    network()->register_music_event_in_proxy( P_.port_name_, P_.channel_, this );
    S_.registered_ = true;
  }
}

void
nest::music_event_in_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
}

void
nest::music_event_in_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_ );     // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  network()->register_music_in_port( ptmp.port_name_ );
  network()->unregister_music_in_port( P_.port_name_ );

  P_ = ptmp;
  S_ = stmp;
}

void
nest::music_event_in_proxy::handle( SpikeEvent& e )
{
  e.set_sender( *this );

  for ( thread t = 0; t < network()->get_num_threads(); ++t )
    network()->send_local( t, *this, e );
}

#endif
