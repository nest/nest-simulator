/*
 *  nestmodule.cpp
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
 *
 *  First Version: April 2002
 *
 */

#include "nestmodule.h"

#include <iostream>  
#include <sstream>
#include "nest.h"
#include "network.h"
#include "nodelist.h"
#include "leaflist.h"
#include "interpret.h"
#include "node.h"
#include "compound.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "booldatum.h"
#include "arraydatum.h"
#include "stringdatum.h"
#include "tokenutils.h"
#include "sliexceptions.h"
#include "random_datums.h"
#include "connectiondatum.h"
#include "communicator.h"
#include "genericmodel.h"

#include <set>

extern int SLIsignalflag;

namespace nest
{
  SLIType NestModule::ConnectionType;

  Network* NestModule::net_ = 0;

  // At the time when NestModule is constructed, the SLI Interpreter
  // must already be initialized. NestModule relies on the presence of
  // the following SLI datastructures: Name, Dictionary

  NestModule::NestModule()
  {
    // The network pointer must be initalized using register_network()
    // before the NestModule instance is created.
    assert(net_ != 0);
  }

  NestModule::~NestModule()
  {
    // The network is deleted outside NestModule, since the
    // dynamicloadermodule also needs it

    ConnectionType.deletetypename();
  }

  void NestModule::register_network(Network& net)
  {
    assert(net_ == 0);   // register_network() must be called once only
    net_ = &net;
  }

  // The following concerns the new module:

  const std::string NestModule::name(void) const
  {
    return std::string("NEST Kernel 2"); // Return name of the module
  }

  const std::string NestModule::commandstring(void) const
  {
    return std::string("/nest-init /C++ ($Revision: 9902 $) provide-component "
                       "/nest-init /SLI (1.21) require-component");
  }


  /* BeginDocumentation
     Name: ChangeSubnet - change the current working subnet.
     Synopsis:
     [adr] ChangeSubnet -> -
     gid   ChangeSubnet -> -
     Parameters:
     [adr]/gid - The address of GID of the new current subnet.
     Description:
     Change the current subnet to the one given as argument. Create
     will place newly created nodes in the current working subnet.
     ChangeSubnet is not allowed for layer subnets used in the
     topology module.
     
     This function can be used to change the working subnet to a new
     location, similar to the UNIX command cd.

     SeeAlso: CurrentSubnet
  */
  void NestModule::ChangeSubnet_aFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    TokenArray node_adr = getValue<TokenArray>(i->OStack.pick(0));

    if(get_network().get_node(node_adr)->allow_entry())
      get_network().go_to(node_adr);
    else
      throw SubnetExpected();
    
