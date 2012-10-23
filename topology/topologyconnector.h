#ifndef TOPOLOGYCONNECTOR_H
#define TOPOLOGYCONNECTOR_H

/*
 *  topologyconnector.h
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
#include "network.h"

#include "nodewrapper.h"
#include "parameters.h"
#include "walker.h"
#include "binomial.h"

/** @file topologyconnector.h
 *  Implements the TopologyConnector structure.
 */

namespace nest
{
  class Gaussian;
  class Discrete;
  class DiscreteRegion;
  class Walker;
  //  class Parameters;

  /**
   * The TopologyConnector is the base of the connector structure. 
   * The structure decides the character of the connection pattern 
   * created by the ConnectLayer function.
   *
   * At present two connectors exist:
   * ---> Convergent topology connection 
   *      -> connecting a source mask to a target node
   * ---> Divergent topology connection
   *      -> connecting a source node to a target mask
   *
   * The class is used by the topology::ConnectionCreator class
   */
  class TopologyConnector
  {
  public:
    
    /**
     * Creates a simple TopologyConnector that performs the simple
     * connection of a source and a target node. TopologyConnector
     * contain a reference to the main simulation network. 
     *
     * @param net     Reference to main network. 
     */
    TopologyConnector(Network& net);

    virtual ~TopologyConnector();

    /**
     * Initialise connector member variables.
     *
     * @param connection_dict Input connection dictionary.
     */
    void init(const DictionaryDatum& connection_dict);

    /**
     * Check if any of the Parameters class variables are set to 
     * Gaussian. If this is the case the variables are converted 
     * to Discrete type. This function should only be called if
     * fixed grid layers are used.
     *
     * @param pool_dpd Node density of pool layer.
     * @param region   Region used by ConnectionCreator when creating
     *                 connections.
     * @param extent  layer extent for periodic boundary conditions, or 0 otherwise
     */
    void modify_to_fixed_grid(const Position<double_t> pool_dpd,
			      const DiscreteRegion& region,
                              const std::vector<double_t> * extent);

    /**
     * Checks if any of the Parameters class variables are set to 
     * Discrete. Throws an error if this is the case. This function
     * should only be called if unrestricted layers are used.
     */
    void modify_to_unrestricted();

    /**
     * Create a Parameters object based upon an entry in the input
     * dictionary. The type of the Parameters object is decided by 
     * the type of the dictionary entry.
     *
     * @param mask_dict Mask dictionary
     * @param feature   Name of Parameters object being initialised
     * @returns a Parameters object.
     */
    Parameters* init_parameter(const DictionaryDatum& mask_dict,
			       const Name& feature);
 
    /**
     * Function that calls the Network::connect() function.
     * 
     * @param source  Reference to source Node used in a one-to-one
     *                connection.
     * @param target  Reference to target Node used in a one-to-one
     *                connection.
     * @param weight  Weight of synaptic connection.
     * @param delay   Delay of synaptic connection.
     */
    void connect(Node& source, 
		 Node& target,
		 double_t weight,
		 double_t delay);

    /**
     * Retrieve the right weight and delay from the TopologyConnector
     * Parameters variables, and connect a driver and a pool node
     * in a receptive field or projective field fashion.
     *
     * @param source  Reference to source NodeWrapper used in a 
     *                one-to-one connection.
     * @param target  Reference to target NodeWrapper used in a 
     *                one-to-one connection.
     */
    virtual void connect(NodeWrapper& target, 
			 NodeWrapper& source) = 0;

    /**
     * Connects a set of driver nodes to a set of pool nodes. The
     * position of the nodes are passed together with the node
     * pointers to the function.
     * 
     * @param driver  The driver nodes that we want to connect to 
     *                the given set of pool nodes.
     * @param pool    The pool nodes.
     */
    void connect(lockPTR<std::vector<NodeWrapper> >& driver,
		 lockPTR<std::vector<NodeWrapper> >& pool);
  
    /**
     * Function that returns a random number generator. Function 
     * works differently for ConvergentConnector and DivergentConnector. Please
     * see the documentation for these classes.
     */
    virtual bool get_rng(Node& node, librandom::RngPtr& rng) = 0;

    /**
     * @param driver_nodes List of input nodes.
     * @returns true if the input nodes are target and any of the
     *               input nodes are local.
     */
    virtual bool is_local(lockPTR<std::vector<NodeWrapper> > driver_nodes) = 0;

  protected:
    // Reference to main simulation network.
    Network& net_;

    // Connection parameters.
    Parameters* weight_;
    Parameters* delay_;
    Parameters* probability_;

  private:
    // If this variable is set only a limited number of connections 
    // are created for each driver node. The nodes being connected
    // to are randomly selected if the number of nodes within
    // the mask region is greater than the limit set (similar to 
    // random_divergent_connect and random_convergent_connect).
    size_t number_of_connections_;
    bool allow_multapses_;
    bool allow_autapses_;

