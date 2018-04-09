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
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"
#include "token.h"
#include "tokenutils.h"

#ifdef USE_PMA
#include "allocator.h"
#ifdef IS_K
extern PaddedPMA poormansallocpool[];
#else // not IS_K
extern PoorMansAllocator poormansallocpool;
#ifdef _OPENMP
#pragma omp threadprivate( poormansallocpool )
#endif // _OPENMP
#endif // IS_K
#endif // USE_PMA

nest::ConnectionManager::ConnectionManager()
  : connruledict_( new Dictionary() )
  , connbuilder_factories_()
  , min_delay_( 1 )
  , max_delay_( 1 )
  , initial_connector_capacity_( CONFIG_CONNECTOR_CUTOFF )
  , large_connector_limit_( CONFIG_CONNECTOR_CUTOFF * 2 )
  , large_connector_growth_factor_( 1.5 )
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
  tVSConnector tmp( kernel().vp_manager.get_num_threads(), tSConnector() );
  connections_.swap( tmp );

  tVDelayChecker tmp2( kernel().vp_manager.get_num_threads() );
  delay_checkers_.swap( tmp2 );

  tVVCounter tmp3( kernel().vp_manager.get_num_threads(), tVCounter() );
  vv_num_connections_.swap( tmp3 );

  // The following line is executed by all processes, no need to communicate
  // this change in delays.
  min_delay_ = max_delay_ = 1;

#ifdef _OPENMP
#ifdef USE_PMA
// initialize the memory pools
#ifdef IS_K
  const thread n_threads = kernel().vp_manager.get_num_threads();
  assert( n_threads <= MAX_THREAD
    && "MAX_THREAD is a constant defined in allocator.h" );

#pragma omp parallel
  poormansallocpool[ kernel().vp_manager.get_thread_id() ].init();
#else
#pragma omp parallel
  poormansallocpool.init();
#endif
#endif
#endif
}

void
nest::ConnectionManager::finalize()
{
  delete_connections_();
}

void
nest::ConnectionManager::set_status( const DictionaryDatum& d )
{
  long initial_connector_capacity = initial_connector_capacity_;
  if ( updateValue< long >(
         d, names::initial_connector_capacity, initial_connector_capacity ) )
  {
    if ( initial_connector_capacity < CONFIG_CONNECTOR_CUTOFF )
    {
      throw KernelException(
        "The initial connector capacity should be higher or equal to "
        "connector_cutoff value specified via cmake flag [default 3]" );
    }

    initial_connector_capacity_ = initial_connector_capacity;
  }

  long large_connector_limit = large_connector_limit_;
  if ( updateValue< long >(
         d, names::large_connector_limit, large_connector_limit ) )
  {
    if ( large_connector_limit < CONFIG_CONNECTOR_CUTOFF )
    {
      throw KernelException(
        "The large connector limit should be higher or equal to "
        "connector_cutoff value specified via cmake flag [default 3]" );
    }

    large_connector_limit_ = large_connector_limit;
  }

  double large_connector_growth_factor = large_connector_growth_factor_;
  if ( updateValue< double >( d,
         names::large_connector_growth_factor,
         large_connector_growth_factor ) )
  {
    if ( large_connector_growth_factor <= 1.0 )
    {
      throw KernelException(
        "The large connector capacity growth factor should be higher than "
        "1.0" );
    }

    large_connector_growth_factor_ = large_connector_growth_factor;
  }

  for ( size_t i = 0; i < delay_checkers_.size(); ++i )
  {
    delay_checkers_[ i ].set_status( d );
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
nest::ConnectionManager::get_status( DictionaryDatum& d )
{
  update_delay_extrema_();
  def< double >(
    d, names::min_delay, Time( Time::step( min_delay_ ) ).get_ms() );
  def< double >(
    d, names::max_delay, Time( Time::step( max_delay_ ) ).get_ms() );

  def< long >(
    d, names::initial_connector_capacity, initial_connector_capacity_ );
  def< long >( d, names::large_connector_limit, large_connector_limit_ );
  def< double >(
    d, names::large_connector_growth_factor, large_connector_growth_factor_ );

  size_t n = get_num_connections();
  def< long >( d, names::num_connections, n );
}

DictionaryDatum
nest::ConnectionManager::get_synapse_status( index gid,
  synindex syn_id,
  port p,
  thread tid )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );
  validate_pointer( connections_[ tid ].get( gid ) )
    ->get_synapse_status( syn_id, dict, p, tid );
  ( *dict )[ names::source ] = gid;
  ( *dict )[ names::synapse_model ] = LiteralDatum(
    kernel().model_manager.get_synapse_prototype( syn_id ).get_name() );

  return dict;
}

