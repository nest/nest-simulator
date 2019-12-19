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
#include "sp_manager_impl.h"

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
SLIType NestModule::NodeCollectionType;
SLIType NestModule::NodeCollectionIteratorType;
SLIType NestModule::ParameterType;

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
  NodeCollectionType.deletetypename();
  NodeCollectionIteratorType.deletetypename();
  ParameterType.deletetypename();
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

ParameterDatum
NestModule::create_parameter( const Token& t )
{
  // t can be an existing ParameterDatum, a DoubleDatum containing a
  // constant value for this parameter, or a Dictionary containing
  // parameters
  ParameterDatum* pd = dynamic_cast< ParameterDatum* >( t.datum() );
  if ( pd )
  {
    return *pd;
  }

  // If t is a DoubleDatum, create a ConstantParameter with this value
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  if ( dd )
  {
    return new ConstantParameter( *dd );
  }

  DictionaryDatum* dictd = dynamic_cast< DictionaryDatum* >( t.datum() );
  if ( dictd )
  {

    // The dictionary should only have a single key, which is the name of
    // the parameter type to create.
    if ( ( *dictd )->size() != 1 )
    {
      throw BadProperty( "Parameter definition dictionary must contain one single key only." );
    }

    Name n = ( *dictd )->begin()->first;
    DictionaryDatum pdict = getValue< DictionaryDatum >( *dictd, n );
    return create_parameter( n, pdict );
  }
  else
  {
    throw BadProperty( "Parameter must be parametertype, constant or dictionary." );
  }
}

Parameter*
NestModule::create_parameter( const Name& name, const DictionaryDatum& d )
{
  // The parameter factory will create the parameter
  Parameter* param = parameter_factory_().create( name, d );

  return param;
}


GenericFactory< Parameter >&
NestModule::parameter_factory_( void )
{
  static GenericFactory< Parameter > factory;
  return factory;
}

/** @BeginDocumentation
   Name: SetStatus - sets the value of properties of a node, connection, random
   deviate generator or object

   Synopsis:
   node_id   dict SetStatus -> -
   conn  dict SetStatus -> -
   rdev  dict SetStatus -> -
   obj   dict SetStatus -> -

   Description:
   SetStatus changes properties of a node (specified by its node_id), a connection
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

   SeeAlso: ShowStatus, GetStatus, GetKernelStatus, info, modeldict, Set,
   SetStatus_v, SetStatus_dict
*/
void
NestModule::SetStatus_idFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );
  index node_id = getValue< long >( i->OStack.pick( 1 ) );

  set_node_status( node_id, dict );

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
NestModule::SetKernelStatus_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );

  set_kernel_status( dict );

  i->OStack.pop();
  i->EStack.pop();
}

