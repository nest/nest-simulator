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

#include <cassert>

#include "network.h"
#include "nodelist.h"
#include "subnet.h"
#include "kernel_manager.h"
#include "communicator.h"
#include "communicator_impl.h"
#include "token.h"
#include "sliexceptions.h"
#include "exceptions.h"

namespace nest
{

void
init_nest( int argc, char* argv[] )
{
}

void
fail_exit( int exitcode )
{
}

void
install_module( const std::string& module_name )
{
}

void
reset_kernel()
{
  Network::get_network().reset_kernel();
}

void
reset_network()
{
  kernel().simulation_manager.reset_network();
  LOG( M_INFO,
    "ResetNetworkFunction",
    "The network has been reset. Random generators and time have NOT been reset." );
}

void
enable_dryrun_mode( const index n_procs )
{
  Communicator::set_num_processes( n_procs );
}

void
register_logger_client( const deliver_logging_event_ptr client_callback )
{
  kernel().logging_manager.register_logging_client( client_callback );
}

void
print_network( index gid, index depth, std::ostream& out )
{
  Network::get_network().print( gid, depth - 1 );
}

librandom::RngPtr
get_vp_rng( index target )
{
  Node* target_node = Network::get_network().get_node( target );

  if ( !Network::get_network().is_local_node( target_node ) )
    throw LocalNodeExpected( target );

  // Only nodes with proxies have a well-defined VP and thus thread.
  // Asking for the VP of, e.g., a subnet or spike_detector is meaningless.
  if ( !target_node->has_proxies() )
    throw NodeWithProxiesExpected( target );

  return Network::get_network().get_rng( target_node->get_thread() );
}

librandom::RngPtr
get_global_rng()
{
  return Network::get_network().get_grng();
}

void
set_kernel_status( const DictionaryDatum& dict )
{
  Network::get_network().set_status( 0, dict );
}

DictionaryDatum
get_kernel_status()
{
  return Network::get_network().get_status( 0 );
}

void
set_node_status( const index node_id, const DictionaryDatum& dict )
{
  Network::get_network().set_status( node_id, dict );
}

DictionaryDatum
get_node_status( const index node_id )
{
  return Network::get_network().get_status( node_id );
}

void
set_connection_status( const ConnectionDatum& conn, const DictionaryDatum& dict )
{
  DictionaryDatum conn_dict = conn.get_dict();
  long synapse_id = getValue< long >( conn_dict, nest::names::synapse_modelid );
  long port = getValue< long >( conn_dict, nest::names::port );
  long gid = getValue< long >( conn_dict, nest::names::source );
  thread tid = getValue< long >( conn_dict, nest::names::target_thread );
  Network::get_network().get_node( gid ); // Just to check if the node exists

  dict->clear_access_flags();

  Network::get_network().set_synapse_status( gid, synapse_id, port, tid, dict );

  std::string missed;
  if ( !dict->all_accessed( missed ) )
  {
    if ( Network::get_network().dict_miss_is_error() )
    {
      throw UnaccessedDictionaryEntry(missed +
                                      "\nMaybe you tried to set common synapse properties through"
                                      " an individual synapse?");
    }
    else
    {
      LOG( M_WARNING, "SetStatus", ( "Unread dictionary entries: " + missed ).c_str() );
    }
  }
}

DictionaryDatum
get_connection_status( const ConnectionDatum& conn )
{
  long gid = conn.get_source_gid();
  Network::get_network().get_node( gid ); // Just to check if the node exists

  return Network::get_network().get_synapse_status(
    gid, conn.get_synapse_model_id(), conn.get_port(), conn.get_target_thread() );
}

index
create( const Name& model_name, const index n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const Token model = Network::get_network().get_modeldict().lookup( model_name );
  if ( model.empty() )
    throw UnknownModelName( model_name );

  // create
  const index model_id = static_cast< index >( model );

  return Network::get_network().add_node( model_id, n_nodes );
}

void
connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& connectivity,
  const DictionaryDatum& synapse_params )
{
  Network::get_network().connect( sources, targets, connectivity, synapse_params );
}

ArrayDatum
get_connections( const DictionaryDatum& dict )
{
  dict->clear_access_flags();

  ArrayDatum array = Network::get_network().get_connections( dict );

  std::string missed;
  if ( !dict->all_accessed( missed ) )
  {
    if ( Network::get_network().dict_miss_is_error() )
      throw UnaccessedDictionaryEntry( missed );
    else
      LOG( M_WARNING, "GetConnections", ( "Unread dictionary entries: " + missed ).c_str() );
  }

  return array;
}

void
simulate( const double_t& time )
{
  std::ostringstream os;
  os << "Simulating " << time << " ms.";
  LOG( M_INFO, "Simulate", os.str() );
  Time t = Time::ms( time );

  kernel().simulation_manager.simulate( t );
}

void
copy_model( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict )
{
  const Dictionary& modeldict = Network::get_network().get_modeldict();
  const Dictionary& synapsedict = Network::get_network().get_synapsedict();

  if ( modeldict.known( newmodname ) || synapsedict.known( newmodname ) )
    throw NewModelNameExists( newmodname );

  dict->clear_access_flags(); // set properties with access control
  const Token oldnodemodel = modeldict.lookup( oldmodname );
  const Token oldsynmodel = synapsedict.lookup( oldmodname );

  if ( !oldnodemodel.empty() )
  {
    const index old_id = static_cast< index >( oldnodemodel );
    const index new_id = Network::get_network().copy_model( old_id, newmodname.toString() );
    Network::get_network().get_model( new_id )->set_status( dict );
  }
  else if ( !oldsynmodel.empty() )
  {
    const index old_id = static_cast< index >( oldsynmodel );
    const index new_id =
      Network::get_network().copy_synapse_prototype( old_id, newmodname.toString() );
    Network::get_network().set_connector_defaults( new_id, dict );
  }
  else
    throw UnknownModelName( oldmodname );

  std::string missed;
  if ( !dict->all_accessed( missed ) )
  {
    if ( Network::get_network().dict_miss_is_error() )
    {
      throw UnaccessedDictionaryEntry( missed );
    }
    else
    {
      LOG( M_WARNING, "CopyModel", "Unread dictionary entries: " + missed );
    }
  }
}

