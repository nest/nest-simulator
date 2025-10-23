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
#include "enum_bitfield.h"

// Includes from nestkernel:
#include "connection.h"
#include "connector_base.h"
#include "delay_checker.h"
#include "delay_types.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "secondary_event_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

template < typename ConnectionT >
ConnectorModel*
GenericConnectorModel< ConnectionT >::clone( std::string name, synindex syn_id ) const
{
  ConnectorModel* new_cm = new GenericConnectorModel( *this, name ); // calls copy construtor
  new_cm->set_syn_id( syn_id );

  const bool is_primary = new_cm->has_property( ConnectionModelProperties::IS_PRIMARY );
  if ( not is_primary )
  {
    new_cm->get_secondary_event()->add_syn_id( syn_id );
  }

  return new_cm;
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
  ( *d )[ names::synapse_modelid ] = kernel().model_manager.get_synapse_model_id( name_ );
  ( *d )[ names::requires_symmetric ] = has_property( ConnectionModelProperties::REQUIRES_SYMMETRIC );
  ( *d )[ names::has_delay ] = has_property( ConnectionModelProperties::HAS_DELAY );
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

  // we've possibly just got a new default delay. So enforce checking next time it is used
  default_delay_needs_check_ = true;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::check_synapse_params( const DictionaryDatum& syn_spec ) const
{
  // This is called just once per Connect() call, so we need not worry much about performance.
  // We get a dictionary with synapse default values and check if any of its keys are in syn_spec.
  DictionaryDatum dummy( new Dictionary );
  cp_.get_status( dummy );

  for ( [[maybe_unused]] const auto& [ key, val ] : *syn_spec )
  {
    if ( dummy->known( key ) )
    {
      throw NotImplemented(
        String::compose( "Synapse parameter \"%1\" can only be set via SetDefaults() or CopyModel().", key ) );
    }
  }

  default_connection_.check_synapse_params( syn_spec );
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
      if ( has_property( ConnectionModelProperties::HAS_DELAY ) )
      {
        const double d = default_connection_.get_delay_ms();
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( d );
      }
      // Let connections without delay contribute to the delay extrema with
      // wfr_comm_interval. For those connections the min_delay is important
      // as it determines the length of the global communication interval.
      // The call to assert_valid_delay_ms needs to happen only once
      // (either here or in add_connection()) when the first connection
      // without delay is created.
      else
      {
        const double wfr_comm_interval = kernel().simulation_manager.get_wfr_comm_interval();
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( wfr_comm_interval );
      }
    }
    catch ( BadDelay& e )
    {
      throw BadDelay( default_connection_.get_delay_ms(),
        String::compose( "Default delay of '%1' must be between min_delay %2 and max_delay %3.",
          get_name(),
          Time::delay_steps_to_ms( kernel().connection_manager.get_min_delay() ),
          Time::delay_steps_to_ms( kernel().connection_manager.get_max_delay() ) ) );
    }
    default_delay_needs_check_ = false;
  }
}

