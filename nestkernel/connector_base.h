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

#include <vector>

#include "node.h"
#include "event.h"
#include "network.h"
#include "dictutils.h"
#include "spikecounter.h"
#include "nest_names.h"
#include "connector_model.h"
#include "connection_label.h"
#include "nest_datums.h"
#ifdef _OPENMP
#include <omp.h>
#endif


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
  Tnew* p = new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( Tnew ) ) )
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

template < typename Tnew, typename Told >
inline Tnew*
suicide_and_resurrect( Told* connector, size_t i )
{
#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
  Tnew* p =
    new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( Tnew ) ) ) Tnew( *connector, i );
#else
  Tnew* p = new ( poormansallocpool.alloc( sizeof( Tnew ) ) ) Tnew( *connector, i );
#endif
  connector->~Told();
#else
  Tnew* p = new Tnew( *connector, i );
  delete connector; // suicide
#endif
  return p;
}

template < typename Tnew, typename Told >
inline void
suicide( Told* connector )
{
#ifdef USE_PMA
  connector->~Told();
#else
  delete connector; // suicide
#endif
}

// when to truncate the recursive instantiation
#define K_cutoff 3

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

  virtual void get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const = 0;
  virtual void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p ) = 0;

  virtual size_t get_num_connections() = 0;
  virtual size_t get_num_connections( synindex syn_id ) = 0;
  virtual size_t get_num_connections( size_t target_gid, size_t thrd, synindex syn_id ) = 0;

  virtual void get_connections( size_t source_gid,
    size_t thrd,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const = 0;

  virtual void get_connections( size_t source_gid,
    size_t target_gid,
    size_t thrd,
    size_t synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const = 0;

  virtual void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const = 0;

  virtual void send( Event& e, thread t, const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void trigger_update_weight( long_t vt_gid,
    thread t,
    const vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm ) = 0;

  virtual void
  send_secondary( SecondaryEvent& e, thread t, const std::vector< ConnectorModel* >& cm ) = 0;

  // returns id of synapse type
  virtual synindex get_syn_id() const = 0;

  // returns true, if all synapse models are of same type
  virtual bool homogeneous_model() = 0;

  // destructor needed to delete connections
  virtual ~ConnectorBase(){};

  double_t
  get_t_lastspike() const
  {
    return t_lastspike_;
  }
  void
  set_t_lastspike( const double_t t_lastspike )
  {
    t_lastspike_ = t_lastspike;
  }

private:
  double_t t_lastspike_;
};

// vector with 1 vtable overhead
// vector like base class to abstract away the template argument K
// provides interface like vector i.p. (suicidal) push_back
template < typename ConnectionT >
class vector_like : public ConnectorBase
{

public:
  virtual ConnectorBase& push_back( const ConnectionT& c ) = 0;
  virtual ConnectorBase& erase( size_t i ) = 0;
  virtual size_t size() = 0;
  virtual ConnectionT& at( size_t i ) = 0;

  void
  send_secondary( SecondaryEvent&, thread, const std::vector< ConnectorModel* >& )
  {
    assert( false ); // should not be called, only needed for heterogeneous connectors
  };
};

// homogeneous connector containing K entries
template < size_t K, typename ConnectionT >
class Connector : public vector_like< ConnectionT > // unfortunately, we need the virtual base class
{
  ConnectionT C_[ K ];

public:
  Connector( const Connector< K - 1, ConnectionT >& Cm1,
    const ConnectionT& c ) //: syn_id_(Cm1.get_syn_id())
  {
    for ( size_t i = 0; i < K - 1; i++ )
      C_[ i ] = Cm1.get_C()[ i ];
    C_[ K - 1 ] = c;
  }

  /**
   * Creates a new connector and remove the ith connection. To do so, the contents
   * of the original connector are copied into the new one. The copy is performed
   * in two parts, first up to the specified index and then the rest of the
   * connections after the specified index in order to
   * exclude the ith connection from the copy. As a result, returns a connector
   * with size K from a connector of size K+1.
   *
   * @param Cm1 the original connector
   * @param i the index of the connection to be deleted
   */
  Connector( const Connector< K + 1, ConnectionT >& Cm1, size_t i ) //: syn_id_(Cm1.get_syn_id())
  {
    assert( i < K && i >= 0 );
    for ( size_t k = 0; k < i; k++ )
    {
      C_[ k ] = Cm1.get_C()[ k ];
    }

    for ( size_t k = i + 1; k < K + 1; k++ )
    {
      C_[ k - 1 ] = Cm1.get_C()[ k ];
    }
  }

  ~Connector()
  {
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < K );
      C_[ p ].get_status( d );
    }
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < K );
      C_[ p ].set_status( d, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
    }
  }

  size_t
  get_num_connections()
  {
    return K;
  }

  size_t
  get_num_connections( synindex syn_id )
  {
    if ( syn_id == get_syn_id() )
      return K;
    else
      return 0;
  }

  /**
   * Returns the number of connections that this connector is holding for
   * a given target and synapse type.
   * @param target_gid The GID of the target of the searched connections
   * @param thrd The thread id of the target
   * @param syn_id Id of the synapse of the searched connections
   * @return
   */
  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    size_t num_connections = 0;
    if ( syn_id == get_syn_id() )
    {
      for ( size_t i = 0; i < K; i++ )
      {
        if ( C_[ i ].get_target( thrd )->get_gid() == target_gid )
        {
          num_connections++;
        }
      }
    }
    return num_connections;
  }

  Connector< K + 1, ConnectionT >&
  push_back( const ConnectionT& c )
  {
    return *suicide_and_resurrect< Connector< K + 1, ConnectionT > >( this, c );
  }

  /**
   * Delete a single connection from the connector
   * @param i the index of the connection to be erased
   * @return A connector of size K-1
   */
  Connector< K - 1, ConnectionT >&
  erase( size_t i )
  {
    // try to cast the connector one size shorter
    return *suicide_and_resurrect< Connector< K - 1, ConnectionT > >( this, i );
  }

  /**
   * Getter for the size of the Connection array
   * @return The number of connections which this Connector currently holds
   */
  size_t
  size()
  {
    return K;
  }

  /**
   * Operator to obtain a connection at a given index from the Connector,
   * in the same manner as it would work with an array.
   * @param i the index of the connection to be retrieved.
   * @return The connection stored at position i.
   */
  ConnectionT&
  at( size_t i )
  {
    if ( i >= K || i < 0 )
    {
      throw std::out_of_range(
        String::compose( "Invalid attempt to access a connection: index %1 out of range.", i ) );
    }
    return C_[ i ];
  }

  void
  get_connections( size_t source_gid,
    size_t thrd,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < K; i++ )
      if ( get_syn_id() == synapse_id )
        if ( synapse_label == UNLABELED_CONNECTION || C_[ i ].get_label() == synapse_label )
          conns.push_back( ConnectionDatum( ConnectionID(
            source_gid, C_[ i ].get_target( thrd )->get_gid(), thrd, synapse_id, i ) ) );
  }

  void
  get_connections( size_t source_gid,
    size_t target_gid,
    size_t thrd,
    size_t synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < K; i++ )
      if ( get_syn_id() == synapse_id )
        if ( synapse_label == UNLABELED_CONNECTION || C_[ i ].get_label() == synapse_label )
          if ( C_[ i ].get_target( thrd )->get_gid() == target_gid )
            conns.push_back(
              ConnectionDatum( ConnectionID( source_gid, target_gid, thrd, synapse_id, i ) ) );
  }

  /**
   * Return the GIDs of the target nodes in a given thread, for all connections
   * on this Connector which match a defined synapse_id.
   * @param target_gids Vector to store the GIDs of the target nodes
   * @param thrd Thread where targets are being looked for
   * @param synapse_id Synapse type
   */
  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    if ( get_syn_id() == synapse_id )
    {
      for ( size_t i = 0; i < K; i++ )
      {
        target_gids.push_back( C_[ i ].get_target( thrd )->get_gid() );
      }
    }
  }

  void
  send( Event& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    synindex syn_id = C_[ 0 ].get_syn_id();
    for ( size_t i = 0; i < K; i++ )
    {
      e.set_port( i );
      C_[ i ].send( e,
        t,
        ConnectorBase::get_t_lastspike(),
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
          ->get_common_properties() );
    }
    ConnectorBase::set_t_lastspike( e.get_stamp().get_ms() );
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    synindex syn_id = C_[ 0 ].get_syn_id();
    for ( size_t i = 0; i < K; i++ )
      if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
             ->get_common_properties()
             .get_vt_gid() == vt_gid )
        C_[ i ].trigger_update_weight( t,
          dopa_spikes,
          t_trig,
          static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
            ->get_common_properties() );
  }

  synindex
  get_syn_id() const
  {
    // return syn_id_;
    return C_[ 0 ].get_syn_id();
  }

  const ConnectionT*
  get_C() const
  {
    return C_;
  }

  bool
  homogeneous_model()
  {
    return true;
  }
};