void
nest::ConnectionManager::set_synapse_status( index gid,
  synindex syn_id,
  port p,
  thread tid,
  const DictionaryDatum& dict )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );
  try
  {
    validate_pointer( connections_[ tid ].get( gid ) )
      ->set_synapse_status( syn_id,
        kernel().model_manager.get_synapse_prototype( syn_id, tid ),
        dict,
        p );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty( String::compose(
      "Setting status of '%1' connecting from GID %2 to port %3: %4",
      kernel().model_manager.get_synapse_prototype( syn_id, tid ).get_name(),
      gid,
      p,
      e.message() ) );
  }
}

void
nest::ConnectionManager::delete_connections_()
{
#ifdef _OPENMP
#pragma omp parallel
  {
#pragma omp for schedule( static, 1 )
#endif
    for ( size_t t = 0; t < connections_.size(); ++t )
    {
      for (
        tSConnector::nonempty_iterator iit = connections_[ t ].nonempty_begin();
        iit != connections_[ t ].nonempty_end();
        ++iit )
      {
#ifdef USE_PMA
        validate_pointer( *iit )->~ConnectorBase();
#else
      delete validate_pointer( *iit );
#endif
      }
      connections_[ t ].clear();
    }

#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
    poormansallocpool[ kernel().vp_manager.get_thread_id() ].destruct();
    poormansallocpool[ kernel().vp_manager.get_thread_id() ].init();
#else
    poormansallocpool.destruct();
    poormansallocpool.init();
#endif
#endif

#ifdef _OPENMP
  }
#endif
}

const nest::Time
nest::ConnectionManager::get_min_delay_time_() const
{
  Time min_delay = Time::pos_inf();

  tVDelayChecker::const_iterator it;
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

  tVDelayChecker::const_iterator it;
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

  tVDelayChecker::const_iterator it;
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
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    delay_checkers_[ t ].calibrate( tc );
  }
}

void
nest::ConnectionManager::connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
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

// gid node thread syn delay weight
void
nest::ConnectionManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  double d,
  double w )
{
  const thread tid = kernel().vp_manager.get_thread_id();
  Node* source = kernel().node_manager.get_node( sgid, target_thread );

  // target is a normal node or device with proxies
  if ( target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, d, w );
  }
  else if ( target->local_receiver() ) // target is a normal device
  {
    // make sure source is on this MPI rank
    if ( source->is_proxy() )
    {
      return;
    }

    if ( target->one_node_per_process() )
    {
      // connection to music proxy or similar device with one node per process.
      connect_( *source, *target, sgid, target_thread, syn, d, w );
      return;
    }

    // make sure connections are only created on the thread of the device
    if ( ( source->get_thread() != target_thread )
      && ( source->has_proxies() ) )
    {
      return;
    }

    if ( source->has_proxies() ) // normal neuron->device connection
    {
      connect_( *source, *target, sgid, target_thread, syn, d, w );
    }
    else // create device->device connections on suggested thread of target
    {
      target_thread = kernel().vp_manager.vp_to_thread(
        kernel().vp_manager.suggest_vp( target->get_gid() ) );
      if ( target_thread == tid )
      {
        source = kernel().node_manager.get_node( sgid, target_thread );
        target =
          kernel().node_manager.get_node( target->get_gid(), target_thread );
        connect_( *source, *target, sgid, target_thread, syn, d, w );
      }
    }
  }
  else // globally receiving devices, e.g., volume transmitter
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      throw IllegalConnection( "The models " + target->get_name() + " and "
        + source->get_name() + " cannot be connected." );
    }
    connect_( *source, *target, sgid, tid, syn, d, w );
  }
}

