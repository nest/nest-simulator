/*
 *  layer_regular.cpp
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

/*
  This file is part of the NEST topology module.
  Author: Kittel Austvoll
*/

#include "layer_regular.h"

#include <vector>

#include "compound.h"
#include "dictutils.h"
#include "node.h"
#include "nestmodule.h"
#include "leaflist.h"
#include "sliexceptions.h"
#include "iostreamdatum.h"

#include "position.h"
#include "topologyconnector.h"
#include "nodewrapper.h"
#include "region.h"
#include "selector.h"
#include "topology_names.h"
#include "layer_unrestricted.h"
#include "layer_slice.h"

namespace nest
{

  LayerRegular::LayerRegular():
    Layer(2),
    rows_(0),
    columns_(0),
    dpd_(0,0)
  {
  }

  LayerRegular::LayerRegular(const LayerRegular& l):
    Layer(l),
    rows_(l.rows_),
    columns_(l.columns_),
    dpd_(l.dpd_)
  {
  }

  LayerRegular::LayerRegular(const LayerRegular& l, const std::vector<Node*>& nodes):
    Layer(l),
    rows_(l.rows_),
    columns_(l.columns_), 
    dpd_(l.dpd_)
  {
    nodes_ = nodes;
  }
  
  LayerRegular::~LayerRegular()
  {
  }

  lockPTR<Layer> LayerRegular::
  slice(bool unrestricted,
	const DictionaryDatum& layer_connection_dict) const
  {
    // Convert layer temporarily to an unrestricted layer if the 
    // mask region in the connection dictionary isn't suited for 
    // a discrete layer (isn't grid).
    if(unrestricted)
    {
      return lockPTR<Layer>(new LayerSlice<LayerUnrestricted>(*this, layer_connection_dict));
    }

    // Return the new layer.
    return lockPTR<Layer>(new LayerSlice<LayerRegular>(*this, layer_connection_dict));
  }

  void LayerRegular::set_status(const DictionaryDatum& layer_dict)
  {
    // Layer properties can either be passed inside a sub topology 
    // dictionary or on the upper level of a dictionary
    DictionaryDatum layer_dictionary = new Dictionary();
    if(!updateValue<DictionaryDatum>(layer_dict, names::topology, 
				     layer_dictionary))
      {
	layer_dictionary = layer_dict;
      }

    // Properties that are not set by the user 
    // here takes on the default values.
    updateValue<long_t>(layer_dictionary, names::rows, rows_);
    updateValue<long_t>(layer_dictionary, names::columns, columns_);

    if(rows_ < 0)
      throw TypeMismatch("rows >= 0", "rows < 0");

    if(columns_ < 0)
      throw TypeMismatch("columns >= 0", "columns < 0");

    Layer::set_status(layer_dictionary);

    calc_dpd_();

    //Asserts that layer dimensions are valid
    test_validity();

  }

  void LayerRegular::test_validity() const
  {
    if(nodes_.size() != static_cast<uint_t>(rows_*columns_))
      {
	throw DimensionMismatch(nodes_.size(), rows_*columns_);
      }
  }

  void LayerRegular::get_status(DictionaryDatum& d) const
  {
    Layer::get_status(d);

    DictionaryDatum dict = getValue<DictionaryDatum>((*d)[names::topology]);

    (*dict)[names::rows] = rows_;
    (*dict)[names::columns] = columns_;
  }

  Position<double_t> LayerRegular::get_dpd() const
  {
    return dpd_;
  }

  uint_t LayerRegular::get_columns() const
  {
    return columns_;
  }
  
  uint_t LayerRegular::get_rows() const
  {
    return rows_;
  }

  const Position<double_t> LayerRegular::calc_dpd_()
  {
    double_t y;
    double_t x;

    if(extent_.at(1) == 0)
      {
	y = 0;
      }
    else
      {
	// Nodes are placed in a regular grid with equal space on each
	// side of the nodes, such that the grid is centered in the
	// available space.

	y = rows_/extent_.at(1); 
      }
    
    if(extent_.at(0) == 0)
      {
	x = 0;
      }
    else
      {
	x = columns_/extent_.at(0);
      }
 
    dpd_ = Position<double_t>(x, y);
    return dpd_;
  }

