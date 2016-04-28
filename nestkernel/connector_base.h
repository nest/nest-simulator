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

// C++ includes:
#include <cstdlib>
#include <vector>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "connection_label.h"
#include "connector_model.h"
#include "event.h"
#include "kernel_manager.h"
#include "nest_datums.h"
#include "nest_names.h"
#include "node.h"
#include "spikecounter.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"

#ifdef USE_PMA

#ifdef IS_K

extern PaddedPMA poormansallocpool[];

#else

extern PoorMansAllocator poormansallocpool;

#ifdef _OPENMP
#pragma omp threadprivate( poormansallocpool )
#endif

#endif

#endif

template < typename Tnew, typename Told, typename C >
inline Tnew*
suicide_and_resurrect( Told* connector, C connection )
{
#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
  Tnew* p =
    new ( poormansallocpool[ nest::kernel().vp_manager.get_thread_id() ].alloc( sizeof( Tnew ) ) )
      Tnew( *connector, connection );
#else
  Tnew* p = new ( poormansallocpool.alloc( sizeof( Tnew ) ) ) Tnew( *connector, connection );
#endif
  connector->~Told();
#else
  Tnew* p = new Tnew( *connector, connection );
  delete connector; // suicide
#endif
  return p;
}


namespace nest
{

// base clase to provide interface to decide
// - homogeneous connector (containing =1 synapse type)
//    -- which synapse type stored (syn_id)
// - heterogeneous connector (containing >1 synapse type)
class ConnectorBase
{

public:
  ConnectorBase();

  // destructor needed to delete connections
  virtual ~ConnectorBase(){};

  virtual void get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const = 0;
  virtual void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p ) = 0;

  virtual size_t get_num_connections() = 0;
  virtual size_t get_num_connections( synindex syn_id ) = 0;
  virtual size_t get_num_connections( size_t target_gid, size_t thrd, synindex syn_id ) = 0;

