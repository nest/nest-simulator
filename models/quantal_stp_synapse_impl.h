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
quantal_stp_synapse< targetidentifierT >::get_status( Dictionary& d ) const
{
  ConnectionBase::get_status( d );
  d[ names::weight ] = weight_;
  d[ names::dU ] = U_;
  d[ names::u ] = u_;
  d[ names::tau_rec ] = tau_rec_;
  d[ names::tau_fac ] = tau_fac_;
  d[ names::n ] = n_;
  d[ names::a ] = a_;
}


template < typename targetidentifierT >
void
quantal_stp_synapse< targetidentifierT >::set_status( const Dictionary& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );

  d.update_value( names::weight, weight_ );

  d.update_value( names::dU, U_ );
  d.update_value( names::u, u_ );
  d.update_value( names::tau_rec, tau_rec_ );
  d.update_value( names::tau_fac, tau_fac_ );
  d.update_integer_value( names::n, n_ );
  d.update_integer_value( names::a, a_ );

  if ( U_ > 1.0 or U_ < 0.0 )
  {
    throw BadProperty( "'U' must be in [0,1]." );
  }

  if ( u_ > 1.0 or u_ < 0.0 )
  {
    throw BadProperty( "'u' must be in [0,1]." );
  }

  if ( tau_rec_ <= 0.0 )
  {
    throw BadProperty( "'tau_rec' must be > 0." );
  }

  if ( tau_fac_ < 0.0 )
  {
    throw BadProperty( "'tau_fac' must be >= 0." );
  }
}

} // of namespace nest

#endif // #ifndef QUANTAL_STP_SYNAPSE_IMPL_H
