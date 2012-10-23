/*
 *  quadtree.cpp
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

#include "quadtree.h"

#include <vector>
#include <iostream>

#include "nest.h"

#include "position.h"
#include "quadrant.h"
#include "nodewrapper.h"
#include "region.h"

namespace nest
{

  Quadtree::Quadtree()
  {
    root_ = lockPTR<Quadrant>();
  }

  Quadtree::Quadtree(const std::vector<Node*>& nodes, 
		     const std::vector<Position<double_t> >& pos, 
		     const Position<double_t>& lower_left, 
		     const Position<double_t>& upper_right,
		     index quadrant_max_nodes)
  {
    // Create root quadrant covering given input region
    root_ = lockPTR<Quadrant>(new Quadrant(lower_left, 
					   upper_right, 
					   quadrant_max_nodes));

    // Insert nodes in the quadtree, new quadrants are
    // created automatically when needed.
    for(size_t i = 0; i < nodes.size(); ++i)
      {
	insert(nodes.at(i), pos.at(i));
      }

    // @todo Remove empty leaves here for optimal performance, if
    // tree is poorly balanced.
  }

  Quadtree::~Quadtree()
  {
  }

  void Quadtree::insert(Node* node, 
			Position<double_t> pos)
  {
    // Quadrant::insert(..) traverses the quadtree structure
    // and inserts the node at the appropriate point
    root_->insert(NodeWrapper(node, pos));
  }

  void Quadtree::print()
  {
    std::cout << "Quadtree: " << std::endl;

    // Traverse quadtree structure and print leaves to std::cout.
    root_->print_leaves();
  }

  std::vector<Node*> Quadtree::find(Position<double_t> pos)
  {
    std::vector<Node*> nodes;

    // Retrieve list of nodes and positions in correct quadrant.
    std::vector<NodeWrapper> nodewrappers = 
      root_->find(pos)->get_nodes();

    // Convert NodeWrappers to Node pointers.
    for(std::vector<NodeWrapper>::iterator it = nodewrappers.begin();
	it != nodewrappers.end(); ++it)
      {
	nodes.push_back((*it).get_node());
      }

    return nodes;
  }

  lockPTR<std::vector<NodeWrapper> >
  Quadtree::get_nodewrappers(Region const * const box, std::vector<double_t> * extent) const
  {
    lockPTR<std::vector<NodeWrapper> > nodes(new std::vector<NodeWrapper>());

    // Split box into shifted sub regions covering the quadtree space 
    // if edge wrap is used. Truncate box otherwise. The result of the
    // splitting is stored in the boxes vector.
    std::vector<Shift> boxes;

    Shift(*box).split_box(boxes, 
			  root_->get_lower_left().get_x(), 
			  root_->get_lower_left().get_y(),
			  root_->get_upper_right().get_x(),
			  root_->get_upper_right().get_y(),
			  0, 0);

    for(std::vector<Shift>::iterator it = boxes.begin(); 
	it != boxes.end(); ++it)
      {
	// Get a list of quadrants overlapping or almost overlapping
	// the given region.
	std::vector<Quadrant> region = 
	  get_leaves(Position<double_t>(it->get_lower_left().get_x(), 
					      it->get_upper_right().get_y()),
		     Position<double_t>(it->get_upper_right().get_x(),
					      it->get_lower_left().get_y()));
	
	// Iterate through the quadrant list and check which nodes
	// that overlap the input region.
 	for(std::vector<Quadrant>::iterator it_sub = region.begin();
	    it_sub != region.end(); ++it_sub)
	  {
	    it_sub->get_nodes(nodes, box, it->get_shift(), extent);
	  }
      }

     // Deleting temporary input box
     delete box;

     return nodes;
  }

  std::vector<Quadrant> 
  Quadtree::get_leaves(const Position<double_t>& upper_left, 
		       const Position<double_t>& lower_right) const
  {
    std::vector<Quadrant> region;
   
    // Find all quadrants in between the two input positions.
    root_->find_region(upper_left, root_->find(lower_right),
		       region, false);
    
    return region;
  }

}