  virtual void get_connection(
    index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const = 0;

  virtual void get_connection(
    index source_gid,
    index target_gid,
    thread thrd,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const = 0;

  virtual void get_all_connections(
    index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const = 0;

  virtual void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const = 0;

  virtual index
  get_target_gid( const thread tid, const synindex syn_index, const unsigned int lcid ) const = 0;

  virtual void send_to_all( Event& e, thread tid, const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void send( thread tid, synindex syn_index, unsigned int lcid, Event& e, const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void trigger_update_weight( long_t vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void
  send_to_all_secondary( SecondaryEvent& e, thread t, const std::vector< ConnectorModel* >& cm ) = 0;

  // returns id of synapse type
  virtual synindex get_syn_id() const = 0;
};

// homogeneous connector containing >=K_cutoff entries ***OUTDATED***TODO@5g
// specialization to define recursion termination for push_back
// internally use a normal vector to store elements
template < typename ConnectionT >
class Connector : public ConnectorBase
{
private:
  std::vector< ConnectionT > C_;
  synindex syn_id_;

public:
  Connector( synindex syn_id )
    : syn_id_ ( syn_id )
  {
  }

  ~Connector()
  {
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    if ( syn_id == syn_id_ )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < C_.size() );
      C_[ p ].get_status( d );
    }
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    if ( syn_id == syn_id_ )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < C_.size() );
      C_[ p ].set_status( d, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
    }
  }

  size_t
  get_num_connections()
  {
    return C_.size();
  }

  size_t
  get_num_connections( synindex syn_id )
  {
    if ( syn_id == syn_id_ )
      return C_.size();
    else
      return 0;
  }

  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    typename std::vector< ConnectionT >::iterator C_it;
    size_t num_connections = 0;
    if ( syn_id == syn_id_ )
    {
      for ( C_it = C_.begin(); C_it != C_.end(); C_it++ )
      {
        if ( ( *C_it ).get_target( thrd )->get_gid() == target_gid )
        {
          num_connections++;
        }
      }
    }
    return num_connections;
  }

  Connector< ConnectionT >&
  push_back( const ConnectionT& c )
  {
    C_.push_back( c );
    return *this;
  }

  ConnectorBase&
  erase( size_t i )
  {
    typename std::vector< ConnectionT >::iterator it;
    it = C_.begin() + i;
    C_.erase( it );
    return *this;
  }

  size_t
  size()
  {
    return C_.size();
  }

  ConnectionT&
  at( size_t i )
  {
    if ( i >= C_.size() || i < 0 )
      throw std::out_of_range(
        String::compose( "Invalid attempt to access a connection: index %1 out of range.", i ) );
    return C_[ i ];
  }

  void
  get_connection(
    index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( syn_id_ == synapse_id )
    {
      if ( synapse_label == UNLABELED_CONNECTION ||
           C_[ lcid ].get_label() == synapse_label )
      {
        conns.push_back(
          ConnectionDatum(
            ConnectionID(
              source_gid,
              C_[ lcid ].get_target( tid )->get_gid(),
              tid,
              synapse_id,
              lcid )
            )
          );
      }
    }
  }

  void
  get_connection(
    index source_gid,
    index target_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( syn_id_ == synapse_id )
    {
        if ( synapse_label == UNLABELED_CONNECTION ||
             C_[ lcid ].get_label() == synapse_label )
        {
          if ( C_[ lcid ].get_target( tid )->get_gid() == target_gid )
          {
            conns.push_back(
              ConnectionDatum( ConnectionID( source_gid, target_gid, tid, synapse_id, lcid ) ) );
          }
        }
    }
  }

  void
  get_all_connections(
    index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( syn_id_ == synapse_id )
    {
      for ( size_t i = 0; i < C_.size(); ++i )
      {
        const index target_gid = C_[ i ].get_target( tid )->get_gid();
        if ( target_gid == requested_target_gid || requested_target_gid == 0 )
        {
          if ( synapse_label == UNLABELED_CONNECTION ||
               C_[ i ].get_label() == synapse_label )
          {
            conns.push_back(
              ConnectionDatum(
                ConnectionID(
                  source_gid,
                  target_gid,
                  tid,
                  synapse_id,
                  i )
                )
              );
          }
        }
      }
    }
  }

  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    typename std::vector< ConnectionT >::const_iterator C_it;
    if ( syn_id_ == synapse_id )
    {
      for ( C_it = C_.begin(); C_it != C_.end(); C_it++ )
      {
        target_gids.push_back( ( *C_it ).get_target( thrd )->get_gid() );
      }
    }
  }

  index
  get_target_gid( const thread tid, const synindex, const unsigned int lcid ) const
  {
    return C_[ lcid ].get_target( tid )->get_gid();
  }

  void
  send_to_all( Event& e, thread tid, const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      e.set_port( i );
      C_[ i ].send( e, tid, static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties() );
    }

  }

  void
  send( thread tid, synindex syn_index, unsigned int lcid, Event& e, const std::vector< ConnectorModel* >& cm )
  {
    e.set_port( lcid ); // TODO@5g: does this make sense?
    C_[ lcid ].send( e, tid, static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )->get_common_properties() );
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
      if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
             ->get_common_properties()
             .get_vt_gid() == vt_gid )
        C_[ i ].trigger_update_weight( t,
          dopa_spikes,
          t_trig,
          static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
            ->get_common_properties() );
  }

  void
  send_to_all_secondary( SecondaryEvent&, thread, const std::vector< ConnectorModel* >& )
  {
    assert( false ); // should not be called, only needed for heterogeneous connectors
  };

  synindex
  get_syn_id() const
  {
    return syn_id_;
  }
};

// heterogeneous connector containing different types of synapses is a
// vector of homogeneous connectors
class HetConnector : public std::vector< ConnectorBase* >, public ConnectorBase
{
public:
  HetConnector()
    : std::vector< ConnectorBase* >( 0 )
  {
  }

