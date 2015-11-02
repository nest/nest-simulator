/*
 *  music_manager.h
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

#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include "config.h"

#ifdef HAVE_MUSIC
#include <music.hh>
#endif
#include <string>

// Includes from nestkernel:
#include "manager_interface.h"
#include "dict.h"
#include "nest_types.h"


/*
Encapsulates all calls to MUSIC. We need to strip out all #ifdef
HAVE_MUSIC from other places and put them here. Look into those
functions:

void nest::Communicator::finalize()

Linked Functions:

void set_status( index, const DictionaryDatum& );
DictionaryDatum get_status( index );
void register_music_in_port( std::string portname );
void unregister_music_in_port( std::string portname );
void register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp );
void set_music_in_port_acceptable_latency( std::string portname, double_t latency );
void set_music_in_port_max_buffered( std::string portname, int_t maxbuffered );
void publish_music_in_ports_();
void update_music_event_handlers_( Time const&, const long_t, const long_t );

Linked Data Structures:

struct MusicPortData
std::map< std::string, MusicPortData > music_in_portlist_;
std::map< std::string, MusicEventHandler > music_in_portmap_;
 */

namespace nest
{

class MUSICManager : ManagerInterface
{
public:
  virtual void initialize();  // called from meta-manager to construct
  virtual void finalize(); // called from meta-manger to reinit

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  MUSICManager();

  void init_music( int* argc, char** argv[] );

  void set_music_in_port_acceptable_latency( std::string portname, double_t latency );
  void set_music_in_port_max_buffered( std::string portname, int_t maxbuffered );
#ifdef HAVE_MUSIC
  MPI_Comm communicator ();
#endif

private:
#ifdef HAVE_MUSIC
  MUSIC::Setup* music_setup;
#endif
};
}


#endif /* MUSIC_MANAGER_H */
