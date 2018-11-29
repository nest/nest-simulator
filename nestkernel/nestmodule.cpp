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
 */

#include "nestmodule.h"

// C++ includes:
#include <iostream>
#include <sstream>

// Includes from libnestutil:
#include "logging.h"

// Includes from librandom:
#include "random_datums.h"

// Includes from nestkernel:
#include "conn_builder.h"
#include "connection_manager_impl.h"
#include "genericmodel.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest.h"
#include "nest_datums.h"
#include "nest_types.h"
#include "node.h"
#include "nodelist.h"
#include "sp_manager_impl.h"
#include "subnet.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "interpret.h"
#include "sliexceptions.h"
#include "stringdatum.h"
#include "tokenutils.h"

extern int SLIsignalflag;

namespace nest
{
SLIType NestModule::ConnectionType;
SLIType NestModule::GIDCollectionType;

// At the time when NestModule is constructed, the SLI Interpreter
// must already be initialized. NestModule relies on the presence of
// the following SLI datastructures: Name, Dictionary

NestModule::NestModule()
{
}

NestModule::~NestModule()
{
  // The network is deleted outside NestModule, since the
  // dynamicloadermodule also needs it

  ConnectionType.deletetypename();
  GIDCollectionType.deletetypename();
}

// The following concerns the new module:

const std::string
NestModule::name( void ) const
{
  return std::string( "NEST Kernel 2" ); // Return name of the module
}

const std::string
NestModule::commandstring( void ) const
{
  return std::string( "(nest-init) run" );
}


/** @BeginDocumentation
   Name: ChangeSubnet - change the current working subnet.
   Synopsis:
   gid   ChangeSubnet -> -
   Parameters:
   gid - The GID of the new current subnet.
   Description:
   Change the current subnet to the one given as argument. Create
   will place newly created nodes in the current working subnet.
   ChangeSubnet is not allowed for layer subnets used in the
   topology module.

   This function can be used to change the working subnet to a new
   location, similar to the UNIX command cd.

   SeeAlso: CurrentSubnet
*/

void
NestModule::ChangeSubnet_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  index node_gid = getValue< long >( i->OStack.pick( 0 ) );

  change_subnet( node_gid );

  i->OStack.pop();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: CurrentSubnet - return the gid of the current network node.

   Synopsis: CurrentSubnet -> gid
   Description:
   CurrentSubnet returns the gid of the current working subnet in form
   of an integer number
   Availability: NEST
   SeeAlso: ChangeSubnet
   Author: Marc-Oliver Gewaltig
*/
void
NestModule::CurrentSubnetFunction::execute( SLIInterpreter* i ) const
{
  index current = current_subnet();

  i->OStack.push( current );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: SetStatus - sets the value of properties of a node, connection, random
   deviate generator or object

   Synopsis:
   gid   dict SetStatus -> -
   conn  dict SetStatus -> -
   rdev  dict SetStatus -> -
   obj   dict SetStatus -> -

   Description:
   SetStatus changes properties of a node (specified by its gid), a connection
   (specified by a connection object), a random deviate generator (see
   GetStatus_v for more) or an object as used in object-oriented programming in
   SLI (see cvo for more). Properties can be inspected with GetStatus.

   Note that many properties are read-only and cannot be changed.

   Examples:
   /dc_generator Create /dc_gen Set  %Creates a dc_generator, which is a node
   dc_gen GetStatus info %view properties (amplitude is 0)
   dc_gen << /amplitude 1500. >> SetStatus
   dc_gen GetStatus info % amplitude is now 1500

   Author: docu by Sirko Straube

   SeeAlso: ShowStatus, GetStatus, info, modeldict, Set, SetStatus_v,
   SetStatus_dict
*/
void
NestModule::SetStatus_idFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );
  index node_id = getValue< long >( i->OStack.pick( 1 ) );

  // Network::set_status() performs entry access checks for each
  // target and throws UnaccessedDictionaryEntry where necessary
  if ( node_id == 0 )
  {
    set_kernel_status( dict );
  }
  else
  {
    set_node_status( node_id, dict );
  }

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::SetStatus_CDFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );
  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.pick( 1 ) );

  set_connection_status( conn, dict );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::Cva_CFunction::execute( SLIInterpreter* i ) const
{
  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.top() );
  ArrayDatum ad;
  ad.push_back( conn.get_source_gid() );
  ad.push_back( conn.get_target_gid() );
  ad.push_back( conn.get_target_thread() );
  ad.push_back( conn.get_synapse_model_id() );
  ad.push_back( conn.get_port() );
  Token result( ad );
  i->OStack.top().swap( result );
  i->EStack.pop();
}