// homogeneous connector containing 1 entry (specialization to define constructor)
template < typename ConnectionT >
class Connector< 1, ConnectionT > : public vector_like< ConnectionT >
{
  ConnectionT C_[ 1 ];

public:
  // Connector(const ConnectionT &c, synindex syn_id) : syn_id_(syn_id)
  Connector( const ConnectionT& c )
  {
    C_[ 0 ] = c;
  };

  /**
   * Returns a new Connector of size 1 after deleting one of the
   * connections.
   * @param Cm1 Original Connector of size 2
   * @param i Index of the connection to be erased
   */
  Connector( const Connector< 2, ConnectionT >& Cm1, size_t i ) //: syn_id_(Cm1.get_syn_id())
  {
    assert( i < 2 && i >= 0 );
    if ( i == 0 )
    {
      C_[ 0 ] = Cm1.get_C()[ 1 ];
    }
    if ( i == 1 )
    {
      C_[ 0 ] = Cm1.get_C()[ 0 ];
    }
  }

  ~Connector()
  {
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
    {
      assert( static_cast< size_t >( p ) == 0 );
      C_[ 0 ].get_status( d );
    }
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
    {
      assert( static_cast< size_t >( p ) == 0 );
      C_[ 0 ].set_status( d, static_cast< GenericConnectorModel< ConnectionT >& >( cm ) );
    }
  }

