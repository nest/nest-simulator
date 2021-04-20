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
#include "conn_builder_conngen.h"
#include "connection_creator_impl.h"
#include "connection_manager_impl.h"
#include "free_layer.h"
#include "genericmodel.h"
#include "grid_layer.h"
#include "grid_mask.h"
#include "kernel_manager.h"
#include "layer.h"
#include "layer_impl.h"
#include "mask.h"
#include "mask_impl.h"
#include "model_manager_impl.h"
#include "nest.h"
#include "nest_datums.h"
#include "nest_types.h"
#include "node.h"
#include "sp_manager_impl.h"
#include "spatial.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "interpret.h"
#include "sliexceptions.h"
#include "stringdatum.h"
#include "tokenutils.h"

namespace nest
{
#ifdef HAVE_LIBNEUROSIM
SLIType NestModule::ConnectionGeneratorType;
#endif
SLIType NestModule::ConnectionType;
SLIType NestModule::MaskType;
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


GenericFactory< AbstractMask >&
NestModule::mask_factory_( void )
{
  static GenericFactory< AbstractMask > factory;
  return factory;
}

MaskDatum
NestModule::create_mask( const Token& t )
{
  // t can be either an existing MaskDatum, or a Dictionary containing
  // mask parameters
  MaskDatum* maskd = dynamic_cast< MaskDatum* >( t.datum() );
  if ( maskd )
  {
    return *maskd;
  }
  else
  {

    DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( t.datum() );
    if ( dd == 0 )
    {
      throw BadProperty( "Mask must be masktype or dictionary." );
    }

    // The dictionary should contain one key which is the name of the
    // mask type, and optionally the key 'anchor'. To find the unknown
    // mask type key, we must loop through all keys. The value for the
    // anchor key will be stored in the anchor_token variable.
    Token anchor_token;
    bool has_anchor = false;
    AbstractMask* mask = 0;

    for ( Dictionary::iterator dit = ( *dd )->begin(); dit != ( *dd )->end(); ++dit )
    {

      if ( dit->first == names::anchor )
      {

        anchor_token = dit->second;
        has_anchor = true;
      }
      else
      {

        if ( mask != 0 )
        { // mask has already been defined
          throw BadProperty( "Mask definition dictionary contains extraneous items." );
        }
        mask = create_mask( dit->first, getValue< DictionaryDatum >( dit->second ) );
      }
    }

    if ( has_anchor )
    {

      // The anchor may be an array of doubles (a spatial position).
      // For grid layers only, it is also possible to provide an array of longs.
      try
      {

        std::vector< double > anchor = getValue< std::vector< double > >( anchor_token );
        AbstractMask* amask;

        switch ( anchor.size() )
        {
        case 2:
          amask = new AnchoredMask< 2 >( dynamic_cast< Mask< 2 >& >( *mask ), anchor );
          break;
        case 3:
          amask = new AnchoredMask< 3 >( dynamic_cast< Mask< 3 >& >( *mask ), anchor );
          break;
        default:
          throw BadProperty( "Anchor must be 2- or 3-dimensional." );
        }

        delete mask;
        mask = amask;
      }
      catch ( TypeMismatch& e )
      {
        std::vector< long > anchor = getValue< std::vector< long > >( anchor_token );

        switch ( anchor.size() )
        {
        case 2:
          try
          {
            GridMask< 2 >& grid_mask_2d = dynamic_cast< GridMask< 2 >& >( *mask );
            grid_mask_2d.set_anchor( Position< 2, int >( anchor[ 0 ], anchor[ 1 ] ) );
          }
          catch ( std::bad_cast& e )
          {
            throw BadProperty( "Mask must be 2-dimensional grid mask." );
          }
          break;
        case 3:
          try
          {
            GridMask< 3 >& grid_mask_3d = dynamic_cast< GridMask< 3 >& >( *mask );
            grid_mask_3d.set_anchor( Position< 3, int >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          }
          catch ( std::bad_cast& e )
          {
            throw BadProperty( "Mask must be 3-dimensional grid mask." );
          }
          break;
        }
      }
    }

    return mask;
  }
}

static AbstractMask*
create_doughnut( const DictionaryDatum& d )
{
  // The doughnut (actually an annulus) is created using a DifferenceMask
  Position< 2 > center( 0, 0 );
  if ( d->known( names::anchor ) )
  {
    center = getValue< std::vector< double > >( d, names::anchor );
  }

  const double outer = getValue< double >( d, names::outer_radius );
  const double inner = getValue< double >( d, names::inner_radius );
  if ( inner >= outer )
  {
    throw BadProperty(
      "nest::create_doughnut: "
      "inner_radius < outer_radius required." );
  }

  BallMask< 2 > outer_circle( center, outer );
  BallMask< 2 > inner_circle( center, inner );

  return new DifferenceMask< 2 >( outer_circle, inner_circle );
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
   Name: Create - create nodes

   Synopsis:
   /model          Create -> NodeCollection
   /model n        Create -> NodeCollection
   /model   params Create -> NodeCollection
   /model n params Create -> NodeCollection

   Parameters:
   /model - literal naming the modeltype (entry in modeldict)
   n      - the desired number of nodes
   params - parameters for the newly created node(s)

   Returns:
   node_ids   - NodeCollection representing nodes created

   Description:
   Create generates n new network objects of the supplied model
   type. If n is not given, a single node is created. params is a
   dictionary with parameters for the new nodes.

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
  kernel().connection_manager.sw_construction_connect.start();

  i->assert_stack_load( 4 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by connect
  kernel().connection_manager.connect( sources, targets, connectivity, { synapse_params } );

  i->OStack.pop( 4 );
  i->EStack.pop();

  kernel().connection_manager.sw_construction_connect.stop();
}

void
NestModule::Connect_g_g_D_aFunction::execute( SLIInterpreter* i ) const
{
  kernel().connection_manager.sw_construction_connect.start();

  i->assert_stack_load( 4 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  ArrayDatum synapse_params_arr = getValue< ArrayDatum >( i->OStack.pick( 0 ) );
  std::vector< DictionaryDatum > synapse_params;

  for ( auto syn_param : synapse_params_arr )
  {
    synapse_params.push_back( getValue< DictionaryDatum >( syn_param ) );
  }

  // dictionary access checking is handled by connect
  kernel().connection_manager.connect( sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();

  kernel().connection_manager.sw_construction_connect.stop();
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
   Parameters:
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

/**
 * Enable Structural Plasticity within the simulation. This allows
 * dynamic rewiring of the network based on mean electrical activity.
 * Please note that, in the current implementation of structural plasticity,
 * spikes could occasionally be delivered via connections that were not present
 * at the time of the spike.
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

#ifdef HAVE_LIBNEUROSIM

/** @BeginDocumentation
Name: CGParse - Call ConnectionGenerator::fromXML() and return a
ConnectionGenerator

Synopsis:
xml_string CGParse -> cg

Parameters:
xml_string - The XML string to parse.

Description:
Return a ConnectionGenerator created by deserializing the given
XML string. The library to parse the XML string can be selected using
CGSelectImplementation

Availability: Only if compiled with libneurosim support
Author: Jochen Martin Eppler
FirstVersion: September 2013
SeeAlso: CGParseFile, CGSelectImplementation
*/
void
NestModule::CGParse_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = ConnectionGenerator::fromXML( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
}

/** @BeginDocumentation
Name: CGParseFile - Call ConnectionGenerator::fromXMLFile() and return a
ConnectionGenerator

Synopsis:
xml_filename CGParseFile -> cg

Parameters:
xml_filename - The XML file to read.

Description:
Return a ConnectionGenerator created by deserializing the given
XML file. The library to parse the XML file can be selected using
CGSelectImplementation

Availability: Only if compiled with libneurosim support
Author: Jochen Martin Eppler
FirstVersion: February 2014
SeeAlso: CGParse, CGSelectImplementation
*/
void
NestModule::CGParseFile_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = ConnectionGenerator::fromXMLFile( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
}

/** @BeginDocumentation
Name: CGSelectImplementation - Call
ConnectionGenerator::selectCGImplementation()

Synopsis:
tag library CGParse -> -

Parameters:
tag     - The XML tag to associate with a library.
library - The library to provide the parsing for CGParse

Description:
Select a library to provide a parser for XML files and associate
an XML tag with the library.

Availability: Only if compiled with libneurosim support
Author: Jochen Martin Eppler
FirstVersion: September 2013
SeeAlso: CGParse, CGParseFile
*/
void
NestModule::CGSelectImplementation_s_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  StringDatum library = getValue< StringDatum >( i->OStack.pick( 0 ) );
  StringDatum tag = getValue< StringDatum >( i->OStack.pick( 1 ) );

  ConnectionGenerator::selectCGImplementation( tag, library );

  i->OStack.pop( 1 );
  i->EStack.pop();
}

#endif /* #ifdef HAVE_LIBNEUROSIM */

//
// SLI functions for spatial networks
//

/** @BeginDocumentation
  Name: nest::CreateLayer - create nodes with spatial properties

  Synopsis:
  dict CreateLayer -> layer

  Parameters:
  dict - dictionary with layer specification

  Description: Creates a NodeCollection which contains information
  about the spatial position of its nodes. Positions can be organized
  in one of two layer classes: grid-based layers, in which each element
  is placed at a location in a regular grid, and free layers, in which
  elements can be placed arbitrarily in space.  Which kind of layer
  this command creates depends on the elements in the supplied
  specification dictionary.

  Author: Håkon Enger, Kittel Austvoll
*/
void
NestModule::CreateLayer_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum layer_dict = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  NodeCollectionDatum layer = create_layer( layer_dict );

  for ( auto&& node_id_triple : *layer )
  {
    set_node_status( node_id_triple.node_id, params );
  }

  i->OStack.pop( 2 );
  i->OStack.push( layer );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::GetPosition - retrieve position of input node

  Synopsis: NodeCollection GetPosition -> [array]

  Parameters:
  layer      - NodeCollection for layer with layer nodes

  Returns:
  [array]    - spatial position of node [x y]

  Description: Retrieves spatial 2D position of layer node(s).

  Examples:

  %%Create layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> /dictionary Set

  dictionary CreateLayer /src Set

  src [4] Take GetPosition

  Author: Kittel Austvoll
*/

void
NestModule::GetPosition_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  ArrayDatum result = get_position( layer );

  i->OStack.pop( 1 );
  if ( layer->size() == 1 )
  {
    i->OStack.push( result[ 0 ] );
  }
  else
  {
    i->OStack.push( result );
  }
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::Displacement - compute displacement vector

  Synopsis: layer from_node_id to_node_id Displacement -> [double vector]
            layer from_pos to_node_id Displacement -> [double vector]

  Parameters:
  layer           - NodeCollection for layer
  from_node_id    - int, node_id of node in a spatial NodeCollection
  from_pos        - double vector, position in layer
  to_node_id      - int, node_id of node in a spatial NodeCollection

  Returns:
  [double vector] - vector pointing from position "from" to position "to"

  Description:
  This function returns a vector connecting the position of the "from_node_id"
  node or the explicitly given "from_pos" position and the position of the
  "to_node_id" node. Nodes must be parts of a spatial NodeCollection.

  The "from" position is projected into the layer of the "to_node_id" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest displacement vector is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer
  /layer Set

  layer [4] Take layer [5] Take Displacement
  [[0.2 0.3]] layer [5] Take Displacement

  Author: Håkon Enger, Hans E Plesser, Kittel Austvoll

  See also: Distance, GetPosition
*/
void
NestModule::Displacement_g_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer_to = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  const NodeCollectionDatum layer_from = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  if ( layer_to->size() != 1 and layer_from->size() != 1 and not( layer_to->size() == layer_from->size() ) )
  {
    throw BadProperty( "NodeCollections must have equal length or one must have size 1." );
  }

  ArrayDatum result = displacement( layer_to, layer_from );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::Displacement_a_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  const ArrayDatum point = getValue< ArrayDatum >( i->OStack.pick( 1 ) );

  ArrayDatum result = displacement( layer, point );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::Distance - compute distance between nodes

  Synopsis: layer from_node_id to_node_id Distance -> double
            layer from_pos to_node_id Distance -> double

  Parameters:
  layer       - NodeCollection for layer
  from_node_id    - int, node_id of node in a spatial NodeCollection
  from_pos    - double vector, position in layer
  to_node_id      - int, node_id of node in a spatial NodeCollection

  Returns:
  double - distance between nodes or given position and node

  Description:
  This function returns the distance between the position of the "from_node_id"
  node or the explicitly given "from_pos" position and the position of the
  "to_node_id" node. Nodes must be parts of a spatial NodeCollection.

  The "from" position is projected into the layer of the "to_node_id" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest distance is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  /layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer def

  layer [4] Take layer [5] Take Distance
  [[ 0.2 0.3 ]] layer [5] Take Distance

  Author: Hans E Plesser, Kittel Austvoll

  See also: Displacement, GetPosition
*/
void
NestModule::Distance_g_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer_to = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  const NodeCollectionDatum layer_from = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  if ( layer_to->size() != 1 and layer_from->size() != 1 and not( layer_to->size() == layer_from->size() ) )
  {
    throw BadProperty( "NodeCollections must have equal length or one must have size 1." );
  }

  Token result = distance( layer_to, layer_from );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::Distance_a_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  const ArrayDatum point = getValue< ArrayDatum >( i->OStack.pick( 1 ) );

  Token result = distance( layer, point );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
NestModule::Distance_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const ArrayDatum conns = getValue< ArrayDatum >( i->OStack.pick( 0 ) );

  Token result = distance( conns );

  i->OStack.pop( 1 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::CreateMask - create a spatial mask

  Synopsis:
  << /type dict >> CreateMask -> mask

  Parameters:
  /type - mask type
  dict  - dictionary with mask specifications

  Description: Masks can be used when creating connections between nodes
  with spatial parameters. A mask describes which area of the pool layer
  shall be searched for nodes to connect for any given node in the driver
  layer. This command creates a mask object which may be combined with other
  mask objects using Boolean operators. The mask is specified in a dictionary.

  Author: Håkon Enger
*/
void
NestModule::CreateMask_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const DictionaryDatum mask_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  MaskDatum datum = nest::create_mask( mask_dict );

  i->OStack.pop( 1 );
  i->OStack.push( datum );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::Inside - test if a point is inside a mask

  Synopsis:
  point mask Inside -> bool

  Parameters:
  point - array of coordinates
  mask - mask object

  Returns:
  bool - true if the point is inside the mask
*/
void
NestModule::Inside_a_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::vector< double > point = getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  bool ret = inside( point, mask );

  i->OStack.pop( 2 );
  i->OStack.push( Token( BoolDatum( ret ) ) );
  i->EStack.pop();
}

void
NestModule::And_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = intersect_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

void
NestModule::Or_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = union_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

void
NestModule::Sub_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = minus_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::ConnectLayers - connect two layers

  Synopsis: sourcelayer targetlayer connection_dict
  ConnectLayers -> -

  Description: Connects nodes in two topological layers.

  The parameters set in the input dictionary decides the nature
  of the connection pattern being created. Please see parameter
  list below for a detailed description of these variables.

  The connections are created by iterating through either the
  source or the target layer, consecutively connecting each node
  to a region in the opposing layer.

  Parameters:
  sourcelayer  - NodeCollection for source layer
  targetlayer  - NodeCollection for target layer

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

  *modeltype (i.e. /iaf_psc_alpha) of nodes that should be connected to
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


      Parameter name: synapse_model

      Type: literal

      Parameter description:

      The synapse model to be used for creating the connection.
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

  %Create source layer with CreateLayer
  << /rows 15
     /columns 43
     /extent [1.0 2.0]
     /elements /iaf_psc_alpha
  >> /src_dictionary Set

  src_dictionary CreateLayer /src Set

  %Create target layer with CreateLayer
  %%Create layer
  << /rows 34
     /columns 71
     /extent [3.0 1.0]
     /elements /iaf_psc_alpha
  >> /tgt_dictionary Set

  tgt_dictionary CreateLayer /tgt Set

  <<  /connection_type (convergent)
      /mask << /grid << /rows 2 /columns 3 >>
               /anchor << /row 4 /column 2 >> >>
      /weight 2.3
      /delay [2.3 1.2 3.2 1.3 2.3 1.2]
      /kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
      /synapse_model /stdp_synapse

  >> /parameters Set

  src tgt parameters ConnectLayers

  Author: Håkon Enger, Kittel Austvoll

  SeeAlso: nest::CreateLayer
*/
void
NestModule::ConnectLayers_g_g_DFunction::execute( SLIInterpreter* i ) const
{
  kernel().connection_manager.sw_construction_connect.start();

  i->assert_stack_load( 3 );

  const NodeCollectionDatum source = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  const NodeCollectionDatum target = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const DictionaryDatum connection_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  connect_layers( source, target, connection_dict );

  i->OStack.pop( 3 );
  i->EStack.pop();

  kernel().connection_manager.sw_construction_connect.stop();
}

/** @BeginDocumentation

  Name: nest::GetLayerStatus - return information about layer

  Synopsis:
  layer GetLayerStatus -> dict

  Parameters:
  layer - NodeCollection representing layer

  Returns:
  Status dictionary with information about layer
 */
void
NestModule::GetLayerStatus_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  DictionaryDatum result = get_layer_status( layer );

  i->OStack.pop( 1 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::DumpLayerNodes - write information about layer nodes to file

  Synopsis: ostream layer DumpLayerNodes -> ostream

  Parameters:
  ostream - open output stream
  layer   - NodeCollection for layer

  Description:
  Write information about each element in the given layer to the
  output stream. The file format is one line per element with the
  following contents:

  node ID x-position y-position [z-position]

  X and y position are given as physical coordinates in the extent,
  not as grid positions. The number of decimals can be controlled by
  calling setprecision on the output stream before calling DumpLayerNodes.

  Remarks:
  In distributed simulations, this function should only be called for
  MPI rank 0. If you call it on several MPI ranks, you must use a
  different file name on each.

  Examples:

  /my_layer << /rows 5 /columns 4 /elements /iaf_psc_alpha >> CreateLayer def

  (my_layer_dump.lyr) (w) file
  my_layer DumpLayerNodes
  close

  Author: Kittel Austvoll, Hans Ekkehard Plesser

  SeeAlso: nest::DumpLayerConnections, setprecision, modeldict
*/
void
NestModule::DumpLayerNodes_os_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  OstreamDatum out = getValue< OstreamDatum >( i->OStack.pick( 1 ) );

  dump_layer_nodes( layer, out );

  i->OStack.pop( 1 ); // leave ostream on stack
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: nest::DumpLayerConnections - prints a list of the connections of the
                                         nodes in the layer to file

  Synopsis: ostream source_layer synapse_model DumpLayerConnections ->
                                                                         ostream

  Parameters:
  ostream          - open outputstream
  source_layer     - NodeCollection for layer
  synapse_model    - synapse model (literal)

  Description:
  Dumps information about all connections of the given type having their source
  in the given layer to the given output stream. The data format is one line per
  connection as follows:

  source_node_id target_node_id weight delay displacement[x,y,z]

  where displacement are up to three coordinates of the vector from the source
  to the target node. If targets do not have positions (eg. spike recorders
  outside any layer), NaN is written for each displacement coordinate.

  Remarks:
  For distributed simulations
  - this function will dump the connections with local targets only.
  - the user is responsible for writing to a different output stream (file)
    on each MPI process.

  Examples:

  (out.cnn) (w) file layer_node_id /static_synapse PrintLayerConnections close

  Author: Kittel Austvoll, Hans Ekkehard Plesser

  SeeAlso: nest::DumpLayerNodes
*/

void
NestModule::DumpLayerConnections_os_g_g_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  OstreamDatum out_file = getValue< OstreamDatum >( i->OStack.pick( 3 ) );
  const NodeCollectionDatum source_layer = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  const NodeCollectionDatum target_layer = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const Token syn_model = i->OStack.pick( 0 );

  dump_layer_connections( syn_model, source_layer, target_layer, out_file );

  i->OStack.pop( 3 ); // leave ostream on stack
  i->EStack.pop();
}

void
NestModule::Cvdict_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );
  DictionaryDatum dict = mask->get_dict();

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}


void
NestModule::SelectNodesByMask_g_a_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const NodeCollectionDatum layer_nc = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  std::vector< double > anchor = getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  std::vector< index > mask_node_ids;

  const int dim = anchor.size();

  if ( dim != 2 and dim != 3 )
  {
    throw BadProperty( "Center must be 2- or 3-dimensional." );
  }

  AbstractLayerPTR abstract_layer = get_layer( layer_nc );

  if ( dim == 2 )
  {
    Layer< 2 >* layer = dynamic_cast< Layer< 2 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "2D layer", "other type" );
    }

    MaskedLayer< 2 > ml = MaskedLayer< 2 >( *layer, mask, false, layer_nc );

    for ( Ntree< 2, index >::masked_iterator it = ml.begin( Position< 2 >( anchor[ 0 ], anchor[ 1 ] ) ); it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }
  else
  {
    Layer< 3 >* layer = dynamic_cast< Layer< 3 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "3D layer", "other type" );
    }

