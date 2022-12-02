/*
 *  spatial.h
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

#ifndef SPATIAL_H
#define SPATIAL_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "nest_types.h"
#include "node_collection.h"

// Includes from spatial:
#include "free_layer.h"
#include "layer.h"
#include "mask.h"


namespace nest
{

/**
 * Class containing spatial information to be used as metadata in a NodeCollection.
 */
class LayerMetadata : public NodeCollectionMetadata
{
public:
  LayerMetadata( AbstractLayerPTR );
  ~LayerMetadata() override
  {
  }

  void set_status( const dictionary&, bool ) override {};

  void
  get_status( dictionary& d ) const override
  {
    layer_->get_status( d );
  }

  //! Returns pointer to object with layer representation
  const AbstractLayerPTR
  get_layer() const
  {
    return layer_;
  }

  // Using string as enum would make stuff more complicated
  std::string
  get_type() const override
  {
    return "spatial";
  }

  void
  set_first_node_id( index node_id ) override
  {
    first_node_id_ = node_id;
  }

  index
  get_first_node_id() const override
  {
    return first_node_id_;
  }

  bool
  operator==( const NodeCollectionMetadataPTR rhs ) const override
  {
    const auto rhs_layer_metadata = dynamic_cast< LayerMetadata* >( rhs.get() );
    if ( not rhs_layer_metadata )
    {
      return false;
    }
    // Compare status dictionaries of this layer and rhs layer
    dictionary dict;
    dictionary rhs_dict;
    get_status( dict );
    rhs_layer_metadata->get_status( rhs_dict );
    return dict == rhs_dict;
  }

private:
  const AbstractLayerPTR layer_; //!< layer object
  index first_node_id_;
};

AbstractLayerPTR get_layer( NodeCollectionPTR layer_nc );
NodeCollectionPTR create_layer( const dictionary& layer_dict );
std::vector< std::vector< double > > get_position( NodeCollectionPTR layer_nc );
std::vector< double > get_position( const index node_id );
std::vector< std::vector< double > > displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
std::vector< std::vector< double > > displacement( NodeCollectionPTR layer_nc,
  const std::vector< std::vector< double > >& point );
std::vector< double > distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
std::vector< double > distance( NodeCollectionPTR layer_nc, const std::vector< std::vector< double > >& point );
std::vector< double > distance( const std::vector< ConnectionID >& conns );
MaskPTR create_mask( const dictionary& mask_dict );
bool inside( const std::vector< double >& point, const MaskPTR mask );
MaskPTR intersect_mask( const MaskPTR mask1, const MaskPTR mask2 );
MaskPTR union_mask( const MaskPTR mask1, const MaskPTR mask2 );
MaskPTR minus_mask( const MaskPTR mask1, const MaskPTR mask2 );
void connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const dictionary& dict );
void dump_layer_nodes( NodeCollectionPTR layer_nc, std::ostream& out );
void dump_layer_connections( const Token& syn_model,
  NodeCollectionPTR source_layer_nc,
  NodeCollectionPTR target_layer_nc,
  std::ostream& out_file );
dictionary get_layer_status( NodeCollectionPTR layer_nc );
}

#endif /* SPATIAL_H */
