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

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "free_layer.h"
#include "grid_mask.h"
#include "kernel_manager.h"
#include "layer_impl.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "module_manager.h"
#include "music_manager.h"
#include "nest.h"
#include "nest_datums.h"
#include "parameter.h"
#include "sp_manager.h"
#include "spatial.h"
#include "stopwatch_impl.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "doubledatum.h"
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
  // The network is deleted outside NestModule, since the dynamicloadermodule also needs it

  ConnectionType.deletetypename();
  NodeCollectionType.deletetypename();
  NodeCollectionIteratorType.deletetypename();
  ParameterType.deletetypename();
}

// The following concerns the new module:

const std::string
NestModule::name() const
{
  return std::string( "NEST Kernel" ); // Return name of the module
}

const std::string
NestModule::commandstring() const
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

  // If t is a IntegerDatum, create a ConstantParameter with this value
  IntegerDatum* id = dynamic_cast< IntegerDatum* >( t.datum() );
  if ( id )
  {
    return new ConstantParameter( static_cast< double >( *id ) );
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
NestModule::parameter_factory_()
{
  static GenericFactory< Parameter > factory;
  return factory;
}


GenericFactory< AbstractMask >&
NestModule::mask_factory_()
{
  static GenericFactory< AbstractMask > factory;
  return factory;
}

MaskDatum
NestModule::create_mask( const Token& t )
{
  // t can be either an existing MaskDatum, or a Dictionary containing mask parameters
  MaskDatum* maskd = dynamic_cast< MaskDatum* >( t.datum() );
  if ( maskd )
  {
    return *maskd;
  }
  else
  {

    DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( t.datum() );
    if ( not dd )
    {
      throw BadProperty( "Mask must be masktype or dictionary." );
    }

    // The dictionary should contain one key which is the name of the
    // mask type, and optionally the key 'anchor'. To find the unknown
    // mask type key, we must loop through all keys. The value for the
    // anchor key will be stored in the anchor_token variable.
    Token anchor_token;
    bool has_anchor = false;
    AbstractMask* mask = nullptr;

    for ( Dictionary::iterator dit = ( *dd )->begin(); dit != ( *dd )->end(); ++dit )
    {

      if ( dit->first == names::anchor )
      {

        anchor_token = dit->second;
        has_anchor = true;
      }
      else
      {

        if ( mask )
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
      catch ( TypeMismatch& e )
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


void
NestModule::SetStatus_idFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );
  size_t node_id = getValue< long >( i->OStack.pick( 1 ) );

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
      kernel::manager< ConnectionManager >.set_synapse_status( con_id.get_source_node_id(),
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
      kernel::manager< ConnectionManager >.set_synapse_status( con_id.get_source_node_id(),
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

void
NestModule::GetStatus_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  NodeCollectionDatum nc = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  if ( not nc->valid() )
  {
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }

  size_t nc_size = nc->size();
  ArrayDatum result;

  result.reserve( nc_size );

  for ( NodeCollection::const_iterator it = nc->begin(); it < nc->end(); ++it )
  {
    size_t node_id = ( *it ).node_id;
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

  size_t node_id = getValue< long >( i->OStack.pick( 0 ) );
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

  DictionaryDatum result_dict = kernel::manager< ConnectionManager >.get_synapse_status( conn.get_source_node_id(),
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
    DictionaryDatum result_dict = kernel::manager< ConnectionManager >.get_synapse_status( con_id.get_source_node_id(),
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
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }

  DictionaryDatum dict = DictionaryDatum( new Dictionary );
  nc->get_metadata_status( dict );

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

void
NestModule::SetDefaults_l_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const std::string name = getValue< std::string >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  set_model_defaults( name, params );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::GetDefaults_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const std::string modelname = getValue< std::string >( i->OStack.pick( 0 ) );

  DictionaryDatum dict = get_model_defaults( modelname );

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

void
NestModule::Install_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const std::string modulename = getValue< std::string >( i->OStack.pick( 0 ) );

  kernel::manager< ModuleManager >.install( modulename );

  i->OStack.pop();
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

void
NestModule::RunFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const double time = i->OStack.top();

  run( time );

  i->OStack.pop();
  i->EStack.pop();
}


void
NestModule::PrepareFunction::execute( SLIInterpreter* i ) const
{
  prepare();
  i->EStack.pop();
}

void
NestModule::CleanupFunction::execute( SLIInterpreter* i ) const
{
  cleanup();
  i->EStack.pop();
}

void
NestModule::CopyModel_l_l_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  // fetch existing model name from stack
  const Name old_name = getValue< Name >( i->OStack.pick( 2 ) );
  const Name new_name = getValue< Name >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  kernel::manager< ModelManager >.copy_model( old_name, new_name, params );

  i->OStack.pop( 3 );
  i->EStack.pop();
}

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
  kernel::manager< SPManager >.disconnect( sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();
}

// Disconnect for arraydatum
void
NestModule::Disconnect_aFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const ArrayDatum conns = getValue< ArrayDatum >( i->OStack.pick( 0 ) );

  disconnect( conns );

  i->OStack.pop( 1 );
  i->EStack.pop();
}

// Connect for nodecollection nodecollection conn_spec syn_spec
// See lib/sli/nest-init.sli for details
void
NestModule::Connect_g_g_D_DFunction::execute( SLIInterpreter* i ) const
{
  kernel::manager< ConnectionManager >.sw_construction_connect.start();

  i->assert_stack_load( 4 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  // dictionary access checking is handled by connect
  kernel::manager< ConnectionManager >.connect( sources, targets, connectivity, { synapse_params } );

  i->OStack.pop( 4 );
  i->EStack.pop();

  kernel::manager< ConnectionManager >.sw_construction_connect.stop();
}

void
NestModule::Connect_g_g_D_aFunction::execute( SLIInterpreter* i ) const
{
  kernel::manager< ConnectionManager >.sw_construction_connect.start();

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
  kernel::manager< ConnectionManager >.connect( sources, targets, connectivity, synapse_params );

  i->OStack.pop( 4 );
  i->EStack.pop();

  kernel::manager< ConnectionManager >.sw_construction_connect.stop();
}


void
NestModule::ConnectTripartite_g_g_g_D_D_DFunction::execute( SLIInterpreter* i ) const
{
  kernel::manager< ConnectionManager >.sw_construction_connect.start();

  i->assert_stack_load( 6 );

  NodeCollectionDatum sources = getValue< NodeCollectionDatum >( i->OStack.pick( 5 ) );
  NodeCollectionDatum targets = getValue< NodeCollectionDatum >( i->OStack.pick( 4 ) );
  NodeCollectionDatum third = getValue< NodeCollectionDatum >( i->OStack.pick( 3 ) );
  DictionaryDatum connectivity = getValue< DictionaryDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum third_connectivity = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum synapse_specs_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  std::map< Name, std::vector< DictionaryDatum > > synapse_specs {
    { names::primary, {} }, { names::third_in, {} }, { names::third_out, {} }
  };

  for ( auto& [ key, syn_spec_array ] : synapse_specs )
  {
    ArrayDatum spec = getValue< ArrayDatum >( ( *synapse_specs_dict )[ key ] );

    for ( auto syn_param : spec )
    {
      syn_spec_array.push_back( getValue< DictionaryDatum >( syn_param ) );
    }
  }

  // dictionary access checking is handled by connect
  connect_tripartite( sources, targets, third, connectivity, third_connectivity, synapse_specs );

  i->OStack.pop( 6 );
  i->EStack.pop();

  kernel::manager< ConnectionManager >.sw_construction_connect.stop();
}


void
NestModule::ConnectSonata_D_Function::execute( SLIInterpreter* i ) const
{
  kernel::manager< ConnectionManager >.sw_construction_connect.start();

  i->assert_stack_load( 2 );

  DictionaryDatum graph_specs = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const long hyberslab_size = getValue< long >( i->OStack.pick( 0 ) );

  kernel::manager< ConnectionManager >.connect_sonata( graph_specs, hyberslab_size );

  i->OStack.pop( 2 );
  i->EStack.pop();

  kernel::manager< ConnectionManager >.sw_construction_connect.stop();
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
  kernel::manager< ModelManager >.memory_info();
  i->EStack.pop();
}

void
NestModule::PrintNodesFunction::execute( SLIInterpreter* i ) const
{
  print_nodes_to_stream();
  std::cout << std::endl;
  i->EStack.pop();
}

void
NestModule::PrintNodesToStreamFunction::execute( SLIInterpreter* i ) const
{
  std::stringstream out;
  print_nodes_to_stream( out );

  i->OStack.push( out.str() );
  i->EStack.pop();
}

void
NestModule::RankFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel::manager< MPIManager >.get_rank() );
  i->EStack.pop();
}

void
NestModule::NumProcessesFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel::manager< MPIManager >.get_num_processes() );
  i->EStack.pop();
}

void
NestModule::SyncProcessesFunction::execute( SLIInterpreter* i ) const
{
  kernel::manager< MPIManager >.synchronize();
  i->EStack.pop();
}

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
    time = kernel::manager< MPIManager >.time_communicate_offgrid( num_bytes, samples );
  }
  else
  {
    time = kernel::manager< MPIManager >.time_communicate( num_bytes, samples );
  }

  i->OStack.pop( 3 );
  i->OStack.push( time );
  i->EStack.pop();
}

