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
#include <fstream>
#include <memory>
#include <ostream>
#include <string>

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest.h"
#include "node.h"

// Includes from spatial:
#include "grid_layer.h"
#include "layer_impl.h"


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
create_layer( const Dictionary& layer_dict )
{
  layer_dict.init_access_flags();

  NodeCollectionPTR layer = AbstractLayer::create_layer( layer_dict );

  layer_dict.all_entries_accessed( "CreateLayer", "params" );

  return layer;
}

std::vector< std::vector< double > >
get_position( NodeCollectionPTR layer_nc )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  std::vector< std::vector< double > > result;
  result.reserve( layer_nc->size() );

  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;

    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "GetPosition is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;
    const auto arr = layer->get_position_vector( lid );

    result.push_back( arr );
  }

  return result;
}

std::vector< double >
get_position( const size_t node_id )
{
  if ( not kernel().node_manager.is_local_node_id( node_id ) )
  {
    throw KernelException( "GetPosition is currently implemented for local nodes only." );
  }

  NodeCollectionPTR nc = kernel().node_manager.node_id_to_node_collection( node_id );
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

std::vector< std::vector< double > >
displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc )
{
  auto layer_to_positions = get_position( layer_to_nc );

  AbstractLayerPTR layer_from = get_layer( layer_from_nc );
  NodeCollectionMetadataPTR meta = layer_from_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< std::vector< double > > result;

  // If layer_from has size equal to one, but layer_to do not, we want the displacement between every node
  // in layer_to against the one in layer_from. Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_nc->size() == 1 )
  {
    size_t node_id = layer_from_nc->operator[]( 0 );
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }
    const long lid = node_id - first_node_id;

    // If layer_from has size 1, we need to iterate over the layer_to positions
    for ( auto& pos : layer_to_positions )
    {
      result.push_back( layer_from->compute_displacement( pos, lid ) );
    }
  }
  else
  {
    for ( NodeCollection::const_iterator it = layer_from_nc->begin(); it < layer_from_nc->end(); ++it )
    {
      size_t node_id = ( *it ).node_id;
      if ( not kernel().node_manager.is_local_node_id( node_id ) )
      {
        throw KernelException( "Displacement is currently implemented for local nodes only." );
      }

      const long lid = node_id - first_node_id;
      const auto pos = layer_to_positions[ counter ];
      result.push_back( layer_from->compute_displacement( pos, lid ) );

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

std::vector< std::vector< double > >
displacement( NodeCollectionPTR layer_nc, const std::vector< std::vector< double > >& point )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< std::vector< double > > result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it != layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;

    const auto pos = point[ counter ];
    result.push_back( layer->compute_displacement( pos, lid ) );

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
  if ( layer_to_nc->size() != 1 and layer_from_nc->size() != 1 and not( layer_to_nc->size() == layer_from_nc->size() ) )
  {
    throw BadProperty( "NodeCollections must have equal length or one must have size 1." );
  }

  auto layer_to_positions = get_position( layer_to_nc );

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
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
    }
    const long lid = node_id - first_node_id;

    // If layer_from has size 1, we need to iterate over the layer_to positions
    for ( auto& pos : layer_to_positions )
    {
      result.push_back( layer_from->compute_distance( pos, lid ) );
    }
  }
  else
  {
    for ( NodeCollection::const_iterator it = layer_from_nc->begin(); it < layer_from_nc->end(); ++it )
    {
      size_t node_id = ( *it ).node_id;
      if ( not kernel().node_manager.is_local_node_id( node_id ) )
      {
        throw KernelException( "Distance is currently implemented for local nodes only." );
      }

      const long lid = node_id - first_node_id;

      const auto pos = layer_to_positions[ counter ];
      const double disp = layer_from->compute_distance( pos, lid );
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
distance( NodeCollectionPTR layer_nc, const std::vector< std::vector< double > >& point )
{
  AbstractLayerPTR layer = get_layer( layer_nc );
  NodeCollectionMetadataPTR meta = layer_nc->get_metadata();
  size_t first_node_id = meta->get_first_node_id();

  int counter = 0;
  std::vector< double > result;
  for ( NodeCollection::const_iterator it = layer_nc->begin(); it < layer_nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;
    if ( not kernel().node_manager.is_local_node_id( node_id ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
    }

    const long lid = node_id - first_node_id;

    const auto pos = point[ counter ];
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
distance( const std::vector< ConnectionID >& conns )
{
  std::vector< double > result;

  size_t num_conns = conns.size();
  result.reserve( num_conns );

  for ( auto& conn_id : conns )
  {
    size_t src = conn_id.get_source_node_id();
    auto src_position = get_position( src );

    size_t trgt = conn_id.get_target_node_id();

    if ( not kernel().node_manager.is_local_node_id( trgt ) )
    {
      throw KernelException( "Distance is currently implemented for local nodes only." );
    }


    NodeCollectionPTR trgt_nc = kernel().node_manager.node_id_to_node_collection( trgt );
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

MaskPTR
create_mask( const Dictionary& mask_dict )
{
  mask_dict.init_access_flags();

  // The dictionary should contain one key which is the name of the
  // mask type, and optionally the key 'anchor'. To find the unknown
  // mask type key, we must loop through all keys.
  bool has_anchor = false;
  MaskPTR mask;

  for ( auto& kv : mask_dict )
  {
    if ( kv.first == names::anchor )
    {
      has_anchor = true;
    }
    else
    {
      mask = create_mask( kv.first, mask_dict.get< Dictionary >( kv.first ) );
    }
  }

  if ( has_anchor )
  {
    // The anchor may be an array of doubles (a spatial position).
    // For grid layers only, it is also possible to provide an array of longs.
    try
    {
      const std::vector< long >& anchor = mask_dict.get< std::vector< long > >( names::anchor );

      switch ( anchor.size() )
      {
      case 2:
        try
        {
          auto& grid_mask_2d = dynamic_cast< GridMask< 2 >& >( *mask );
          grid_mask_2d.set_anchor( Position< 2, int >( anchor[ 0 ], anchor[ 1 ] ) );
        }
        catch ( std::bad_cast& e )
        {
          throw BadProperty( "Mask must be 2-dimensional grid mask." );
        }
        break;
      case 3:
        try
        {
          auto& grid_mask_3d = dynamic_cast< GridMask< 3 >& >( *mask );
          grid_mask_3d.set_anchor( Position< 3, int >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
        }
        catch ( std::bad_cast& e )
        {
          throw BadProperty( "Mask must be 3-dimensional grid mask." );
        }
        break;
      default:
        throw BadProperty( "Anchor must be 2- or 3-dimensional." );
      }
    }
    catch ( TypeMismatch& e )
    {
      std::vector< double > double_anchor = mask_dict.get< std::vector< double > >( names::anchor );
      std::shared_ptr< AbstractMask > amask;

      switch ( double_anchor.size() )
      {
      case 2:
        amask = std::shared_ptr< AbstractMask >(
          new AnchoredMask< 2 >( dynamic_cast< Mask< 2 >& >( *mask ), double_anchor ) );
        break;
      case 3:
        amask = std::shared_ptr< AbstractMask >(
          new AnchoredMask< 3 >( dynamic_cast< Mask< 3 >& >( *mask ), double_anchor ) );
        break;
      default:
        throw BadProperty( "Anchor must be 2- or 3-dimensional." );
      }

      mask = amask;
    }
  }
  mask_dict.all_entries_accessed( "CreateMask", "mask_dict" );

  return mask;
}

NodeCollectionPTR
select_nodes_by_mask( const NodeCollectionPTR layer_nc, const std::vector< double >& anchor, const MaskPTR mask )
{
  std::vector< size_t > mask_node_ids;

  const auto dim = anchor.size();

  if ( dim != 2 and dim != 3 )
  {
    throw BadProperty( "Center must be 2- or 3-dimensional." );
  }

  AbstractLayerPTR abstract_layer = get_layer( layer_nc );

  if ( dim == 2 )
  {
    auto layer = dynamic_cast< Layer< 2 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "2D layer", "other type" );
    }

    auto ml = MaskedLayer< 2 >( *layer, mask, false, layer_nc );

    for ( Ntree< 2, size_t >::masked_iterator it = ml.begin( Position< 2 >( anchor[ 0 ], anchor[ 1 ] ) );
          it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }
  else
  {
    auto layer = dynamic_cast< Layer< 3 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "3D layer", "other type" );
    }

    auto ml = MaskedLayer< 3 >( *layer, mask, false, layer_nc );

    for ( Ntree< 3, size_t >::masked_iterator it = ml.begin( Position< 3 >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }
  // Nodes must be sorted when creating a NodeCollection
  std::sort( mask_node_ids.begin(), mask_node_ids.end() );
  return NodeCollection::create( mask_node_ids );
}

bool
inside( const std::vector< double >& point, const MaskPTR mask )
{
  return mask->inside( point );
}

MaskPTR
intersect_mask( const MaskPTR mask1, const MaskPTR mask2 )
{
  return MaskPTR( mask1->intersect_mask( *mask2 ) );
}

MaskPTR
union_mask( const MaskPTR mask1, const MaskPTR mask2 )
{
  return MaskPTR( mask1->union_mask( *mask2 ) );
}

MaskPTR
minus_mask( const MaskPTR mask1, const MaskPTR mask2 )
{
  return MaskPTR( mask1->minus_mask( *mask2 ) );
}

// PyNEST-NG-FUTURE: This needs a wrapper in nest.{h,cpp} and the wrapper should then handle stopwatches
void
connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const Dictionary& connection_dict )
{
  kernel().connection_manager.sw_construction_connect.start();

  AbstractLayerPTR source = get_layer( source_nc );
  AbstractLayerPTR target = get_layer( target_nc );

  connection_dict.init_access_flags();
  ConnectionCreator connector( connection_dict );
  connection_dict.all_entries_accessed( "ConnectLayers", "connection_dict" );

  kernel().node_manager.update_thread_local_node_data();

  // Set flag before calling source->connect() in case exception is thrown after some connections have been created.
  kernel().connection_manager.set_connections_have_changed();
  source->connect( source_nc, target, target_nc, connector );

  kernel().connection_manager.sw_construction_connect.start();
}

void
dump_layer_nodes( const NodeCollectionPTR layer_nc, const std::string& filename )
{
  AbstractLayerPTR layer = get_layer( layer_nc );

  std::ofstream out( filename );
  if ( out.good() )
  {
    layer->dump_nodes( out );
  }
  out.close();
}

void
dump_layer_connections( const NodeCollectionPTR source_layer_nc,
  const NodeCollectionPTR target_layer_nc,
  const std::string& syn_model,
  const std::string& filename )
{
  AbstractLayerPTR source_layer = get_layer( source_layer_nc );
  AbstractLayerPTR target_layer = get_layer( target_layer_nc );

  std::ofstream out( filename );
  if ( out.good() )
  {
    source_layer->dump_connections( out, source_layer_nc, target_layer, syn_model );
  }
  out.close();
}

Dictionary
get_layer_status( NodeCollectionPTR )
{
  assert( false and "not implemented" );

  return {};
}

} // namespace nest