  const Position<double_t> 
  LayerRegular::layer2spatial(const Position<double_t>& discrete) const
  {
    //The vertical layerspace axis has the opposite direction of the
    //spatial axis.
    return (discrete+Position<double_t>(0.5, 0.5))*Position<double_t>(1.0, -1.0)/dpd_ + upper_left_;
  }

  const Position<int_t> 
  LayerRegular::spatial2layer(const Position<double_t>& spatial) const
  {
    //The vertical spatial axis has the opposite direction of the
    //layerspace axis.
    return ((spatial-upper_left_)*Position<double_t>(1.0, -1.0)*dpd_ 
            - Position<double_t>(0.5,0.5)).to_nearest_int();
  }

  Node*
  LayerRegular::get_node(const Position<int_t>& coordinates) const
  {
    //Ignore values if position is outside of layer bounds.
    //The layer anchors should be accounted for prior to 
    //this function (in this function we assume that the layer
    //has upper-left corner in layerspace center (0, 0)).
    if(coordinates.within_range(Position<int_t>(0,0),  
				Position<int_t>(columns_-1,  
						      rows_-1))) 
      { 
	//Convert 2-dim position to local ID, and retrieve node.
	return nodes_.at(coordinates.pos2lid(rows_));
      }
    return 0;
  }

  const Position<double_t> 
  LayerRegular::get_position(index lid) const
  {
    //Valid for column-wise folding.
    //Find position of iterator node in space (anchor is taken
    //into account).

    return layer2spatial(Position<double_t>
			 (lid / (rows_),
			  lid % (rows_)));
  }

  const Position<double_t> LayerRegular::compute_displacement(const Position<double_t>& from_pos,
							      const Node& to) const
  {
    // make sure node really belongs to this layer
    assert(get_layer(to) == this);

    // obtain "raw" displacement
    const Position<double_t> to_pos = Layer::get_position(to);
    Position<double_t> d = to_pos - from_pos;

    // handle periodic boundary conditions
    if ( EDGE_WRAP_ )
    {
      d.wrap_displacement_max_half(Position<double_t>(extent_));
    }

    return d;
  }

  std::vector<Node*> LayerRegular::get_nodes(Position<int_t> pos) const
  {
    //Check if position is out of range
    if(!pos.within_range(Position<int_t>(0,0),  
			 Position<int_t>(columns_-1,  
					       rows_-1))) 
      { 
	if(EDGE_WRAP_)
	  {
	    pos.edge_wrap(columns_, rows_);
	  }
	else
	  {
	    return std::vector<Node*>();
	  }
      } 

    int_t lid = pos.pos2lid(rows_);

    return Layer::get_nodes(nodes_.at(lid));
  }

  lockPTR<std::vector<NodeWrapper> >
  LayerRegular::get_pool_nodewrappers(const Position<double_t>& driver_coo,
			       AbstractRegion* aregion)
  {
    // Ensure that a discrete region is used
    DiscreteRegion* region = dynamic_cast<DiscreteRegion*>(aregion);
    if (region==0) {
      throw TypeMismatch("discrete region","unrestricted region");
    }

    lockPTR<std::vector<NodeWrapper> > 
      connections(new std::vector<NodeWrapper>());
    
    connections->reserve(region->size());
    
    const Position<int_t> coordinates = 
      spatial2layer(driver_coo);
    
    // Needed when evaluating parameter functions for periodic boundary conditions
    std::vector<double_t> *extent = 0;
    if (EDGE_WRAP_) {
      extent = &extent_;
    }

    for(uint_t i = 0; i != region->size(); ++i)
      {
	// Retrieve nodes at correct 2D position.
	std::vector<Node*> temp = 
	  get_nodes(coordinates - region->get_position(i));
	
	for(std::vector<Node*>::iterator it = temp.begin();
	    it != temp.end(); ++it)
	  {
	    connections->push_back(NodeWrapper(*it, Position<double_t>(i), extent));
	  }
      }

    return connections;
  }

}
