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

#include "network.h"

#include "compose.hpp"
#include "dictstack.h"
#include "interpret.h"
#include "nestmodule.h"
#include "subnet.h"
#include "mpi_manager.h"
#include "mpi_manager_impl.h"

#include "kernel_manager.h"

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
  interpreter_.def( "modeldict", kernel().model_manager.get_modeldict()  );
  interpreter_.def( "synapsedict", kernel().model_manager.get_synapsedict()  );
  interpreter_.def( "connruledict", kernel().connection_builder_manager.get_connruledict() );
}

Network::~Network()
{
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


    def< long >( d, "send_buffer_size", kernel().mpi_manager.get_send_buffer_size() );
    def< long >( d, "receive_buffer_size", kernel().mpi_manager.get_recv_buffer_size() );

  }
  return d;
}


} // end of namespace
