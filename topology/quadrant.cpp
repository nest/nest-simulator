/*
 *  quadrant.cpp
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

#include "quadrant.h"

#include <vector>
#include <iostream>

#include "nest.h"
#include "exceptions.h"
#include "nestmodule.h"

#include "nodewrapper.h"
#include "region.h"

namespace nest
{
  Quadrant::Quadrant():
    lower_left_(0,0,0),
    upper_right_(0,0,0),
    leaf_(true),
    nodes_(),
    max_nodes_(100), // @todo find an appropriate value for this parameter
    children_()
  {
  }

  Quadrant::Quadrant(const Position<double_t>& lower_left, 
		     const Position<double_t>& upper_right,
		     index max_nodes):
    lower_left_(lower_left),
    upper_right_(upper_right),
    leaf_(true),
    nodes_(),
    max_nodes_(max_nodes),
    children_()
  {
  }

  Quadrant::~Quadrant()
  {
    for(std::vector<Quadrant*>::iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	delete *it;
      }
  }

  index Quadrant::size() const
  {
    return nodes_.size();
  }

  bool Quadrant::hit(const Position<double_t>& pos) const
  {
    return pos.within_range(lower_left_, upper_right_);
  }

  void Quadrant::split()
  {
    assert(children_.size() == 0);

    leaf_ = false;

    // Create four new children quadrants
    for(index i=0; i<4; ++i)
      {
	Position<double_t> dist =
	  Position<double_t>(upper_right_ - lower_left_).absolute()/2.0;

	Position<double_t> lower_left = lower_left_;
	Position<double_t> upper_right = upper_right_;
	
	switch(i)
	  {
	  case 0: // first quadrant
	    lower_left += dist*Position<double_t>(0, 1);
	    upper_right -= dist*Position<double_t>(1, 0);
	    break;
	  case 1: // second quadrant
	    // 	    lower_left;
	    upper_right -= dist;
	    break;
	  case 2: // third quadrant
	    lower_left += dist;
	    // 	    upper_right;
	    break;
	  case 3: // fourth quadrant
	    lower_left += dist*Position<double_t>(1, 0);
	    upper_right -= dist*Position<double_t>(0, 1);
	    break;
	  default:
	    assert(false);
	    break;
	  }

	children_.push_back(new Quadrant(lower_left, upper_right, max_nodes_));
      }

    // Move quadrant nodes to the new leaf quadrants
    for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	it != nodes_.end(); ++it)
      {
	insert(*it);
      }

    nodes_.clear();
  }

  void Quadrant::insert(NodeWrapper node)
  {
    // PROBLEM IF MORE THAN MAX NODES ARE ON EXACTLY ONE POSITION OR 
    // EXTREMELY CLOSE TO EACH OTHER IN SPACE. @todo Fix error in 
    // Quadrant::insert(..)

    if(!is_leaf()) // Only insert nodes in leaf
      {
	find(node.get_position())->insert(node);
      }
    else if(size() != max_nodes_)
      {
	nodes_.push_back(node);
      }
    else if(size() == max_nodes_) // Create new quadrants if quadrant is full
      {
	split();
	insert(node);
      }
  }

  bool Quadrant::is_leaf() const
  {
    return leaf_;
  }

  Quadrant* Quadrant::find(const Position<double_t>& pos)
  {
    // When we reach the leaf quadrant that overlaps the input
    // position we stop the recursive function.
    if(leaf_)
      {
	return this;
      }

    // Iterate through children and find quadrant that the input
    // position overlaps.
    for(std::vector<Quadrant*>::iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	if((*it)->hit(pos))
	  {
	    return (*it)->find(pos);
	  }
      }

    NestModule::get_network().message(SLIInterpreter::M_WARNING, 
			    "Topology", "There might be a problem "
			    "with your mask or layer dimensions. "
			    "If you're using edge wrap please make "
			    "sure that you didn't put nodes on the "
			    "layer edge.");
 
    throw DimensionMismatch();

  }

  void Quadrant::find_region(const Position<double_t>& upper_left, 
			     const Quadrant* lower_right, 
			     std::vector<Quadrant>& quad_region, 
			     bool within_region) const
  {
    // Return all quadrants in the quadtree between the quadrant 
    // overlapping the input upper left position and the input
    // lower right quadrant pointer. This selection have to be 
    // refined by functions using the output quadrant list.
    
    if(leaf_)
      {
	quad_region.push_back(*this);
	return;
      }

    for(std::vector<Quadrant*>::const_iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	if((*it) == lower_right)
	  {
	    // End recursive function if input lower right quadrant
	    // pointer have been reached.
	    (*it)->find_region(upper_left, lower_right, 
			       quad_region, false);
	    return;
	  }
	if(!within_region && (*it)->hit(upper_left))
	  {
	    // Find upper left position quadrant if this
	    // haven't been found yet.
	    (*it)->find_region(upper_left, lower_right, 
			       quad_region, false);
	    within_region = true;
	    continue;
	  }
	
	if(within_region)
	  {
	    // Check if quadrant leaves are between upper left
	    // quadrant and lower right quadrant
	    (*it)->find_region(upper_left, lower_right,
			       quad_region, true);
	  }
      }
  }

  void Quadrant::get_nodes(lockPTR<std::vector<NodeWrapper> > nodes,
			  Region const * const box,
			  const Position<double_t>& shift,
                          std::vector<double_t> * extent)
  {
    Region rect(lower_left_-shift, upper_right_-shift);
    
    // Check if quadrant is fully inside input region.
    if(box->within_range(rect))
      {
	for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	    it != nodes_.end(); ++it)
	  {
	    Compound *subnet = dynamic_cast<Compound*>(it->get_node());
	    assert(subnet != 0);
	    // Slicing of layer before calling ConnectLayer function
	    // assures that the subnet isn't nested.
	    for(std::vector<Node*>::iterator it_sub = subnet->begin();
		it_sub != subnet->end(); ++it_sub)
	      {
		nodes->push_back(NodeWrapper(*it_sub, (*it).get_position()-shift, extent));
	      }
	  }
      }
    // Check if quadrant is fully outside the input region.
    else if(box->outside(rect))
      {
	return;
      }
    // Quadrant intersects region so we have to check every node
    // in the quadrant manually.
    else
      {
	for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	    it != nodes_.end(); ++it)
	  {
	    if(box->within_range((*it).get_position()-shift))
	      {
		Compound *subnet = 
		  dynamic_cast<Compound*>(it->get_node());

		assert(subnet != 0);
		// Slicing of layer before calling ConnectLayer function
		// assures that the subnet isn't nested.
		for(std::vector<Node*>::iterator it_sub = subnet->begin();
		    it_sub != subnet->end(); ++it_sub)
		  {
		    if(*it_sub != NULL)
		      {
			nodes->push_back(NodeWrapper(*it_sub, (*it).get_position()-shift, extent));
		      }
		  }

// 		nodes.push_back(NodeWrapper((*it).node_, (*it).pos_-shift));
	      }
	  }
      }

    return;
  }

  std::vector<NodeWrapper> Quadrant::get_nodes() const
  {
    return nodes_;
  }

  void Quadrant::print_leaves()
  {
    if(leaf_)
      {
	print_nodes();
	return;
      }
    
    for(std::vector<Quadrant*>::iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	(*it)->print_leaves();
      }
    return;
  }

  void Quadrant::print_nodes()
  {
    assert(leaf_ == true);

    std::cout << "LL: (" << lower_left_.get_x() << ", " 
	      << lower_left_.get_y() << ", " << lower_left_.get_z() << ") ";

    std::cout << "UR: (" << upper_right_.get_x() << ", " 
	      << upper_right_.get_y() << ", " << upper_right_.get_z() << ") ";

    std::cout << "[";

    for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	it != nodes_.end(); ++it)
      {
	std::cout << (*it).get_node()->get_gid() << " ";
      }

    std::cout << "]" << std::endl;
  }

  Position<double_t> Quadrant::get_lower_left() const
  {
    return lower_left_;
  }

  Position<double_t> Quadrant::get_upper_right() const
  {
    return upper_right_;
  }

  Position<double_t> Quadrant::get_center() const
  {
    return Position<double_t>(lower_left_ + upper_right_)/2.0;
  }

}
