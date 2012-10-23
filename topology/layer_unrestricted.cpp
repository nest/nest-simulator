/*
 *  layer_unrestricted.cpp
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

#include "layer_unrestricted.h"

#include <limits>
#include <string>

#include "dictutils.h"
#include "exceptions.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "compound.h"

#include "tokenarray.h"
#include "quadtree.h"
#include "quadrant.h"
#include "topology_names.h"
#include "nodewrapper.h"
#include "region.h"
#include "topologyconnector.h"
#include "layer_slice.h"

namespace nest
{
  LayerUnrestricted::LayerUnrestricted():
    Layer(2),
    positions_(),
    tree_(),
    quadrant_max_nodes_(100) // @todo Find the best value for this parameter.
  {}

  LayerUnrestricted::LayerUnrestricted(const LayerUnrestricted& l):
    Layer(l),
    positions_(l.positions_),
    tree_(l.tree_),
    quadrant_max_nodes_(l.quadrant_max_nodes_)
  {
  }

  LayerUnrestricted::LayerUnrestricted(const LayerUnrestricted& l, 
				       const std::vector<Node*>& nodes):
    Layer(l),
    positions_(l.positions_),
    tree_(),
    quadrant_max_nodes_(l.quadrant_max_nodes_)
  {
    nodes_ = nodes;
    make_tree();
  }

  LayerUnrestricted::LayerUnrestricted(const LayerRegular& l):
    Layer(l),
    positions_(),
    tree_(),
    quadrant_max_nodes_(100)
  {
    Position<double_t> dpd = l.get_dpd();

    // Generate position vector based upon fixed grid layer dimensions
    for(index i = 0;i<l.size();++i)
      {
	positions_.push_back(l.get_position(i));
      }

    upper_left_ = 
      Position<double_t>(center_.at(0) - extent_.at(0)/2,
			       center_.at(1) + extent_.at(1)/2);

    // Create layer tree
    make_tree();

  }

  lockPTR<Layer> LayerUnrestricted::
  slice(bool, const DictionaryDatum& options) const
  {
    return lockPTR<Layer>(new LayerSlice<LayerUnrestricted>(*this, options));
  }

  void LayerUnrestricted::set_status(const DictionaryDatum& layer_dict)
  {
    // Layer properties can either be passed inside a sub topology 
    // dictionary or on the upper level of a dictionary
    DictionaryDatum layer_dictionary = new Dictionary();
    if(!updateValue<DictionaryDatum>(layer_dict, names::topology, 
				     layer_dictionary))
      {
	layer_dictionary = layer_dict;
      }

    // Read positions from dictionary
    TokenArray pos;
    if(updateValue<TokenArray>(layer_dictionary, names::positions, pos))
      {
	if(nodes_.size() != pos.size())
	  {
	    std::stringstream expected;
	    std::stringstream got;
	    expected << "position array with length " << nodes_.size();
	    got << "position array with length" << pos.size();
	    throw TypeMismatch(expected.str(), got.str());
	  }

	positions_.clear();

	for(uint_t i = 0; i < pos.size(); ++i) 
	  {
	    std::vector<double_t> point = 
	      getValue<std::vector<double_t> >(pos[i]);
	    
	    if(point.size() < 2)
	      {
		std::stringstream got;
		got << "position array with sub-elements with length " 
		    << point.size();
		throw 
		  TypeMismatch("position array with sub-elements with length above 1",
			       got.str());
	      }

	    positions_.push_back(Position<double_t>(point));
	  }
      }

    set_tree_settings(layer_dictionary);

    Layer::set_status(layer_dictionary);

    make_tree();

    test_validity();
  }

  void LayerUnrestricted::
  set_tree_settings(const DictionaryDatum& layer_dictionary)
  {
    // Set quadrant settings
    if(updateValue<long_t>(layer_dictionary, "quadrant_max_nodes",
				 quadrant_max_nodes_))
      {
	if(quadrant_max_nodes_ <= 0)
	  {
	    throw EntryTypeMismatch("quadrant_max_nodes > 0",
				    "quadrant_max_nodes <= 0");
	  }
      }
  }

  void LayerUnrestricted::get_tree_settings(DictionaryDatum& d) const
  {
    def<long_t>(d, "quadrant_max_nodes", quadrant_max_nodes_);
  }

  void LayerUnrestricted::get_status(DictionaryDatum& dict) const
  {
    Layer::get_status(dict);

    DictionaryDatum d =getValue<DictionaryDatum>((*dict)[names::topology]);

    def2<TokenArray, ArrayDatum>(d, names::positions, get_points());
    get_tree_settings(d);
  }

  void LayerUnrestricted::make_tree()
  {
    tree_ = Quadtree(nodes_, positions_, 
		     Position<double_t>(center_.at(0) - extent_.at(0)/2,
					      center_.at(1) - extent_.at(1)/2), 
		     Position<double_t>(center_.at(0) + extent_.at(0)/2, 
					      center_.at(1) + extent_.at(1)/2),
		     quadrant_max_nodes_);
  }

  lockPTR<std::vector<NodeWrapper> > LayerUnrestricted::
  get_pool_nodewrappers(const Position<double_t>& driver_coo,
			AbstractRegion* region)
  {
    Shift::EDGE_WRAP = EDGE_WRAP_;

    Region* r = dynamic_cast<Region*>(region);

    if(r != NULL)
      {
	// The region needs to be altered (shifted)
	// before calling the get_nodewrappers(..) 
	// function. We perform the changes on a copy
	// of the region object. 
	r = r->copy();
	r->set_anchor(driver_coo);

	if (EDGE_WRAP_)
	  {
	    return tree_.get_nodewrappers(r,&extent_);
	  }
	else
	  {
	    return tree_.get_nodewrappers(r);
	  }
      }
    else
      {
	throw EntryTypeMismatch("unrestricted region", 
				"fixed grid region");
      }
  }

  TokenArray LayerUnrestricted::get_points() const
  {
    TokenArray points;

    for(std::vector<Position<double_t> >::const_iterator it = 
	  positions_.begin(); it != positions_.end(); ++it)
      {
	points.push_back(it->getToken());
      }

    return points;
  }

  const Position<double_t> 
  LayerUnrestricted::get_position(index lid) const
  {
    return positions_.at(lid);
  }

  const Position<double_t> LayerUnrestricted::compute_displacement(const Position<double_t>& from_pos,
								   const Node& to) const
  {
    // make sure node really belongs to this layer
    assert(get_layer(to) == this);

    // obtain "raw" displacement
    const Position<double_t> to_pos = Layer::get_position(to);
    Position<double_t> d = to_pos - from_pos;

    // handle periodic boundary conditions
    if ( EDGE_WRAP_ )
      d.wrap_displacement_max_half(Position<double_t>(extent_));

    return d;
  }

  void LayerUnrestricted::test_validity() const
  {
    if(nodes_.size() != positions_.size())
      {
	throw DimensionMismatch(nodes_.size(), positions_.size());
      }

    Position<double_t> lower_left = 
      Position<double_t>(center_.at(0) - extent_.at(0)/2,
			       center_.at(1) - extent_.at(1)/2);
    
    Position<double_t> upper_right = 
      Position<double_t>(center_.at(0) + extent_.at(0)/2,
			       center_.at(1) + extent_.at(1)/2);

    bool nodes_on_perimeter = false;

    for(std::vector<Position<double_t> >::const_iterator it = 
	  positions_.begin(); it != positions_.end(); ++it)
      {
	if((*it).get_x() > upper_right.get_x() ||
	   (*it).get_y() > upper_right.get_y() ||
	   (*it).get_x() < lower_left.get_x() || 
	   (*it).get_y() < lower_left.get_y())
	  {
	    throw BadProperty("All nodes must be placed inside the layer's extent.");
	  }

    nodes_on_perimeter = nodes_on_perimeter ||
    		it->get_x() == upper_right.get_x() ||
    		it->get_y() == upper_right.get_y() ||
    		it->get_x() == lower_left.get_x() ||
    		it->get_y() == lower_left.get_y() ;

      }

    if ( nodes_on_perimeter && EDGE_WRAP_ )
      throw BadProperty("Some nodes are placed on the perimeter of the extent. "
    		        "This is currently not compatible with periodic boundary conditions.");
  }

}
