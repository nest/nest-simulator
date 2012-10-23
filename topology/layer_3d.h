#ifndef LAYER_3D_H
#define LAYER_3D_H

/*
 *  layer_3d.h
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

#include "compound.h"

#include "layer_unrestricted.h"
#include "quadtree.h"
#include "octtree.h"
#include "position.h"

#include "nest.h"

class TokenArray;

/** @file layer_3d.h
 *  Implements the unrestricted layer structure.
 */

namespace nest
{
  class NodeWrapper;

  /*BeginDocumentation
    Name: layer_3d - spatial distribution of nodes
  
    Description:
    The layer_3d model is a special layer model. The class inherits
    from the layer class. Unlike the layer model the nodes in the 3d
    layer class can be placed freely in the 2D space where the layer resides.
    The exact position of the different nodes is given in the dictionary
    passed to the class upon construction.  

    Parameters:

    The 3d layer contains the same parameters as the fixed grid 
    layer with the exception of the rows and columns parameters which are 
    replaced with the position parameter.

    ------------------------------------------------------------------------
    Name              Type               Description
    ------------------------------------------------------------------------
    position          arraytype          contains an array describing
                                         the x, y and z positions of the
                                         layer nodes
    ------------------------------------------------------------------------

    Author: Kittel Austvoll
  */

  /**
   * Class that implements the Layer3d modeltype. Used to add 
   * spatial information to a group of nodes. This modeltype differs 
   * from the LayerRegular modeltype in that the nodes for this model are not
   * fixed to a uniform grid. The class is still somewhat in the 
   * experimental phase.
   */
  class Layer3D: public LayerUnrestricted
  {
  public:
    /**
     * Default constructor.
     */
    Layer3D();

    /**
     * Copy constructor.
     * @param l  reference layer
     */
    Layer3D(const Layer3D& l);

    /**
     * Modified copy constructor. Identical to the input layer 
     * with the exception of the node list, which is replaced with 
     * the input nodes.
     */
    Layer3D(const Layer3D& l, 
	    const std::vector<Node*>& nodes);

    /**
     * Removes node elements in the layer that don't fit the criterias
     * set in the input dictionary.
     * @param dict  Dictionary containing the criterias that the function
     *              extracts nodes based on. See the documentation for
     *              the SLI topology/ConnectLayer function.
     */
    lockPTR<Layer> slice(bool, const DictionaryDatum& options) const;

    /**
     * Set octtree settings
     */
    void set_tree_settings(const DictionaryDatum& dict);

    /**
     * Get octtree settings
     */
    void get_tree_settings(DictionaryDatum& d) const;

    /**
     * Create a quadtree based on the layer structure.
     */
    void make_tree();

    /**
     * Retrieve nodes within circular region defined in input mask.
     * @param pos  centre of circular mask region.
     * @param box  Mask where the region dimensions are stored.
     * @returns vector of node pointers.
     */
    lockPTR<std::vector<NodeWrapper> >
      get_pool_nodewrappers(const Position<double_t>& driver_coo,
			    AbstractRegion* region);

  private:
    Octtree tree_;

  };

}

#endif