    i->OStack.pop();
    i->EStack.pop();
  }

  void NestModule::ChangeSubnet_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    index node_gid = getValue<long>(i->OStack.pick(0));

    if(get_network().get_node(node_gid)->allow_entry())
      get_network().go_to(node_gid);
    else
      throw SubnetExpected();
    
    i->OStack.pop();
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: CurrentSubnet - return the address of the current network node.

     Synopsis: CurrentSubnet -> array
     Description:
     CurrentSubnet returns the address of the current working subnet in form
     of an address array. The address must conform the semantics for network
     addresses.
     Availability: NEST
     SeeAlso: ChangeSubnet
     Author: Marc-Oliver Gewaltig
  */
  void NestModule::CurrentSubnetFunction::execute(SLIInterpreter *i) const
  {
    assert(get_network().get_cwn() != 0);
    vector<size_t> current = get_network().get_adr(get_network().get_cwn());

    i->OStack.push(ArrayDatum(current));
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: SetStatus - sets the value of a property of a node or object

     Synopsis: 
     [adr] dict SetStatus -> -
     gid   dict SetStatus -> -
     rdev  dict SetStatus -> -
     conn  dict SetStatus -> -

     Description:
     SetStatus is the tool to alter properties of nodes. These can be viewed
     with GetStatus info.
     SetStatus fulfills the same function for random number distributions
     and dictionaries used in SLI to represent the objects of oo-programming.
  
     Examples:
     /dc_generator Create /dc_gen Set  %Creates a dc_generator, which is a node
     dc_gen GetStatus info %view properties (amplitude is 0)
     dc_gen << /amplitude 1500. >> SetStatus
     dc_gen GetStatus info % amplitude is now 1500

     Author: docu by Sirko Straube

     SeeAlso: ShowStatus, GetStatus, info, modeldict, Set, SetStatus_v, SetStatus_dict
  */
  void NestModule::SetStatus_idFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    DictionaryDatum dict = getValue<DictionaryDatum>(i->OStack.top());
    index node_id = getValue<long>(i->OStack.pick(1));

    // Network::set_status() performs entry access checks for each
    // target and throws UnaccessedDictionaryEntry where necessary 
    get_network().set_status(node_id,dict);
		
    i->OStack.pop(2);
    i->EStack.pop();
  }

  void NestModule::SetStatus_CDFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    DictionaryDatum dict = getValue<DictionaryDatum>(i->OStack.top());
    ConnectionDatum conn = getValue<ConnectionDatum>(i->OStack.pick(1));
    DictionaryDatum conn_dict = conn.get_dict();

    long synapse_id = getValue<long>(conn_dict, nest::names::synapse_typeid);
    long port = getValue<long>(conn_dict, nest::names::port);
    long gid = getValue<long>(conn_dict, nest::names::source);
    thread tid = getValue<long>(conn_dict, nest::names::target_thread);
    get_network().get_node(gid); // Just to check if the node exists

    dict->clear_access_flags();

    get_network().set_synapse_status(gid, synapse_id, port, tid, dict);

    std::string missed;
    if ( !dict->all_accessed(missed) )
    {
      if ( get_network().dict_miss_is_error() )
        throw UnaccessedDictionaryEntry(missed);
      else
        get_network().message(SLIInterpreter::M_WARNING, "SetStatus", 
                              ("Unread dictionary entries: " + missed).c_str());
    }

    i->OStack.pop(2);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: GetStatus - return the property dictionary of a node or object
     Synopsis: 
     [adr] GetStatus -> dict
     gid   GetStatus -> dict
     rdev  GetStatus -> dict
     conn  GetStatus -> dict

     Description:
     GetStatus returns a dictionary with the status information 
     of the node, specified by its global id or address.
     GetStatus fulfills the same function for random number distributions
     and dictionaries used in SLI to represent the objects of oo-programming.

     The interpreter exchanges data with the network element using
     its status dictionary. To abbreviate the access pattern
          gid GetStatus /lit get
     a variant of get implicitly calls GetStatus
          gid /lit get .
     In this way network elements and dictionaries can be accessed
     with the same syntax. Sometimes access to nested data structures in 
     the status dictionary is required. In this case the advanced addressing
     scheme of get is useful in which the second argument is an array of 
     literals. See the documentation of get for details.

     The information contained in the property dictionary depends on the
     concrete node model.
     
     Please refer to the model documentation for details.
  
     Standard entries:

     global_id   - local ID of the node
     status      - integer, representing the status flags of the node
     model       - literal, defining the current node
     thread      - the thread the node is allocated on
     vp          - the virtual process a node belongs to
  
     Note that the standard entries cannot be modified directly.

     Author: Marc-Oliver Gewaltig
     Availability: NEST
     SeeAlso: ShowStatus, info, SetStatus, get, GetStatus_v, GetStatus_dict
  */
  void NestModule::GetStatus_iFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);
     
    index node_id = getValue<long>(i->OStack.pick(0));  
    DictionaryDatum dict= get_network().get_status(node_id);

    i->OStack.pop();
    i->OStack.push(dict);
    i->EStack.pop();
  }

  void NestModule::GetStatus_CFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    ConnectionDatum conn = getValue<ConnectionDatum>(i->OStack.pick(0));
    DictionaryDatum conn_dict = conn.get_dict();

    long synapse_id = getValue<long>(conn_dict, nest::names::synapse_typeid);
    long port = getValue<long>(conn_dict, nest::names::port);
    long gid = getValue<long>(conn_dict, nest::names::source);
    thread tid = getValue<long>(conn_dict, nest::names::target_thread);
    get_network().get_node(gid); // Just to check if the node exists
     
    DictionaryDatum result_dict = get_network().get_synapse_status(gid, synapse_id, port, tid);
    
    i->OStack.pop();
    i->OStack.push(result_dict);
    i->EStack.pop();
  }

  /*BeginDocumentation
    Name: SetDefaults - Set the default values for a node or synapse model.
    Synopsis: /modelname dict SetDefaults -> -
    SeeAlso: GetDefaults
    Author: Jochen Martin Eppler
    FirstVersion: September 2008
  */
  void NestModule::SetDefaults_l_DFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const Name modelname = getValue<Name>(i->OStack.pick(1));
    DictionaryDatum dict = getValue<DictionaryDatum>(i->OStack.pick(0));

    const Token nodemodel = get_network().get_modeldict().lookup(modelname);
    const Token synmodel = get_network().get_synapsedict().lookup(modelname);

    dict->clear_access_flags(); // set properties with access control

    if (!nodemodel.empty())
    {
      const index model_id = static_cast<index>(nodemodel);
      get_network().get_model(model_id)->set_status(dict);
    }
    else if (!synmodel.empty())
    {
      const index synapse_id = static_cast<index>(synmodel);
      get_network().set_connector_defaults(synapse_id, dict);
    }
    else
      throw UnknownModelName(modelname.toString());

    std::string missed;
    if (!dict->all_accessed(missed))
    {
      if (get_network().dict_miss_is_error())
        throw UnaccessedDictionaryEntry(missed);
      else
        get_network().message(SLIInterpreter::M_WARNING, "SetDefaults", ("Unread dictionary entries: " + missed).c_str());
    }

    i->OStack.pop(2);
    i->EStack.pop();
  }

  /*BeginDocumentation
    Name: GetDefaults - Return the default values for a node or synapse model.
    Synopsis: /modelname GetDefaults -> dict
    SeeAlso: SetDefaults
    Author: Jochen Martin Eppler
    FirstVersion: September 2008
  */
  void NestModule::GetDefaults_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    const Name modelname = getValue<Name>(i->OStack.pick(0));

    const Token nodemodel = get_network().get_modeldict().lookup(modelname);
    const Token synmodel = get_network().get_synapsedict().lookup(modelname);

    DictionaryDatum dict;
 
    if (!nodemodel.empty())
    {
      const long model_id = static_cast<long>(nodemodel);
      Model* m= get_network().get_model(model_id);
      dict = m->get_status();
    }
    else if (!synmodel.empty())
    {
      const long synapse_id = static_cast<long>(synmodel);
      dict = get_network().get_connector_defaults(synapse_id);
    }
    else
      throw UnknownModelName(modelname.toString());

    i->OStack.pop();
    i->OStack.push(dict);
    i->EStack.pop();
  }

  // params: params
  void NestModule::FindConnections_DFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    DictionaryDatum dict = getValue<DictionaryDatum>(i->OStack.pick(0));

    dict->clear_access_flags();
    ArrayDatum array = get_network().find_connections(dict);
    std::string missed;
    if ( !dict->all_accessed(missed) )
      {
	if ( get_network().dict_miss_is_error() )
	  throw UnaccessedDictionaryEntry(missed);
	else
	  get_network().message(SLIInterpreter::M_WARNING, "FindConnections", 
				("Unread dictionary entries: " + missed).c_str());
      }
     
    i->OStack.pop();
    i->OStack.push(array);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: Simulate - simulate n milliseconds
  
     Synopsis:
     n(int) Simulate -> -
  
     Description: Simulate the network for n milliseconds. 
     Use resume to continue the simulation after an interrupt.

     SeeAlso: resume, ResumeSimulation, unit_conversion
  */
  void NestModule::SimulateFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    const double time= i->OStack.top();

    std::ostringstream os;
    os << "Simulating " << time << " ms.";
    i->message(SLIInterpreter::M_INFO, "Simulate", os.str().c_str());
    Time t = Time::ms(time);

    //experimental callback and signal safe, uncomment for testing, MD 090105
        //i->EStack.push(i->execbarrier_token);  // protect by barrier
    get_network().simulate(t);
        //i->EStack.pop();                       // pop the barrier


    // successful end of simulate
    i->OStack.pop();
    i->EStack.pop();
  }

  /*BeginDocumentation
    Name: ResumeSimulation - resume an interrupted simulation
    SeeAlso: Simulate
  */
  void NestModule::ResumeSimulationFunction::execute(SLIInterpreter *i) const
  {
    get_network().resume();
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: CopyModel - copy a model to a new name, set parameters for copy, if given
     Synopsis:
     /model /new_model param_dict -> -
     /model /new_model            -> -
     Parameters:
     /model      - literal naming an existing model
     /new_model  - literal giving the name of the copy to create, must not
                   exist in modeldict or synapsedict before
     /param_dict - parameters to set in the new_model
     Description: 
     A copy of model is created and registered in modeldict or synapsedict
     under the name new_model. If a parameter dictionary is given, the parameters
     are set in new_model.
     Warning: It is impossible to unload modules after use of CopyModel.
   */
  void NestModule::CopyModel_l_l_DFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(3);

    // fetch existing model name from stack
    const Name oldmodname = getValue<Name>(i->OStack.pick(2));
    const std::string newmodname = getValue<std::string>(i->OStack.pick(1));
    DictionaryDatum dict = getValue<DictionaryDatum>(i->OStack.pick(0));

    const Dictionary &modeldict = get_network().get_modeldict();
    const Dictionary &synapsedict = get_network().get_synapsedict();

    if (modeldict.known(newmodname) || synapsedict.known(newmodname))
      throw NewModelNameExists(newmodname);

    dict->clear_access_flags(); // set properties with access control
    const Token oldnodemodel = modeldict.lookup(oldmodname);
    const Token oldsynmodel = synapsedict.lookup(oldmodname);

    if (!oldnodemodel.empty())
    {
      const index old_id = static_cast<index>(oldnodemodel);
      const index new_id = get_network().copy_model(old_id, newmodname);
      get_network().get_model(new_id)->set_status(dict);
    }
    else if (!oldsynmodel.empty())
    {
      const index old_id = static_cast<index>(oldsynmodel);
      const index new_id = get_network().copy_synapse_prototype(old_id, newmodname);
      get_network().set_connector_defaults(new_id, dict);
    }
    else
      throw UnknownModelName(oldmodname);

    std::string missed;
    if ( !dict->all_accessed(missed) )
    {
      if ( get_network().dict_miss_is_error() )
        throw UnaccessedDictionaryEntry(missed);
      else
        get_network().message(SLIInterpreter::M_WARNING, "CopyModel", ("Unread dictionary entries: " + missed).c_str());
    }
    
    i->OStack.pop(3);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: Create - create a number of equal nodes in the current subnet
     Synopsis:
     /model          Create -> gid
     /model n        Create -> gid
     /model   params Create -> gid
     /model n params Create -> gid
     Parameters:
     /model - literal naming the modeltype (entry in modeldict)
     n      - the desired number of nodes
     params - parameters for the newly created node(s)
     gid    - gid of last created node
     Description:
     Create generates n new network objects of the supplied model
     type. If n is not given, a single node is created. The objects
     are added as children of the current working node. params is a
     dictsionary with parameters for the new nodes.
 
     SeeAlso: modeldict, ChangeSubnet
  */
  void NestModule::Create_l_iFunction::execute(SLIInterpreter *i) const
  {
    // check for stack load
    i->assert_stack_load(2);

    // extract arguments
    const long n_nodes = getValue<long>(i->OStack.pick(0));
    if ( n_nodes <= 0 )
      throw RangeCheck();

    const std::string modname = getValue<std::string>(i->OStack.pick(1));
    const Token model = get_network().get_modeldict().lookup(modname);
    if ( model.empty() )
      throw  UnknownModelName(modname);
       
    // create
    const long model_id = static_cast<long>(model);
    const long last_node_id = get_network().add_node(model_id, n_nodes);
    i->OStack.pop(2);
    i->OStack.push(last_node_id);
    i->EStack.pop();
  }

  void NestModule::GetNodes_i_bFunction::execute(SLIInterpreter *i) const
  {
     i->assert_stack_load(2);

     const bool  include_remote = not getValue<bool>(i->OStack.pick(0));
     const index node_id        = getValue<long>(i->OStack.pick(1));
     Compound *subnet = dynamic_cast<Compound *>(get_network().get_node(node_id));
     
     if (subnet==NULL)
       throw SubnetExpected();
 
     NodeList nodes(*subnet);

     ArrayDatum result;
     if ( include_remote )
       result.reserve(nodes.size());
     else
       result.reserve(nodes.size() / get_network().get_num_processes());

     for(NodeList::iterator n = nodes.begin(); n != nodes.end(); ++n)
       if ( include_remote or (*n)->is_local() )
	 result.push_back(new IntegerDatum((*n)->get_gid()));
     
     i->OStack.pop(2);
     i->OStack.push(result);
     i->EStack.pop();
   }

  void NestModule::GetChildren_i_bFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const bool  include_remote = not getValue<bool>(i->OStack.pick(0));
    const index node_id        = getValue<long>(i->OStack.pick(1));
    Compound *subnet = dynamic_cast<Compound *>(get_network().get_node(node_id));
     
    if (subnet == NULL)
      throw SubnetExpected();
 
    ArrayDatum result;
    if ( include_remote )
      result.reserve(subnet->size());
    else
      result.reserve(subnet->size() / get_network().get_num_processes());
     
    for(vector<Node *>::iterator n = subnet->begin(); n != subnet->end(); ++n)
      if ( include_remote or (*n)->is_local() )
	result.push_back(new IntegerDatum((*n)->get_gid()));

    i->OStack.pop(2);
    i->OStack.push(result);
    i->EStack.pop();
  }

  void NestModule::GetLeaves_i_bFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);

    const bool  include_remote = not getValue<bool>(i->OStack.pick(0));
    const index node_id        = getValue<long>(i->OStack.pick(1));
    Compound *subnet = dynamic_cast<Compound *>(get_network().get_node(node_id));
     
    if (subnet == NULL)
      throw SubnetExpected();
 
    LeafList nodes(*subnet);

    ArrayDatum result;
    if ( include_remote )
      result.reserve(nodes.size());
    else
      result.reserve(nodes.size() / get_network().get_num_processes());
     
    for(LeafList::iterator n = nodes.begin(); n != nodes.end(); ++n)
      if ( include_remote or (*n)->is_local() )
	result.push_back(new IntegerDatum((*n)->get_gid()));
     
    i->OStack.pop(2);
    i->OStack.push(result);
    i->EStack.pop();
  }

  /* BeginDocumentation      
     Name: GetGID - Return the global ID of a node
     
     Synopsis: [address] GetGID -> gid
   
     Parameters:
     [address] - address of the node
     gid       - Global id of a node

     Description:
     This function returns the global node ID which belongs to the
     specified network address.

     SeeAlso: GetAddress
  */
  void NestModule::GetGIDFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    TokenArray node_adr;

    node_adr = getValue<TokenArray>(i->OStack.pick(0));
    Node* node = get_network().get_node(node_adr);

    i->OStack.pop();
    i->OStack.push(node->get_gid());
    i->EStack.pop();
  }

  /* BeginDocumentation      
     Name: GetLID - Return the local ID of a node
   
     Synopsis: GID GetLID -> lid
   
     Parameters:
     gid       - Global id of a node
     lid       - Local if of the node

     Description:
     This function returns the local node ID of a node within its parent subnet.

     SeeAlso: GetAddress, GetGID
  */
  void NestModule::GetLIDFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    long gid  =  i->OStack.pick(0);
    Node * node= get_network().get_node(gid);
     
    i->OStack.pop();
    i->OStack.push(node->get_lid()+1);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: GetAddress - Return the address of a node
     Synopsis: gid GetAddress -> [adr]
     Parameterrs:
     gid   - Global id of a node
     [adr] - address of the node

     Description:
     This function returns the network address which belongs to
     the specified  node gid (global id).
  */
  void NestModule::GetAddressFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    long gid  =  i->OStack.pick(0);
    ArrayDatum node_adr(get_network().get_adr(gid));
     
    i->OStack.pop();
    i->OStack.push(node_adr);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: ResetKernel - Put the simulation kernel back to its initial state.
     Description: 
     This function re-initializes the simulation kernel, returning it to the 
     same state as after NEST has started. 
     In particular,
     - all network nodes
     - all connections
     - all user-defined neuron and synapse models
     are deleted, and 
     - time 
     - random generators
     are reset. The only exception is that dynamically loaded modules are not 
     unloaded. This may change in a future version of NEST. The SLI interpreter
     is not affected by ResetKernel.
     Availability: NEST
     Author: Marc-oliver Gewaltig
     SeeAlso: ResetNetwork, reset, ResetOptions
  */
  void NestModule::ResetKernelFunction::execute(SLIInterpreter *i) const
  {
    get_network().reset_kernel();
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: ResetNetwork - Reset the dynamic state of the network.
     Synopsis: ResetNetwork -> -
     Description:
     ResetNetwork resets the dynamic state of the entire network to its state 
     at T=0. The dynamic state comprises typically the membrane potential, 
     synaptic currents, buffers holding input that has been delivered, but not
     yet become effective, and all events pending delivery. Technically, this
     is achieve by calling init_state() on all nodes and forcing a call to 
     init_buffers() upon the next call to Simulate. Node parameters, such as
     time constants and threshold potentials, are not affected.
     
     Note: 
     - Time and random number generators are NOT reset.
     - Files belonging to recording devices (spike detector, voltmeter, etc)
       are closed. You must change the file name before simulating again, otherwise
       the files will be overwritten og you will receive an error, depending on
       the value of /overwrite_files (in root node).
     - ResetNetwork will reset the nodes to the state values stored in the model
       prototypes. So if you have used SetDefaults to change a state value of a
       model since you called Simulate the first time, the network will NOT be reset
       to the status at T=0.
     - The dynamic state of synapses with internal dynamics (STDP, facilitation) is
       NOT reset at present. This will be implemented in a future version of NEST.

     SeeAlso: ResetKernel, reset
  */
  void NestModule::ResetNetworkFunction::execute(SLIInterpreter *i) const
  {
    get_network().reset_network();
    i->message(SLIInterpreter::M_INFO, "ResetNetworkFunction",
      "The network has been reset. Random generators and time have NOT been reset.");
      
    i->EStack.pop();
  }

  // Connect for gid gid
  // See lib/sli/nest-init.sli for details
  void NestModule::Connect_i_i_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(3);
    
    index source = getValue<long>(i->OStack.pick(2));
    index target = getValue<long>(i->OStack.pick(1));
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().connect(source, target, synmodel_id);
    
    i->OStack.pop(3);
    i->EStack.pop();
  }

  // Connect for gid gid weight delay
  // See lib/sli/nest-init.sli for details
  void NestModule::Connect_i_i_d_d_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(5);

    index source = getValue<long>(i->OStack.pick(4));
    index target = getValue<long>(i->OStack.pick(3));
    double_t weight = getValue<double_t>(i->OStack.pick(2));
    double_t delay  = getValue<double_t>(i->OStack.pick(1));
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().connect(source, target, weight, delay, synmodel_id);
    
    i->OStack.pop(5);
    i->EStack.pop();
  }

  // Connect for gid gid dict
  // See lib/sli/nest-init.sli for details
  void NestModule::Connect_i_i_D_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(4);
     
    index source = getValue<long>(i->OStack.pick(3));
    index target = getValue<long>(i->OStack.pick(2));
    DictionaryDatum params = getValue<DictionaryDatum>(i->OStack.pick(1));
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    params->clear_access_flags();
 
    if ( get_network().connect(source, target, params, synmodel_id) )
    {
      // dict access control only if we actually made a connection
      std::string missed;
      if ( !params->all_accessed(missed) )
      {
	if ( get_network().dict_miss_is_error() )
	  throw UnaccessedDictionaryEntry(missed);
	else
	  get_network().message(SLIInterpreter::M_WARNING, "Connect", 
				("Unread dictionary entries: " + missed).c_str());
      }
    }

    i->OStack.pop(4);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: CompoundConnect - Connect a source compound to a target compound.

     Synopsis: 
     sources targets radius           CompoundConnect -> -
     sources targets radius /synmodel CompoundConnect -> -

     Options:
     If not given, the synapse model is taken from the Options dictionary
     of the Connect command.

     Description:
     Connect every node in a source compound to a selection of nodes in a 
     target compound.

     Goes through every target node and connects it to source nodes.
     Source nodes connected is in the within the range 'radius' from
     target.
     
     Uses two for-loops. The outer for-loop shifts the connection
     process from one row to the next. The inner for-loop goes through
     all the columns for every row. Every target node gets connected
     to source nodes defined by the scope list. The scope is being
     reset at the beginning of every row, and is being shifted to the
     right as the for-loops traverse the columns.
     
     Nodes outside the boundaries of the source compound are given the
     value false in the scope list. The nodes in the scope list gets
     connected to the target by use of the function
     convergent_connect.

     Author: Kittel Austvoll, Hans Ekkehard Plesser
     FirstVersion: 24.10.2006
     SeeAlso: Connect
  */
  void NestModule::CompoundConnect_i_i_i_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(4);
     
    index node_id = getValue<long>(i->OStack.pick(3));
    Compound *sources=dynamic_cast<Compound *>(get_network().get_node(node_id));
     
    if (sources==NULL)
    {
      i->message(SLIInterpreter::M_ERROR, "CompoundConnect","Input sources must be a compound.");
      throw SubnetExpected();
    }

    node_id = getValue<long>(i->OStack.pick(2));
    Compound *targets=dynamic_cast<Compound *>(get_network().get_node(node_id));

    if (targets==NULL)
    {
      i->message(SLIInterpreter::M_ERROR, "CompoundConnect","Input targets must be a compound.");
      throw SubnetExpected();
    }
 
    long radius = getValue<long>(i->OStack.pick(1));

    if(radius<0)
    {
      i->message(SLIInterpreter::M_ERROR, "CompoundConnect","Radius must be a positive integer.");
      throw RangeCheck();
    }

    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);
     
    get_network().compound_connect(*sources, *targets, radius, synmodel_id);
     
    i->OStack.pop(4);
    i->EStack.pop(); 
  }

  /* BeginDocumentation
     Name: DivergentConnect - Connect node to a population of nodes.
     
     Synopsis:
     source [targets]                              DivergentConnect -> -
     source [targets]                    /synmodel DivergentConnect -> -
     source [targets] [weights] [delays]           DivergentConnect -> -
     source [targets] [weights] [delays] /synmodel DivergentConnect -> -

     Parameters: 
     source    - GID of source node 
     [targets] - array of (global IDs of) potential target nodes 
     [weights] - weights for the connections. List of the same size as targets or 1.
     [delays]  - delays for the connections. List of the same size as targets or 1.
     /synmodel - The synapse model for the connection (see Options below)

     Options:
     If not given, the synapse model is taken from the Options dictionary
     of the Connect command.

     Description:
     Connect a neuron with a set of target neurons.
     
     Author:
     Marc-Oliver Gewaltig
     modified Ruediger Kupper, 20.3.2003
     modified Jochen Eppler, 04.11.2005
     SeeAlso: RandomDivergentConnect, ConvergentConnect
  */
  
  void NestModule::DivergentConnect_i_ia_a_a_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(5);

    long source_adr = getValue<long>(i->OStack.pick(4));
    TokenArray target_adr = getValue<TokenArray>(i->OStack.pick(3));

    TokenArray weights = getValue<TokenArray>(i->OStack.pick(2));
    TokenArray delays = getValue<TokenArray>(i->OStack.pick(1));

    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));
    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().divergent_connect(source_adr, target_adr, weights, delays, synmodel_id);

    i->OStack.pop(5);
    i->EStack.pop();
  }

  // Documentation can be found in lib/sli/nest-init.sli near definition
  // of the trie for RandomConvergentConnect.
  void NestModule::RDivergentConnect_i_i_ia_da_da_b_b_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(8);
     
    long source_adr = getValue<long>(i->OStack.pick(7));
    size_t n = getValue<long>(i->OStack.pick(6));
    TokenArray target_adr = getValue<TokenArray>(i->OStack.pick(5));
    TokenArray weights = getValue<TokenArray>(i->OStack.pick(4));
    TokenArray delays = getValue<TokenArray>(i->OStack.pick(3));      
    bool allow_multapses = getValue<bool>(i->OStack.pick(2));
    bool allow_autapses = getValue<bool>(i->OStack.pick(1));

    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));
    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().random_divergent_connect(source_adr, target_adr, n, weights, delays, allow_multapses, allow_autapses, synmodel_id);

    i->OStack.pop(8);
    i->EStack.pop();     
  }


  /* BeginDocumentation
     Name: ConvergentConnect - Connect population of nodes to a single node.

     Synopsis: 
     [sources] target                    /synmodel ConvergentConnect -> -
     [sources] target                    /synmodel ConvergentConnect -> -
     [sources] target [weights] [delays] /synmodel ConvergentConnect -> -

     Parameters: 
     [source]  - GID of source nodes
     target    - array of (global IDs of) potential target nodes 
     [weights] - weights for the connections. List of the same size as sources or 1.
     [delays]  - delays for the connections. List of the same size as sources or 1.
     /synmodel - The synapse model for the connection (see Options below)

     Options:
     If not given, the synapse model is taken from the Options dictionary
     of the Connect command.

     Description:
     Connect a set of source neurons to a single neuron.

     Author:
     Ruediger Kupper, via copy-paste-and-modify from Oliver's DivergentConnect.
     Modified Ruediger Kupper, 20.03.2003
     Modified Jochen Martin Eppler, 03.03.2009
     SeeAlso: RandomConvergentConnect, DivergentConnect
  */

  void NestModule::ConvergentConnect_ia_i_a_a_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(5);

    TokenArray source_adr = getValue<TokenArray>(i->OStack.pick(4));
    long target_adr = getValue<long>(i->OStack.pick(3));

    TokenArray weights = getValue<TokenArray>(i->OStack.pick(2));
    TokenArray delays = getValue<TokenArray>(i->OStack.pick(1));
     
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));
    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().convergent_connect(source_adr, target_adr, weights, delays, synmodel_id);

    i->OStack.pop(5);
    i->EStack.pop();
  }
  
     
  // Documentation can be found in lib/sli/nest-init.sli near definition
  // of the trie for RandomConvergentConnect.
  void NestModule::RConvergentConnect_ia_i_i_da_da_b_b_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(8);
     
    TokenArray source_adr = getValue<TokenArray>(i->OStack.pick(7));
    long target_adr = getValue<long>(i->OStack.pick(6));
    size_t n = getValue<long>(i->OStack.pick(5));
    TokenArray weights = getValue<TokenArray>(i->OStack.pick(4));
    TokenArray delays = getValue<TokenArray>(i->OStack.pick(3));      
    bool allow_multapses = getValue<bool>(i->OStack.pick(2));
    bool allow_autapses = getValue<bool>(i->OStack.pick(1));

    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));
    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    get_network().random_convergent_connect(source_adr, target_adr, n, weights, delays, allow_multapses, allow_autapses, synmodel_id);

    i->OStack.pop(8);
    i->EStack.pop();     
  }


  /* BeginDocumentation
     Name: NetworkDimensions - Determine dimensions of a subnet

     Synopsis:
     [sourcelayeradr] NetworkDimensions -> [height width]

     Description:
     NetworkDimensions corresponds to the command LayoutNetwork
     in the same way as the command
     Dimensions corresponds to the command LayoutArray.
     NetworkDimensions takes the address of a subnet tree and returns a list with
     dimensions [d1 d2 ... dn] where di gives the dimension of the
     subnet tree at level i.
     The length of the dimensions-list corresponds to the Depth of
     the subnet tree.
     
     Note:
     NetworkDimensions assumes that the subnet tree is hyper-rectangular
     (rectangle, cuboid, ...), i.e., all subnets at a given level
     have the same number of nodes.
     NetworkDimensions does not check, if the subnet tree really is
     hyper-rectangular. It will not fail if this is not the case. Instead,
     the dimensions that are returned correspond to the number of
     nodes in the first subnet in each level.

     Alternatives: NetworkDimensions_a (undocumented) -> behaviour and synopsis are the same.
     Availability: NEST
     Author: Rüdiger Kupper
     SeeAlso: LayoutNetwork, Dimensions
  */
  void NestModule::NetworkDimensions_aFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);
  
    TokenArray compound_adr = getValue<TokenArray>(i->OStack.pick(0));
    Compound  *compound_ptr; //!< pointer to source layer
     
    compound_ptr = dynamic_cast<Compound*>(get_network().get_node(compound_adr));
    if (compound_ptr == NULL)
        throw SubnetExpected();
    
    // determine dimensions:
    TokenArray result;
    index sh;  //!< number of nodes of subnet
  
    // stopping conditions for loop:
    // 1. the next level node is not a compound (non-empty subnet)
    // 2. there is no next level (empty subnet)
    do
    {
      sh = compound_ptr->size();
      result.push_back(new IntegerDatum(sh));
      if (sh == 0) break;
      compound_ptr = dynamic_cast<Compound*>((*compound_ptr)[0]);
    }
    while ( compound_ptr );

    i->OStack.pop(1);
    i->OStack.push(ArrayDatum(result));

    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: MemoryInfo - Report current memory usage.
     Description:
     MemoryInfo reports the current utilization of the memory manager for all models,
     which are used at least once. The output is sorted ascending according according
     to the name of the model is written to stdout. The unit of the data is byte.
     Note that MemoryInfo only gives you information about the memory requirements of
     the static model data inside of NEST. It does not tell anything about the memory
     situation on your computer. 
     Synopsis:
     MemoryInfo -> -
     Availability: NEST
     Author: Jochen Martin Eppler
  */
  void NestModule::MemoryInfoFunction::execute(SLIInterpreter *i) const
  {
    get_network().memory_info();
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: PrintNetwork - Print network tree in readable form.
     Description:
     Synopsis: 
     [adr] depth  PrintNetwork -> -
     Parameters: 
     [adr]       - Address of the root subnet to start tree printout. 
     depth      - Integer, specifies down to which level the network is printed.
     Description:
     This function prints the network structure in a concise tree-like format 
     according to the following rules:
     - Each Node is shown on a separate line, showing its model name followed 
     by its in global id in brackets.
     
     +-[0] Compound Dim=[1]
     |
     +- iaf_neuron [1]
     
     - Consecutive Nodes of the same model are summarised in a list. 
     The list shows the model name, the global id of the first node in the
     sequence, then the number of consecutive nodes, then the global id of 
     the last node in the sequence.
     
     +-[0] Compound Dim=[1]
     |
     +- iaf_neuron [1]..(2)..[2]
     
     - If a node is a compound, its global id is printed first, followed by the model
     name or its label (if it is defined). Next, the dimension is shown.
     If the current recursion level is less than the specified depth, the printout descends
     to the children of the compound. 
     After the header, a new line is printed, followed by the list of children
     at the next indentation level.
     After the last child, a new line is printed and the printout of the parent compound
     is continued.

     Example:
     SLI ] /iaf_neuron Create
     SLI [1] /iaf_cond_alpha 10 Create
     SLI [2] /dc_generator [2 5 6] LayoutNetwork
     SLI [3] [0] 1 PrintNetwork
     +-[0] Compound Dim=[12]
        |
        +- iaf_neuron [1]
        +- lifb_cond_neuron [2]..(10)..[11]
        +-[12] Compound Dim=[2 5 6]
     SLI [3] [0] 2 PrintNetwork
     +-[0] Compound Dim=[12]
        |
        +- iaf_neuron [1]
        +- lifb_cond_neuron [2]..(10)..[11]
        +-[12] Compound Dim=[2 5 6]
            |
            +-[13] Compound Dim=[5 6]
            +-[49] Compound Dim=[5 6]
     SLI [3] [0] 3 PrintNetwork
     +-[0] Compound Dim=[12]
        |
        +- iaf_neuron [1]
        +- lifb_cond_neuron [2]..(10)..[11]
        +-[12] Compound Dim=[2 5 6]
            |
            +-[13] Compound Dim=[5 6]
            |   |
            |   +-[14] Compound Dim=[6]
            |   +-[21] Compound Dim=[6]
            |   +-[28] Compound Dim=[6]
            |   +-[35] Compound Dim=[6]
            |   +-[42] Compound Dim=[6]
            +-[49] Compound Dim=[5 6]
                |
                +-[50] Compound Dim=[6]
                +-[57] Compound Dim=[6]
                +-[64] Compound Dim=[6]
                +-[71] Compound Dim=[6]
                +-[78] Compound Dim=[6]
     SLI [3] [0] 4 PrintNetwork
     +-[0] Compound Dim=[12]
        |
        +- iaf_neuron [1]
        +- lifb_cond_neuron [2]..(10)..[11]
        +-[12] Compound Dim=[2 5 6]
            |
            +-[13] Compound Dim=[5 6]
            |   |
            |   +-[14] Compound Dim=[6]
            |   |   |
            |   |   +- dc_generator [15]..(6)..[20]
            |   |
            |   +-[21] Compound Dim=[6]
            |   |   |
            |   |   +- dc_generator [22]..(6)..[27]
            |   |
            |   +-[28] Compound Dim=[6]
            |   |   |
            |   |   +- dc_generator [29]..(6)..[34]
            |   |
            |   +-[35] Compound Dim=[6]
            |   |   |
            |   |   +- dc_generator [36]..(6)..[41]
            |   |
            |   +-[42] Compound Dim=[6]
            |       |
            |       +- dc_generator [43]..(6)..[48]
            |
            +-[49] Compound Dim=[5 6]
                |
                +-[50] Compound Dim=[6]
                |   |
                |   +- dc_generator [51]..(6)..[56]
                |
                +-[57] Compound Dim=[6]
                |   |
                |   +- dc_generator [58]..(6)..[63]
                |
                +-[64] Compound Dim=[6]
                |   |
                |   +- dc_generator [65]..(6)..[70]
                |
                +-[71] Compound Dim=[6]
                |   |
                |   +- dc_generator [72]..(6)..[77]
                |
                +-[78] Compound Dim=[6]
                    |
                    +- dc_generator [79]..(6)..[84]


     Availability: NEST
     Author: Marc-Oliver Gewaltig, Jochen Martin Eppler
  */
  void NestModule::PrintNetworkFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);
    
    TokenArray node_adr = getValue<TokenArray>(i->OStack.pick(1));
    long depth = getValue<long>(i->OStack.pick(0));
    get_network().print(node_adr, depth - 1);

    i->OStack.pop(2);
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: Rank - Return the MPI rank (MPI_Comm_rank) of the process.
     Availability: NEST 2.0
     Author: Jochen Martin Eppler
     FirstVersion: January 2006
     SeeAlso: NumProcesses, SyncProcesses, MPIProcessorName 
  */
  void NestModule::RankFunction::execute(SLIInterpreter *i) const
  {
    i->OStack.push(Communicator::get_rank());
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: NumProcesses - Return the number of MPI processes (MPI_Comm_size).
     Availability: NEST 2.0
     Author: Jochen Martin Eppler
     FirstVersion: January 2006
     SeeAlso: Rank, SyncProcesses, MPIProcessorName
  */
  void NestModule::NumProcessesFunction::execute(SLIInterpreter *i) const
  {
    i->OStack.push(Communicator::get_num_processes());
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: SyncProcesses - Synchronize all MPI processes.
     Availability: NEST 2.0
     Author: Alexander Hanuschkin
     FirstVersion: April 2009
     Description:
     This function allows to synchronize all MPI processes at any
     point in a simulation script. Internally, the function uses
     MPI_Barrier(). Note that during simulation the processes are
     automatically synchronized without the need for user interaction.
     SeeAlso: Rank, NumProcesses, MPIProcessorName
   */
   void NestModule::SyncProcessesFunction::execute(SLIInterpreter *i) const
   {
     Communicator::synchronize();
     i->EStack.pop();
   }

   void NestModule::TimeCommunication_i_i_bFunction::execute(SLIInterpreter *i) const 
   { 
     i->assert_stack_load(3); 
     long samples = getValue<long>(i->OStack.pick(2)); 
     long num_bytes = getValue<long>(i->OStack.pick(1)); 
     bool offgrid = getValue<bool>(i->OStack.pick(0));

     double_t time = 0.0;
     if ( offgrid )
       time = Communicator::time_communicate_offgrid(num_bytes,samples);
     else
       time = Communicator::time_communicate(num_bytes,samples);

     i->OStack.pop(3); 
     i->OStack.push(time);
     i->EStack.pop(); 
   } 

  /* BeginDocumentation
     Name: MPIProcessorName - Return an unique specifier for the compute node (MPI_Get_processor_name).
     Availability: NEST 2.0
     Author: Alexander Hanuschkin
     FirstVersion: April 2009
     Description:
     This function returns the name of the processor it was called
     on. See MPI documentation for more details. If NEST is not
     compiled with MPI support, this function returns the hostname of
     the machine as returned by the POSIX function gethostname().
     Examples:
     (I'm process ) =only Rank 1 add =only ( of ) =only NumProcesses =only ( on machine ) =only MPIProcessorName =
     SeeAlso: Rank, NumProcesses, SyncProcesses
  */
  void NestModule::MPIProcessorNameFunction::execute(SLIInterpreter *i) const
  {
    i->OStack.push(Communicator::get_processor_name());
    i->EStack.pop();
  }


  /* BeginDocumentation
     Name: GetVpRNG - return random number generator associated to virtual process of node
     Synopsis:
     gid GetVpRNG -> rngtype
     Parameters: 
     gid  - global id of the node  
     Description: 
     This function is helpful in the implementation of parallelized wiring
     routines that create idential random structures independent of the
     number of machines and threads participating in the simulation. The
     function is used in SLI libraries, e.g. in the implementation of
     RandomConvergentConnect. There is probably no need to directly use
     GetVpRNG in scripts describing a particular simulation.

     In NEST each node (e.g. neuron) is assigned to a virtual process and
     each virtual process maintains its own random number generator. In a
     simulation run the virtual processes are equally distributed over the
     participating machines and threads as speciefied by the user. In NEST
     2.0 virtual processes are identified with threads.  Thus, with the
     option /total_num_virtual_procs of [0] set to n, there are in total
     always n threads (virtual processes) independent of the number of
     participating machines.  The concept of virtual processes is described
     in detail in [1].

     Identical results are achieved independent of the number of machines
     and threads participating in a simulation if all operations modifying
     a neuron and its incoming synapses use the random number generator of
     the virtual process the neuron is assigned to.

     An ArgumentTypeError is raised if GetVpRNG is called for a 
     non-local gid.

     Examples:
     In the implementation of RandomConvergentConnect the Connect
     operations are only carried out on the machine the target neuron lives
     on. Whether the neuron is located on a specific machine is tested
     using the /local property of the neuron.  The random selection of
     source neurons is made using the random number generator of the thread
     the target neuron is assigned to.

     References:
     [1] Morrison A, Mehring C, Geisel T, Aertsen A, and Diesmann M (2005)
         Advancing the boundaries of high connectivity network simulation 
         with distributed computing. Neural Computation 17(8):1776-1801
         The article is available at www.nest-initiative.org

     Author: Tobias Potjans, Moritz Helias, Diesmann     
     SeeAlso: GetGlobalRNG, RandomConvergentConnect
  */
  void NestModule::GetVpRngFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);
     
    index target = getValue<long>(i->OStack.pick(0));
    Node* target_node = get_network().get_node(target);

    if (!get_network().is_local_node(target_node))
      throw LocalNodeExpected(target);

    // Only nodes with proxies have a well-defined VP and thus thread.
    // Asking for the VP of, e.g., a subnet or spike_detector is meaningless.
    if ( !target_node->has_proxies() )
      throw NodeWithProxiesExpected(target);

    librandom::RngPtr rng = get_network().get_rng(target_node->get_thread());
  
    Token rt( new librandom::RngDatum(rng) );
    i->OStack.pop(1);
    i->OStack.push_move(rt);
    
    i->EStack.pop();
  }

  /* BeginDocumentation
     Name: GetGlobalRNG - return global random number generator
     Synopsis:
     GetGlobalRNG -> rngtype
     Description: 
     This function returns the global random number generator which
     can be used in situations where the same sequence of random
     numbers is needed in all MPI processes. The user must EXERT
     EXTREME CARE to ensure that all MPI processes use exactly
     the same random numbers while executing a script. NEST performs
     only a simple test upon each call to Simulate to check if the
     global RNGs on all MPI processes are still in sync.

     Examples:
     The RandomDivergentConnect function makes use of numbers
     from the global RNG.

     References:
     [1] Morrison A, Mehring C, Geisel T, Aertsen A, and Diesmann M (2005)
         Advancing the boundaries of high connectivity network simulation 
         with distributed computing. Neural Computation 17(8):1776-1801
         The article is available at www.nest-initiative.org

     Author: Tobias Potjans, Moritz Helias, Diesmann     
     SeeAlso: GetVpRNG, RandomDivergentConnect
  */
  void NestModule::GetGlobalRngFunction::execute(SLIInterpreter *i) const
  {
    librandom::RngPtr rng = get_network().get_grng();
  
    Token rt( new librandom::RngDatum(rng) );
    i->OStack.push_move(rt);
    
    i->EStack.pop();
  }

  void NestModule::Cvdict_CFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);

    ConnectionDatum conn = getValue<ConnectionDatum>(i->OStack.pick(0));
    DictionaryDatum dict = conn.get_dict();

    i->OStack.pop();
    i->OStack.push(dict);
    i->EStack.pop();
  }

