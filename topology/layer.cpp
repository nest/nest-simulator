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
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"
#include "integerdatum.h"

// Includes from topology:
#include "free_layer.h"
#include "grid_layer.h"
#include "topology_names.h"

namespace nest {

index AbstractLayer::cached_ntree_layer_ = -1;
index AbstractLayer::cached_vector_layer_ = -1;

AbstractLayer::~AbstractLayer()
{
}

index
AbstractLayer::create_layer( const DictionaryDatum& layer_dict )
{
  index length = 0;
  const char* layer_model_name = 0;
  std::vector< long > element_ids;
  std::string element_name;
  Token element_model;

  const Token& t = layer_dict->lookup( names::elements );
  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( t.datum() );

  if ( ad ) {

    for ( Token* tp = ad->begin(); tp != ad->end(); ++tp ) {

      element_name = std::string( *tp );
      element_model = kernel().model_manager.get_modeldict()->lookup( element_name );

      if ( element_model.empty() ) {
        throw UnknownModelName( element_name );
      }
      // Creates several nodes if the next element in
      // the elements variable is a number.
      if ( ( tp + 1 != ad->end() ) && dynamic_cast< IntegerDatum* >( ( tp + 1 )->datum() ) ) {
        // Select how many nodes that should be created.
        const long number = getValue< long >( *( ++tp ) );
        for ( long i = 0; i < number; ++i ) {
          element_ids.push_back( static_cast< long >( element_model ) );
        }
      }
      else {
        element_ids.push_back( static_cast< long >( element_model ) );
      }
    }
  }
  else {

    element_name = getValue< std::string >( layer_dict, names::elements );
    element_model = kernel().model_manager.get_modeldict()->lookup( element_name );

    if ( element_model.empty() ) {
      throw UnknownModelName( element_name );
    }
    element_ids.push_back( static_cast< long >( element_model ) );
  }

  if ( layer_dict->known( names::positions ) ) {
    if ( layer_dict->known( names::rows ) or layer_dict->known( names::columns )
      or layer_dict->known( names::layers ) ) {
      throw BadProperty( "Can not specify both positions and rows or columns." );
    }
    TokenArray positions = getValue< TokenArray >( layer_dict, names::positions );

    if ( positions.size() == 0 ) {
      throw BadProperty( "Empty positions array." );
    }

    std::vector< double > pos = getValue< std::vector< double > >( positions[ 0 ] );
    if ( pos.size() == 2 ) {
      layer_model_name = "topology_layer_free";
    }
    else if ( pos.size() == 3 ) {
      layer_model_name = "topology_layer_free_3d";
    }
    else {
      throw BadProperty( "Positions must have 2 or 3 coordinates." );
    }

    length = positions.size();
  }
  else if ( layer_dict->known( names::columns ) ) {

    if ( not layer_dict->known( names::rows ) ) {
      throw BadProperty( "Both columns and rows must be given." );
    }

    length = getValue< long >( layer_dict, names::columns ) * getValue< long >( layer_dict, names::rows );

    if ( layer_dict->known( names::layers ) ) {
      layer_model_name = "topology_layer_grid_3d";
      length *= getValue< long >( layer_dict, names::layers );
    }
    else {
      layer_model_name = "topology_layer_grid";
    }
  }
  else {
    throw BadProperty( "Unknown layer type." );
  }

  assert( layer_model_name != 0 );
  Token layer_model = kernel().model_manager.get_modeldict()->lookup( layer_model_name );
  if ( layer_model.empty() ) {
    throw UnknownModelName( layer_model_name );
  }
  index layer_node = kernel().node_manager.add_node( layer_model );

  // Remember original subnet
  const index cwnode = kernel().node_manager.get_cwn()->get_gid();

  kernel().node_manager.go_to( layer_node );

  // Create layer nodes.
  for ( size_t i = 0; i < element_ids.size(); ++i ) {
    kernel().node_manager.add_node( element_ids[ i ], length );
  }
  // Return to original subnet
  kernel().node_manager.go_to( cwnode );

  // Set layer parameters according to input dictionary.
  AbstractLayer* layer = dynamic_cast< AbstractLayer* >( kernel().node_manager.get_node( layer_node ) );
  layer->depth_ = element_ids.size();
  layer->set_status( layer_dict );

  return layer_node;
}

std::vector< Node* >::iterator
AbstractLayer::local_begin( int depth )
{
  if ( depth >= depth_ ) {
    throw BadProperty( "Selected depth out of range" );
  }
  index min_nodes_per_layer = local_size() / depth_;
  index first_gid_at_depth = gids_[ depth * ( global_size() / depth_ ) ];
  std::vector< Node* >::iterator iter = local_begin();
  for ( iter += depth * min_nodes_per_layer; iter != local_end(); ++iter ) {
    if ( ( *iter )->get_gid() >= first_gid_at_depth ) {
      break;
    }
  }
  return iter;
}

std::vector< Node* >::iterator
AbstractLayer::local_end( int depth )
{
  if ( depth >= depth_ ) {
    throw BadProperty( "Selected depth out of range" );
  }
  index min_nodes_per_layer = local_size() / depth_;
  index last_gid_at_depth = gids_[ ( depth + 1 ) * ( global_size() / depth_ ) - 1 ];
  std::vector< Node* >::iterator iter = local_begin();
  for ( iter += ( depth + 1 ) * min_nodes_per_layer; iter != local_end(); ++iter ) {
    if ( ( *iter )->get_gid() > last_gid_at_depth ) {
      break;
    }
  }
  return iter;
}

std::vector< Node* >::const_iterator
AbstractLayer::local_begin( int depth ) const
{
  if ( depth >= depth_ ) {
    throw BadProperty( "Selected depth out of range" );
  }
  index min_nodes_per_layer = local_size() / depth_;
  index first_gid_at_depth = gids_[ depth * ( global_size() / depth_ ) ];
  std::vector< Node* >::const_iterator iter = local_begin();
  for ( iter += depth * min_nodes_per_layer; iter != local_end(); ++iter ) {
    if ( ( *iter )->get_gid() >= first_gid_at_depth ) {
      break;
    }
  }
  return iter;
}

std::vector< Node* >::const_iterator
AbstractLayer::local_end( int depth ) const
{
  if ( depth >= depth_ ) {
    throw BadProperty( "Selected depth out of range" );
  }
  index min_nodes_per_layer = local_size() / depth_;
  index last_gid_at_depth = gids_[ ( depth + 1 ) * ( global_size() / depth_ ) - 1 ];
  std::vector< Node* >::const_iterator iter = local_begin();
  for ( iter += ( depth + 1 ) * min_nodes_per_layer; iter != local_end(); ++iter ) {
    if ( ( *iter )->get_gid() > last_gid_at_depth ) {
      break;
    }
  }
  return iter;
}

} // namespace nest
