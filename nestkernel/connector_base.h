/*
 *  connector_base.h
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

#ifndef CONNECTOR_BASE_H
#define CONNECTOR_BASE_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <cstdlib>
#include <vector>

// Includes from libnestutil:
#include "compose.hpp"
#include "sort.h"
#include "vector_util.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection_label.h"
#include "connector_model.h"
#include "event.h"
#include "nest_datums.h"
#include "nest_names.h"
#include "node.h"
#include "source.h"
#include "spikecounter.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"

namespace nest
{

/**
 * Base class to allow storing Connectors for different synapse types
 * in vectors. We define the interface here to avoid casting.
 */
class ConnectorBase
{

public:
  // Destructor needs to be declared virtual to avoid undefined
  // behavior, avoid possible memory leak and needs to be defined to
  // avoid linker error, see, e.g., Meyers, S. (2005) p40ff
  virtual ~ConnectorBase(){};

  /**
   * Return syn_id_ of the synapse type of this Connector (index in
   * list of synapse prototypes).
   */
  virtual synindex get_syn_id() const = 0;

  /**
   * Return the number of connections in this Connector.
   */
  virtual size_t size() const = 0;

  /**
   * Write status of the connection at position lcid to the dictionary
   * dict.
   */
  virtual void get_synapse_status( const thread tid, const index lcid, DictionaryDatum& dict ) const = 0;

  /**
   * Set status of the connection at position lcid according to the
   * dictionary dict.
   */
  virtual void set_synapse_status( const index lcid, const DictionaryDatum& dict, ConnectorModel& cm ) = 0;

