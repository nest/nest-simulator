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
#include "nest_types.h"

// Includes from sli:
#include "dict.h"

#ifdef HAVE_MUSIC
#include "music_event_handler.h"
#endif

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
void register_music_event_in_proxy( std::string portname, int channel,
nest::Node* mp );
void set_music_in_port_acceptable_latency( std::string portname, double
latency );
void set_music_in_port_max_buffered( std::string portname, int maxbuffered );
void publish_music_in_ports_();
void update_music_event_handlers_( Time const&, const long, const long );

Linked Data Structures:

struct MusicPortData
std::map< std::string, MusicPortData > music_in_portlist_;
std::map< std::string, MusicEventHandler > music_in_portmap_;
 */

namespace nest
{

class MUSICManager : public ManagerInterface
{
public:
  virtual void initialize(); // called from meta-manager to construct
  virtual void finalize();   // called from meta-manger to reinit

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  MUSICManager();

  void init_music( int* argc, char** argv[] );

  /**
   * Enter the runtime mode. This must be done before simulating. After having
   * entered runtime mode ports cannot be published anymore.
   * \param h_min_delay is the length of a time slice, after which
   * communication should take place.
   */
  void enter_runtime( double h_min_delay );

  /**
   * Advance the time of music by 1 simulation step.
   */
  void advance_music_time();

  void music_finalize(); // called from MPIManager::mpi_finalize

#ifdef HAVE_MUSIC
  MPI::Intracomm communicator();

  MUSIC::Setup* get_music_setup();
  MUSIC::Runtime* get_music_runtime();

  /**
   * Register a MUSIC input port (portname) with the port list.
   * This will increment the counter of the respective entry in the
   * music_in_portlist.
   *
   * The argument pristine should be set to true when a model
   * registers the initial port name. This typically happens when the
   * copy constructor of the model registers a port, as in
   * models/music_event_in_proxy.cpp. Setting pristine = true causes
   * the port to be also added to pristine_music_in_portlist.  See
   * also comment above Network::pristine_music_in_portlist_.
   */
  void register_music_in_port( std::string portname, bool pristine = false );

  /**
   * Unregister a MUSIC input port (portname) from the port list.
   * This will decrement the counter of the respective entry in the
   * music_in_portlist and remove the entry if the counter is 0
   * after decrementing it.
   */
  void unregister_music_in_port( std::string portname );

  /**
   * Register a node (of type music_input_proxy) with a given MUSIC
   * port (portname) and a specific channel. The proxy will be
   * notified, if a MUSIC event is being received on the respective
   * channel and port.
   */
  void register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp );

  /**
   * Set the acceptable latency (latency) for a music input port (portname).
   */
  void set_music_in_port_acceptable_latency( std::string portname, double latency );
  void set_music_in_port_max_buffered( std::string portname, int maxbuffered );
  /**
   * Data structure to hold variables and parameters associated with a port.
   */
  struct MusicPortData
  {
    MusicPortData( size_t n, double latency, int m )
      : n_input_proxies( n )
      , acceptable_latency( latency )
      , max_buffered( m )
    {
    }
    MusicPortData()
    {
    }
    size_t n_input_proxies; // Counter for number of music_input proxies
                            // connected to this port
    double acceptable_latency;
    int max_buffered;
  };

  /**
   * The mapping between MUSIC input ports identified by portname
   * and the corresponding port variables and parameters.
   * @see register_music_in_port()
   * @see unregister_music_in_port()
   */
  std::map< std::string, MusicPortData > music_in_portlist_;

  /**
   * A copy of music_in_portlist_ at the pristine state.
   *
   * This is used to reset music_in_portlist_ to its pristine state in
   * initialize (a default state). Pristine here refers to the
   * initial state of music_in_portlist_ after the loading of the
   * pristine_models_.
   */
  std::map< std::string, MusicPortData > pristine_music_in_portlist_;

  /**
   * The mapping between MUSIC input ports identified by portname
   * and the corresponding MUSIC event handler.
   */
  std::map< std::string, MusicEventHandler > music_in_portmap_;

  /**
   * Publish all MUSIC input ports that were registered using
   * Network::register_music_event_in_proxy().
   */
  void publish_music_in_ports_();

  /**
   * Call update() for each of the registered MUSIC event handlers
   * to deliver all queued events to the target music_in_proxies.
   */
  void update_music_event_handlers( Time const&, const long, const long );
#endif

private:
#ifdef HAVE_MUSIC
  MUSIC::Setup* music_setup;     //!< pointer to a MUSIC setup object
  MUSIC::Runtime* music_runtime; //!< pointer to a MUSIC runtime object
#endif
};
}


#endif /* MUSIC_MANAGER_H */
