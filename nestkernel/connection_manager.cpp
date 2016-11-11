/*
 *  connection_manager.cpp
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

#include "connection_manager.h"

// Generated includes:
#include "config.h"

// C++ includes:
#include <cassert>
#include <cmath>
#include <set>
#include <algorithm>

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_builder_factory.h"
#include "connection_label.h"
#include "connector_base.h"
#include "connector_model.h"
#include "delay_checker.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "nest_names.h"
#include "nest_types.h"
#include "node.h"
#include "nodelist.h"
#include "subnet.h"
#include "target_table_devices_impl.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"
#include "token.h"
#include "tokenutils.h"

nest::ConnectionManager::ConnectionManager()
  : connruledict_( new Dictionary() )
  , connbuilder_factories_()
  , min_delay_( 1 )
  , max_delay_( 1 )
  , keep_source_table_( true )
  , have_connections_changed_( false )
{
}

nest::ConnectionManager::~ConnectionManager()
{
  source_table_.finalize();
  delete_connections_5g_();
}

void
nest::ConnectionManager::initialize()
{
  thread num_threads = kernel().vp_manager.get_num_threads();
  connections_5g_.resize( num_threads, 0 );
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    connections_5g_[ tid ] = new HetConnector();
  }
  source_table_.initialize();
  target_table_.initialize();
  target_table_devices_.initialize();

  std::vector< DelayChecker > tmp2( kernel().vp_manager.get_num_threads() );
  delay_checkers_.swap( tmp2 );

  std::vector< std::vector< size_t > > tmp3( kernel().vp_manager.get_num_threads(), std::vector< size_t >() );
  vv_num_connections_.swap( tmp3 );

  // The following line is executed by all processes, no need to communicate
  // this change in delays.
  min_delay_ = max_delay_ = 1;

}

void
nest::ConnectionManager::finalize()
{
  source_table_.finalize();
  target_table_.finalize();
  target_table_devices_.finalize();
  delete_connections_5g_();
}

void
nest::ConnectionManager::set_status( const DictionaryDatum& d )
{
  for ( size_t i = 0; i < delay_checkers_.size(); ++i )
  {
    delay_checkers_[ i ].set_status( d );
  }

  updateValue< bool >( d, "keep_source_table", keep_source_table_ );
  if ( not keep_source_table_ && kernel().sp_manager.is_structural_plasticity_enabled() )
  {
    throw KernelException( "Structural plasticity can not be enabled if source table is not kept." );
  }
}

nest::DelayChecker&
nest::ConnectionManager::get_delay_checker()
{
  return delay_checkers_[ kernel().vp_manager.get_thread_id() ];
}

void
nest::ConnectionManager::get_status( DictionaryDatum& d )
{
  update_delay_extrema_();
  def< double >( d, "min_delay", Time( Time::step( min_delay_ ) ).get_ms() );
  def< double >( d, "max_delay", Time( Time::step( max_delay_ ) ).get_ms() );

  size_t n = get_num_connections();
  def< long >( d, "num_connections", n );
  def< bool >( d, "keep_source_table", keep_source_table_ );
}

DictionaryDatum
nest::ConnectionManager::get_synapse_status( const index source_gid,
  const index target_gid,
  const thread tid,
  const synindex syn_id,
  const port p ) const // TODO@5g: rename port -> lcid?
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );
  ( *dict )[ names::source ] = source_gid;
  ( *dict )[ names::synapse_model ] =
    LiteralDatum( kernel().model_manager.get_synapse_prototype( syn_id ).get_name() );

  const Node* source = kernel().node_manager.get_node( source_gid, tid );
  const Node* target = kernel().node_manager.get_node( target_gid, tid );

  if ( source->has_proxies() and target->has_proxies() )
  {
    // TODO@5g: get_synapse_neuron_to_neuron_status
    connections_5g_[ tid ]->get_synapse_status( syn_id, dict, p );
  }
  else if ( source->has_proxies() and not target->has_proxies() )
  {
    // TODO@5g: get_synapse_neuron_to_device_status
    target_table_devices_.get_synapse_status_to_device( tid, source_gid, syn_id, dict, p );
  }
  else if ( not source->has_proxies() )
  {
    // TODO@5g: get_synapse_from_device_status
    const index ldid = source->get_local_device_id();
    target_table_devices_.get_synapse_status_from_device( tid, ldid, syn_id, dict, p );
  }
  else
  {
    assert( false );
  }

  return dict;
}

void
nest::ConnectionManager::set_synapse_status(
  const index source_gid,
  const index target_gid,
  const thread tid,
  const synindex syn_id,
  const port p,
  const DictionaryDatum& dict )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  const Node* source = kernel().node_manager.get_node( source_gid, tid );
  const Node* target = kernel().node_manager.get_node( target_gid, tid );

  try
  {
    if ( source->has_proxies() and target->has_proxies() )
    {
      connections_5g_[ tid ]->set_synapse_status( syn_id, kernel().model_manager.get_synapse_prototype( syn_id, tid ), dict, p );
    }
    else if ( source->has_proxies() and not target->has_proxies() )
    {
      target_table_devices_.set_synapse_status_to_device( tid, source_gid, syn_id, kernel().model_manager.get_synapse_prototype( syn_id, tid ), dict, p );
    }
    else if ( not source->has_proxies() )
    {
      const index ldid = source->get_local_device_id();
      target_table_devices_.set_synapse_status_from_device( tid, ldid, syn_id, kernel().model_manager.get_synapse_prototype( syn_id, tid ), dict, p );
    }
    else
    {
      assert( false );
    }
  }
  catch ( BadProperty& e )
  {
    throw BadProperty(
      String::compose( "Setting status of '%1' connecting from GID %2 to GID %3 via port %4: %5",
        kernel().model_manager.get_synapse_prototype( syn_id, tid ).get_name(),
        source_gid,
        target_gid,
        p,
        e.message() ) );
  }
}

void
nest::ConnectionManager::delete_connections_5g_()
{
  for( std::vector< HetConnector* >::iterator it = connections_5g_.begin();
       it != connections_5g_.end(); ++it)
  {
    (*it)->~HetConnector();
  }
  connections_5g_.clear();
}

const nest::Time
nest::ConnectionManager::get_min_delay_time_() const
{
  Time min_delay = Time::pos_inf();

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    min_delay = std::min( min_delay, it->get_min_delay() );

  return min_delay;
}

const nest::Time
nest::ConnectionManager::get_max_delay_time_() const
{
  Time max_delay = Time::get_resolution();

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    max_delay = std::max( max_delay, it->get_max_delay() );

  return max_delay;
}

bool
nest::ConnectionManager::get_user_set_delay_extrema() const
{
  bool user_set_delay_extrema = false;

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    user_set_delay_extrema |= it->get_user_set_delay_extrema();

  return user_set_delay_extrema;
}

nest::ConnBuilder*
nest::ConnectionManager::get_conn_builder( const std::string& name,
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
  const size_t rule_id = connruledict_->lookup( name );
  return connbuilder_factories_.at( rule_id )->create(
    sources, targets, conn_spec, syn_spec );
}

void
nest::ConnectionManager::calibrate( const TimeConverter& tc )
{
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    delay_checkers_[ tid ].calibrate( tc );
  }
}

void
nest::ConnectionManager::connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
  have_connections_changed_ = true;

  conn_spec->clear_access_flags();
  syn_spec->clear_access_flags();

  if ( !conn_spec->known( names::rule ) )
    throw BadProperty( "Connectivity spec must contain connectivity rule." );
  const Name rule_name =
    static_cast< const std::string >( ( *conn_spec )[ names::rule ] );

  if ( !connruledict_->known( rule_name ) )
    throw BadProperty(
      String::compose( "Unknown connectivity rule: %1", rule_name ) );
  const long rule_id = ( *connruledict_ )[ rule_name ];

  ConnBuilder* cb = connbuilder_factories_.at( rule_id )->create(
    sources, targets, conn_spec, syn_spec );
  assert( cb != 0 );

  // at this point, all entries in conn_spec and syn_spec have been checked
  ALL_ENTRIES_ACCESSED(
    *conn_spec, "Connect", "Unread dictionary entries in conn_spec: " );
  ALL_ENTRIES_ACCESSED(
    *syn_spec, "Connect", "Unread dictionary entries in syn_spec: " );

  cb->connect();
  delete cb;
}

void
nest::ConnectionManager::update_delay_extrema_()
{
  min_delay_ = get_min_delay_time_().get_steps();
  max_delay_ = get_max_delay_time_().get_steps();

  if ( not get_user_set_delay_extrema() )
  {
    // If no min/max_delay is set explicitly (SetKernelStatus), then the default
    // delay used by the SPBuilders have to be respected for the min/max_delay.
    min_delay_ =
      std::min( min_delay_, kernel().sp_manager.builder_min_delay() );
    max_delay_ =
      std::max( max_delay_, kernel().sp_manager.builder_max_delay() );
  }

  if ( kernel().mpi_manager.get_num_processes() > 1 )
  {
    std::vector< delay > min_delays( kernel().mpi_manager.get_num_processes() );
    min_delays[ kernel().mpi_manager.get_rank() ] = min_delay_;
    kernel().mpi_manager.communicate( min_delays );
    min_delay_ = *std::min_element( min_delays.begin(), min_delays.end() );

    std::vector< delay > max_delays( kernel().mpi_manager.get_num_processes() );
    max_delays[ kernel().mpi_manager.get_rank() ] = max_delay_;
    kernel().mpi_manager.communicate( max_delays );
    max_delay_ = *std::max_element( max_delays.begin(), max_delays.end() );
  }

  if ( min_delay_ == Time::pos_inf().get_steps() )
    min_delay_ = Time::get_resolution().get_steps();
}

// gid node thread syn delay weight
void
nest::ConnectionManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  double_t d,
  double_t w )
{
  have_connections_changed_ = true;

  Node* const source = kernel().node_manager.get_node( sgid, target_thread );
  const thread tid = kernel().vp_manager.get_thread_id();

  // normal nodes and devices with proxies -> normal nodes and devices with proxies
  if ( source->has_proxies() && target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, d, w );
  }
  // normal nodes and devices with proxies -> normal devices
  else if ( source->has_proxies() && not target->has_proxies() )
  {
    if ( source->is_proxy() || source->get_thread() != tid)
    {
      return;
    }

    connect_to_device_( *source, *target, sgid, target_thread, syn, d, w );
  }
  // normal devices -> normal nodes and devices with proxies
  else if ( not source->has_proxies() && target->has_proxies() )
  {
    connect_from_device_( *source, *target, sgid, target_thread, syn, d, w );
  }
  // normal devices -> normal devices
  else if ( not source->has_proxies() && not target->has_proxies() )
  {
    // create connection only on suggested thread of target
    target_thread = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( target->get_gid() ) );
    if ( target_thread == tid )
    {
      connect_from_device_( *source, *target, sgid, target_thread, syn, d, w );
    }
  }
  // globally receiving devices
  // e.g., volume transmitter
  else if ( not target->has_proxies() && not target->local_receiver() )
  {
     // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return;
    }
    // TODO@5g: implement
    assert( false );
    // globally receiving devices iterate over all target threads
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread tid = 0; tid < n_threads; ++tid )
    {
      target = kernel().node_manager.get_node( target->get_gid(), tid );
      connect_( *source, *target, sgid, tid, syn, d, w );
    }
  }
  else
  {
    assert( false );
  }
}

// gid node thread syn dict delay weight
void
nest::ConnectionManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  DictionaryDatum& params,
  double_t d,
  double_t w )
{
  have_connections_changed_ = true;

  Node* const source = kernel().node_manager.get_node( sgid, target_thread );

  // normal nodes and devices with proxies -> normal nodes and devices with proxies
  if ( source->has_proxies() && target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  // normal nodes and devices with proxies -> normal devices
  else if ( source->has_proxies() && not target->has_proxies() )
  {
    if ( source->is_proxy() )
    {
      return;
    }

    if ( ( source->get_thread() != target_thread ) && ( source->has_proxies() ) )
    {
      target_thread = source->get_thread();
      target = kernel().node_manager.get_node( target->get_gid(), target_thread );
    }

    connect_to_device_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  // normal devices -> normal nodes and devices with proxies
  else if ( not source->has_proxies() && target->has_proxies() )
  {
    connect_from_device_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  // normal devices -> normal devices
  else if ( not source->has_proxies() && not target->has_proxies() )
  {
    // create connection only on suggested thread of target
    thread tid = kernel().vp_manager.get_thread_id();
    target_thread = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( target->get_gid() ) );
    if ( target_thread == tid )
    {
      connect_from_device_( *source, *target, sgid, target_thread, syn, params, d, w );
    }
  }
  // globally receiving devices
  // e.g., volume transmitter
  else if ( not target->has_proxies() && not target->local_receiver() )
  {
     // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return;
    }
    // TODO@5g: implement
    assert( false );
    // globally receiving devices iterate over all target threads
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread tid = 0; tid < n_threads; ++tid )
    {
      target = kernel().node_manager.get_node( target->get_gid(), tid );
      connect_to_device_( *source, *target, sgid, tid, syn, params, d, w );
    }
  }
  else
  {
    assert( false );
  }
}

// gid gid dict
bool
nest::ConnectionManager::connect( index sgid,
  index tgid,
  DictionaryDatum& params,
  index syn )
{
  have_connections_changed_ = true;

  thread tid = kernel().vp_manager.get_thread_id();

  if ( !kernel().node_manager.is_local_gid( tgid ) )
    return false;

  Node* target = kernel().node_manager.get_node( tgid, tid );

  thread target_thread = target->get_thread();

  Node* source = kernel().node_manager.get_node( sgid, target_thread );

  // normal nodes and devices with proxies -> normal nodes and devices with proxies
  if ( source->has_proxies() && target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, params );
  }
  // normal nodes and devices with proxies -> normal devices
  else if ( source->has_proxies() && not target->has_proxies() )
  {
    if ( source->is_proxy() )
    {
      return false;
    }

    if ( ( source->get_thread() != target_thread ) && ( source->has_proxies() ) )
    {
      target_thread = source->get_thread();
      target = kernel().node_manager.get_node( tgid, target_thread );
    }

    connect_to_device_( *source, *target, sgid, target_thread, syn, params );
  }
  // normal devices -> normal nodes and devices with proxies
  else if ( not source->has_proxies() && target->has_proxies() )
  {
    connect_from_device_( *source, *target, sgid, target_thread, syn, params );
  }
  // normal devices -> normal devices
  else if ( not source->has_proxies() && not target->has_proxies() )
  {
    // create connection only on suggested thread of target
    target_thread = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( target->get_gid() ) );
    if ( target_thread == tid )
    {
      connect_from_device_( *source, *target, sgid, target_thread, syn, params );
    }
  }
  // globally receiving devices
  // e.g., volume transmitter
  else if ( not target->has_proxies() && not target->local_receiver() )
  {
     // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return false;
    }
    // TODO@5g: implement
    assert( false );
    // globally receiving devices iterate over all target threads
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread tid = 0; tid < n_threads; ++tid )
    {
      target = kernel().node_manager.get_node( tgid, tid );
      connect_( *source, *target, sgid, tid, syn, params );
    }
  }
  else
  {
    assert( false );
  }

  // We did not exit prematurely due to proxies, so we have connected.
  return true;
}

/**
 * The parameters delay and weight have the default value NAN.
 */
