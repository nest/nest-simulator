/*
 *  axonal_delay_connection.h
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

#ifndef NEST_AXONAL_DELAY_CONNECTION_H
#define NEST_AXONAL_DELAY_CONNECTION_H


// Includes from nestkernel:
#include "connection.h"
#include "exceptions.h"
#include "nest_names.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

template < typename targetidentifierT >
class AxonalDelayConnection : public Connection< targetidentifierT >
{
  typedef Connection< targetidentifierT > ConnectionBase;

  double axonal_delay_; //!< Axonal delay in ms
public:
  AxonalDelayConnection()
    : Connection< targetidentifierT >()
    , axonal_delay_( 0 )
  {
  }

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Set the proportion of the transmission delay attributed to the axon.
   */
  void
  set_dendritic_delay( const double dendritic_delay )
  {
    ConnectionBase::set_dendritic_delay( dendritic_delay );
  }

  /**
   * Get the proportion of the transmission delay attributed to the axon.
   */
  double
  get_dendritic_delay() const
  {
    return ConnectionBase::get_dendritic_delay();
  }

  /**
   * Set the proportion of the transmission delay attributed to the axon.
   */
  void set_axonal_delay( const double axonal_delay );

  /**
   * Get the proportion of the transmission delay attributed to the axon.
   */
  double get_axonal_delay() const;
};

template < typename targetidentifierT >
inline void
AxonalDelayConnection< targetidentifierT >::set_axonal_delay( const double axonal_delay )
{
  if ( axonal_delay < 0.0 ) // consistency with overall delay is checked in check_connection()
  {
    throw BadProperty( "Axonal delay should not be negative." );
  }
  axonal_delay_ = axonal_delay;
}

template < typename targetidentifierT >
inline double
AxonalDelayConnection< targetidentifierT >::get_axonal_delay() const
{
  return axonal_delay_;
}

template < typename targetidentifierT >
inline void
AxonalDelayConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );

  def< double >( d, names::axonal_delay, axonal_delay_ );
}

template < typename targetidentifierT >
inline void
AxonalDelayConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // no call to ConnectionBase::set_status, as it assumes purely dendritic delay when checking the validity of the delay
  double axonal_delay = get_axonal_delay();
  double dendritic_delay = get_dendritic_delay();
  if ( updateValue< double >( d, names::delay, dendritic_delay )
    or updateValue< double >( d, names::axonal_delay, axonal_delay ) )
  {
    kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( axonal_delay + dendritic_delay );
    set_dendritic_delay( dendritic_delay );
    set_axonal_delay( axonal_delay );
  }
}

}
#endif // NEST_AXONAL_DELAY_CONNECTION_H
