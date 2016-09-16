/*
 *  stdp_connection.cpp
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

#include "stdp_connection.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
//
// Implementation of class STDPCommonProperties.
//
//
STDPCommonProperties::STDPCommonProperties()
  : CommonSynapseProperties()
  , wr_( 0 )
{
}

void
STDPCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  if ( wr_ != 0 )
    def< long >( d, "wr", wr_->get_gid() );
  else
    def< long >( d, "wr", -1 );
}

void
STDPCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  long wrgid;
  if ( updateValue< long >( d, "wr", wrgid ) )
  {
    wr_ = dynamic_cast< Node* >( kernel().node_manager.get_node( wrgid ) );
  }
}


} // of namespace nest