void
nest::ConnectionManager::connect_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn );

  kernel().model_manager.get_synapse_prototype( syn, tid ).add_connection_5g(
    s, r, connections_5g_[ tid ], syn, d, w );
  source_table_.add_source( tid, syn, s_gid, kernel().model_manager.get_synapse_prototype( syn, tid ).is_primary() );

  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

void
nest::ConnectionManager::connect_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  DictionaryDatum& p,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn );

  kernel().model_manager.get_synapse_prototype( syn, tid ).add_connection_5g(
    s, r, connections_5g_[ tid ], syn, p, d, w );
  source_table_.add_source( tid, syn, s_gid, kernel().model_manager.get_synapse_prototype( syn, tid ).is_primary() );

  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

void
nest::ConnectionManager::connect_to_device_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn );

  // create entries in connection structure for connections to devices
  target_table_devices_.add_connection_to_device( s, r, s_gid, tid, syn, d, w );

  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

void
nest::ConnectionManager::connect_to_device_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  DictionaryDatum& p,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn );

  // create entries in connection structure for connections to devices
  target_table_devices_.add_connection_to_device( s, r, s_gid, tid, syn, p, d, w );

  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

void
nest::ConnectionManager::connect_from_device_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn ); // TODO@5g: move to connect(...)

  // create entries in connections vector of devices
  target_table_devices_.add_connection_from_device( s, r, s_gid, tid, syn, d, w );

  // TODO@5g: move to connect(...)
  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