void
NestModule::SetStatus_aaFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ArrayDatum dict_a = getValue< ArrayDatum >( i->OStack.top() );
  ArrayDatum conn_a = getValue< ArrayDatum >( i->OStack.pick( 1 ) );

  if ( ( dict_a.size() != 1 ) and ( dict_a.size() != conn_a.size() ) )
  {
    throw RangeCheck();
  }
  if ( dict_a.size() == 1 ) // Broadcast
  {
    DictionaryDatum dict = getValue< DictionaryDatum >( dict_a[ 0 ] );
    const size_t n_conns = conn_a.size();
    for ( size_t con = 0; con < n_conns; ++con )
    {
      ConnectionDatum con_id = getValue< ConnectionDatum >( conn_a[ con ] );
      dict->clear_access_flags();
      kernel().connection_manager.set_synapse_status( con_id.get_source_gid(),
        con_id.get_target_gid(),
        con_id.get_target_thread(),
        con_id.get_synapse_model_id(),
        con_id.get_port(),
        dict );

      ALL_ENTRIES_ACCESSED( *dict, "SetStatus", "Unread dictionary entries: " );
    }
  }
  else
  {
    const size_t n_conns = conn_a.size();
    for ( size_t con = 0; con < n_conns; ++con )
    {
      DictionaryDatum dict = getValue< DictionaryDatum >( dict_a[ con ] );
      ConnectionDatum con_id = getValue< ConnectionDatum >( conn_a[ con ] );
      dict->clear_access_flags();
      kernel().connection_manager.set_synapse_status( con_id.get_source_gid(),
        con_id.get_target_gid(),
        con_id.get_target_thread(),
        con_id.get_synapse_model_id(),
        con_id.get_port(),
        dict );

      ALL_ENTRIES_ACCESSED( *dict, "SetStatus", "Unread dictionary entries: " );
    }
  }

  i->OStack.pop( 2 );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: GetStatus - return the property dictionary of a node, connection,
   random deviate generator or object

   Synopsis:
   gid   GetStatus -> dict
   conn  GetStatus -> dict
   rdev  GetStatus -> dict
   obj   GetStatus -> dict

   Description:
   GetStatus returns a dictionary with the status information
   for a node (specified by its gid), a connection (specified by a connection
   object), a random deviate generator (see GetStatus_v for more) or an
   object as used in object-oriented programming in SLI (see cvo for more).

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

   Standard entries for nodes:

   global_id   - local ID of the node
   model       - literal, defining the current node
   frozen      - frozen nodes are not updated
   thread      - the thread the node is allocated on
   vp          - the virtual process a node belongs to

   Note that the standard entries cannot be modified directly.

   Author: Marc-Oliver Gewaltig
   Availability: NEST
   SeeAlso: ShowStatus, info, SetStatus, get, GetStatus_v, GetStatus_dict
*/
void
NestModule::GetStatus_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  index node_id = getValue< long >( i->OStack.pick( 0 ) );
  DictionaryDatum dict;
  if ( node_id == 0 )
  {
    dict = get_kernel_status();
  }
  else
  {
    dict = get_node_status( node_id );
  }

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::GetStatus_CFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.pick( 0 ) );

  long gid = conn.get_source_gid();
  kernel().node_manager.get_node( gid ); // Just to check if the node exists

  DictionaryDatum result_dict =
    kernel().connection_manager.get_synapse_status( conn.get_source_gid(),
      conn.get_target_gid(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port() );

  i->OStack.pop();
  i->OStack.push( result_dict );
  i->EStack.pop();
}

// [intvector1,...,intvector_n]  -> [dict1,.../dict_n]
void
NestModule::GetStatus_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  const ArrayDatum conns = getValue< ArrayDatum >( i->OStack.pick( 0 ) );
  size_t n_results = conns.size();
  ArrayDatum result;
  result.reserve( n_results );
  for ( size_t nt = 0; nt < n_results; ++nt )
  {
    ConnectionDatum con_id = getValue< ConnectionDatum >( conns.get( nt ) );
    DictionaryDatum result_dict =
      kernel().connection_manager.get_synapse_status( con_id.get_source_gid(),
        con_id.get_target_gid(),
        con_id.get_target_thread(),
        con_id.get_synapse_model_id(),
        con_id.get_port() );
    result.push_back( result_dict );
  }

  i->OStack.pop();
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: SetDefaults - Set the default values for a node or synapse model.
  Synopsis: /modelname dict SetDefaults -> -
  SeeAlso: GetDefaults
  Author: Jochen Martin Eppler
  FirstVersion: September 2008
*/
void
NestModule::SetDefaults_l_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const Name name = getValue< Name >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  kernel().model_manager.set_model_defaults( name, params );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: GetDefaults - Return the default values for a node or synapse model.
  Synopsis: /modelname GetDefaults -> dict
  SeeAlso: SetDefaults
  Author: Jochen Martin Eppler
  FirstVersion: September 2008
*/
void
NestModule::GetDefaults_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const Name modelname = getValue< Name >( i->OStack.pick( 0 ) );

  DictionaryDatum dict = get_model_defaults( modelname );

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::GetConnections_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  ArrayDatum array = get_connections( dict );

  i->OStack.pop();
  i->OStack.push( array );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: Simulate - simulate n milliseconds

   Synopsis:
   n(int) Simulate -> -

   Description: Simulate the network for n milliseconds.
   Use resume to continue the simulation after an interrupt.

   SeeAlso: resume, ResumeSimulation, unit_conversion
*/
void
NestModule::SimulateFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const double time = i->OStack.top();

  simulate( time );

  // successful end of simulate
  i->OStack.pop();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: Run - simulate n milliseconds

   Synopsis:
   n(int) Run -> -

   Description: Simulate the network for n milliseconds.
   Call prepare before, and cleanup after.
   t m mul Simulate = Prepare m { t Run } repeat Cleanup

   Note: Run must only be used after Prepare is called, and
   before Cleanup to finalize state (close files, etc).
   Any changes made between Prepare and Cleanup may cause
   undefined behavior and incorrect results.

   SeeAlso: Simulate, resume, unit_conversion, Prepare, Cleanup
*/
void
NestModule::RunFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const double time = i->OStack.top();

  run( time );

  i->OStack.pop();
  i->EStack.pop();
}


