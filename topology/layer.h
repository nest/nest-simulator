#ifndef LAYER_H
#define LAYER_H

/*
 *  layer.h
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
    Authors: Kittel Austvoll, Håkon Enger

*/

#include "compound.h"
#include "iostreamdatum.h"
#include "nest.h"

#include "position.h"
#include "nodewrapper.h"
#include "region.h"

#include <iostream>

/** @file layer.h
 *  Implements the abstract layer interface.
 */

namespace nest
{
  template <class DataType>
    class Position;

  /**
   * Abstract base class for the Layer* modeltypes used to add spatial
   * information to a group of nodes.
   */
  class Layer: public Compound
  {
  public:

    /**
     * Creates an empty layer.
     * @param dim The number of dimensions of this layer.
     */
    Layer(int dim);

    /**
     * Copy constructor.
     */
    Layer(const Layer &);

    /**
     * Virtual destructor.
     */
    virtual ~Layer();

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
				 const DictionaryDatum&) const = 0;

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
     * @returns The upper left position of the layer
     */
    Position<double_t> get_upper_left() const;

    /**
     * @returns extent of layer.
     */
    const std::vector<double_t>& get_extent() const;
    
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
			    AbstractRegion* region) = 0;

    /**
     * Function that retrieves all nodes in the input node pointer (should
     * be a subnet).
     * @param n  node pointer
     * @returns vector of node pointers
     */
    static std::vector<Node*> get_nodes(Node* n);

    /**
     * Returns position of node recognized by current Layer/Compound
     * local id value.
     * @param lid  local id of node
     * @returns position of node
     */
    virtual const Position<double_t> get_position(index lid) const = 0;

    /**
     * Returns displacement of node from given position
     * @param from_pos  position vector in layer
     * @param to        node in layer to which displacement is to be computed
     * @returns vector pointing from from_pos to node to's position
     */
    virtual const Position<double_t> compute_displacement(const Position<double_t>& from_pos,
							  const Node& to) const = 0; 

    /**
     * Returns position of input node.
     * @param n  node
     * @returns position of node
     * @throws LayerExpected if the node is not inside a layer
     */
    static const Position<double_t> get_position(const Node& n);
    
     /**
      * Get parent layer and local id within parent of node
      * @param const Node& child node node to find layer for
      * @returns pair of layer and id, layer is 0 if the node is not in a layer
      */
    static Layer* get_layer(const Node& child);

    /**
     * Write layer data to stream.
     * For each node in layer, write one line to stream containing:
     * GID x-position y-position [z-position]
     * @param   output stream
     */
    void dump_nodes(std::ostream &) const;

    /**
     * @returns true if edge wrap is used.
     */
    bool edge_wrap_is_set() const;
    
    virtual bool allow_entry() const;

  protected:
    /**
     * Called by the layer slice() functions. Extract nodes based on 
     * modeltype and depth from calling layer and returns these nodes
     * in a node vector.
     * @param dict  Dictionary containing the criterias that the function
     *              extracts nodes based on. See the documentation for
     *              the SLI topology/ConnectLayer function.
     * @returns the selected nodes
     */
    std::vector<Node*> 
      slice_layer(const DictionaryDatum& dict) const;

    // Geometry of the layer
    std::vector<double_t> extent_;  //< size of layer
    std::vector<double_t> center_;  //< coordinate of layer center
    Position<double_t> upper_left_; //< upper left corner of layer

    // Layer parameters.
    uint_t depth_;                  //< number of nodes at each position

    // True if edges of layer are wrapped 
    bool EDGE_WRAP_;
  };


  /**
   * Exception to be thrown if a layer was excepted
   * but some other node passed.
   * @ingroup KernelExceptions
   */
  class LayerExpected: public KernelException
  {
  public:
  LayerExpected()
    : KernelException("TopologyLayerExpected") {}
    ~LayerExpected() throw () {}
    
    std::string message();
  };

}

#endif