template < typename ConnectionT >
size_t
GenericConnectorModel< ConnectionT >::get_syn_id() const
{
  return syn_id_;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::set_syn_id( synindex syn_id )
{
  syn_id_ = syn_id;
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::check_valid_default_delay_parameters( DictionaryDatum syn_params ) const
{
  if constexpr ( std::is_base_of< Connection< TargetIdentifierPtrRport, AxonalDendriticDelay >, ConnectionT >::value
    or std::is_base_of< Connection< TargetIdentifierIndex, AxonalDendriticDelay >, ConnectionT >::value )
  {
    if ( syn_params->known( names::delay ) )
    {
      throw BadParameter( "Synapse type does not support explicitly setting total transmission delay." );
    }
  }
  else
  {
    if ( syn_params->known( names::dendritic_delay ) )
    {
      throw BadParameter( "Synapse type does not support explicitly setting dendritic delay." );
    }
    if ( syn_params->known( names::axonal_delay ) )
    {
      throw BadParameter( "Synapse type does not support explicitly setting axonal delay." );
    }
  }
}

template < typename ConnectionT >
void
GenericConnectorModel< ConnectionT >::add_connection( Node& src,
  Node& tgt,
  std::vector< ConnectorBase* >& thread_local_connectors,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double delay,
  const double dendritic_delay,
  const double axonal_delay,
  const double weight )
{
  // create a new instance of the default connection
  ConnectionT connection = ConnectionT( default_connection_ );

  bool default_delay_used = true;

  if ( has_property( ConnectionModelProperties::HAS_DELAY ) )
  {
    if constexpr ( std::is_base_of< Connection< TargetIdentifierPtrRport, AxonalDendriticDelay >, ConnectionT >::value
      or std::is_base_of< Connection< TargetIdentifierIndex, AxonalDendriticDelay >, ConnectionT >::value )
    {
      if ( not numerics::is_nan( delay ) or p->known( names::delay ) )
      {
        throw BadProperty( "Setting the total transmission delay via the parameter '" + names::delay.toString()
          + "' is not allowed for synapse types which use both dendritic and axonal delays, because of ambiguity." );
      }

      if ( not numerics::is_nan( dendritic_delay ) and p->known( names::dendritic_delay ) )
      {
        throw BadParameter(
          "Parameter dictionary must not contain dendritic delay if dendritic delay is given explicitly." );
      }

      if ( not numerics::is_nan( axonal_delay ) and p->known( names::axonal_delay ) )
      {
        throw BadParameter( "Parameter dictionary must not contain axonal delay if axonal delay is given explicitly." );
      }

      double actual_dendritic_delay = dendritic_delay;
      double actual_axonal_delay = axonal_delay;
      if ( not numerics::is_nan( dendritic_delay )
        or updateValue< double >( p, names::dendritic_delay, actual_dendritic_delay ) )
      {
        connection.set_dendritic_delay_ms( actual_dendritic_delay );
      }
      if ( not numerics::is_nan( axonal_delay )
        or updateValue< double >( p, names::axonal_delay, actual_axonal_delay ) )
      {
        connection.set_axonal_delay_ms( axonal_delay );
      }
      if ( not numerics::is_nan( actual_dendritic_delay ) or not numerics::is_nan( actual_axonal_delay ) )
      {
        default_delay_used = false;
      }
    }
    else
    {
      if ( not numerics::is_nan( dendritic_delay ) or p->known( names::dendritic_delay ) )
      {
        throw BadParameter( "Synapse type does not support explicitly setting dendritic delay." );
      }

      if ( not numerics::is_nan( axonal_delay ) or p->known( names::axonal_delay ) )
      {
        throw BadParameter( "Synapse type does not support explicitly setting axonal delay." );
      }

      if ( not numerics::is_nan( delay ) and ( p->known( names::delay ) or p->known( names::dendritic_delay ) ) )
      {
        throw BadParameter( "Parameter dictionary must not contain delay if delay is given explicitly." );
      }

      double actual_delay = delay;
      if ( updateValue< double >( p, names::delay, actual_delay ) or not numerics::is_nan( delay ) )
      {
        connection.set_delay_ms( actual_delay );
        default_delay_used = false;
      }
    }
  }
  else if ( p->known( names::delay ) or p->known( names::dendritic_delay ) or p->known( names::axonal_delay )
    or not numerics::is_nan( delay ) or not numerics::is_nan( dendritic_delay )
    or not numerics::is_nan( axonal_delay ) )
  {
    throw BadProperty( "Delay specified for a connection type which doesn't use delays." );
  }

  if ( not numerics::is_nan( weight ) )
  {
    connection.set_weight( weight );
  }

  if ( not p->empty() )
  {
    // Reference to connector model needed here to check delay (maybe this could be done one level above?).
    connection.set_status( p, *this );
  }

  if ( has_property( ConnectionModelProperties::HAS_DELAY ) )
  {
    kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( connection.get_delay_ms() );
  }

  if ( default_delay_used )
  {
    used_default_delay();
  }

  // We must use a local variable here to hold the actual value of the
  // receptor type. We must not change the receptor_type_ data member, because
  // that represents the *default* value. See #921.
  size_t actual_receptor_type = receptor_type_;
#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue< long >( p, names::music_channel, actual_receptor_type );
#endif
  updateValue< long >( p, names::receptor_type, actual_receptor_type );
  assert( syn_id != invalid_synindex );

  if ( not thread_local_connectors[ syn_id ] )
  {
    // No homogeneous Connector with this syn_id exists, we need to create a new
    // homogeneous Connector.
    thread_local_connectors[ syn_id ] = new Connector< ConnectionT >( syn_id );
  }

  ConnectorBase* connector = thread_local_connectors[ syn_id ];
  // The following line will throw an exception, if it does not work.
  connection.check_connection( src, tgt, actual_receptor_type, syn_id, get_common_properties() );

  assert( connector );

  Connector< ConnectionT >* vc = static_cast< Connector< ConnectionT >* >( connector );
  vc->push_back( std::move( connection ) );
}

} // namespace nest

#endif