/** @BeginDocumentation
   Name: Prepare - prepare the network for a simulation

   Synopsis:
   Prepare -> -

   Description: sets up network calibration before run is called
   any number of times

   Note: Run must only be used after Prepare is called, and
   before Cleanup to finalize state (close files, etc).
   Any changes made between Prepare and Cleanup may cause
   undefined behavior and incorrect results.

   SeeAlso: Run, Cleanup, Simulate
*/
void
NestModule::PrepareFunction::execute( SLIInterpreter* i ) const
{
  prepare();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: Cleanup - cleanup the network after a simulation

   Synopsis:
   Cleanup -> -

   Description: tears down a network after run is called
   any number of times

   Note: Run must only be used after Prepare is called, and
   before Cleanup to finalize state (close files, etc).
   Any changes made between Prepare and Cleanup may cause
   undefined behavior and incorrect results.

   SeeAlso: Run, Prepare, Simulate
*/
void
NestModule::CleanupFunction::execute( SLIInterpreter* i ) const
{
  cleanup();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: CopyModel - copy a model to a new name, set parameters for copy, if
   given
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
void
NestModule::CopyModel_l_l_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  // fetch existing model name from stack
  const Name old_name = getValue< Name >( i->OStack.pick( 2 ) );
  const Name new_name = getValue< Name >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  kernel().model_manager.copy_model( old_name, new_name, params );

  i->OStack.pop( 3 );
  i->EStack.pop();
}

/** @BeginDocumentation
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
void
NestModule::Create_l_iFunction::execute( SLIInterpreter* i ) const
{
  // check for stack load
  i->assert_stack_load( 2 );

  // extract arguments
  const long n_nodes = getValue< long >( i->OStack.pick( 0 ) );
  if ( n_nodes <= 0 )
  {
    throw RangeCheck();
  }

  const std::string modname = getValue< std::string >( i->OStack.pick( 1 ) );

  const long last_node_id = create( modname, n_nodes );

  i->OStack.pop( 2 );
  i->OStack.push( last_node_id );
  i->EStack.pop();
}

void
NestModule::RestoreNodes_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  ArrayDatum node_list = getValue< ArrayDatum >( i->OStack.top() );

  restore_nodes( node_list );

  i->OStack.pop();
  i->EStack.pop();
}

void
NestModule::GetNodes_i_D_b_bFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  const bool return_gids_only = getValue< bool >( i->OStack.pick( 0 ) );
  const bool include_remote = not getValue< bool >( i->OStack.pick( 1 ) );
  const DictionaryDatum params =
    getValue< DictionaryDatum >( i->OStack.pick( 2 ) );
  const index node_id = getValue< long >( i->OStack.pick( 3 ) );

  ArrayDatum result =
    get_nodes( node_id, params, include_remote, return_gids_only );

  i->OStack.pop( 4 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::GetChildren_i_D_bFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const bool include_remote = not getValue< bool >( i->OStack.pick( 0 ) );
  const DictionaryDatum params =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const index node_id = getValue< long >( i->OStack.pick( 2 ) );

  ArrayDatum result = get_children( node_id, params, include_remote );

  i->OStack.pop( 3 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::GetLeaves_i_D_bFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const bool include_remote = not getValue< bool >( i->OStack.pick( 0 ) );
  const DictionaryDatum params =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const index node_id = getValue< long >( i->OStack.pick( 2 ) );

  ArrayDatum result = get_leaves( node_id, params, include_remote );

  i->OStack.pop( 3 );
  i->OStack.push( result );
  i->EStack.pop();
}


/** @BeginDocumentation
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
void
NestModule::ResetKernelFunction::execute( SLIInterpreter* i ) const
{
  reset_kernel();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: ResetNetwork - Reset the dynamic state of the network.
   Synopsis: ResetNetwork -> -
   Description:
   ResetNetwork resets the dynamic state of the entire network to its state
   at T=0. The dynamic state comprises typically the membrane potential,
   synaptic currents, buffers holding input that has been delivered, but not
   yet become effective, and all events pending delivery. Technically, this
   is achieved by calling init_state() on all nodes and forcing a call to
   init_buffers() upon the next call to Simulate. Node parameters, such as
   time constants and threshold potentials, are not affected.

   Remarks:
   - Time and random number generators are NOT reset.
   - Files belonging to recording devices (spike detector, multimeter,
     voltmeter, etc) are closed. You must change the file name before
     simulating again, otherwise the files will be overwritten and you
     will receive an error, depending on the value of /overwrite_files
     (in the root node).
   - ResetNetwork will reset the nodes to the state values stored in the model
     prototypes. So if you have used SetDefaults to change a state value of a
     model since you called Simulate the first time, the network will NOT be
     reset to the status at T=0.
   - The dynamic state of synapses with internal dynamics (STDP, facilitation)
     is NOT reset at present. This will be implemented in a future version
     of NEST.

   SeeAlso: ResetKernel, reset
*/
void
NestModule::ResetNetworkFunction::execute( SLIInterpreter* i ) const
{
  reset_network();

  i->EStack.pop();
}

// Disconnect for gid gid syn_model
// See lib/sli/nest-init.sli for details
void
NestModule::Disconnect_i_i_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  index source = getValue< long >( i->OStack.pick( 2 ) );
  index target = getValue< long >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // check whether the target is on this process
  if ( kernel().node_manager.is_local_gid( target ) )
  {
    Node* const target_node = kernel().node_manager.get_node( target );
    const thread target_thread = target_node->get_thread();
    kernel().sp_manager.disconnect_single(
      source, target_node, target_thread, synapse_params );
  }

  i->OStack.pop( 3 );
  i->EStack.pop();
}

// Disconnect for gidcollection gidcollection conn_spec syn_spec
void
NestModule::Disconnect_g_g_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  GIDCollectionDatum sources =
    getValue< GIDCollectionDatum >( i->OStack.pick( 3 ) );
  GIDCollectionDatum targets =
    getValue< GIDCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by disconnect
  kernel().sp_manager.disconnect(
    sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();
}

