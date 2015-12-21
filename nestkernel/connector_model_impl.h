/*
 *  connector_model_impl.h
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

#ifndef CONNECTOR_MODEL_IMPL_H
#define CONNECTOR_MODEL_IMPL_H

#include "nest_time.h"
#include "nest_timeconverter.h"
#include "dictutils.h"
#include "network.h"
#include "connector_model.h"
#include "connector_base.h"
#include "connection_label.h"
#include "string_utils.h"


template < typename T, typename C >
inline T*
allocate( C c )
{
#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
  T* p = new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( T ) ) ) T( c );
#else
  T* p = new ( poormansallocpool.alloc( sizeof( T ) ) ) T( c );
#endif
#else
  T* p = new T( c );
#endif
  // we need to check, if the two lowest bits of the pointer
  // are 0, because we want to use them to encode for the
  // existence of primary and secondary events
  assert( ( reinterpret_cast< unsigned long >( p ) & 3 ) == 0 );
  return p;
}

template < typename T >
inline T*
allocate()
{
#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
  T* p = new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( T ) ) ) T();
#else
  T* p = new ( poormansallocpool.alloc( sizeof( T ) ) ) T();
#endif
#else
  T* p = new T();
#endif
  // we need to check, if the two lowest bits of the pointer
  // are 0, because we want to use them to encode for the
  // existence of primary and secondary events
  assert( ( reinterpret_cast< unsigned long >( p ) & 3 ) == 0 );
  return p;
}


namespace nest
{

// standard implementation to obtain the default delay, assuming that it
// is located in GenericConnectorModel::default_connection
// synapse types with homogeneous delays must provide a specialization
// that returns the default delay from CommonProperties (or from  else where)
// template<typename ConnectionT>
// double_t get_default_delay(const GenericConnectorModel<ConnectionT> &cm)
// {
//   //std::cout << "standard implementation of get_default_delay" << std::endl;
//   return cm.get_default_connection().get_delay();
// }

// template<typename ConnectionT>
// SynIdDelay & syn_id_delay(const GenericConnectorModel<ConnectionT> &cm)
// {
//   return cm.get_default_connection().get_syn_id_delay();
// }

template < typename ConnectionT >
ConnectorModel*
GenericConnectorModel< ConnectionT >::clone( std::string name ) const
{
  return new GenericConnectorModel( *this, name ); // calls copy construtor
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::calibrate( const TimeConverter& tc )
{
  // calibrate the dalay of the default properties here
  default_connection_.calibrate( tc );

  // Calibrate will be called after a change in resolution, when there are no network elements
  // present.

  // calibrate any time objects that might reside in CommonProperties
  cp_.calibrate( tc );

  min_delay_ = tc.from_old_steps( min_delay_.get_steps() );
  max_delay_ = tc.from_old_steps( max_delay_.get_steps() );
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::get_status( DictionaryDatum& d ) const
{
  // first get properties common to all synapses
  // these are stored only once (not within each Connection)
  cp_.get_status( d );

  // then get default properties for individual synapses
  default_connection_.get_status( d );

  ( *d )[ "min_delay" ] = get_min_delay().get_ms();
  ( *d )[ "max_delay" ] = get_max_delay().get_ms();
  ( *d )[ names::receptor_type ] = receptor_type_;
  ( *d )[ "synapsemodel" ] = LiteralDatum( name_ );

  long_t old_count;
  // if field "num_connections" already exists
  // we will add to this number
  // used to add up connections from connector_models
  // for different threads
  if ( updateValue< long_t >( d, "num_connections", old_count ) )
    ( *d )[ "num_connections" ] = old_count + get_num_connections();
  else
    ( *d )[ "num_connections" ] = get_num_connections();
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::set_status( const DictionaryDatum& d )
{
  updateValue< long_t >( d, names::receptor_type, receptor_type_ );
#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue< long_t >( d, names::music_channel, receptor_type_ );
#endif

  // For the minimum delay, we always round down. The easiest way to do this,
  // is to round up and then subtract one step. The only remaining edge case
  // is that the min delay is exactly at a step, in which case one would get
  // a min delay that is one step too small. We can detect this by an
  // additional test.
  double_t delay_tmp;
  bool min_delay_updated = updateValue< double_t >( d, "min_delay", delay_tmp );
  Time new_min_delay;
  if ( min_delay_updated )
  {
    delay new_min_delay_steps = Time( Time::ms_stamp( delay_tmp ) ).get_steps();
    if ( Time( Time::step( new_min_delay_steps ) ).get_ms() > delay_tmp )
    {
      new_min_delay_steps -= 1;
    }
    new_min_delay = Time( Time::step( new_min_delay_steps ) );
  }

  // For the maximum delay, we always round up, using ms_stamp
  bool max_delay_updated = updateValue< double_t >( d, "max_delay", delay_tmp );
  Time new_max_delay = Time( Time::ms_stamp( delay_tmp ) );

  if ( min_delay_updated xor max_delay_updated )
  {
    throw BadProperty( "Both min_delay and max_delay have to be specified" );
  }

  if ( min_delay_updated && max_delay_updated )
  {
    if ( num_connections_ > 0 )
    {
      throw BadProperty( "Connections already exist. Please call ResetKernel first" );
    }
    else if ( new_min_delay < Time::get_resolution() )
    {
      throw BadDelay(
        new_min_delay.get_ms(), "min_delay must be greater than or equal to resolution." );
    }
    else if ( new_max_delay < new_min_delay )
    {
      throw BadDelay(
        new_min_delay.get_ms(), "min_delay must be smaller than or equal to max_delay." );
    }
    else
    {
      min_delay_ = new_min_delay;
      max_delay_ = new_max_delay;
      user_set_delay_extrema_ = true;
    }
  }

  // If the parameter dict d contains /delay, this should set the delay
  // on the default connection, but not affect the actual min/max_delay
  // until a connection with that default delay is created. Since the
  // set_status calls on common properties and default connection may
  // modify min/max delay, we need to preserve and restore.

  const Time save_min_delay_ = min_delay_;
  const Time save_max_delay_ = max_delay_;

  cp_.set_status( d, *this );
  default_connection_.set_status( d, *this );

  min_delay_ = save_min_delay_;
  max_delay_ = save_max_delay_;

  // we've possibly just got a new default delay. So enforce checking next time it is used
  default_delay_needs_check_ = true;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::used_default_delay()
{
  // if not used before, check now. Solves bug #138, MH 08-01-08
  // replaces whole delay checking for the default delay, see bug #217, MH 08-04-24
  // get_default_delay_ must be overridden by derived class to return the correct default delay
  // (either from commonprops or default connection)
  if ( default_delay_needs_check_ )
  {
    assert_valid_delay_ms( default_connection_.get_delay() );
    default_delay_needs_check_ = false;
  }
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::set_syn_id( synindex syn_id )
{
  default_connection_.set_syn_id( syn_id );
}

/**
 * delay and weight have the default value NAN.
 * NAN is a special value in cmath, which describes double values that
 * are not a number. If delay or weight is omitted in an add_connection call,
 * NAN indicates this and weight/delay are set only, if they are valid.
 */