// gid node thread syn dict delay weight
void
nest::ConnectionManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  DictionaryDatum& params,
  double d,
  double w )
{
  const thread tid = kernel().vp_manager.get_thread_id();
  Node* source = kernel().node_manager.get_node( sgid, target_thread );

  // target is a normal node or device with proxies
  if ( target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  else if ( target->local_receiver() ) // target is a normal device
  {
    // make sure source is on this MPI rank
    if ( source->is_proxy() )
    {
      return;
    }

    if ( target->one_node_per_process() )
    {
      // connection to music proxy or similar device with one node per process.
      connect_( *source, *target, sgid, target_thread, syn, params, d, w );
      return;
    }

    // make sure connections are only created on the thread of the device
    if ( ( source->get_thread() != target_thread )
      && ( source->has_proxies() ) )
    {
      return;
    }

    if ( source->has_proxies() ) // normal neuron->device connection
    {
      connect_( *source, *target, sgid, target_thread, syn, params, d, w );
    }
    else // create device->device connections on suggested thread of target
    {
      target_thread = kernel().vp_manager.vp_to_thread(
        kernel().vp_manager.suggest_vp( target->get_gid() ) );
      if ( target_thread == tid )
      {
        source = kernel().node_manager.get_node( sgid, target_thread );
        target =
          kernel().node_manager.get_node( target->get_gid(), target_thread );
        connect_( *source, *target, sgid, target_thread, syn, params, d, w );
      }
    }
  }
  else // globally receiving devices, e.g., volume transmitter
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      throw IllegalConnection( "The models " + target->get_name() + " and "
        + source->get_name() + " cannot be connected." );
    }
    connect_( *source, *target, sgid, tid, syn, params, d, w );
  }
}

// gid gid dict
bool
nest::ConnectionManager::connect( index sgid,
  index tgid,
  DictionaryDatum& params,
  index syn )
{
  const thread tid = kernel().vp_manager.get_thread_id();

  // make sure target is on this MPI rank
  if ( not kernel().node_manager.is_local_gid( tgid ) )
  {
    return false;
  }

  Node* target = kernel().node_manager.get_node( tgid, tid );
  thread target_thread = target->get_thread();
  Node* source = kernel().node_manager.get_node( sgid, target_thread );

  // target is a normal node or device with proxies
  if ( target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, params );
  }
  else if ( target->local_receiver() ) // target is a normal device
  {
    // make sure source is on this MPI rank
    if ( source->is_proxy() )
    {
      return false;
    }

    if ( target->one_node_per_process() )
    {
      // connection to music proxy or similar device with one node per process.
      connect_( *source, *target, sgid, target_thread, syn, params );
      return true;
    }

    // make sure connections are only created on the thread of the device
    if ( ( source->get_thread() != target_thread )
      && ( source->has_proxies() ) )
    {
      return false;
    }

    if ( source->has_proxies() ) // normal neuron->device connection
    {
      connect_( *source, *target, sgid, target_thread, syn, params );
    }
    else // create device->device connections on suggested thread of target
    {
      target_thread = kernel().vp_manager.vp_to_thread(
        kernel().vp_manager.suggest_vp( target->get_gid() ) );
      if ( target_thread == tid )
      {
        source = kernel().node_manager.get_node( sgid, target_thread );
        target =
          kernel().node_manager.get_node( target->get_gid(), target_thread );
        connect_( *source, *target, sgid, target_thread, syn, params );
      }
    }
  }
  else // globally receiving devices, e.g., volume transmitter
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      throw IllegalConnection( "The models " + target->get_name() + " and "
        + source->get_name() + " cannot be connected." );
    }
    connect_( *source, *target, sgid, tid, syn, params );
  }
  // We did not exit prematurely due to proxies, so we have connected.
  return true;
}