// Connect for gidcollection gidcollection conn_spec syn_spec
// See lib/sli/nest-init.sli for details
void
NestModule::Connect_g_g_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  GIDCollectionDatum sources =
    getValue< GIDCollectionDatum >( i->OStack.pick( 3 ) );
  GIDCollectionDatum targets =
    getValue< GIDCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by connect
  kernel().connection_manager.connect(
    sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: DataConnect_i_D_s - Connect many neurons from data.

   Synopsis:
   gid dict model  DataConnect_i_D_s -> -

   gid    - GID of the source neuron
   dict   - dictionary with connection parameters
   model  - the synapse model as string or literal

   Description:
   Connects the source neuron to targets according to the data in dict, using
   the synapse 'model'.

   Dict is a parameter dictionary that must contain the connection parameters as
   DoubleVectors.
   The parameter dictionary must contain at least the fields:
   /target <. gid_1 ... gid_n .>
   /weight <. w1_1 ... w_n .>
   /delay  <. d_1 ... d_n .>
   All of these must be DoubleVectors of the same length.

   Depending on the synapse model, the dictionary may contain other keys, again
   as DoubleVectors of the same length as /target.

   DataConnect will iterate all vectors and create the connections according to
   the parameters given.
   SeeAlso: DataConnect_a, DataConnect
   Author: Marc-Oliver Gewaltig
   FirstVersion: August 2011
   SeeAlso: Connect
*/
void
NestModule::DataConnect_i_D_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  if ( kernel().vp_manager.get_num_threads() > 1 )
  {
    throw KernelException( "DataConnect cannot be used with multiple threads" );
  }

  const index source = getValue< long >( i->OStack.pick( 2 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const Name synmodel_name = getValue< std::string >( i->OStack.pick( 0 ) );

  const Token synmodel =
    kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( synmodel_name.toString() );
  }
  const index synmodel_id = static_cast< index >( synmodel );

  kernel().connection_manager.data_connect_single(
    source, params, synmodel_id );

  ALL_ENTRIES_ACCESSED(
    *params, "Connect", "The following synapse parameters are unused: " );

  i->OStack.pop( 3 );
  i->EStack.pop();
}

/** @BeginDocumentation
    Name: DataConnect_a - Connect many neurons from a list of synapse status
   dictionaries.

    Synopsis:
    [dict1, dict2, ..., dict_n ]  DataConnect_a -> -

    This variant of DataConnect can be used to re-instantiate a given
   connectivity matrix.
    The argument is a list of dictionaries, each containing at least the keys
    /source
    /target
    /weight
    /delay
    /synapse_model

    Example:

    % assume a connected network

    << >> GetConnections Flatten /conns Set % Get all connections
    conns { GetStatus } Map      /syns  Set % retrieve their synapse status

    ResetKernel                             % clear everything
    % rebuild neurons
    syns DataConnect                        % restore the connecions


    Author: Marc-Oliver Gewaltig
    FirstVersion: May 2012
    SeeAlso: DataConnect_i_D_s, Connect
 */
void
NestModule::DataConnect_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  if ( kernel().vp_manager.get_num_threads() > 1 )
  {
    throw KernelException( "DataConnect cannot be used with multiple threads" );
  }

  const ArrayDatum connectome = getValue< ArrayDatum >( i->OStack.top() );

  kernel().connection_manager.data_connect_connectome( connectome );
  i->OStack.pop();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: MemoryInfo - Report current memory usage.
   Description:
   MemoryInfo reports the current utilization of the memory manager for all
   models, which are used at least once. The output is sorted ascending
   according according to the name of the model is written to stdout. The unit
   of the data is byte. Note that MemoryInfo only gives you information about
   the memory requirements of the static model data inside of NEST. It does not
   tell anything about the memory situation on your computer.
   Synopsis:
   MemoryInfo -> -
   Availability: NEST
   Author: Jochen Martin Eppler
*/
void
NestModule::MemoryInfoFunction::execute( SLIInterpreter* i ) const
{
  kernel().model_manager.memory_info();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: PrintNetwork - Print network tree in readable form.
   Synopsis:
   gid depth  PrintNetwork -> -
   Parameters:
   gid        - Global ID of the subnet to start tree printout.
   depth      - Integer, specifies down to which level the network is printed.
   Description:
   This function prints the network structure in a concise tree-like format
   according to the following rules:
   - Each Node is shown on a separate line, showing its model name followed
   by its in global id in brackets.

   +-[0] subnet Dim=[1]
   |
   +- iaf_psc_alpha [1]

   - Consecutive Nodes of the same model are summarised in a list.
   The list shows the model name, the global id of the first node in the
   sequence, then the number of consecutive nodes, then the global id of
   the last node in the sequence.

   +-[0] subnet Dim=[1]
   |
   +- iaf_psc_alpha [1]..(2)..[2]

   - If a node is a subnet, its global id is printed first, followed by the
   model name or its label (if it is defined). Next, the dimension is shown.
   If the current recursion level is less than the specified depth, the printout
   descends to the children of the subnet.
   After the header, a new line is printed, followed by the list of children
   at the next indentation level.
   After the last child, a new line is printed and the printout of the parent
   subnet is continued.

   Example:
   SLI ] /iaf_psc_alpha Create
   SLI [1] /iaf_cond_alpha 10 Create
   SLI [2] /dc_generator [2 5 6] LayoutNetwork
   SLI [3] 0 1 PrintNetwork
   +-[0] root dim=[12]
   SLI [3] 0 2 PrintNetwork
   +-[0] root dim=[12]
      |
      +-[1] iaf_psc_alpha
      +-[2]...[11] iaf_cond_alpha
      +-[12] subnet dim=[2 5 6]
   SLI [3] 0 3 PrintNetwork
   +-[0] root dim=[12]
      |
      +-[1] iaf_psc_alpha
      +-[2]...[11] iaf_cond_alpha
      +-[12] subnet dim=[2 5 6]
         |
         +-[1] subnet dim=[5 6]
         +-[2] subnet dim=[5 6]
   SLI [3] 0 4 PrintNetwork
   +-[0] root dim=[12]
      |
      +-[1] iaf_psc_alpha
      +-[2]...[11] iaf_cond_alpha
      +-[12] subnet dim=[2 5 6]
         |
         +-[1] subnet dim=[5 6]
         |  |
         |  +-[1] subnet dim=[6]
         |  +-[2] subnet dim=[6]
         |  +-[3] subnet dim=[6]
         |  +-[4] subnet dim=[6]
         |  +-[5] subnet dim=[6]
         +-[2] subnet dim=[5 6]
            |
            +-[1] subnet dim=[6]
            +-[2] subnet dim=[6]
            +-[3] subnet dim=[6]
            +-[4] subnet dim=[6]
            +-[5] subnet dim=[6]
   SLI [3] 0 5 PrintNetwork
   +-[0] root dim=[12]
      |
      +-[1] iaf_psc_alpha
      +-[2]...[11] iaf_cond_alpha
      +-[12] subnet dim=[2 5 6]
         |
         +-[1] subnet dim=[5 6]
         |  |
         |  +-[1] subnet dim=[6]
         |  |  |
         |  |  +-[1]...[6] dc_generator
         |  |
         |  +-[2] subnet dim=[6]
         |  |  |
         |  |  +-[1]...[6] dc_generator
         |  |
         |  +-[3] subnet dim=[6]
         |  |  |
         |  |  +-[1]...[6] dc_generator
         |  |
         |  +-[4] subnet dim=[6]
         |  |  |
         |  |  +-[1]...[6] dc_generator
         |  |
         |  +-[5] subnet dim=[6]
         |     |
         |     +-[1]...[6] dc_generator
         |
         +-[2] subnet dim=[5 6]
            |
            +-[1] subnet dim=[6]
            |  |
            |  +-[1]...[6] dc_generator
            |
            +-[2] subnet dim=[6]
            |  |
            |  +-[1]...[6] dc_generator
            |
            +-[3] subnet dim=[6]
            |  |
            |  +-[1]...[6] dc_generator
            |
            +-[4] subnet dim=[6]
            |  |
            |  +-[1]...[6] dc_generator
            |
            +-[5] subnet dim=[6]
               |
               +-[1]...[6] dc_generator

   Availability: NEST
   Author: Marc-Oliver Gewaltig, Jochen Martin Eppler
*/
void
NestModule::PrintNetworkFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  long gid = getValue< long >( i->OStack.pick( 1 ) );
  long depth = getValue< long >( i->OStack.pick( 0 ) );

  print_network( gid, depth - 1 );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: Rank - Return the MPI rank of the process.
   Synopsis: Rank -> int
   Description:
   Returns the rank of the MPI process (MPI_Comm_rank) executing the
   command. This function is mainly meant for logging and debugging
   purposes. It is highly discouraged to use this function to write
   rank-dependent code in a simulation script as this can break NEST
   in funny ways, of which dead-locks are the nicest.
   Availability: NEST 2.0
   Author: Jochen Martin Eppler
   FirstVersion: January 2006
   SeeAlso: NumProcesses, SyncProcesses, ProcessorName
*/
void
NestModule::RankFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel().mpi_manager.get_rank() );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: NumProcesses - Return the number of MPI processes.
   Synopsis: NumProcesses -> int
   Description:
   Returns the number of MPI processes (MPI_Comm_size). This
   function is mainly meant for logging and debugging purposes.
   Availability: NEST 2.0
   Author: Jochen Martin Eppler
   FirstVersion: January 2006
   SeeAlso: Rank, SyncProcesses, ProcessorName
