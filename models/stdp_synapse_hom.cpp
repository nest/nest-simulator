/*
 *  stdp_synapse_hom.cpp
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

#include "stdp_synapse_hom.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "nest_impl.h"


void
nest::register_stdp_synapse_hom( const std::string& name )
{
  register_connection_model< stdp_synapse_hom >( name );
}

namespace nest
{

STDPHomCommonProperties::STDPHomCommonProperties()
  : CommonSynapseProperties()
  , tau_plus_( 20.0 )
  , lambda_( 0.01 )
  , alpha_( 1.0 )
  , mu_plus_( 1.0 )
  , mu_minus_( 1.0 )
  , Wmax_( 100.0 )
{
}

void
STDPHomCommonProperties::get_status( Dictionary& d ) const
{
  CommonSynapseProperties::get_status( d );

  d[ names::tau_plus ] = tau_plus_;
  d[ names::lambda ] = lambda_;
  d[ names::alpha ] = alpha_;
  d[ names::mu_plus ] = mu_plus_;
  d[ names::mu_minus ] = mu_minus_;
  d[ names::Wmax ] = Wmax_;
}

void
STDPHomCommonProperties::set_status( const Dictionary& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  d.update_value( names::tau_plus, tau_plus_ );
  d.update_value( names::lambda, lambda_ );
  d.update_value( names::alpha, alpha_ );
  d.update_value( names::mu_plus, mu_plus_ );
  d.update_value( names::mu_minus, mu_minus_ );
  d.update_value( names::Wmax, Wmax_ );
}

} // of namespace nest
