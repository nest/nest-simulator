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
 * Base clase to allow storing Connectors for different synapse types
 * in vectors. We define the interface here to avoid casting.
 */
class ConnectorBase
{

public:
  unsigned int call_count;

   // destructor needs to be declared virtual to avoid undefined
   // behaviour, avoid possible memory leak and needs to be defined to
   // avoid linker error, see, e.g., Meyers, S. (2005) p40ff
  virtual ~ConnectorBase(){};

  /** Adds status of synapse of type syn_id at position lcid to
   * dictionary d.
   */
  virtual void get_synapse_status( const thread tid,
    const synindex syn_id,
    DictionaryDatum& d,
    const index lcid ) const = 0;

  /** Sets status of synapse of type syn_id at position lcid according
   * to dictionary d.
   */
  virtual void set_synapse_status( const synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& d,
    const index lcid ) = 0;

  /** Returns the number of connections of type syn_id.
   */
  virtual size_t get_num_connections( const synindex syn_id ) const = 0;

  /** Adds ConnectionID with given source and lcid to conns. If
   * target_gid is given, only add connection if target_gid matches
   * with target of connection.
   */
  virtual void get_connection( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /** Adds ConnectionIDs with given source to conns, looping over all
   * lcid. If target_gid is given, only add connection if target_gid
   * matches with target of connection.
   */
  virtual void get_all_connections( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  /** For a given target_gid adds all lcids matching this target to
   * source_lcids.
   */
  virtual void get_source_lcids( const thread tid,
    const index target_gid,
    std::vector< index >& source_lcids ) const = 0;

  /** For a given start lcid, adds all target gids to target_gids
   * that belong to the same source.
   */
  virtual void get_target_gids( const thread tid,
    const index start_lcid,
    std::vector< index >& target_gids,
    const std::string post_synaptic_element) const = 0;

  /** For a given lcid, returns the gid of the corresponding target.
   */
  virtual index get_target_gid( const thread tid,
    const unsigned int lcid ) const = 0;

  /** Sends an event to all connections.
   */
  virtual void send_to_all( Event& e,
    const thread tid,
    const std::vector< ConnectorModel* >& cm ) = 0;

  /** Sends an event to the specified connection, returning whether
   * the subsequent connection belongs to the same source.
   */
  virtual bool send( const thread tid,
    const synindex syn_id,
    const unsigned int lcid,
    Event& e,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void send_weight_event( const thread tid,
    const synindex syn_id,
    const unsigned int lcid,
    Event& e,
    const CommonSynapseProperties& cp ) = 0;

  /** Updates weights of dopamine modulated STDP connections.
   */
  virtual void trigger_update_weight( const long vt_gid,
    const thread tid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  /** Returns syn_id of synapse (index in list of prototypes).
   */
  virtual synindex get_syn_id() const = 0;

  /** Sorts connections be source.
   */
  virtual void
  sort_connections( std::vector< Source >& ) = 0;

  /** Reserves the specified amount of connections.
   */
  virtual void reserve( const size_t ) = 0;

  /** Sets a flag in the connection to signal that the following
   * connection has the same source.
   */
  virtual void
  set_has_source_subsequent_targets( const index lcid,
    const bool subsequent_targets ) = 0;

  /** Returns lcid of first connection with correct target gid,
   * starting at position lcid.
   */
  virtual index
  find_first_target( const thread tid,
    const index start_lcid,
    const index target_gid ) const = 0;

  /** Returns lcid of connection with correct target gid, using lcids
   * in matching_lcids array.
   */
  virtual index
  find_matching_target( const thread tid,
    const std::vector< index >& matching_lcids,
    const index target_gid ) const = 0;

  /** Disables a connection which will not transfer events any more
   * afterwards.
   */
  virtual void
  disable_connection( const index lcid ) = 0;

  /** Removes disabled connections from the connector.
   */
  virtual void
  remove_disabled_connections( const index first_disabled_index ) = 0;

  virtual void
  print_connections( const thread tid ) const = 0;

  /** Returns the number of connections in this Connector.
   */
  virtual size_t size() const = 0;
};

/** Homogeneous connector, contains synapses of one particluar type (syn_id).
 */
template < typename ConnectionT >
class Connector : public ConnectorBase
{
private:
  std::vector< ConnectionT > C_;
  const synindex syn_id_;

public:
  explicit Connector( const synindex syn_id )
    : syn_id_( syn_id )
  {
    call_count = 0;
  }

  ~Connector()
  {
    C_.clear();
  }

  void
  get_synapse_status( const thread tid, const synindex syn_id, DictionaryDatum& d, const index lcid ) const
  {
    assert( syn_id == syn_id_ );
    assert( lcid >= 0 && lcid < C_.size() );
    C_[ lcid ].get_status( d );

    // set target gid here, where tid is available; necessary for hpc
    // synapses using TargetIdentifierIndex
    def< long >( d, names::target, C_[ lcid ].get_target( tid )->get_gid() );
  }

  void
  set_synapse_status( const synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& d,
    const index lcid )
  {
    assert( syn_id == syn_id_ );
    assert( lcid >= 0 && lcid < C_.size() );
    C_[ lcid ].set_status(
      d, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
  }

  size_t
  get_num_connections( const synindex syn_id ) const
  {
    assert( syn_id == syn_id_ );
    return C_.size();
  }

  size_t
  get_num_connections( const index target_gid, const thread tid, const synindex syn_id ) const
  {
    size_t num_connections = 0;
    assert( syn_id == syn_id_ );
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      if ( C_[ i ].get_target( tid )->get_gid() == target_gid )
      {
        ++num_connections;
      }
    }
    return num_connections;
  }

  Connector< ConnectionT >&
  push_back( const ConnectionT& c )
  {
    // use 1.5 growth strategy (see, e.g.,
    // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md)
    if ( C_.size() == C_.capacity() )
    {
      C_.reserve( ( C_.size() * 3 + 1 ) / 2 );
    }
    C_.push_back( c );
    return *this;
  }

  ConnectionT&
  at( const size_t i )
  {
    if ( i >= C_.size() || i < 0 )
    {
      throw std::out_of_range( String::compose(
        "Invalid attempt to access a connection: index %1 out of range.", i ) );
    }
    return C_[ i ];
  }

  void
  get_connection( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const index lcid,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    assert( syn_id_ == syn_id );
    if ( not C_[ lcid ].is_disabled() )
    {
      if ( synapse_label == UNLABELED_CONNECTION
        || C_[ lcid ].get_label() == synapse_label )
      {
        const index current_target_gid = C_[ lcid ].get_target( tid )->get_gid();
        if ( current_target_gid == target_gid || target_gid == 0 )
        {
          conns.push_back( ConnectionDatum(
            ConnectionID( source_gid, current_target_gid, tid, syn_id, lcid ) ) );
        }
      }
    }
  }

  void
  get_all_connections( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    assert( syn_id_ == syn_id );
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      if ( not C_[ i ].is_disabled() )
      {
        const index current_target_gid = C_[ i ].get_target( tid )->get_gid();
        if ( current_target_gid == target_gid || target_gid == 0 )
        {
          if ( synapse_label == UNLABELED_CONNECTION
               || C_[ i ].get_label() == synapse_label )
          {
            conns.push_back( ConnectionDatum(
                               ConnectionID( source_gid, current_target_gid, tid, syn_id, i ) ) );
          }
        }
      }
    }
  }

  void
  get_source_lcids( const thread tid,
    const index target_gid,
    std::vector< index >& source_lcids ) const
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
    std::vector< index >& target_gids,
    const std::string post_synaptic_element ) const
  {
    index lcid = start_lcid;
    while ( true )
    {
      if ( C_[ lcid ].get_target( tid )->get_synaptic_elements(
             post_synaptic_element ) != 0.0 and not C_[ lcid ].is_disabled() )
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
  get_target_gid( const thread tid,
    const unsigned int lcid ) const
  {
    return C_[ lcid ].get_target( tid )->get_gid();
  }

  void
  send_to_all( Event& e, const thread tid, const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      e.set_port( i ); // TODO@5g: does this make sense?
      assert( not C_[ i ].is_disabled() );
      C_[ i ].send( e,
        tid,
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
          ->get_common_properties() );
    }
  }

  bool
  send( const thread tid,
    const synindex syn_id,
    const unsigned int lcid,
    Event& e,
    const std::vector< ConnectorModel* >& cm )
  {
#ifndef DISABLE_TIMING
    ++call_count;
#endif

    e.set_port( lcid ); // TODO@5g: does this make sense?
    if ( not C_[ lcid ].is_disabled() )
    {
      typename ConnectionT::CommonPropertiesType const& cp = static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties();
      C_[ lcid ].send( e, tid, cp );
      send_weight_event( tid, syn_id, lcid, e, cp );
    }
    return C_[ lcid ].has_source_subsequent_targets();
  }

  // implemented in connector_base_impl.h
  void
  send_weight_event( const thread tid,
    const synindex syn_id,
    const unsigned int lcid,
    Event& e,
    const CommonSynapseProperties& cp );

  void
  trigger_update_weight( const long vt_gid,
    const thread tid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
      if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
             ->get_common_properties()
             .get_vt_gid() == vt_gid )
        C_[ i ].trigger_update_weight( tid,
          dopa_spikes,
          t_trig,
          static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
            ->get_common_properties() );
  }

  synindex
  get_syn_id() const
  {
    return syn_id_;
  }

  void
  reserve( const size_t count )
  {
    C_.reserve( count );
  }

  void
  sort_connections( std::vector< Source >& sources )
  {
    sort::sort( sources, C_ );
  }

  void
  set_has_source_subsequent_targets( const index lcid,
    const bool subsequent_targets )
  {
    C_[ lcid ].set_has_source_subsequent_targets( subsequent_targets );
  }

  index
  find_first_target( const thread tid,
    const index start_lcid,
    const index target_gid ) const
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
  find_matching_target( const thread tid,
    const std::vector< index >& matching_lcids,
    const index target_gid ) const
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

  void
  print_connections( const thread tid ) const
  {
    std::cout << "---------------------------------------\n";
    for ( typename std::vector< ConnectionT >::const_iterator cit = C_.begin();
          cit != C_.end();
          ++cit )
    {
      std::cout << "(" << cit->get_target( tid )->get_gid() << ", "
                << cit->is_disabled() << ", "
                << cit->has_source_subsequent_targets() << ")";
      if ( not cit->has_source_subsequent_targets() )
      {
        std::cout << std::endl;
      }
    }
    std::cout << std::endl;
    std::cout << "---------------------------------------\n";
  }

  size_t
  size() const
  {
    return C_.size();
  }
};

} // of namespace nest

#endif
