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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <limits>
#include <set>
#include <vector>

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
  , have_connections_changed_( true )
  , sort_connections_by_source_( true )
  , has_primary_connections_( false )
  , secondary_connections_exist_( false )
  , stdp_eps_( 1.0e-6 )
{
}

nest::ConnectionManager::~ConnectionManager()
{
  // Memory leak on purpose!
  // The ConnectionManager is deleted, when the network is deleted, and
  // this happens only, when main() is finished and we give the allocated memory
  // back to the system anyway. Hence, why bother cleaning up our highly
  // scattered connection infrastructure? They do not have any open files, which
  // need to be closed or similar.
}

void
nest::ConnectionManager::initialize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();
  connections_.resize( num_threads );
  secondary_recv_buffer_pos_.resize( num_threads );
  sort_connections_by_source_ = true;

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    connections_[ tid ] = std::vector< ConnectorBase* >(
      kernel().model_manager.get_num_synapse_prototypes() );
    secondary_recv_buffer_pos_[ tid ] = std::vector< std::vector< size_t > >();
  } // of omp parallel

  source_table_.initialize();
  target_table_.initialize();
  target_table_devices_.initialize();

  std::vector< DelayChecker > tmp( kernel().vp_manager.get_num_threads() );
  delay_checkers_.swap( tmp );

  std::vector< std::vector< size_t > > tmp2(
    kernel().vp_manager.get_num_threads(), std::vector< size_t >() );
  num_connections_.swap( tmp2 );

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
  delete_connections_();
  std::vector< std::vector< ConnectorBase* > >().swap( connections_ );
  std::vector< std::vector< std::vector< size_t > > >().swap(
    secondary_recv_buffer_pos_ );
}

void
nest::ConnectionManager::set_status( const DictionaryDatum& d )
{
  for ( size_t i = 0; i < delay_checkers_.size(); ++i )
  {
    delay_checkers_[ i ].set_status( d );
  }

  updateValue< bool >( d, names::keep_source_table, keep_source_table_ );
  if ( not keep_source_table_
    and kernel().sp_manager.is_structural_plasticity_enabled() )
  {
    throw KernelException(
      "If structural plasticity is enabled, keep_source_table can not be set "
      "to false." );
  }

  updateValue< bool >(
    d, names::sort_connections_by_source, sort_connections_by_source_ );
  if ( not sort_connections_by_source_
    and kernel().sp_manager.is_structural_plasticity_enabled() )
  {
    throw KernelException(
      "If structural plasticity is enabled, sort_connections_by_source can not "
      "be set to false." );
  }
  //  Need to update the saved values if we have changed the delay bounds.
  if ( d->known( names::min_delay ) or d->known( names::max_delay ) )
  {
    update_delay_extrema_();
  }
}

nest::DelayChecker&
nest::ConnectionManager::get_delay_checker()
{
  return delay_checkers_[ kernel().vp_manager.get_thread_id() ];
}

void
nest::ConnectionManager::get_status( DictionaryDatum& dict )
{
  update_delay_extrema_();
  def< double >(
    dict, names::min_delay, Time( Time::step( min_delay_ ) ).get_ms() );
  def< double >(
    dict, names::max_delay, Time( Time::step( max_delay_ ) ).get_ms() );

  const size_t n = get_num_connections();
  def< long >( dict, names::num_connections, n );
  def< bool >( dict, names::keep_source_table, keep_source_table_ );
  def< bool >(
    dict, names::sort_connections_by_source, sort_connections_by_source_ );
}

DictionaryDatum
nest::ConnectionManager::get_synapse_status( const index source_gid,
  const index target_gid,
  const thread tid,
  const synindex syn_id,
  const index lcid ) const
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );
  ( *dict )[ names::source ] = source_gid;
  ( *dict )[ names::synapse_model ] = LiteralDatum(
    kernel().model_manager.get_synapse_prototype( syn_id ).get_name() );

  const Node* source = kernel().node_manager.get_node( source_gid, tid );
  const Node* target = kernel().node_manager.get_node( target_gid, tid );

  // synapses from neurons to neurons and from neurons to globally
  // receiving devices
  if ( ( source->has_proxies() and target->has_proxies()
         and connections_[ tid ][ syn_id ] != NULL )
    or ( ( source->has_proxies() and not target->has_proxies()
         and not target->local_receiver()
         and connections_[ tid ][ syn_id ] != NULL ) ) )
  {
    connections_[ tid ][ syn_id ]->get_synapse_status( tid, lcid, dict );
  }
  else if ( source->has_proxies() and not target->has_proxies()
    and target->local_receiver() )
  {
    target_table_devices_.get_synapse_status_to_device(
      tid, source_gid, syn_id, dict, lcid );
  }
  else if ( not source->has_proxies() )
  {
    const index ldid = source->get_local_device_id();
    target_table_devices_.get_synapse_status_from_device(
      tid, ldid, syn_id, dict, lcid );
  }
  else
  {
    assert( false );
  }

  return dict;
}

