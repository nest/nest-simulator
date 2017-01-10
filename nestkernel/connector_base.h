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

  virtual void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const = 0;
  virtual void set_synapse_status( synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& d,
    port p ) = 0;

  virtual size_t get_num_connections() = 0;
  virtual size_t get_num_connections( synindex syn_id ) = 0;
  virtual size_t
  get_num_connections( index target_gid, thread tid, synindex syn_id ) = 0;

  virtual void get_connection( index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  virtual void get_connection( index source_gid,
    index target_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  virtual void get_all_connections( index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const = 0;

  virtual void get_source_lcids( const thread tid,
    const synindex syn_index,
    const index tgid,
    std::vector< index >& source_lcids ) const = 0;

  // virtual void get_target_gids( std::vector< index >& target_gids,
  //   thread tid,
  //   synindex synapse_id ) const = 0;

  virtual void get_target_gids( const thread tid,
    const synindex syn_index,
    const index start_lcid,
    std::vector< index >& target_gids ) const = 0;

  virtual index get_target_gid( const thread tid,
    const synindex syn_index,
    const unsigned int lcid ) const = 0;

  virtual void send_to_all( Event& e,
    thread tid,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual bool send( const thread tid,
    const synindex syn_index,
    const unsigned int lcid,
    Event& e,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void trigger_update_weight( long vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void send_to_all_secondary( SecondaryEvent& e,
    thread t,
    const std::vector< ConnectorModel* >& cm ) = 0;

  // returns id of synapse type
  virtual synindex get_syn_id() const = 0;

  virtual void
  sort_connections( std::vector< Source >& )
  {
    assert( false );
  };
  virtual void
  sort_connections( std::vector< std::vector< Source >* >& )
  {
    assert( false );
  };

  virtual void
  reserve( const size_t )
  {
    assert( false );
  };

  virtual void
  set_has_source_subsequent_targets( const synindex syn_index,
    const index lcid,
    const bool subsequent_targets )
  {
    assert( false );
  };
  virtual void
  set_has_source_subsequent_targets( const index lcid,
    const bool subsequent_targets )
  {
    assert( false );
  };

  virtual bool
  has_source_subsequent_targets( const synindex syn_index,
    const index lcid ) const
  {
    assert( false );
  };
  virtual bool
  has_source_subsequent_targets( const index lcid ) const
  {
    assert( false );
  };

  virtual index
  find_first_target( const thread tid,
    const synindex syn_index,
    const index start_lcid,
    const index tgid ) const
  {
    assert( false );
  };
  virtual index
  find_first_target( const thread tid,
    const index start_lcid,
    const index tgid ) const
  {
    assert( false );
  };

  virtual index
  find_matching_target( const thread tid,
    const index tgid,
    const std::vector< index >& matching_lcids ) const
  {
    assert( false );
  }
  virtual index
  find_matching_target( const thread tid,
    const index tgid,
    const synindex syn_index,
    const std::vector< index >& matching_lcids ) const
  {
    assert( false );
  }

  virtual void
  disable_connection( const synindex syn_index, const index lcid )
  {
    assert( false );
  };
  virtual void
  disable_connection( const index lcid )
  {
    assert( false );
  };

  virtual void
  remove_disabled_connections( const synindex syn_index,
    const index first_disabled_index )
  {
    assert( false );
  };
  virtual void
  remove_disabled_connections( const index first_disabled_index )
  {
    assert( false );
  };

  virtual void
  print_connections( const thread tid, const synindex syn_index ) const
  {
    assert( false );
  };
  virtual void
  print_connections( const thread tid ) const
  {
    assert( false );
  };

  virtual size_t
  size() const
  {
    assert( false );
  }

  virtual size_t
  capacity() const
  {
    assert( false );
  }
};

// homogeneous connector
// internally use a normal vector to store elements
template < typename ConnectionT >
class Connector : public ConnectorBase
{
private:
  std::vector< ConnectionT > C_;
  synindex syn_id_;

public:
  explicit Connector( synindex syn_id )
    : syn_id_( syn_id )
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
  set_synapse_status( synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& d,
    port p )
  {
    if ( syn_id == syn_id_ )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < C_.size() );
      C_[ p ].set_status(
        d, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
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
  get_num_connections( index target_gid, thread tid, synindex syn_id )
  {
    size_t num_connections = 0;
    if ( syn_id == syn_id_ )
    {
      for ( size_t i = 0; i < C_.size(); i++ )
      {
        if ( C_[ i ].get_target( tid )->get_gid() == target_gid )
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
    // use 1.5 growth strategy (see, e.g.,
    // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md)
    if ( C_.size() == C_.capacity() )
    {
      C_.reserve( ( C_.size() * 3 + 1 ) / 2 );
    }
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
      throw std::out_of_range( String::compose(
        "Invalid attempt to access a connection: index %1 out of range.", i ) );
    return C_[ i ];
  }

  void
  get_connection( index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    if ( syn_id_ == synapse_id && not C_[ lcid ].is_disabled() )
    {
      if ( synapse_label == UNLABELED_CONNECTION
        || C_[ lcid ].get_label() == synapse_label )
      {
        conns.push_back( ConnectionDatum( ConnectionID( source_gid,
          C_[ lcid ].get_target( tid )->get_gid(),
          tid,
          synapse_id,
          lcid ) ) );
      }
    }
  }

  void
  get_connection( index source_gid,
    index target_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    if ( syn_id_ == synapse_id && not C_[ lcid ].is_disabled() )
    {
      if ( synapse_label == UNLABELED_CONNECTION
        || C_[ lcid ].get_label() == synapse_label )
      {
        if ( C_[ lcid ].get_target( tid )->get_gid() == target_gid )
        {
          conns.push_back( ConnectionDatum(
            ConnectionID( source_gid, target_gid, tid, synapse_id, lcid ) ) );
        }
      }
    }
  }

  void
  get_all_connections( index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    if ( syn_id_ == synapse_id )
    {
      for ( size_t i = 0; i < C_.size(); ++i )
      {
        const index target_gid = C_[ i ].get_target( tid )->get_gid();
        if ( target_gid == requested_target_gid || requested_target_gid == 0 )
        {
          if ( synapse_label == UNLABELED_CONNECTION
            || C_[ i ].get_label() == synapse_label )
          {
            conns.push_back( ConnectionDatum(
              ConnectionID( source_gid, target_gid, tid, synapse_id, i ) ) );
          }
        }
      }
    }
  }

  void
  get_source_lcids( const thread tid,
    const synindex,
    const index tgid,
    std::vector< index >& source_lcids ) const
  {
    for ( index lcid = 0; lcid < C_.size(); ++lcid )
    {
      const index current_tgid = C_[ lcid ].get_target( tid )->get_gid();
      if ( current_tgid == tgid )
      {
        source_lcids.push_back( lcid );
      }
    }
  }

  void
  get_target_gids( const thread tid,
    const synindex,
    const index start_lcid,
    std::vector< index >& target_gids ) const
  {
    index lcid = start_lcid;
    while ( true )
    {
      target_gids.push_back( C_[ lcid ].get_target( tid )->get_gid() );
      std::cout<<"found target "<<target_gids.back()<<std::endl;

      if ( not C_[ lcid ].has_source_subsequent_targets() )
      {
        break;
      }

      ++lcid;
    }
  }

  index
  get_target_gid( const thread tid,
    const synindex,
    const unsigned int lcid ) const
  {
    return C_[ lcid ].get_target( tid )->get_gid();
  }

  void
  send_to_all( Event& e, thread tid, const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < C_.size(); ++i )
    {
      e.set_port( i );
      C_[ i ].send( e,
        tid,
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
          ->get_common_properties() );
    }
  }

  bool
  send( const thread tid,
    const synindex,
    const unsigned int lcid,
    Event& e,
    const std::vector< ConnectorModel* >& cm )
  {
    e.set_port( lcid ); // TODO@5g: does this make sense?
    if ( not C_[ lcid ].is_disabled() )
    {
      C_[ lcid ].send( e,
        tid,
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id_ ] )
          ->get_common_properties() );
    }
    return C_[ lcid ].has_source_subsequent_targets();
  }

  void
  trigger_update_weight( long vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double t_trig,
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
  send_to_all_secondary( SecondaryEvent&,
    thread,
    const std::vector< ConnectorModel* >& )
  {
    assert(
      false ); // should not be called, only needed for heterogeneous connectors
  };

  synindex
  get_syn_id() const
  {
    return syn_id_;
  }

  void
  sort_connections( std::vector< Source >& sources )
  {
    sort::sort( sources, C_ );
  }

  void
  reserve( const size_t count )
  {
    C_.reserve( count );
  }

  void
  set_has_source_subsequent_targets( const index lcid,
    const bool subsequent_targets )
  {
    C_[ lcid ].set_has_source_subsequent_targets( subsequent_targets );
  }

  bool
  has_source_subsequent_targets( const index lcid ) const
  {
    return C_[ lcid ].has_source_subsequent_targets();
  }

  index
  find_first_target( const thread tid,
    const index start_lcid,
    const index tgid ) const
  {
    index lcid = start_lcid;
    while ( true )
    {
      if ( C_[ lcid ].get_target( tid )->get_gid() == tgid )
      {
        // std::cout << " - found target " << tgid << std::endl;
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
    const index tgid,
    const std::vector< index >& matching_lcids ) const
  {
    for ( size_t i = 0; i < matching_lcids.size(); ++i )
    {
      if ( C_[ matching_lcids[ i ] ].get_target( tid )->get_gid() == tgid )
      {
        // std::cout << " - found target " << tgid << std::endl;
        return matching_lcids[ i ];
      }
    }

    return invalid_index;
  }

  void
  disable_connection( const index lcid )
  {
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

  size_t
  capacity() const
  {
    return C_.capacity();
  }
};

// heterogeneous connector containing different types of synapses
// each entry is of type connectorbase, so in principle the structure could be
// nested indefinitely
// the logic in add_connection, however, assumes that these entries are
// homogeneous connectors
class HetConnector : public std::vector< ConnectorBase* >, public ConnectorBase
{
public:

  // avoid ambigous names size and capacity, since they are also
  // defined in ConnectorBase
  using std::vector< ConnectorBase* >::size;
  using std::vector< ConnectorBase* >::capacity;

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
  set_synapse_status( synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& d,
    port p )
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
  get_num_connections( index target_gid, thread tid, synindex syn_id )
  {
    const size_t i = find_synapse_index( syn_id );
    if ( i != invalid_synindex )
    {
      return at( i )->get_num_connections( target_gid, tid, syn_id );
    }
    else
    {
      return 0;
    }
  }

  void
  get_connection( index source_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_connection(
        source_gid, tid, synapse_id, lcid, synapse_label, conns );
    }
  }

  void
  get_connection( index source_gid,
    index target_gid,
    thread tid,
    synindex synapse_id,
    index lcid,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_connection(
        source_gid, target_gid, tid, synapse_id, lcid, synapse_label, conns );
    }
  }

  void
  get_all_connections( index source_gid,
    index requested_target_gid,
    thread tid,
    synindex synapse_id,
    long synapse_label,
    std::deque< ConnectionID >& conns ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      at( i )->get_all_connections( source_gid,
        requested_target_gid,
        tid,
        synapse_id,
        synapse_label,
        conns );
    }
  }

  void
  get_source_lcids( const thread tid,
    const synindex syn_index,
    const index tgid,
    std::vector< index >& source_lcids ) const
  {
    at( syn_index )->get_source_lcids( tid, syn_index, tgid, source_lcids );
  }

  // void
  // get_target_gids( std::vector< size_t >& target_gids, thread tid, synindex
  // synapse_id ) const
  // {
  //   for ( size_t i = 0; i < size(); ++i )
  //   {
  //     if ( synapse_id == at( i )->get_syn_id() )
  //     {
  //       at( i )->get_target_gids( target_gids, tid, synapse_id );
  //     }
  //   }
  // }

  void
  get_target_gids( const thread tid,
    const synindex syn_index,
    const index start_lcid,
    std::vector< index >& target_gids ) const
  {
    // std::cout << "het conn" << std::endl;
    at( syn_index )->get_target_gids( tid, syn_index, start_lcid, target_gids );
  }

  index
  get_target_gid( const thread tid,
    const synindex syn_index,
    const unsigned int lcid ) const
  {
    return at( syn_index )->get_target_gid( tid, syn_index, lcid );
  }

  bool
  send( const thread tid,
    const synindex syn_index,
    const unsigned int lcid,
    Event& e,
    const std::vector< ConnectorModel* >& cm )
  {
    return at( syn_index )->send( tid, syn_index, lcid, e, cm );
  }

  void
  send_to_all( Event& e, thread tid, const std::vector< ConnectorModel* >& cm )
  {
    // only called for events from or to devices. can not contain
    // secondary-event connections, so we can delegate send to
    // homogeneous connectors for all connections
    for ( std::vector< ConnectorBase* >::iterator it = begin(); it != end();
          ++it )
    {
      ( *it )->send_to_all( e, tid, cm );
    }
  }

  void
  trigger_update_weight( long vt_gid,
    thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < size(); ++i )
      at( i )->trigger_update_weight( vt_gid, t, dopa_spikes, t_trig, cm );
  }

  // TODO@5g: can probably be removed
  void
  send_to_all_secondary( SecondaryEvent& e,
    thread t,
    const std::vector< ConnectorModel* >& cm )
  {
    assert( false );
    // // for all secondary connections delegate send to the matching
    // homogeneous connector only
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

  synindex
  get_syn_id( const synindex syn_index ) const
  {
    return at( syn_index )->get_syn_id();
  }

  // TODO@5g: can probably be removed
  void
  add_connector( bool is_primary, ConnectorBase* conn )
  {
    assert( false );
    // if ( is_primary )
    // {
    //   insert( begin() + primary_end_,
    //     conn ); // if empty, insert (begin(), conn) inserts into the first
    //     position
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
  find_synapse_index( const synindex syn_id ) const
  {
    for ( size_t i = 0; i < size(); ++i )
    {
      if ( at( i )->get_syn_id() == syn_id )
      {
        return i;
      }
    }
    return invalid_synindex;
  }

  void
  sort_connections( std::vector< std::vector< Source >* >& sources )
  {
    for ( unsigned int i = 0; i < size(); ++i )
    {
      at( i )->sort_connections( *sources[ i ] );
    }
  }

  void
  set_has_source_subsequent_targets( const synindex syn_index,
    const index lcid,
    const bool subsequent_targets )
  {
    at( syn_index )
      ->set_has_source_subsequent_targets( lcid, subsequent_targets );
  }

  bool
  has_source_subsequent_targets( const synindex syn_index,
    const index lcid ) const
  {
    return at( syn_index )->has_source_subsequent_targets( lcid );
  }

  index
  find_first_target( const thread tid,
    const synindex syn_index,
    const index start_lcid,
    const index tgid ) const
  {
    return at( syn_index )->find_first_target( tid, start_lcid, tgid );
  }

  index
  find_matching_target( const thread tid,
    const index tgid,
    const synindex syn_index,
    const std::vector< index >& matching_lcids ) const
  {
    return at( syn_index )->find_matching_target( tid, tgid, matching_lcids );
  }

  void
  disable_connection( const synindex syn_index, const index lcid )
  {
    at( syn_index )->disable_connection( lcid );
  }

  void
  remove_disabled_connections( const synindex syn_index,
    const index first_disabled_index )
  {
    at( syn_index )->remove_disabled_connections( first_disabled_index );
  }

  void
  print_connections( const thread tid, const synindex syn_index ) const
  {
    if ( syn_index >= size() )
    {
      return;
    }

    at( syn_index )->print_connections( tid );
  }
};

} // of namespace nest

#endif
