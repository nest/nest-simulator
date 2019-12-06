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

LayerMetadata::LayerMetadata( AbstractLayerPTR layer )
  : NodeCollectionMetadata()
  , layer_( layer )
  , first_node_id_( 0 )
{
}


AbstractLayerPTR
get_layer( NodeCollectionPTR nc )
{
  NodeCollectionMetadataPTR meta = nc->get_metadata();

  LayerMetadata const* const layer_meta = dynamic_cast< LayerMetadata const* >( meta.get() );
  if ( not layer_meta )
  {
    throw LayerExpected();
  }
  return layer_meta->get_layer();
}

NodeCollectionPTR
create_layer( const DictionaryDatum& layer_dict )
{
  layer_dict->clear_access_flags();

  NodeCollectionPTR layer = AbstractLayer::create_layer( layer_dict );

  ALL_ENTRIES_ACCESSED( *layer_dict, "topology::CreateLayer", "Unread dictionary entries: " );

  return layer;
}

ArrayDatum
get_position( NodeCollectionPTR layer_nc )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  index first_node_id = meta->get_first_node_id();

  ArrayDatum result;
  result.reserve( layer_nc->size() );

  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    index node_id = ( *it ).node_id;

    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "GetPosition is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;
    Token arr = layer->get_position_vector( lid );

    result.push_back( arr );
  }

  return result;
}

ArrayDatum
displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc )
{
  ArrayDatum layer_to_positions = get_position( layer_to_nc );

  AbstractLayerPTR layer_from = get_layer( layer_from_nc );
  NodeCollectionMetadataPTR meta = layer_from_nc->get_metadata();
  index first_node_id = meta->get_first_node_id();

  int counter = 0;
  ArrayDatum result;

  // If layer_from has size equal to one, but layer_to do not, we want the
  // displacement between every node in layer_to against the one in layer_from.
  // Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_nc->size() == 1 )
  {
    index node_id = layer_from_nc->operator[]( 0 );
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }
    const long lid = node_id - first_node_id;

    // If layer_from has size 1, we need to iterate over the layer_to positions
    for ( Token const* it = layer_to_positions.begin(); it != layer_to_positions.end(); ++it )
    {
      std::vector< double > pos = getValue< std::vector< double > >( *it );
      Token disp = layer_from->compute_displacement( pos, lid );
      result.push_back( disp );
    }
  }
  else
  {
    for ( NodeCollection::const_iterator it = layer_from_nc->begin(); it < layer_from_nc->end(); ++it )
    {
      index node_id = ( *it ).node_id;
      if ( not kernel().node_manager.is_local_node_id( node_id ) )
      {
        throw KernelException( "Displacement is currently implemented for local nodes only." );
      }

      const long lid = node_id - first_node_id;

      std::vector< double > pos = getValue< std::vector< double > >( layer_to_positions[ counter ] );
      Token disp = layer_from->compute_displacement( pos, lid );
      result.push_back( disp );

      // We only iterate the layer_to positions vector if it has more than one
      // element.
      if ( layer_to_nc->size() != 1 )
      {
        ++counter;
      }
    }
  }

  return result;
}

ArrayDatum
displacement( NodeCollectionPTR layer_nc, const ArrayDatum point )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  index first_node_id = meta->get_first_node_id();

  int counter = 0;
  ArrayDatum result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it != layer_nc->end(); ++it )
  {
    index node_id = ( *it ).node_id;
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;

    std::vector< double > pos = getValue< std::vector< double > >( point[ counter ] );
    Token disp = layer->compute_displacement( pos, lid );
    result.push_back( disp );

    // We only iterate the positions vector if it has more than one
    // element.
    if ( point.size() != 1 )
    {
      ++counter;
    }
  }
  return result;
}

std::vector< double >
distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc )
{
  ArrayDatum layer_to_positions = get_position( layer_to_nc );

  AbstractLayerPTR layer_from = get_layer( layer_from_nc );
  NodeCollectionMetadataPTR meta = layer_from_nc->get_metadata();
  index first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< double > result;

  // If layer_from has size equal to one, but layer_to do not, we want the
  // distance between every node in layer_to against the one in layer_from.
  // Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_nc->size() == 1 )
  {
    index node_id = layer_from_nc->operator[]( 0 );
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }
    const long lid = node_id - first_node_id;

    // If layer_from has size 1, we need to iterate over the layer_to positions
    for ( Token const* it = layer_to_positions.begin(); it != layer_to_positions.end(); ++it )
    {
      std::vector< double > pos = getValue< std::vector< double > >( *it );
      double disp = layer_from->compute_distance( pos, lid );
      result.push_back( disp );
    }
  }
  else
  {
    for ( NodeCollection::const_iterator it = layer_from_nc->begin(); it < layer_from_nc->end(); ++it )
    {
      index node_id = ( *it ).node_id;
      if ( not kernel().node_manager.is_local_node_id( node_id ) )
      {
        throw KernelException( "Displacement is currently implemented for local nodes only." );
      }

      const long lid = node_id - first_node_id;

      std::vector< double > pos = getValue< std::vector< double > >( layer_to_positions[ counter ] );
      double disp = layer_from->compute_distance( pos, lid );
      result.push_back( disp );

      // We only iterate the layer_to positions vector if it has more than one
      // element.
      if ( layer_to_nc->size() != 1 )
      {
        ++counter;
      }
    }
  }

  return result;
}

std::vector< double >
distance( NodeCollectionPTR layer_nc, const ArrayDatum point )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  index first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< double > result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    index node_id = ( *it ).node_id;
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;

    std::vector< double > pos = getValue< std::vector< double > >( point[ counter ] );
    double disp = layer->compute_distance( pos, lid );
    result.push_back( disp );

    // We only iterate the positions vector if it has more than one
    // element.
    if ( point.size() != 1 )
    {
      ++counter;
    }
  }
  return result;
}

MaskDatum
create_mask( const DictionaryDatum& mask_dict )
{
  mask_dict->clear_access_flags();

  MaskDatum datum( TopologyModule::create_mask( mask_dict ) );

  ALL_ENTRIES_ACCESSED( *mask_dict, "topology::CreateMask", "Unread dictionary entries: " );

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

void
connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const DictionaryDatum& connection_dict )
{
  kernel().connection_manager.set_have_connections_changed( true );

  AbstractLayerPTR source = get_layer( source_nc );
  AbstractLayerPTR target = get_layer( target_nc );

  connection_dict->clear_access_flags();
  ConnectionCreator connector( connection_dict );
  ALL_ENTRIES_ACCESSED( *connection_dict, "topology::CreateLayers", "Unread dictionary entries: " );

  source->connect( target, target_nc, connector );
}

void
dump_layer_nodes( NodeCollectionPTR layer_nc, OstreamDatum& out )
{
  AbstractLayerPTR layer = get_layer( layer_nc );

  if ( out->good() )
  {
    layer->dump_nodes( *out );
  }
}

void
dump_layer_connections( const Token& syn_model,
  NodeCollectionPTR source_layer_nc,
  NodeCollectionPTR target_layer_nc,
  OstreamDatum& out )
{
  AbstractLayerPTR source_layer = get_layer( source_layer_nc );
  AbstractLayerPTR target_layer = get_layer( target_layer_nc );

  if ( out->good() )
  {
    source_layer->dump_connections( *out, target_layer, syn_model );
  }
}

DictionaryDatum
get_layer_status( NodeCollectionPTR layer_nc )
{
  assert( false && "not implemented" );

  return DictionaryDatum();
}

} // namespace nest