void
nest::ConnectionManager::set_synapse_status( const index source_gid,
  const index target_gid,
  const thread tid,
  const synindex syn_id,
  const index lcid,
  const DictionaryDatum& dict )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  const Node* source = kernel().node_manager.get_node( source_gid, tid );
  const Node* target = kernel().node_manager.get_node( target_gid, tid );

  try
  {
    ConnectorModel& cm =
      kernel().model_manager.get_synapse_prototype( syn_id, tid );
    // synapses from neurons to neurons and from neurons to globally
    // receiving devices
    if ( ( source->has_proxies() and target->has_proxies()
           and connections_[ tid ][ syn_id ] != NULL )
      or ( ( source->has_proxies() and not target->has_proxies()
           and not target->local_receiver()
           and connections_[ tid ][ syn_id ] != NULL ) ) )
    {
      connections_[ tid ][ syn_id ]->set_synapse_status( lcid, dict, cm );
    }
    else if ( source->has_proxies() and not target->has_proxies()
      and target->local_receiver() )
    {
      target_table_devices_.set_synapse_status_to_device(
        tid, source_gid, syn_id, cm, dict, lcid );
    }
    else if ( not source->has_proxies() )
    {
      const index ldid = source->get_local_device_id();
      target_table_devices_.set_synapse_status_from_device(
        tid, ldid, syn_id, cm, dict, lcid );
    }
    else
    {
      assert( false );
    }
  }
  catch ( BadProperty& e )
  {
    throw BadProperty( String::compose(
      "Setting status of '%1' connecting from GID %2 to GID %3 via port %4: %5",
      kernel().model_manager.get_synapse_prototype( syn_id, tid ).get_name(),
      source_gid,
      target_gid,
      lcid,
      e.message() ) );
  }
}

void
nest::ConnectionManager::delete_connections_()
{
#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    for ( std::vector< ConnectorBase* >::iterator conn =
            connections_[ tid ].begin();
          conn != connections_[ tid ].end();
          ++conn )
    {
      delete *conn;
    }
  } // end omp parallel
}

const nest::Time
nest::ConnectionManager::get_min_delay_time_() const
{
  Time min_delay = Time::pos_inf();

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
  {
    min_delay = std::min( min_delay, it->get_min_delay() );
  }

  return min_delay;
}

const nest::Time
nest::ConnectionManager::get_max_delay_time_() const
{
  Time max_delay = Time::get_resolution();

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
  {
    max_delay = std::max( max_delay, it->get_max_delay() );
  }

  return max_delay;
}

bool
nest::ConnectionManager::get_user_set_delay_extrema() const
{
  bool user_set_delay_extrema = false;

  std::vector< DelayChecker >::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
  {
    user_set_delay_extrema |= it->get_user_set_delay_extrema();
  }

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

  if ( not conn_spec->known( names::rule ) )
  {
    throw BadProperty( "Connectivity spec must contain connectivity rule." );
  }
  const Name rule_name =
    static_cast< const std::string >( ( *conn_spec )[ names::rule ] );

  if ( not connruledict_->known( rule_name ) )
  {
    throw BadProperty(
      String::compose( "Unknown connectivity rule: %1", rule_name ) );
  }

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
  {
    min_delay_ = Time::get_resolution().get_steps();
  }
}

// gid node thread syn_id dict delay weight
void
nest::ConnectionManager::connect( const index sgid,
  Node* target,
  thread target_thread,
  const synindex syn_id,
  const DictionaryDatum& params,
  const double delay,
  const double weight )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  have_connections_changed_ = true;

  Node* const source = kernel().node_manager.get_node( sgid, target_thread );
  const thread tid = kernel().vp_manager.get_thread_id();

  // normal nodes and devices with proxies -> normal nodes and devices with
  // proxies
  if ( source->has_proxies() and target->has_proxies() )
  {
    connect_(
      *source, *target, sgid, target_thread, syn_id, params, delay, weight );
  }
  // normal nodes and devices with proxies -> normal devices
  else if ( source->has_proxies() and not target->has_proxies()
    and target->local_receiver() )
  {
    // Connections to nodes with one node per process (MUSIC proxies
    // or similar devices) have to be established by the thread of the
    // target if the source is on the local process even though the
    // source may be a proxy on target_thread.
    if ( target->one_node_per_process() and not source->is_proxy() )
    {
      connect_to_device_(
        *source, *target, sgid, target_thread, syn_id, params, delay, weight );
      return;
    }

    // make sure source is on this MPI rank and on this thread
    if ( source->is_proxy() or source->get_thread() != tid )
    {
      return;
    }

    connect_to_device_(
      *source, *target, sgid, target_thread, syn_id, params, delay, weight );
  }
  // normal devices -> normal nodes and devices with proxies
  else if ( not source->has_proxies() and target->has_proxies() )
  {
    connect_from_device_(
      *source, *target, target_thread, syn_id, params, delay, weight );
  }
  // normal devices -> normal devices
  else if ( not source->has_proxies() and not target->has_proxies() )
  {
    // create connection only on suggested thread of target
    const thread tid = kernel().vp_manager.get_thread_id();
    const thread suggested_thread = kernel().vp_manager.vp_to_thread(
      kernel().vp_manager.suggest_vp_for_gid( target->get_gid() ) );
    if ( suggested_thread == tid )
    {
      connect_from_device_(
        *source, *target, suggested_thread, syn_id, params, delay, weight );
    }
  }
  // globally receiving devices, e.g. volume transmitter
  else if ( not target->has_proxies() and not target->local_receiver() )
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return;
    }
    target = kernel().node_manager.get_node( target->get_gid(), tid );
    connect_( *source, *target, sgid, tid, syn_id, params, delay, weight );
  }
  else
  {
    assert( false );
  }
}