void
NestModule::TimeCommunicationv_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel::manager< MPIManager >.time_communicatev( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

void
NestModule::TimeCommunicationAlltoall_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel::manager< MPIManager >.time_communicate_alltoall( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

void
NestModule::TimeCommunicationAlltoallv_i_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  long samples = getValue< long >( i->OStack.pick( 1 ) );
  long num_bytes = getValue< long >( i->OStack.pick( 0 ) );


  double time = 0.0;

  time = kernel::manager< MPIManager >.time_communicate_alltoallv( num_bytes, samples );

  i->OStack.pop( 2 );
  i->OStack.push( time );
  i->EStack.pop();
}

void
NestModule::ProcessorNameFunction::execute( SLIInterpreter* i ) const
{
  i->OStack.push( kernel::manager< MPIManager >.get_processor_name() );
  i->EStack.pop();
}

#ifdef HAVE_MPI
void
NestModule::MPIAbort_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  long exitcode = getValue< long >( i->OStack.pick( 0 ) );
  kernel::manager< MPIManager >.mpi_abort( exitcode );
  i->EStack.pop();
}
#endif

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
NestModule::Cva_g_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const std::string selection = getValue< std::string >( i->OStack.pick( 0 ) );
  NodeCollectionDatum nodecollection = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  ArrayDatum node_ids = nodecollection->to_array( selection );

  i->OStack.pop( 2 );
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

  const auto res = nodecollection->get_nc_index( node_id );
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

  size_t node_id = ( **it ).node_id;

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

  const size_t node_id = ( *nodecollection )[ idx ];

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

  // If start or stop are counted backwards from the end with negative keys, they must be adjusted.
  if ( start < 0 )
  {
    start += g_size;
    stop = stop == 0 ? g_size : stop;
  }

  if ( stop < 0 )
  {
    stop += g_size;
  }

  NodeCollectionDatum sliced_nc = nodecollection->slice( start, stop, step );

  i->OStack.pop( 2 );
  i->OStack.push( sliced_nc );
  i->EStack.pop();
}


