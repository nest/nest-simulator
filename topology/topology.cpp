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
{
}


AbstractLayerPTR
get_layer( GIDCollectionPTR gc )
{
  GIDCollectionMetadataPTR meta = gc->get_metadata();

  LayerMetadata const* const layer_meta =
    dynamic_cast< LayerMetadata const* >( meta.get() );
  meta.unlock();
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

  ALL_ENTRIES_ACCESSED(
    *layer_dict, "topology::CreateLayer", "Unread dictionary entries: " );

  return layer;
}

std::vector< double >
get_position( GIDCollectionPTR layer_gc, const index gid )
{
  if ( not kernel().node_manager.is_local_gid( gid ) )
  {
    throw KernelException(
      "GetPosition is currently implemented for local nodes only." );
  }

  AbstractLayerPTR layer = get_layer( layer_gc );
  const long lid = layer_gc->find( gid );
  if ( lid < 0 )
  {
    throw LayerNodeExpected();
  }
  return layer->get_position_vector( lid );
}

std::vector< double >
displacement( GIDCollectionPTR layer_gc,
  const std::vector< double >& point,
  const index node_gid )
{
  if ( not kernel().node_manager.is_local_gid( node_gid ) )
  {
    throw KernelException(
      "Displacement is currently implemented for local nodes only." );
  }

  AbstractLayerPTR layer = get_layer( layer_gc );
  return layer->compute_displacement( point, node_gid );
}

double
distance( GIDCollectionPTR layer_gc,
  const std::vector< double >& point,
  const index node_gid )
{
  if ( not kernel().node_manager.is_local_gid( node_gid ) )
  {
    throw KernelException(
      "Distance is currently implemented for local nodes only." );
  }

  AbstractLayerPTR layer = get_layer( layer_gc );
  return layer->compute_distance( point, node_gid );
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

void
connect_layers( GIDCollectionPTR source_gc,
  GIDCollectionPTR target_gc,
  const DictionaryDatum& connection_dict )
{
  AbstractLayerPTR source = get_layer( source_gc );
  AbstractLayerPTR target = get_layer( target_gc );

  connection_dict->clear_access_flags();
  ConnectionCreator connector( connection_dict );
  ALL_ENTRIES_ACCESSED(
    *connection_dict, "topology::CreateLayers", "Unread dictionary entries: " );

  source->connect( target, target_gc, connector );
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
  GIDCollectionPTR layer_gc,
  OstreamDatum& out )
{
  AbstractLayerPTR layer = get_layer( layer_gc );

  if ( out->good() )
  {
    layer->dump_connections( *out, syn_model );
  }
}

index
get_element( GIDCollectionPTR layer_gc, const TokenArray array )
{
  index node_gid;

  AbstractLayerPTR abstract_layer = get_layer( layer_gc );

  switch ( array.size() )
  {
  case 2:
  {
    GridLayer< 2 > const* layer =
      dynamic_cast< GridLayer< 2 > const* >( abstract_layer.get() );
    if ( layer_gc.islocked() )
    {
      layer_gc.unlock();
    }
    if ( not layer )
    {
      throw TypeMismatch( "2D grid layer", "other layer type" );
    }

    node_gid =
      layer->get_node( Position< 2, int >( static_cast< index >( array[ 0 ] ),
        static_cast< index >( array[ 1 ] ) ) );
  }
  break;

  case 3:
  {
    GridLayer< 3 > const* layer =
      dynamic_cast< GridLayer< 3 > const* >( abstract_layer.get() );
    if ( layer_gc.islocked() )
    {
      layer_gc.unlock();
    }

    if ( not layer )
    {
      throw TypeMismatch( "3D grid layer node", "other layer type" );
    }

    node_gid =
      layer->get_node( Position< 3, int >( static_cast< index >( array[ 0 ] ),
        static_cast< index >( array[ 1 ] ),
        static_cast< index >( array[ 2 ] ) ) );
  }
  break;

  default:
    throw TypeMismatch( "array with length 2 or 3", "something else" );
  }

  if ( abstract_layer.islocked() )
  {
    // Need to unlock the layer in case we call the function several times,
    // or the layer is to be used in some other way.
    abstract_layer.unlock();
  }
  return node_gid;
}

DictionaryDatum
get_layer_status( GIDCollectionPTR layer_gc )
{
  assert( false && "not implemented" );

  return DictionaryDatum();
}

} // namespace nest
