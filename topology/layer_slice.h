#ifndef LAYER_SLICE_H
#define LAYER_SLICE_H

/*
 *  layer_slice.h
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

#include "nodewrapper.h"
#include "selector.h"
#include "layer.h"
#include "layer_regular.h"
#include "layer_unrestricted.h"
#include "layer_3d.h"

namespace nest {
  template<class LayerType>
  class LayerSlice : public LayerType {
  public:
    /**
     * Copy constructor
     */
    LayerSlice(const LayerSlice& other);

    /**
     * Create a slice from another layer. The layer to slice will
     * normally be the same type as the slice (ie. the template
     * argument), but a common special case is when an unrestricted
     * slice is created from a regular layer.
     * @param layer Layer to slice
     * @param dict Dictionary containing the criterias 
     *             that the function extracts nodes based on. See the 
     *             documentation for the SLI topology/ConnectLayer 
     *             function.
     */
    template<class FromLayerType>
    LayerSlice(const FromLayerType& layer, const DictionaryDatum& dict);

    /**
     * Destructor. Deletes the compounds created when slicing.
    */
    ~LayerSlice();

    const Position<double_t> get_position(index lid) const;

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
    lockPTR<std::vector<NodeWrapper> >
      get_pool_nodewrappers(const Position<double_t>& driver_coo,
			    AbstractRegion* region);

  protected:
    /**
     * Layer type specific initialization.
     */
    void init_internals();
    const Layer * original_layer_;
  };


  template<class LayerType> template<class FromLayerType>
  LayerSlice<LayerType>::LayerSlice(const FromLayerType &layer,
				    const DictionaryDatum &layer_connection_dict):
  LayerType(layer),
  original_layer_(&layer)
  {
    this->nodes_.clear();

    // Create Selector object used to remove unwanted nodes
    // from the layer. 
    // Note to developers: Additional selector objects can be 
    // added by manipulating this class. 
    Selector selector(layer_connection_dict);

    // Retrieve nodes at selected depth level.
    // Iterate through nodes and retrieve nodes that fit criteria.
    // Selected nodes are inserted into a new compound structure
    // (i.e. nested compound structures are flattened). 
    for(std::vector<Node*>::const_iterator it=layer.begin(); it != layer.end(); ++it)
    {
      Compound* subnet = new Compound();

      selector.slice_node(*subnet, *it);

      this->nodes_.push_back(subnet);
    }

    // Do layer type specific initialization
    init_internals();
  }

  template<class LayerType>
  void inline LayerSlice<LayerType>::init_internals()
  {
    // No layer type specific initialization by default.
  }

  template<>
  void inline LayerSlice<LayerUnrestricted>::init_internals()
  {
    make_tree();
  }

  template<>
  void inline LayerSlice<Layer3D>::init_internals()
  {
    make_tree();
  }

  template<class LayerType>
  LayerSlice<LayerType>::~LayerSlice()
  {
    for(std::vector<Node*>::iterator it = this->begin();
	it != this->end(); ++it)
      {
      Compound* c = dynamic_cast<Compound*>(*it);
      assert(c);
	    
      delete c;
    }
  }

  template<class LayerType>
  const Position<double_t> 
  inline LayerSlice<LayerType>::get_position(index lid) const
  {
    return LayerType::get_position(lid);
  }

  template<class LayerType>
  lockPTR<std::vector<NodeWrapper> > inline
  LayerSlice<LayerType>::get_pool_nodewrappers(const Position<double_t>& driver_coo,
			       AbstractRegion* region)
  {
    return LayerType::get_pool_nodewrappers(driver_coo, region);
  }


}

#endif
