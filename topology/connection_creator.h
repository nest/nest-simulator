#ifndef CONNECTION_CREATOR_H
#define CONNECTION_CREATOR_H

/*
 *  connection_creator.h
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

#include "nest.h"
#include "dictdatum.h"
#include "node.h"

#include "layer.h"
#include "topologyconnector.h"

/** @file connection_creator.h
 *  Main management file for setting up a topological connection.
 */

namespace nest
{
  class LayerRegular;
  class TopologyConnector;
  class AbstractRegion;
  
  /**
   * The ConnectionCreator class main responsibility is to prepare and 
   * execute a ConnectLayer function. 
   *
   * Only the constructor of the class is public. The creation of a 
   * ConnectionCreator object automatically sets up and executes the 
   * connection procedure. The exact nature of the connection being 
   * created is given in the dictionaries passed as input to the 
   * constructor.
   */

  class ConnectionCreator
  {

  public:
    /**
     * The ConnectionCreator constructor prepares for and creates node 
     * connections between two layers of nodes (@see Layer). The 
     * constructor consists of three main parts: Initialising the layers 
     * used in the connection, creating a TopologyConnector object 
     * responsible for creating the actual node connections, and executing 
     * the selected connection command.
     *
     * The constructor accepts the input parameters:
     * @param sources  Pointer to the source layer used in the connection.
     *                 The layer can be of any layer type.
     * @param targets  Like sources, but target layer.
     * @param dict     Dictionary containing the information needed to 
     *                 set up the selected connection. See
     *                 topologyModule.cpp->ConnectLayer
     * @param net      Reference to main network. Needed by the created
     *                 TopologyConnector object to perform calls to the
     *                 network connect(..) function. Also used to get 
     *                 access to the random number generator.
     * @param progress If true, display progress while connecting.
     */
    ConnectionCreator(Layer* sources, 
		      Layer* targets,
		      const DictionaryDatum& dict, 
		      Network& net,
                      bool progress);

    ~ConnectionCreator();

  private:
    // Layers being connected:
    // Driver refers to the layer being iterated through in a convergent 
    // or divergent function (i.e. the target layer for divergent).
    // Pool refers to the layer which has the connection mask attached
    // to it (i.e. the source layer for convergent).
    lockPTR<Layer> driver_;
    lockPTR<Layer> pool_;

    // Dictionary containing information about the connection pattern.
    DictionaryDatum connection_dict_;

    // Reference to network where the nodes reside. 
    Network& net_;

    // The Connector object being used to create the actual node
    // connections.
    TopologyConnector* topology_;

    // Region object describing the scope used to pick nodes
    // from the pool layer.
    AbstractRegion* region_;

    // Experimental new population connect feature
    // @todo Finalise or remove this feature
    double_t population_probability_;
    bool population_multapses_;

    // Progress display
    bool display_progress_;

    /**
     * Initialises or creates the TopologyConnector object,
     * the driver_ and pool_ layer pointers.
     * 
     * @param sources  pointer to source layer
     * @param targets  pointer to target layer
     */
    void init_connector(Layer* sources, Layer* targets);

    /**
     * Creates connections. All the class variables should
     * be initialised before calling this function.
     */
    void execute();

    /**
     * Function implements the dictutils.* getValue<DictionaryDatum> 
     * function, but instead of throwing an error if the dictionary 
     * doesn't exist the function returns an empty dictionary. 
     * @param dict  Dictionary where we should search for the new dictionary.
     * @param type  Name of dictionary that we are trying to retrieve.
     * @returns the dictionary searched for in the function or an empty 
     *          dictionary.
     */
    DictionaryDatum get_dictionary(const DictionaryDatum& dict, 
				   const Name type);
    
    /**
     * @returns true if the mask parameter in the input dictionary
     *          is of a different type than the grid mask parameter
     */
    bool dict_is_unrestricted(const DictionaryDatum& connection_dict);

    /**
     * Adjust topology_ variable depending upon whether the layer is
     * of unrestricted or fixed grid type.
     * @param pool pointer to layer
     */
    void adjust_topology_parameters(lockPTR<Layer>);

  };

}

#endif
