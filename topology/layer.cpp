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

GIDCollectionMetadataPTR AbstractLayer::cached_ntree_gc_ =
  GIDCollectionMetadataPTR( 0 );
GIDCollectionMetadataPTR AbstractLayer::cached_vector_gc_ =
  GIDCollectionMetadataPTR( 0 );

AbstractLayer::~AbstractLayer()
{
}

GIDCollectionPTR
AbstractLayer::create_layer( const DictionaryDatum& layer_dict )
{
  index length = 0;
  std::vector< long > element_ids;
  std::string element_name;
  Token element_model;

  const Token& t = layer_dict->lookup( names::elements );
  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( t.datum() );

  AbstractLayer* layer_local = 0;

  if ( ad )
  {

    for ( Token* tp = ad->begin(); tp != ad->end(); ++tp )
    {

      element_name = std::string( *tp );
      element_model =
        kernel().model_manager.get_modeldict()->lookup( element_name );

      if ( element_model.empty() )
      {
        throw UnknownModelName( element_name );
      }
      // Creates several nodes if the next element in
      // the elements variable is a number.
      if ( ( tp + 1 != ad->end() )
        && dynamic_cast< IntegerDatum* >( ( tp + 1 )->datum() ) )
      {
        // Select how many nodes that should be created.
        const long number = getValue< long >( *( ++tp ) );
        for ( long i = 0; i < number; ++i )
        {
          element_ids.push_back( static_cast< long >( element_model ) );
        }
      }
      else
      {
        element_ids.push_back( static_cast< long >( element_model ) );
      }
    }
  }
  else
  {

    element_name = getValue< std::string >( layer_dict, names::elements );
    element_model =
      kernel().model_manager.get_modeldict()->lookup( element_name );

    if ( element_model.empty() )
    {
      throw UnknownModelName( element_name );
    }
    element_ids.push_back( static_cast< long >( element_model ) );
  }

  if ( layer_dict->known( names::positions ) )
  {
    if ( layer_dict->known( names::rows ) or layer_dict->known( names::columns )
      or layer_dict->known( names::layers ) )
    {
      throw BadProperty(
        "Can not specify both positions and rows or columns." );
    }
    int num_dimensions = 0;

    const Token& tkn = layer_dict->lookup( names::positions );
    if ( tkn.is_a< TokenArray >() )
    {
      TokenArray positions = getValue< TokenArray >( tkn );
      length = positions.size();
      std::vector< double > pos =
        getValue< std::vector< double > >( positions[ 0 ] );
      num_dimensions = pos.size();
    }
    else if ( tkn.is_a< ParameterDatum >() )
    {
      std::cerr << __FILE__ << "::" << __LINE__ << "\n";
      auto pd = dynamic_cast< ParameterDatum* >( tkn.datum() );
      auto positions = dynamic_cast< DimensionParameter* >( pd->get() );
      pd->unlock();
      std::cerr << __FILE__ << "::" << __LINE__ << "\n";
      length = getValue< long >( layer_dict, names::n );
      std::cerr << __FILE__ << "::" << __LINE__ << "\n";
      num_dimensions = positions->get_num_dimensions();
      std::cerr << __FILE__ << "::" << __LINE__ << "\n";
    }
    else
    {
      throw KernelException(
        "'positions' must be an array or a DimensionParameter." );
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
  else if ( layer_dict->known( names::columns ) )
  {

    if ( not layer_dict->known( names::rows ) )
    {
      throw BadProperty( "Both columns and rows must be given." );
    }

    length = getValue< long >( layer_dict, names::columns )
      * getValue< long >( layer_dict, names::rows );

    if ( layer_dict->known( names::layers ) )
    {
      layer_local = new GridLayer< 3 >();
      length *= getValue< long >( layer_dict, names::layers );
    }
    else
    {
      layer_local = new GridLayer< 2 >();
    }
  }
  else
  {
    throw BadProperty( "Unknown layer type." );
  }

  assert( layer_local );
  lockPTR< AbstractLayer > layer_safe( layer_local );
  GIDCollectionMetadataPTR layer_meta( new LayerMetadata( layer_safe ) );

  // We have at least one element, create a GIDCollection for it
  GIDCollectionPTR gid_coll =
    kernel().node_manager.add_node( element_ids[ 0 ], length );

  gid_coll->set_metadata( layer_meta );

  // Create all remaining elements and add
  for ( size_t i = 1; i < element_ids.size(); ++i )
  {
    GIDCollectionPTR next_coll =
      kernel().node_manager.add_node( element_ids[ i ], length );
    next_coll->set_metadata( layer_meta );
    gid_coll = gid_coll->operator+( next_coll );
    next_coll.unlock();
  }

  get_layer( gid_coll )->gid_collection_ = gid_coll;

  layer_meta->set_first_gid( gid_coll->operator[]( 0 ) );

  std::cerr << __FILE__ << "::" << __LINE__ << "\n";
  layer_local->set_status( layer_dict );
  std::cerr << __FILE__ << "::" << __LINE__ << "\n";

  return gid_coll;
}

GIDCollectionMetadataPTR
AbstractLayer::get_metadata() const
{
  return gid_collection_->get_metadata();
}

} // namespace nest
