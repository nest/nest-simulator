/*
 *  tsodyks_connection_hom.cpp
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

#include "tsodyks_connection_hom.h"

// Includes from nestkernel:
#include "connector_model.h"

namespace nest
{

//
// Implementation of class TsodyksHomCommonProperties.
//

TsodyksHomCommonProperties::TsodyksHomCommonProperties()
  : CommonPropertiesHomW()
  , tau_psc_( 3.0 )
  , tau_fac_( 0.0 )
  , tau_rec_( 800.0 )
  , U_( 0.5 )
{
}

void
TsodyksHomCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonPropertiesHomW::get_status( d );

  def< double >( d, names::U, U_ );
  def< double >( d, names::tau_psc, tau_psc_ );
  def< double >( d, names::tau_rec, tau_rec_ );
  def< double >( d, names::tau_fac, tau_fac_ );
}

void
TsodyksHomCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonPropertiesHomW::set_status( d, cm );

  updateValue< double >( d, names::U, U_ );
  if ( U_ > 1.0 || U_ < 0.0 )
  {
    throw BadProperty( "U must be in [0,1]." );
  }

  updateValue< double >( d, names::tau_psc, tau_psc_ );
  if ( tau_psc_ <= 0.0 )
  {
    throw BadProperty( "tau_psc must be > 0." );
  }

  updateValue< double >( d, names::tau_rec, tau_rec_ );
  if ( tau_rec_ <= 0.0 )
  {
    throw BadProperty( "tau_rec must be > 0." );
  }

  updateValue< double >( d, names::tau_fac, tau_fac_ );
  if ( tau_fac_ < 0.0 )
  {
    throw BadProperty( "tau_fac must be >= 0." );
  }
}

} // of namespace nest