#ifdef HAVE_MUSIC

void
NestModule::SetAcceptableLatencyFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::string port_name = getValue< std::string >( i->OStack.pick( 1 ) );
  double latency = getValue< double >( i->OStack.pick( 0 ) );

  kernel::manager< MUSICManager >.set_music_in_port_acceptable_latency( port_name, latency );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

void
NestModule::SetMaxBufferedFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::string port_name = getValue< std::string >( i->OStack.pick( 1 ) );
  int maxBuffered = getValue< long >( i->OStack.pick( 0 ) );

  kernel::manager< MUSICManager >.set_music_in_port_max_buffered( port_name, maxBuffered );

  i->OStack.pop( 2 );
  i->EStack.pop();
}
#endif


void
NestModule::EnableStructuralPlasticity_Function::execute( SLIInterpreter* i ) const
{
  kernel::manager< SPManager >.enable_structural_plasticity();

  i->EStack.pop();
}
void
NestModule::DisableStructuralPlasticity_Function::execute( SLIInterpreter* i ) const
{
  kernel::manager< SPManager >.disable_structural_plasticity();

  i->EStack.pop();
}

void
NestModule::SetStdpEps_dFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  const double stdp_eps = getValue< double >( i->OStack.top() );

  kernel::manager< ConnectionManager >.set_stdp_eps( stdp_eps );

  i->OStack.pop();
  i->EStack.pop();
}


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

void
NestModule::Apply_P_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  auto positions = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );
  auto param = getValue< ParameterDatum >( i->OStack.pick( 1 ) );

  // ADL requires explicit namespace qualification to avoid confusion with std::apply() in C++17
  // See https://github.com/llvm/llvm-project/issues/53084#issuecomment-1007969489
  auto result = nest::apply( param, positions );

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

  // ADL requires explicit namespace qualification to avoid confusion with std::apply() in C++17
  // See https://github.com/llvm/llvm-project/issues/53084#issuecomment-1007969489
  auto result = nest::apply( param, nc );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

