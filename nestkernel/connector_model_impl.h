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
#include "connector_base.h"
#include "delay_checker.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "secondary_event_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

// standard implementation to obtain the default delay, assuming that it
// is located in GenericConnectorModel::default_connection
// synapse types with homogeneous delays must provide a specialization
// that returns the default delay from CommonProperties (or from else where)
// template<typename ConnectionT>
// double get_default_delay(const GenericConnectorModel<ConnectionT> &cm)
// {
//   //std::cout << "standard implementation of get_default_delay" << std::endl;
//   return cm.get_default_delay();
// }

// template<typename ConnectionT>
// SynIdDelay & syn_id_delay(const GenericConnectorModel<ConnectionT> &cm)
// {
//   return cm.get_default_connection().get_syn_id_delay();
// }

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
  Time t = tc.from_old_steps( default_delay_ );
  default_delay_ = t.get_steps();
  t = tc.from_old_steps( default_axonal_delay_ );
  default_axonal_delay_ = t.get_steps();

  if ( default_delay_ + default_axonal_delay_ == 0 )
  {
    default_delay_ = Time::delay_ms_to_steps( 1. );
  }

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
  ( *d )[ names::delay ] = Time::delay_steps_to_ms( default_delay_ );
  ( *d )[ names::axonal_delay ] = Time::delay_steps_to_ms( default_axonal_delay_ );

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

  double new_default_delay = Time::delay_steps_to_ms( default_delay_ );
  double new_default_axonal_delay = Time::delay_steps_to_ms( default_axonal_delay_ );
  updateValue< double >( d, names::delay, new_default_delay );
  updateValue< double >( d, names::axonal_delay, new_default_axonal_delay );
  if ( new_default_delay != Time::delay_steps_to_ms( default_delay_ )
    or new_default_axonal_delay != Time::delay_steps_to_ms( default_axonal_delay_ ) )
  {
    // kernel().connection_manager.get_delay_checker().assert_valid_delay_ms(
    //   new_default_delay + new_default_axonal_delay );
    default_delay_ = Time::delay_ms_to_steps( new_default_delay );
    default_axonal_delay_ = Time::delay_ms_to_steps( new_default_axonal_delay );
  }

  default_connection_.set_status( d, *this );

  kernel().connection_manager.get_delay_checker().enable_delay_update();

  // we've possibly just got a new default delay. So enforce checking next time
  //  // it is used
  //  default_delay_needs_check_ = true;
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
        kernel().connection_manager.get_delay_checker().assert_valid_delay_ms(
          Time::delay_steps_to_ms( default_delay_ + default_axonal_delay_ ) );
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
      throw BadDelay( Time::delay_steps_to_ms( default_delay_ + default_axonal_delay_ ),
        String::compose( "Default delay of '%1' (total: %2, axonal: %3, dendritic: %4) must be between min_delay %5 "
                         "and max_delay %6.",
          get_name(),
          Time::delay_steps_to_ms( default_delay_ + default_axonal_delay_ ),
          Time::delay_steps_to_ms( default_axonal_delay_ ),
          Time::delay_steps_to_ms( default_delay_ ),
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
  const double axonal_delay,
  const double weight )
{
  bool user_defined_delay = false;
  long actual_dendritic_delay;
  // check if dendritic delay was not provided explicitly
  if ( numerics::is_nan( delay ) )
  {
    double delay_temp;
    if ( updateValue< double >( p, names::delay, delay_temp ) )
    {
      user_defined_delay = true;
      actual_dendritic_delay = Time::delay_ms_to_steps( delay_temp );
    }
    else
    {
      actual_dendritic_delay = default_delay_;
    }
  }
  else // dendritic delay provided
  {
    user_defined_delay = true;
    actual_dendritic_delay = Time::delay_ms_to_steps( delay );
    if ( p->known( names::delay ) )
    {
      throw BadParameter( "Parameter dictionary must not contain delay if delay is given explicitly." );
    }
  }
  long actual_axonal_delay;
  // check if axonal delay was not provided explicitly
  if ( numerics::is_nan( axonal_delay ) )
  {
    double delay_temp;
    if ( updateValue< double >( p, names::axonal_delay, delay_temp ) )
    {
      user_defined_delay = true;
      actual_axonal_delay = Time::delay_ms_to_steps( delay_temp );
    }
    else
    {
      actual_axonal_delay = default_axonal_delay_;
    }
  }
  else // axonal delay provided
  {
    user_defined_delay = true;
    actual_axonal_delay = Time::delay_ms_to_steps( axonal_delay );
    if ( p->known( names::axonal_delay ) )
    {
      throw BadParameter( "Parameter dictionary must not contain axonal delay if axonal delay is given explicitly." );
    }
  }

  // create a new instance of the default connection
  ConnectionT connection = ConnectionT( default_connection_ );

  if ( not numerics::is_nan( weight ) )
  {
    connection.set_weight( weight );
  }

  if ( not p->empty() )
  {
    // Reference to connector model needed here to check delay (maybe this could
    // be done one level above?).
    connection.set_status( p, *this );
  }

  if ( not user_defined_delay )
  {
    used_default_delay();
  }

  if ( user_defined_delay or has_property( ConnectionModelProperties::HAS_DELAY ) )
  {
    KernelManager::get_kernel_manager().connection_manager.get_delay_checker().assert_valid_delay_ms(
      Time::delay_steps_to_ms( actual_dendritic_delay + actual_axonal_delay ) );
    connection.set_dendritic_delay_steps( actual_dendritic_delay );
    if ( actual_axonal_delay != 0 )
    {
      connection.set_axonal_delay( Time::delay_steps_to_ms( actual_axonal_delay ) );
    }
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
  connection.check_connection(
    src, tgt, actual_receptor_type, actual_dendritic_delay, actual_axonal_delay, get_common_properties() );

  assert( connector );

  Connector< ConnectionT >* vc = static_cast< Connector< ConnectionT >* >( connector );
  vc->push_back( connection );
}

} // namespace nest

#endif