*/
void
NestModule::NumProcessesFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel().mpi_manager.get_num_processes() );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: SetFakeNumProcesses - Set a fake number of MPI processes.
   Synopsis: n_procs SetFakeNumProcesses -> -
   Description:
   Sets the number of MPI processes to n_procs. Used for benchmarking purposes
   of memory consumption only.
   Please note:
   - Simulation of the network will not be possible after setting fake
     processes.
   - It is not possible to use this function when running a script on multiple
     actual MPI processes.
   - The setting of the fake number of processes has to happen before the kernel
     reset and before the setting of the local number of threads.
     After calling SetFakeNumProcesses, it is obligatory to call either
     ResetKernel or SetStatus on the Kernel for the setting of the fake
     number of processes to come into effect.

   A typical use case would be to test if a neuronal network fits on a machine
   of given size without using the actual resources.

   Example:
             %%% Set fake number of processes
             100 SetFakeNumProcesses
             ResetNetwork

             %%% Build network
             /iaf_psc_alpha 100 Create
             [1 100] Range /n Set

             << /source n /target n >> Connect

             %%% Measure memory consumption
             memory_thisjob ==

       Execute this script with
             mpirun -np 1 nest example.sli

   Availability: NEST 2.2
   Author: Susanne Kunkel
   FirstVersion: July 2011
   SeeAlso: NumProcesses
*/
void
NestModule::SetFakeNumProcesses_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  long n_procs = getValue< long >( i->OStack.pick( 0 ) );

  enable_dryrun_mode( n_procs );

  i->OStack.pop( 1 );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: SyncProcesses - Synchronize all MPI processes.
   Synopsis: SyncProcesses -> -
   Availability: NEST 2.0
   Author: Alexander Hanuschkin
   FirstVersion: April 2009
   Description:
   This function allows to synchronize all MPI processes at any
   point in a simulation script. Internally, the function uses
   MPI_Barrier(). Note that during simulation the processes are
   automatically synchronized without the need for user interaction.
   SeeAlso: Rank, NumProcesses, ProcessorName