template < typename ConnectionT >
ConnectorBase*
GenericConnectorModel< ConnectionT >::add_connection( Node& src,
  Node& tgt,
  ConnectorBase* conn,
  synindex syn_id,
  double_t delay,
  double_t weight )
{
  // create a new instance of the default connection
  ConnectionT c = ConnectionT( default_connection_ );
  if ( not numerics::is_nan( weight ) )
  {
    c.set_weight( weight );
  }
  if ( not numerics::is_nan( delay ) )
  {
    assert_valid_delay_ms( delay );
    c.set_delay( delay );
  }
  else
  {
    // tell the connector model, that we used the default delay
    used_default_delay();
  }

  return add_connection( src, tgt, conn, syn_id, c, receptor_type_ );
}

/**
 * delay and weight have the default value NAN.
 * NAN is a special value in cmath, which describes double values that
 * are not a number. If delay or weight is omitted in an add_connection call,
 * NAN indicates this and weight/delay are set only, if they are valid.
 */
template < typename ConnectionT >
ConnectorBase*
GenericConnectorModel< ConnectionT >::add_connection( Node& src,
  Node& tgt,
  ConnectorBase* conn,
  synindex syn_id,
  DictionaryDatum& p,
  double_t delay,
  double_t weight )
{
  if ( not numerics::is_nan( delay ) )
  {
    assert_valid_delay_ms( delay );

    if ( p->known( names::delay ) )
      throw BadParameter(
        "Parameter dictionary must not contain delay if delay is given explicitly." );
  }
  else
  {
    // check delay
    double_t delay = 0.0;

    if ( updateValue< double_t >( p, names::delay, delay ) )
      assert_valid_delay_ms( delay );
    else
      used_default_delay();
  }

  // create a new instance of the default connection
  ConnectionT c = ConnectionT( default_connection_ );
  if ( !p->empty() )
    c.set_status( p, *this ); // reference to connector model needed here to check delay (maybe this
                              // could be done one level above?)
  if ( not numerics::is_nan( weight ) )
  {
    c.set_weight( weight );
  }
  if ( not numerics::is_nan( delay ) )
  {
    c.set_delay( delay );
  }

  // We must use a local variable here to hold the actual value of the
  // receptor type. We must not change the receptor_type_ data member, because
  // that represents the *default* value. See #921.
  rport actual_receptor_type = receptor_type_;
#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue< long_t >( p, names::music_channel, actual_receptor_type );
#endif
  updateValue< long_t >( p, names::receptor_type, actual_receptor_type );

  return add_connection( src, tgt, conn, syn_id, c, actual_receptor_type );
}