  size_t
  get_num_connections()
  {
    return 1;
  }

  size_t
  get_num_connections( synindex syn_id )
  {
    if ( syn_id == get_syn_id() )
      return 1;
    else
      return 0;
  }

  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    size_t num_connections = 0;
    if ( syn_id == get_syn_id() )
    {
      if ( C_[ 0 ].get_target( thrd )->get_gid() == target_gid )
      {
        num_connections = 1;
      }
    }
    return num_connections;
  }

  Connector< 2, ConnectionT >&
  push_back( const ConnectionT& c )
  {
    return *suicide_and_resurrect< Connector< 2, ConnectionT > >( this, c );
  }

  ConnectorBase& erase( size_t )
  {
    // Destroys the Connector
    suicide< Connector< 1, ConnectionT > >( this );
    ConnectorBase* p = 0;
    return *p;
  }

  size_t
  size()
  {
    return 1;
  }

  ConnectionT&
  at( size_t i )
  {

    if ( i != 0 )
    {
      throw std::out_of_range(
        String::compose( "Invalid attempt to access a connection: index %1 out of range.", i ) );
    }
    return C_[ i ];
  }

  void
  get_connections( size_t source_gid,
    size_t thrd,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( get_syn_id() == synapse_id )
    {
      if ( synapse_label == UNLABELED_CONNECTION || C_[ 0 ].get_label() == synapse_label )
      {
        conns.push_back( ConnectionDatum( ConnectionID(
          source_gid, C_[ 0 ].get_target( thrd )->get_gid(), thrd, synapse_id, 0 ) ) );
      }
    }
  }

  void
  get_connections( size_t source_gid,
    size_t target_gid,
    size_t thrd,
    size_t synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( get_syn_id() == synapse_id )
    {
      if ( synapse_label == UNLABELED_CONNECTION || C_[ 0 ].get_label() == synapse_label )
      {
        if ( C_[ 0 ].get_target( thrd )->get_gid() == target_gid )
          conns.push_back(
            ConnectionDatum( ConnectionID( source_gid, target_gid, thrd, synapse_id, 0 ) ) );
      }
    }
  }

  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    if ( get_syn_id() == synapse_id )
    {
      target_gids.push_back( C_[ 0 ].get_target( thrd )->get_gid() );
    }
  }

  void
  send( Event& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    e.set_port( 0 );
    C_[ 0 ].send( e,
      t,
      ConnectorBase::get_t_lastspike(),
      static_cast< GenericConnectorModel< ConnectionT >* >( cm[ C_[ 0 ].get_syn_id() ] )
        ->get_common_properties() );
    ConnectorBase::set_t_lastspike( e.get_stamp().get_ms() );
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    synindex syn_id = C_[ 0 ].get_syn_id();
    if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
           ->get_common_properties()
           .get_vt_gid() == vt_gid )
      C_[ 0 ].trigger_update_weight( t,
        dopa_spikes,
        t_trig,
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
          ->get_common_properties() );
  }

  synindex
  get_syn_id() const
  {
    return C_[ 0 ].get_syn_id();
  }

  const ConnectionT*
  get_C() const
  {
    return C_;
  }

  bool
  homogeneous_model()
  {
    return true;
  }
};


