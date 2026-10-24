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

// C++ includes:
#include <typeinfo>

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from models:
#include "weight_recorder.h"

#ifndef CONNECTOR_BASE_IMPL_H
#define CONNECTOR_BASE_IMPL_H

namespace nest
{

template < typename ConnectionT >
bool
Connector< ConnectionT >::send_direct_( const size_t tid,
  const size_t lcid,
  const typename ConnectionT::CommonPropertiesType& cp,
  Event& e,
  size_t& n_targets )
{
  EventDeliveryManager& edm = kernel().event_delivery_manager;
  if ( not edm.get_direct_spike_delivery() or cp.get_weight_recorder() != nullptr or typeid( e ) != typeid( SpikeEvent )
    or static_cast< SpikeEvent& >( e ).get_multiplicity() != 1 )
  {
    return false;
  }

  direct_table_.validate( edm.get_direct_delivery_generation() );
  const long* const moduli = edm.get_moduli_data();
  // as SpikeEvent::get_rel_delivery_steps(), without the delay
  const long rel0 = e.get_stamp().get_steps() - 1 - kernel().simulation_manager.get_slice_origin().get_steps();

  size_t lcid_offset = 0;
  while ( true )
  {
    assert( lcid + lcid_offset < C_.size() );
    ConnectionT& conn = C_[ lcid + lcid_offset ];
    if ( not conn.is_disabled() )
    {
      const DirectSpikeInput& in =
        direct_table_.get( conn.get_target_index(), [ & ]() { return conn.get_target( tid ); } );
      if ( in.slot0 != nullptr )
      {
        const double w = conn.get_weight();
        in.slot0[ in.stride * moduli[ rel0 + conn.get_delay_steps() ]
          + ( w > 0 ? in.excitatory_channel : in.inhibitory_channel ) ] += w;
      }
      else
      {
        e.set_port( lcid + lcid_offset );
        conn.send( e, tid, cp );
      }
    }
    if ( not conn.source_has_more_targets() )
    {
      break;
    }
    ++lcid_offset;
  }
  n_targets = 1 + lcid_offset;
  return true;
}

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

}  // of namespace nest

#endif
