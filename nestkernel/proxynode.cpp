/*
 *  proxynode.cpp
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
#include "dictutils.h"
#include "proxynode.h"
#include "connection.h"

namespace nest
{

proxynode::proxynode( index gid, index parent_gid, index model_id, index vp )
  : Node()
{
  set_gid_( gid );
  Subnet* parent = dynamic_cast< Subnet* >( network()->get_node( parent_gid ) );
  assert( parent );
  set_parent_( parent );
  set_model_id( model_id );
  set_vp( vp );
  set_frozen_( true );
}

port
proxynode::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  return network()
    ->get_model( get_model_id() )
    ->send_test_event( target, receptor_type, syn_id, dummy_target );
}

void
proxynode::sends_secondary_event( GapJunctionEvent& ge )
{
  network()->get_model( get_model_id() )->sends_secondary_event( ge );
}

/**
  * @returns type of signal this node produces
  * used in check_connection to only connect neurons which send / receive compatible information
  * delgates to underlying model
  */
nest::SignalType
proxynode::sends_signal() const
{
  return network()->get_model( get_model_id() )->sends_signal();
}


} // namespace
