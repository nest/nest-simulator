/*
 *  connector_base.cpp
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

#include "connection_manager.h"

namespace nest
{

void
ConnectorBase::prepare_weight_recorder_event( WeightRecorderEvent& wr_e,
  const size_t tid,
  const synindex syn_id,
  const unsigned int lcid,
  const Event& e,
  const CommonSynapseProperties& cp )
{
  wr_e.set_port( e.get_port() );
  wr_e.set_rport( e.get_rport() );
  wr_e.set_stamp( e.get_stamp() );
  // Sender is not available for SecondaryEvents, and not needed, so we do not set it to avoid undefined behavior.
  // wr_e.set_sender_node_id( kernel::manager< ConnectionManager >.get_source_node_id( tid, syn_id, lcid ) );
  wr_e.set_sender_node_id( kernel::manager< ConnectionManager >.get_source_node_id( tid, syn_id, lcid ) );
  wr_e.set_weight( e.get_weight() );
  wr_e.set_delay_steps( e.get_delay_steps() );
  wr_e.set_receiver( *static_cast< Node* >( cp.get_weight_recorder() ) );
  // Set the node_id of the postsynaptic node as receiver node ID
  wr_e.set_receiver_node_id( e.get_receiver_node_id() );
}

} // namespace nest