  /**
   * Add ConnectionID with given source_gid and lcid to conns. If
   * target_gid is given, only add connection if target_gid matches
   * the gid of the target of the connection.
   */
  virtual void get_connection( const index source_gid,
    const index target_gid,
    const thread tid,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * Add ConnectionID with given source_gid and lcid to conns. If
   * target_neuron_gids is given, only add connection if
   * target_neuron_gids contains the gid of the target of the connection.
   */
  virtual void get_connection_with_specified_targets( const index source_gid,
    const std::vector< size_t >& target_neuron_gids,
    const thread tid,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * Add ConnectionIDs with given source_gid to conns, looping over
   * all lcids. If target_gid is given, only add connection if
   * target_gid matches the gid of the target of the connection.
   */
  virtual void get_all_connections( const index source_gid,
    const index target_gid,
    const thread tid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * For a given target_gid add lcids of all connections with matching
   * gid of target to source_lcids.
   */
  virtual void
  get_source_lcids( const thread tid, const index target_gid, std::vector< index >& source_lcids ) const = 0;

  /**
   * For a given start_lcid add gids of all targets that belong to the
   * same source to target_gids.
   */
  virtual void get_target_gids( const thread tid,
    const index start_lcid,
    const std::string& post_synaptic_element,
    std::vector< index >& target_gids ) const = 0;

  /**
   * For a given lcid return the gid of the target of the connection.
   */
  virtual index get_target_gid( const thread tid, const unsigned int lcid ) const = 0;

  /**
   * Send the event e to all connections of this Connector.
   */
  virtual void send_to_all( const thread tid, const std::vector< ConnectorModel* >& cm, Event& e ) = 0;

  /**
   * Send the event e to the connection at position lcid. Return bool
   * indicating whether the following connection belongs to the same
   * source.
   */
  virtual index send( const thread tid, const index lcid, const std::vector< ConnectorModel* >& cm, Event& e ) = 0;

  virtual void
  send_weight_event( const thread tid, const unsigned int lcid, Event& e, const CommonSynapseProperties& cp ) = 0;

  /**
   * Update weights of dopamine modulated STDP connections.
   */
  virtual void trigger_update_weight( const long vt_gid,
    const thread tid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  /**
   * Sort connections according to source gids.
   */
  virtual void sort_connections( BlockVector< Source >& ) = 0;

  /**
   * Set a flag in the connection indicating whether the following
   * connection belongs to the same source.
   */
  virtual void set_has_source_subsequent_targets( const index lcid, const bool has_subsequent_targets ) = 0;

  /**
   * Return lcid of the first connection after start_lcid (inclusive)
   * where the gid of the target matches target_gid. If there are no matches,
   * the function returns invalid_index.
   */
  virtual index find_first_target( const thread tid, const index start_lcid, const index target_gid ) const = 0;

  /**
   * Return lcid of first connection where the gid of the target
   * matches target_gid; consider only the connections with lcids
   * given in matching_lcids. If there is no match, the function returns
   * invalid_index.
   */
  virtual index find_matching_target( const thread tid,
    const std::vector< index >& matching_lcids,
    const index target_gid ) const = 0;

  /**
   * Disable the transfer of events through the connection at position
   * lcid.
   */
  virtual void disable_connection( const index lcid ) = 0;

  /**
   * Remove disabled connections from the connector.
   */
  virtual void remove_disabled_connections( const index first_disabled_index ) = 0;
};

/**
 * Homogeneous connector, contains synapses of one particular type (syn_id_).
 */
template < typename ConnectionT >
class Connector : public ConnectorBase
{
private:
  BlockVector< ConnectionT > C_;
  const synindex syn_id_;

public:
  explicit Connector( const synindex syn_id )
    : syn_id_( syn_id )
  {
  }

  ~Connector()
  {
    C_.clear();
  }

  synindex
  get_syn_id() const
  {
    return syn_id_;
  }

  size_t
  size() const
  {
    return C_.size();
  }

  void
  get_synapse_status( const thread tid, const index lcid, DictionaryDatum& dict ) const
  {
    assert( lcid >= 0 and lcid < C_.size() );

    C_[ lcid ].get_status( dict );

    // get target gid here, where tid is available
    // necessary for hpc synapses using TargetIdentifierIndex
    def< long >( dict, names::target, C_[ lcid ].get_target( tid )->get_gid() );
  }

  void
  set_synapse_status( const index lcid, const DictionaryDatum& dict, ConnectorModel& cm )
  {
    assert( lcid < C_.size() );

    C_[ lcid ].set_status( dict, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
  }

  void
  push_back( const ConnectionT& c )
  {
    C_.push_back( c );
  }

  void
  get_connection( const index source_gid,
    const index target_gid,
    const thread tid,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    if ( not C_[ lcid ].is_disabled() )
    {
      if ( synapse_label == UNLABELED_CONNECTION or C_[ lcid ].get_label() == synapse_label )
      {
        const index current_target_gid = C_[ lcid ].get_target( tid )->get_gid();
        if ( current_target_gid == target_gid or target_gid == 0 )
        {
          conns.push_back( ConnectionDatum( ConnectionID( source_gid, current_target_gid, tid, syn_id_, lcid ) ) );
        }
      }
    }
  }

  void
  get_connection_with_specified_targets( const index source_gid,
    const std::vector< size_t >& target_neuron_gids,
    const thread tid,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    if ( not C_[ lcid ].is_disabled() )
    {
      if ( synapse_label == UNLABELED_CONNECTION or C_[ lcid ].get_label() == synapse_label )
      {
        const index current_target_gid = C_[ lcid ].get_target( tid )->get_gid();
        if ( std::find( target_neuron_gids.begin(), target_neuron_gids.end(), current_target_gid )
          != target_neuron_gids.end() )
        {
          conns.push_back( ConnectionDatum( ConnectionID( source_gid, current_target_gid, tid, syn_id_, lcid ) ) );
        }
      }
    }
  }

  void
  get_all_connections( const index source_gid,
    const index target_gid,
    const thread tid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
    {
      get_connection( source_gid, target_gid, tid, lcid, synapse_label, conns );
    }
  }

  void
  get_source_lcids( const thread tid, const index target_gid, std::vector< index >& source_lcids ) const
  {
    for ( index lcid = 0; lcid < C_.size(); ++lcid )
    {
      const index current_target_gid = C_[ lcid ].get_target( tid )->get_gid();
      if ( current_target_gid == target_gid and not C_[ lcid ].is_disabled() )
      {
        source_lcids.push_back( lcid );
      }
    }
  }

  void
  get_target_gids( const thread tid,
    const index start_lcid,
    const std::string& post_synaptic_element,
    std::vector< index >& target_gids ) const
  {
    index lcid = start_lcid;
    while ( true )
    {
      if ( C_[ lcid ].get_target( tid )->get_synaptic_elements( post_synaptic_element ) != 0.0
        and not C_[ lcid ].is_disabled() )
      {
        target_gids.push_back( C_[ lcid ].get_target( tid )->get_gid() );
      }

      if ( not C_[ lcid ].has_source_subsequent_targets() )
      {
        break;
      }

      ++lcid;
    }
  }

  index
  get_target_gid( const thread tid, const unsigned int lcid ) const
  {
    return C_[ lcid ].get_target( tid )->get_gid();
  }

  void
  send_to_all( const thread tid, const std::vector< ConnectorModel* >& cm, Event& e )
  {
    for ( size_t lcid = 0; lcid < C_.size(); ++lcid )
    {
      e.set_port( lcid );
      assert( not C_[ lcid ].is_disabled() );
      C_[ lcid ].send(
        e, tid, static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties() );
    }
  }

  index
  send( const thread tid, const index lcid, const std::vector< ConnectorModel* >& cm, Event& e )
  {
    typename ConnectionT::CommonPropertiesType const& cp =
      static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties();

    index lcid_offset = 0;

    while ( true )
    {
      ConnectionT& conn = C_[ lcid + lcid_offset ];
      const bool is_disabled = conn.is_disabled();
      const bool has_source_subsequent_targets = conn.has_source_subsequent_targets();

      e.set_port( lcid + lcid_offset );
      if ( not is_disabled )
      {
        conn.send( e, tid, cp );
        send_weight_event( tid, lcid + lcid_offset, e, cp );
      }
      if ( not has_source_subsequent_targets )
      {
        break;
      }
      ++lcid_offset;
    }

    return 1 + lcid_offset; // event was delivered to at least one target
  }

  // Implemented in connector_base_impl.h
  void send_weight_event( const thread tid, const unsigned int lcid, Event& e, const CommonSynapseProperties& cp );

  void
  trigger_update_weight( const long vt_gid,
    const thread tid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties().get_vt_gid()
        == vt_gid )
      {
        C_[ i ].trigger_update_weight( tid,
          dopa_spikes,
          t_trig,
          static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties() );
      }
    }
  }

  void
  sort_connections( BlockVector< Source >& sources )
  {
    nest::sort( sources, C_ );
  }

  void
  set_has_source_subsequent_targets( const index lcid, const bool has_subsequent_targets )
  {
    C_[ lcid ].set_has_source_subsequent_targets( has_subsequent_targets );
  }

  index
  find_first_target( const thread tid, const index start_lcid, const index target_gid ) const
  {
    index lcid = start_lcid;
    while ( true )
    {
      if ( C_[ lcid ].get_target( tid )->get_gid() == target_gid and not C_[ lcid ].is_disabled() )
      {
        return lcid;
      }

      if ( not C_[ lcid ].has_source_subsequent_targets() )
      {
        return invalid_index;
      }

      ++lcid;
    }
  }

  index
  find_matching_target( const thread tid, const std::vector< index >& matching_lcids, const index target_gid ) const
  {
    for ( size_t i = 0; i < matching_lcids.size(); ++i )
    {
      if ( C_[ matching_lcids[ i ] ].get_target( tid )->get_gid() == target_gid )
      {
        return matching_lcids[ i ];
      }
    }

    return invalid_index;
  }

  void
  disable_connection( const index lcid )
  {
    assert( not C_[ lcid ].is_disabled() );
    C_[ lcid ].disable();
  }

  void
  remove_disabled_connections( const index first_disabled_index )
  {
    assert( C_[ first_disabled_index ].is_disabled() );
    C_.erase( C_.begin() + first_disabled_index, C_.end() );
  }
};

} // of namespace nest

#endif