// gid gid dict syn_id
bool
nest::ConnectionManager::connect( const index sgid,
  const index tgid,
  const DictionaryDatum& params,
  const synindex syn_id )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  have_connections_changed_ = true;

  const thread tid = kernel().vp_manager.get_thread_id();

  if ( not kernel().node_manager.is_local_gid( tgid ) )
  {
    return false;
  }

  Node* target = kernel().node_manager.get_node( tgid, tid );
  const thread target_thread = target->get_thread();
  Node* source = kernel().node_manager.get_node( sgid, target_thread );

  // normal nodes and devices with proxies -> normal nodes and devices with
  // proxies
  if ( source->has_proxies() and target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn_id, params );
  }
  // normal nodes and devices with proxies -> normal devices
  else if ( source->has_proxies() and not target->has_proxies()
    and target->local_receiver() )
  {
    // Connections to nodes with one node per process (MUSIC proxies
    // or similar devices) have to be established by the thread of the
    // target if the source is on the local process even though the
    // source may be a proxy on target_thread.
    if ( target->one_node_per_process() and not source->is_proxy() )
    {
      connect_to_device_(
        *source, *target, sgid, target_thread, syn_id, params );
      return true;
    }

    // make sure source is on this MPI rank
    if ( source->is_proxy() or source->get_thread() != tid )
    {
      return false;
    }

    connect_to_device_( *source, *target, sgid, target_thread, syn_id, params );
  }
  // normal devices -> normal nodes and devices with proxies
  else if ( not source->has_proxies() and target->has_proxies() )
  {
    connect_from_device_( *source, *target, target_thread, syn_id, params );
  }
  // normal devices -> normal devices
  else if ( not source->has_proxies() and not target->has_proxies() )
  {
    // create connection only on suggested thread of target
    const thread suggested_thread = kernel().vp_manager.vp_to_thread(
      kernel().vp_manager.suggest_vp_for_gid( target->get_gid() ) );
    if ( suggested_thread == tid )
    {
      connect_from_device_(
        *source, *target, suggested_thread, syn_id, params );
    }
  }
  // globally receiving devices, e.g. volume transmitter
  else if ( not target->has_proxies() and not target->local_receiver() )
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return false;
    }
    target = kernel().node_manager.get_node( tgid, tid );
    connect_( *source, *target, sgid, tid, syn_id, params );
  }
  else
  {
    assert( false );
  }

  // We did not exit prematurely due to proxies, so we have connected.
  return true;
}

void
nest::ConnectionManager::connect_( Node& s,
  Node& r,
  const index s_gid,
  const thread tid,
  const synindex syn_id,
  const DictionaryDatum& params,
  const double delay,
  const double weight )
{
  const bool is_primary =
    kernel().model_manager.get_synapse_prototype( syn_id, tid ).is_primary();

  kernel()
    .model_manager.get_synapse_prototype( syn_id, tid )
    .add_connection( s, r, connections_[ tid ], syn_id, params, delay, weight );
  source_table_.add_source( tid, syn_id, s_gid, is_primary );

  increase_connection_count( tid, syn_id );

  if ( is_primary )
  {
    has_primary_connections_ = true;
  }
  else
  {
    secondary_connections_exist_ = true;
  }
}

void
nest::ConnectionManager::connect_to_device_( Node& s,
  Node& r,
  const index s_gid,
  const thread tid,
  const synindex syn_id,
  const DictionaryDatum& params,
  const double delay,
  const double weight )
{
  // create entries in connection structure for connections to devices
  target_table_devices_.add_connection_to_device(
    s, r, s_gid, tid, syn_id, params, delay, weight );

  increase_connection_count( tid, syn_id );
}

void
nest::ConnectionManager::connect_from_device_( Node& s,
  Node& r,
  const thread tid,
  const synindex syn_id,
  const DictionaryDatum& params,
  const double delay,
  const double weight )
{
  // create entries in connections vector of devices
  target_table_devices_.add_connection_from_device(
    s, r, tid, syn_id, params, delay, weight );

  increase_connection_count( tid, syn_id );
}

void
nest::ConnectionManager::increase_connection_count( const thread tid,
  const synindex syn_id )
{
  if ( num_connections_[ tid ].size() <= syn_id )
  {
    num_connections_[ tid ].resize( syn_id + 1 );
  }
  ++num_connections_[ tid ][ syn_id ];
}

nest::index
nest::ConnectionManager::find_connection( const thread tid,
  const synindex syn_id,
  const index sgid,
  const index tgid )
{
  // lcid will hold the position of the /first/ connection from node
  // sgid to any local node, or be invalid
  index lcid = source_table_.find_first_source( tid, syn_id, sgid );
  if ( lcid == invalid_index )
  {
    return invalid_index;
  }

  // lcid will hold the position of the /first/ connection from node
  // sgid to node tgid, or be invalid
  lcid = connections_[ tid ][ syn_id ]->find_first_target( tid, lcid, tgid );
  if ( lcid != invalid_index )
  {
    return lcid;
  }

  return lcid;
}

