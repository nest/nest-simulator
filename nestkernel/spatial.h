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

#include <stddef.h>
// C++ includes:
#include <string>
#include <vector>

// Includes from nestkernel:
#include "nest_types.h"
#include "node_collection.h"
// Includes from spatial:
#include "dictionary.h"
#include "free_layer.h"
#include "layer.h"
#include "mask.h"


namespace nest
{
class ConnectionID;

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

  void set_status( const Dictionary&, bool ) override {};

  void get_status( Dictionary& d, NodeCollection const* const nc ) const override;

  void get_status( Dictionary& d, const NodeCollectionPTR nc ) const override;

  //! Returns pointer to object with layer representation
  const AbstractLayerPTR get_layer() const;

  // Using string as enum would make stuff more complicated
  std::string get_type() const override;

  void set_first_node_id( size_t node_id ) override;

  size_t get_first_node_id() const override;
  bool operator==( const NodeCollectionMetadataPTR rhs ) const override;

private:
  const AbstractLayerPTR layer_; //!< layer object
  size_t first_node_id_;
};

AbstractLayerPTR get_layer( NodeCollectionPTR layer_nc );
NodeCollectionPTR create_layer( const Dictionary& layer_dict );
std::vector< std::vector< double > > get_position( NodeCollectionPTR layer_nc );
std::vector< double > get_position( const size_t node_id );
std::vector< std::vector< double > > displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
std::vector< std::vector< double > > displacement( NodeCollectionPTR layer_nc,
  const std::vector< std::vector< double > >& point );
std::vector< double > distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
std::vector< double > distance( NodeCollectionPTR layer_nc, const std::vector< std::vector< double > >& point );
std::vector< double > distance( const std::vector< ConnectionID >& conns );
MaskPTR create_mask( const Dictionary& mask_dict );
NodeCollectionPTR
select_nodes_by_mask( const NodeCollectionPTR layer_nc, const std::vector< double >& anchor, const MaskPTR mask );
bool inside( const std::vector< double >& point, const MaskPTR mask );
MaskPTR intersect_mask( const MaskPTR mask1, const MaskPTR mask2 );
MaskPTR union_mask( const MaskPTR mask1, const MaskPTR mask2 );
MaskPTR minus_mask( const MaskPTR mask1, const MaskPTR mask2 );
void connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const Dictionary& dict );
void dump_layer_nodes( NodeCollectionPTR layer_nc, const std::string& filename );
void dump_layer_connections( const NodeCollectionPTR source_layer,
  const NodeCollectionPTR target_layer,
  const std::string& synapse_model,
  const std::string& filename );
Dictionary get_layer_status( NodeCollectionPTR layer_nc );
}

#endif /* SPATIAL_H */
