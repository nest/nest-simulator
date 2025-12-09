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

#ifndef CONNECTOR_BASE_IMPL_H
#define CONNECTOR_BASE_IMPL_H

#include "connector_base.h"

#include <algorithm>
#include <cassert>

#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"
#include "nest_names.h"
#include "node.h"
#include "sort.h"
#include "spikecounter.h"
#include "weight_recorder.h"

namespace nest
{

/**
 * Homogeneous connector, contains synapses of one particular type (syn_id_).
 */
template < typename ConnectionT >
Connector< ConnectionT >::Connector( const synindex syn_id )
  : syn_id_( syn_id )
{
}

template < typename ConnectionT >
Connector< ConnectionT >::~Connector()
{
  C_.clear();
}

template < typename ConnectionT >
synindex
Connector< ConnectionT >::get_syn_id() const
{
  return syn_id_;
}

template < typename ConnectionT >
size_t
Connector< ConnectionT >::size() const
{
  return C_.size();
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_synapse_status( const size_t tid, const size_t lcid, DictionaryDatum& dict ) const
{
  assert( lcid < C_.size() );

  C_[ lcid ].get_status( dict );

  // get target node ID here, where tid is available
  // necessary for hpc synapses using TargetIdentifierIndex
  def< long >( dict, names::target, C_[ lcid ].get_target( tid )->get_node_id() );
}

template < typename ConnectionT >
void
Connector< ConnectionT >::set_synapse_status( const size_t lcid, const DictionaryDatum& dict, ConnectorModel& cm )
{
  assert( lcid < C_.size() );

  C_[ lcid ].set_status( dict, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
}

template < typename ConnectionT >
void
Connector< ConnectionT >::push_back( const ConnectionT& c )
{
  C_.push_back( c );
}

template < typename ConnectionT >
void
Connector< ConnectionT >::push_back( ConnectionT&& c )
{
  C_.push_back( std::move( c ) );
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_connection( const size_t source_node_id,
  const size_t target_node_id,
  const size_t tid,
  const size_t lcid,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( not C_[ lcid ].is_disabled() )
  {
    if ( synapse_label == UNLABELED_CONNECTION or C_[ lcid ].get_label() == synapse_label )
    {
      const size_t current_target_node_id = C_[ lcid ].get_target( tid )->get_node_id();
      if ( current_target_node_id == target_node_id or target_node_id == 0 )
      {
        conns.push_back(
          ConnectionDatum( ConnectionID( source_node_id, current_target_node_id, tid, syn_id_, lcid ) ) );
      }
    }
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_connection_with_specified_targets( const size_t source_node_id,
  const std::vector< size_t >& target_neuron_node_ids,
  const size_t tid,
  const size_t lcid,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( not C_[ lcid ].is_disabled() )
  {
    if ( synapse_label == UNLABELED_CONNECTION or C_[ lcid ].get_label() == synapse_label )
    {
      const size_t current_target_node_id = C_[ lcid ].get_target( tid )->get_node_id();
      if ( std::find( target_neuron_node_ids.begin(), target_neuron_node_ids.end(), current_target_node_id )
        != target_neuron_node_ids.end() )
      {
        conns.push_back(
          ConnectionDatum( ConnectionID( source_node_id, current_target_node_id, tid, syn_id_, lcid ) ) );
      }
    }
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_all_connections( const size_t source_node_id,
  const size_t target_node_id,
  const size_t tid,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
  {
    get_connection( source_node_id, target_node_id, tid, lcid, synapse_label, conns );
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_source_lcids( const size_t tid,
  const size_t target_node_id,
  std::vector< size_t >& source_lcids ) const
{
  for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
  {
    const size_t current_target_node_id = C_[ lcid ].get_target( tid )->get_node_id();
    if ( current_target_node_id == target_node_id and not C_[ lcid ].is_disabled() )
    {
      source_lcids.push_back( lcid );
    }
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::get_target_node_ids( const size_t tid,
  const size_t start_lcid,
  const std::string& post_synaptic_element,
  std::vector< size_t >& target_node_ids ) const
{
  size_t lcid = start_lcid;
  while ( true )
  {
    if ( C_[ lcid ].get_target( tid )->get_synaptic_elements( post_synaptic_element ) != 0.0
      and not C_[ lcid ].is_disabled() )
    {
      target_node_ids.push_back( C_[ lcid ].get_target( tid )->get_node_id() );
    }

    if ( not C_[ lcid ].source_has_more_targets() )
    {
      break;
    }

    ++lcid;
  }
}

template < typename ConnectionT >
size_t
Connector< ConnectionT >::get_target_node_id( const size_t tid, const unsigned int lcid ) const
{
  return C_[ lcid ].get_target( tid )->get_node_id();
}

template < typename ConnectionT >
void
Connector< ConnectionT >::send_to_all( const size_t tid, const std::vector< ConnectorModel* >& cm, Event& e )
{
  auto const& cp = static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties();

  for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
  {
    e.set_port( lcid );
    assert( not C_[ lcid ].is_disabled() );
    C_[ lcid ].send( e, tid, cp );
  }
}

template < typename ConnectionT >
size_t
Connector< ConnectionT >::send( const size_t tid,
  const size_t lcid,
  const std::vector< ConnectorModel* >& cm,
  Event& e )
{
  typename ConnectionT::CommonPropertiesType const& cp =
    static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties();

  size_t lcid_offset = 0;

  while ( true )
  {
    assert( lcid + lcid_offset < C_.size() );
    ConnectionT& conn = C_[ lcid + lcid_offset ];

    e.set_port( lcid + lcid_offset );
    if ( not conn.is_disabled() )
    {
      // Some synapses, e.g., bernoulli_synapse, may not send an event after all
      const bool event_sent = conn.send( e, tid, cp );
      if ( event_sent )
      {
        send_weight_event( tid, lcid + lcid_offset, e, cp );
      }
    }
    if ( not conn.source_has_more_targets() )
    {
      break;
    }
    ++lcid_offset;
  }

  return 1 + lcid_offset; // event was delivered to at least one target
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
    prepare_weight_recorder_event( wr_e, tid, syn_id_, lcid, e, cp );
    wr_e();
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::trigger_update_weight( const long vt_node_id,
  const size_t tid,
  const std::vector< spikecounter >& dopa_spikes,
  const double t_trig,
  const std::vector< ConnectorModel* >& cm )
{
  for ( size_t i = 0; i < C_.size(); ++i )
  {
    if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties().get_vt_node_id()
      == vt_node_id )
    {
      C_[ i ].trigger_update_weight( tid,
        dopa_spikes,
        t_trig,
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties() );
    }
  }
}

template < typename ConnectionT >
void
Connector< ConnectionT >::sort_connections( BlockVector< Source >& sources )
{
  nest::sort( sources, C_ );
}

template < typename ConnectionT >
void
Connector< ConnectionT >::set_source_has_more_targets( const size_t lcid, const bool has_more_targets )
{
  C_[ lcid ].set_source_has_more_targets( has_more_targets );
}

template < typename ConnectionT >
size_t
Connector< ConnectionT >::find_first_target( const size_t tid,
  const size_t start_lcid,
  const size_t target_node_id ) const
{
  assert( kernel::manager< ConnectionManager >.use_compressed_spikes() );

  size_t lcid = start_lcid;
  while ( true )
  {
    if ( C_[ lcid ].get_target( tid )->get_node_id() == target_node_id and not C_[ lcid ].is_disabled() )
    {
      return lcid;
    }

    if ( not C_[ lcid ].source_has_more_targets() )
    {
      return invalid_index;
    }

    ++lcid;
  }
}

template < typename ConnectionT >
size_t
Connector< ConnectionT >::find_enabled_connection( const size_t tid,
  const size_t syn_id,
  const size_t source_node_id,
  const size_t target_node_id,
  const SourceTable& source_table ) const
{
  for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
  {
    if ( source_table.get_node_id( tid, syn_id, lcid ) == source_node_id
      and C_[ lcid ].get_target( tid )->get_node_id() == target_node_id and not C_[ lcid ].is_disabled() )
    {
      return lcid;
    }
  }

  return invalid_index;
}

template < typename ConnectionT >
void
Connector< ConnectionT >::disable_connection( const size_t lcid )
{
  assert( not C_[ lcid ].is_disabled() );
  C_[ lcid ].disable();
}

template < typename ConnectionT >
void
Connector< ConnectionT >::remove_disabled_connections( const size_t first_disabled_index )
{
  assert( C_[ first_disabled_index ].is_disabled() );
  C_.erase( C_.begin() + first_disabled_index, C_.end() );
}

} // namespace nest

#endif