void
NestModule::Cva_CFunction::execute( SLIInterpreter* i ) const
{
  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.top() );
  ArrayDatum ad;
  ad.push_back( conn.get_source_node_id() );
  ad.push_back( conn.get_target_node_id() );
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
      kernel().connection_manager.set_synapse_status( con_id.get_source_node_id(),
        con_id.get_target_node_id(),
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
      kernel().connection_manager.set_synapse_status( con_id.get_source_node_id(),
        con_id.get_target_node_id(),
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
   node_id   GetStatus -> dict
   conn  GetStatus -> dict
   rdev  GetStatus -> dict
   obj   GetStatus -> dict

   Description:
   GetStatus returns a dictionary with the status information
   for a node (specified by its node_id), a connection (specified by a connection
   object), a random deviate generator (see GetStatus_v for more) or an
   object as used in object-oriented programming in SLI (see cvo for more).

   The interpreter exchanges data with the network element using
   its status dictionary. To abbreviate the access pattern
        node_id GetStatus /lit get
   a variant of get implicitly calls GetStatus
        node_id /lit get .
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
   SeeAlso: ShowStatus, info, SetStatus, get, GetStatus_v, GetStatus_dict,
   GetKernelStatus
*/
void
NestModule::GetStatus_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  NodeCollectionDatum nc = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  if ( not nc->valid() )
  {
    throw KernelException( "InvalidNodeCollection" );
  }

  size_t nc_size = nc->size();
  ArrayDatum result;

  result.reserve( nc_size );

  for ( NodeCollection::const_iterator it = nc->begin(); it < nc->end(); ++it )
  {
    index node_id = ( *it ).node_id;
    DictionaryDatum dict = get_node_status( node_id );
    result.push_back( dict );
  }

  i->OStack.pop();
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::GetStatus_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  index node_id = getValue< long >( i->OStack.pick( 0 ) );
  DictionaryDatum dict = get_node_status( node_id );

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::GetStatus_CFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ConnectionDatum conn = getValue< ConnectionDatum >( i->OStack.pick( 0 ) );

  DictionaryDatum result_dict = kernel().connection_manager.get_synapse_status( conn.get_source_node_id(),
    conn.get_target_node_id(),
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
    DictionaryDatum result_dict = kernel().connection_manager.get_synapse_status( con_id.get_source_node_id(),
      con_id.get_target_node_id(),
      con_id.get_target_thread(),
      con_id.get_synapse_model_id(),
      con_id.get_port() );
    result.push_back( result_dict );
  }

  i->OStack.pop();
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::GetMetadata_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  NodeCollectionDatum nc = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  if ( not nc->valid() )
  {
    throw KernelException( "InvalidNodeCollection" );
  }

  NodeCollectionMetadataPTR meta = nc->get_metadata();
  DictionaryDatum dict = DictionaryDatum( new Dictionary );

  // return empty dict if NC does not have metadata
  if ( meta.get() )
  {
    meta->get_status( dict );

    ( *dict )[ names::network_size ] = nc->size();
  }

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::GetKernelStatus_Function::execute( SLIInterpreter* i ) const
{
  DictionaryDatum dict = get_kernel_status();

  i->OStack.push( dict );
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

   SeeAlso: Run, Prepare, Cleanup, unit_conversion
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

   SeeAlso: Simulate, unit_conversion, Prepare, Cleanup
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
   /model          Create -> node_ids
   /model n        Create -> node_ids
   /model   params Create -> node_ids
   /model n params Create -> node_ids

   Parameters:
   /model - literal naming the modeltype (entry in modeldict)
   n      - the desired number of nodes
   params - parameters for the newly created node(s)

   Returns:
   node_ids   - NodeCollection representing nodes created

   Description:
   Create generates n new network objects of the supplied model
   type. If n is not given, a single node is created. The objects
   are added as children of the current working node. params is a
   dictsionary with parameters for the new nodes.

   SeeAlso: modeldict
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

  NodeCollectionDatum nodes_created = create( modname, n_nodes );

  i->OStack.pop( 2 );
  i->OStack.push( nodes_created );
  i->EStack.pop();
}

void
NestModule::GetNodes_D_b::execute( SLIInterpreter* i ) const
{
  // check for stack load
  i->assert_stack_load( 2 );

  // extract arguments
  const bool local_only = getValue< bool >( i->OStack.pick( 0 ) );
  const DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );

  NodeCollectionDatum nodes = get_nodes( params, local_only );

  i->OStack.pop( 2 );
  i->OStack.push( nodes );
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
   SeeAlso: reset, ResetOptions
*/
void
NestModule::ResetKernelFunction::execute( SLIInterpreter* i ) const
{
  reset_kernel();
  i->EStack.pop();
}

// Disconnect for nodecollection nodecollection conn_spec syn_spec
void
NestModule::Disconnect_g_g_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by disconnect
  kernel().sp_manager.disconnect( sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();
}

// Connect for nodecollection nodecollection conn_spec syn_spec
// See lib/sli/nest-init.sli for details
void
NestModule::Connect_g_g_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by connect
  kernel().connection_manager.connect( sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();
}

void
NestModule::Connect_nonunique_ia_ia_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  TokenArray sources = getValue< TokenArray >( i->OStack.pick( 2 ) );
  TokenArray targets = getValue< TokenArray >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by connect
  kernel().connection_manager.connect( sources, targets, synapse_params );

  i->OStack.pop( 3 );
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
   Name: PrintNodes - Print nodes in the network.
   Synopsis:
   -  PrintNodes -> -
   Description:
   Print node ID ranges and model names of the nodes in the network. Print the
   information directly to screen.
*/

void
NestModule::PrintNodesFunction::execute( SLIInterpreter* i ) const
{
  print_nodes_to_stream();
  std::cout << std::endl;
  i->EStack.pop();
}

/* BeginDocumentation
   Name: PrintNodesToStream - Redirect printing of nodes in the network.
   Synopsis:
   -  PrintNodesToStream -> -
   Description:
   Returns string output that can be used to print information about the nodes
   in the network.
   The string is the information directly printed by PrintNodes.
*/

void
NestModule::PrintNodesToStreamFunction::execute( SLIInterpreter* i ) const
{
  std::stringstream out;
  print_nodes_to_stream( out );

  i->OStack.push( out.str() );
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
NestModule::TimeCommunicationAlltoall_i_iFunction::execute( SLIInterpreter* i ) const
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
NestModule::TimeCommunicationAlltoallv_i_iFunction::execute( SLIInterpreter* i ) const
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
NestModule::Cvnodecollection_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const long first = getValue< long >( i->OStack.pick( 1 ) );
  const long last = getValue< long >( i->OStack.pick( 0 ) );

  NodeCollectionDatum nodecollection = new NodeCollectionPrimitive( first, last );

  i->OStack.pop( 2 );
  i->OStack.push( nodecollection );
  i->EStack.pop();
}

void
NestModule::Cvnodecollection_iaFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  TokenArray node_ids = getValue< TokenArray >( i->OStack.pick( 0 ) );

  NodeCollectionDatum nodecollection( NodeCollection::create( node_ids ) );

  i->OStack.pop();
  i->OStack.push( nodecollection );
  i->EStack.pop();
}

void
NestModule::Cvnodecollection_ivFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  IntVectorDatum node_ids = getValue< IntVectorDatum >( i->OStack.pick( 0 ) );
  NodeCollectionDatum nodecollection( NodeCollection::create( node_ids ) );

  i->OStack.pop();
  i->OStack.push( nodecollection );
  i->EStack.pop();
}