void
nest::ConnectionManager::connect_from_device_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  DictionaryDatum& p,
  double_t d,
  double_t w )
{
  kernel().model_manager.assert_valid_syn_id( syn );

  // create entries in connections vector of devices
  target_table_devices_.add_connection_from_device( s, r, s_gid, tid, syn, p, d, w );

  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

// TODO@5g: implement
/**
 * Works in a similar way to connect, same logic but removes a connection.
 * @param target target node
 * @param sgid id of the source
 * @param target_thread thread of the target
 * @param syn_id type of synapse
 */
void
nest::ConnectionManager::disconnect( Node& target,
  index sgid,
  thread target_thread,
  index syn_id )
{
  assert( false );

  // if ( kernel().node_manager.is_local_gid( target.get_gid() ) )
  // {
  //   // get the ConnectorBase corresponding to the source
  //   ConnectorBase* conn = validate_pointer( validate_source_entry_( target_thread, sgid, syn_id ) );
  //   ConnectorBase* c = kernel()
  //                        .model_manager.get_synapse_prototype( syn_id, target_thread )
  //                        .delete_connection( target, target_thread, conn, syn_id );
  //   if ( c == 0 )
  //   {
  //     connections_[ target_thread ].erase( sgid );
  //   }
  //   else
  //   {
  //     connections_[ target_thread ].set( sgid, c );
  //   }
  //   --vv_num_connections_[ target_thread ][ syn_id ];
  // }
}

nest::index
nest::ConnectionManager::find_connection_sorted( const thread tid, const synindex syn_index, const index sgid, const index tgid )
{
  // lcid will hold the position of the /first/ connection from node
  // sgid to any local node or be invalid
  index lcid = source_table_.find_first_source( tid, sgid );
  if ( lcid == invalid_index )
  {
    return invalid_index;
  }

  // lcid will hold the position of the /first/ connection from node
  // sgid to node tgid or be invalid
  lcid = (*connections_5g_[ tid ]).find_first_target( tid, syn_index, lcid, tgid );
  if ( lcid != invalid_index )
  {
    return lcid;
  }

  return invalid_index;
}

// TODO@5g: remove?
// std::pair< nest::synindex, nest::index>
// nest::ConnectionManager::find_connection_sorted( const thread tid, const index sgid, const index tgid )
// {
//   for ( synindex syn_index = 0; syn_index < (*connections_5g_[ tid ]).size(); ++syn_index )
//   {
//     // lcid will hold the position of the /first/ connection from node
//     // sgid to any local node or be invalid
//     index lcid = source_table_.find_first_source( tid, sgid );
//     if ( lcid == invalid_index )
//     {
//       continue;
//     }

//     // lcid will hold the position of the /first/ connection from node
//     // sgid to node tgid or be invalid
//     lcid = (*connections_5g_[ tid ]).find_first_target( tid, syn_index, lcid, tgid );
//     if ( lcid != invalid_index )
//     {
//       return std::pair< synindex, index >( syn_index, lcid );
//     }
//   }

//   return std::pair< synindex, index >( invalid_synindex, invalid_index );
// }

nest::index
nest::ConnectionManager::find_connection_unsorted( const thread tid, const synindex syn_index, const index sgid, const index tgid )
{
  std::cout<<"searching unsorted"<<std::endl;
  std::vector< index > matching_lcids;

  source_table_.find_all_sources( tid, sgid, syn_index, matching_lcids );
  std::cout<<"size: "<<matching_lcids.size()<<std::endl;
  if ( matching_lcids.size() > 0 )
  {
    const index lcid = (*connections_5g_[ tid ]).find_matching_target( tid, tgid, syn_index, matching_lcids );
    if ( lcid != invalid_index )
    {
      std::cout<<lcid<<std::endl;
      return lcid;
    }
  }

  return invalid_index;
}

// std::pair< nest::synindex, nest::index >
// nest::ConnectionManager::find_connection_unsorted( const thread tid, const index sgid, const index tgid )
// {
//   std::cout<<"searching unsorted"<<std::endl;
//   std::vector< index > matching_lcids;
//   for ( synindex syn_index = 0; syn_index < (*connections_5g_[ tid ]).size(); ++syn_index )
//   {
//     matching_lcids.clear();
//     source_table_.find_all_sources( tid, sgid, syn_index, matching_lcids );
//     std::cout<<"size: "<<matching_lcids.size()<<std::endl;
//     if ( matching_lcids.size() > 0 )
//     {
//       const index lcid = (*connections_5g_[ tid ]).find_matching_target( tid, tgid, syn_index, matching_lcids );
//       if ( lcid != invalid_index )
//       {
//         std::cout<<lcid<<std::endl;
//         return std::pair< synindex, index >( syn_index, lcid );
//       }
//     }
//   }
//   return std::pair< synindex, index >( invalid_synindex, invalid_index );
// }

void
nest::ConnectionManager::disconnect_5g( const thread tid, const synindex syn_id, const index sgid, const index tgid )
{
  const synindex syn_index = (*connections_5g_[ tid ]).find_synapse_index( syn_id );
  assert( syn_index != invalid_synindex );

  std::cout<<"#################################################\n";
  std::cout<<"disconnect("<<sgid<<", "<<tgid<<")\n";

  index lcid = find_connection_sorted( tid, syn_index, sgid, tgid );
  if ( lcid == invalid_index )
  {
    lcid = find_connection_unsorted( tid, syn_index, sgid, tgid );
  }
  assert( lcid != invalid_index ); // this function should only be
                                   // called with a valid connection

  (*connections_5g_[ tid ]).disable_connection( syn_index, lcid );
  source_table_.disable_connection( tid, syn_index, lcid );

  std::cout<<"#################################################\n";
}

// TODO@5g: remove?
// void
// nest::ConnectionManager::disconnect_5g( const thread tid, const index sgid, const index tgid )
// {
//   std::cout<<"#################################################\n";
//   std::cout<<"disconnect("<<sgid<<", "<<tgid<<")\n";
//   std::pair< synindex, index > syn_index_lcid = find_connection_sorted( tid, sgid, tgid );
//   if ( syn_index_lcid.first == invalid_synindex || syn_index_lcid.second == invalid_index )
//   {
//     syn_index_lcid = find_connection_unsorted( tid, sgid, tgid );
//   }

//   if ( syn_index_lcid.first != invalid_synindex && syn_index_lcid.second != invalid_index )
//   {
//     (*connections_5g_[ tid ]).disable_connection( syn_index_lcid.first, syn_index_lcid.second );
//     source_table_.disable_connection( tid, syn_index_lcid.first, syn_index_lcid.second );
//   }
//   else
//   {
//     std::cout<<"connection not found :(\n";
//   }
//   std::cout<<"#################################################\n";

//   // TODO@5g
//   // assert( false ); // this should never be reached, since this
//                    // function should only be called with a valid
//                    // connection
// }

/**
 * Divergent connection routine for use by DataConnect.
 *
 * @note This method is used only by DataConnect.
 */
void
nest::ConnectionManager::divergent_connect( index source_id,
  DictionaryDatum pars,
  index syn )
{
  assert( false );
  // We extract the parameters from the dictionary explicitly since getValue()
  // for DoubleVectorDatum
  // copies the data into an array, from which the data must then be copied once
  // more.
  Dictionary::iterator di_s, di_t;

  // To save time, we first create the parameter dictionary for connect(), then
  // we copy
  // all keys from the original dictionary into the parameter dictionary.
  // We can the later use iterators to change the values inside the parameter
  // dictionary,
  // rather than using the lookup operator.
  // We also do the parameter checking here so that we can later use unsafe
  // operations.
  for ( di_s = ( *pars ).begin(); di_s != ( *pars ).end(); ++di_s )
  {
    DoubleVectorDatum const* tmp =
      dynamic_cast< DoubleVectorDatum* >( di_s->second.datum() );
    IntVectorDatum const* tmpint =
      dynamic_cast< IntVectorDatum* >( di_s->second.datum() );
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( di_s->second.datum() );
    if ( tmp == 0 )
    {

      std::string msg = String::compose(
        "Parameter '%1' must be a DoubleVectorArray or numpy.array. ",
        di_s->first.toString() );
      LOG( M_DEBUG, "DivergentConnect", msg );
      LOG( M_DEBUG,
        "DivergentConnect",
        "Trying to convert, but this takes time." );

      if ( tmpint )
      {
        std::vector< double >* data =
          new std::vector< double >( ( *tmpint )->begin(), ( *tmpint )->end() );
        DoubleVectorDatum* dvd = new DoubleVectorDatum( data );
        di_s->second = dvd;
      }
      else if ( ad )
      {
        std::vector< double >* data = new std::vector< double >;
        ad->toVector( *data );
        DoubleVectorDatum* dvd = new DoubleVectorDatum( data );
        di_s->second = dvd;
      }
      else
      {
        throw TypeMismatch( DoubleVectorDatum().gettypename().toString()
            + " or " + ArrayDatum().gettypename().toString(),
          di_s->second.datum()->gettypename().toString() );
      }
    }
  }

  const Token target_t = pars->lookup2( names::target );
  DoubleVectorDatum const* ptarget_ids =
    static_cast< DoubleVectorDatum* >( target_t.datum() );
  const std::vector< double >& target_ids( **ptarget_ids );

  // Only to check consistent
  const Token weight_t = pars->lookup2( names::weight );
  DoubleVectorDatum const* pweights =
    static_cast< DoubleVectorDatum* >( weight_t.datum() );

  const Token delay_t = pars->lookup2( names::delay );
  DoubleVectorDatum const* pdelays =
    static_cast< DoubleVectorDatum* >( delay_t.datum() );


  bool complete_wd_lists = ( ( *ptarget_ids )->size() == ( *pweights )->size()
    && ( *pweights )->size() == ( *pdelays )->size() );
  // check if we have consistent lists for weights and delays
  if ( !complete_wd_lists )
  {
    LOG( M_ERROR,
      "DivergentConnect",
      "All lists in the parameter dictionary must be of equal size." );
    throw DimensionMismatch();
  }

  Node* source = kernel().node_manager.get_node( source_id );

  Subnet* source_comp = dynamic_cast< Subnet* >( source );
  if ( source_comp != 0 )
  {
    LOG(
      M_INFO, "DivergentConnect", "Source ID is a subnet; I will iterate it." );

    // collect all leaves in source subnet, then divergent-connect each leaf
    LocalLeafList local_sources( *source_comp );
    std::vector< MPIManager::NodeAddressingData > global_sources;
    kernel().mpi_manager.communicate( local_sources, global_sources );
    for ( std::vector< MPIManager::NodeAddressingData >::iterator src =
            global_sources.begin();
          src != global_sources.end();
          ++src )
      divergent_connect( src->get_gid(), pars, syn );

    return;
  }

#pragma omp parallel private( di_s )
  {
    thread tid = kernel().vp_manager.get_thread_id();
    DictionaryDatum par_i( new Dictionary() );

    size_t n_targets = target_ids.size();
    for ( index i = 0; i < n_targets; ++i )
    {
      Node* target = 0;
      try
      {
        target = kernel().node_manager.get_node( target_ids[ i ], tid );
      }
      catch ( UnknownNode& e )
      {
        std::string msg = String::compose(
          "Target with ID %1 does not exist. "
          "The connection will be ignored.",
          target_ids[ i ] );
        if ( !e.message().empty() )
          msg += "\nDetails: " + e.message();
        LOG( M_WARNING, "DivergentConnect", msg.c_str() );
        continue;
      }

      if ( target->get_thread() != tid )
      {
        continue;
      }

      // here we fill a parameter dictionary with the values of the current loop
      // index.
      par_i->clear();
      for ( di_s = ( *pars ).begin(); di_s != ( *pars ).end(); ++di_s )
      {
        DoubleVectorDatum const* tmp =
          static_cast< DoubleVectorDatum* >( di_s->second.datum() );
        const std::vector< double >& tmpvec = **tmp;
        par_i->insert( di_s->first, Token( new DoubleDatum( tmpvec[ i ] ) ) );
      }

      try
      {
        connect( source_id, target_ids[ i ], par_i, syn );
      }
      catch ( UnexpectedEvent& e )
      {
        std::string msg = String::compose(
          "Target with ID %1 does not support the connection. "
          "The connection will be ignored.",
          target_ids[ i ] );
        if ( !e.message().empty() )
          msg += "\nDetails: " + e.message();
        LOG( M_WARNING, "DivergentConnect", msg.c_str() );
        continue;
      }
      catch ( IllegalConnection& e )
      {
        std::string msg = String::compose(
          "Target with ID %1 does not support the connection. "
          "The connection will be ignored.",
          target_ids[ i ] );
        if ( !e.message().empty() )
          msg += "\nDetails: " + e.message();
        LOG( M_WARNING, "DivergentConnect", msg.c_str() );
        continue;
      }
      catch ( UnknownReceptorType& e )
      {
        std::string msg = String::compose(
          "In Connection from global source ID %1 to target ID %2: "
          "Target does not support requested receptor type. "
          "The connection will be ignored",
          source_id,
          target_ids[ i ] );
        if ( !e.message().empty() )
          msg += "\nDetails: " + e.message();
        LOG( M_WARNING, "DivergentConnect", msg.c_str() );
        continue;
      }
    }
  }
}

/**
 * Connect, using a dictionary with arrays.
 * The connection rule is based on the details of the dictionary entries source
 * and target.
 * If source and target are both either a GID or a list of GIDs with equal size,
 * then source and target are connected one-to-one.
 * If source is a gid and target is a list of GIDs then the sources is
 * connected to all targets.
 * If source is a list of GIDs and target is a GID, then all sources are
 * connected to the target.
 * At this stage, the task of connect is to separate the dictionary into one
 * for each thread and then to forward the connect call to the connectors who
 * can then deal with the details of the connection.
 *
 * @note This method is used only by DataConnect.
 */
bool
nest::ConnectionManager::connect( ArrayDatum& conns )
{
  assert( false );
  for ( Token* ct = conns.begin(); ct != conns.end(); ++ct )
  {
    DictionaryDatum cd = getValue< DictionaryDatum >( *ct );
    index target_gid = static_cast< size_t >( ( *cd )[ names::target ] );
    Node* target_node = kernel().node_manager.get_node( target_gid );
    size_t thr = target_node->get_thread();

    size_t syn_id = 0;
    index source_gid = ( *cd )[ names::source ];

    Token synmodel = cd->lookup( names::synapse_model );
    if ( !synmodel.empty() )
    {
      std::string synmodel_name = getValue< std::string >( synmodel );
      synmodel =
        kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
      if ( !synmodel.empty() )
        syn_id = static_cast< size_t >( synmodel );
      else
        throw UnknownModelName( synmodel_name );
    }
    Node* source_node = kernel().node_manager.get_node( source_gid );
    connect_( *source_node, *target_node, source_gid, thr, syn_id, cd );
  }
  return true;
}



void
nest::ConnectionManager::trigger_update_weight( const long_t vt_id,
  const std::vector< spikecounter >& dopa_spikes,
  const double_t t_trig )
{
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    connections_5g_[ tid ]->trigger_update_weight(
      vt_id, tid, dopa_spikes, t_trig, kernel().model_manager.get_synapse_prototypes( tid ) );
  }
}

