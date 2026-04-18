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

#include <memory>
#include <string>

#include "dictionary.h"
#include "node.h"
#include "node_manager.h"
// Includes from models:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "node_collection.h"
#include "vp_manager.h"
#include "weight_recorder.h"

namespace nest
{

CommonSynapseProperties::CommonSynapseProperties()
  : weight_recorder_()
{
}

CommonSynapseProperties::~CommonSynapseProperties()
{
}

void
CommonSynapseProperties::get_status( Dictionary& d ) const
{
  d[ names::weight_recorder ] = NodeCollection::create( weight_recorder_ );
}

void
CommonSynapseProperties::set_status( const Dictionary& d, ConnectorModel& )
{
  NodeCollectionPTR wr_nc;
  if ( d.update_value( names::weight_recorder, wr_nc ) )
  {
    if ( wr_nc->size() != 1 )
    {
      throw BadProperty( "Property weight_recorder must be a single element NodeCollection" );
    }

    const size_t tid = kernel::manager< VPManager >.get_thread_id();
    Node* wr_node = kernel::manager< NodeManager >.get_node_or_proxy( ( *wr_nc )[ 0 ], tid );
    weight_recorder* wr = dynamic_cast< weight_recorder* >( wr_node );
    if ( not wr )
    {
      throw BadProperty( "Property weight_recorder must be set to a node of type weight_recorder" );
    }

    weight_recorder_ = wr;
  }
}

void
CommonSynapseProperties::calibrate( const TimeConverter& )
{
}

weight_recorder*
CommonSynapseProperties::get_weight_recorder() const
{
  return weight_recorder_;
}

long
CommonSynapseProperties::get_vt_node_id() const
{
  return -1;
}
}  // namespace nest