/*
 Connection::Manager::connect()

 Here a short description of the logic of the following connect() methods
 (from a mail conversation between HEP and MH, 2013-07-03)

 1. On the first line, conn is assigned from connections_[tid], may
 be 0.  It may be zero, if there is no outgoing connection from
 the neuron s_gid on this thread.  It will also create the sparse
 table for the specified thread tid, if it does not exist yet.

 2. After the second line, c will contain a pointer to a
 ConnectorBase object, c will never be zero. The pointer address
 conn may be changed by add_connection, due to suicide.
 This possibly new pointer is returned and stored in c.

 3. The third line inserts c into the same place where conn was
 taken from on the first line.  It stores the pointer conn in the
 sparse table, either overwriting the old value, if unequal 0, or
 creating a new entry.


 The parameters delay and weight have the default value numerics::nan.
 numerics::nan is a special value, which describes double values that
 are not a number. If delay or weight is omitted in an connect call,
 numerics::nan indicates this and weight/delay are set only, if they are valid.
 */

void
nest::ConnectionManager::connect_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double d,
  double w )
{
  // see comment above for explanation
  ConnectorBase* conn = validate_source_entry_( tid, s_gid, syn );
  ConnectorBase* c = kernel()
                       .model_manager.get_synapse_prototype( syn, tid )
                       .add_connection( s, r, conn, syn, d, w );
  connections_[ tid ].set( s_gid, c );
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
  double d,
  double w )
{
  // see comment above for explanation
  ConnectorBase* conn = validate_source_entry_( tid, s_gid, syn );
  ConnectorBase* c = kernel()
                       .model_manager.get_synapse_prototype( syn, tid )
                       .add_connection( s, r, conn, syn, p, d, w );
  connections_[ tid ].set( s_gid, c );
  // TODO: set size of vv_num_connections in init
  if ( vv_num_connections_[ tid ].size() <= syn )
  {
    vv_num_connections_[ tid ].resize( syn + 1 );
  }
  ++vv_num_connections_[ tid ][ syn ];
}

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

  if ( kernel().node_manager.is_local_gid( target.get_gid() ) )
  {
    // We check that a connection actually exists between target and source
    // This is to properly handle the case when structural plasticity is not
    // enabled but the user wants to delete a connection between a target and
    // a source which are not connected
    if ( validate_source_entry_( target_thread, sgid, syn_id ) == 0 )
    {
      throw InexistentConnection();
    }
    DictionaryDatum data = DictionaryDatum( new Dictionary );
    def< index >( data, names::target, target.get_gid() );
    def< index >( data, names::source, sgid );
    ArrayDatum conns = kernel().connection_manager.get_connections( data );
    if ( conns.numReferences() == 0 )
    {
      throw InexistentConnection();
    }
    ConnectorBase* c =
      kernel()
        .model_manager.get_synapse_prototype( syn_id, target_thread )
        .delete_connection( target,
          target_thread,
          validate_source_entry_( target_thread, sgid, syn_id ),
          syn_id );
    if ( c == 0 )
    {
      connections_[ target_thread ].erase( sgid );
    }
    else
    {
      connections_[ target_thread ].set( sgid, c );
    }
    --vv_num_connections_[ target_thread ][ syn_id ];
  }
}

// -----------------------------------------------------------------------------

void
nest::ConnectionManager::data_connect_single( const index source_id,
  DictionaryDatum pars,
  const index syn )
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
      data_connect_single( src->get_gid(), pars, syn );
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

nest::ConnectorBase*
nest::ConnectionManager::validate_source_entry_( const thread tid,
  const index s_gid,
  const synindex syn_id )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );
  return validate_source_entry_( tid, s_gid );
}

nest::ConnectorBase*
nest::ConnectionManager::validate_source_entry_( const thread tid,
  const index s_gid )
{
  // resize sparsetable to full network size
  if ( connections_[ tid ].size() < kernel().node_manager.size() )
  {
    connections_[ tid ].resize( kernel().node_manager.size() );
  }

  // check, if entry exists
  // if not put in zero pointer
  if ( connections_[ tid ].test( s_gid ) )
  {
    return connections_[ tid ].get( s_gid );
  }
  else
  {
    return 0; // if non-existing
  }
}

// -----------------------------------------------------------------------------