void
nest::ConnectionManager::disconnect( const thread tid,
  const synindex syn_id,
  const index sgid,
  const index tgid )
{
  have_connections_changed_ = true;

  assert( syn_id != invalid_synindex );

  const index lcid = find_connection( tid, syn_id, sgid, tgid );

  if ( lcid == invalid_index ) // this function should only be called
                               // with a valid connection
  {
    throw InexistentConnection();
  }

  connections_[ tid ][ syn_id ]->disable_connection( lcid );
  source_table_.disable_connection( tid, syn_id, lcid );

  --num_connections_[ tid ][ syn_id ];
}

void
nest::ConnectionManager::data_connect_single( const index source_id,
  DictionaryDatum params,
  const index syn_id )
{
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
  for ( di_s = ( *params ).begin(); di_s != ( *params ).end(); ++di_s )
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
      LOG( M_DEBUG, "DataConnect", msg );
      LOG( M_DEBUG, "DataConnect", "Trying to convert, but this takes time." );

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

  const Token target_t = params->lookup2( names::target );
  DoubleVectorDatum const* ptarget_ids =
    static_cast< DoubleVectorDatum* >( target_t.datum() );
  const std::vector< double >& target_ids( **ptarget_ids );

  // Only to check consistent
  const Token weight_t = params->lookup2( names::weight );
  DoubleVectorDatum const* pweights =
    static_cast< DoubleVectorDatum* >( weight_t.datum() );

  const Token delay_t = params->lookup2( names::delay );
  DoubleVectorDatum const* pdelays =
    static_cast< DoubleVectorDatum* >( delay_t.datum() );


  bool complete_wd_lists = ( ( *ptarget_ids )->size() == ( *pweights )->size()
    and ( *pweights )->size() == ( *pdelays )->size() );
  // check if we have consistent lists for weights and delays
  if ( not complete_wd_lists )
  {
    LOG( M_ERROR,
      "DataConnect",
      "All lists in the parameter dictionary must be of equal size." );
    throw DimensionMismatch();
  }

  Node* source = kernel().node_manager.get_node( source_id );

  Subnet* source_comp = dynamic_cast< Subnet* >( source );
  if ( source_comp != 0 )
  {
    LOG( M_INFO, "DataConnect", "Source ID is a subnet; I will iterate it." );

    // collect all leaves in source subnet, then data-connect each leaf
    LocalLeafList local_sources( *source_comp );
    std::vector< MPIManager::NodeAddressingData > global_sources;
    kernel().mpi_manager.communicate( local_sources, global_sources );
    for ( std::vector< MPIManager::NodeAddressingData >::iterator src =
            global_sources.begin();
          src != global_sources.end();
          ++src )
    {
      data_connect_single( src->get_gid(), params, syn_id );
    }

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
        if ( not e.message().empty() )
        {
          msg += "\nDetails: " + e.message();
        }
        LOG( M_WARNING, "DataConnect", msg.c_str() );
        continue;
      }

      if ( target->get_thread() != tid )
      {
        continue;
      }

      // here we fill a parameter dictionary with the values of the current loop
      // index.
      for ( di_s = ( *params ).begin(); di_s != ( *params ).end(); ++di_s )
      {
        DoubleVectorDatum const* tmp =
          static_cast< DoubleVectorDatum* >( di_s->second.datum() );
        const std::vector< double >& tmpvec = **tmp;
        par_i->insert( di_s->first, Token( new DoubleDatum( tmpvec[ i ] ) ) );
      }

      try
      {
        connect( source_id, target_ids[ i ], par_i, syn_id );
      }
      catch ( UnexpectedEvent& e )
      {
        std::string msg = String::compose(
          "Target with ID %1 does not support the connection. "
          "The connection will be ignored.",
          target_ids[ i ] );
        if ( not e.message().empty() )
        {
          msg += "\nDetails: " + e.message();
        }
        LOG( M_WARNING, "DataConnect", msg.c_str() );
        continue;
      }
      catch ( IllegalConnection& e )
      {
        std::string msg = String::compose(
          "Target with ID %1 does not support the connection. "
          "The connection will be ignored.",
          target_ids[ i ] );
        if ( not e.message().empty() )
        {
          msg += "\nDetails: " + e.message();
        }
        LOG( M_WARNING, "DataConnect", msg.c_str() );
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
        if ( not e.message().empty() )
        {
          msg += "\nDetails: " + e.message();
        }
        LOG( M_WARNING, "DataConnect", msg.c_str() );
        continue;
      }
    }
  }
}

bool
nest::ConnectionManager::data_connect_connectome( const ArrayDatum& connectome )
{
  for ( Token* ct = connectome.begin(); ct != connectome.end(); ++ct )
  {
    DictionaryDatum cd = getValue< DictionaryDatum >( *ct );
    index target_gid = static_cast< size_t >( ( *cd )[ names::target ] );
    Node* target_node = kernel().node_manager.get_node( target_gid );
    size_t thr = target_node->get_thread();

    size_t syn_id = 0;
    index source_gid = ( *cd )[ names::source ];

    Token synmodel = cd->lookup( names::synapse_model );
    if ( not synmodel.empty() )
    {
      std::string synmodel_name = getValue< std::string >( synmodel );
      synmodel =
        kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
      if ( not synmodel.empty() )
      {
        syn_id = static_cast< size_t >( synmodel );
      }
      else
      {
        throw UnknownModelName( synmodel_name );
      }
    }
    Node* source_node = kernel().node_manager.get_node( source_gid );
    connect_( *source_node, *target_node, source_gid, thr, syn_id, cd );
  }
  return true;
}