  virtual ~HetConnector()
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      delete at( i );
    }
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_synapse_status( syn_id, d, p );
    }
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->set_synapse_status( syn_id, cm, d, p );
    }
  }

  size_t
  get_num_connections()
  {
    size_t n = 0;
    for ( size_t i = 0; i < size(); ++i )
    {
      n += at( i )->get_num_connections();
    }
    return n;
  }

  size_t
  get_num_connections( synindex syn_id )
  {
    const size_t i = find_synapse_index( syn_id );
    if ( i != invalid_synindex )
    {
      return at( i )->get_num_connections( syn_id );
    }
    else
    {
      return 0;
    }
  }

  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    const size_t i = find_synapse_index( syn_id );
    if ( i != invalid_synindex )
    {
      return at( i )->get_num_connections( target_gid, thrd, syn_id );
    }
    else
    {
      return 0;
    }
  }

  void
  get_connection(
    index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_connection( source_gid, tid, synapse_id, lcid, synapse_label, conns );
    }
  }

  void
  get_connection(
    index source_gid,
    index target_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_connection( source_gid, target_gid, tid, synapse_id, lcid, synapse_label, conns );
    }
  }

  void
  get_all_connections(
    index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_all_connections( source_gid, requested_target_gid, tid, synapse_id, synapse_label, conns );
    }
  }

  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      if ( synapse_id == at( i )->get_syn_id() )
      {
        at( i )->get_target_gids( target_gids, thrd, synapse_id );
      }
    }
  }

  index
  get_target_gid( const thread tid, const synindex syn_index, const unsigned int lcid ) const
  {
    return at( syn_index )->get_target_gid( tid, syn_index, lcid );
  }

  void
  send( thread tid, synindex syn_index, unsigned int lcid, Event& e, const std::vector< ConnectorModel* >& cm )
  {
    at( syn_index )->send( tid, syn_index, lcid, e, cm );
  }

  void
  send_to_all( Event& e, thread tid, const std::vector< ConnectorModel* >& cm )
  {
    // only called for events from or to devices. can not contain
    // secondary-event connections, so we can delegate send to
    // homogeneous connectors for all connections
    for ( std::vector< ConnectorBase* >::iterator it = begin(); it != end(); ++it )
    {
      (*it)->send_to_all( e, tid, cm );
    }
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < size(); ++i )
      at( i )->trigger_update_weight( vt_gid, t, dopa_spikes, t_trig, cm );
  }

  // TODO@5g: can probably be removed
  void
  send_to_all_secondary( SecondaryEvent& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    assert( false );
    // // for all secondary connections delegate send to the matching homogeneous connector only
    // for ( size_t i = primary_end_; i < size(); ++i )
    //   if ( e.supports_syn_id( at( i )->get_syn_id() ) )
    //   {
    //     at( i )->send_to_all( e, t, cm );
    //     break;
    //   }
  }

  // returns id of synapse type
  synindex
  get_syn_id() const
  {
    return invalid_synindex;
  }

  // TODO@5g: can probably be removed
  void
  add_connector( bool is_primary, ConnectorBase* conn )
  {
    assert( false );
    // if ( is_primary )
    // {
    //   insert( begin() + primary_end_,
    //     conn ); // if empty, insert (begin(), conn) inserts into the first position
    //   ++primary_end_;
    // }
    // else
    // {
    //   push_back( conn );
    // }
  }

  // we check whether an entry with this synapse id already exists in
  // this heterogeneous connector, if not we return a flag indicating
  // an invalid index
  synindex
  find_synapse_index( synindex syn_id ) const
    {
      for ( size_t i = 0; i < size(); ++i )
      {
        if( at( i )->get_syn_id() == syn_id )
        {
          return i;
        }
      }
      return invalid_synindex;
    }
};

} // of namespace nest

#endif
