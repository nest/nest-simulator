/*
 *  layer.cpp
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

#include "layer.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "gid_collection.h"
#include "kernel_manager.h"
#include "parameter.h"

// Includes from sli:
#include "dictutils.h"
#include "integerdatum.h"

// Includes from topology:
#include "connection_creator_impl.h"
#include "free_layer.h"
#include "grid_layer.h"
#include "layer_impl.h"
#include "mask_impl.h"
#include "topology.h"
#include "topology_names.h"

namespace nest
{

GIDCollectionMetadataPTR AbstractLayer::cached_ntree_md_ = GIDCollectionMetadataPTR( 0 );
GIDCollectionMetadataPTR AbstractLayer::cached_vector_md_ = GIDCollectionMetadataPTR( 0 );

AbstractLayer::~AbstractLayer()
{
}

GIDCollectionPTR
AbstractLayer::create_layer( const DictionaryDatum& layer_dict )
{
  index length = 0;
  AbstractLayer* layer_local = 0;

  auto element_name = getValue< std::string >( layer_dict, names::elements );
  auto element_model = kernel().model_manager.get_modeldict()->lookup( element_name );

  if ( element_model.empty() )
  {
    throw UnknownModelName( element_name );
  }
  auto element_id = static_cast< long >( element_model );

  if ( layer_dict->known( names::positions ) )
  {
    if ( layer_dict->known( names::shape ) )
    {
      throw BadProperty( "Cannot specify both positions and shape." );
    }
    int num_dimensions = 0;

    const Token& tkn = layer_dict->lookup( names::positions );
    if ( tkn.is_a< TokenArray >() )
    {
      TokenArray positions = getValue< TokenArray >( tkn );
      length = positions.size();
      std::vector< double > pos = getValue< std::vector< double > >( positions[ 0 ] );
      num_dimensions = pos.size();
    }
    else if ( tkn.is_a< ParameterDatum >() )
    {
      auto pd = dynamic_cast< ParameterDatum* >( tkn.datum() );
      auto positions = dynamic_cast< DimensionParameter* >( pd->get() );
      // To avoid nasty segfaults, we check that the parameter is indeed a DimensionParameter.
      if ( not std::is_same< std::remove_reference< decltype( *positions ) >::type, DimensionParameter >::value )
      {
        throw KernelException( "When 'positions' is a Parameter, it must be a DimensionParameter." );
      }
      length = getValue< long >( layer_dict, names::n );
      num_dimensions = positions->get_num_dimensions();
    }
    else
    {
      throw KernelException( "'positions' must be an array or a DimensionParameter." );
    }

    if ( length == 0 )
    {
      throw BadProperty( "Empty positions array." );
    }

    if ( num_dimensions == 2 )
    {
      layer_local = new FreeLayer< 2 >();
    }
    else if ( num_dimensions == 3 )
    {
      layer_local = new FreeLayer< 3 >();
    }
    else
    {
      throw BadProperty( "Positions must have 2 or 3 coordinates." );
    }
  }
  else if ( layer_dict->known( names::shape ) )
  {
    std::vector< long > shape = getValue< std::vector< long > >( layer_dict, names::shape );

    if ( not std::all_of( shape.begin(),
           shape.end(),
           []( long x )
           {
             return x > 0;
           } ) )
    {
      throw BadProperty( "All shape entries must be positive." );
    }

    int num_dimensions = shape.size();
    length = std::accumulate( std::begin( shape ), std::end( shape ), 1, std::multiplies< long >() );

    if ( num_dimensions == 2 )
    {
      layer_local = new GridLayer< 2 >();
    }
    else if ( num_dimensions == 3 )
    {
      layer_local = new GridLayer< 3 >();
    }
    else
    {
      throw BadProperty( "Shape must be of length 2 or 3." );
    }
  }
  else
  {
    throw BadProperty( "Unknown layer type." );
  }

  assert( layer_local );
  std::shared_ptr< AbstractLayer > layer_safe( layer_local );
  GIDCollectionMetadataPTR layer_meta( new LayerMetadata( layer_safe ) );

  // We have at least one element, create a GIDCollection for it
  GIDCollectionPTR gid_coll = kernel().node_manager.add_node( element_id, length );

  gid_coll->set_metadata( layer_meta );

  get_layer( gid_coll )->gid_collection_ = gid_coll;

  layer_meta->set_first_gid( gid_coll->operator[]( 0 ) );

  layer_local->set_status( layer_dict );

  return gid_coll;
}

GIDCollectionMetadataPTR
AbstractLayer::get_metadata() const
{
  return gid_collection_->get_metadata();
}

} // namespace nest