void
nest::ConnectionManager::trigger_update_weight( const long vt_id,
  const std::vector< spikecounter >& dopa_spikes,
  const double t_trig )
{
  const thread tid = kernel().vp_manager.get_thread_id();

  for (
    std::vector< ConnectorBase* >::iterator it = connections_[ tid ].begin();
    it != connections_[ tid ].end();
    ++it )
  {
    if ( *it != NULL )
    {
      ( *it )->trigger_update_weight( vt_id,
        tid,
        dopa_spikes,
        t_trig,
        kernel().model_manager.get_synapse_prototypes( tid ) );
    }
  }
}

size_t
nest::ConnectionManager::get_num_target_data( const thread tid ) const
{
  size_t num_connections = 0;
  for ( synindex syn_id = 0; syn_id < connections_[ tid ].size(); ++syn_id )
  {
    if ( connections_[ tid ][ syn_id ] != NULL )
    {
      num_connections += source_table_.num_unique_sources( tid, syn_id );
    }
  }
  return num_connections;
}

size_t
nest::ConnectionManager::get_num_connections() const
{
  size_t num_connections = 0;
  for ( index t = 0; t < num_connections_.size(); ++t )
  {
    for ( index s = 0; s < num_connections_[ t ].size(); ++s )
    {
      num_connections += num_connections_[ t ][ s ];
    }
  }

  return num_connections;
}

size_t
nest::ConnectionManager::get_num_connections( const synindex syn_id ) const
{
  size_t num_connections = 0;
  for ( index t = 0; t < num_connections_.size(); ++t )
  {
    if ( num_connections_[ t ].size() > syn_id )
    {
      num_connections += num_connections_[ t ][ syn_id ];
    }
  }

  return num_connections;
}

ArrayDatum
nest::ConnectionManager::get_connections( const DictionaryDatum& params ) const
{
  std::deque< ConnectionID > connectome;

  const Token& source_t = params->lookup( names::source );
  const Token& target_t = params->lookup( names::target );
  const Token& syn_model_t = params->lookup( names::synapse_model );
  const TokenArray* source_a = 0;
  const TokenArray* target_a = 0;
  long synapse_label = UNLABELED_CONNECTION;
  updateValue< long >( params, names::synapse_label, synapse_label );

  if ( not source_t.empty() )
  {
    source_a = dynamic_cast< TokenArray const* >( source_t.datum() );
  }
  if ( not target_t.empty() )
  {
    target_a = dynamic_cast< TokenArray const* >( target_t.datum() );
  }

  // If connections have changed, (re-)build presynaptic infrastructure,
  // as this may involve sorting connections by source gids.
  if ( have_connections_changed() )
  {
    if ( not kernel().simulation_manager.has_been_simulated() )
    {
      kernel().model_manager.create_secondary_events_prototypes();
    }
#pragma omp parallel
    {
      const thread tid = kernel().vp_manager.get_thread_id();
      kernel().simulation_manager.update_connection_infrastructure( tid );
    }
  }

  size_t syn_id = 0;

  // First we check, whether a synapse model is given.
  // If not, we will iterate all.
  if ( not syn_model_t.empty() )
  {
    Name synmodel_name = getValue< Name >( syn_model_t );
    const Token synmodel =
      kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
    if ( not synmodel.empty() )
    {
      syn_id = static_cast< size_t >( synmodel );
    }
    else
    {
      throw UnknownModelName( synmodel_name.toString() );
    }
    get_connections( connectome, source_a, target_a, syn_id, synapse_label );
  }
  else
  {
    for ( syn_id = 0;
          syn_id < kernel().model_manager.get_num_synapse_prototypes();
          ++syn_id )
    {
      get_connections( connectome, source_a, target_a, syn_id, synapse_label );
    }
  }

  ArrayDatum result;
  result.reserve( connectome.size() );

  while ( not connectome.empty() )
  {
    result.push_back( ConnectionDatum( connectome.front() ) );
    connectome.pop_front();
  }

  return result;
}

// Helper method which removes ConnectionIDs from input deque and
// appends them to output deque.
static inline std::deque< nest::ConnectionID >&
extend_connectome( std::deque< nest::ConnectionID >& out,
  std::deque< nest::ConnectionID >& in )
{
  while ( not in.empty() )
  {
    out.push_back( in.front() );
    in.pop_front();
  }

  return out;
}

void
nest::ConnectionManager::split_to_neuron_device_vectors_( const thread tid,
  TokenArray const* gid_token_array,
  std::vector< index >& neuron_gids,
  std::vector< index >& device_gids ) const
{
  for ( size_t t_id = 0; t_id < gid_token_array->size(); ++t_id )
  {
    const index gid = gid_token_array->get( t_id );
    if ( kernel().node_manager.get_node( gid, tid )->has_proxies() )
    {
      neuron_gids.push_back( gid );
    }
    else
    {
      device_gids.push_back( gid );
    }
  }
}

