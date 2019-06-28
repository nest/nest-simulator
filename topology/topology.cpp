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
  : GIDCollectionMetadata()
  , layer_( layer )
  , first_gid_( 0 )
{
}


AbstractLayerPTR
get_layer( GIDCollectionPTR gc )
{
  GIDCollectionMetadataPTR meta = gc->get_metadata();

  LayerMetadata const* const layer_meta = dynamic_cast< LayerMetadata const* >( meta.get() );
  if ( not layer_meta )
  {
    throw LayerExpected();
  }
  return layer_meta->get_layer();
}

GIDCollectionPTR
create_layer( const DictionaryDatum& layer_dict )
{
  layer_dict->clear_access_flags();

  GIDCollectionPTR layer = AbstractLayer::create_layer( layer_dict );

  ALL_ENTRIES_ACCESSED( *layer_dict, "topology::CreateLayer", "Unread dictionary entries: " );

  return layer;
}

ArrayDatum
get_position( GIDCollectionPTR layer_gc )
{
  AbstractLayerPTR layer = get_layer( layer_gc );
  GIDCollectionMetadataPTR meta = layer_gc->get_metadata();
  index first_gid = meta->get_first_gid();

  ArrayDatum result;
  result.reserve( layer_gc->size() );

  for ( GIDCollection::const_iterator it = layer_gc->begin(); it < layer_gc->end(); ++it )
  {
    index gid = ( *it ).gid;

    if ( not kernel().node_manager.is_local_gid( gid ) )
    {
      throw KernelException( "GetPosition is currently implemented for local nodes only." );
    }

    const long lid = gid - first_gid;
    Token arr = layer->get_position_vector( lid );

    result.push_back( arr );
  }

  return result;
}

ArrayDatum
displacement( GIDCollectionPTR layer_to_gc, GIDCollectionPTR layer_from_gc )
{
  ArrayDatum layer_to_positions = get_position( layer_to_gc );

  AbstractLayerPTR layer_from = get_layer( layer_from_gc );
  GIDCollectionMetadataPTR meta = layer_from_gc->get_metadata();
  index first_gid = meta->get_first_gid();

  int counter = 0;
  ArrayDatum result;

  // If layer_from has size equal to one, but layer_to do not, we want the
  // displacement between every node in layer_to against the one in layer_from.
  // Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_gc->size() == 1 )
  {
    index gid = layer_from_gc->operator[]( 0 );
    if ( not kernel().node_manager.is_local_gid( gid ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }
    const long lid = gid - first_gid;

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
    for ( GIDCollection::const_iterator it = layer_from_gc->begin(); it < layer_from_gc->end(); ++it )
    {
      index gid = ( *it ).gid;
      if ( not kernel().node_manager.is_local_gid( gid ) )
      {
        throw KernelException( "Displacement is currently implemented for local nodes only." );
      }

      const long lid = gid - first_gid;

      std::vector< double > pos = getValue< std::vector< double > >( layer_to_positions[ counter ] );
      Token disp = layer_from->compute_displacement( pos, lid );
      result.push_back( disp );

      // We only iterate the layer_to positions vector if it has more than one
      // element.
      if ( layer_to_gc->size() != 1 )
      {
        ++counter;
      }
    }
  }

  return result;
}

ArrayDatum
displacement( GIDCollectionPTR layer_gc, const ArrayDatum point )
{
  AbstractLayerPTR layer = get_layer( layer_gc );
  GIDCollectionMetadataPTR meta = layer_gc->get_metadata();
  index first_gid = meta->get_first_gid();

  int counter = 0;
  ArrayDatum result;
  for ( GIDCollection::const_iterator it = layer_gc->begin(); it != layer_gc->end(); ++it )
  {
    index gid = ( *it ).gid;
    if ( not kernel().node_manager.is_local_gid( gid ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }

    const long lid = gid - first_gid;

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
distance( GIDCollectionPTR layer_to_gc, GIDCollectionPTR layer_from_gc )
{
  ArrayDatum layer_to_positions = get_position( layer_to_gc );

  AbstractLayerPTR layer_from = get_layer( layer_from_gc );
  GIDCollectionMetadataPTR meta = layer_from_gc->get_metadata();
  index first_gid = meta->get_first_gid();

  int counter = 0;
  std::vector< double > result;

  // If layer_from has size equal to one, but layer_to do not, we want the
  // distance between every node in layer_to against the one in layer_from.
  // Likewise if layer_to has size 1 and layer_from do not.
  if ( layer_from_gc->size() == 1 )
  {
    index gid = layer_from_gc->operator[]( 0 );
    if ( not kernel().node_manager.is_local_gid( gid ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }
    const long lid = gid - first_gid;

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
    for ( GIDCollection::const_iterator it = layer_from_gc->begin(); it < layer_from_gc->end(); ++it )
    {
      index gid = ( *it ).gid;
      if ( not kernel().node_manager.is_local_gid( gid ) )
      {
        throw KernelException( "Displacement is currently implemented for local nodes only." );
      }

      const long lid = gid - first_gid;

      std::vector< double > pos = getValue< std::vector< double > >( layer_to_positions[ counter ] );
      double disp = layer_from->compute_distance( pos, lid );
      result.push_back( disp );

      // We only iterate the layer_to positions vector if it has more than one
      // element.
      if ( layer_to_gc->size() != 1 )
      {
        ++counter;
      }
    }
  }

  return result;
}

std::vector< double >
distance( GIDCollectionPTR layer_gc, const ArrayDatum point )
{
  AbstractLayerPTR layer = get_layer( layer_gc );
  GIDCollectionMetadataPTR meta = layer_gc->get_metadata();
  index first_gid = meta->get_first_gid();

  int counter = 0;
  std::vector< double > result;
  for ( GIDCollection::const_iterator it = layer_gc->begin(); it < layer_gc->end(); ++it )
  {
    index gid = ( *it ).gid;
    if ( not kernel().node_manager.is_local_gid( gid ) )
    {
      throw KernelException( "Displacement is currently implemented for local nodes only." );
    }

    const long lid = gid - first_gid;

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
connect_layers( GIDCollectionPTR source_gc, GIDCollectionPTR target_gc, const DictionaryDatum& connection_dict )
{
  kernel().connection_manager.set_have_connections_changed( true );

  AbstractLayerPTR source = get_layer( source_gc );
  AbstractLayerPTR target = get_layer( target_gc );

  connection_dict->clear_access_flags();
  ConnectionCreator connector( connection_dict );
  ALL_ENTRIES_ACCESSED( *connection_dict, "topology::CreateLayers", "Unread dictionary entries: " );

  source->connect( target, target_gc, connector );
}

void
dump_layer_nodes( GIDCollectionPTR layer_gc, OstreamDatum& out )
{
  AbstractLayerPTR layer = get_layer( layer_gc );

  if ( out->good() )
  {
    layer->dump_nodes( *out );
  }
}

void
dump_layer_connections( const Token& syn_model,
  GIDCollectionPTR source_layer_gc,
  GIDCollectionPTR target_layer_gc,
  OstreamDatum& out )
{
  AbstractLayerPTR source_layer = get_layer( source_layer_gc );
  AbstractLayerPTR target_layer = get_layer( target_layer_gc );

  if ( out->good() )
  {
    source_layer->dump_connections( *out, target_layer, syn_model );
  }
}

DictionaryDatum
get_layer_status( GIDCollectionPTR layer_gc )
{
  assert( false && "not implemented" );

  return DictionaryDatum();
}

} // namespace nest