// TODO@5g: implement
void
nest::ConnectionManager::send( thread t, index sgid, Event& e )
{
  assert( false );
  // if ( sgid < connections_[ t ].size() ) // probably test only fails, if there are no connections
  // {
  //   ConnectorBase* p = connections_[ t ].get( sgid );
  //   if ( p != 0 ) // only send, if connections exist
  //   {
  //     // the two least significant bits of the pointer
  //     // contain the information, whether there are
  //     // primary and secondary connections behind
  //     if ( has_primary( p ) )
  //     {
  //       // erase 2 least significant bits to obtain the correct pointer
  //       validate_pointer( p )->send_to_all( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
  //     }
  //   }
  // }
}

// TODO@5g: implement
void
nest::ConnectionManager::send_secondary( thread t, SecondaryEvent& e )
{
  assert( false );
  // index sgid = e.get_sender_gid();

  // if ( sgid < connections_[ t ].size() ) // probably test only fails, if there are no connections
  // {
  //   ConnectorBase* p = connections_[ t ].get( sgid );
  //   if ( p != 0 ) // only send, if connections exist
  //   {
  //     if ( has_secondary( p ) )
  //     {
  //       // erase 2 least significant bits to obtain the correct pointer
  //       p = validate_pointer( p );

  //       if ( p->homogeneous_model() )
  //       {
  //         if ( e.supports_syn_id( p->get_syn_id() ) )
  //           p->send_to_all( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
  //       }
  //       else
  //         p->send_to_all_secondary( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
  //     }
  //   }
  // }
}

