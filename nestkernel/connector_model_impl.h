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

#include "connector_model.h"

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "connector_base.h"
#include "delay_checker.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "nest_timeconverter.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

// standard implementation to obtain the default delay, assuming that it
// is located in GenericConnectorModel::default_connection
// synapse types with homogeneous delays must provide a specialization
// that returns the default delay from CommonProperties (or from  else where)
// template<typename ConnectionT>
// double get_default_delay(const GenericConnectorModel<ConnectionT> &cm)
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
  // calibrate the delay of the default properties here
  default_connection_.calibrate( tc );

  // Calibrate will be called after a change in resolution, when there are no
  // network elements present.

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
  ( *d )[ names::synapse_model ] = LiteralDatum( name_ );
  ( *d )[ names::requires_symmetric ] = requires_symmetric_;
  ( *d )[ names::has_delay ] = has_delay_;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::set_status( const DictionaryDatum& d )
{
  updateValue< long >( d, names::receptor_type, receptor_type_ );
#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue< long >( d, names::music_channel, receptor_type_ );
#endif

  // If the parameter dict d contains /delay, this should set the delay
  // on the default connection, but not affect the actual min/max_delay
  // until a connection with that default delay is created. Since the
  // set_status calls on common properties and default connection may
  // modify min/max delay, we need to freeze the min/max_delay checking.

  kernel().connection_manager.get_delay_checker().freeze_delay_update();

  cp_.set_status( d, *this );
  default_connection_.set_status( d, *this );

  kernel().connection_manager.get_delay_checker().enable_delay_update();

  // we've possibly just got a new default delay. So enforce checking next time
  // it is used
  default_delay_needs_check_ = true;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::used_default_delay()
{
  // if not used before, check now. Solves bug #138, MH 08-01-08
  // replaces whole delay checking for the default delay, see bug #217
  // MH 08-04-24
  // get_default_delay_ must be overridden by derived class to return the
  // correct default delay (either from commonprops or default connection)
  if ( default_delay_needs_check_ )
  {
    try
    {
      if ( has_delay_ )
      {
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( default_connection_.get_delay() );
      }
      // Let connections without delay contribute to the delay extrema with
      // wfr_comm_interval. For those connections the min_delay is important
      // as it determines the length of the global communication interval.
      // The call to assert_valid_delay_ms needs to happen only once
      // (either here or in add_connection()) when the first connection
      // without delay is created.
      else
      {
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms(
          kernel().simulation_manager.get_wfr_comm_interval() );
      }
    }
    catch ( BadDelay& e )
    {
      throw BadDelay( default_connection_.get_delay(),
        String::compose( "Default delay of '%1' must be between min_delay %2 "
                         "and max_delay %3.",
                        get_name(),
                        Time::delay_steps_to_ms( kernel().connection_manager.get_min_delay() ),
                        Time::delay_steps_to_ms( kernel().connection_manager.get_max_delay() ) ) );
    }
    default_delay_needs_check_ = false;
  }
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::set_syn_id( synindex syn_id )
{
  default_connection_.set_syn_id( syn_id );
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::add_connection( Node& src,
  Node& tgt,
  std::vector< ConnectorBase* >& thread_local_connectors,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double delay,
  const double weight )
{
  if ( not numerics::is_nan( delay ) )
  {
    if ( has_delay_ )
    {
      kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( delay );
    }

    if ( p->known( names::delay ) )
    {
      throw BadParameter(
        "Parameter dictionary must not contain delay if delay is given "
        "explicitly." );
    }
  }
  else
  {
    // check delay
    double delay = 0.0;

    if ( updateValue< double >( p, names::delay, delay ) )
    {
      if ( has_delay_ )
      {
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( delay );
      }
    }
    else
    {
      used_default_delay();
    }
  }

  // create a new instance of the default connection
  ConnectionT connection = ConnectionT( default_connection_ );

  if ( not numerics::is_nan( weight ) )
  {
    connection.set_weight( weight );
  }

  if ( not numerics::is_nan( delay ) )
  {
    connection.set_delay( delay );
  }

  if ( not p->empty() )
  {
    // Reference to connector model needed here to check delay (maybe this could
    // be done one level above?).
    connection.set_status( p, *this );
  }

  // We must use a local variable here to hold the actual value of the
  // receptor type. We must not change the receptor_type_ data member, because
  // that represents the *default* value. See #921.
  rport actual_receptor_type = receptor_type_;
#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue< long >( p, names::music_channel, actual_receptor_type );
#endif
  updateValue< long >( p, names::receptor_type, actual_receptor_type );

  add_connection_( src, tgt, thread_local_connectors, syn_id, connection, actual_receptor_type );
}


template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::add_connection_( Node& src,
  Node& tgt,
  std::vector< ConnectorBase* >& thread_local_connectors,
  const synindex syn_id,
  ConnectionT& connection,
  const rport receptor_type )
{
  assert( syn_id != invalid_synindex );

  if ( thread_local_connectors[ syn_id ] == NULL )
  {
    // No homogeneous Connector with this syn_id exists, we need to create a new
    // homogeneous Connector.
    thread_local_connectors[ syn_id ] = new Connector< ConnectionT >( syn_id );
  }

  ConnectorBase* connector = thread_local_connectors[ syn_id ];
  // The following line will throw an exception, if it does not work.
  connection.check_connection( src, tgt, receptor_type, get_common_properties() );

  assert( connector != 0 );

  Connector< ConnectionT >* vc = static_cast< Connector< ConnectionT >* >( connector );
  vc->push_back( connection );
}

} // namespace nest

#endif
