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
#include "connection_register.h"


template < typename T, typename C >
inline T*
allocate( C c )
{
#ifdef USE_PMA
#ifdef IS_K
  T* p = new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( T ) ) ) T( c );
#else
  T* p = new ( poormansallocpool.alloc( sizeof( T ) ) ) T( c );
#endif
#else
  T* p = new T( c );
#endif
  return p;
}

template < typename T >
inline T*
allocate()
{
#ifdef USE_PMA
#ifdef IS_K
  T* p = new ( poormansallocpool[ omp_get_thread_num() ].alloc( sizeof( T ) ) ) T();
#else
  T* p = new ( poormansallocpool.alloc( sizeof( T ) ) ) T();
#endif
#else
  T* p = new T();
#endif
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

  ( *d )[ names::receptor_type ] = receptor_type_;
  ( *d )[ "synapsemodel" ] = LiteralDatum( name_ );
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

  cp_.set_status( d, *this );
  default_connection_.set_status( d, *this );

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
  if ( !std::isnan( delay ) )
    assert_valid_delay_ms( delay );

  // create a new instance of the default connection
  ConnectionT c = ConnectionT( default_connection_ );
  if ( !std::isnan( weight ) )
  {
    c.set_weight( weight );
  }
  if ( !std::isnan( delay ) )
  {
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
  if ( !std::isnan( delay ) )
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
  if ( !std::isnan( weight ) )
  {
    c.set_weight( weight );
  }
  if ( !std::isnan( delay ) )
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
  }
  else
  {
    // case 1 or case 2

    // the following line will throw an exception, if it does not work
    c.check_connection( src, tgt, receptor_type, conn->get_t_lastspike(), get_common_properties() );

    if ( conn->homogeneous_model() ) //  there is already a homogeneous entry
    {
      if ( conn->get_syn_id() == syn_id ) // case 1: connector for this syn_id
      {
        // we can safely static cast, because we checked syn_id == syn_id(connectionT)
        vector_like< ConnectionT >* vc = static_cast< vector_like< ConnectionT >* >( conn );
        conn = &vc->push_back( c );
      }
      else
      {
        // syn_id is different from the one stored in the homogeneous connector
        // we need to create a heterogeneous connector now and insert the existing
        // homogeneous connector and a new homogeneous connector for the new syn_id
        HetConnector* hc = allocate< HetConnector >();

        // add existing connector
        hc->push_back( conn );

        // create hom connector for new synid
        vector_like< ConnectionT >* vc = allocate< Connector< 1, ConnectionT > >( c );

        // append new homogeneous connector to heterogeneous connector
        hc->push_back( vc );

        // make entry in connections_[sgid] point to new heterogeneous connector
        conn = hc;
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
          // so we can savely static cast
          vector_like< ConnectionT >* vc =
            static_cast< vector_like< ConnectionT >* >( ( *hc )[ i ] );
          ( *hc )[ i ] = &vc->push_back( c );
          found = true;
        }
      }             // of for
      if ( !found ) // we need to create a new entry for this type of connection
      {
        vector_like< ConnectionT >* vc = allocate< Connector< 1, ConnectionT > >( c );

        hc->push_back( vc );
      }
    }
  }

  num_connections_++;

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
synindex
register_connection_model( const std::string& name )
{
  return Network::get_network().register_synapse_prototype(
    new GenericConnectorModel< ConnectionT >( name ) );
}

} // namespace nest

#endif
