/*
 *  topology.cpp
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

#include "topology.h"

// C++ includes:
#include <ostream>
#include <string>

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest.h"
#include "node.h"

// Includes from sli:
#include "sliexceptions.h"

// Includes from topology:
#include "grid_layer.h"
#include "layer.h"
#include "topologymodule.h" // LayerException, TopologyModule::create_parameter,
                            // TopologyModule::create_mask


namespace nest
{
index
create_layer( const DictionaryDatum& layer_dict )
{
  layer_dict->clear_access_flags();

  index layernode = AbstractLayer::create_layer( layer_dict );

  ALL_ENTRIES_ACCESSED(
    *layer_dict, "topology::CreateLayer", "Unread dictionary entries: " );

  return layernode;
}

std::vector< double >
get_position( const index node_gid )
{
  if ( not kernel().node_manager.is_local_gid( node_gid ) )
  {
    throw KernelException(
      "GetPosition is currently implemented for local nodes only." );
  }
  Node const* const node = kernel().node_manager.get_node( node_gid );

  AbstractLayer* const layer =
    dynamic_cast< AbstractLayer* >( node->get_parent() );
  if ( not layer )
  {
    throw LayerExpected();
  }

  return layer->get_position_vector( node->get_subnet_index() );
}

std::vector< double >
displacement( const std::vector< double >& point, const index node_gid )
{
  if ( not kernel().node_manager.is_local_gid( node_gid ) )
  {
    throw KernelException(
      "Displacement is currently implemented for local nodes only." );
  }
  Node const* const node = kernel().node_manager.get_node( node_gid );

  AbstractLayer* const layer =
    dynamic_cast< AbstractLayer* >( node->get_parent() );
  if ( not layer )
  {
    throw LayerExpected();
  }

  return layer->compute_displacement( point, node->get_lid() );
}

double
distance( const std::vector< double >& point, const index node_gid )
{
  if ( not kernel().node_manager.is_local_gid( node_gid ) )
  {
    throw KernelException(
      "Distance is currently implemented for local nodes only." );
  }
  Node const* const node = kernel().node_manager.get_node( node_gid );

  AbstractLayer* const layer =
    dynamic_cast< AbstractLayer* >( node->get_parent() );
  if ( not layer )
  {
    throw LayerExpected();
  }

  return layer->compute_distance( point, node->get_lid() );
}

MaskDatum
create_mask( const DictionaryDatum& mask_dict )
{
  mask_dict->clear_access_flags();

  MaskDatum datum( TopologyModule::create_mask( mask_dict ) );

  ALL_ENTRIES_ACCESSED(
    *mask_dict, "topology::CreateMask", "Unread dictionary entries: " );

  return datum;
}

BoolDatum
inside( const std::vector< double >& point, const MaskDatum& mask )
{
  return mask->inside( point );
}

MaskDatum
intersect_mask( const MaskDatum& mask1, const MaskDatum& mask2 )
{
  return mask1->intersect_mask( *mask2 );
}

MaskDatum
union_mask( const MaskDatum& mask1, const MaskDatum& mask2 )
{
  return mask1->union_mask( *mask2 );
}

MaskDatum
minus_mask( const MaskDatum& mask1, const MaskDatum& mask2 )
{
  return mask1->minus_mask( *mask2 );
}

ParameterDatum
multiply_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->multiply_parameter( *param2 );
}

ParameterDatum
divide_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->divide_parameter( *param2 );
}

ParameterDatum
add_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->add_parameter( *param2 );
}

ParameterDatum
subtract_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->subtract_parameter( *param2 );
}

ArrayDatum
get_global_children( const index gid,
  const MaskDatum& maskd,
  const std::vector< double >& anchor )
{
  AbstractLayer* layer =
    dynamic_cast< AbstractLayer* >( kernel().node_manager.get_node( gid ) );
  if ( layer == NULL )
  {
    throw LayerExpected();
  }

  std::vector< index > gids = layer->get_global_nodes( maskd, anchor, false );

  ArrayDatum result;
  result.reserve( gids.size() );
  for ( std::vector< index >::iterator it = gids.begin(); it != gids.end();
        ++it )
  {
    result.push_back( new IntegerDatum( *it ) );
  }
  return result;
}

void
connect_layers( const index source_gid,
  const index target_gid,
  const DictionaryDatum& connection_dict )
{
  kernel().connection_manager.set_have_connections_changed(true);
  
  AbstractLayer* source = dynamic_cast< AbstractLayer* >(
    kernel().node_manager.get_node( source_gid ) );
  AbstractLayer* target = dynamic_cast< AbstractLayer* >(
    kernel().node_manager.get_node( target_gid ) );

  if ( ( source == NULL ) || ( target == NULL ) )
  {
    throw LayerExpected();
  }
  connection_dict->clear_access_flags();

  ConnectionCreator connector( connection_dict );

  ALL_ENTRIES_ACCESSED(
    *connection_dict, "topology::CreateLayers", "Unread dictionary entries: " );

  source->connect( *target, connector );
}

ParameterDatum
create_parameter( const DictionaryDatum& param_dict )
{
  param_dict->clear_access_flags();

  ParameterDatum datum( TopologyModule::create_parameter( param_dict ) );

  ALL_ENTRIES_ACCESSED(
    *param_dict, "topology::CreateParameter", "Unread dictionary entries: " );

  return datum;
}

double
get_value( const std::vector< double >& point, const ParameterDatum& param )
{
  librandom::RngPtr rng = get_global_rng();
  return param->value( point, rng );
}

void
dump_layer_nodes( const index layer_gid, OstreamDatum& out )
{
  AbstractLayer const* const layer = dynamic_cast< AbstractLayer* >(
    kernel().node_manager.get_node( layer_gid ) );

  if ( layer != 0 && out->good() )
  {
    layer->dump_nodes( *out );
  }
}

void
dump_layer_connections( const Token& syn_model,
  const index layer_gid,
  OstreamDatum& out_file )
{
  std::ostream& out = *out_file;

  AbstractLayer* const layer = dynamic_cast< AbstractLayer* >(
    kernel().node_manager.get_node( layer_gid ) );
  if ( layer == NULL )
  {
    throw TypeMismatch( "any layer type", "something else" );
  }

  layer->dump_connections( out, syn_model );
}

std::vector< index >
get_element( const index layer_gid, const TokenArray array )
{
  std::vector< index > node_gids;

  switch ( array.size() )
  {
  case 2:
  {
    GridLayer< 2 >* layer = dynamic_cast< GridLayer< 2 >* >(
      kernel().node_manager.get_node( layer_gid ) );
    if ( layer == 0 )
    {
      throw TypeMismatch( "grid layer node", "something else" );
    }

    node_gids =
      layer->get_nodes( Position< 2, int >( static_cast< index >( array[ 0 ] ),
        static_cast< index >( array[ 1 ] ) ) );
  }
  break;

  case 3:
  {
    GridLayer< 3 >* layer = dynamic_cast< GridLayer< 3 >* >(
      kernel().node_manager.get_node( layer_gid ) );
    if ( layer == 0 )
    {
      throw TypeMismatch( "grid layer node", "something else" );
    }

    node_gids =
      layer->get_nodes( Position< 3, int >( static_cast< index >( array[ 0 ] ),
        static_cast< index >( array[ 1 ] ),
        static_cast< index >( array[ 2 ] ) ) );
  }
  break;

  default:
    throw TypeMismatch( "array with length 2 or 3", "something else" );
  }
  return node_gids;
}

} // namespace nest