    // Synapse type of connections
    index synmodel_id_;

    // The walker object is used to speed up the drawing of random
    // connections.
    Walker walker_;
    //    Binomial binomial_;

    /**
     * Function called by modify_to_fixed_grid(..). Checks if a 
     * single parameter object is set to Gaussian. Converts this
     * parameter to discrete if that is the case.
     *
     * @param p        Reference to pointer to Parameters object being checked.
     *                 Reference is used to be able to call delete on the 
     *                 variable inside the function.
     * @param pool_dpd Node density of pool layer.
     * @param region   Region used by ConnectionCreator when creating
     *                 connections.
     * @param extent  layer extent for periodic boundary conditions, or 0 otherwise
     */
    void adjust_parameter(Parameters*& p, 
			  const Position<double_t> pool_dpd,
			  const DiscreteRegion& region,
			  const std::vector<double_t> * extent);

    /**
     * Creates a Discrete parameters object based upon an other
     * parameters object. Function is called by the adjust_parameter(..)
     * function.
     *
     * @param par      Parameters object.
     * @param pool_dpd Node density of pool layer.
     * @param region   Region used by ConnectionCreator when creating
     *                 connections.
     * @param extent  layer extent for periodic boundary conditions, or 0 otherwise
     */
    Discrete 
      create_discrete_from_parameters(const Parameters* par,
				      const Position<double_t> pool_dpd,
				      const DiscreteRegion& region,
				      const std::vector<double_t> * extent);
    
  };

  inline
    void TopologyConnector::connect(Node& source, 
				    Node& target,
				    double_t weight,
				    double_t delay)
  {
    //Connect nodes.
    net_.connect(source.get_gid(), 
		 target.get_gid(),
		 weight,
		 delay,
		 synmodel_id_);
  }

  /**
   * Class that implements the ConvergentConnector setting.
   */
  class ConvergentConnector: public TopologyConnector
  {
  public:
    
    /**
     * @param net  reference to main simulation network
     */
    ConvergentConnector(Network& net);

    virtual ~ConvergentConnector(){}

    /**
     * Calls the ConvergentConnector connection functions.
     * @param target  reference to connection target node with position
     * @param source  reference to source node with position
     *                target node should @strong precede @strong 
     *                source node as input parameter
     */
    void connect(NodeWrapper& target, 
		 NodeWrapper& source);
    
    /**
     * @param node  @strong target @strong node that random number
     *              generator should be based upon
     * @param rng   random number generator where target thread
     *              rng will be stored
     * @returns true if target is local
     */
    bool get_rng(Node& node, librandom::RngPtr& rng);

    bool is_local(lockPTR<std::vector<NodeWrapper> > driver_nodes);

  };

  inline
    void ConvergentConnector::connect(NodeWrapper& target, 
			      NodeWrapper& source)
  {
    // Reverses target and source position in input parameter list.
    // Retrieve correct weight and delay and create connection.
    TopologyConnector::connect(*(source.get_node()), 
			       *(target.get_node()), 
			       weight_->get_value(target.get_position(),
						  source.get_position(),
				                  source.get_extent()), 
			       delay_->get_value(target.get_position(),
						 source.get_position(),
				                 source.get_extent()));
  }

  /**
   * Class that implements the DivergentConnector setting.
   * Class inherits from Decorator instead of TopologyConnector
   * (see documentation for Decorator class).
   */  
  class DivergentConnector: public TopologyConnector
  {
  public:

    /**
     * @param net  reference to main simulation network
     */
    DivergentConnector(Network& net);

    virtual ~DivergentConnector(){}

    /**
     * Calls the DivergentConnector connection functions.
     * @param source  reference to connection source node with position
     * @param target  reference to target node with position
     *                source node should @strong precede @strong 
     *                target node as input parameter
     */
    void connect(NodeWrapper& source, 
		 NodeWrapper& target);

    /**
     * @param node  any node pointer (only used in ConvergentConnector
     *              version of same function)
     * @param rng   random number generator where global
     *              rng will be stored
     * @returns true in any case
     */
    bool get_rng(Node& node, librandom::RngPtr& rng);

    bool is_local(lockPTR<std::vector<NodeWrapper> > driver_nodes);

  };

  inline
    void DivergentConnector::connect(NodeWrapper& source, 
				     NodeWrapper& target)
  {
    // Connections are only stored on local target nodes.
    if(target.get_node()->is_proxy())
      return;

    // Reverses target and source position in input parameter list.
    // Retrieve correct weight and delay and create connection.
    TopologyConnector::connect(*(source.get_node()), 
			       *(target.get_node()), 
			       weight_->get_value(source.get_position(),
						  target.get_position(),
				                  target.get_extent()), 
			       delay_->get_value(source.get_position(),
						 target.get_position(),
				                 target.get_extent()));
  }

}
#endif

