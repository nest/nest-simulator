/*
 *  nest_impl.h
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


// Includes from nestkernel:
#include "connector_model_impl.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"

namespace nest
{

template < template < typename > class ConnectorModelT >
void
register_connection_model( const std::string& name )
{
  kernel().model_manager.register_connection_model< ConnectorModelT >( name );
}

template < typename NodeModelT >
void
register_node_model( const std::string& name, std::string deprecation_info )
{
  kernel().model_manager.register_node_model< NodeModelT >( name, deprecation_info );
}
}
