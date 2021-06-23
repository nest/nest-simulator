/*
 *  music_rate_in_proxy.cpp
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

#include "music_rate_in_proxy.h"

#ifdef HAVE_MUSIC

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "event_delivery_manager_impl.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_rate_in_proxy::Parameters_::Parameters_()
  : port_name_( "rate_in" )
  , channel_( 0 )
{
}

nest::music_rate_in_proxy::State_::State_()
  : registered_( false )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_rate_in_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
}

void
nest::music_rate_in_proxy::Parameters_::set( const DictionaryDatum& d, State_& s )
{
  // TODO: This is not possible, as P_ does not know about get_name()
  //  if(d->known(names::port_name) && s.registered_)
  //    throw MUSICPortAlreadyPublished(get_name(), P_.port_name_);

  if ( not s.registered_ )
  {
    updateValue< string >( d, names::port_name, port_name_ );
    updateValue< long >( d, names::music_channel, channel_ );
  }
}

void
nest::music_rate_in_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::registered ] = registered_;
}

void
nest::music_rate_in_proxy::State_::set( const DictionaryDatum&, const Parameters_& )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_rate_in_proxy::music_rate_in_proxy()
  : DeviceNode()
  , P_()
  , S_()
{
}

nest::music_rate_in_proxy::music_rate_in_proxy( const music_rate_in_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
  kernel().music_manager.register_music_in_port( P_.port_name_, true );
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::music_rate_in_proxy::init_buffers_()
{
}

void
nest::music_rate_in_proxy::calibrate()
{
  // only publish the port once
  if ( not S_.registered_ )
  {
    kernel().music_manager.register_music_rate_in_proxy( P_.port_name_, P_.channel_, this );
    S_.registered_ = true;
  }
}

void
nest::music_rate_in_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  ( *d )[ names::data ] = DoubleVectorDatum( new std::vector< double >( 1, B_.data_ ) );
}

void
nest::music_rate_in_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_ );     // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  kernel().music_manager.register_music_in_port( ptmp.port_name_ );
  kernel().music_manager.unregister_music_in_port( P_.port_name_ );
  P_ = ptmp;
  S_ = stmp;
}

void
nest::music_rate_in_proxy::update( Time const&, const long, const long )
{
}

void
nest::music_rate_in_proxy::handle( InstantaneousRateConnectionEvent& e )
{
  kernel().event_delivery_manager.send_secondary( *this, e );
}


#endif