void
nest::ConnectionManager::get_connections(
  std::deque< ConnectionID >& connectome,
  TokenArray const* source,
  TokenArray const* target,
  synindex syn_id,
  long synapse_label ) const
{
  if ( is_source_table_cleared() )
  {
    throw KernelException(
      "Invalid attempt to access connection information: source table was "
      "cleared." );
  }

  const size_t num_connections = get_num_connections( syn_id );

  if ( num_connections == 0 )
  {
    return;
  }

  if ( source == 0 and target == 0 )
  {
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();

      std::deque< ConnectionID > conns_in_thread;

      ConnectorBase* connections = connections_[ tid ][ syn_id ];
      if ( connections != NULL )
      {
        // Passing target_gid = 0 ignores target_gid while getting connections.
        const size_t num_connections_in_thread = connections->size();
        for ( index lcid = 0; lcid < num_connections_in_thread; ++lcid )
        {
          const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
          connections->get_connection(
            source_gid, 0, tid, lcid, synapse_label, conns_in_thread );
        }
      }

      target_table_devices_.get_connections(
        0, 0, tid, syn_id, synapse_label, conns_in_thread );

      if ( conns_in_thread.size() > 0 )
      {
#pragma omp critical( get_connections )
        {
          extend_connectome( connectome, conns_in_thread );
        }
      }
    } // of omp parallel
    return;
  } // if
  else if ( source == 0 and target != 0 )
  {
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();

      std::deque< ConnectionID > conns_in_thread;

      // Split targets into neuron- and device-vectors.
      std::vector< index > target_neuron_gids;
      std::vector< index > target_device_gids;
      split_to_neuron_device_vectors_(
        tid, target, target_neuron_gids, target_device_gids );

      ConnectorBase* connections = connections_[ tid ][ syn_id ];
      if ( connections != NULL )
      {
        for ( std::vector< index >::const_iterator t_gid =
                target_neuron_gids.begin();
              t_gid != target_neuron_gids.end();
              ++t_gid )
        {
          std::vector< index > source_lcids;
          connections->get_source_lcids( tid, *t_gid, source_lcids );

          for ( size_t i = 0; i < source_lcids.size(); ++i )
          {
            conns_in_thread.push_back( ConnectionDatum( ConnectionID(
              source_table_.get_gid( tid, syn_id, source_lcids[ i ] ),
              *t_gid,
              tid,
              syn_id,
              source_lcids[ i ] ) ) );
          }
          // target_table_devices_ contains connections both to and from
          // devices. First we get connections from devices.
          target_table_devices_.get_connections_from_devices_(
            0, *t_gid, tid, syn_id, synapse_label, conns_in_thread );
        }
      }

      for (
        std::vector< index >::const_iterator t_gid = target_device_gids.begin();
        t_gid != target_device_gids.end();
        ++t_gid )
      {
        // Then, we get connections to devices.
        target_table_devices_.get_connections_to_devices_(
          0, *t_gid, tid, syn_id, synapse_label, conns_in_thread );
      }

      if ( conns_in_thread.size() > 0 )
      {
#pragma omp critical( get_connections )
        {
          extend_connectome( connectome, conns_in_thread );
        }
      }
    } // of omp parallel
    return;
  } // else if
  else if ( source != 0 )
  {
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();

      std::deque< ConnectionID > conns_in_thread;

      std::vector< index > sources;
      source->toVector( sources );
      std::sort( sources.begin(), sources.end() );

      // Split targets into neuron- and device-vectors.
      std::vector< index > target_neuron_gids;
      std::vector< index > target_device_gids;
      if ( target != 0 )
      {
        split_to_neuron_device_vectors_(
          tid, target, target_neuron_gids, target_device_gids );
      }

      const ConnectorBase* connections = connections_[ tid ][ syn_id ];
      if ( connections != NULL )
      {
        const size_t num_connections_in_thread = connections->size();
        for ( index lcid = 0; lcid < num_connections_in_thread; ++lcid )
        {
          const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
          if ( std::binary_search(
                 sources.begin(), sources.end(), source_gid ) )
          {
            if ( target == 0 )
            {
              // Passing target_gid = 0 ignores target_gid while getting
              // connections.
              connections->get_connection(
                source_gid, 0, tid, lcid, synapse_label, conns_in_thread );
            }
            else
            {
              for ( std::vector< index >::const_iterator t_gid =
                      target_neuron_gids.begin();
                    t_gid != target_neuron_gids.end();
                    ++t_gid )
              {
                connections->get_connection( source_gid,
                  *t_gid,
                  tid,
                  lcid,
                  synapse_label,
                  conns_in_thread );
              }
            }
          }
        }
      }

      for ( size_t s_id = 0; s_id < source->size(); ++s_id )
      {
        const index source_gid = source->get( s_id );
        if ( target == 0 )
        {
          target_table_devices_.get_connections(
            source_gid, 0, tid, syn_id, synapse_label, conns_in_thread );
        }
        else
        {
          for ( std::vector< index >::const_iterator t_gid =
                  target_neuron_gids.begin();
                t_gid != target_neuron_gids.end();
                ++t_gid )
          {
            // target_table_devices_ contains connections both to and from
            // devices. First we get connections from devices.
            target_table_devices_.get_connections_from_devices_(
              source_gid, *t_gid, tid, syn_id, synapse_label, conns_in_thread );
          }
          for ( std::vector< index >::const_iterator t_gid =
                  target_device_gids.begin();
                t_gid != target_device_gids.end();
                ++t_gid )
          {
            // Then, we get connections to devices.
            target_table_devices_.get_connections_to_devices_(
              source_gid, *t_gid, tid, syn_id, synapse_label, conns_in_thread );
          }
        }
      }

      if ( conns_in_thread.size() > 0 )
      {
#pragma omp critical( get_connections )
        {
          extend_connectome( connectome, conns_in_thread );
        }
      }
    } // of omp parallel
    return;
  } // else if
}

