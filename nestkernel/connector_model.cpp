/*
 *  connector_model.cpp
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

#include "connector_model.h"
#include "model_manager.h"

namespace nest
{

ConnectorModel::ConnectorModel( const std::string name, const ConnectionModelProperties& properties )
  : name_( name )
  , default_delay_needs_check_( true )
  , properties_( properties )
{
}

ConnectorModel::ConnectorModel( const ConnectorModel& cm, const std::string name )
  : name_( name )
  , default_delay_needs_check_( true )
  , properties_( cm.properties_ )
{
}

size_t
ConnectorModel::get_synapse_model_id( const std::string& name )
{
  return kernel::manager< ModelManager >.get_synapse_model_id( name );
}

} // namespace nest