#ifdef HAVE_LIBNEUROSIM

void
NestModule::CGParse_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = ConnectionGenerator::fromXML( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
}

void
NestModule::CGParseFile_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = ConnectionGenerator::fromXMLFile( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
}

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


void
NestModule::ConnectLayers_g_g_DFunction::execute( SLIInterpreter* i ) const
{
  kernel::manager< ConnectionManager >.sw_construction_connect.start();

  i->assert_stack_load( 3 );

  const NodeCollectionDatum source = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  const NodeCollectionDatum target = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const DictionaryDatum connection_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  connect_layers( source, target, connection_dict );

  i->OStack.pop( 3 );
  i->EStack.pop();

  kernel::manager< ConnectionManager >.sw_construction_connect.stop();
}

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

  std::vector< size_t > mask_node_ids;

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

    for ( Ntree< 2, size_t >::masked_iterator it = ml.begin( Position< 2 >( anchor[ 0 ], anchor[ 1 ] ) );
          it != ml.end();
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

    for ( Ntree< 3, size_t >::masked_iterator it = ml.begin( Position< 3 >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
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

  i->createcommand( "Install", &install_sfunction );

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
  i->createcommand( "ConnectSonata_D", &ConnectSonata_D_Function );
  i->createcommand( "ConnectTripartite_g_g_g_D_D_D", &connect_tripartite_g_g_g_D_D_Dfunction );

  i->createcommand( "ResetKernel", &resetkernelfunction );

  i->createcommand( "MemoryInfo", &memoryinfofunction );

  i->createcommand( "PrintNodes", &printnodesfunction );
  i->createcommand( "PrintNodesToStream", &printnodestostreamfunction );

  i->createcommand( "Rank", &rankfunction );
  i->createcommand( "NumProcesses", &numprocessesfunction );
  i->createcommand( "SyncProcesses", &syncprocessesfunction );
  i->createcommand( "TimeCommunication_i_i_b", &timecommunication_i_i_bfunction );
  i->createcommand( "TimeCommunicationv_i_i", &timecommunicationv_i_ifunction );
  i->createcommand( "TimeCommunicationAlltoall_i_i", &timecommunicationalltoall_i_ifunction );
  i->createcommand( "TimeCommunicationAlltoallv_i_i", &timecommunicationalltoallv_i_ifunction );
  i->createcommand( "ProcessorName", &processornamefunction );
#ifdef HAVE_MPI
  i->createcommand( "MPI_Abort", &mpiabort_ifunction );
#endif

  i->createcommand( "cvdict_C", &cvdict_Cfunction );

  i->createcommand( "cvnodecollection_i_i", &cvnodecollection_i_ifunction );
  i->createcommand( "cvnodecollection_ia", &cvnodecollection_iafunction );
  i->createcommand( "cvnodecollection_iv", &cvnodecollection_ivfunction );
  i->createcommand( "cva_g_l", &cva_g_lfunction );
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
  i->createcommand( "Disconnect_a", &disconnect_afunction );

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

  Token statusd = i->baselookup( Name( "statusdict" ) );
  DictionaryDatum dd = getValue< DictionaryDatum >( statusd );
  dd->insert( Name( "kernelname" ), new StringDatum( "NEST" ) );
  dd->insert( Name( "is_mpi" ), new BoolDatum( kernel::manager< MPIManager >.is_mpi_used() ) );

  register_parameter< ConstantParameter >( "constant" );
  register_parameter< UniformParameter >( "uniform" );
  register_parameter< UniformIntParameter >( "uniform_int" );
  register_parameter< NormalParameter >( "normal" );
  register_parameter< LognormalParameter >( "lognormal" );
  register_parameter< ExponentialParameter >( "exponential" );
  register_parameter< NodePosParameter >( "position" );
  register_parameter< SpatialDistanceParameter >( "distance" );
  register_parameter< GaussianParameter >( "gaussian" );
  register_parameter< Gaussian2DParameter >( "gaussian2d" );
  register_parameter< GammaParameter >( "gamma" );
  register_parameter< ExpDistParameter >( "exp_distribution" );
  register_parameter< GaborParameter >( "gabor" );

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

bool
NestModule::register_mask( const Name& name, MaskCreatorFunction creator )
{
  return mask_factory_().register_subtype( name, creator );
}

AbstractMask*
NestModule::create_mask( const Name& name, const DictionaryDatum& d )
{
  return mask_factory_().create( name, d );
}


} // namespace nest