size_t
nest::ConnectionManager::get_num_connections() const
{
  size_t num_connections = 0;
  std::vector< DelayChecker >::const_iterator i;
  for ( index t = 0; t < vv_num_connections_.size(); ++t )
    for ( index s = 0; s < vv_num_connections_[ t ].size(); ++s )
      num_connections += vv_num_connections_[ t ][ s ];

  return num_connections;
}

size_t
nest::ConnectionManager::get_num_connections( synindex syn_id ) const
{
  size_t num_connections = 0;
  std::vector< DelayChecker >::const_iterator i;
  for ( index t = 0; t < vv_num_connections_.size(); ++t )
  {
    if ( vv_num_connections_[ t ].size() > syn_id )
    {
      num_connections += vv_num_connections_[ t ][ syn_id ];
    }
  }

  return num_connections;
}

ArrayDatum
nest::ConnectionManager::get_connections( DictionaryDatum params ) const
{
  ArrayDatum connectome;

  const Token& source_t = params->lookup( names::source );
  const Token& target_t = params->lookup( names::target );
  const Token& syn_model_t = params->lookup( names::synapse_model );
  const TokenArray* source_a = 0;
  const TokenArray* target_a = 0;
  long_t synapse_label = UNLABELED_CONNECTION;
  updateValue< long_t >( params, names::synapse_label, synapse_label );

  if ( not source_t.empty() )
  {
    source_a = dynamic_cast< TokenArray const* >( source_t.datum() );
  }
  if ( not target_t.empty() )
  {
    target_a = dynamic_cast< TokenArray const* >( target_t.datum() );
  }

  size_t syn_id = 0;

  // TODO@5g: why do we need to do this? can this be removed?
// #ifdef _OPENMP
//   std::string msg;
//   msg =
//     String::compose( "Setting OpenMP num_threads to %1.", kernel().vp_manager.get_num_threads() );
//   LOG( M_DEBUG, "ConnectionManager::get_connections", msg );
//   omp_set_num_threads( kernel().vp_manager.get_num_threads() );
// #endif

  // First we check, whether a synapse model is given.
  // If not, we will iterate all.
  if ( not syn_model_t.empty() )
  {
    Name synmodel_name = getValue< Name >( syn_model_t );
    const Token synmodel =
      kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
    if ( !synmodel.empty() )
      syn_id = static_cast< size_t >( synmodel );
    else
      throw UnknownModelName( synmodel_name.toString() );
    get_connections( connectome, source_a, target_a, syn_id, synapse_label );
  }
  else
  {
    for ( syn_id = 0;
          syn_id < kernel().model_manager.get_num_synapse_prototypes();
          ++syn_id )
    {
      ArrayDatum conn;
      get_connections( conn, source_a, target_a, syn_id, synapse_label );
      if ( conn.size() > 0 )
      {
        connectome.push_back( new ArrayDatum( conn ) );
      }
    }
  }

  return connectome;
}