void
nest::ConnectionManager::trigger_update_weight( const long vt_id,
  const std::vector< spikecounter >& dopa_spikes,
  const double t_trig )
{
  const index t = kernel().vp_manager.get_thread_id();
  for ( tSConnector::const_nonempty_iterator it =
          connections_[ t ].nonempty_begin();
        it != connections_[ t ].nonempty_end();
        ++it )
  {
    validate_pointer( *it )->trigger_update_weight( vt_id,
      t,
      dopa_spikes,
      t_trig,
      kernel().model_manager.get_synapse_prototypes( t ) );
  }
}

void
nest::ConnectionManager::send( thread t, index sgid, Event& e )
{
  if ( sgid
    < connections_[ t ]
        .size() ) // probably test only fails, if there are no connections
  {
    ConnectorBase* p = connections_[ t ].get( sgid );
    if ( p != 0 ) // only send, if connections exist
    {
      // the two least significant bits of the pointer
      // contain the information, whether there are
      // primary and secondary connections behind
      if ( has_primary( p ) )
      {
        // erase 2 least significant bits to obtain the correct pointer
        validate_pointer( p )->send(
          e, t, kernel().model_manager.get_synapse_prototypes( t ) );
      }
    }
  }
}

void
nest::ConnectionManager::send_secondary( thread t, SecondaryEvent& e )
{

  index sgid = e.get_sender_gid();

  // probably test only fails, if there are no connections
  if ( sgid < connections_[ t ].size() )
  {
    ConnectorBase* p = connections_[ t ].get( sgid );
    if ( p != 0 ) // only send, if connections exist
    {
      if ( has_secondary( p ) )
      {
        // erase 2 least significant bits to obtain the correct pointer
        p = validate_pointer( p );

        if ( p->homogeneous_model() )
        {
          if ( e.supports_syn_id( p->get_syn_id() ) )
          {
            p->send( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
          }
        }
        else
        {
          p->send_secondary(
            e, t, kernel().model_manager.get_synapse_prototypes( t ) );
        }
      }
    }
  }
}

size_t
nest::ConnectionManager::get_num_connections() const
{
  size_t num_connections = 0;
  tVDelayChecker::const_iterator i;
  for ( index t = 0; t < vv_num_connections_.size(); ++t )
  {
    for ( index s = 0; s < vv_num_connections_[ t ].size(); ++s )
    {
      num_connections += vv_num_connections_[ t ][ s ];
    }
  }

  return num_connections;
}

size_t
nest::ConnectionManager::get_num_connections( synindex syn_id ) const
{
  size_t num_connections = 0;
  tVDelayChecker::const_iterator i;
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

  size_t syn_id = 0;

#ifdef _OPENMP
  std::string msg;
  msg = String::compose( "Setting OpenMP num_threads to %1.",
    kernel().vp_manager.get_num_threads() );
  LOG( M_DEBUG, "ConnectionManager::get_connections", msg );
  omp_set_num_threads( kernel().vp_manager.get_num_threads() );
#endif

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

// Helper method, implemented as operator<<(), that removes ConnectionIDs from
// input deque and appends them to output deque.
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
nest::ConnectionManager::get_connections(
  std::deque< ConnectionID >& connectome,
  TokenArray const* source,
  TokenArray const* target,
  size_t syn_id,
  long synapse_label ) const
{
  if ( get_num_connections( syn_id ) == 0 )
  {
    return;
  }

  if ( source == 0 and target == 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread t = kernel().vp_manager.get_thread_id();
#else
    for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
#endif
      std::deque< ConnectionID > conns_in_thread;

      for ( index source_id = 1; source_id < connections_[ t ].size();
            ++source_id )
      {
        if ( connections_[ t ].get( source_id ) != 0 )
        {
          validate_pointer( connections_[ t ].get( source_id ) )
            ->get_connections(
              source_id, t, syn_id, synapse_label, conns_in_thread );
        }
      }
      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        extend_connectome( connectome, conns_in_thread );
      }
    }

    return;
  }
  else if ( source == 0 and target != 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread t = kernel().vp_manager.get_thread_id();
#else
    for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
#endif
      std::deque< ConnectionID > conns_in_thread;

      for ( index source_id = 1; source_id < connections_[ t ].size();
            ++source_id )
      {
        if ( validate_pointer( connections_[ t ].get( source_id ) ) != 0 )
        {
          for ( index t_id = 0; t_id < target->size(); ++t_id )
          {
            size_t target_id = target->get( t_id );
            validate_pointer( connections_[ t ].get( source_id ) )
              ->get_connections( source_id,
                target_id,
                t,
                syn_id,
                synapse_label,
                conns_in_thread );
          }
        }
      }
      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        extend_connectome( connectome, conns_in_thread );
      }
    }
    return;
  }
  else if ( source != 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      size_t t = kernel().vp_manager.get_thread_id();
#else
    for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
#endif
      std::deque< ConnectionID > conns_in_thread;

      for ( index s = 0; s < source->size(); ++s )
      {
        size_t source_id = source->get( s );
        if ( source_id < connections_[ t ].size()
          && validate_pointer( connections_[ t ].get( source_id ) ) != 0 )
        {
          if ( target == 0 )
          {
            validate_pointer( connections_[ t ].get( source_id ) )
              ->get_connections(
                source_id, t, syn_id, synapse_label, conns_in_thread );
          }
          else
          {
            for ( index t_id = 0; t_id < target->size(); ++t_id )
            {
              size_t target_id = target->get( t_id );
              validate_pointer( connections_[ t ].get( source_id ) )
                ->get_connections( source_id,
                  target_id,
                  t,
                  syn_id,
                  synapse_label,
                  conns_in_thread );
            }
          }
        }
      }

      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        extend_connectome( connectome, conns_in_thread );
      }
    }
    return;
  } // else
}


