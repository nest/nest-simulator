/*
 *  nest.cpp
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

#include "nest.h"

// C++ includes:
#include <cassert>

// Includes from nestkernel:
#include "connection_manager.h"
#include "exceptions.h"
#include "io_manager.h"
#include "kernel_manager.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "node_manager.h"
#include "parameter.h"
#include "random_manager.h"
#include "simulation_manager.h"
#include "sp_manager.h"

// Includes from sli:
#include "sliexceptions.h"
#include "token.h"

namespace nest
{

void
init_nest( int* argc, char** argv[] )
{
  kernel::manager< MPIManager >.init_mpi( argc, argv );
  kernel::manager< KernelManager >.initialize();
}

void
fail_exit( int )
{
}

void
install_module( const std::string& )
{
}

void
reset_kernel()
{
  kernel::manager< KernelManager >.reset();
}

void
register_logger_client( const deliver_logging_event_ptr client_callback )
{
  kernel::manager< LoggingManager >.register_logging_client( client_callback );
}

void
print_nodes_to_stream( std::ostream& ostr )
{
  kernel::manager< NodeManager >.print( ostr );
}

RngPtr
get_rank_synced_rng()
{
  return kernel::manager< RandomManager >.get_rank_synced_rng();
}

RngPtr
get_vp_synced_rng( size_t tid )
{
  return kernel::manager< RandomManager >.get_vp_synced_rng( tid );
}

RngPtr
get_vp_specific_rng( size_t tid )
{
  return kernel::manager< RandomManager >.get_vp_specific_rng( tid );
}

void
set_kernel_status( const DictionaryDatum& dict )
{
  dict->clear_access_flags();
  kernel::manager< KernelManager >.set_status( dict );
  ALL_ENTRIES_ACCESSED( *dict, "SetKernelStatus", "Unread dictionary entries: " );
}

DictionaryDatum
get_kernel_status()
{
  assert( kernel::manager< KernelManager >.is_initialized() );

  DictionaryDatum d( new Dictionary );
  kernel::manager< KernelManager >.get_status( d );

  return d;
}

void
set_node_status( const size_t node_id, const DictionaryDatum& dict )
{
  kernel::manager< NodeManager >.set_status( node_id, dict );
}

DictionaryDatum
get_node_status( const size_t node_id )
{
  return kernel::manager< NodeManager >.get_status( node_id );
}

void
set_connection_status( const ConnectionDatum& conn, const DictionaryDatum& dict )
{
  DictionaryDatum conn_dict = conn.get_dict();
  const size_t source_node_id = getValue< long >( conn_dict, nest::names::source );
  const size_t target_node_id = getValue< long >( conn_dict, nest::names::target );
  const size_t tid = getValue< long >( conn_dict, nest::names::target_thread );
  const synindex syn_id = getValue< long >( conn_dict, nest::names::synapse_modelid );
  const size_t p = getValue< long >( conn_dict, nest::names::port );

  dict->clear_access_flags();

  kernel::manager< ConnectionManager >.set_synapse_status( source_node_id, target_node_id, tid, syn_id, p, dict );

  ALL_ENTRIES_ACCESSED2( *dict,
    "SetStatus",
    "Unread dictionary entries: ",
    "Maybe you tried to set common synapse properties through an individual "
    "synapse?" );
}

DictionaryDatum
get_connection_status( const ConnectionDatum& conn )
{
  return kernel::manager< ConnectionManager >.get_synapse_status( conn.get_source_node_id(),
    conn.get_target_node_id(),
    conn.get_target_thread(),
    conn.get_synapse_model_id(),
    conn.get_port() );
}

NodeCollectionPTR
create( const Name& model_name, const size_t n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const size_t model_id = kernel::manager< ModelManager >.get_node_model_id( model_name );
  return kernel::manager< NodeManager >.add_node( model_id, n_nodes );
}

NodeCollectionPTR
get_nodes( const DictionaryDatum& params, const bool local_only )
{
  return kernel::manager< NodeManager >.get_nodes( params, local_only );
}

void
connect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const DictionaryDatum& connectivity,
  const std::vector< DictionaryDatum >& synapse_params )
{
  kernel::manager< ConnectionManager >.connect( sources, targets, connectivity, synapse_params );
}

void
connect_tripartite( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  NodeCollectionPTR third,
  const DictionaryDatum& connectivity,
  const DictionaryDatum& third_connectivity,
  const std::map< Name, std::vector< DictionaryDatum > >& synapse_specs )
{
  kernel::manager< ConnectionManager >.connect_tripartite(
    sources, targets, third, connectivity, third_connectivity, synapse_specs );
}

void
connect_arrays( long* sources,
  long* targets,
  double* weights,
  double* delays,
  std::vector< std::string >& p_keys,
  double* p_values,
  size_t n,
  std::string syn_model )
{
  kernel::manager< ConnectionManager >.connect_arrays(
    sources, targets, weights, delays, p_keys, p_values, n, syn_model );
}

ArrayDatum
get_connections( const DictionaryDatum& dict )
{
  dict->clear_access_flags();

  ArrayDatum array = kernel::manager< ConnectionManager >.get_connections( dict );

  ALL_ENTRIES_ACCESSED( *dict, "GetConnections", "Unread dictionary entries: " );

  return array;
}

void
disconnect( const ArrayDatum& conns )
{
  // probably not strictly necessary here, but does nothing if all is up to date
  kernel::manager< NodeManager >.update_thread_local_node_data();

  for ( size_t conn_index = 0; conn_index < conns.size(); ++conn_index )
  {
    const auto conn_datum = getValue< ConnectionDatum >( conns.get( conn_index ) );
    const auto target_node = kernel::manager< NodeManager >.get_node_or_proxy( conn_datum.get_target_node_id() );
    kernel::manager< SPManager >.disconnect(
      conn_datum.get_source_node_id(), target_node, conn_datum.get_target_thread(), conn_datum.get_synapse_model_id() );
  }
}

void
simulate( const double& t )
{
  prepare();
  run( t );
  cleanup();
}

void
run( const double& time )
{
  const Time t_sim = Time::ms( time );

  if ( time < 0 )
  {
    throw BadParameter( "The simulation time cannot be negative." );
  }
  if ( not t_sim.is_finite() )
  {
    throw BadParameter( "The simulation time must be finite." );
  }
  if ( not t_sim.is_grid_time() )
  {
    throw BadParameter(
      "The simulation time must be a multiple "
      "of the simulation resolution." );
  }

  kernel::manager< SimulationManager >.run( t_sim );
}

void
prepare()
{
  kernel::manager< KernelManager >.prepare();
}

void
cleanup()
{
  kernel::manager< KernelManager >.cleanup();
}

void
copy_model( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict )
{
  kernel::manager< ModelManager >.copy_model( oldmodname, newmodname, dict );
}

void
set_model_defaults( const std::string component, const DictionaryDatum& dict )
{
  if ( kernel::manager< ModelManager >.set_model_defaults( component, dict ) )
  {
    return;
  }

  if ( kernel::manager< IOManager >.is_valid_recording_backend( component ) )
  {
    kernel::manager< IOManager >.set_recording_backend_status( component, dict );
    return;
  }

  throw UnknownComponent( component );
}

DictionaryDatum
get_model_defaults( const std::string component )
{
  try
  {
    const size_t model_id = kernel::manager< ModelManager >.get_node_model_id( component );
    return kernel::manager< ModelManager >.get_node_model( model_id )->get_status();
  }
  catch ( UnknownModelName& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  try
  {
    const size_t synapse_model_id = kernel::manager< ModelManager >.get_synapse_model_id( component );
    return kernel::manager< ModelManager >.get_connector_defaults( synapse_model_id );
  }
  catch ( UnknownSynapseType& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  if ( kernel::manager< IOManager >.is_valid_recording_backend( component ) )
  {
    return kernel::manager< IOManager >.get_recording_backend_status( component );
  }

  throw UnknownComponent( component );
  return DictionaryDatum(); // supress missing return value warning; never reached
}

ParameterDatum
create_parameter( const DictionaryDatum& param_dict )
{
  param_dict->clear_access_flags();

  ParameterDatum datum( NestModule::create_parameter( param_dict ) );

  ALL_ENTRIES_ACCESSED( *param_dict, "nest::CreateParameter", "Unread dictionary entries: " );

  return datum;
}

double
get_value( const ParameterDatum& param )
{
  RngPtr rng = get_rank_synced_rng();
  return param->value( rng, nullptr );
}

bool
is_spatial( const ParameterDatum& param )
{
  return param->is_spatial();
}

std::vector< double >
apply( const ParameterDatum& param, const NodeCollectionDatum& nc )
{
  std::vector< double > result;
  result.reserve( nc->size() );
  RngPtr rng = get_rank_synced_rng();
  for ( auto it = nc->begin(); it < nc->end(); ++it )
  {
    auto node = kernel::manager< NodeManager >.get_node_or_proxy( ( *it ).node_id );
    result.push_back( param->value( rng, node ) );
  }
  return result;
}

std::vector< double >
apply( const ParameterDatum& param, const DictionaryDatum& positions )
{
  auto source_tkn = positions->lookup( names::source );
  auto source_nc = getValue< NodeCollectionPTR >( source_tkn );

  auto targets_tkn = positions->lookup( names::targets );
  TokenArray target_tkns = getValue< TokenArray >( targets_tkn );
  return param->apply( source_nc, target_tkns );
}

Datum*
node_collection_array_index( const Datum* datum, const long* array, unsigned long n )
{
  const NodeCollectionDatum node_collection = *dynamic_cast< const NodeCollectionDatum* >( datum );
  assert( node_collection->size() >= n );
  std::vector< size_t > node_ids;
  node_ids.reserve( n );

  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr )
  {
    node_ids.push_back( node_collection->operator[]( *node_ptr ) );
  }
  return new NodeCollectionDatum( NodeCollection::create( node_ids ) );
}

Datum*
node_collection_array_index( const Datum* datum, const bool* array, unsigned long n )
{
  const NodeCollectionDatum node_collection = *dynamic_cast< const NodeCollectionDatum* >( datum );
  assert( node_collection->size() == n );
  std::vector< size_t > node_ids;
  node_ids.reserve( n );

  auto nc_it = node_collection->begin();
  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr, ++nc_it )
  {
    if ( *node_ptr )
    {
      node_ids.push_back( ( *nc_it ).node_id );
    }
  }
  return new NodeCollectionDatum( NodeCollection::create( node_ids ) );
}

} // namespace nest
