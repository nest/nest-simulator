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

// Includes from models:
#include "weight_recorder.h"

#ifndef CONNECTOR_BASE_IMPL_H
#define CONNECTOR_BASE_IMPL_H

namespace nest
{

template < typename ConnectionT >
void
Connector< ConnectionT >::send_weight_event( const size_t tid,
  const unsigned int lcid,
  Event& e,
  const CommonSynapseProperties& cp )
{
  // If the pointer to the receiver node in the event is invalid,
  // the event was not sent, and a WeightRecorderEvent is therefore not created.
  if ( cp.get_weight_recorder() and e.receiver_is_valid() )
  {
    // Create new event to record the weight and copy relevant content.
    WeightRecorderEvent wr_e;
    wr_e.set_port( e.get_port() );
    wr_e.set_rport( e.get_rport() );
    wr_e.set_stamp( e.get_stamp() );
    // Sender is not available for SecondaryEvents, and not needed, so we do not
    // set it to avoid undefined behavior.
    wr_e.set_sender_node_id( kernel().connection_manager.get_source_node_id( tid, syn_id_, lcid ) );
    wr_e.set_weight( e.get_weight() );
    wr_e.set_delay_steps( e.get_delay_steps() );
    wr_e.set_receiver( *static_cast< Node* >( cp.get_weight_recorder() ) );
    // Set the node_id of the postsynaptic node as receiver node ID
    wr_e.set_receiver_node_id( e.get_receiver_node_id() );
    wr_e();
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::correct_synapse_stdp_ax_delay( const size_t tid,
  const synindex syn_id,
  const size_t lcid,
  const double t_last_pre_spike,
  const double t_spike_critical_interval_end,
  const double weight_revert,
  double& new_weight,
  const double K_plus_revert,
  const double t_post_spike )
{
  typename ConnectionT::CommonPropertiesType const& cp = static_cast< GenericConnectorModel< ConnectionT >* >(
    kernel().model_manager.get_connection_models( tid )[ syn_id ] )
                                                           ->get_common_properties();
  C_[ lcid ].correct_synapse_stdp_ax_delay( tid,
    lcid,
    t_last_pre_spike,
    t_spike_critical_interval_end,
    weight_revert,
    new_weight,
    K_plus_revert,
    t_post_spike,
    cp );
}

} // of namespace nest

#endif
