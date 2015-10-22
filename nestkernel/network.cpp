/*
 *  network.cpp
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

#include "instance.h"
#include "network.h"
#include "genericmodel.h"
#include "subnet.h"
#include "sibling_container.h"
#include "interpret.h"
#include "dict.h"
#include "dictstack.h"
#include "integerdatum.h"
#include "booldatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "tokenutils.h"
#include "tokenarray.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include "processes.h"
#include "nestmodule.h"
#include "sibling_container.h"
#include "communicator_impl.h"
#include "random_datums.h"

#include "kernel_manager.h"
#include "vp_manager_impl.h"
#include "connection_builder_manager_impl.h"

#include "nest_timeconverter.h"

#include <cmath>
#include <sys/time.h>
#include <set>
#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef N_DEBUG
#undef N_DEBUG
#endif

extern int SLIsignalflag;

namespace nest
{

Network* Network::network_instance_ = NULL;
bool Network::created_network_instance_ = false;

void
Network::create_network( SLIInterpreter& i )
{
#pragma omp critical( create_network )
  {
    if ( !created_network_instance_ )
    {
      network_instance_ = new Network( i );
      assert( network_instance_ );
      created_network_instance_ = true;
    }
  }
}

void
Network::destroy_network()
{
  delete network_instance_;
}


Network::Network( SLIInterpreter& i )
  : interpreter_( i )
  , dict_miss_is_error_( true )
{
  // the subsequent function-calls need a
  // network instance, hence the instance
  // need to be set here
  // e.g. constructor of GenericModel, ConnectionManager -> get_num_threads()
  //
  network_instance_ = this;
  created_network_instance_ = true;

  kernel().initialize();
  
  // this can make problems with reference counting, if 
  // the intepreter decides cleans up memory before NEST is ready
  interpreter_.def( "modeldict", new DictionaryDatum( kernel().model_manager.get_modeldict() ) );
  interpreter_.def( "synapsedict", new DictionaryDatum( kernel().model_manager.get_synapsedict() ) );
  interpreter_.def( "connruledict", new DictionaryDatum( kernel().connection_builder_manager.get_connruledict() ) );

  init_();
}

Network::~Network()
{
}

void
Network::init_()
{
  /*
   * We initialise the network with one subnet that is the root of the tree.
   * Note that we MUST NOT call add_node(), since it expects a properly
   * initialized network.
   */
#ifdef HAVE_MUSIC
  music_in_portlist_.clear();
#endif
}

void
Network::reset_kernel()
{
  /*
   * TODO: reset() below mixes both destruction of old nodes and
   * configuration of the fresh kernel. set_num_rec_processes() chokes
   * on this, as it expects a kernel without nodes. We now suppress that
   * test manually. Ideally, though, we should split reset() into one
   * part deleting all the old stuff, then perform settings for the
   * fresh kernel, then do remaining initialization.
   */

  kernel().reset();
  
  init_();
}

void
Network::memory_info()
{
  // TODO We decided to remove the memory_info function, so for now we
  // just comment it out. If we want to keep it, it has to go to the
  // ModelManager prints the models unsorted

//
//  std::cout.setf( std::ios::left );
//  std::vector< index > idx( kernel().model_manager.get_num_node_models() );
//
//
//  for ( index i = 0; i < kernel().model_manager.get_num_node_models(); ++i )
//    idx[ i ] = i;
//
//  std::sort( idx.begin(), idx.end(), ModelComp( models_ ) );
//
//  std::string sep( "--------------------------------------------------" );
//
//  std::cout << sep << std::endl;
//  std::cout << std::setw( 25 ) << "Name" << std::setw( 13 ) << "Capacity" << std::setw( 13 )
//            << "Available" << std::endl;
//  std::cout << sep << std::endl;
//
//  for ( index i = 0; i < kernel().model_manager.get_num_node_models(); ++i )
//  {
//    Model* mod = models_[ idx[ i ] ];
//    if ( mod->mem_capacity() != 0 )
//      std::cout << std::setw( 25 ) << mod->get_name() << std::setw( 13 )
//                << mod->mem_capacity() * mod->get_element_size() << std::setw( 13 )
//                << mod->mem_available() * mod->get_element_size() << std::endl;
//  }
//
//  std::cout << sep << std::endl;
//  std::cout.unsetf( std::ios::left );
}