// homogeneous connector containing >=K_cutoff entries
// specialization to define recursion termination for push_back
// internally use a normal vector to store elements
template < typename ConnectionT >
class Connector< K_cutoff, ConnectionT > : public vector_like< ConnectionT >
{
  std::vector< ConnectionT > C_;

public:
  Connector( const Connector< K_cutoff - 1, ConnectionT >& C, const ConnectionT& c )
    : C_( K_cutoff ) //, syn_id_(C.get_syn_id())
  {
    for ( size_t i = 0; i < K_cutoff - 1; i++ )
      C_[ i ] = C.get_C()[ i ];
    C_[ K_cutoff - 1 ] = c;
  };

  /**
   * Creates a new connector and removes the ith connection. To do so, the contents
   * of the original connector are copied into the new one. The copy is performed
   * in two parts, first up to the specified index and then the rest of the
   * connections after the specified index in order to
   * exclude the ith connection from the copy. As a result, returns a connector
   * with size K_cutoff-1 from a connector of size K_cutoff.
   *
   * @param Cm1 Original connector of size K_cutoff.
   * @param i The index of the connection to be deleted.
   */
  Connector( const Connector< K_cutoff, ConnectionT >& Cm1, size_t i ) //: syn_id_(Cm1.get_syn_id())
  {
    assert( i < Cm1.get_C().size() && i >= 0 );
    for ( size_t k = 0; k < i; k++ )
    {
      C_[ k ] = Cm1.get_C()[ k ];
    }

    for ( size_t k = i + 1; k < K_cutoff; k++ )
    {
      C_[ k ] = Cm1.get_C()[ k + 1 ];
    }
  }

  ~Connector()
  {
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
    {
      assert( p >= 0 && static_cast< size_t >( p ) < C_.size() );
      C_[ p ].get_status( d );
    }
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    if ( syn_id == C_[ 0 ].get_syn_id() )
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
    if ( syn_id == get_syn_id() )
      return C_.size();
    else
      return 0;
  }

  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    typename std::vector< ConnectionT >::iterator C_it;
    size_t num_connections = 0;
    if ( syn_id == get_syn_id() )
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

  Connector< K_cutoff, ConnectionT >&
  push_back( const ConnectionT& c )
  {
    C_.push_back( c );
    return *this;
  }

  Connector< K_cutoff, ConnectionT >&
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
  get_connections( size_t source_gid,
    size_t thrd,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < C_.size(); i++ )
      if ( get_syn_id() == synapse_id )
        if ( synapse_label == UNLABELED_CONNECTION || C_[ i ].get_label() == synapse_label )
          conns.push_back( ConnectionDatum( ConnectionID(
            source_gid, C_[ i ].get_target( thrd )->get_gid(), thrd, synapse_id, i ) ) );
  }

  void
  get_connections( size_t source_gid,
    size_t target_gid,
    size_t thrd,
    size_t synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    if ( get_syn_id() == synapse_id )
      for ( size_t i = 0; i < C_.size(); i++ )
        if ( synapse_label == UNLABELED_CONNECTION || C_[ i ].get_label() == synapse_label )
          if ( C_[ i ].get_target( thrd )->get_gid() == target_gid )
            conns.push_back(
              ConnectionDatum( ConnectionID( source_gid, target_gid, thrd, synapse_id, i ) ) );
  }

  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    typename std::vector< ConnectionT >::const_iterator C_it;
    if ( get_syn_id() == synapse_id )
    {
      for ( C_it = C_.begin(); C_it != C_.end(); C_it++ )
      {
        target_gids.push_back( ( *C_it ).get_target( thrd )->get_gid() );
      }
    }
  }

  void
  send( Event& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    synindex syn_id = C_[ 0 ].get_syn_id();

    for ( size_t i = 0; i < C_.size(); i++ )
    {

      e.set_port( i );
      C_[ i ].send( e,
        t,
        ConnectorBase::get_t_lastspike(),
        static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
          ->get_common_properties() );
    }

    ConnectorBase::set_t_lastspike( e.get_stamp().get_ms() );
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    synindex syn_id = C_[ 0 ].get_syn_id();
    for ( size_t i = 0; i < C_.size(); i++ )
      if ( static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
             ->get_common_properties()
             .get_vt_gid() == vt_gid )
        C_[ i ].trigger_update_weight( t,
          dopa_spikes,
          t_trig,
          static_cast< GenericConnectorModel< ConnectionT >* >( cm[ syn_id ] )
            ->get_common_properties() );
  }

  synindex
  get_syn_id() const
  {
    return C_[ 0 ].get_syn_id();
  }

  bool
  homogeneous_model()
  {
    return true;
  }
};

