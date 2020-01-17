/*
 *  quantal_stp_connection_impl.h
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

#ifndef QUANTAL_STP_CONNECTION_IMPL_H
#define QUANTAL_STP_CONNECTION_IMPL_H

#include "quantal_stp_connection.h"

// Includes from nestkernel:
#include "connection.h"
#include "connector_model.h"
#include "nest_names.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{


/* Polymorphic version of update_value.
 * This code will take either an int or a double and convert is to an
 * int.
 */
bool
update_value_int( const DictionaryDatum& d, Name propname, int& prop )
{
  if ( d->known( propname ) )
  {
    Datum* dat = ( *d )[ propname ].datum();
    IntegerDatum* intdat = dynamic_cast< IntegerDatum* >( dat );
    if ( intdat != 0 )
    {
      prop = static_cast< int >( intdat->get() );
      return true;
    }
    DoubleDatum* doubledat = dynamic_cast< DoubleDatum* >( dat );
    if ( doubledat != 0 )
    {
      prop = static_cast< int >( doubledat->get() );
      return true;
    }
    else
    {
      throw TypeMismatch();
    }
  }

  return false;
}

template < typename targetidentifierT >
Quantal_StpConnection< targetidentifierT >::Quantal_StpConnection()
  : ConnectionBase()
  , weight_( 1.0 )
  , U_( 0.5 )
  , u_( U_ )
  , tau_rec_( 800.0 )
  , tau_fac_( 10.0 )
  , n_( 1 )
  , a_( n_ )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
Quantal_StpConnection< targetidentifierT >::Quantal_StpConnection( const Quantal_StpConnection& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , U_( rhs.U_ )
  , u_( rhs.u_ )
  , tau_rec_( rhs.tau_rec_ )
  , tau_fac_( rhs.tau_fac_ )
  , n_( rhs.n_ )
  , a_( rhs.a_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}


template < typename targetidentifierT >
void
Quantal_StpConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::dU, U_ );
  def< double >( d, names::u, u_ );
  def< double >( d, names::tau_rec, tau_rec_ );
  def< double >( d, names::tau_fac, tau_fac_ );
  def< int >( d, names::n, n_ );
  def< int >( d, names::a, a_ );
}


template < typename targetidentifierT >
void
Quantal_StpConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::dU, U_ );
  updateValue< double >( d, names::u, u_ );
  updateValue< double >( d, names::tau_rec, tau_rec_ );
  updateValue< double >( d, names::tau_fac, tau_fac_ );
  update_value_int( d, names::n, n_ );
  update_value_int( d, names::a, a_ );
}

template < typename targetidentifierT >
void
Quantal_StpConnection< targetidentifierT >::check_synapse_params( const DictionaryDatum& syn_spec ) const
{
  // Throw error if n or a are set in quantal_stp_synapse, Connect cannot handle
  // them since they are integers.
  if ( syn_spec->known( names::n ) )
  {
    throw NotImplemented(
      "Connect doesn't support the setting of parameter "
      "n in quantal_stp_synapse. Use SetDefaults() or CopyModel()." );
  }
  if ( syn_spec->known( names::a ) )
  {
    throw NotImplemented(
      "Connect doesn't support the setting of parameter "
      "a in quantal_stp_synapse. Use SetDefaults() or CopyModel()." );
  }
}

} // of namespace nest

#endif // #ifndef QUANTAL_STP_CONNECTION_IMPL_H
