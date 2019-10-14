/*
 *  connector_base_impl.h
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

#include "connector_base.h"

// Includes from nestkernel:
#include "kernel_manager.h"

#ifndef CONNECTOR_BASE_IMPL_H
#define CONNECTOR_BASE_IMPL_H

namespace nest
{

template < typename ConnectionT >
void
Connector< ConnectionT >::send_weight_event( const thread tid,
  const unsigned int lcid,
  Event& e,
  const CommonSynapseProperties& cp )
{
  if ( cp.get_wr_gid() != 0 )
  {
    // Create new event to record the weight and copy relevant content.
    WeightRecorderEvent wr_e;
    wr_e.set_port( e.get_port() );
    wr_e.set_rport( e.get_rport() );
    wr_e.set_stamp( e.get_stamp() );
    wr_e.set_sender( e.get_sender() );
    wr_e.set_sender_gid( kernel().connection_manager.get_source_gid( tid, syn_id_, lcid ) );
    wr_e.set_weight( e.get_weight() );
    wr_e.set_delay_steps( e.get_delay_steps() );
    // Set weight_recorder as receiver
    index wr_gid = cp.get_wr_gid();
    Node* wr_node = kernel().node_manager.get_node_or_proxy( wr_gid, tid );
    wr_e.set_receiver( *wr_node );
    // Put the gid of the postsynaptic node as receiver gid
    wr_e.set_receiver_gid( e.get_receiver().get_gid() );
    wr_e();
  }
}

} // of namespace nest

#endif
