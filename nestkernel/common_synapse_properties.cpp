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
#include "node.h"

// Includes from models:
#include "weight_recorder.h"

// Includes from sli:
#include "dictdatum.h"

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
CommonSynapseProperties::get_status( DictionaryDatum& d ) const
{
  const NodeCollectionDatum wr = NodeCollectionDatum( NodeCollection::create( weight_recorder_ ) );
  def< NodeCollectionDatum >( d, names::weight_recorder, wr );
}

void
CommonSynapseProperties::set_status( const DictionaryDatum& d, ConnectorModel& )
{
  NodeCollectionDatum wr_datum;
  if ( updateValue< NodeCollectionDatum >( d, names::weight_recorder, wr_datum ) )
  {
    if ( wr_datum->size() != 1 )
    {
      throw BadProperty( "Property weight_recorder must be a single element NodeCollection" );
    }

    const size_t tid = kernel().vp_manager.get_thread_id();
    Node* wr_node = kernel().node_manager.get_node_or_proxy( ( *wr_datum )[ 0 ], tid );
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


CommonHomAxonalDelaySynapseProperties::CommonHomAxonalDelaySynapseProperties()
  : axonal_delay_( 0. )
{
}

CommonHomAxonalDelaySynapseProperties::~CommonHomAxonalDelaySynapseProperties()
{
}

void
CommonHomAxonalDelaySynapseProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double >( d, names::axonal_delay, axonal_delay_ );
}

void
CommonHomAxonalDelaySynapseProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  double axonal_delay;
  if ( updateValue< double >( d, names::axonal_delay, axonal_delay ) )
  {
    if ( axonal_delay >= 0 )
    {
      axonal_delay_ = axonal_delay;
    }
    else
    {
      throw BadDelay( axonal_delay, "Axonal delay must be positive or 0." );
    }
  }
}

} // namespace nest
