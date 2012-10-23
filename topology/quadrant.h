#ifndef QUADRANT_H
#define QUADRANT_H

/*
 *  quadrant.h
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

#include <vector>
#include "nest.h"
#include "node.h"

#include "position.h"

/** @file quadrant.h
 *  Implements the layer structure.
 */

namespace nest
{
  template <class DataType>
    class Position;

  class Region;
  class NodeWrapper;

  /**
   * A Quadrant object represents a node (branch or leaf) in a 
   * Quadtree structure. Any quadrant covers a specific region 
   * in a 2D space. A leaf quadrant contains a list of Node
   * pointers and their corresponding positions. A branch 
   * quadrant contains a list of four other quadrants, each 
   * covering a region corresponding to the upper-left, 
   * lower-left, upper-right and lower-left corner of their mother
   * quadrant.
   *
   */
  class Quadrant
  {
  public:
    /**
     * Default constructor
     */
    Quadrant();
    
    /**
     * Create a Quadrant that covers the region defined by the two 
     * input positions.
     * @param lower_left  Lower left corner of quadrant.
     * @param upper_right Upper right corner of quadrant.
     * @param max_nodes maximum number of nodes allowed in each 
     *               quadtree leaf region. It is not necessary
     *               to adjust this parameter for most users. In a 
     *               few cases it might lead to improved performance
     *               of the quadtree structure if this variable is
     *               altered.
     */
    Quadrant(const Position<double_t>& pos, 
	     const Position<double_t>&,
	     index max_nodes);
    
    virtual ~Quadrant();
       
    /**
     * @returns number of nodes in quadrant. Should be 0 for non-leaf
     * quadrants.
     */
    index size() const;

    /**
     * Find leaf quadrant covering input position.
     * @param pos Input position.
     * @returns quadrant pointer.
     */
    Quadrant* find(const Position<double_t>&);

    /**
     * Traverse quadtree structure from current quadrant. 
     * Find all leaves that covers the input region. 
     * @param upper_left    Position of upper_left corner
     *                      of scope region
     * @param lower_right   Quadrant pointer to quadrant
     *                      covering lower right corner 
     *                      of scope region.
     * @param quad_region   Vector where the leaf quadrants
     *                      are stored.
     * @param within_region true if Quadrant covering upper
     *                      left corner have been found.
     */
    void find_region(const Position<double_t>& upper_left, 
		     const Quadrant* lower_right, 
		     std::vector<Quadrant>& quad_region, 
		     bool within_region) const;

    /**
     * Traverse quadtree structure from current quadrant. 
     * Inserts node in correct leaf in quadtree.
     * @param node NodeWrapper structure containing pointer
     *             to node that should be inserted in 
     *             tree.
     */
    virtual void insert(NodeWrapper);

    /**
     * @returns true if quadrant is a leaf.
     */
    bool is_leaf() const;

    /**
     * Change a leaf quadrant to a regular quadrant with four 
     * children regions.
     */
    virtual void split();

    /**
     * @returns member nodes in quadrant
     */
    std::vector<NodeWrapper> get_nodes() const;

    /**
     * Retrieve nodes covering input region. Shifts these nodes by 
     * the value given as input.
     * @param nodes  vector where output node pointers and positions are
     *               stored
     * @param box    region 
     * @param shift  2D value which the output nodes should be shifted by
     * @param extent layer extent for periodic boundary conditions, or 0 otherwise
     */
    virtual void get_nodes(lockPTR<std::vector<NodeWrapper> > nodes,
			   Region const * const box,
			   const Position<double_t>& shift,
                           std::vector<double_t> * extent=0);

    /**
     * Traverse quadtree structure starting from current quadrant
     * and print nodes of all leaves. Test function for developers.
     */
    void print_leaves();

    /**
     * Print nodes of current quadrant. Test function for developers.
     */
    void print_nodes();

    /**
     * @returns lower left position of quadrant
     */
    Position<double_t> get_lower_left() const;

    /**
     * @returns upper right position of quadrant
     */
    Position<double_t> get_upper_right() const;

    /**
     * @returns global center (anchor) of quadrant
     */
    Position<double_t> get_center() const;

  protected:

    // Bounds of quadrant
    Position<double_t> lower_left_;
    Position<double_t> upper_right_;

    // True if quadrant is a leaf
    bool leaf_;

    // Quadrant nodes (empty if quadrant is a branch)
    std::vector<NodeWrapper> nodes_;

    //The maximum number of nodes allowed in a single quadrant.
    index max_nodes_;

    /**
     * @param pos Position in a 2D space.
     * @returns true if quadrant covers input position.
     */
    bool hit(const Position<double_t>& pos) const;

  private:

    // Quadrant children (empty if quadrant is a leaf)
    std::vector<Quadrant*> children_;

  };

}

#endif
