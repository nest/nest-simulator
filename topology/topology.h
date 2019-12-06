/*
 *  topology.h
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

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "node_collection.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dictdatum.h"
#include "iostreamdatum.h"
#include "token.h"

// Includes from topology:
#include "layer.h"
#include "mask.h"


namespace nest
{

/**
 * Class representing metadata for topology layer.
 */
class LayerMetadata : public NodeCollectionMetadata
{
public:
  LayerMetadata( AbstractLayerPTR );
  ~LayerMetadata()
  {
  }

  void set_status( const DictionaryDatum&, bool ){};

  void
  get_status( DictionaryDatum& d ) const
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
  get_type() const
  {
    return "spatial";
  }

  void
  set_first_node_id( index node_id )
  {
    first_node_id_ = node_id;
  }
  index
  get_first_node_id() const
  {
    return first_node_id_;
  }

private:
  const AbstractLayerPTR layer_; //!< layer object
  index first_node_id_;
};

AbstractLayerPTR get_layer( NodeCollectionPTR layer_nc );
NodeCollectionPTR create_layer( const DictionaryDatum& layer_dict );
ArrayDatum get_position( NodeCollectionPTR layer_nc );
ArrayDatum displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
ArrayDatum displacement( NodeCollectionPTR layer_nc, const ArrayDatum point );
std::vector< double > distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc );
std::vector< double > distance( NodeCollectionPTR layer_nc, const ArrayDatum point );
MaskDatum create_mask( const DictionaryDatum& mask_dict );
BoolDatum inside( const std::vector< double >& point, const MaskDatum& mask );
MaskDatum intersect_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
MaskDatum union_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
MaskDatum minus_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
void connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const DictionaryDatum& dict );
void dump_layer_nodes( NodeCollectionPTR layer_nc, OstreamDatum& out );
void dump_layer_connections( const Token& syn_model,
  NodeCollectionPTR source_layer_nc,
  NodeCollectionPTR target_layer_nc,
  OstreamDatum& out_file );
DictionaryDatum get_layer_status( NodeCollectionPTR layer_nc );
}

#endif /* TOPOLOGY_H */
