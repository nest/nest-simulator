#ifndef LAYER_REGULAR_H
#define LAYER_REGULAR_H

/*
 *  layer_regular.h
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

#include <map>
#include <vector>

#include "compound.h"
#include "leaflist.h"
#include "iostreamdatum.h"
#include "nest.h"

#include "layer.h"
#include "position.h"

/** @file layer_regular.h
 *  Implements the regular 2d layer structure.
 */

namespace nest
{
  template <class DataType>
    class Position;

  class TopologyConnector;
  class AbstractRegion;
  class NodeWrapper;

  /*BeginDocumentation
    Name: layer - spatial distribution of nodes
  
    Description:
    The layer model is a special subnet model. The class inherits
    from the subnet class. 

    Parameters:

    Parameters that can be accessed via the GetStatus and SetStatus functions:
    ------------------------------------------------------------------------
    Name              Type               Description
    ------------------------------------------------------------------------
    extent*           arraytype          [width height]
    center^           arraytype          [x_center y_center]
    edge_wrap^        booltype           boundary condition
    ------------------------------------------------------------------------

    Parameters that can be accessed via the GetStatus function (in addition
    to the parameters above):
    ------------------------------------------------------------------------
    rows*             integertype        node rows
    columns*          integertype        node columns
    depth             integertype        number of nodes at each 2D position
    ------------------------------------------------------------------------

    Parameters that should be included in the CreateLayer dictionary (but
    can't be accessed with SetStatus or GetStatus):
    ------------------------------------------------------------------------
    elements*         Literal/Procedure  nodes at each 2D position
    ------------------------------------------------------------------------

    *All these variables have to be included in the CreateLayer dictionary.
    ^These variables can be included in the CreateLayer dictionary.

    The element variable can be set in the following ways:
    /modeltype                   - create a layer consisting of single
                                   neurons
    {procedure}                  - creates a compound of nodes decided 
                                   by the procedure at each position in 
				   the layer

    Deprecated syntax:				   

    [/iaf_neuron /iaf_psc_alpha] - create a compound of one iaf_neuron and
                                   one iaf_psc_alpha
    [/iaf_neuron 2]              - create a compound of two iaf_neurons
    or a combination of the two (e.g. [/iaf_psc_alpha [/iaf_psc_alpha 3]])

    Author: Kittel Austvoll
  */


  /**
   * Class that implements the LayerRegular modeltype. Used to add spatial
   * information to a group of nodes.
   */
  class LayerRegular: public Layer
    {
    public:
    
    /**
     * Creates an empty layer.
     */
    LayerRegular();

    /**
     * Copy constructor.
     * @param layer  Reference to layer being copied.
     */
    LayerRegular(const LayerRegular& layer);

    /**
     * Modified copy constructor. Replaces original set of 
     * layer nodes with a new set.
     * @param layer  Reference to layer being copied.
     * @param nodes  New vector of layer nodes.
     */
    LayerRegular(const LayerRegular& layer, const std::vector<Node*>& nodes);

    virtual ~LayerRegular();

    /**
     * Removes node elements in the layer that don't fit the criterias
     * set in the input dictionary (such as model type and depth). The
     * modified layer is returned by the function, while the original
     * layer remains unchanged.
     * @param layer_connection_dict Dictionary containing the criterias 
     *              that the function extracts nodes based on. See the 
     *              documentation for the SLI topology/ConnectLayer 
     *              function.
     * @returns a copy of the layer where only the selected nodes exist
     */
    virtual lockPTR<Layer> slice(bool,
				 const DictionaryDatum&) const;

    /**
     * Set member variables in layer according to details in input 
     * dictionary. 
     * @param dict  Dictionary containing new values for selected 
     *              layer variables.
     */
    virtual void set_status(const DictionaryDatum& c_dict);

    /**
     * Function that retrieves a list of the major layer variables.
     * @param dict  Dictionary where the properties of the layer
     *              are stored.
     */
    virtual void get_status(DictionaryDatum& dict) const;

    /**
     * Returns the node density of the layer.
     * @returns The spatial node density of the layer. The density is
     *          passed as a Position structure, where each dimension
     *          in the Position structure corresponds to a dimension 
     *          in the layer space.
     */
    Position<double_t> get_dpd() const;

    /**
     * @returns The number of columns in the layer
     */
    uint_t get_columns() const;

    /**
     * @returns The number of rows in the layer
     */
    uint_t get_rows() const;

    /**
     * Function that retrieves nodes in the layer that covers a selected 
     * mask region centered around an input driver position.
     * @param driver  Position of the center of the mask region in the pool
     *                topological space (i.e. the position of the driver
     *                node overlapping this center).
     * @param region  Mask region where nodes are retrieved from.
     * @returns vector containing the node pointers and the node positions 
     *          of the nodes covering the input region centered around the 
     *          input driver coordinate.
     */
    virtual lockPTR<std::vector<NodeWrapper> >
      get_pool_nodewrappers(const Position<double_t>& driver_coo,
			    AbstractRegion* region);

    /**
     * Returns node pointer for given [row, column] position in layer. 
     * @param coordinates  row and column position 
     * @returns node pointer
     */
    Node* 
      get_node(const Position<int_t>& coordinates) const;

    using Layer::get_position;

    /**
     * Returns 2D position of node recognized by current Layer/Compound
     * local id value.
     * @param lid  local id of node
     * @returns 2D position of node
     */
    virtual const Position<double_t> get_position(index lid) const;

    const Position<double_t> compute_displacement(const Position<double_t>& from_pos,
						  const Node& to) const; 

    /**
     * Converts between layerspace and topologicalspace coordinates.
     * @param discrete  Discrete layerspace position.
     * @returns Continuous position in topological space.
     */
    const Position<double_t> 
      layer2spatial(const Position<double_t>& discrete) const; 

    /**
     * Converts between topologicalspace and layerspace coordinates.
     * @param spatial  Continuous topological space position.
     * @returns Discrete position in layerspace.
     */
    const Position<int_t> 
      spatial2layer(const Position<double_t>& spatial) const;

    /**
     * Returns nodes at a given 2D discrete layerspace position.
     * @param pos  Discrete position in layerspace.
     * @returns vector of node pointers at the depth column covering
     *          the input position.
     */
    std::vector<Node*> get_nodes(Position<int_t> pos) const;

    protected:
    int_t rows_;
    int_t columns_;

    // Derived parameters.
    Position<double_t> dpd_; // dots per degree (node density)

    /**
     * Calculates the 2D node resolution of the layer.
     * @returns Spatial 2D node resolution of layer.
     */
    const Position<double_t> calc_dpd_();

    /**
     * Throws an error if the number of nodes in the layer
     * doesn't correspond with the layer dimensions.
     */
    virtual void test_validity() const;
    
  };

}
#endif
