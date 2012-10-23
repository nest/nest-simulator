/*
 *  topologymodule.cpp
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

// mandatory includes
#include "config.h"
#include "network.h"
#include "model.h"

// includes required by your module
#include "topologymodule.h"

#include "arraydatum.h"
#include "numerics.h"
#include <string>
#include "nestmodule.h"
#include "communicator.h"

#include <map>

#include "nestmodule.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include "network.h"
#include "genericmodel.h"
#include "iostreamdatum.h"
#include "tokenarray.h"
#include "integerdatum.h"
#include "stringdatum.h"
#include "leaflist.h"

#include "layer_regular.h"
#include "layer_unrestricted.h"
#include "layer_3d.h"
#include "topology_names.h"
#include "region.h"
#include "topologyconnector.h"
#include "connection_creator.h"

namespace nest
{
  
  Network* TopologyModule::net_ = 0;

  TopologyModule::TopologyModule(Network& net) 
  {
    assert(net_ == 0); // install only once
    net_ = &net;
  }

  TopologyModule::~TopologyModule()
  {}

  const std::string TopologyModule::name(void) const
  {
    return std::string("TopologyModule"); // Return name of the module
  }
  
  const std::string TopologyModule::commandstring(void) const
  {
    return 
      std::string("/topology /C++ ($Revision: 7862 $) provide-component "
		  "/topology-interface /SLI (6203) require-component ");
  }

  //---------------------------------------------------------------------

  //---------------------------------------------------------------------
  
  void TopologyModule::init(SLIInterpreter *i)
  {
    // ensure c'tor has set net_ pointer
    assert(net_ != 0);

    //Register the topology functions as SLI commands.

    i->createcommand("ConnectLayers_i_i_dict", 
		     &connectlayers_i_i_dictfunction);

    i->createcommand("CreateLayer_dict", 
		     &createlayer_dictfunction);

    i->createcommand("GetElement_i_ia",
		     &getelement_i_iafunction);

    i->createcommand("GetPosition_i",
		     &getposition_ifunction);

    i->createcommand("GetLayer_i",
		     &getlayer_ifunction);

    i->createcommand("Displacement_i_i",
		     &displacement_i_ifunction);

    i->createcommand("Displacement_a_i",
		     &displacement_a_ifunction);

    i->createcommand("Distance_i_i",
		     &distance_i_ifunction);

    i->createcommand("Distance_a_i",
		     &distance_a_ifunction);
 
    i->createcommand("DumpLayerNodes_os_i",
		     &dumplayernodes_os_ifunction);

    i->createcommand("DumpLayerConnections_os_i_l",
		     &dumplayerconnections_os_i_lfunction);

    register_model<LayerRegular>(*net_, "topology_layer_grid");
    register_model<LayerUnrestricted>(*net_, "topology_layer_free");
    register_model<Layer3D>(*net_, "topology_layer_3d");

  }  // TopologyModule::init()


//   void TopologyModule::unregister(SLIInterpreter *i, Network* net)
//   {

//   }

  static std::pair<long_t,index> layer_type(Network* net_,
					    const DictionaryDatum& layer_dictionary);
  static void create_from_array(Network* net_,
				const TokenArray& elements, index length);
  static void create_from_name(Network* net_,
			       const std::string& element_name, index length);
  static void create_from_procedure(Network* net_, SLIInterpreter *i, 
				    ProcedureDatum& pd, int_t layer_size);
  static void create_depth_column(Network* net_,
				  const TokenArray elements);


  /*
    BeginDocumentation
    
    Name: topology::CreateLayer - Creates topological node layer.
    
    Synopsis: dictionary CreateLayer -> layer_gid

    Parameters:
    dictionary -  Dictionary that describes the layout of a layer. 
                  Depending on the type of layer (fixed grid or
		  unrestricted) the dictionary can contain the 
		  elements:

    ----------------------------------------------------------------
    Name          Type          Description
    ----------------------------------------------------------------
    elements      Literal/      nodes at each 2D position
                  Procedure  

    extent        arraytype     [width height] of layer
    center        arraytype     [x_center y_center]

    edge_wrap     booltype      boundary condition

    rows*         integertype   node rows
    columns*      integertype   node columns

    position^     arraytype     an array describing the x 
                                and y positions of the layer 
                                nodes
    ----------------------------------------------------------------
    * Only for restricted layers.
    ^ Only for unrestricted layers.
 
    The element variable can be set in the following ways:
    /modeltype                   - create a layer consisting of single
                                   neurons
    {procedure}                  - creates a compound of nodes decided 
                                   by the procedure at each position in 
				   the layer

    [/iaf_neuron /iaf_psc_alpha] - create a compound of one iaf_neuron and
                                   one iaf_psc_alpha
    [/iaf_neuron 2]              - create a compound of two iaf_neurons
    or a combination of the two (e.g. [/iaf_psc_alpha [/iaf_psc_alpha 3]])


    Description: Creates a topological node layer. The layer can be
    of two types; one where the nodes are fixed on a rectangular 
    grid (fixed grid) and one where the nodes can be distributed 
    freely in the 2D space (unrestricted). The layers can be used 
    together with other topology functions to create topological 
    connection patterns.
   
    Example 1:

    topology using

    << /rows 3
       /columns 4
       /elements {/iaf_neuron Create ; /iaf_psc_alpha Create ;}
       /extent [2.0 2.0]
       /center [0.0 0.0]
       /edge_wrap true
    >> /dictionary Set

    dictionary CreateLayer

    Example 2:

    topology using

    << /elements /iaf_neuron
       /extent [2.0 2.0]
       /center [0.0 0.0]
       /edge_wrap false
       /positions [[0.5 -0.5] [0.5 0.5] [0.7 -0.5] [0.5 -0.5] [0.5 0.7]]
    >> /dictionary Set
    dictionary CreateLayer
       
    Author: Kittel Austvoll
  */

  void TopologyModule::
  CreateLayer_dictFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    assert(net_ != 0);
    assert(net_->get_cwn() != 0); 

    // Load dictionary containing layer specifications

    DictionaryDatum layer_dictionary = 
      getValue<DictionaryDatum>(i->OStack.pick(0));

    // Ensure that either rows/columns or positions is passed, but not both
    if ( not (           layer_dictionary->known(names::positions)
               xor (     layer_dictionary->known(names::rows)
                     and layer_dictionary->known(names::columns)
                   )
             )
       )
      throw BadProperty("Create layers requires either rows and columns or positions, "
                        "but not both.");

    //Retrieve type of layer (fixed grid or unrestricted) and number
    //of elements (length) in the 2D space of the layer. 
    const std::pair<long_t,index> p = layer_type(net_,layer_dictionary);
    const index length=p.second;
    const index layernode = net_->add_node(p.first);

    if(length != 0)
      {
	//Stores current subnet node. The function returns to this
	//subnet once the layer is constructed.
	const index cwnode = net_->get_cwn()->get_gid();
					  
	net_->go_to(layernode);

	//Create layer nodes.

	const Token& t = layer_dictionary->lookup(names::elements);

	ProcedureDatum* pd = dynamic_cast<ProcedureDatum *>(t.datum());
	ArrayDatum* ad = dynamic_cast<ArrayDatum *>(t.datum());

	if(pd)
	  {
	    create_from_procedure(net_, i, *pd, length);

	    //Set layer depth
	    (*layer_dictionary)[names::depth] = net_->get_cwn()->at(0)->size();

	  }
	else if (ad)
	  {
	    TokenArray elements = TokenArray(*ad);
	    create_from_array(net_, elements, length);

	    //Set layer depth
	    (*layer_dictionary)[names::depth] = elements.size();
	  }
	else
	  {
	    std::string element_name = 
	      getValue<std::string>(layer_dictionary, names::elements);
	    create_from_name(net_, element_name, length);

	    //Set layer depth
	    (*layer_dictionary)[names::depth] = 1;
	  }

	//Return to original subnet
	net_->go_to(cwnode);
      }
    else
      {
	net_->message(SLIInterpreter::M_WARNING, 
		      "Topology", "Creating empty layer.");
      }

    //Set layer parameters according to input dictionary.
    Layer *layer = 
      dynamic_cast<Layer *>(net_->get_node(layernode));   
    
    layer->set_status(layer_dictionary);

    i->OStack.pop(1);
    i->OStack.push(layernode);
    i->EStack.pop(); 
  }


  /** 
   * Function that determines whether we should create an unrestricted
   * or a restricted layer. An unrestricted layer is created if 
   * information about positions is set in the CreateLayer input
   * dictionary.
   * @param net_              Network
   * @param layer_dictionary  Connection dictionary
   * @returns pair of layer model id and number of nodes to be created
   */
  static std::pair<long_t,index> layer_type(Network* net_,
					    const DictionaryDatum& layer_dictionary)
  {
    index length;

    Token model_layer;

    TokenArray positions;

    //Selects an unrestricted layer if the reference "positions" exist
    //in the input dictionary.
    if(updateValue<TokenArray>(layer_dictionary, names::positions, 
			       positions))
      {
	if(positions.size() == 0)
	  {
	    throw TypeMismatch("positions array with coordinates",
			       "empty positions array");
	  }

	if(getValue<std::vector<double_t> >(positions[0]).size() == 3)
	  {
	    model_layer = 
	      net_->get_modeldict().lookup("topology_layer_3d");
	    if ( model_layer.empty() )
	      throw UnknownModelName("topology_layer_3d");
	  }
	else
	  {
	    model_layer = 
	      net_->get_modeldict().lookup("topology_layer_free");
	    if ( model_layer.empty() )
	      throw UnknownModelName("topology_layer_free");
	  }

	//Sets length (i.e. number of nodes in 2D layer) equal
	//to the number of positions passed in the input dictionary.
	length = positions.size();
      }
    else
      {
	//Selects a restricted layer.
	model_layer = 
	  net_->get_modeldict().lookup("topology_layer_grid");
	if ( model_layer.empty() )
	  throw UnknownModelName("topology_layer_grid");

	const int_t rows = getValue<long_t>(layer_dictionary, 
						  names::rows);
	const int_t columns = getValue<long_t>(layer_dictionary, 
						     names::columns);

	if(rows < 0 || columns < 0)
	  {
	    throw TypeMismatch("positive rows and columns numbers",
			       "negative rows or columns");
	  }
	
	//Sets length equal to the number of grid nodes (rows*columns) 
	//in the layer.
	length = rows*columns;
      }

    return std::pair<long_t,index>(static_cast<long_t>(model_layer),length);
  }

  /*
   *Sub-create layer function. Used to create a homogenous layer.
   *Called by CreateLayer_dictFunction::execute(..)
   */
  

  /**
     Initialize layer with elements given in array
  */
  static void create_from_array(Network* net_,
				const TokenArray& elements, index length)
  {
    for(index n=0;n<length;++n)
      {
	create_depth_column(net_, elements);
      }
  }

  /**
     Initialize layer with nodes with given name
   */
  static void create_from_name(Network* net_,
			       const std::string& element_name, index length)
  {
    const Token element_model = 
      net_->get_modeldict().lookup(element_name);
    if ( element_model.empty() )
      throw UnknownModelName(element_name);

    long_t element_id = static_cast<long>(element_model);

    //Create layer nodes.
    for(index n=0;n<length;++n)
      {
	net_->add_node(element_id);
      }
  }

  /*
   *Sub-create layer function. Used to create a layer where the elements
   *are created by a procedure argument. Called by 
   *CreateLayer_dictFunction::execute(..)
   */
	
  /**
   * Function that creates a layer node depth column. Creates a depth
   * column based upon a set of input SLI commands. 
   * @param i   Pointer to interpreter
   * @param pd  ProcedureDatum loaded from a SLI dictionary or elsewhere
   * @param layer_size Number of depth columns that shall be created.
   */
  static void create_from_procedure(Network* net_, SLIInterpreter *i, 
				    ProcedureDatum& pd, int_t layer_size)
  {
    const index subnet_id = net_->get_cwn()->get_gid();

    const index init_estack = i->EStack.load();
    const index init_ostack = i->OStack.load();
	
    //Insertion of layer elements starts.
	
    for(index n=0;n<static_cast<index>(layer_size);++n)
      {
	const index next_subnet_id = net_->add_node(0);

	net_->go_to(next_subnet_id);

	try
	  {
	    //Procedure is loaded onto execution stack.
	    i->EStack.push(pd);
	    
	    //Procedure is executed. Execution is stopped 
	    //when procedure is finished.
	    while(i->EStack.load() > init_estack)
	      {
		i->EStack.top()->execute(i);
	      }
	            
	    //Assert that OStack is left unchanged.
	    if(i->OStack.load() != init_ostack)
	      {
		net_->message(SLIInterpreter::M_ERROR, 
			      "Topology", "Please make sure that "
                              "the elements parameter procedure "
                              "leaves the stack unchanged.");

		i->raiseerror(i->ArgumentTypeError);
	      }
	  }
	catch(std::exception &exc)
	  {
	    i->raiseerror(exc);
	  }

	net_->go_to(subnet_id);
      }
  }

  // Deprecated function: @todo should be removed when an appropriate
  // way to pass a procedure from a dictionary at Python level have 
  // been found.
  /**
   * Function that creates nodes in a layer. 
   * @param i   Pointer to interpreter
   * @param ld  LiteralDatum loaded from a SLI dictionary or elsewhere
   * @param layer_size Number of nodes that shall be created.
   */
  static void create_depth_column(Network* net_,
				  const TokenArray elements)
  {
    // Store current subnet
    const index subnet_id = net_->get_cwn()->get_gid();

    // Insert a new depth column in the 2D layer.
    const index next_subnet_id = net_->add_node(0);
	        
    net_->go_to(next_subnet_id);
	
    // Insert elements in the depth column.
    for(index i = 0; i < elements.size(); ++i)
      {
	// A new nested depth column is created for every sub-array in
	// the input elements array.
	if(dynamic_cast<TokenArray *>(elements[i].datum()))
	  {
	    // Creates several nodes if the next element in
	    // the elements variable is a number.
	    if(i != elements.size()-1 &&
	        dynamic_cast<IntegerDatum*>(elements[i+1].datum()))
	      {
	        // Select how many nodes that should be created.
	        const index number =  getValue<long_t>(elements[i+1]);
	        for ( index j = 0 ; j < number ; ++j )
	          create_depth_column(net_, getValue<TokenArray>(elements[i]));
	        ++i;
	      }
	    else
	      create_depth_column(net_, getValue<TokenArray>(elements[i]));
	  }
	else
	  {
	    // Creates a set of identical nodes.
	    const std::string name = getValue<std::string>(elements[i]);
	            
	    // Creates several nodes if the next element in
	    // the elements variable is a number.
	    if(i != elements.size()-1 && 
	       dynamic_cast<IntegerDatum*>(elements[i+1].datum()))
	      {
		// Select how many nodes that should be created.
		const index number = 
		  getValue<long_t>(elements[i+1]);
	                
		// Create nodes.
		for(index j = 0; j < number; ++j)
		  {
		    const Token model = 
		      net_->get_modeldict().lookup(name);
		    if ( model.empty() )
		      throw UnknownModelName(name);
	                    
		    net_->add_node(static_cast<long>(model));
		  }
		++i;
	      }
	    else
	      {
		// Creates a single node.
		const Token model = 
		  net_->get_modeldict().lookup(name);
		if ( model.empty() )
		  throw UnknownModelName(name);
                
		net_->add_node(static_cast<long>(model));
	        
	      }
          }
      }
    net_->go_to(subnet_id);
  }

  /*
    BeginDocumentation
    
    Name: topology::GetElement - return node GID at specified layer position
    
    Synopsis: layer_gid [array] GetElement -> node_gid

    Parameters:
    layer_gid     - topological layer
    [array]       - position of node
    node_gid      - node GID
		 
    Description: Retrieves node at the layer position 
    set in [array]. [array] is on the format [column row]
   
    Examples:

    topology using

    %%Create layer
    << /rows 5
       /columns 4
       /elements /iaf_neuron
    >> /dictionary Set

    dictionary CreateLayer /src Set 

    src [2 3] GetElement
       
    Author: Kittel Austvoll
  */

  void TopologyModule::
  GetElement_i_iaFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const index layer_gid = getValue<long_t>(i->OStack.pick(1));
    TokenArray array = getValue<TokenArray>(i->OStack.pick(0));

    if(array.size() != 2)
      {
	throw TypeMismatch("array with length 2", "something else");
      }

    if(static_cast<long_t>(array[0]) < 0 ||
       static_cast<long_t>(array[1]) < 0)
      {
	throw TypeMismatch("positive array elements",
			   "negative array elements");
      }

    LayerRegular const * const layer = 
      dynamic_cast<LayerRegular*>(net_->get_node(layer_gid));

    Node* node;

    if(layer)
      {
	if(dynamic_cast<LayerUnrestricted*>(net_->get_node(layer_gid)))
	  {
	    throw TypeMismatch("topology_layer_grid", "topology_layer_free");
	  }

	node = 
	  layer->get_node(Position<int_t>(static_cast<index>(array[0]), 
						static_cast<index>(array[1])));
	if(node == 0)
	  {
	    throw UnknownNode();
	  }
      }
    else
      {
	throw TypeMismatch("topology_layer", "node");
      }

    i->OStack.pop(2);
    //Node GID is pushed onto stack.
    i->OStack.push(node->get_gid());
    i->EStack.pop();  
  }

  /*
    BeginDocumentation
    
    Name: topology::GetPosition - retrieve position of input node
    
    Synopsis: node_gid GetPosition -> [array]

    Parameters:
    node_gid      - gid of layer node
    [array]       - spatial position of node [x y]
		 
    Description: Retrieves spatial 2D position of layer node.
   
    Examples:

    topology using

    %%Create layer
    << /rows 5
       /columns 4
       /elements /iaf_neuron
    >> /dictionary Set

    dictionary CreateLayer /src Set 

    4 GetPosition
       
    Author: Kittel Austvoll
  */

  void TopologyModule::
  GetPosition_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    const index node_gid = getValue<long_t>(i->OStack.pick(0));

    Node const * const node = 
      dynamic_cast<Node*>(net_->get_node(node_gid));

    Position<double_t> pos = Layer::get_position(*node);
	
    Token result = pos.getToken();
	
    i->OStack.pop(1);
    //Position is pushed onto stack.
    i->OStack.push(result);
    i->EStack.pop();  

  }

  void TopologyModule::
  GetLayer_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    const index node_gid = getValue<long_t>(i->OStack.pick(0));

    Node const * const node = 
      dynamic_cast<Node*>(net_->get_node(node_gid));

    // Get parent layer and local id within parent of node
    Layer* layer = Layer::get_layer(*node);
    
    i->OStack.pop(1);

    if(layer)
      {
	//Layer GID is pushed onto stack.
	i->OStack.push(layer->get_gid());
      }
    else
      {
	// Root GID is pushed onto stack if node isn't member 
	// of any layer.
	i->OStack.push(0);
      }

    i->EStack.pop();  
  }

  /*
    BeginDocumentation
    
    Name: topology::Distance - compute distance between nodes
    
    Synopsis: from_gid to_gid Distance -> double
              from_pos to_gid Distance -> double

    Parameters:
    from_gid    - int, gid of node in a topology layer
    from_pos    - double vector, position in layer
    to_gid      - int, gid of node in a topology layer

    Returns:
    double - distance between nodes or given position and node
		 
    Description: 
    This function returns the distance between the position of the "from_gid"
    node or the explicitly given "from_pos" position and the position of the
    "to_gid" node. Nodes must be parts of topology layers.

    The "from" position is projected into the layer of the "to_gid" node. If
    this layer has periodic boundary conditions (EdgeWrap is true), then the
    shortest distance is returned, taking into account the 
    periodicity. Fixed grid layers are in this case extended so that the
    nodes at the edges of the layer have a distance of one grid unit when
    wrapped.
   
    Example:

    topology using
    << /rows 5
       /columns 4
       /elements /iaf_neuron
    >> CreateLayer ;

    4 5         Distance
    [0.2 0.3] 5 Distance
       
    Author: Hans E Plesser, Kittel Austvoll

    See also: Displacement, GetPosition
  */

  void TopologyModule::Distance_i_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const index from_gid = getValue<long_t>(i->OStack.pick(1));
    const index to_gid   = getValue<long_t>(i->OStack.pick(0));

    Node const * const from = dynamic_cast<Node*>(net_->get_node(from_gid));
    assert(from);
    Node const * const to   = dynamic_cast<Node*>(net_->get_node(to_gid  ));
    assert(to);

    const Position<double_t> d = TopologyModule::compute_displacement(*from, *to);
    i->OStack.pop(2);
    i->OStack.push(Token(d.length()));
    i->EStack.pop();  
  }

  void TopologyModule::Distance_a_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const Position<double_t> from_pos 
      = getValue<std::vector<double_t> >(i->OStack.pick(1));
    const index to_gid   = getValue<long_t>(i->OStack.pick(0));

    Node const * const to   = dynamic_cast<Node*>(net_->get_node(to_gid  ));
    assert(to);

    const Position<double_t> d = TopologyModule::compute_displacement(from_pos, *to);
    i->OStack.pop(2);
    i->OStack.push(Token(d.length()));
    i->EStack.pop();  
  }

  /*
    BeginDocumentation
    
    Name: topology::Displacement - compute displacement vector
    
    Synopsis: from_gid to_gid Displacement -> [double vector]
              from_pos to_gid Displacement -> [double vector]

    Parameters:
    from_gid    - int, gid of node in a topology layer
    from_pos    - double vector, position in layer
    to_gid      - int, gid of node in a topology layer

    Returns:
    [double vector] - vector pointing from position "from" to position "to"
		 
    Description: 
    This function returns a vector connecting the position of the "from_gid"
    node or the explicitly given "from_pos" position and the position of the
    "to_gid" node. Nodes must be parts of topology layers.

    The "from" position is projected into the layer of the "to_gid" node. If
    this layer has periodic boundary conditions (EdgeWrap is true), then the
    shortest displacement vector is returned, taking into account the 
    periodicity. Fixed grid layers are in this case extended so that the
    nodes at the edges of the layer have a distance of one grid unit when
    wrapped.
   
    Example:

    topology using
    << /rows 5
       /columns 4
       /elements /iaf_neuron
    >> CreateLayer ;

    4 5         Displacement
    [0.2 0.3] 5 Displacement
       
    Author: Hans E Plesser, Kittel Austvoll

    See also: Distance, GetPosition
  */
  
  void TopologyModule::Displacement_i_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const index from_gid = getValue<long_t>(i->OStack.pick(1));
    const index to_gid   = getValue<long_t>(i->OStack.pick(0));

    Node const * const from = dynamic_cast<Node*>(net_->get_node(from_gid));
    assert(from);
    Node const * const to   = dynamic_cast<Node*>(net_->get_node(to_gid  ));
    assert(to);

    const Position<double_t> d = TopologyModule::compute_displacement(*from, *to);
    i->OStack.pop(2);
    i->OStack.push(d.getToken());
    i->EStack.pop();  
  }

  void TopologyModule::Displacement_a_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const Position<double_t> from_pos 
      = getValue<std::vector<double_t> >(i->OStack.pick(1));
    const index to_gid   = getValue<long_t>(i->OStack.pick(0));

    Node const * const to   = dynamic_cast<Node*>(net_->get_node(to_gid));
    assert(to);

    const Position<double_t> d = TopologyModule::compute_displacement(from_pos, *to);
    i->OStack.pop(2);
    i->OStack.push(d.getToken());
    i->EStack.pop();  
  }

  Position<double_t> TopologyModule::compute_displacement(const Node& from,
							  const Node& to)
  {
    const Position<double_t> from_pos = Layer::get_position(from);

    return compute_displacement(from_pos, to);
  }

  Position<double_t> TopologyModule::compute_displacement(const Position<double_t>& from_pos,
							  const Node& to)
  {
    Layer* to_layer = Layer::get_layer(to);
    if ( !to_layer )
      throw LayerExpected();

    return to_layer->compute_displacement(from_pos, to);
  }

  /*
    BeginDocumentation
    
    Name: topology::DumpLayerNodes - write information about layer nodes to file
    
    Synopsis: ostream layer_gid DumpLayerNodes -> ostream

    Parameters:
    ostream   - open output stream
    layer_gid - topology layer
		 
    Description:
    Write information about each element in the given layer to the
    output stream. The file format is one line per element with the
    following contents:

    GID x-position y-position [z-position]

    X and y position are given as physical coordinates in the extent,
    not as grid positions. The number of decimals can be controlled by
    calling setprecision on the output stream before calling DumpLayerNodes.

    Note:
    In distributed simulations, this function should only be called for
    MPI rank 0. If you call it on several MPI ranks, you must use a
    different file name on each.

    Examples:

    topology using
    /my_layer << /rows 5 /columns 4 /elements /iaf_neuron >> CreateLayer def

    (my_layer_dump.lyr) (w) file
    my_layer DumpLayerNodes
    close
       
    Author: Kittel Austvoll, Hans Ekkehard Plesser
    
    SeeAlso: topology::DumpLayerConnections, setprecision, modeldict
  */

  void TopologyModule::
  DumpLayerNodes_os_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);
    
    const index layer_gid =  getValue<long_t>(i->OStack.pick(0));
    OstreamDatum out = getValue<OstreamDatum>(i->OStack.pick(1));

    Layer const * const layer = dynamic_cast<Layer*>(net_->get_node(layer_gid));

    if( layer != 0 && out->good() )
      layer->dump_nodes(*out);

    i->OStack.pop(1);  // leave ostream on stack
    i->EStack.pop();  
  }

  /*
    BeginDocumentation
    
    Name: topology::DumpLayerConnections - prints a list of the connections of the nodes in the layer to file
    
    Synopsis: ostream source_layer_gid synapse_type DumpLayerConnections -> ostream

    Parameters:
    ostream          - open outputstream
    source_layer_gid - topology layer
    synapse_type     - synapse model (literal)
		 
    Description: 
    Dumps information about all connections of the given type having their source in
    the given layer to the given output stream. The data format is one line per connection as follows:

    source_gid target_gid weight delay displacement[x,y,z]

    where displacement are up to three coordinates of the vector from the source to
    the target node. If targets do not have positions (eg spike detectors outside any layer),
    NaN is written for each displacement coordinate.

    Note:
    For distributed simulations
    - this function will dump the connections with local targets only.
    - the user is responsible for writing to a different output stream (file)
      on each MPI process.

    Examples:

    topology using
    ...
    (out.cnn) (w) file layer_gid /static_synapse PrintLayerConnections close
       
    Author: Kittel Austvoll, Hans Ekkehard Plesser
    
    SeeAlso: topology::DumpLayerNodes
  */

  void TopologyModule::
  DumpLayerConnections_os_i_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(3);

    OstreamDatum  out_file = getValue<OstreamDatum>(i->OStack.pick(2));
    std::ostream& out = *out_file;

    const index layer_gid = getValue<long_t>(i->OStack.pick(1));

    const std::string synname = getValue<std::string>(i->OStack.pick(0));
    const Token synapse = net_->get_synapsedict().lookup(synname);
    if ( synapse.empty() )
      throw UnknownSynapseType(synname);
    const long synapse_id = static_cast<long>(synapse);

    Layer* const layer = dynamic_cast<Layer*>(net_->get_node(layer_gid));
    if (layer == NULL)
      throw TypeMismatch("any layer type", "something else");

    // Get layer leaves
    LeafList nodes(*layer);

    // Iterate over leaves
    for( LeafList::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        DictionaryDatum dict = net_->get_connector_status(**it, synapse_id);

        TokenArray targets = getValue<TokenArray>(dict, names::targets);
        TokenArray weights = getValue<TokenArray>(dict, names::weights);
        TokenArray delays  = getValue<TokenArray>(dict, names::delays);

        assert(targets.size() == weights.size());
        assert(targets.size() == delays.size());

        const Position<double_t> source_pos = Layer::get_position(**it);

        // Print information about all connections for current leaf
        for ( size_t i = 0; i < targets.size(); ++i )
          {
            Node const * const target = net_->get_node(targets[i]);
            assert(target);

            // Print source, target, weight, delay, rports
            out << (*it)->get_gid() << ' ' << targets[i] << ' '
                << weights[i] << ' ' << delays[i];

            try
            {
                const Position<double_t> displacement =
                    TopologyModule::compute_displacement(source_pos, *target);
                out << ' ';
                displacement.print(out);
            } catch ( LayerExpected &le )
            {
                // Happens if target does not belong to layer, eg spike_detector.
                // We then print NaNs for the displacement, take dimension from
                // position of source node, which definitely has position as
                // it belongs to a layer.
                for ( int n = 0 ; n < source_pos.get_dim() ; ++n )
                  out << " NaN";
            }
            out << '\n';
          }
    }  // for LeafList ...

    i->OStack.pop(2);  // leave ostream on stack
    i->EStack.pop();
  }

  /*
    BeginDocumentation
    
    Name: topology::ConnectLayers - connect two layers
    
    Synopsis: sourcelayergid targetlayergid connection_dict 
    ConnectLayers -> -

    Description: Connects nodes in two topological layers. 

    The parameters set in the input dictionary decides the nature
    of the connection pattern being created. Please see parameter
    list below for a detailed description of these variables.
    
    The connections are created by iterating through either the 
    source or the target layer, consecutively connecting each node
    to a region in the opposing layer.

    Parameters:
    sourcelayergid  - GID of source layer 
    targetlayergid  - GID of target layer 

    connection_dict - dictionary containing any of the following 
                      elements:
    
    ------------------------------------------------------------------
    Connection dictionary parameters:
    ------------------------------------------------------------------
    Parameter name: connection-type               

    Type: string       

    Parameter description: 

    Decides the type of connection pattern being created (i.e. 
    convergent or divergent topological connection). A convergent
    topological connection is a connection between a source region 
    and a target node. A divergent topological connection is a 
    connection between a source node and a target region. A convergent
    topological connection can also be called a receptive field connection.
    A divergent topological connection can also be called a projective
    field connection. A one-to-one connection can be created by setting 
    the size of the source or target region equal to one. The connection 
    type has particular effect on the connection pattern when used together 
    with the number_of_connections variable.


    Parameter name: mask
    
    Type: dictionary

    Parameter description:

    The mask defines the region used in the connection type described
    above. There exists a selection of many different region sizes and
    shapes. Examples are the grid region, the rectangular, circular or
    doughnut region.

    The grid region takes an optional anchor parameter. The anchor
    parameter indicates which node of the grid region is aligned with
    the source node.


    Parameter name: weights, delays and kernel
    
    Type: dictionary

    Parameter description:

    These parameters can be initialised in many ways. Either as a constant
    value, with the help of a dictionary, or in an array (only for fixed 
    grid layers). The dictionary can be of type gaussian, 2D gaussian, 
    linear, exponential and other.


    Parameter name: source

    Type: dictionary

    Parameter description:

    The source dictionary enables us to give further detail on
    how the nodes in the source layer used in the connection function
    should be processed.

    Parameters:
    model*             literal
    lid^               integer

    *modeltype (i.e. /iaf_neuron) of nodes that should be connected to
    in the layer. All nodes are used if this variable isn't set.
    ^Nesting depth of nodes that should be connected to. All layers are used
    if this variable isn't set.


    Parameter name: target
    
    Type: dictionary

    Parameter description:

    See description for source dictionary.


    Parameter name: number_of_connections
    
    Type: integer

    Parameter description:

    Maximum number of connections that each iterating node is allowed.
    The actual connections being created are picked at random from all
    the candidate connections. 
    
.
    Parameter name: allow_autapses
    
    Type: bool

    Parameter description: Used together with the number_of_connections option to
    indicate if autapses are allowed.


    Parameter name: allow_multapses
    
    Type: bool

    Parameter description: Used together with the number_of_connections option to
    indicate if multapses are allowed.

    ------------------------------------------------------------------
   
    Example:

    topology using

    %Create source layer with CreateLayer
    << /rows 15
       /columns 43
       /extent [1.0 2.0]
       /elements /iaf_neuron
    >> /src_dictionary Set

    src_dictionary CreateLayer /src Set 

    %Create target layer with CreateLayer
    %%Create layer
    << /rows 34
       /columns 71
       /extent [3.0 1.0]
       /elements {/iaf_neuron Create ; /iaf_psc_alpha Create ;}
    >> /tgt_dictionary Set

    tgt_dictionary CreateLayer /tgt Set 

    <<	/connection_type (convergent)
        /mask << /grid << /rows 2 /columns 3 >>
	         /anchor << /row 4 /column 2 >> >>
	/weights 2.3
	/delays [2.3 1.2 3.2 1.3 2.3 1.2]
	/kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
	/sources << /model /iaf_neuron
		    /lid 1 >>
	/targets << /model /iaf_neuron
		    /lid 2 >>
	
    >> /parameters Set

    src tgt parameters ConnectLayers
       
    Author: Kittel Austvoll
    
    SeeAlso: topology::CreateLayer
  */

  void TopologyModule::
  ConnectLayers_i_i_dictFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(3);

    assert(net_ != 0);
    
    //Input starts.

    const index sources_gid = getValue<long_t>(i->OStack.pick(2));
    const index targets_gid = getValue<long_t>(i->OStack.pick(1));

    const DictionaryDatum connection_dict = 
      getValue<DictionaryDatum>(i->OStack.pick(0));

    //Input ends

    Layer* sources =
      dynamic_cast<Layer*>(net_->get_node(sources_gid));
    Layer* targets = 
      dynamic_cast<Layer*>(net_->get_node(targets_gid));

    if ( (sources == 0) || (targets == 0) )
    {
      throw LayerExpected();
    }

    //Create connections. The entire connection network is set up
    //by the ConnectionCreator(..) constructor.

    connection_dict->clear_access_flags();

    ConnectionCreator(sources, targets,
		      connection_dict, *net_,
		      i->verbosity()<=SLIInterpreter::M_INFO);

    std::string missed;
    if ( !connection_dict->all_accessed(missed) )
    {
      if ( NestModule::get_network().dict_miss_is_error() )
        throw UnaccessedDictionaryEntry(missed);
      else
	NestModule::get_network().message(SLIInterpreter::M_WARNING, "ConnectLayers", 
					  ("Unread dictionary entries: " + missed).c_str());
    }

    i->OStack.pop(3);
    i->EStack.pop();     
  }


  std::string LayerExpected::message()
  {
    return std::string("A topology module Layer was expected.");
  }

} // namespace nest