// needs Connection < >

template < typename ConnectionT >
ConnectorBase*
GenericConnectorModel< ConnectionT >::add_connection( Node& src,
  Node& tgt,
  ConnectorBase* conn,
  synindex syn_id,
  ConnectionT& c,
  rport receptor_type )
{
  // here we need to distinguish several cases:
  // - neuron src has no target on this machine yet (case 0)
  // - neuron src has n targets on this machine, all of same type syn_id_existing (case 1)
  //     -- new connection of type syn_id == syn_id_existing
  //     -- new connection of type syn_id != syn_id_existing
  // - neuron src has n targets of more than a single synapse type (case 2)
  //     -- there are already synapses of type syn_id
  //     -- there are no connections of type syn_id yet

  if ( conn == 0 )
  {
    // case 0

    // the following line will throw an exception, if it does not work
    c.check_connection(
      src, tgt, receptor_type, 0., get_common_properties() ); // set last_spike to 0

    // no entry at all, so start with homogeneous container for exactly one connection
    conn = allocate< Connector< 1, ConnectionT > >( c );

    // there is only one connection, so either it is primary or secondary
    conn = pack_pointer( conn, is_primary_, !is_primary_ );
  }
  else
  {
    // case 1 or case 2

    // Already existing pointers of type ConnectorBase contain (in their two lowest bits) the
    // information if *conn has primary and/or secondary connections. Before the pointer can
    // be used as a valid pointer this information needs to be read and the original pointer
    // needs to be restored by calling validate_pointer( conn ).
    bool b_has_primary = has_primary( conn );
    bool b_has_secondary = has_secondary( conn );

    conn = validate_pointer( conn );
    // from here on we can use conn as a valid pointer

    // the following line will throw an exception, if it does not work
    c.check_connection( src, tgt, receptor_type, conn->get_t_lastspike(), get_common_properties() );

    if ( conn->homogeneous_model() ) //  there is already a homogeneous entry
    {
      if ( conn->get_syn_id() == syn_id ) // case 1: connector for this syn_id
      {
        // we can safely static cast, because we checked syn_id == syn_id(connectionT)
        vector_like< ConnectionT >* vc = static_cast< vector_like< ConnectionT >* >( conn );

        // we do not need to change the flags is_primary or is_secondary, because the new synapse is
        // of the same type as the existing ones
        conn = pack_pointer( &vc->push_back( c ), b_has_primary, b_has_secondary );
      }
      else
      {
        // syn_id is different from the one stored in the homogeneous connector
        // we need to create a heterogeneous connector now and insert the existing
        // homogeneous connector and a new homogeneous connector for the new syn_id
        HetConnector* hc = allocate< HetConnector >();

        // add existing connector
        // we read out the primary/secondary property of the existing connector conn above
        hc->add_connector( b_has_primary, conn );

        // create hom connector for new synid
        vector_like< ConnectionT >* vc = allocate< Connector< 1, ConnectionT > >( c );

        // append new homogeneous connector to heterogeneous connector
        hc->add_connector( is_primary_, vc );

        // make entry in connections_[sgid] point to new heterogeneous connector
        // the existing connections had b_has_primary or b_has_secondary,
        // our new connection is_primary
        conn =
          pack_pointer( hc, b_has_primary || is_primary_, b_has_secondary || ( !is_primary_ ) );
      }
    }
    else // case 2: the entry is heterogeneous, need to search for syn_id
    {
      // go through all entries and search for correct syn_id
      // if not found create new entry for this syn_id
      HetConnector* hc = static_cast< HetConnector* >( conn );
      bool found = false;
      for ( size_t i = 0; i < hc->size() && !found; i++ )
      {
        // need to cast to vector_like to access syn_id
        if ( ( *hc )[ i ]->get_syn_id() == syn_id ) // there is already an entry for this type
        {
          // here we know that the type is vector_like<connectionT>, because syn_id agrees
          // so we can safely static cast
          vector_like< ConnectionT >* vc =
            static_cast< vector_like< ConnectionT >* >( ( *hc )[ i ] );
          ( *hc )[ i ] = &vc->push_back( c );
          found = true;
        }
      }            // of for
      if ( found ) // we need to create a new entry for this type of connection
        conn = pack_pointer( hc, b_has_primary, b_has_secondary );
      else
      {
        vector_like< ConnectionT >* vc = allocate< Connector< 1, ConnectionT > >( c );

        hc->add_connector( is_primary_, vc );

        conn =
          pack_pointer( hc, b_has_primary || is_primary_, b_has_secondary || ( !is_primary_ ) );
      }
    }
  }

  num_connections_++;

  return conn;
}