*/
void
NestModule::SyncProcessesFunction::execute( SLIInterpreter* i ) const
{
  kernel().mpi_manager.synchronize();
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: TimeCommunication - returns average time taken for MPI_Allgather over n
   calls with m bytes
   Synopsis:
   n m TimeCommunication -> time
   Availability: NEST 2.0
   Author: Abigail Morrison
   FirstVersion: August 2009
   Description:
   The function allows a user to test how much time a call the Allgather costs
*/
void
NestModule::TimeCommunication_i_i_bFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );
  long samples = getValue< long >( i->OStack.pick( 2 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 1 ) );
  bool offgrid = getValue< bool >( i->OStack.pick( 0 ) );

  double time = 0.0;
  if ( offgrid )
  {
    time = kernel().mpi_manager.time_communicate_offgrid( num_bytes, samples );
  }
  else
  {
    time = kernel().mpi_manager.time_communicate( num_bytes, samples );
  }

  i->OStack.pop( 3 );
  i->OStack.push( time );
  i->EStack.pop();
}
/** @BeginDocumentation
   Name: TimeCommunicationv - returns average time taken for MPI_Allgatherv over
   n calls with m
   bytes
   Synopsis:
   n m TimeCommunication -> time
   Availability: NEST 2.0
   Author:
   FirstVersion: August 2012
   Description:
   The function allows a user to test how much time a call the Allgatherv costs
   Does not work for offgrid!!!
*/
void
NestModule::TimeCommunicationv_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel().mpi_manager.time_communicatev( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: TimeCommunicationAlltoall - returns average time taken for MPI_Alltoall
   over n calls with m
   bytes
   Synopsis:
   n m TimeCommunicationAlltoall -> time
   Availability: 10kproject (>r11254)
   Author: Jakob Jordan
   FirstVersion: June 2014
   Description:
   The function allows a user to test how much time a call to MPI_Alltoall costs
   SeeAlso: TimeCommunication
 */
void
NestModule::TimeCommunicationAlltoall_i_iFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel().mpi_manager.time_communicate_alltoall( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: TimeCommunicationAlltoallv - returns average time taken for
   MPI_Alltoallv over n calls with
   m bytes
   Synopsis:
   n m TimeCommunicationAlltoallv -> time
   Availability: 10kproject (>r11300)
   Author: Jakob Jordan
   FirstVersion: July 2014
   Description:
   The function allows a user to test how much time a call to MPI_Alltoallv
   costs
   SeeAlso: TimeCommunication
 */
void
NestModule::TimeCommunicationAlltoallv_i_iFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel().mpi_manager.time_communicate_alltoallv( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: ProcessorName - Returns a unique specifier for the actual node.
   Synopsis: ProcessorName -> string
   Availability: NEST 2.0
   Author: Alexander Hanuschkin
   FirstVersion: April 2009
   Description:
   This function returns the name of the processor it was called
   on (MPI_Get_processor_name). See MPI documentation for more details. If NEST
   is not compiled with MPI support, this function returns the hostname of
   the machine as returned by the POSIX function gethostname().
   Examples:
   (I'm process ) =only Rank 1 add =only ( of ) =only NumProcesses =only ( on
   machine ) =only
   ProcessorName =
   SeeAlso: Rank, NumProcesses, SyncProcesses
*/
void
NestModule::ProcessorNameFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel().mpi_manager.get_processor_name() );
  i->EStack.pop();
}

#ifdef HAVE_MPI
/** @BeginDocumentation
   Name: abort - Abort all NEST processes gracefully.
   Paramteres:
   exitcode - The exitcode to quit with
   Description:
   This function can be run by the user to end all NEST processes as
   gracefully as possible. If NEST is compiled without MPI support,
   this will just call quit_i. If compiled with MPI support, it will
   call MPI_Abort, which will kill all processes of the application
   and thus prevents deadlocks. The exitcode is userabort in both
   cases (see statusdict/exitcodes).
   Availability: NEST 2.0
   Author: Jochen Martin Eppler
   FirstVersion: October 2012
   SeeAlso: quit, Rank, SyncProcesses, ProcessorName
*/
void
NestModule::MPIAbort_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  long exitcode = getValue< long >( i->OStack.pick( 0 ) );
  kernel().mpi_manager.mpi_abort( exitcode );
  i->EStack.pop();
}
#endif

/** @BeginDocumentation
   Name: GetVpRNG - return random number generator associated to virtual process
   of node
   Synopsis:
   gid GetVpRNG -> rngtype
   Parameters:
   gid  - global id of the node
   Description:
   This function is helpful in the implementation of parallelized wiring
   routines that create identical random structures independent of the
   number of machines and threads participating in the simulation. The
   function is used in SLI libraries. There is probably no need to
   directly use GetVpRNG in scripts describing a particular simulation.

   In NEST each node (e.g. neuron) is assigned to a virtual process and
   each virtual process maintains its own random number generator. In a
   simulation run the virtual processes are equally distributed over the
   participating machines and threads as specified by the user. In NEST
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

   References:
   [1] Morrison A, Mehring C, Geisel T, Aertsen A, and Diesmann M (2005)
       Advancing the boundaries of high connectivity network simulation
       with distributed computing. Neural Computation 17(8):1776-1801
       The article is available at www.nest-simulator.org

   Author: Tobias Potjans, Moritz Helias, Diesmann
   SeeAlso: GetGlobalRNG
*/
void
NestModule::GetVpRngFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  index target = getValue< long >( i->OStack.pick( 0 ) );

  librandom::RngPtr rng = get_vp_rng_of_gid( target );

  Token rt( new librandom::RngDatum( rng ) );
  i->OStack.pop( 1 );
  i->OStack.push_move( rt );

  i->EStack.pop();
}