void
nest::ConnectionManager::get_source_gids_( const thread tid,
  const synindex syn_id,
  const index tgid,
  std::vector< index >& sources )
{
  std::vector< index > source_lcids;
  if ( connections_[ tid ][ syn_id ] != NULL )
  {
    connections_[ tid ][ syn_id ]->get_source_lcids( tid, tgid, source_lcids );
    source_table_.get_source_gids( tid, syn_id, source_lcids, sources );
  }
}

void
nest::ConnectionManager::get_sources( const std::vector< index >& targets,
  const index syn_id,
  std::vector< std::vector< index > >& sources )
{
  sources.resize( targets.size() );
  for ( std::vector< std::vector< index > >::iterator i = sources.begin();
        i != sources.end();
        ++i )
  {
    ( *i ).clear();
  }

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    for ( size_t i = 0; i < targets.size(); ++i )
    {
      get_source_gids_( tid, syn_id, targets[ i ], sources[ i ] );
    }
  }
}

void
nest::ConnectionManager::get_targets( const std::vector< index >& sources,
  const index syn_id,
  const std::string& post_synaptic_element,
  std::vector< std::vector< index > >& targets )
{
  targets.resize( sources.size() );
  for ( std::vector< std::vector< index > >::iterator i = targets.begin();
        i != targets.end();
        ++i )
  {
    ( *i ).clear();
  }

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    for ( size_t i = 0; i < sources.size(); ++i )
    {
      const index start_lcid =
        source_table_.find_first_source( tid, syn_id, sources[ i ] );
      if ( start_lcid != invalid_index )
      {
        connections_[ tid ][ syn_id ]->get_target_gids(
          tid, start_lcid, post_synaptic_element, targets[ i ] );
      }
    }
  }
}

void
nest::ConnectionManager::sort_connections( const thread tid )
{
  assert( not source_table_.is_cleared() );
  if ( sort_connections_by_source_ )
  {
    for ( synindex syn_id = 0; syn_id < connections_[ tid ].size(); ++syn_id )
    {
      if ( connections_[ tid ][ syn_id ] != NULL )
      {
        connections_[ tid ][ syn_id ]->sort_connections(
          source_table_.get_thread_local_sources( tid )[ syn_id ] );
      }
    }
    remove_disabled_connections( tid );
  }
}

void
nest::ConnectionManager::compute_target_data_buffer_size()
{
  // Determine number of target data on this rank. Since each thread
  // has its own data structures, we need to count connections on every
  // thread separately to compute the total number of sources.
  size_t num_target_data = 0;
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    num_target_data += get_num_target_data( tid );
  }

  // Determine maximum number of target data across all ranks, because
  // all ranks need identically sized buffers.
  std::vector< long > global_num_target_data(
    kernel().mpi_manager.get_num_processes() );
  global_num_target_data[ kernel().mpi_manager.get_rank() ] = num_target_data;
  kernel().mpi_manager.communicate( global_num_target_data );
  const size_t max_num_target_data =
    *std::max_element(
      global_num_target_data.begin(), global_num_target_data.end() );

  // MPI buffers should have at least two entries per process
  const size_t min_num_target_data =
    2 * kernel().mpi_manager.get_num_processes();

  // Adjust target data buffers accordingly
  if ( min_num_target_data < max_num_target_data )
  {
    kernel().mpi_manager.set_buffer_size_target_data( max_num_target_data );
  }
  else
  {
    kernel().mpi_manager.set_buffer_size_target_data( min_num_target_data );
  }
}

void
nest::ConnectionManager::compute_compressed_secondary_recv_buffer_positions(
  const thread tid )
{
#pragma omp single
  {
    buffer_pos_of_source_gid_syn_id_.clear();
  }

  source_table_.compute_buffer_pos_for_unique_secondary_sources(
    tid, buffer_pos_of_source_gid_syn_id_ );
  secondary_recv_buffer_pos_[ tid ].resize( connections_[ tid ].size() );

  const size_t chunk_size_secondary_events_in_int =
    kernel().mpi_manager.get_chunk_size_secondary_events_in_int();

  const synindex syn_id_end = connections_[ tid ].size();
  for ( synindex syn_id = 0; syn_id < syn_id_end; ++syn_id )
  {
    std::vector< size_t >& positions =
      secondary_recv_buffer_pos_[ tid ][ syn_id ];

    if ( connections_[ tid ][ syn_id ] != NULL )
    {
      if ( not kernel()
                 .model_manager.get_synapse_prototype( syn_id, tid )
                 .is_primary() )
      {
        positions.clear();
        const size_t lcid_end = get_num_connections_( tid, syn_id );
        positions.resize( lcid_end, 0 );

        // Compute and store the buffer position from which this connection
        // should read secondary events.
        for ( size_t lcid = 0; lcid < lcid_end; ++lcid )
        {
          const index source_gid = source_table_.get_gid( tid, syn_id, lcid );
          const index sg_s_id =
            source_table_.pack_source_gid_and_syn_id( source_gid, syn_id );
          const thread source_rank =
            kernel().mpi_manager.get_process_id_of_gid( source_gid );

          positions[ lcid ] = buffer_pos_of_source_gid_syn_id_[ sg_s_id ]
            + chunk_size_secondary_events_in_int * source_rank;
        }
      }
    }
  }
}