    MaskedLayer< 3 > ml = MaskedLayer< 3 >( *layer, mask, false, layer_nc );

    for ( Ntree< 3, index >::masked_iterator it = ml.begin( Position< 3 >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }

  i->OStack.pop( 3 );
  i->OStack.push( mask_node_ids );
  i->EStack.pop();
}


void
NestModule::init( SLIInterpreter* i )
{
  ConnectionType.settypename( "connectiontype" );
  ConnectionType.setdefaultaction( SLIInterpreter::datatypefunction );

  MaskType.settypename( "masktype" );
  MaskType.setdefaultaction( SLIInterpreter::datatypefunction );

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
  i->createcommand( "Connect_g_g_D_a", &connect_g_g_D_afunction );

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
  i->createcommand( "Disconnect_g_g_D_D", &disconnect_g_g_D_Dfunction );

  i->createcommand( "SetStdpEps", &setstdpeps_dfunction );

  // SLI functions for spatial networks
  i->createcommand( "CreateLayer_D_D", &createlayer_D_Dfunction );
  i->createcommand( "GetPosition_g", &getposition_gfunction );
  i->createcommand( "Displacement_g_g", &displacement_g_gfunction );
  i->createcommand( "Displacement_a_g", &displacement_a_gfunction );
  i->createcommand( "Distance_g_g", &distance_g_gfunction );
  i->createcommand( "Distance_a_g", &distance_a_gfunction );
  i->createcommand( "Distance_a", &distance_afunction );
  i->createcommand( "CreateMask_D", &createmask_Dfunction );
  i->createcommand( "Inside_a_M", &inside_a_Mfunction );
  i->createcommand( "and_M_M", &and_M_Mfunction );
  i->createcommand( "or_M_M", &or_M_Mfunction );
  i->createcommand( "sub_M_M", &sub_M_Mfunction );
  i->createcommand( "ConnectLayers_g_g_D", &connectlayers_g_g_Dfunction );
  i->createcommand( "GetLayerStatus_g", &getlayerstatus_gfunction );
  i->createcommand( "DumpLayerNodes_os_g", &dumplayernodes_os_gfunction );
  i->createcommand( "DumpLayerConnections_os_g_g_l", &dumplayerconnections_os_g_g_lfunction );
  i->createcommand( "cvdict_M", &cvdict_Mfunction );
  i->createcommand( "SelectNodesByMask_g_a_M", &selectnodesbymask_g_a_Mfunction );


  // Add connection rules
  kernel().connection_manager.register_conn_builder< OneToOneBuilder >( "one_to_one" );
  kernel().connection_manager.register_conn_builder< AllToAllBuilder >( "all_to_all" );
  kernel().connection_manager.register_conn_builder< FixedInDegreeBuilder >( "fixed_indegree" );
  kernel().connection_manager.register_conn_builder< FixedOutDegreeBuilder >( "fixed_outdegree" );
  kernel().connection_manager.register_conn_builder< BernoulliBuilder >( "pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< SymmetricBernoulliBuilder >( "symmetric_pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< FixedTotalNumberBuilder >( "fixed_total_number" );
#ifdef HAVE_LIBNEUROSIM
  kernel().connection_manager.register_conn_builder< ConnectionGeneratorBuilder >( "conngen" );
#endif

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

#ifdef HAVE_LIBNEUROSIM
  i->createcommand( "CGParse", &cgparse_sfunction );
  i->createcommand( "CGParseFile", &cgparsefile_sfunction );
  i->createcommand( "CGSelectImplementation", &cgselectimplementation_s_sfunction );
#endif

  register_mask< BallMask< 2 > >();
  register_mask< BallMask< 3 > >();
  register_mask< EllipseMask< 2 > >();
  register_mask< EllipseMask< 3 > >();
  register_mask< BoxMask< 2 > >();
  register_mask< BoxMask< 3 > >();
  register_mask( "doughnut", create_doughnut );
  register_mask< GridMask< 2 > >();
}

} // namespace nest