void
NestModule::Cva_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  ArrayDatum node_ids = nodecollection->to_array();

  i->OStack.pop();
  i->OStack.push( node_ids );
  i->EStack.pop();
}

void
NestModule::Size_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  i->OStack.pop();
  i->OStack.push( nodecollection->size() );
  i->EStack.pop();
}

void
NestModule::ValidQ_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  i->OStack.pop();
  i->OStack.push( nodecollection->valid() );
  i->EStack.pop();
}

void
NestModule::Join_g_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum left = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  NodeCollectionDatum right = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  NodeCollectionDatum combined = left + right;

  i->OStack.pop( 2 );
  i->OStack.push( combined );
  i->EStack.pop();
}

void
NestModule::MemberQ_g_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const long node_id = getValue< long >( i->OStack.pick( 0 ) );

  const bool res = nodecollection->contains( node_id );
  i->OStack.pop( 2 );
  i->OStack.push( res );
  i->EStack.pop();
}

void
NestModule::Find_g_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const long node_id = getValue< long >( i->OStack.pick( 0 ) );

  const auto res = nodecollection->find( node_id );
  i->OStack.pop( 2 );
  i->OStack.push( res );
  i->EStack.pop();
}

void
NestModule::eq_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  NodeCollectionDatum nodecollection_other = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  const bool res = nodecollection->operator==( nodecollection_other );
  i->OStack.pop( 2 );
  i->OStack.push( res );
  i->EStack.pop();
}

