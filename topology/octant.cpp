/*
 *  octant.cpp
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

#include "octant.h"

#include <vector>
#include "nest.h"

#include "exceptions.h"

#include <iostream>

#include "nodewrapper.h"
#include "region.h"

#include "nestmodule.h"

namespace nest
{
  Octant::Octant():
    Quadrant(),
    children_()
  {
//     children_.clear();
  }

  // upper left and lower right should be at opposite depths.
  
  Octant::Octant(const Position<double_t>& lower_left, 
		 const Position<double_t>& upper_right,
		 index max_nodes):
    Quadrant(lower_left, upper_right, max_nodes),
    children_()
  {
 //    children_.clear();
  }

  Octant::~Octant()
  {
    for(std::vector<Octant*>::iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	delete *it;
      }
  }

  void Octant::split()
  {
    assert(children_.size() == 0);

    leaf_ = false;

    Position<double_t> dist =
      Position<double_t>(upper_right_ - lower_left_).absolute()/
      Position<double_t>(2.0);
    
    for(index i=0; i<8; ++i)
      {
	Position<double_t> lower_left = lower_left_;
	Position<double_t> upper_right = upper_right_;
	
	switch(i)
	  {
	    // Upper depth
	  case 0:
	    // Upper left box
	    lower_left += dist*Position<double_t>(0, 1, 1);
	    upper_right -= dist*Position<double_t>(1, 0, 0);
	    break;
	  case 1:
	    // Lower left box
	    lower_left += dist*Position<double_t>(0, 0, 1);
	    upper_right -= dist*Position<double_t>(1, 1, 0);
	    break;
	  case 2:
	    // Upper right box
	    lower_left += dist*Position<double_t>(1, 1, 1);
	    // 	    upper_right;
	    break;
	  case 3:
	    // Lower right box
	    lower_left += dist*Position<double_t>(1, 0, 1);
	    upper_right -= dist*Position<double_t>(0, 1, 0);
	    break;
	    // Lower depth
	  case 4:
	    // Upper left box
	    lower_left += dist*Position<double_t>(0, 1, 0);
	    upper_right -= dist*Position<double_t>(1, 0, 1);
	    break;
	  case 5:
	    // Lower left box
	    // 	    lower_left;
	    upper_right -= dist*Position<double_t>(1, 1, 1);
	    break;
	  case 6:
	    // Upper right box
	    lower_left += dist*Position<double_t>(1, 1, 0);
	    upper_right -= dist*Position<double_t>(0, 0, 1);;
	    break;
	  case 7:
	    // Lower right box
	    lower_left += dist*Position<double_t>(1, 0, 0);
	    upper_right -= dist*Position<double_t>(0, 1, 1);
	    break;
	  default:
	    assert(false);
	    break;
	  }

	children_.push_back(new Octant(lower_left, upper_right, max_nodes_));
      }
    for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	it != nodes_.end(); ++it)
      {
	insert(*it);
      }

    nodes_.clear();
  }
  
  void Octant::get_nodes(lockPTR<std::vector<NodeWrapper> > nodes,
			 Volume const * const box,
			 std::vector<double_t> * extent)
  {
    // Check everything below

    Volume rect(lower_left_, upper_right_);
    
    if(box->within_range(lower_left_) && 
       box->within_range(upper_right_))
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
		nodes->push_back(NodeWrapper(*it_sub, (*it).get_position(), extent));
	      }
	  }
      }
    else if(box->outside(rect))
      {
	return;
      }
    else
      {
	for(std::vector<NodeWrapper>::iterator it = nodes_.begin();
	    it != nodes_.end(); ++it)
	  {
	    if(box->within_range((*it).get_position()))
	      {
		Compound *subnet = dynamic_cast<Compound*>(it->get_node());
		assert(subnet != 0);
		// Slicing of layer before calling ConnectLayer function
		// assures that the subnet isn't nested.
		for(std::vector<Node*>::iterator it_sub = subnet->begin();
		    it_sub != subnet->end(); ++it_sub)
		  {
		    if(*it_sub != NULL)
		      {
			nodes->push_back(NodeWrapper(*it_sub, (*it).get_position(), extent));
		      }
		  }
	      }
	  }
      }

    return;
  }

  void Octant::find_region(const Position<double_t>& upper_left, 
			   Quadrant* lower_right, 
			   std::list<Octant>& oct_region, 
			   bool within_region)
  {
    if(leaf_)
      {
	oct_region.push_back(*this);
	return;
      }

    for(std::vector<Octant*>::iterator it = children_.begin();
	it != children_.end(); ++it)
      {
	if((*it) == lower_right)
	  {
	    (*it)->find_region(upper_left, lower_right, 
			       oct_region, false);
	    return;
	  }
	if(!within_region && (*it)->hit(upper_left))
	  {
	    (*it)->find_region(upper_left, lower_right, 
			       oct_region, false);
	    within_region = true;
	    continue;
	  }
	
	if(within_region)
	  {
	    (*it)->find_region(upper_left, lower_right,
			       oct_region, true);
	  }
      }
  }

}