void
nest::ConnectionManager::get_sources( std::vector< index > targets,
  std::vector< std::vector< index > >& sources,
  index synapse_model )
{
  thread thread_id;
  index source_gid;
  std::vector< std::vector< index > >::iterator source_it;
  std::vector< index >::iterator target_it;
  size_t num_connections;

  sources.resize( targets.size() );
  for ( std::vector< std::vector< index > >::iterator i = sources.begin();
        i != sources.end();
        i++ )
  {
    ( *i ).clear();
  }

  // loop over the threads
  for ( tVSConnector::iterator it = connections_.begin();
        it != connections_.end();
        ++it )
  {
    thread_id = it - connections_.begin();
    // loop over the sources (return the corresponding ConnectorBase)
    for ( tSConnector::nonempty_iterator iit = it->nonempty_begin();
          iit != it->nonempty_end();
          ++iit )
    {
      source_gid = connections_[ thread_id ].get_pos( iit );

      // loop over the targets/sources
      source_it = sources.begin();
      target_it = targets.begin();
      for ( ; target_it != targets.end(); target_it++, source_it++ )
      {
        num_connections = validate_pointer( *iit )->get_num_connections(
          *target_it, thread_id, synapse_model );
        for ( size_t c = 0; c < num_connections; c++ )
        {
          ( *source_it ).push_back( source_gid );
        }
      }
    }
  }
}

void
nest::ConnectionManager::get_targets( const std::vector< index >& sources,
  std::vector< std::vector< index > >& targets,
  const index synapse_model,
  const std::string& post_synaptic_element )
{
  // Clear targets vector and resize to sources size
  std::vector< std::vector< index > >( sources.size() ).swap( targets );

  // We go through the connections data structure to retrieve all
  // targets which have an specific post synaptic element for each
  // source.
  for ( thread tid = 0;
        static_cast< unsigned int >( tid ) < connections_.size();
        ++tid )
  {
    // loop over the targets/sources
    std::vector< index >::const_iterator sources_it = sources.begin();
    std::vector< std::vector< index > >::iterator targets_it = targets.begin();
    for ( ; sources_it != sources.end(); ++sources_it, ++targets_it )
    {
      ConnectorBase* connector = validate_source_entry_( tid, *sources_it );
      if ( connector != 0 )
      {
        validate_pointer( connector )
          ->get_target_gids(
            *targets_it, tid, synapse_model, post_synaptic_element );
      }
    }
  }
}
