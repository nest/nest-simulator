/*
 *  spatial.cpp
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

#include "spatial.h"

// C++ includes:
#include <ostream>
#include <string>

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "logging_manager.h"
#include "nest.h"
#include "nestmodule.h"
#include "node.h"


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

  ALL_ENTRIES_ACCESSED( *layer_dict, "nest::CreateLayer", "Unread dictionary entries: " );

  return layer;
}

ArrayDatum
get_position( NodeCollectionPTR layer_nc )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  ArrayDatum result;
  result.reserve( layer_nc->size() );

  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;

    if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
    {
      throw KernelException( "GetPosition is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;
    Token arr = layer->get_position_vector( lid );

    result.push_back( arr );
  }

  return result;
}

std::vector< double >
get_position( const size_t node_id )
{
  if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
  {
    throw KernelException( "GetPosition is currently implemented for local nodes only." );
  }

  NodeCollectionPTR nc = kernel::manager< NodeManager >.node_id_to_node_collection( node_id );
  NodeCollectionMetadataPTR meta = nc->get_metadata();

  if ( not meta )
  {
    // We return NaN if node_id is not spatially distributed
    std::vector< double > positions = { std::nan( "1" ), std::nan( "1" ) };
    return positions;
  }

  AbstractLayerPTR spatial_nc = get_layer( nc );
  size_t first_node_id = meta->get_first_node_id();

  auto pos_vec = spatial_nc->get_position_vector( node_id - first_node_id );

  return pos_vec;
}

ArrayDatum
displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc )
{
  ArrayDatum layer_to_positions = get_position( layer_to_nc );

  AbstractLayerPTR layer_from = get_layer( layer_from_nc );
  NodeCollectionMetadataPTR meta = layer_from_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  ArrayDatum result;

  // If layer_from has size equal to one, but layer_to do not, we want the displacement between every node
  // in layer_to against the one in layer_from. Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_nc->size() == 1 )
  {
    size_t node_id = layer_from_nc->operator[]( 0 );
    if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
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
      size_t node_id = ( *it ).node_id;
      if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
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
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  ArrayDatum result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it != layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;
    if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
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
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< double > result;

  // If layer_from has size equal to one, but layer_to do not, we want the distance between every node
  // in layer_to against the one in layer_from. Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_nc->size() == 1 )
  {
    size_t node_id = layer_from_nc->operator[]( 0 );
    if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
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
      size_t node_id = ( *it ).node_id;
      if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
      {
        throw KernelException( "Distance is currently implemented for local nodes only." );
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
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< double > result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;
    if ( not kernel::manager< NodeManager >.is_local_node_id( node_id ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
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

std::vector< double >
distance( const ArrayDatum conns )
{
  std::vector< double > result;

  size_t num_conns = conns.size();
  result.reserve( num_conns );

  for ( size_t conn_indx = 0; conn_indx < num_conns; ++conn_indx )
  {
    ConnectionDatum conn_id = getValue< ConnectionDatum >( conns.get( conn_indx ) );

    size_t src = conn_id.get_source_node_id();
    auto src_position = get_position( src );

    size_t trgt = conn_id.get_target_node_id();

    if ( not kernel::manager< NodeManager >.is_local_node_id( trgt ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
    }


    NodeCollectionPTR trgt_nc = kernel::manager< NodeManager >.node_id_to_node_collection( trgt );
    NodeCollectionMetadataPTR meta = trgt_nc->get_metadata();

    // distance is NaN if source, target is not spatially distributed
    double dist = std::nan( "1" );

    if ( meta )
    {
      AbstractLayerPTR spatial_trgt_nc = get_layer( trgt_nc );
      NodeCollectionMetadataPTR meta = trgt_nc->get_metadata();
      size_t first_trgt_node_id = meta->get_first_node_id();

      dist = spatial_trgt_nc->compute_distance( src_position, trgt - first_trgt_node_id );
    }

    result.push_back( dist );
  }
  return result;
}

MaskDatum
create_mask( const DictionaryDatum& mask_dict )
{
  mask_dict->clear_access_flags();

  MaskDatum datum( NestModule::create_mask( mask_dict ) );

  ALL_ENTRIES_ACCESSED( *mask_dict, "nest::CreateMask", "Unread dictionary entries: " );

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
  AbstractLayerPTR source = get_layer( source_nc );
  AbstractLayerPTR target = get_layer( target_nc );

  connection_dict->clear_access_flags();
  ConnectionCreator connector( connection_dict );
  ALL_ENTRIES_ACCESSED( *connection_dict, "nest::CreateLayers", "Unread dictionary entries: " );

  kernel::manager< NodeManager >.update_thread_local_node_data();

  // Set flag before calling source->connect() in case exception is thrown after some connections have been created.
  kernel::manager< ConnectionManager >.set_connections_have_changed();

  source->connect( source_nc, target, target_nc, connector );
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
    source_layer->dump_connections( *out, source_layer_nc, target_layer, syn_model );
  }
}

DictionaryDatum
get_layer_status( NodeCollectionPTR )
{
  assert( false and "not implemented" );

  return DictionaryDatum();
}

void
LayerMetadata::get_status( DictionaryDatum& d, NodeCollection const* nc ) const
{
  layer_->get_status( d, nc );
}

const AbstractLayerPTR
LayerMetadata::get_layer() const
{
  return layer_;
}

std::string
LayerMetadata::get_type() const
{
  return "spatial";
}

void
LayerMetadata::set_first_node_id( size_t node_id )
{
  first_node_id_ = node_id;
}

size_t
LayerMetadata::get_first_node_id() const
{
  return first_node_id_;
}

bool
LayerMetadata::operator==( const NodeCollectionMetadataPTR rhs ) const
{
  const auto rhs_layer_metadata = dynamic_cast< LayerMetadata* >( rhs.get() );
  if ( not rhs_layer_metadata )
  {
    return false;
  }
  // Compare status dictionaries of this layer and rhs layer
  DictionaryDatum dict( new Dictionary() );
  DictionaryDatum rhs_dict( new Dictionary() );

  // Since we do not have access to the node collection here, we
  // compare based on all metadata, irrespective of any slicing
  get_status( dict, /* nc */ nullptr );
  rhs_layer_metadata->get_status( rhs_dict, /* nc */ nullptr );
  return *dict == *rhs_dict;
}
} // namespace nest
