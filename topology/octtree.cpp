/*
 *  octtree.cpp
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

#include "octtree.h"

#include <vector>
#include "nest.h"

#include "position.h"

#include "octant.h"

#include "nodewrapper.h"
#include "region.h"

#include <iostream>

namespace nest
{

  Octtree::Octtree()
  {
    root_ = new Octant();
  }

  Octtree::Octtree(const std::vector<Node*>& nodes, 
		   const std::vector<Position<double_t> >& pos, 
		   const Position<double_t>& lower_left, 
		   const Position<double_t>& upper_right,
		   index octant_max_nodes)
  {
    // Find the best value for this parameter.
//     Octant::MAX_NODES = 2;

    // Create root octant covering given input region
    root_ = new Octant(lower_left, upper_right,
		       octant_max_nodes);

    // Insert nodes in the octtree, new octants are
    // created automatically when needed.
    for(size_t i = 0; i < nodes.size(); ++i)
      {
	insert(nodes.at(i), pos.at(i));
      }

    // TODO: Remove empty leaves here for optimal performance
  }

  Octtree::~Octtree()
  {
    //     delete root_;
  }

  void Octtree::insert(Node* node, 
		       Position<double_t> pos)
  {
    // Octant::insert(..) traverses the octtree structure
    // and inserts the node at the appropriate point
    root_->insert(NodeWrapper(node, pos));
  }

  void Octtree::print()
  {
    std::cout << "Octtree: " << std::endl;

    // Traverse octtree structure and print leaves to std::cout.
    root_->print_leaves();
  }

  std::vector<Node*> 
  Octtree::get_nodes(Position<double_t> lower_left, 
		     Position<double_t> upper_right) const
  {
    std::vector<Node*> nodes;

    // new Region() is deleted automatically by the 
    // get_nodewrappers() function.
    lockPTR<std::vector<NodeWrapper> > nodewrappers = 
      get_nodewrappers(new Volume(lower_left, upper_right));

    for(std::vector<NodeWrapper>::iterator it = nodewrappers->begin();
	it != nodewrappers->end(); ++it)
      {
	nodes.push_back((*it).get_node());
      }
    return nodes;
  }

  lockPTR<std::vector<NodeWrapper> >
  Octtree::get_nodewrappers(Volume const * const box, std::vector<double_t> * extent) const
  {
    lockPTR<std::vector<NodeWrapper> > nodes(new std::vector<NodeWrapper>());

    // Add edge truncate (and edge wrap) here.
    
    // Get a list of octants overlapping or almost overlapping
    // the given region.
    Position<double_t> lower_left = box->get_lower_left();
    Position<double_t> upper_right = box->get_upper_right();
    
    std::list<Octant> region = 
      get_leaves(Position<double_t>(lower_left.get_x(), 
					  upper_right.get_y(),
					  upper_right.get_z()),
		 Position<double_t>(upper_right.get_x(),
					  lower_left.get_y(),
					  lower_left.get_z()));
    
    // Iterate through the octant list and check which nodes
    // that overlap the input region.
    for(std::list<Octant>::iterator it_sub = region.begin();
	it_sub != region.end(); ++it_sub)
      {
	it_sub->get_nodes(nodes, box, extent);
      }
    
    // Deleting temporary input box
    delete box;
    
    return nodes;
  }
  
  std::list<Octant> 
  Octtree::get_leaves(const Position<double_t>& upper_left, 
		      const Position<double_t>& lower_right) const
  {
    std::list<Octant> region;
   
    // Find all octants in between the two input positions.
    root_->find_region(upper_left, root_->find(lower_right),
		       region, false);
    
    return region;
  }

}
