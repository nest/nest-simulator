/*
 *  connection_creator.cpp
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

#include "connection_creator.h"

#include <set>

#include "compound.h"
#include "dictdatum.h"
#include "dictutils.h"
#include "exceptions.h"
#include "binomial_randomdev.h"

#include "layer_regular.h"
#include "layer_unrestricted.h"
#include "layer_3d.h"
#include "topologyconnector.h"
#include "topology_names.h"
#include "region.h"

namespace nest
{

  ConnectionCreator::ConnectionCreator(Layer* sources, 
				       Layer* targets,
				       const DictionaryDatum& connection_dict, 
				       Network& net,
                                       bool display_progress):
    connection_dict_(connection_dict),
    net_(net),
    population_probability_(1.0),
    population_multapses_(true),
    display_progress_(display_progress)
  {
    // Initialise region parameter
    region_ = 
      Region::create_region(get_dictionary(connection_dict_, names::mask));
      
    // Ensure that mask diameter does not exceed extent unless explicitly requested
    // See #482. Special handling of DiscreteRegion is not very elegant ...
    bool allow_oversized_mask = false;
    updateValue<bool>(connection_dict, names::allow_oversized_mask, allow_oversized_mask);
    if ( not allow_oversized_mask )  // need not check mask-size otherwise
    {
      const DiscreteRegion* disc_reg = dynamic_cast<DiscreteRegion*>(region_);
      if ( disc_reg )
      {
        // discrete region, consider rows, columns
        const LayerRegular* reg_lyr = dynamic_cast<LayerRegular*>(targets);
        if ( not reg_lyr )
          throw BadProperty("Grid mask can only be used with grid-based target layer.");

        if ( disc_reg->get_rows() > reg_lyr->get_rows() || disc_reg->get_columns() > reg_lyr->get_columns() )
          throw BadProperty("Mask size must not exceed layer size; set allow_oversized_mask to override.");
      }
      else
      {
        // standard region, consider coordinates
        const Position<double_t> l_ext = targets->get_extent();
        const Position<double_t> m_dia = region_->get_upper_right() - region_->get_lower_left();
        // due to the special way in which comparison for position is defined, not <= is not the inverse of >.
        if ( not ( m_dia <= l_ext )  )
          throw BadProperty("Mask size must not exceed layer size; set allow_oversized_mask to override.");
      }
    }

    // This function assigns the source and target layer to the
    // driver and pool layer pointers. The mask, the connector
    // object and the number of connections are also initialised 
    // in this function.
    init_connector(sources, targets);

    updateValue<double_t>(connection_dict_, "population",
				population_probability_); 
    updateValue<bool>(connection_dict_, "population_multapses",
		      population_multapses_); 

    if(population_probability_ < 0)
      {
	throw TypeMismatch("population probability >= 0",
			   "population probability < 0");
      }

    //Create connections
    execute();
  }

  ConnectionCreator::~ConnectionCreator()
  {
    delete topology_;
    delete region_;
    //pool_ and driver_ are lockPTRs and will be deleted automatically
  }

  void ConnectionCreator::init_connector(Layer* sources, Layer* targets)
  {
    // Initialise connector. 

    // Read connection type from connection dictionary.
    std::string connection_type = 
      getValue<std::string>(connection_dict_, "connection_type");

    if(connection_type == "convergent")
      {
	// Create a Convergentonnector. 
	topology_ = new ConvergentConnector(net_);

	// Initialise topology_. 
	topology_->init(connection_dict_);

	// Set driver and pool pointers to the correct layers.

	// Creates a copy of the source and target layer. The copied layers 
	// are modified forms of the original layers.
	//
	// The duplication of the layer objects involve a duplication 
	// of a set of node pointers, not a duplication of actual nodes.
	// So the connections begin created are still created on the original 
	// nodes.
	driver_ = targets->slice(dict_is_unrestricted(connection_dict_),
				 get_dictionary(connection_dict_, 
						names::targets)); 

	pool_ = sources->slice(dict_is_unrestricted(connection_dict_),
			       get_dictionary(connection_dict_, 
					      names::sources));

	// Modify topology_ variable depending upon
	// layer type. region_ should be set up
	// and topology_ initialised before calling 
	// this function.
	adjust_topology_parameters(pool_);
      }
    else if(connection_type == "divergent")
      {
	topology_ = new DivergentConnector(net_);

	topology_->init(connection_dict_);

	driver_ = sources->slice(dict_is_unrestricted(connection_dict_),
				 get_dictionary(connection_dict_, 
						names::sources));

	pool_ = targets->slice(dict_is_unrestricted(connection_dict_),
			       get_dictionary(connection_dict_, 
					      names::targets));

	adjust_topology_parameters(pool_);
      }
    else
      {
	// Connection type must be Convergentonnect or DivergentConnect.
	throw EntryTypeMismatch("convergent or divergent",
				"something else");
      }

  }
  
  void ConnectionCreator::adjust_topology_parameters(lockPTR<Layer> pool)
  {
    LayerUnrestricted* unrestricted =
      dynamic_cast<LayerUnrestricted*>(&*pool);
    
    Layer3D* layer_3d = 
      dynamic_cast<Layer3D*>(&*pool);
    
    if(unrestricted || layer_3d)
      {
	// Assert that no topology_ parameters are 
	// of fixed grid type.
	topology_->modify_to_unrestricted();
      }
    else
      {
	const LayerRegular* pool_reg = dynamic_cast<LayerRegular*>(&(*pool));
	assert(pool_reg);
	// Fixed grid layers must have discrete
	// region_ parameter. Otherwise the 
	// fixed grid layer should have been
	// converted to an unrestricted layer
	// at this point.
	DiscreteRegion* dr = 
	  dynamic_cast<DiscreteRegion*>(region_);

	if(dr != NULL)
	  {
	    // Assert that all topology_ parameters
	    // are of fixed grid type.
	    topology_->modify_to_fixed_grid(pool_reg->get_dpd(), *dr,
					    pool->edge_wrap_is_set() ? &pool->get_extent() : 0);
	  }
	else
	  {
	    // Impossible error
	    assert(false);
	  }
      }
  }

  void ConnectionCreator::execute()
  {
    // Create connections.

    // Iterate through driver layer and connect against selected mask 
    // region in pool layer. If population connect is used only
    // selected driver nodes are used.

    // Keep a record in "i" of how many driver nodes that have been
    // connected to.
    int_t i = 0;

    // Number of driver nodes to connect to.
    int_t number = driver_->size();

    if(population_probability_ != 1.0)
      {
	// Determine number of driver nodes to connect to based 
	// upon a binomial distribution and a probability value.
	
	// What if population probability is close to 1.0 and
	// brng draws a number larger than driver size????

	librandom::BinomialRandomDev brng(net_.get_grng(),
					  population_probability_,
					  number);

	number = brng.uldev();
      }

    // Vector keeping track of which nodes that have been
    // picked.
    std::vector<bool> chosen_nodes;
    
    if(!population_multapses_)
      {
	chosen_nodes.resize(driver_->size(), false);
      }
    
    librandom::RngPtr rng = net_.get_grng();
    
    long_t n_id = -1;

    for(size_t j = 0; 
	j < static_cast<size_t>(number); 
	++j)
      {
	do 
	  {
	    // Re-draw nodes untill unique node is picked if 
	    // multapses aren't allowed.
	    if(static_cast<size_t>(number) < driver_->size())
	      {
		// Pick random node if population connect is used.
		n_id = rng->ulrand(driver_->size());
	      }
	    else
	      {
		++n_id;
	      }
	  }
	while(!population_multapses_ && chosen_nodes.at(n_id) == true);
	
	if(!population_multapses_)
	  {
	    // Note that node is picked.
	    chosen_nodes.at(n_id) = true;
	  }

	// Retrieve driver nodes at the same 2D position as the main driver
	// node.
  	lockPTR<std::vector<NodeWrapper> > driver_nodes = 
	  NodeWrapper::get_nodewrappers(driver_->at(n_id), driver_->get_position(n_id));
	
	// If the nodes at the driver position are removed, the iteration 
	// skips to the next position. 
	if(driver_nodes->size() != 0)
	  {
	    // Check if driver node is target node. Skip iteration if
	    // target isn't local.
	    if(topology_->is_local(driver_nodes))
	      {
		// Retrieve pool nodes within mask region covering the 2D 
		// driver position given above.
		lockPTR<std::vector<NodeWrapper> > pool_nodes = 
		  pool_->get_pool_nodewrappers(driver_nodes->at(0).get_position(),
					       region_);
		
		//Connect driver and pool nodes.
		topology_->connect(driver_nodes,
				   pool_nodes);
	      }
	  }


	// 	// THESE LINES ARE USED DURING DEVELOPMENT AND SHOULD BE
	// 	// REMOVED ONCE THE MODULE IS FINISHED.
	// 	if(i == 10)
	// 	  {
	// 	    break;
	// 	  }

	// 	std::cout << i << " " << std::flush;

	++i;

	// Print information about connection progress to screen.
	if(display_progress_ &&
	   (driver_->size() > 100) && (i%(driver_->size()/100) == 0))
	  {
	    std::cout << " \rConnecting: " << std::setw(5)
		      << (100.0*i)/driver_->size() 
		      << " %" << std::flush;

	    // 	    // THESE LINES ARE USED DURING DEVELOPMENT AND SHOULD BE
	    // 	    // REMOVED ONCE THE MODULE IS FINISHED.
	    // 	    // Remove comments below to abort run before connection 
	    // 	    // is finished.
	    // 	    if((100.0*i)/driver_->size() >= 1)
	    // 	      {
	    // 		break;
	    // 	      }
	  }
      }

    // Complete connection process by printing a line shift if 
    // connection progress is printed above.
    if(display_progress_ &&
       (driver_->size() > 100) && (i%(driver_->size()/100) == 0))
      {
	std::cout << std::endl;
      }
  }

  DictionaryDatum ConnectionCreator::
  get_dictionary(const DictionaryDatum& dict, const Name sub_dict_name)
  {
    DictionaryDatum sub_dict = new Dictionary();

    updateValue<DictionaryDatum>(dict, sub_dict_name, sub_dict);
    
    return sub_dict;
  }

  bool ConnectionCreator::
  dict_is_unrestricted(const DictionaryDatum& connection_dict)
  {
    return !(getValue<DictionaryDatum>
	     (connection_dict, "mask")->known("grid"));
  }

}// topology namespace ends