// heterogeneous connector containing different types of synapses
// each entry is of type connectorbase, so in principle the structure could be
// nested indefinitely
// the logic in add_connection, however, assumes that these entries are
// homogeneous connectors
class HetConnector : public vector< ConnectorBase* >, public ConnectorBase
{
private:
  synindex
    primary_end_; // index of first secondary connector contained in the heterogeneous connector

public:
  HetConnector()
    : vector< ConnectorBase* >()
    , primary_end_( 0 )
  {
  }

  virtual ~HetConnector()
  {
    for ( size_t i = 0; i < size(); i++ )
#ifdef USE_PMA
      at( i )->~ConnectorBase();
#else
      delete at( i );
#endif
  }

  void
  get_synapse_status( synindex syn_id, DictionaryDatum& d, port p ) const
  {
    for ( size_t i = 0; i < size(); i++ )
      at( i )->get_synapse_status( syn_id, d, p );
  }

  void
  set_synapse_status( synindex syn_id, ConnectorModel& cm, const DictionaryDatum& d, port p )
  {
    for ( size_t i = 0; i < size(); i++ )
      at( i )->set_synapse_status( syn_id, cm, d, p );
  }

  size_t
  get_num_connections()
  {
    size_t n = 0;
    for ( size_t i = 0; i < size(); i++ )
    {
      n += at( i )->get_num_connections();
    }
    return n;
  }

  size_t
  get_num_connections( synindex syn_id )
  {
    for ( size_t i = 0; i < size(); i++ )
      if ( syn_id == at( i )->get_syn_id() )
        return at( i )->get_num_connections();
    return 0;
  }

  size_t
  get_num_connections( size_t target_gid, size_t thrd, synindex syn_id )
  {
    for ( size_t i = 0; i < size(); i++ )
    {
      if ( syn_id == at( i )->get_syn_id() )
      {
        return at( i )->get_num_connections( target_gid, thrd, syn_id );
      }
    }
    return 0;
  }

  void
  get_connections( size_t source_gid,
    size_t thrd,
    synindex synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < size(); i++ )
      at( i )->get_connections( source_gid, thrd, synapse_id, synapse_label, conns );
  }

  void
  get_connections( size_t source_gid,
    size_t target_gid,
    size_t thrd,
    size_t synapse_id,
    long_t synapse_label,
    ArrayDatum& conns ) const
  {
    for ( size_t i = 0; i < size(); i++ )
      at( i )->get_connections( source_gid, target_gid, thrd, synapse_id, synapse_label, conns );
  }

  void
  get_target_gids( std::vector< size_t >& target_gids, size_t thrd, synindex synapse_id ) const
  {
    for ( size_t i = 0; i < size(); i++ )
    {
      if ( synapse_id == at( i )->get_syn_id() )
      {
        at( i )->get_target_gids( target_gids, thrd, synapse_id );
      }
    }
  }

  void
  send( Event& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    // for all primary connections delegate send to homogeneous connectors
    for ( size_t i = 0; i < primary_end_; i++ )
      at( i )->send( e, t, cm );
  }

  void
  trigger_update_weight( long_t vt_gid,
    thread t,
    const vector< spikecounter >& dopa_spikes,
    double_t t_trig,
    const std::vector< ConnectorModel* >& cm )
  {
    for ( size_t i = 0; i < size(); i++ )
      at( i )->trigger_update_weight( vt_gid, t, dopa_spikes, t_trig, cm );
  }

  void
  send_secondary( SecondaryEvent& e, thread t, const std::vector< ConnectorModel* >& cm )
  {
    // for all secondary connections delegate send to the matching homogeneous connector only
    for ( size_t i = primary_end_; i < size(); i++ )
      if ( e.supports_syn_id( at( i )->get_syn_id() ) )
      {
        at( i )->send( e, t, cm );
        break;
      }
  }

  // returns id of synapse type
  synindex
  get_syn_id() const
  {
    return invalid_synindex;
  }

  // returns true, if all synapse models are of same type
  bool
  homogeneous_model()
  {
    return false;
  }

  void
  add_connector( bool is_primary, ConnectorBase* conn )
  {
    if ( is_primary )
    {
      insert( begin() + primary_end_,
        conn ); // if empty, insert (begin(), conn) inserts into the first position
      ++primary_end_;
    }
    else
    {
      push_back( conn );
    }
  }
};

} // of namespace nest

#endif