/** @BeginDocumentation
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

   References:
   [1] Morrison A, Mehring C, Geisel T, Aertsen A, and Diesmann M (2005)
       Advancing the boundaries of high connectivity network simulation
       with distributed computing. Neural Computation 17(8):1776-1801
       The article is available at www.nest-simulator.org

   Author: Tobias Potjans, Moritz Helias, Diesmann
   SeeAlso: GetVpRNG
*/
void
NestModule::GetGlobalRngFunction::execute( SLIInterpreter* i ) const
{
  librandom::RngPtr rng = get_global_rng();

  Token rt( new librandom::RngDatum( rng ) );
  i->OStack.push_move( rt );

  i->EStack.pop();
}

void
NestModule::Cvdict_CFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.pick( 0 ) );
  DictionaryDatum dict = conn.get_dict();

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::Cvgidcollection_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const long first = getValue< long >( i->OStack.pick( 1 ) );
  const long last = getValue< long >( i->OStack.pick( 0 ) );
  GIDCollectionDatum gidcoll = GIDCollection( first, last );

  i->OStack.pop( 2 );
  i->OStack.push( gidcoll );
  i->EStack.pop();
}

void
NestModule::Cvgidcollection_iaFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  TokenArray gids = getValue< TokenArray >( i->OStack.pick( 0 ) );
  GIDCollectionDatum gidcoll = GIDCollection( gids );

  i->OStack.pop();
  i->OStack.push( gidcoll );
  i->EStack.pop();
}

void
NestModule::Cvgidcollection_ivFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  IntVectorDatum gids = getValue< IntVectorDatum >( i->OStack.pick( 0 ) );
  GIDCollectionDatum gidcoll = GIDCollection( gids );

  i->OStack.pop();
  i->OStack.push( gidcoll );
  i->EStack.pop();
}

void
NestModule::Size_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  GIDCollectionDatum gidcoll =
    getValue< GIDCollectionDatum >( i->OStack.pick( 0 ) );

  i->OStack.pop();
  i->OStack.push( gidcoll.size() );
  i->EStack.pop();
}

#ifdef HAVE_MUSIC
/** @BeginDocumentation
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
void
NestModule::SetAcceptableLatencyFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::string port_name = getValue< std::string >( i->OStack.pick( 1 ) );
  double latency = getValue< double >( i->OStack.pick( 0 ) );

  kernel().music_manager.set_music_in_port_acceptable_latency(
    port_name, latency );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::SetMaxBufferedFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::string port_name = getValue< std::string >( i->OStack.pick( 1 ) );
  int maxBuffered = getValue< long >( i->OStack.pick( 0 ) );

  kernel().music_manager.set_music_in_port_max_buffered(
    port_name, maxBuffered );

  i->OStack.pop( 2 );
  i->EStack.pop();
}
#endif

/** @BeginDocumentation
   Name: SetStructuralPlasticityStatus - Set up parameters for structural
   plasticity.

   Synopsis:
   Structural plasticity allows the user to treat the nodes as neurons with
   synaptic elements, allowing new synapses to be created and existing synapses
   to be deleted during the simulation according to a set of growth and
   homeostatic rules. This function allows the user to set up various
   parameters for structural plasticity.

   Parameters:
   structural_plasticity_dictionary - is a dictionary which states the settings
   for the structural plasticity functionality

   Author: Mikael Naveau, Sandra Diaz
   FirstVersion: December 2014
*/
void
NestModule::SetStructuralPlasticityStatus_DFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  DictionaryDatum structural_plasticity_dictionary =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  kernel().sp_manager.set_status( structural_plasticity_dictionary );

  i->OStack.pop( 1 );
  i->EStack.pop();
}

void
NestModule::GetStructuralPlasticityStatus_DFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  DictionaryDatum current_status =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );
  kernel().sp_manager.get_status( current_status );

  i->OStack.pop( 1 );
  i->OStack.push( current_status );
  i->EStack.pop();
}

/**
 * Enable Structural Plasticity within the simulation. This means, allowing
 * dynamic rewiring of the network based on mean electrical activity.
 * @param i
 */
void
NestModule::EnableStructuralPlasticity_Function::execute(
  SLIInterpreter* i ) const
{
  kernel().sp_manager.enable_structural_plasticity();

  i->EStack.pop();
}
/**
 * Disable Structural Plasticity in the network.
 * @param i
 */
void
NestModule::DisableStructuralPlasticity_Function::execute(
  SLIInterpreter* i ) const
{
  kernel().sp_manager.disable_structural_plasticity();

  i->EStack.pop();
}

/**
 * Set epsilon that is used for comparing spike times in STDP.
 * Spike times in STDP synapses are currently represented as double
 * values. The epsilon defines the maximum distance between spike
 * times that is still considered 0.
 *
 * Note: See issue #894
 */
void
NestModule::SetStdpEps_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  const double stdp_eps = getValue< double >( i->OStack.top() );

  kernel().connection_manager.set_stdp_eps( stdp_eps );

  i->OStack.pop();
  i->EStack.pop();
}

