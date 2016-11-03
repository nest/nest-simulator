/*
 *  stdp_pl_connection_hom.cpp
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

#include "stdp_pl_connection_hom.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"

// Includes from sli:
#include "dictdatum.h"


namespace nest
{

//
// Implementation of class STDPPLHomCommonProperties.
//

STDPPLHomCommonProperties::STDPPLHomCommonProperties()
  : CommonSynapseProperties()
  , tau_plus_( 20.0 )
  , lambda_( 0.1 )
  , alpha_( 1.0 )
  , mu_( 0.4 )
{
}

void
STDPPLHomCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double >( d, "tau_plus", tau_plus_ );
  def< double >( d, "lambda", lambda_ );
  def< double >( d, "alpha", alpha_ );
  def< double >( d, "mu", mu_ );
}

void
STDPPLHomCommonProperties::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  updateValue< double >( d, "tau_plus", tau_plus_ );
  updateValue< double >( d, "lambda", lambda_ );
  updateValue< double >( d, "alpha", alpha_ );
  updateValue< double >( d, "mu", mu_ );
}

} // of namespace nest