void
set_model_defaults( const Name& modelname, const DictionaryDatum& dict )
{
  const Token nodemodel = Network::get_network().get_modeldict().lookup( modelname );
  const Token synmodel = Network::get_network().get_synapsedict().lookup( modelname );

  dict->clear_access_flags(); // set properties with access control

  if ( !nodemodel.empty() )
  {
    const index model_id = static_cast< index >( nodemodel );
    Network::get_network().get_model( model_id )->set_status( dict );
    Network::get_network().set_model_defaults_modified();
  }
  else if ( !synmodel.empty() )
  {
    const index synapse_id = static_cast< index >( synmodel );
    Network::get_network().set_connector_defaults( synapse_id, dict );
    Network::get_network().set_model_defaults_modified();
  }
  else
  {
    throw UnknownModelName( modelname.toString() );
  }


  std::string missed;
  if ( !dict->all_accessed( missed ) )
  {

    if ( Network::get_network().dict_miss_is_error() )
    {

      throw UnaccessedDictionaryEntry( missed );
    }
    else
    {
      LOG( M_WARNING, "SetDefaults", ( "Unread dictionary entries: " + missed ).c_str() );
    }
  }
}

DictionaryDatum
get_model_defaults( const Name& modelname )
{
  const Token nodemodel = Network::get_network().get_modeldict().lookup( modelname );
  const Token synmodel = Network::get_network().get_synapsedict().lookup( modelname );

  DictionaryDatum dict;

  if ( !nodemodel.empty() )
  {
    const long model_id = static_cast< long >( nodemodel );
    Model* m = Network::get_network().get_model( model_id );
    dict = m->get_status();
  }
  else if ( !synmodel.empty() )
  {
    const long synapse_id = static_cast< long >( synmodel );
    dict = Network::get_network().get_connector_defaults( synapse_id );
  }
  else
  {
    throw UnknownModelName( modelname.toString() );
  }

  return dict;
}

void
set_num_rec_processes( const index n_rec_procs )
{
  Network::get_network().set_num_rec_processes( n_rec_procs, false );
}

void
change_subnet( const index node_gid )
{
  if ( Network::get_network().get_node( node_gid )->is_subnet() )
  {
    Network::get_network().go_to( node_gid );
  }
  else
  {
    throw SubnetExpected();
  }
}

index
current_subnet()
{
  assert( Network::get_network().get_cwn() != 0 );
  return Network::get_network().get_cwn()->get_gid();
}

ArrayDatum
get_nodes( const index node_id,
  const DictionaryDatum& params,
  const bool include_remotes,
  const bool return_gids_only )
{
  Subnet* subnet = dynamic_cast< Subnet* >( Network::get_network().get_node( node_id ) );
  if ( subnet == NULL )
    throw SubnetExpected();

  LocalNodeList localnodes( *subnet );
  vector< Communicator::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    Communicator::communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    Communicator::communicate( localnodes, globalnodes, params, include_remotes );
  }

  ArrayDatum result;
  result.reserve( globalnodes.size() );
  for ( vector< Communicator::NodeAddressingData >::iterator n = globalnodes.begin();
        n != globalnodes.end();
        ++n )
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
  Subnet* subnet = dynamic_cast< Subnet* >( Network::get_network().get_node( node_id ) );
  if ( subnet == NULL )
  {
    throw SubnetExpected();
  }

  LocalLeafList localnodes( *subnet );
  ArrayDatum result;

  vector< Communicator::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    nest::Communicator::communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    nest::Communicator::communicate( localnodes, globalnodes, params, include_remotes );
  }
  result.reserve( globalnodes.size() );

  for ( vector< Communicator::NodeAddressingData >::iterator n = globalnodes.begin();
        n != globalnodes.end();
        ++n )
  {
    result.push_back( new IntegerDatum( n->get_gid() ) );
  }

  return result;
}

ArrayDatum
get_children( const index node_id, const DictionaryDatum& params, const bool include_remotes )
{
  Subnet* subnet = dynamic_cast< Subnet* >( Network::get_network().get_node( node_id ) );
  if ( subnet == NULL )
  {
    throw SubnetExpected();
  }

  LocalChildList localnodes( *subnet );
  ArrayDatum result;

  vector< Communicator::NodeAddressingData > globalnodes;
  if ( params->empty() )
  {
    nest::Communicator::communicate( localnodes, globalnodes, include_remotes );
  }
  else
  {
    nest::Communicator::communicate( localnodes, globalnodes, params, include_remotes );
  }
  result.reserve( globalnodes.size() );
  for ( vector< Communicator::NodeAddressingData >::iterator n = globalnodes.begin();
        n != globalnodes.end();
        ++n )
  {
    result.push_back( new IntegerDatum( n->get_gid() ) );
  }

  return result;
}

void
restore_nodes( const ArrayDatum& node_list )
{
  Network::get_network().restore_nodes( node_list );
}

} // namespace nest