void
NestModule::BeginIterator_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  NodeCollectionIteratorDatum it = new nc_const_iterator( nodecollection->begin( nodecollection ) );

  i->OStack.pop();
  i->OStack.push( it );
  i->EStack.pop();
}

void
NestModule::EndIterator_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  NodeCollectionIteratorDatum it = new nc_const_iterator( nodecollection->end( nodecollection ) );

  i->OStack.pop();
  i->OStack.push( it );
  i->EStack.pop();
}

void
NestModule::GetNodeID_qFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionIteratorDatum it = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 0 ) );

  index node_id = ( **it ).node_id;

  i->OStack.pop();
  i->OStack.push( node_id );
  i->EStack.pop();
}

void
NestModule::GetNodeIDModelID_qFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionIteratorDatum it = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 0 ) );

  ArrayDatum gm_pair;
  const NodeIDTriple& gp = **it;
  gm_pair.push_back( gp.node_id );
  gm_pair.push_back( gp.model_id );

  i->OStack.pop();
  i->OStack.push( gm_pair );
  i->EStack.pop();
}

void
NestModule::Next_qFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  NodeCollectionIteratorDatum it = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 0 ) );

  ++( *it );

  // leave iterator on stack
  i->EStack.pop();
}

void
NestModule::Eq_q_qFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionIteratorDatum it_l = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 1 ) );
  NodeCollectionIteratorDatum it_r = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 0 ) );

  const bool res = not it_l->operator!=( *it_r );

  // leave iterator on stack
  i->OStack.pop( 2 );
  i->OStack.push( res );
  i->EStack.pop();
}

void
NestModule::Lt_q_qFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionIteratorDatum it_l = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 1 ) );
  NodeCollectionIteratorDatum it_r = getValue< NodeCollectionIteratorDatum >( i->OStack.pick( 0 ) );

  const bool res = it_l->operator<( *it_r );

  // leave iterator on stack
  i->OStack.pop( 2 );
  i->OStack.push( res );
  i->EStack.pop();
}

void
NestModule::Get_g_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  long idx = getValue< long >( i->OStack.pick( 0 ) );

  const size_t g_size = nodecollection->size();
  if ( idx < 0 )
  {
    idx = g_size + idx;
  }
  if ( not( 0 <= idx and idx < static_cast< long >( g_size ) ) )
  {
    throw RangeCheck();
  }

  const index node_id = ( *nodecollection )[ idx ];

  i->OStack.pop( 2 );
  i->OStack.push( node_id );
  i->EStack.pop();
}

void
NestModule::Take_g_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  TokenArray slice = getValue< TokenArray >( i->OStack.pick( 0 ) );

  if ( slice.size() != 3 )
  {
    throw DimensionMismatch( 3, slice.size() );
  }

  const size_t g_size = nodecollection->size();
  long start = slice[ 0 ];
  long stop = slice[ 1 ];
  long step = slice[ 2 ];

  if ( step < 1 )
  {
    throw BadParameter( "Slicing step must be strictly positive." );
  }

  if ( start >= 0 )
  {
    start -= 1; // adjust from 1-based to 0-based indexing
  }
  else
  {
    start += g_size; // automatically correct for 0-based indexing
  }

  if ( stop >= 0 )
  {
    // no adjustment necessary: adjustment from 1- to 0- based indexing
    // and adjustment from last- to stop-based logic cancel
  }
  else
  {
    stop += g_size + 1; // adjust from 0- to 1- based indexin
  }

  NodeCollectionDatum sliced_nc = nodecollection->slice( start, stop, step );

  i->OStack.pop( 2 );
  i->OStack.push( sliced_nc );
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

  kernel().music_manager.set_music_in_port_acceptable_latency( port_name, latency );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::SetMaxBufferedFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::string port_name = getValue< std::string >( i->OStack.pick( 1 ) );
  int maxBuffered = getValue< long >( i->OStack.pick( 0 ) );

  kernel().music_manager.set_music_in_port_max_buffered( port_name, maxBuffered );

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
NestModule::SetStructuralPlasticityStatus_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  DictionaryDatum structural_plasticity_dictionary = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  kernel().sp_manager.set_status( structural_plasticity_dictionary );

  i->OStack.pop( 1 );
  i->EStack.pop();
}

