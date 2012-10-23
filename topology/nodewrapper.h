#ifndef NODEWRAPPER_H
#define NODEWRAPPER_H

/*
 *  nodewrapper.h
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

#include "node.h"
#include "position.h"

/** @file nodewrapper.h
 *  Implements the NodeWrapper structure.
 */

namespace nest
{
  /**
   * Class that stores information about node pointer and its
   * position.
   */
  class NodeWrapper
  {
  private:
    Node* node_;
    Position<double_t> pos_;

    // Exent of layer is necessary for evaluating parameter functions
    // when periodic boundary conditions are in effect. A null pointer
    // signals that periodic boundary conditions are not used.
    std::vector<double_t>* extent_;

  public:
    /**
     * Creates a NodeWrapper for a given node and position.
     * @param node    node pointer
     * @param pos     position of the node
     * @param extent  layer extent for periodic boundary conditions, or 0 otherwise
     */
    NodeWrapper(Node* node, const Position<double_t>& pos, std::vector<double_t> *extent=0):
      node_(node),
      pos_(pos),
      extent_(extent)
      {}
    
    /**
     * Returns the node pointer.
     */
    Node* get_node() const
      {
	return node_;
      }
    
    /**
     * Returns the position of the node.
     */
    Position<double_t> get_position() const
      {
	return pos_;
      }

    /**
     * Returns the extent of the layer containing the node if periodic
     * boundary conditions are in effect, or a null pointer otherwise.
     */
    std::vector<double_t>* get_extent() const
      {
	return extent_;
      }

    /**
     * Checks if input node is a subnet and returns a list of nodes in this
     * subnet together with the given position.
     * @param n      node pointer (should be subnet)
     * @param pos    position
     * @param extent layer extent for periodic boundary conditions, or 0 otherwise
     * @returns list of node pointers and positions
     */
    static lockPTR<std::vector<NodeWrapper> >
    get_nodewrappers(Node* n, const Position<double_t>& pos,std::vector<double_t> *extent=0);


  };
}

#endif