#ifdef HAVE_MUSIC
  /* BeginDocumentation
     Name: SetAcceptableLatency - set the acceptable latency of a MUSIC input port

     Synopsis:
     (spikes_in) 0.5 SetAcceptableLatency -> -

     Parameters: 
     port_name - the name of the MUSIC input port
     latency   - the acceptable latency (ms) to set for the port

     Author: Jochen Martin Eppler
     FirstVersion: April 2009
     Availability: Only when compiled with MUSIC
     SeeAlso: music_event_in_proxy
  */  
  void NestModule::SetAcceptableLatencyFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(2);
     
    std::string port_name = getValue<std::string>(i->OStack.pick(1));
    double latency = getValue<double>(i->OStack.pick(0));

    get_network().set_music_in_port_acceptable_latency(port_name, latency);

    i->OStack.pop(2);
    i->EStack.pop();
  }
#endif


  void NestModule::init(SLIInterpreter *i)
  {
    ConnectionType.settypename("connectiontype");
    ConnectionType.setdefaultaction(SLIInterpreter::datatypefunction);

    // ensure we have a network: it is created outside and registered via register_network()
    assert(net_ != 0);
     
    // set resolution, ensure clock is calibrated to new resolution
    Time::reset_resolution();
    net_->calibrate_clock();
      
    // register interface functions with interpreter
    i->createcommand("ChangeSubnet_a", &changesubnet_afunction);
    i->createcommand("ChangeSubnet_i", &changesubnet_ifunction);
    i->createcommand("CurrentSubnet",  &currentsubnetfunction);
    i->createcommand("GetNodes_i_b",   &getnodes_i_bfunction);
    i->createcommand("GetLeaves_i_b",  &getleaves_i_bfunction);
    i->createcommand("GetChildren_i_b",&getchildren_i_bfunction);

    i->createcommand("GetGID",       &getgidfunction);
    i->createcommand("GetLID",       &getlidfunction);
    i->createcommand("GetAddress",   &getaddressfunction);

    i->createcommand("SetStatus_id", &setstatus_idfunction);
    i->createcommand("SetStatus_CD", &setstatus_CDfunction);

    i->createcommand("GetStatus_i",  &getstatus_ifunction);
    i->createcommand("GetStatus_C",  &getstatus_Cfunction);

    i->createcommand("FindConnections_D", &findconnections_Dfunction);

    i->createcommand("Simulate_d",   &simulatefunction);

    i->createcommand("CopyModel_l_l_D", &copymodel_l_l_Dfunction);
    i->createcommand("SetDefaults_l_D", &setdefaults_l_Dfunction);
    i->createcommand("GetDefaults_l",   &getdefaults_lfunction);
   
    i->createcommand("ResumeSimulation", &resumesimulationfunction);
    i->createcommand("Create_l_i", &create_l_ifunction);
   
    i->createcommand("Connect_i_i_l", &connect_i_i_lfunction);
    i->createcommand("Connect_i_i_d_d_l", &connect_i_i_d_d_lfunction);
    i->createcommand("Connect_i_i_D_l", &connect_i_i_D_lfunction);

    i->createcommand("CompoundConnect_i_i_i_l", &compoundconnect_i_i_i_lfunction);
   
    i->createcommand("DivergentConnect_i_ia_a_a_l", &divergentconnect_i_ia_a_a_lfunction);
    i->createcommand("RandomDivergentConnect_i_i_ia_da_da_b_b_l", &rdivergentconnect_i_i_ia_da_da_b_b_lfunction);
    
    i->createcommand("ConvergentConnect_ia_i_a_a_l", &convergentconnect_ia_i_a_a_lfunction);
    i->createcommand("RandomConvergentConnect_ia_i_i_da_da_b_b_l", &rconvergentconnect_ia_i_i_da_da_b_b_lfunction);
   
    i->createcommand("ResetNetwork",&resetnetworkfunction);
    i->createcommand("ResetKernel",&resetkernelfunction);
   
    i->createcommand("NetworkDimensions_a",&networkdimensions_afunction);
   
    i->createcommand("MemoryInfo", &memoryinfofunction);
   
    i->createcommand("PrintNetwork", &printnetworkfunction);
    
    i->createcommand("Rank", &rankfunction);
    i->createcommand("NumProcesses", &numprocessesfunction);
    i->createcommand("SyncProcesses", &syncprocessesfunction);
    i->createcommand("TimeCommunication_i_i_b", &timecommunication_i_i_bfunction); 
    i->createcommand("MPIProcessorName", &mpiprocessornamefunction);
   
    i->createcommand("GetVpRNG", &getvprngfunction);
    i->createcommand("GetGlobalRNG", &getglobalrngfunction);

    i->createcommand("cvdict_C", &cvdict_Cfunction);

#ifdef HAVE_MUSIC     
    i->createcommand("SetAcceptableLatency", &setacceptablelatency_l_dfunction);
#endif
   
    Token statusd = i->baselookup(Name("statusdict"));
    DictionaryDatum dd=getValue<DictionaryDatum>(statusd);
    dd->insert(Name("kernelname"), new StringDatum("NEST"));
    dd->insert(Name("kernelrevision"), new StringDatum("$Revision: 9902 $"));
    dd->insert(Name("is_mpi"), new BoolDatum(Communicator::get_initialized()));
  }

} // namespace nest
