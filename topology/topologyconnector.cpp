/*
 *  topologyconnector.cpp
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

#include "topologyconnector.h"

#include <set>

#include "nest.h"
#include "network.h"
#include "doubledatum.h"
#include "arraydatum.h"
#include "namedatum.h"
#include "nestmodule.h"

#include "topology_names.h"
#include "parameters.h"
#include "region.h"
#include "position.h"
#include "walker.h"

#include "binomial.h"

namespace nest
{
  TopologyConnector::TopologyConnector(Network& net):
    net_(net),
    weight_(),
    delay_(),
    probability_(),
    number_of_connections_(0),
    allow_multapses_(true),
    allow_autapses_(true),
    synmodel_id_(0),
    walker_()//,
    //binomial_()
  {
  }

  TopologyConnector::~TopologyConnector()
  {
    delete weight_;
    delete delay_;
    delete probability_;
  }

  void TopologyConnector::init(const DictionaryDatum& connection_dict)
  {
    // Read number of connections from dictionary.
    updateValue<long_t>(connection_dict, "number_of_connections", 
			      number_of_connections_);
    
    //     if(number_of_connections_ < 0)
    //       {
    // 	throw EntryTypeMismatch("number_of_connections > 0", 
    // 				"number_of_connections < 0");
    //       }

    updateValue<bool>(connection_dict, "allow_multapses", allow_multapses_);
    updateValue<bool>(connection_dict, "allow_autapses", allow_autapses_);
    
    weight_ = init_parameter(connection_dict, "weights");
    delay_ = init_parameter(connection_dict, "delays");
    probability_ = init_parameter(connection_dict, "kernel");

    // Make sure that delays and probabilities always take on 
    // positive values.
    delay_->force_positive();
    probability_->force_positive();

    // Get synapse type
    if(connection_dict->known("synapse_model"))
      {
	const std::string syn_name = 
	  getValue<std::string>(connection_dict, "synapse_model");

	const Token synmodel = 
	  net_.get_synapsedict().lookup(syn_name);
	
	if ( synmodel.empty() )
	  throw UnknownSynapseType(syn_name);

	synmodel_id_ = static_cast<index>(synmodel);
      }

  }

  void TopologyConnector::
  modify_to_fixed_grid(const Position<double_t> pool_dpd,
		       const DiscreteRegion& region,
                       const std::vector<double_t> * extent)
  {
    adjust_parameter(weight_, pool_dpd, region, extent);
    adjust_parameter(delay_, pool_dpd, region, extent);
    adjust_parameter(probability_, pool_dpd, region, extent);
    
    // The call to set_fixed() indicates that the walker_ object
    // only needs to be initialised at certain intervals in the
    // connection process.
    walker_.set_fixed(region.size());
// 	binomial_.set_fixed(region.size());
  }

  void TopologyConnector::
  adjust_parameter(Parameters*& p, 
		   const Position<double_t> pool_dpd,
		   const DiscreteRegion& region,
		   const std::vector<double_t> * extent)
  {
    Discrete* d = dynamic_cast<Discrete*>(p);
    Uniform* r = dynamic_cast<Uniform*>(p);
    if(!d && !r)
      {
	// Create a discrete parameter array based on another 
	// parameter. The new variable is modified
	// according to the pool node density and the region 
	// dimensions.
	Discrete d = create_discrete_from_parameters(p, pool_dpd, region, extent);
	delete p; // p = 0;
	p = new Discrete(d);
      }
  }

  Discrete TopologyConnector::
  create_discrete_from_parameters(const Parameters* par,
				  const Position<double_t> pool_dpd,
				  const DiscreteRegion& region,
				  const std::vector<double_t> * extent)
  {
    //Creates array and inserts output elements in array. 
    std::vector<double_t> array;
    
    for(index i = 0; 
	i != region.get_rows()*region.get_columns(); ++i)
      {
	//Calculates vector of distance.
	
	Position<double_t> position = 
	  region.get_anchor() - 
	  Position<int_t>(i / region.get_rows(), 
				i % region.get_rows());
	
	Position<double_t> displacement = 
	  Position<double_t>(position)/pool_dpd;
	
	if (extent)
	  {
	    displacement.wrap_displacement_max_half(*extent);
	  }

	//Converts distance vector to absolute distance and calculates 
	//array value according to defined function.
	array.push_back(par->get_value(displacement));
      }    

    return Discrete(array);
  }

  void TopologyConnector::modify_to_unrestricted()
  {
    Discrete* w = dynamic_cast<Discrete*>(weight_);
    Discrete* d = dynamic_cast<Discrete*>(delay_);
    Discrete* p = dynamic_cast<Discrete*>(probability_);

    if(w != NULL || d != NULL || p != NULL)
      {
	throw EntryTypeMismatch("unrestricted region",
				"fixed grid region");
      }
  }

  Parameters* 
  TopologyConnector::init_parameter(const DictionaryDatum& mask_dict,
				    const Name& feature)
  {
    const Token& t = mask_dict->lookup(feature);
    
    DoubleDatum *dd = dynamic_cast<DoubleDatum*>(t.datum());
    
    // Checks if a single constant number is given in dictionary.
    if(dd)
      {
	return new Parameters(Token(*dd));
      } 
    
    DictionaryDatum *dictdat = 
      dynamic_cast<DictionaryDatum*>(t.datum());
    
    // Checks if gaussian parameters are given in dictionary.
    if(dictdat)
      {
	return Parameters::create_parameter(*dictdat);
      }
    
    // Checks if an array is given in the dictionary.
    // Only allowed for fixed grid layers.

    ArrayDatum *ad = dynamic_cast<ArrayDatum*>(t.datum());
    
    if(ad)
      {
	std::vector<double_t> array;

	for(int_t i = ad->size()-1; i >= 0; --i)
	  {
	    array.push_back((*ad)[i]);
	  }

	//		ad->toVector(array);
	return new Discrete(array);
      }
    
    return new Parameters();
  }

  void TopologyConnector::connect(lockPTR<std::vector<NodeWrapper> >& driver,
				  lockPTR<std::vector<NodeWrapper> >& pool)
  {
    // Iterate through the driver and pool nodes and connect
    // every driver node to every pool node.
    for(std::vector<NodeWrapper>::iterator it_driver=driver->begin();
	it_driver != driver->end(); ++it_driver)
      {
	// Retrieve the correcte random number generator
	// from the topology_ object.
	librandom::RngPtr rng;
	
	// For convergent connector the rng is only returned
	// if the driver (target) node is local. 
	// Otherwise false is returned.
	if(get_rng(*((*it_driver).get_node()), rng))
	  {
	    // Check if all pool nodes should be connected to.
	    if(number_of_connections_ != 0) 
	      {
		// If number of connections are greater than the number
		// of candidate pool nodes and multapses aren't 
		// allowed we'll connect to all nodes within the pool 
		// region.
		if(number_of_connections_ >= pool->size() && !allow_multapses_)
		  {
		    // @todo Relate to assertion below. Problem if pool is 
		    // driver node and autapses aren't allowed.
		    if(number_of_connections_ == 1 && !allow_autapses_ &&
		       pool->at(0).get_node() == (*it_driver).get_node())
		      {
			NestModule::get_network().message(SLIInterpreter::M_WARNING, 
			    "Topology", "A very rare error occurred."
			    "The topology module is not suited to connect "
			    "a layer to itself with a mask size covering "
			    "one node without allowing autapses and only "
                            "allowing one connection per node.");
		      }
		    
		    NestModule::get_network().message(SLIInterpreter::M_WARNING, 
			    "Topology", "A rare error occurred."
			    "The number_of_connections variable will be "
			    "ignored. All nodes will be connected to for "
                            "current driver node.");

		    for(uint_t i = 0; i < pool->size(); ++i)
		      {
			connect(*it_driver, pool->at(i));
		      }		    
		  }
		else
		  {
		    // If fixed grid layers are used the walker_ object
		    // only needs to be re-initialised at certain intervals.
		    if(!walker_.is_set(pool->size()))
		      {
			walker_.initialise(*it_driver, pool, *probability_);
		      }

// 		    if(!binomial_.is_set(pool->size()))
// 		      {
// 			binomial_.initialise(*it_driver, pool, *probability_);
// 		      }
		    
		    std::set<long_t> ch_ids; // ch_ids used for multapses 
		                                   // identification
    
		    // Pick a set of nodes equal to number_of_connections_
		    for(size_t j = 0; 
			j < static_cast<size_t>(number_of_connections_); 
			++j)
		      {
			NodeWrapper* n_id;
			
			do 
			  {
// 			    n_id = &pool->at(binomial_.get_random_id(rng));
			    n_id = &pool->at(walker_.get_random_id(rng));
			  }
			while((!allow_autapses_ && 
			       n_id->get_node() == (*it_driver).get_node()) ||
			      (!allow_multapses_ && ch_ids.find( n_id->get_node()->get_gid() ) != ch_ids.end()));
			
			if (!allow_multapses_)
			  ch_ids.insert( n_id->get_node()->get_gid() );

			connect(*it_driver, *n_id);
		      }
		  }
	      }
	    else
	      {
		std::set<long_t> ch_ids; // ch_ids used for multapses 
		                         // identification

		for(uint_t i = 0; i < pool->size(); ++i)
		  {
		    Node const * const pool_node = pool->at(i).get_node();

		    if(allow_autapses_ || 
		       pool_node != (*it_driver).get_node())
		      {
			if( rng->drand() < 
			   probability_->get_value((*it_driver).get_position(),
						   pool->at(i).get_position(),
			                           pool->at(i).get_extent()) )
			  {
			    if (!allow_multapses_)
			      {
				if(ch_ids.find( pool_node->get_gid() ) == 
				   ch_ids.end())
				  {
				    connect(*it_driver, pool->at(i));
				    ch_ids.insert( pool_node->get_gid() );
				  }
			      }
			    else
			      {
				connect(*it_driver, pool->at(i));
			      }
			  }
		      }
		  }
	      }
	  }
      }   
  }

  ConvergentConnector::ConvergentConnector(Network& net):
    TopologyConnector(net)
  {
  }
  
  bool ConvergentConnector::get_rng(Node& node, librandom::RngPtr& rng)
  {
    // Only connect node if node(target) is local.
    if(node.is_local())
      {
	// Get rng for thread. The network pointer is inherited through 
	// the Decorator class from the TopologyConnector class.
	rng = 
	  net_.get_rng(node.get_thread());
	return true;
      }
    return false;
  }

  bool ConvergentConnector::
  is_local(lockPTR<std::vector<NodeWrapper> > driver_nodes)
  {
    for(std::vector<NodeWrapper>::iterator it = driver_nodes->begin();
	it != driver_nodes->end(); ++it)
      {
	if(it->get_node()->is_local())
	  {
	    return true;
	  }
      }
    return false;
  }

  DivergentConnector::
  DivergentConnector(Network& net):
    TopologyConnector(net)
  {
  }
  
  bool DivergentConnector::get_rng(Node&, librandom::RngPtr& rng)
  {
    // The DivergentConnector class is used in the case where we have a single
    // source node for several target nodes. In this case it is not
    // possible to select a single target thread for getting the rng.
    // In this case we use the global rng instead.
    rng = net_.get_grng();
    return true;
  }

bool DivergentConnector::is_local(lockPTR<std::vector<NodeWrapper> >)
  {
    // The target node is not given as input to the function
    // so it is not possible to decide if the target is local yet.
    return true;
  }

}//namespace ends