void
nest::ConnectionManager::set_stdp_eps( const double stdp_eps )
{
  if ( not( stdp_eps < Time::get_resolution().get_ms() ) )
  {
    throw KernelException(
      "The epsilon used for spike-time comparison in STDP must be less "
      "than the simulation resolution." );
  }
  else if ( stdp_eps < 0 )
  {
    throw KernelException(
      "The epsilon used for spike-time comparison in STDP must not be "
      "negative." );
  }
  else
  {
    stdp_eps_ = stdp_eps;

    std::ostringstream os;
    os << "Epsilon for spike-time comparison in STDP was set to "
       << std::setprecision( std::numeric_limits< long double >::digits10 )
       << stdp_eps_ << ".";

    LOG( M_INFO, "ConnectionManager::set_stdp_eps", os.str() );
  }
}

// recv_buffer can not be a const reference as iterators used in
// secondary events must not be const
bool
nest::ConnectionManager::deliver_secondary_events( const thread tid,
  const bool called_from_wfr_update,
  std::vector< unsigned int >& recv_buffer )
{
  const std::vector< ConnectorModel* >& cm =
    kernel().model_manager.get_synapse_prototypes( tid );
  const Time stamp =
    kernel().simulation_manager.get_slice_origin() + Time::step( 1 );
  const std::vector< std::vector< size_t > >& positions_tid =
    secondary_recv_buffer_pos_[ tid ];

  const synindex syn_id_end = positions_tid.size();
  for ( synindex syn_id = 0; syn_id < syn_id_end; ++syn_id )
  {
    if ( not called_from_wfr_update
      or kernel()
           .model_manager.get_synapse_prototypes( tid )[ syn_id ]
           ->supports_wfr() )
    {
      if ( positions_tid[ syn_id ].size() > 0 )
      {
        SecondaryEvent& prototype =
          kernel().model_manager.get_secondary_event_prototype( syn_id, tid );

        index lcid = 0;
        const size_t lcid_end = positions_tid[ syn_id ].size();
        while ( lcid < lcid_end )
        {
          std::vector< unsigned int >::iterator readpos =
            recv_buffer.begin() + positions_tid[ syn_id ][ lcid ];
          prototype << readpos;
          prototype.set_stamp( stamp );

          // send delivers event to all targets with the same source
          // and returns how many targets this event was delivered to
          lcid +=
            connections_[ tid ][ syn_id ]->send( tid, lcid, cm, prototype );
        }
      }
    }
  }

  // Read waveform relaxation done marker from last position in every
  // chunk
  bool done = true;
  const size_t chunk_size_in_int =
    kernel().mpi_manager.get_chunk_size_secondary_events_in_int();
  for ( thread rank = 0; rank < kernel().mpi_manager.get_num_processes();
        ++rank )
  {
    done = done and recv_buffer[ ( rank + 1 ) * chunk_size_in_int - 1 ];
  }
  return done;
}

void
nest::ConnectionManager::compress_secondary_send_buffer_pos( const thread tid )
{
  target_table_.compress_secondary_send_buffer_pos( tid );
}

void
nest::ConnectionManager::remove_disabled_connections( const thread tid )
{
  std::vector< ConnectorBase* >& connectors = connections_[ tid ];

  for ( synindex syn_id = 0; syn_id < connectors.size(); ++syn_id )
  {
    if ( connectors[ syn_id ] == NULL )
    {
      continue;
    }
    const index first_disabled_index =
      source_table_.remove_disabled_sources( tid, syn_id );

    if ( first_disabled_index != invalid_index )
    {
      connectors[ syn_id ]->remove_disabled_connections( first_disabled_index );
    }
  }
}

void
nest::ConnectionManager::resize_connections()
{
  kernel().vp_manager.assert_single_threaded();

  // Resize data structures for connections between neurons
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    connections_[ tid ].resize(
      kernel().model_manager.get_num_synapse_prototypes() );
    source_table_.resize_sources( tid );
  }

  // Resize data structures for connections between neurons and
  // devices
  target_table_devices_.resize_to_number_of_synapse_types();
}

void
nest::ConnectionManager::sync_has_primary_connections()
{
  has_primary_connections_ =
    kernel().mpi_manager.any_true( has_primary_connections_ );
}

void
nest::ConnectionManager::check_secondary_connections_exist()
{
  secondary_connections_exist_ =
    kernel().mpi_manager.any_true( secondary_connections_exist_ );
}
