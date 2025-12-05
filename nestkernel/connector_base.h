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
#include <fstream>
#include <vector>

// Includes from models:
#include "weight_recorder.h"

#ifdef HAVE_SIONLIB
#include <sion.h>
#endif

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection_label.h"
#include "event.h"
#include "node.h"
#include "source.h"
#include "source_table.h"
#include "spikecounter.h"

#include "block_vector.h"


namespace nest
{

class ConnectorModel;
template < typename ConnectionT >
class GenericConnectorModel;

/**
 * Base class to allow storing Connectors for different synapse types
 * in vectors. We define the interface here to avoid casting.
 *
 * @note If any member functions need to do something special for a given connection type,
 * declare specializations in the corresponding header file and define them in the corresponding
 * source file. For an example, see `eprop_synapse_bsshslm_2020`.
 */
class ConnectorBase
{
public:
  // Destructor needs to be declared virtual to avoid undefined
  // behavior, avoid possible memory leak and needs to be defined to
  // avoid linker error, see, e.g., Meyers, S. (2005) p40ff
  virtual ~ConnectorBase() {};

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
  virtual void get_synapse_status( const size_t tid, const size_t lcid, DictionaryDatum& dict ) const = 0;

  /**
   * Set status of the connection at position lcid according to the
   * dictionary dict.
   */
  virtual void set_synapse_status( const size_t lcid, const DictionaryDatum& dict, ConnectorModel& cm ) = 0;