void
NestModule::GetStructuralPlasticityStatus_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  DictionaryDatum current_status = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );
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
NestModule::EnableStructuralPlasticity_Function::execute( SLIInterpreter* i ) const
{
  kernel().sp_manager.enable_structural_plasticity();

  i->EStack.pop();
}
/**
 * Disable Structural Plasticity in the network.
 * @param i
 */
void
NestModule::DisableStructuralPlasticity_Function::execute( SLIInterpreter* i ) const
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


/** @BeginDocumentation
  Name: CreateParameter
*/
void
NestModule::CreateParameter_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  const DictionaryDatum param_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  ParameterDatum datum = nest::create_parameter( param_dict );

  i->OStack.pop( 1 );
  i->OStack.push( datum );
  i->EStack.pop();
}

void
NestModule::Mul_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = multiply_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Div_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = divide_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Add_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = add_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Exp_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = exp_parameter( param );

  i->OStack.pop( 1 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Sin_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = sin_parameter( param );

  i->OStack.pop( 1 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Cos_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = cos_parameter( param );

  i->OStack.pop( 1 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Pow_P_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  double exponent = getValue< double >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = pow_parameter( param, exponent );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Sub_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = subtract_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}


void
NestModule::Compare_P_P_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 2 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum param3 = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = compare_parameter( param1, param2, param3 );

  i->OStack.pop( 3 );
  i->OStack.push( newparam );
  i->EStack.pop();
}


void
NestModule::Conditional_P_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 2 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param3 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = conditional_parameter( param1, param2, param3 );

  i->OStack.pop( 3 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Min_P_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  double other_value = getValue< double >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = min_parameter( param, other_value );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Max_P_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  double other_value = getValue< double >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = max_parameter( param, other_value );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Redraw_P_d_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 2 ) );
  double min = getValue< double >( i->OStack.pick( 1 ) );
  double max = getValue< double >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = redraw_parameter( param, min, max );

  i->OStack.pop( 3 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Dimension2d_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = dimension_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
NestModule::Dimension3d_P_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 2 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param3 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = dimension_parameter( param1, param2, param3 );

  i->OStack.pop( 3 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: GetValue
*/
void
NestModule::GetValue_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  double value = get_value( param );

  i->OStack.pop( 1 );
  i->OStack.push( value );
  i->EStack.pop();
}

void
NestModule::IsSpatial_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  auto param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  bool parameter_is_spatial = is_spatial( param );

  i->OStack.pop( 1 );
  i->OStack.push( parameter_is_spatial );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: Apply
*/
void
NestModule::Apply_P_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  auto positions = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );
  auto param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );

  auto result = apply( param, positions );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::Apply_P_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  NodeCollectionDatum nc = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );

  auto result = apply( param, nc );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::init( SLIInterpreter* i )
{
  ConnectionType.settypename( "connectiontype" );
  ConnectionType.setdefaultaction( SLIInterpreter::datatypefunction );

  NodeCollectionType.settypename( "nodecollectiontype" );
  NodeCollectionType.setdefaultaction( SLIInterpreter::datatypefunction );

  NodeCollectionIteratorType.settypename( "nodecollectioniteratortype" );
  NodeCollectionIteratorType.setdefaultaction( SLIInterpreter::datatypefunction );

  ParameterType.settypename( "parametertype" );
  ParameterType.setdefaultaction( SLIInterpreter::datatypefunction );

  // register interface functions with interpreter

  i->createcommand( "SetStatus_id", &setstatus_idfunction );
  i->createcommand( "SetStatus_CD", &setstatus_CDfunction );
  i->createcommand( "SetStatus_aa", &setstatus_aafunction );
  i->createcommand( "SetKernelStatus", &setkernelstatus_Dfunction );

  i->createcommand( "GetStatus_g", &getstatus_gfunction );
  i->createcommand( "GetStatus_i", &getstatus_ifunction );
  i->createcommand( "GetStatus_C", &getstatus_Cfunction );
  i->createcommand( "GetStatus_a", &getstatus_afunction );
  i->createcommand( "GetMetadata_g", &getmetadata_gfunction );
  i->createcommand( "GetKernelStatus", &getkernelstatus_function );

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

  i->createcommand( "GetNodes_D_b", &getnodes_D_bfunction );

  i->createcommand( "mul_P_P", &mul_P_Pfunction );
  i->createcommand( "div_P_P", &div_P_Pfunction );
  i->createcommand( "add_P_P", &add_P_Pfunction );
  i->createcommand( "sub_P_P", &sub_P_Pfunction );

  i->createcommand( "compare_P_P_D", &compare_P_P_Dfunction );
  i->createcommand( "conditional_P_P_P", &conditional_P_P_Pfunction );

  i->createcommand( "min_P_d", &min_P_dfunction );
  i->createcommand( "max_P_d", &max_P_dfunction );
  i->createcommand( "redraw_P_d_d", &redraw_P_d_dfunction );

  i->createcommand( "exp_P", &exp_Pfunction );
  i->createcommand( "sin_P", &sin_Pfunction );
  i->createcommand( "cos_P", &cos_Pfunction );
  i->createcommand( "pow_P_d", &pow_P_dfunction );

  i->createcommand( "dimension2d_P_P", &dimension2d_P_Pfunction );
  i->createcommand( "dimension3d_P_P_P", &dimension3d_P_P_Pfunction );

  i->createcommand( "CreateParameter_D", &createparameter_Dfunction );

  i->createcommand( "GetValue_P", &getvalue_Pfunction );
  i->createcommand( "IsSpatial_P", &isspatial_Pfunction );
  i->createcommand( "Apply_P_D", &apply_P_Dfunction );
  i->createcommand( "Apply_P_g", &apply_P_gfunction );

  i->createcommand( "Connect_g_g_D_D", &connect_g_g_D_Dfunction );
  i->createcommand( "Connect_nonunique_ia_ia_D", &connect_nonunique_ia_ia_Dfunction );

  i->createcommand( "ResetKernel", &resetkernelfunction );

  i->createcommand( "MemoryInfo", &memoryinfofunction );

  i->createcommand( "PrintNodes", &printnodesfunction );
  i->createcommand( "PrintNodesToStream", &printnodestostreamfunction );

  i->createcommand( "Rank", &rankfunction );
  i->createcommand( "NumProcesses", &numprocessesfunction );
  i->createcommand( "SetFakeNumProcesses", &setfakenumprocesses_ifunction );
  i->createcommand( "SyncProcesses", &syncprocessesfunction );
  i->createcommand( "TimeCommunication_i_i_b", &timecommunication_i_i_bfunction );
  i->createcommand( "TimeCommunicationv_i_i", &timecommunicationv_i_ifunction );
  i->createcommand( "TimeCommunicationAlltoall_i_i", &timecommunicationalltoall_i_ifunction );
  i->createcommand( "TimeCommunicationAlltoallv_i_i", &timecommunicationalltoallv_i_ifunction );
  i->createcommand( "ProcessorName", &processornamefunction );
#ifdef HAVE_MPI
  i->createcommand( "MPI_Abort", &mpiabort_ifunction );
#endif

  i->createcommand( "GetGlobalRNG", &getglobalrngfunction );

  i->createcommand( "cvdict_C", &cvdict_Cfunction );

  i->createcommand( "cvnodecollection_i_i", &cvnodecollection_i_ifunction );
  i->createcommand( "cvnodecollection_ia", &cvnodecollection_iafunction );
  i->createcommand( "cvnodecollection_iv", &cvnodecollection_ivfunction );
  i->createcommand( "cva_g", &cva_gfunction );
  i->createcommand( "size_g", &size_gfunction );
  i->createcommand( "ValidQ_g", &validq_gfunction );
  i->createcommand( "join_g_g", &join_g_gfunction );
  i->createcommand( "MemberQ_g_i", &memberq_g_ifunction );
  i->createcommand( "Find_g_i", &find_g_ifunction );
  i->createcommand( "eq_g", &eq_gfunction );
  i->createcommand( ":beginiterator_g", &beginiterator_gfunction );
  i->createcommand( ":enditerator_g", &enditerator_gfunction );
  i->createcommand( ":getnodeid_q", &getnodeid_qfunction );
  i->createcommand( ":getnodeidmodelid_q", &getnodeidmodelid_qfunction );
  i->createcommand( ":next_q", &next_qfunction );
  i->createcommand( ":eq_q_q", &eq_q_qfunction );
  i->createcommand( ":lt_q_q", &lt_q_qfunction );
  i->createcommand( "get_g_i", &get_g_ifunction );
  i->createcommand( "Take_g_a", &take_g_afunction );

#ifdef HAVE_MUSIC
  i->createcommand( "SetAcceptableLatency", &setacceptablelatency_l_dfunction );
  i->createcommand( "SetMaxBuffered", &setmaxbuffered_l_ifunction );
#endif
  i->createcommand( "EnableStructuralPlasticity", &enablestructuralplasticity_function );
  i->createcommand( "DisableStructuralPlasticity", &disablestructuralplasticity_function );
  i->createcommand( "SetStructuralPlasticityStatus", &setstructuralplasticitystatus_Dfunction );
  i->createcommand( "GetStructuralPlasticityStatus", &getstructuralplasticitystatus_function );
  i->createcommand( "Disconnect_g_g_D_D", &disconnect_g_g_D_Dfunction );

  i->createcommand( "SetStdpEps", &setstdpeps_dfunction );

  // Add connection rules
  kernel().connection_manager.register_conn_builder< OneToOneBuilder >( "one_to_one" );
  kernel().connection_manager.register_conn_builder< AllToAllBuilder >( "all_to_all" );
  kernel().connection_manager.register_conn_builder< FixedInDegreeBuilder >( "fixed_indegree" );
  kernel().connection_manager.register_conn_builder< FixedOutDegreeBuilder >( "fixed_outdegree" );
  kernel().connection_manager.register_conn_builder< BernoulliBuilder >( "pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< SymmetricBernoulliBuilder >( "symmetric_pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< FixedTotalNumberBuilder >( "fixed_total_number" );

  // Add MSP growth curves
  kernel().sp_manager.register_growth_curve< GrowthCurveSigmoid >( "sigmoid" );
  kernel().sp_manager.register_growth_curve< GrowthCurveGaussian >( "gaussian" );
  kernel().sp_manager.register_growth_curve< GrowthCurveLinear >( "linear" );

  Token statusd = i->baselookup( Name( "statusdict" ) );
  DictionaryDatum dd = getValue< DictionaryDatum >( statusd );
  dd->insert( Name( "kernelname" ), new StringDatum( "NEST" ) );
  dd->insert( Name( "is_mpi" ), new BoolDatum( kernel().mpi_manager.is_mpi_used() ) );

  register_parameter< ConstantParameter >( "constant" );
  register_parameter< UniformParameter >( "uniform" );
  register_parameter< NormalParameter >( "normal" );
  register_parameter< LognormalParameter >( "lognormal" );
  register_parameter< ExponentialParameter >( "exponential" );
  register_parameter< NodePosParameter >( "position" );
  register_parameter< SpatialDistanceParameter >( "distance" );
}

} // namespace nest
