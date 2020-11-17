/*
 *  music_manager.cpp
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

#include "music_manager.h"

// C includes:
#ifdef HAVE_MPI
#ifndef HAVE_MUSIC
#include <mpi.h>
#endif
#endif

// C++ includes:
//#include <cstdlib>

// Includes from libnestutil:
#include "compose.hpp"
//#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

MUSICManager::MUSICManager()
{
#ifdef HAVE_MUSIC
  music_setup = 0;
  music_runtime = 0;
  music_in_portlist_.clear();
#endif
}

void
MUSICManager::initialize()
{
#ifdef HAVE_MUSIC
  // Reset music_in_portlist_ to its pristine state.
  // See comment above pristine_music_in_portlist_ in the header.
  music_in_portlist_ = pristine_music_in_portlist_;
#endif
}

void
MUSICManager::finalize()
{
}

/*
     - set the ... properties
*/
void
MUSICManager::set_status( const DictionaryDatum& )
{
}

void
MUSICManager::get_status( DictionaryDatum& )
{
}

#ifdef HAVE_MUSIC

void
MUSICManager::init_music( int* argc, char** argv[] )
{
  int provided_thread_level;
  music_setup = new MUSIC::Setup( *argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level );
}

void
MUSICManager::enter_runtime( double h_min_delay )
{
  publish_music_in_ports_();
  std::string msg = String::compose( "Entering MUSIC runtime with tick = %1 ms", h_min_delay );
  LOG( M_INFO, "MUSICManager::enter_runtime", msg );

  // MUSIC needs the step size in seconds
  // std::cout << "nest::MPIManager::enter_runtime\n";
  // std::cout << "timestep = " << h_min_delay*1e-3 << std::endl;
  if ( music_runtime == 0 )
  {
    music_runtime = new MUSIC::Runtime( music_setup, h_min_delay * 1e-3 );
  }
}

#else /* #ifdef HAVE_MUSIC */

void
MUSICManager::init_music( int*, char*** )
{
}

void
MUSICManager::enter_runtime( double )
{
}

#endif /* #ifdef HAVE_MUSIC */


void
MUSICManager::music_finalize()
{
#ifdef HAVE_MUSIC
  if ( music_runtime == 0 )
  {
    // we need a Runtime object to call finalize(), so we create
    // one, if we don't have one already
    music_runtime = new MUSIC::Runtime( music_setup, 1e-3 );
  }

  music_runtime->finalize();
  delete music_runtime;
#else /* #ifdef HAVE_MUSIC */
#ifdef HAVE_MPI
  MPI_Finalize();
#endif
#endif /* #ifdef HAVE_MUSIC */
}

#ifdef HAVE_MUSIC
MPI::Intracomm
MUSICManager::communicator()
{
  return music_setup->communicator();
}

MUSIC::Setup*
MUSICManager::get_music_setup()
{
  return music_setup;
}

MUSIC::Runtime*
MUSICManager::get_music_runtime()
{
  return music_runtime;
}

void
MUSICManager::advance_music_time()
{
  music_runtime->tick();
}

void
MUSICManager::register_music_in_port( std::string portname, bool pristine )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
  {
    music_in_portlist_[ portname ] = MusicPortData( 1, 0.0, -1 );
  }
  else
  {
    music_in_portlist_[ portname ].n_input_proxies++;
  }

  // pristine is true if we are building up the initial portlist
  if ( pristine )
  {
    pristine_music_in_portlist_[ portname ] = music_in_portlist_[ portname ];
  }
}

void
MUSICManager::unregister_music_in_port( std::string portname )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
  {
    throw MUSICPortUnknown( portname );
  }
  else
  {
    music_in_portlist_[ portname ].n_input_proxies--;
  }

  if ( music_in_portlist_[ portname ].n_input_proxies == 0 )
  {
    music_in_portlist_.erase( it );
  }
}

void
MUSICManager::register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp )
{
  std::map< std::string, MusicEventHandler >::iterator it;
  it = music_event_in_portmap_.find( portname );
  if ( it == music_event_in_portmap_.end() )
  {
    MusicEventHandler tmp(
      portname, music_in_portlist_[ portname ].acceptable_latency, music_in_portlist_[ portname ].max_buffered );
    tmp.register_channel( channel, mp );
    music_event_in_portmap_[ portname ] = tmp;
  }
  else
  {
    it->second.register_channel( channel, mp );
  }
}

void
MUSICManager::register_music_rate_in_proxy( std::string portname, int channel, nest::Node* mp )
{
  std::map< std::string, MusicRateInHandler >::iterator it;
  it = music_rate_in_portmap_.find( portname );
  if ( it == music_rate_in_portmap_.end() )
  {
    MusicRateInHandler tmp( portname );
    tmp.register_channel( channel, mp );
    music_rate_in_portmap_[ portname ] = tmp;
  }
  else
  {
    it->second.register_channel( channel, mp );
  }
}

void
MUSICManager::set_music_in_port_acceptable_latency( std::string portname, double latency )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
  {
    throw MUSICPortUnknown( portname );
  }
  else
  {
    music_in_portlist_[ portname ].acceptable_latency = latency;
  }
}

void
MUSICManager::set_music_in_port_max_buffered( std::string portname, int maxbuffered )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
  {
    throw MUSICPortUnknown( portname );
  }
  else
  {
    music_in_portlist_[ portname ].max_buffered = maxbuffered;
  }
}

void
MUSICManager::publish_music_in_ports_()
{
  for ( std::map< std::string, MusicEventHandler >::iterator it = music_event_in_portmap_.begin();
        it != music_event_in_portmap_.end();
        ++it )
  {
    it->second.publish_port();
  }

  for ( std::map< std::string, MusicRateInHandler >::iterator it = music_rate_in_portmap_.begin();
        it != music_rate_in_portmap_.end();
        ++it )
  {
    it->second.publish_port();
  }
}

void
MUSICManager::update_music_event_handlers( Time const& origin, const long from, const long to )
{
  for ( std::map< std::string, MusicEventHandler >::iterator it = music_event_in_portmap_.begin();
        it != music_event_in_portmap_.end();
        ++it )
  {
    it->second.update( origin, from, to );
  }

  for ( std::map< std::string, MusicRateInHandler >::iterator it = music_rate_in_portmap_.begin();
        it != music_rate_in_portmap_.end();
        ++it )
  {
    it->second.update( origin, from, to );
  }
}
#endif
}