  /**
   * Add ConnectionID with given source_node_id and lcid to conns. If
   * target_node_id is given, only add connection if target_node_id matches
   * the node_id of the target of the connection.
   */
  virtual void get_connection( const size_t source_node_id,
    const size_t target_node_id,
    const size_t tid,
    const size_t lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * Add ConnectionID with given source_node_id and lcid to conns. If
   * target_neuron_node_ids is given, only add connection if
   * target_neuron_node_ids contains the node ID of the target of the connection.
   */
  virtual void get_connection_with_specified_targets( const size_t source_node_id,
    const std::vector< size_t >& target_neuron_node_ids,
    const size_t tid,
    const size_t lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * Add ConnectionIDs with given source_node_id to conns, looping over
   * all lcids. If target_node_id is given, only add connection if
   * target_node_id matches the node ID of the target of the connection.
   */
  virtual void get_all_connections( const size_t source_node_id,
    const size_t target_node_id,
    const size_t tid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /**
   * For a given target_node_id add lcids of all connections with matching
   * node ID of target to source_lcids.
   */
  virtual void
  get_source_lcids( const size_t tid, const size_t target_node_id, std::vector< size_t >& source_lcids ) const = 0;

  /**
   * For a given start_lcid add node IDs of all targets that belong to the
   * same source to target_node_ids.
   */
  virtual void get_target_node_ids( const size_t tid,
    const size_t start_lcid,
    const std::string& post_synaptic_element,
    std::vector< size_t >& target_node_ids ) const = 0;

  /**
   * For a given lcid return the node ID of the target of the connection.
   */
  virtual size_t get_target_node_id( const size_t tid, const unsigned int lcid ) const = 0;

  /**
   * Send the event e to all connections of this Connector.
   */
  virtual void send_to_all( const size_t tid, const std::vector< ConnectorModel* >& cm, Event& e ) = 0;

  /**
   * Send the event e to the connection at position lcid. Return bool
   * indicating whether the following connection belongs to the same
   * source.
   */
  virtual size_t send( const size_t tid, const size_t lcid, const std::vector< ConnectorModel* >& cm, Event& e ) = 0;

  virtual void
  send_weight_event( const size_t tid, const unsigned int lcid, Event& e, const CommonSynapseProperties& cp ) = 0;

  /**
   * Update weights of dopamine modulated STDP connections.
   */
  virtual void trigger_update_weight( const long vt_node_id,
    const size_t tid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  /**
   * Sort connections according to source node IDs.
   */
  virtual void sort_connections( BlockVector< Source >& ) = 0;

  /**
   * Set a flag in the connection indicating whether the following
   * connection belongs to the same source.
   */
  virtual void set_source_has_more_targets( const size_t lcid, const bool has_more_targets ) = 0;

  /**
   * Return lcid of the first connection after start_lcid (inclusive)
   * where the node_id of the target matches target_node_id. If there are no matches,
   * the function returns invalid_index.
   */
  virtual size_t find_first_target( const size_t tid, const size_t start_lcid, const size_t target_node_id ) const = 0;

  /**
   * Return lcid of first connection matching source and target node id and that
   * is not disabled.
   *
   * Intended for use with unsorted (uncompressed) connections.
   */
  virtual size_t find_enabled_connection( const size_t tid,
    const size_t syn_id,
    const size_t source_node_id,
    const size_t target_node_id,
    const SourceTable& source_table ) const = 0;

  /**
   * Disable the transfer of events through the connection at position
   * lcid.
   */
  virtual void disable_connection( const size_t lcid ) = 0;

  /**
   * Remove disabled connections from the connector.
   */
  virtual void remove_disabled_connections( const size_t first_disabled_index ) = 0;

protected:
  static void prepare_weight_recorder_event( WeightRecorderEvent& wr_e,
    const size_t tid,
    const synindex syn_id,
    const unsigned int lcid,
    const Event& e,
    const CommonSynapseProperties& cp );
};

// --- Template connector (declarations only) ---

template < typename ConnectionT >
class Connector : public ConnectorBase
{
  BlockVector< ConnectionT > C_;
  const synindex syn_id_;

public:
  explicit Connector( synindex syn_id );
  ~Connector() override;

  synindex get_syn_id() const override;
  size_t size() const override;

  void get_synapse_status( size_t tid, size_t lcid, DictionaryDatum& dict ) const override;
  void set_synapse_status( size_t lcid, const DictionaryDatum& dict, ConnectorModel& cm ) override;

  void push_back( const ConnectionT& c );
  void push_back( ConnectionT&& c );

  void get_connection( size_t source_node_id,
    size_t target_node_id,
    size_t tid,
    size_t lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const override;

  void get_connection_with_specified_targets( size_t source_node_id,
    const std::vector< size_t >& target_neuron_node_ids,
    size_t tid,
    size_t lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const override;

  void get_all_connections( size_t source_node_id,
    size_t target_node_id,
    size_t tid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const override;

  void get_source_lcids( size_t tid, size_t target_node_id, std::vector< size_t >& source_lcids ) const override;

  void get_target_node_ids( size_t tid,
    size_t start_lcid,
    const std::string& post_synaptic_element,
    std::vector< size_t >& target_node_ids ) const override;

  size_t get_target_node_id( size_t tid, unsigned int lcid ) const override;

  void send_to_all( size_t tid, const std::vector< ConnectorModel* >& cm, Event& e ) override;
  size_t send( size_t tid, size_t lcid, const std::vector< ConnectorModel* >& cm, Event& e ) override;

  void send_weight_event( size_t tid, unsigned int lcid, Event& e, const CommonSynapseProperties& cp ) override;

  void trigger_update_weight( long vt_node_id,
    size_t tid,
    const std::vector< spikecounter >& dopa_spikes,
    double t_trig,
    const std::vector< ConnectorModel* >& cm ) override;

  void sort_connections( BlockVector< Source >& sources ) override;
  void set_source_has_more_targets( size_t lcid, bool has_more_targets ) override;

  size_t find_first_target( size_t tid, size_t start_lcid, size_t target_node_id ) const override;

  size_t find_enabled_connection( const size_t tid,
    const size_t syn_id,
    const size_t source_node_id,
    const size_t target_node_id,
    const SourceTable& source_table ) const override;

  void disable_connection( size_t lcid ) override;
  void remove_disabled_connections( size_t first_disabled_index ) override;
};

} // of namespace nest

#endif