void
nest::ConnectionManager::get_connections( ArrayDatum& connectome,
  TokenArray const* source,
  TokenArray const* target,
  synindex syn_id,
  long_t synapse_label ) const
{
  if ( is_source_table_cleared() )
  {
    throw KernelException( "Invalid attempt to access connection information: source table was cleared." );
  }

  const size_t num_connections = get_num_connections( syn_id );

  connectome.reserve( num_connections );
  if ( source == 0 and target == 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();
#else
    for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
#endif
      ArrayDatum conns_in_thread;

      // collect all connections between neurons
      const size_t num_connections_in_thread = connections_5g_[ tid ]->get_num_connections( syn_id );
      // TODO@5g: why do we need the critical construct?
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index lcid = 0; lcid < num_connections_in_thread; ++lcid )
      {
        const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
        connections_5g_[ tid ]->get_connection( source_gid, tid, syn_id, lcid, synapse_label, conns_in_thread );
      }

      target_table_devices_.get_connections( 0, 0, tid, syn_id, synapse_label, conns_in_thread );

      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        connectome.append_move( conns_in_thread );
      }
    } // of omp parallel
    return;
  } // if
  else if ( source == 0 and target != 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();
#else
    for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
#endif
      ArrayDatum conns_in_thread;

      // collect all connections between neurons
      const size_t num_connections_in_thread = connections_5g_[ tid ]->get_num_connections( syn_id );
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index lcid = 0; lcid < num_connections_in_thread; ++lcid )
      {
        const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
        for ( size_t t_id = 0; t_id < target->size(); ++t_id )
        {
          const index target_gid = target->get( t_id );
          connections_5g_[ tid ]->get_connection( source_gid, target_gid, tid, syn_id, lcid, synapse_label, conns_in_thread );
        }
      }

      for ( size_t t_id = 0; t_id < target->size(); ++t_id )
      {
          const index target_gid = target->get( t_id );
          target_table_devices_.get_connections( 0, target_gid, tid, syn_id, synapse_label, conns_in_thread );
      }

      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        connectome.append_move( conns_in_thread );
      }
    } // of omp parallel
    return;
  } // else if
  else if ( source != 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();
#else
    for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
#endif
      ArrayDatum conns_in_thread;

      // collect all connections between neurons
      const size_t num_connections_in_thread = connections_5g_[ tid ]->get_num_connections( syn_id );
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );

      // TODO@5g: this this too expensive? are there alternatives?
      std::vector< index > sources;
      source->toVector( sources );
      std::sort( sources.begin(), sources.end() );

      for ( index lcid = 0; lcid < num_connections_in_thread; ++lcid )
      {
        const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
        if ( std::binary_search( sources.begin(), sources.end(), source_gid ) )
        {
          if ( target == 0 )
          {
            connections_5g_[ tid ]->get_connection( source_gid, tid, syn_id, lcid, synapse_label, conns_in_thread );
          }
          else
          {
            for ( size_t t_id = 0; t_id < target->size(); ++t_id )
            {
              const index target_gid = target->get( t_id );
              connections_5g_[ tid ]->get_connection( source_gid, target_gid, tid, syn_id, lcid, synapse_label, conns_in_thread );
            }
          }
        }
      }

      for ( size_t s_id = 0; s_id < source->size(); ++s_id )
      {
        const index source_gid = source->get( s_id );
        if ( target == 0 )
        {
          target_table_devices_.get_connections( source_gid, 0, tid, syn_id, synapse_label, conns_in_thread );
        }
        else
        {
          for ( size_t t_id = 0; t_id < target->size(); ++t_id )
          {
            const index target_gid = target->get( t_id );
            target_table_devices_.get_connections( source_gid, target_gid, tid, syn_id, synapse_label, conns_in_thread );
          }
        }
      }

      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        connectome.append_move( conns_in_thread );
      }
    } // of omp parallel
    return;
  } // else if
}

