#ifndef LAYER_UNRESTRICTED_H
#define LAYER_UNRESTRICTED_H

/*
 *  layer_unrestricted.h
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

#include "layer_regular.h"
#include "quadtree.h"
#include "position.h"

#include "nest.h"

class TokenArray;

/** @file layer_unrestricted.h
 *  Implements the unrestricted layer structure.
 */

namespace nest
{
  class NodeWrapper;

  /*BeginDocumentation
    Name: layer_unrestricted - spatial distribution of nodes
  
    Description:
    The layer_unrestricted model is a special layer model. The class inherits
    from the layer class. Unlike the layer model the nodes in the unrestricted
    layer class can be placed freely in the 2D space where the layer resides.
    The exact position of the different nodes is given in the dictionary
    passed to the class upon construction.  

    Parameters:

    The unrestricted layer contains the same parameters as the fixed grid 
    layer with the exception of the rows and columns parameters which are 
    replaced with the position parameter.

    ------------------------------------------------------------------------
    Name              Type               Description
    ------------------------------------------------------------------------
    position          arraytype          contains an array describing
                                         the x and y positions of the
                                         layer nodes
    ------------------------------------------------------------------------

    Author: Kittel Austvoll
  */

  /**
   * Class that implements the LayerUnrestricted modeltype. Used to add 
   * spatial information to a group of nodes. This modeltype differs 
   * from the LayerRegular modeltype in that the nodes for this model are not
   * fixed to a uniform grid. The class is still somewhat in the 
   * experimental phase.
   */
  class LayerUnrestricted: public Layer
  {
  public:
    /**
     * Default constructor.
     */
    LayerUnrestricted();

    /**
     * Copy constructor.
     * @param l  reference layer
     */
    LayerUnrestricted(const LayerUnrestricted& l);

    /**
     * Modified copy constructor. Identical to the input layer 
     * with the exception of the node list, which is replaced with 
     * the input nodes.
     */
    LayerUnrestricted(const LayerUnrestricted& l, 
		      const std::vector<Node*>& nodes);

    /**
     * Alternative copy constructor. Converts a regular layer to
     * an unrestricted layer.
     * @param l  reference layer
     */
    LayerUnrestricted(const LayerRegular& l);

    /**
     * Set member variables in layer according to details in input 
     * dictionary. 
     * @param dict  Dictionary containing new values for selected 
     *              layer variables.
     */
    void set_status(const DictionaryDatum&);

    /**
     * Removes node elements in the layer that don't fit the criterias
     * set in the input dictionary.
     * @param dict  Dictionary containing the criterias that the function
     *              extracts nodes based on. See the documentation for
     *              the SLI topology/ConnectLayer function.
     */
    virtual lockPTR<Layer> slice(bool, const DictionaryDatum& options) const;

    /**
     * Retrieve nodes within input region.
     * @param pos     centre of mask region.
     * @param region  region
     * @returns vector of node pointers and node positions.
     */
    virtual lockPTR<std::vector<NodeWrapper> >
      get_pool_nodewrappers(const Position<double_t>& driver_coo,
			    AbstractRegion* region);

    /**
     * @returns TokenArray with layer node positions.
     */
    TokenArray get_points() const;

    /**
     * Function that retrieves a list of the major layer variables.
     * @param dict  Dictionary where the properties of the layer
     *              are stored.
     */
    void get_status(DictionaryDatum& dict) const;

    /**
     * @param lid local id
     * @returns position at selected local id
     */
    const Position<double_t> get_position(index lid) const;
    
    const Position<double_t> compute_displacement(const Position<double_t>& from_pos,
						  const Node& to) const;

  protected:

    //Vector of positions. Should match node vector in Compound.
    std::vector<Position<double_t> > positions_;

    /**
     * Set quadtree settings
     */
    virtual void set_tree_settings(const DictionaryDatum& dict);

    /**
     * Get quadtree settings
     */
    virtual void get_tree_settings(DictionaryDatum& d) const;

    /**
     * Create a quadtree based on the layer structure.
     */
    virtual void make_tree();

  private:
    Quadtree tree_;

    // The variable quadrant_max_nodes_ decides the layout of the 
    // Quadtree structure. This variable can be modified by adding the 
    // "quadrant_max_nodes" parameter to the layer properties. Should 
    // only be used by advanced users or in the code development phase. 
    long_t quadrant_max_nodes_;

    /**
     * Throws an error if the number of nodes in the layer
     * doesn't correspond with the layer dimensions.
     */
    void test_validity() const;

  };

}

#endif
