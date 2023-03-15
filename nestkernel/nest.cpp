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
#include "exceptions.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "parameter.h"

// Includes from sli:
#include "sliexceptions.h"
#include "token.h"

namespace nest
{

void
init_nest( int* argc, char** argv[] )
{
  KernelManager::create_kernel_manager();
  kernel().mpi_manager.init_mpi( argc, argv );
  kernel().initialize();
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
  kernel().reset();
}

void
enable_dryrun_mode( const index n_procs )
{
  kernel().mpi_manager.set_num_processes( n_procs );
}

void
register_logger_client( const deliver_logging_event_ptr client_callback )
{
  kernel().logging_manager.register_logging_client( client_callback );
}

void
print_nodes_to_stream( std::ostream& ostr )
{
  kernel().node_manager.print( ostr );
}

RngPtr
get_rank_synced_rng()
{
  return kernel().random_manager.get_rank_synced_rng();
}

RngPtr
get_vp_synced_rng( thread tid )
{
  return kernel().random_manager.get_vp_synced_rng( tid );
}

RngPtr
get_vp_specific_rng( thread tid )
{
  return kernel().random_manager.get_vp_specific_rng( tid );
}

void
set_kernel_status( const DictionaryDatum& dict )
{
  dict->clear_access_flags();
  kernel().set_status( dict );
  ALL_ENTRIES_ACCESSED( *dict, "SetKernelStatus", "Unread dictionary entries: " );
}

DictionaryDatum
get_kernel_status()
{
  assert( kernel().is_initialized() );

  DictionaryDatum d( new Dictionary );
  kernel().get_status( d );

  return d;
}

void
set_node_status( const index node_id, const DictionaryDatum& dict )
{
  kernel().node_manager.set_status( node_id, dict );
}

DictionaryDatum
get_node_status( const index node_id )
{
  return kernel().node_manager.get_status( node_id );
}

void
set_connection_status( const ConnectionDatum& conn, const DictionaryDatum& dict )
{
  DictionaryDatum conn_dict = conn.get_dict();
  const index source_node_id = getValue< long >( conn_dict, nest::names::source );
  const index target_node_id = getValue< long >( conn_dict, nest::names::target );
  const thread tid = getValue< long >( conn_dict, nest::names::target_thread );
  const synindex syn_id = getValue< long >( conn_dict, nest::names::synapse_modelid );
  const port p = getValue< long >( conn_dict, nest::names::port );

  dict->clear_access_flags();

  kernel().connection_manager.set_synapse_status( source_node_id, target_node_id, tid, syn_id, p, dict );

  ALL_ENTRIES_ACCESSED2( *dict,
    "SetStatus",
    "Unread dictionary entries: ",
    "Maybe you tried to set common synapse properties through an individual "
    "synapse?" );
}

DictionaryDatum
get_connection_status( const ConnectionDatum& conn )
{
  return kernel().connection_manager.get_synapse_status( conn.get_source_node_id(),
    conn.get_target_node_id(),
    conn.get_target_thread(),
    conn.get_synapse_model_id(),
    conn.get_port() );
}

NodeCollectionPTR
create( const Name& model_name, const index n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const index model_id = kernel().model_manager.get_node_model_id( model_name );
  return kernel().node_manager.add_node( model_id, n_nodes );
}

NodeCollectionPTR
get_nodes( const DictionaryDatum& params, const bool local_only )
{
  return kernel().node_manager.get_nodes( params, local_only );
}

void
connect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const DictionaryDatum& connectivity,
  const std::vector< DictionaryDatum >& synapse_params )
{
  kernel().connection_manager.connect( sources, targets, connectivity, synapse_params );
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
  kernel().connection_manager.connect_arrays( sources, targets, weights, delays, p_keys, p_values, n, syn_model );
}

ArrayDatum
get_connections( const DictionaryDatum& dict )
{
  dict->clear_access_flags();

  ArrayDatum array = kernel().connection_manager.get_connections( dict );

  ALL_ENTRIES_ACCESSED( *dict, "GetConnections", "Unread dictionary entries: " );

  return array;
}

void
disconnect( const ArrayDatum& conns )
{
  for ( size_t conn_index = 0; conn_index < conns.size(); ++conn_index )
  {
    const auto conn_datum = getValue< ConnectionDatum >( conns.get( conn_index ) );
    const auto target_node = kernel().node_manager.get_node_or_proxy( conn_datum.get_target_node_id() );
    kernel().sp_manager.disconnect(
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

  kernel().simulation_manager.run( t_sim );
}

void
prepare()
{
  kernel().prepare();
}

void
cleanup()
{
  kernel().cleanup();
}

void
copy_model( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict )
{
  kernel().model_manager.copy_model( oldmodname, newmodname, dict );
}

void
set_model_defaults( const std::string component, const DictionaryDatum& dict )
{
  if ( kernel().model_manager.set_model_defaults( component, dict ) )
  {
    return;
  }

  if ( kernel().io_manager.is_valid_recording_backend( component ) )
  {
    kernel().io_manager.set_recording_backend_status( component, dict );
    return;
  }

  throw UnknownComponent( component );
}

DictionaryDatum
get_model_defaults( const std::string component )
{
  try
  {
    const index model_id = kernel().model_manager.get_node_model_id( component );
    return kernel().model_manager.get_node_model( model_id )->get_status();
  }
  catch ( UnknownModelName& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  try
  {
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( component );
    return kernel().model_manager.get_connector_defaults( synapse_model_id );
  }
  catch ( UnknownSynapseType& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  if ( kernel().io_manager.is_valid_recording_backend( component ) )
  {
    return kernel().io_manager.get_recording_backend_status( component );
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
    auto node = kernel().node_manager.get_node_or_proxy( ( *it ).node_id );
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
  std::vector< index > node_ids;
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
  std::vector< index > node_ids;
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

void
slice_positions_if_sliced_nc( DictionaryDatum& dict, const NodeCollectionDatum& nc )
{
  // If metadata contains node positions and the NodeCollection is sliced, get only positions of the sliced nodes.
  if ( dict->known( names::positions ) )
  {
    const auto positions = getValue< TokenArray >( dict, names::positions );
    if ( nc->size() != positions.size() )
    {
      TokenArray sliced_points;
      // Iterate only local nodes
      NodeCollection::const_iterator nc_begin = nc->has_proxies() ? nc->MPI_local_begin() : nc->begin();
      NodeCollection::const_iterator nc_end = nc->end();
      for ( auto node = nc_begin; node < nc_end; ++node )
      {
        // Because the local ID also includes non-local nodes, it must be adapted to represent
        // the index for the local node position.
        const auto index =
          static_cast< size_t >( std::floor( ( *node ).lid / kernel().mpi_manager.get_num_processes() ) );
        sliced_points.push_back( positions[ index ] );
      }
      def2< TokenArray, ArrayDatum >( dict, names::positions, sliced_points );
    }
  }
}


} // namespace nest
