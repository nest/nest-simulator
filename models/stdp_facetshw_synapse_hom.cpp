/*
 *  stdp_facetshw_synapse_hom.cpp
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

#include "stdp_facetshw_synapse_hom.h"
#include "stdp_facetshw_synapse_hom_impl.h"

// Includes from nestkernel:
#include "nest_impl.h"

void
nest::register_stdp_facetshw_synapse_hom( const std::string& name )
{
  register_connection_model< stdp_facetshw_synapse_hom >( name );
}