void
NestModule::init( SLIInterpreter* i )
{
  ConnectionType.settypename( "connectiontype" );
  ConnectionType.setdefaultaction( SLIInterpreter::datatypefunction );

  GIDCollectionType.settypename( "gidcollectiontype" );
  GIDCollectionType.setdefaultaction( SLIInterpreter::datatypefunction );

  // register interface functions with interpreter
  i->createcommand( "ChangeSubnet", &changesubnet_ifunction, "NEST 3.0" );
  i->createcommand( "CurrentSubnet", &currentsubnetfunction, "NEST 3.0" );
  i->createcommand( "GetNodes_i_D_b_b", &getnodes_i_D_b_bfunction, "NEST 3.0" );
  i->createcommand( "GetLeaves_i_D_b", &getleaves_i_D_bfunction, "NEST 3.0" );
  i->createcommand(
    "GetChildren_i_D_b", &getchildren_i_D_bfunction, "NEST 3.0" );

  i->createcommand( "RestoreNodes_a", &restorenodes_afunction );

  i->createcommand( "SetStatus_id", &setstatus_idfunction );
  i->createcommand( "SetStatus_CD", &setstatus_CDfunction );
  i->createcommand( "SetStatus_aa", &setstatus_aafunction );

  i->createcommand( "GetStatus_i", &getstatus_ifunction );
  i->createcommand( "GetStatus_C", &getstatus_Cfunction );
  i->createcommand( "GetStatus_a", &getstatus_afunction );

  i->createcommand( "GetConnections_D", &getconnections_Dfunction );
  i->createcommand( "cva_C", &cva_cfunction );

  i->createcommand( "Simulate_d", &simulatefunction );
  i->createcommand( "Run_d", &runfunction );
  i->createcommand( "Prepare", &preparefunction );
  i->createcommand( "Cleanup", &cleanupfunction );

  i->createcommand( "CopyModel_l_l_D", &copymodel_l_l_Dfunction );
  i->createcommand( "SetDefaults_l_D", &setdefaults_l_Dfunction );
  i->createcommand( "GetDefaults_l", &getdefaults_lfunction );

  i->createcommand( "Create_l_i", &create_l_ifunction );

  i->createcommand( "Connect_g_g_D_D", &connect_g_g_D_Dfunction );

  i->createcommand(
    "DataConnect_i_D_s", &dataconnect_i_D_sfunction, "NEST 3.0" );
  i->createcommand( "DataConnect_a", &dataconnect_afunction, "NEST 3.0" );

  i->createcommand( "ResetNetwork", &resetnetworkfunction );
  i->createcommand( "ResetKernel", &resetkernelfunction );

  i->createcommand( "MemoryInfo", &memoryinfofunction );

  i->createcommand( "PrintNetwork", &printnetworkfunction );

  i->createcommand( "Rank", &rankfunction );
  i->createcommand( "NumProcesses", &numprocessesfunction );
  i->createcommand( "SetFakeNumProcesses", &setfakenumprocesses_ifunction );
  i->createcommand( "SyncProcesses", &syncprocessesfunction );
  i->createcommand(
    "TimeCommunication_i_i_b", &timecommunication_i_i_bfunction );
  i->createcommand( "TimeCommunicationv_i_i", &timecommunicationv_i_ifunction );
  i->createcommand(
    "TimeCommunicationAlltoall_i_i", &timecommunicationalltoall_i_ifunction );
  i->createcommand(
    "TimeCommunicationAlltoallv_i_i", &timecommunicationalltoallv_i_ifunction );
  i->createcommand( "ProcessorName", &processornamefunction );
#ifdef HAVE_MPI
  i->createcommand( "MPI_Abort", &mpiabort_ifunction );
#endif

  i->createcommand( "GetVpRNG", &getvprngfunction );
  i->createcommand( "GetGlobalRNG", &getglobalrngfunction );

  i->createcommand( "cvdict_C", &cvdict_Cfunction );

  i->createcommand( "cvgidcollection_i_i", &cvgidcollection_i_ifunction );
  i->createcommand( "cvgidcollection_ia", &cvgidcollection_iafunction );
  i->createcommand( "cvgidcollection_iv", &cvgidcollection_ivfunction );
  i->createcommand( "size_g", &size_gfunction );

#ifdef HAVE_MUSIC
  i->createcommand( "SetAcceptableLatency", &setacceptablelatency_l_dfunction );
  i->createcommand( "SetMaxBuffered", &setmaxbuffered_l_ifunction );
#endif
  i->createcommand(
    "EnableStructuralPlasticity", &enablestructuralplasticity_function );
  i->createcommand(
    "DisableStructuralPlasticity", &disablestructuralplasticity_function );
  i->createcommand(
    "SetStructuralPlasticityStatus", &setstructuralplasticitystatus_Dfunction );
  i->createcommand(
    "GetStructuralPlasticityStatus", &getstructuralplasticitystatus_function );
  i->createcommand( "Disconnect", &disconnect_i_i_lfunction );
  i->createcommand( "Disconnect_g_g_D_D", &disconnect_g_g_D_Dfunction );

  i->createcommand( "SetStdpEps", &setstdpeps_dfunction );

  // Add connection rules
  kernel().connection_manager.register_conn_builder< OneToOneBuilder >(
    "one_to_one" );
  kernel().connection_manager.register_conn_builder< AllToAllBuilder >(
    "all_to_all" );
  kernel().connection_manager.register_conn_builder< FixedInDegreeBuilder >(
    "fixed_indegree" );
  kernel().connection_manager.register_conn_builder< FixedOutDegreeBuilder >(
    "fixed_outdegree" );
  kernel().connection_manager.register_conn_builder< BernoulliBuilder >(
    "pairwise_bernoulli" );
  kernel()
    .connection_manager.register_conn_builder< SymmetricBernoulliBuilder >(
      "symmetric_pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< FixedTotalNumberBuilder >(
    "fixed_total_number" );

  // Add MSP growth curves
  kernel().sp_manager.register_growth_curve< GrowthCurveSigmoid >( "sigmoid" );
  kernel().sp_manager.register_growth_curve< GrowthCurveGaussian >(
    "gaussian" );
  kernel().sp_manager.register_growth_curve< GrowthCurveLinear >( "linear" );

  Token statusd = i->baselookup( Name( "statusdict" ) );
  DictionaryDatum dd = getValue< DictionaryDatum >( statusd );
  dd->insert( Name( "kernelname" ), new StringDatum( "NEST" ) );
  dd->insert(
    Name( "is_mpi" ), new BoolDatum( kernel().mpi_manager.is_mpi_used() ) );
}

} // namespace nest