void
nest::ConnectionManager::get_source_gids_( const thread tid, const synindex syn_index, const index tgid, std::vector< index >& sources )
{
  std::vector< index > source_lcids;
  (*connections_5g_[ tid ]).get_source_lcids( tid, syn_index, tgid, source_lcids );
  source_table_.get_source_gids( tid, syn_index, source_lcids, sources );
}

void
nest::ConnectionManager::get_sources( const std::vector< index >& targets,
  std::vector< std::vector< index > >& sources,
  const index syn_model )
{
  sources.resize( targets.size() );
  for ( std::vector< std::vector< index > >::iterator i = sources.begin();
        i != sources.end(); ++i )
  {
    ( *i ).clear();
  }

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    const synindex syn_index = (*connections_5g_[ tid ]).find_synapse_index( syn_model );
    if ( syn_index != invalid_index )
    {
      for ( size_t i = 0; i < targets.size(); ++i )
      {
        get_source_gids_( tid, syn_index, targets[ i ], sources[ i ] );
      }
    }
  }
}

void
nest::ConnectionManager::get_targets( const std::vector< index >& sources,
  std::vector< std::vector< index > >& targets,
  const index syn_model )
{
  targets.resize( sources.size() );
  for ( std::vector< std::vector< index > >::iterator i = targets.begin();
        i != targets.end(); ++i )
  {
    ( *i ).clear();
  }

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    const synindex syn_index = (*connections_5g_[ tid ]).find_synapse_index( syn_model );
    if ( syn_index != invalid_index )
    {
      for ( size_t i = 0; i < sources.size(); ++i )
      {
        const index start_lcid = source_table_.find_first_source( tid, sources[ i ] );
        if ( start_lcid != invalid_index )
        {
          (*connections_5g_[ tid ]).get_target_gids( tid, syn_index, start_lcid, targets[ i ] );
        }
      }
    }
  }
}

void
nest::ConnectionManager::sort_connections( const thread tid )
{
  assert( not source_table_.is_cleared() );
  (*connections_5g_[ tid ]).sort_connections( source_table_.get_thread_local_sources( tid ) );
  remove_disabled_connections( tid );
  source_table_.update_last_sorted_source( tid );
}

void
nest::ConnectionManager::reserve_connections( const thread tid, const synindex syn_id, const size_t count )
{
  kernel().model_manager.get_synapse_prototype( syn_id, tid ).reserve_connections( connections_5g_[ tid ], syn_id, count );
  source_table_.reserve( tid, syn_id, count );
}

void
nest::ConnectionManager::remove_disabled_connections( const thread tid )
{
  for ( synindex syn_index = 0; syn_index < (*connections_5g_[ tid ]).size(); ++syn_index )
  {
    const index first_disabled_index = source_table_.remove_disabled_sources( tid, syn_index );
    if ( first_disabled_index != invalid_index )
    {
      (*connections_5g_[ tid ]).remove_disabled_connections( syn_index, first_disabled_index );
    }
  }
}


