/*
 *  common_synapse_properties.cpp
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

#include "common_synapse_properties.h"

// Includes from nestkernel:
#include "connector_model.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * Default implementation of an empty CommonSynapseProperties object.
 */

CommonSynapseProperties::CommonSynapseProperties()
  : weight_recorder_( -1 )
{
}

CommonSynapseProperties::~CommonSynapseProperties()
{
}

void
CommonSynapseProperties::get_status( DictionaryDatum& d ) const
{
  def< long >( d, names::weight_recorder, weight_recorder_ );
}

void
CommonSynapseProperties::set_status( const DictionaryDatum& d, ConnectorModel& )
{
  updateValue< long >( d, names::weight_recorder, weight_recorder_ );
}

Node*
CommonSynapseProperties::get_node()
{
  return 0;
}

void
CommonSynapseProperties::calibrate( const TimeConverter& )
{
}

} // namespace nest