void
Network::set_status( index gid, const DictionaryDatum& d )
{
  // we first handle normal nodes, except the root (GID 0)
  if ( gid > 0 )
  {
    kernel().node_manager.set_status(gid, d);
    return;
  }

  /* Code below is executed only for the root node, gid == 0

     In this case, we must
     - set scheduler properties
     - set properties for the compound representing each thread

     The main difficulty here is to handle the access control for
     dictionary items, since the dictionary is read in several places.

     We proceed as follows:
     - clear access flags
     - set scheduler properties; this must be first, anyways
     - at this point, all non-compound property flags are marked accessed
     - loop over all per-thread compounds
     - the first per-thread compound will flag all compound properties as read
     - now, all dictionary entries must be flagged as accessed, otherwise the
       dictionary contains unknown entries. Thus, kernel().node_manager.set_status_single_node
       will not throw an exception
     - since all items in the root node are of type Compound, all read the same
       properties and we can leave the access flags set
   */
  d->clear_access_flags();

  // former scheduler_.set_status( d ); start
  // careful, this may invalidate all node pointers!
  updateValue< bool >( d, "dict_miss_is_error", dict_miss_is_error_ );
  kernel().set_status( d );


  // former scheduler_.set_status( d ); end

}

DictionaryDatum
Network::get_status( index idx )
{
  assert( kernel().is_initialized() );

  Node* target = kernel().node_manager.get_node( idx );
  assert( target != 0 );

  DictionaryDatum d = target->get_status_base();

  if ( target == kernel().node_manager.get_root() )
  {
    // former scheduler_.get_status( d ) start
    kernel().get_status( d );


    def< long >( d, "send_buffer_size", Communicator::get_send_buffer_size() );
    def< long >( d, "receive_buffer_size", Communicator::get_recv_buffer_size() );
    // former scheduler_.get_status( d ) end

    ( *d )[ "dict_miss_is_error" ] = dict_miss_is_error_;

  }
  return d;
}


/**
 * This function is not thread save and has to be called inside a omp critical
 * region, e.g. sli_neuron.
 */
int
Network::execute_sli_protected( DictionaryDatum state, Name cmd )
{
  SLIInterpreter& i = interpreter_;

  i.DStack->push( state ); // push state dictionary as top namespace
  size_t exitlevel = i.EStack.load();
  i.EStack.push( new NameDatum( cmd ) );
  int result = i.execute_( exitlevel );
  i.DStack->pop(); // pop neuron's namespace

  if ( state->known( "error" ) )
  {
    assert( state->known( names::global_id ) );
    index g_id = ( *state )[ names::global_id ];
    std::string model = getValue< std::string >( ( *state )[ names::model ] );
    std::string msg = String::compose( "Error in %1 with global id %2.", model, g_id );

    LOG( M_ERROR, cmd.toString().c_str(), msg.c_str() );
    LOG( M_ERROR, "execute_sli_protected", "Terminating." );

    kernel().simulation_manager.terminate();
  }

  return result;
}

#ifdef HAVE_MUSIC
void
Network::register_music_in_port( std::string portname )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
    music_in_portlist_[ portname ] = MusicPortData( 1, 0.0, -1 );
  else
    music_in_portlist_[ portname ].n_input_proxies++;
}

void
Network::unregister_music_in_port( std::string portname )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
    throw MUSICPortUnknown( portname );
  else
    music_in_portlist_[ portname ].n_input_proxies--;

  if ( music_in_portlist_[ portname ].n_input_proxies == 0 )
    music_in_portlist_.erase( it );
}

void
Network::register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp )
{
  std::map< std::string, MusicEventHandler >::iterator it;
  it = music_in_portmap_.find( portname );
  if ( it == music_in_portmap_.end() )
  {
    MusicEventHandler tmp( portname,
      music_in_portlist_[ portname ].acceptable_latency,
      music_in_portlist_[ portname ].max_buffered );
    tmp.register_channel( channel, mp );
    music_in_portmap_[ portname ] = tmp;
  }
  else
    it->second.register_channel( channel, mp );
}

void
Network::set_music_in_port_acceptable_latency( std::string portname, double latency )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
    throw MUSICPortUnknown( portname );
  else
    music_in_portlist_[ portname ].acceptable_latency = latency;
}

void
Network::set_music_in_port_max_buffered( std::string portname, int_t maxbuffered )
{
  std::map< std::string, MusicPortData >::iterator it;
  it = music_in_portlist_.find( portname );
  if ( it == music_in_portlist_.end() )
    throw MUSICPortUnknown( portname );
  else
    music_in_portlist_[ portname ].max_buffered = maxbuffered;
}

void
Network::publish_music_in_ports_()
{
  std::map< std::string, MusicEventHandler >::iterator it;
  for ( it = music_in_portmap_.begin(); it != music_in_portmap_.end(); ++it )
    it->second.publish_port();
}

void
Network::update_music_event_handlers_( Time const& origin, const long_t from, const long_t to )
{
  std::map< std::string, MusicEventHandler >::iterator it;
  for ( it = music_in_portmap_.begin(); it != music_in_portmap_.end(); ++it )
    it->second.update( origin, from, to );
}
#endif



bool
ModelComp::operator()( int a, int b )
{
  return kernel().model_manager.get_model( a )->get_name() < kernel().model_manager.get_model( b )->get_name();
}

} // end of namespace
