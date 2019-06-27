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
#include "nodelist.h"
#include "subnet.h"

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
reset_network()
{
  kernel().simulation_manager.reset_network();
  LOG( M_INFO,
    "ResetNetworkFunction",
    "The network has been reset. Random generators and time have NOT been "
    "reset." );
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
print_network( index gid, index depth, std::ostream& )
{
  kernel().node_manager.print( gid, depth - 1 );
}

librandom::RngPtr
get_vp_rng_of_gid( index target )
{
  Node* target_node = kernel().node_manager.get_node( target );

  if ( not kernel().node_manager.is_local_node( target_node ) )
  {
    throw LocalNodeExpected( target );
  }

  // Only nodes with proxies have a well-defined VP and thus thread.
  // Asking for the VP of, e.g., a subnet or spike_detector is meaningless.
  if ( not target_node->has_proxies() )
  {
    throw NodeWithProxiesExpected( target );
  }

  return kernel().rng_manager.get_rng( target_node->get_thread() );
}

librandom::RngPtr
get_vp_rng( thread tid )
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( kernel().vp_manager.get_num_threads() ) );
  return kernel().rng_manager.get_rng( tid );
}

librandom::RngPtr
get_global_rng()
{
  return kernel().rng_manager.get_grng();
}

void
set_kernel_status( const DictionaryDatum& dict )
{
  dict->clear_access_flags();
  kernel().set_status( dict );
}

DictionaryDatum
get_kernel_status()
{
  assert( kernel().is_initialized() );

  Node* root = kernel().node_manager.get_root();
  assert( root != 0 );

  DictionaryDatum d = root->get_status_base();
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
  const index source_gid = getValue< long >( conn_dict, nest::names::source );
  const index target_gid = getValue< long >( conn_dict, nest::names::target );
  const thread tid = getValue< long >( conn_dict, nest::names::target_thread );
  const synindex syn_id = getValue< long >( conn_dict, nest::names::synapse_modelid );
  const port p = getValue< long >( conn_dict, nest::names::port );

  dict->clear_access_flags();

  kernel().connection_manager.set_synapse_status( source_gid, target_gid, tid, syn_id, p, dict );

  ALL_ENTRIES_ACCESSED2( *dict,
    "SetStatus",
    "Unread dictionary entries: ",
    "Maybe you tried to set common synapse properties through an individual "
    "synapse?" );
}

DictionaryDatum
get_connection_status( const ConnectionDatum& conn )
{
  return kernel().connection_manager.get_synapse_status( conn.get_source_gid(),
    conn.get_target_gid(),
    conn.get_target_thread(),
    conn.get_synapse_model_id(),
    conn.get_port() );
}

index
create( const Name& model_name, const index n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const Token model = kernel().model_manager.get_modeldict()->lookup( model_name );
  if ( model.empty() )
  {
    throw UnknownModelName( model_name );
  }

  // create
  const index model_id = static_cast< index >( model );

  return kernel().node_manager.add_node( model_id, n_nodes );
}

void
connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& connectivity,
  const DictionaryDatum& synapse_params )
{
  kernel().connection_manager.connect( sources, targets, connectivity, synapse_params );
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
set_model_defaults( const Name& modelname, const DictionaryDatum& dict )
{
  kernel().model_manager.set_model_defaults( modelname, dict );
}

DictionaryDatum
get_model_defaults( const Name& modelname )
{
  const Token nodemodel = kernel().model_manager.get_modeldict()->lookup( modelname );
  const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( modelname );

  DictionaryDatum dict;

  if ( not nodemodel.empty() )
  {
    const long model_id = static_cast< long >( nodemodel );
    Model* m = kernel().model_manager.get_model( model_id );
    dict = m->get_status();
  }
  else if ( not synmodel.empty() )
  {
    const long synapse_id = static_cast< long >( synmodel );
    dict = kernel().model_manager.get_connector_defaults( synapse_id );
  }
  else
  {
    throw UnknownModelName( modelname.toString() );
  }

  return dict;
}

void
change_subnet( const index node_gid )
{
  if ( kernel().node_manager.get_node( node_gid )->is_subnet() )
  {
    kernel().node_manager.go_to( node_gid );
  }
  else
  {
    throw SubnetExpected();
  }
}

index
current_subnet()
{
  assert( kernel().node_manager.get_cwn() != 0 );
  return kernel().node_manager.get_cwn()->get_gid();
}

ArrayDatum
get_nodes( const index node_id, const DictionaryDatum& params, const bool include_remotes, const bool return_gids_only )
{
  Subnet* subnet = dynamic_cast< Subnet* >( kernel().node_manager.get_node( node_id ) );
  if ( subnet == NULL )
  {
    throw SubnetExpected();
  }

  LocalNodeList localnodes( *subnet );
  std::vector< MPIManager::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, params, include_remotes );
  }

  ArrayDatum result;
  result.reserve( globalnodes.size() );
  for ( std::vector< MPIManager::NodeAddressingData >::iterator n = globalnodes.begin(); n != globalnodes.end(); ++n )
  {
    if ( return_gids_only )
    {
      result.push_back( new IntegerDatum( n->get_gid() ) );
    }
    else
    {
      DictionaryDatum* node_info = new DictionaryDatum( new Dictionary );
      ( **node_info )[ names::global_id ] = n->get_gid();
      ( **node_info )[ names::vp ] = n->get_vp();
      ( **node_info )[ names::parent ] = n->get_parent_gid();
      result.push_back( node_info );
    }
  }

  return result;
}

ArrayDatum
get_leaves( const index node_id, const DictionaryDatum& params, const bool include_remotes )
{
  Subnet* subnet = dynamic_cast< Subnet* >( kernel().node_manager.get_node( node_id ) );
  if ( subnet == NULL )
  {
    throw SubnetExpected();
  }

  LocalLeafList localnodes( *subnet );
  ArrayDatum result;

  std::vector< MPIManager::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, params, include_remotes );
  }
  result.reserve( globalnodes.size() );

  for ( std::vector< MPIManager::NodeAddressingData >::iterator n = globalnodes.begin(); n != globalnodes.end(); ++n )
  {
    result.push_back( new IntegerDatum( n->get_gid() ) );
  }

  return result;
}

ArrayDatum
get_children( const index node_id, const DictionaryDatum& params, const bool include_remotes )
{
  Subnet* subnet = dynamic_cast< Subnet* >( kernel().node_manager.get_node( node_id ) );
  if ( subnet == NULL )
  {
    throw SubnetExpected();
  }

  LocalChildList localnodes( *subnet );
  ArrayDatum result;

  std::vector< MPIManager::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    kernel().mpi_manager.communicate( localnodes, globalnodes, params, include_remotes );
  }
  result.reserve( globalnodes.size() );
  for ( std::vector< MPIManager::NodeAddressingData >::iterator n = globalnodes.begin(); n != globalnodes.end(); ++n )
  {
    result.push_back( new IntegerDatum( n->get_gid() ) );
  }

  return result;
}

void
restore_nodes( const ArrayDatum& node_list )
{
  kernel().node_manager.restore_nodes( node_list );
}

} // namespace nest
