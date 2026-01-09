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

#include "music_event_in_proxy.h"

#ifdef HAVE_MUSIC

// External includes:
#include <music.hh>

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"

// Includes from nestkernel:
#include "connection_manager.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "music_manager.h"
#include "nest_impl.h"

void
nest::register_music_event_in_proxy( const std::string& name )
{
  register_node_model< music_event_in_proxy >( name );
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_event_in_proxy::Parameters_::Parameters_()
  : port_name_( "event_in" )
  , channel_( 0 )
{
}

nest::music_event_in_proxy::State_::State_()
  : registered_( false )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
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
  if ( not s.registered_ )
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
  : DeviceNode()
  , P_()
  , S_()
{
  // Register port for the model so it is available as default
  kernel::manager< MUSICManager >.register_music_in_port( P_.port_name_ );
}

nest::music_event_in_proxy::music_event_in_proxy( const music_event_in_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
  // Register port for node instance because MusicManager manages ports via reference count
  kernel::manager< MUSICManager >.register_music_in_port( P_.port_name_ );
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::music_event_in_proxy::init_buffers_()
{
}

void
nest::music_event_in_proxy::pre_run_hook()
{
  // register my port and my channel at the scheduler
  if ( not S_.registered_ )
  {
    kernel::manager< MUSICManager >.register_music_event_in_proxy( P_.port_name_, P_.channel_, this );
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
  kernel::manager< MUSICManager >.unregister_music_in_port( P_.port_name_ );
  kernel::manager< MUSICManager >.register_music_in_port( ptmp.port_name_ );

  P_ = ptmp;
  S_ = stmp;
}

void
nest::music_event_in_proxy::handle( SpikeEvent& e )
{
  e.set_sender( *this );

  for ( size_t t = 0; t < kernel::manager< VPManager >.get_num_threads(); ++t )
  {
    kernel::manager< ConnectionManager >.send_from_device( t, local_device_id_, e );
  }
}

#endif