/**
 * Delete a connection of a given type directed to a defined target Node
 * @param tgt Target node
 * @param target_thread Thread of the target
 * @param conn Connector Base from where the connection will be deleted
 * @param syn_id Synapse type
 * @return A new Connector, equal to the original but with an erased
 * connection to the defined target.
 */
template < typename ConnectionT >
ConnectorBase*
GenericConnectorModel< ConnectionT >::delete_connection( Node& tgt,
  size_t target_thread,
  ConnectorBase* conn,
  synindex syn_id )
{
  assert( conn != 0 ); // we should not delete not existing synapses
  bool found = false;
  vector_like< ConnectionT >* vc;

  bool b_has_primary = has_primary( conn );
  bool b_has_secondary = has_secondary( conn );

  conn = validate_pointer( conn );
  // from here on we can use conn as a valid pointer

  if ( conn->homogeneous_model() )
  {
    assert( conn->get_syn_id() == syn_id );
    vc = static_cast< vector_like< ConnectionT >* >( conn );
    // delete the first Connection corresponding to the target
    for ( size_t i = 0; i < vc->size(); i++ )
    {
      ConnectionT* connection = &vc->at( i );
      if ( connection->get_target( target_thread )->get_gid() == tgt.get_gid() )
      {
        if ( vc->get_num_connections() > 1 )
          conn = &vc->erase( i );
        else
        {
          delete vc;
          conn = 0;
        }
        if ( conn != 0 )
          conn = pack_pointer( conn, is_primary_, !is_primary_ );
        found = true;
        break;
      }
    }
  }
  else
  {
    // heterogeneous case
    // go through all entries and search for correct syn_id
    // if not found create new entry for this syn_id
    HetConnector* hc = static_cast< HetConnector* >( conn );

    for ( size_t i = 0; i < hc->size() && !found; i++ )
    {
      // need to cast to vector_like to access syn_id
      if ( ( *hc )[ i ]->get_syn_id() == syn_id ) // there is already an entry for this type
      {
        // here we know that the type is vector_like<connectionT>, because syn_id agrees
        // so we can safely static cast
        vector_like< ConnectionT >* vc = static_cast< vector_like< ConnectionT >* >( ( *hc )[ i ] );
        // Find and delete the first Connection corresponding to the target
        for ( size_t j = 0; j < vc->size(); j++ )
        {
          ConnectionT* connection = &vc->at( j );
          if ( connection->get_target( target_thread )->get_gid() == tgt.get_gid() )
          {
            // Get rid of the ConnectionBase for this type of synapse if there is only this element
            // left
            if ( vc->size() == 1 )
            {
              ( *hc ).erase( ( *hc ).begin() + i );
              // Test if the homogeneous vector of connections went back to only 1 type of
              // synapse... then go back to the simple vector_like case.
              if ( hc->size() == 1 )
              {
                conn = static_cast< vector_like< ConnectionT >* >( ( *hc )[ 0 ] );
                conn = pack_pointer( conn, b_has_primary, b_has_secondary );
              }
              else
              {
                conn = pack_pointer( hc, b_has_primary, b_has_secondary );
              }
            } // Otherwise, just remove the desired connection
            else
            {
              ( *hc )[ i ] = &vc->erase( j );
              conn = pack_pointer( hc, b_has_primary, b_has_secondary );
            }
            found = true;
            break;
          }
        }
      }
    }
  }
  assert( found );
  num_connections_--;
  return conn;
}


