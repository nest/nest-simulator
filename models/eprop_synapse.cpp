/*
 *  eprop_synapse.cpp
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

#include "eprop_synapse.h"

// nestkernel
#include "nest_impl.h"


namespace nest
{

void
register_eprop_synapse( const std::string& name )
{
  register_connection_model< eprop_synapse >( name );
}

EpropCommonProperties::EpropCommonProperties()
  : CommonSynapseProperties()
  , average_gradient_( false )
{
}

void
EpropCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< bool >( d, names::average_gradient, average_gradient_ );
}

void
EpropCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );
  updateValue< bool >( d, names::average_gradient, average_gradient_ );
}

} // namespace nest
