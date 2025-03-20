/*
 *  quantal_stp_synapse_impl.h
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

#ifndef QUANTAL_STP_SYNAPSE_IMPL_H
#define QUANTAL_STP_SYNAPSE_IMPL_H

#include "quantal_stp_synapse.h"

// Includes from nestkernel:
#include "connection.h"
#include "connector_model.h"
#include "nest_names.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

template < typename targetidentifierT >
quantal_stp_synapse< targetidentifierT >::quantal_stp_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , U_( 0.5 )
  , u_( U_ )
  , tau_rec_( 800.0 )
  , tau_fac_( 0.0 )
  , n_( 1 )
  , a_( n_ )
  , t_lastspike_( -1.0 )
{
}

template < typename targetidentifierT >
void
quantal_stp_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
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
quantal_stp_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::dU, U_ );
  if ( U_ > 1.0 or U_ < 0.0 )
  {
    throw BadProperty( "'U' must be in [0,1]." );
  }

  updateValue< double >( d, names::u, u_ );
  if ( u_ > 1.0 or u_ < 0.0 )
  {
    throw BadProperty( "'u' must be in [0,1]." );
  }

  updateValue< double >( d, names::tau_rec, tau_rec_ );
  if ( tau_rec_ <= 0.0 )
  {
    throw BadProperty( "'tau_rec' must be > 0." );
  }

  updateValue< double >( d, names::tau_fac, tau_fac_ );
  if ( tau_fac_ < 0.0 )
  {
    throw BadProperty( "'tau_fac' must be >= 0." );
  }

  updateValue< long >( d, names::n, n_ );
  updateValue< long >( d, names::a, a_ );
}

} // of namespace nest

#endif // #ifndef QUANTAL_STP_SYNAPSE_IMPL_H