/////////////////////////////////////////////////////////////////////////////////
// Convenient versions of template functions for registering new synapse types //
// by modules                                                                  //
/////////////////////////////////////////////////////////////////////////////////

/**
 * Register a synape with default Connector and without any common properties.
 */
template < class ConnectionT >
void
register_connection_model( Network& net, const std::string& name )
{
  net.register_synapse_prototype( new GenericConnectorModel< ConnectionT >(
    net, name, /*is_primary=*/true, /*has_delay=*/true ) );
  if ( not ends_with( name, "_hpc" ) )
  {
    net.register_synapse_prototype( new GenericConnectorModel< ConnectionLabel< ConnectionT > >(
      net, name + "_lbl", /*is_primary=*/true, /*has_delay=*/true ) );
  }
}

/**
 * Register a synape with default Connector and without any common properties.
 */
template < class ConnectionT >
void
register_secondary_connection_model( Network& net, const std::string& name, bool has_delay = true )
{
  ConnectorModel* cm = new GenericSecondaryConnectorModel< ConnectionT >( net, name, has_delay );

  synindex synid = net.register_secondary_synapse_prototype( cm );

  ConnectionT::EventType::set_syn_id( synid );

  // create labeled secondary event connection model
  cm = new GenericSecondaryConnectorModel< ConnectionLabel< ConnectionT > >(
    net, name + "_lbl", has_delay );

  synid = net.register_secondary_synapse_prototype( cm );

  ConnectionT::EventType::set_syn_id( synid );
}

} // namespace nest

#endif
