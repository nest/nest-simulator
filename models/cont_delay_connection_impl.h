/*
 *  cont_delay_connection_impl.h
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

#ifndef CONT_DELAY_CONNECTION_IMPL_H
#define CONT_DELAY_CONNECTION_IMPL_H

#include "cont_delay_connection.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

template < typename targetidentifierT >
ContDelayConnection< targetidentifierT >::ContDelayConnection()
  : ConnectionBase()
  , weight_( 1.0 )
  , delay_offset_( 0.0 )
{
}

template < typename targetidentifierT >
ContDelayConnection< targetidentifierT >::ContDelayConnection( const ContDelayConnection& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , delay_offset_( rhs.delay_offset_ )
{
}

template < typename targetidentifierT >
void
ContDelayConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );

  def< double >( d, names::weight, weight_ );
  def< double >( d, names::delay, Time( Time::step( get_delay_steps() ) ).get_ms() - delay_offset_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
ContDelayConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );

  updateValue< double >( d, names::weight, weight_ );

  // set delay if mentioned
  double delay;

  if ( updateValue< double >( d, names::delay, delay ) )
  {

    const double h = Time::get_resolution().get_ms();

    double int_delay;
    const double frac_delay = std::modf( delay / h, &int_delay );

    if ( frac_delay == 0 )
    {
      kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( delay );
      set_delay_steps( Time::delay_ms_to_steps( delay ) );
      delay_offset_ = 0.0;
    }
    else
    {
      const long lowerbound = static_cast< long >( int_delay );
      kernel().connection_manager.get_delay_checker().assert_two_valid_delays_steps( lowerbound, lowerbound + 1 );
      set_delay_steps( lowerbound + 1 );
      delay_offset_ = h * ( 1.0 - frac_delay );
    }
  }
}

template < typename targetidentifierT >
void
ContDelayConnection< targetidentifierT >::check_synapse_params( const DictionaryDatum& syn_spec ) const
{
  if ( syn_spec->known( names::delay ) )
  {
    LOG( M_WARNING,
      "Connect",
      "The delay will be rounded to the next multiple of the time step. "
      "To use a more precise time delay it needs to be defined within "
      "the synapse, e.g. with CopyModel()." );
  }
}

} // of namespace nest

#endif // #ifndef CONT_DELAY_CONNECTION_IMPL_H
