/*
 *  connection_builder_manager.cpp
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

#include "connection_builder_manager.h"

// C++ includes:
#include <cassert>
#include <cmath>
#include <set>

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

nest::ConnectionBuilderManager::ConnectionBuilderManager()
  : connruledict_( new Dictionary() )
  , connbuilder_factories_()
  , min_delay_( 1 )
  , max_delay_( 1 )
{
}

nest::ConnectionBuilderManager::~ConnectionBuilderManager()
{
  delete_connections_();
}

void
nest::ConnectionBuilderManager::initialize()
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
  assert( n_threads <= MAX_THREAD && "MAX_THREAD is a constant defined in allocator.h" );

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
nest::ConnectionBuilderManager::finalize()
{
  delete_connections_();
}

void
nest::ConnectionBuilderManager::set_status( const DictionaryDatum& d )
{
  for ( size_t i = 0; i < delay_checkers_.size(); ++i )
  {
    delay_checkers_[ i ].set_status( d );
  }
}

nest::DelayChecker&
nest::ConnectionBuilderManager::get_delay_checker()
{
  return delay_checkers_[ kernel().vp_manager.get_thread_id() ];
}

void
nest::ConnectionBuilderManager::get_status( DictionaryDatum& d )
{
  update_delay_extrema_();
  def< double >( d, "min_delay", Time( Time::step( min_delay_ ) ).get_ms() );
  def< double >( d, "max_delay", Time( Time::step( max_delay_ ) ).get_ms() );

  size_t n = get_num_connections();
  def< long >( d, "num_connections", n );
}

DictionaryDatum
nest::ConnectionBuilderManager::get_synapse_status( index gid, synindex syn_id, port p, thread tid )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );
  validate_pointer( connections_[ tid ].get( gid ) )->get_synapse_status( syn_id, dict, p );
  ( *dict )[ names::source ] = gid;
  ( *dict )[ names::synapse_model ] =
    LiteralDatum( kernel().model_manager.get_synapse_prototype( syn_id ).get_name() );

  return dict;
}

void
nest::ConnectionBuilderManager::set_synapse_status( index gid,
  synindex syn_id,
  port p,
  thread tid,
  const DictionaryDatum& dict )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );
  try
  {
    validate_pointer( connections_[ tid ].get( gid ) )
      ->set_synapse_status(
        syn_id, kernel().model_manager.get_synapse_prototype( syn_id, tid ), dict, p );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty(
      String::compose( "Setting status of '%1' connecting from GID %2 to port %3: %4",
        kernel().model_manager.get_synapse_prototype( syn_id, tid ).get_name(),
        gid,
        p,
        e.message() ) );
  }
}

void
nest::ConnectionBuilderManager::delete_connections_()
{
  for ( tVSConnector::iterator it = connections_.begin(); it != connections_.end(); ++it )
  {
    for ( tSConnector::nonempty_iterator iit = it->nonempty_begin(); iit != it->nonempty_end();
          ++iit )
    {
#ifdef USE_PMA
      validate_pointer( *iit )->~ConnectorBase();
#else
      delete validate_pointer( *iit );
#endif
    }
    it->clear();
  }

#if defined _OPENMP && defined USE_PMA
#ifdef IS_K
#pragma omp parallel
  {
    poormansallocpool[ omp_get_thread_num() ].destruct();
    poormansallocpool[ omp_get_thread_num() ].init();
  }
#else
#pragma omp parallel
  {
    poormansallocpool.destruct();
    poormansallocpool.init();
  }
#endif
#endif
}

const nest::Time
nest::ConnectionBuilderManager::get_min_delay_time_() const
{
  Time min_delay = Time::pos_inf();

  tVDelayChecker::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    min_delay = std::min( min_delay, it->get_min_delay() );

  return min_delay;
}

const nest::Time
nest::ConnectionBuilderManager::get_max_delay_time_() const
{
  Time max_delay = Time::get_resolution();

  tVDelayChecker::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    max_delay = std::max( max_delay, it->get_max_delay() );

  return max_delay;
}

bool
nest::ConnectionBuilderManager::get_user_set_delay_extrema() const
{
  bool user_set_delay_extrema = false;

  tVDelayChecker::const_iterator it;
  for ( it = delay_checkers_.begin(); it != delay_checkers_.end(); ++it )
    user_set_delay_extrema |= it->get_user_set_delay_extrema();

  return user_set_delay_extrema;
}

nest::ConnBuilder*
nest::ConnectionBuilderManager::get_conn_builder( const std::string& name,
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
  const size_t rule_id = connruledict_->lookup( name );
  return connbuilder_factories_.at( rule_id )->create( sources, targets, conn_spec, syn_spec );
}

void
nest::ConnectionBuilderManager::calibrate( const TimeConverter& tc )
{
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    delay_checkers_[ t ].calibrate( tc );
  }
}

void
nest::ConnectionBuilderManager::connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
  conn_spec->clear_access_flags();
  syn_spec->clear_access_flags();

  if ( !conn_spec->known( names::rule ) )
    throw BadProperty( "Connectivity spec must contain connectivity rule." );
  const Name rule_name = static_cast< const std::string >( ( *conn_spec )[ names::rule ] );

  if ( !connruledict_->known( rule_name ) )
    throw BadProperty( String::compose( "Unknown connectivty rule: %s", rule_name ) );
  const long rule_id = ( *connruledict_ )[ rule_name ];

  ConnBuilder* cb =
    connbuilder_factories_.at( rule_id )->create( sources, targets, conn_spec, syn_spec );
  assert( cb != 0 );

  // at this point, all entries in conn_spec and syn_spec have been checked
  ALL_ENTRIES_ACCESSED( *conn_spec, "Connect", "Unread dictionary entries in conn_spec: " );
  ALL_ENTRIES_ACCESSED( *syn_spec, "Connect", "Unread dictionary entries in syn_spec: " );

  cb->connect();
  delete cb;
}

void
nest::ConnectionBuilderManager::update_delay_extrema_()
{
  min_delay_ = get_min_delay_time_().get_steps();
  max_delay_ = get_max_delay_time_().get_steps();

  if ( not get_user_set_delay_extrema() )
  {
    // If no min/max_delay is set explicitly (SetKernelStatus), then the default
    // delay used by the SPBuilders have to be respected for the min/max_delay.
    min_delay_ = std::min( min_delay_, kernel().sp_manager.builder_min_delay() );
    max_delay_ = std::max( max_delay_, kernel().sp_manager.builder_max_delay() );
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
nest::ConnectionBuilderManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  double_t d,
  double_t w )
{
  Node* const source = kernel().node_manager.get_node( sgid, target_thread );

  // normal nodes and devices with proxies
  if ( target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, d, w );
  }
  else if ( target->local_receiver() ) // normal devices
  {
    if ( source->is_proxy() )
      return;

    if ( ( source->get_thread() != target_thread ) && ( source->has_proxies() ) )
    {
      target_thread = source->get_thread();
      target = kernel().node_manager.get_node( target->get_gid(), target_thread );
    }

    connect_( *source, *target, sgid, target_thread, syn, d, w );
  }
  else // globally receiving devices iterate over all target threads
  {
    if ( !source->has_proxies() ) // we do not allow to connect a device to a global receiver at the
      // moment
      return;
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread t = 0; t < n_threads; t++ )
    {
      target = kernel().node_manager.get_node( target->get_gid(), t );
      connect_( *source, *target, sgid, t, syn, d, w );
    }
  }
}

// gid node thread syn dict delay weight
void
nest::ConnectionBuilderManager::connect( index sgid,
  Node* target,
  thread target_thread,
  index syn,
  DictionaryDatum& params,
  double_t d,
  double_t w )
{
  Node* const source = kernel().node_manager.get_node( sgid, target_thread );

  // normal nodes and devices with proxies
  if ( target->has_proxies() )
  {
    connect_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  else if ( target->local_receiver() ) // normal devices
  {
    if ( source->is_proxy() )
      return;

    if ( ( source->get_thread() != target_thread ) && ( source->has_proxies() ) )
    {
      target_thread = source->get_thread();
      target = kernel().node_manager.get_node( target->get_gid(), target_thread );
    }

    connect_( *source, *target, sgid, target_thread, syn, params, d, w );
  }
  else // globally receiving devices iterate over all target threads
  {
    if ( !source->has_proxies() ) // we do not allow to connect a device to a global receiver at the
      // moment
      return;
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread t = 0; t < n_threads; t++ )
    {
      target = kernel().node_manager.get_node( target->get_gid(), t );
      connect_( *source, *target, sgid, t, syn, params, d, w );
    }
  }
}

// gid gid dict
bool
nest::ConnectionBuilderManager::connect( index source_id,
  index target_id,
  DictionaryDatum& params,
  index syn )
{

  if ( !kernel().node_manager.is_local_gid( target_id ) )
    return false;

  Node* target_ptr = kernel().node_manager.get_node( target_id );

  // target_thread defaults to 0 for devices
  thread target_thread = target_ptr->get_thread();

  Node* source_ptr = kernel().node_manager.get_node( source_id, target_thread );

  // normal nodes and devices with proxies
  if ( target_ptr->has_proxies() )
  {
    connect_( *source_ptr, *target_ptr, source_id, target_thread, syn, params );
  }
  else if ( target_ptr->local_receiver() ) // normal devices
  {
    if ( source_ptr->is_proxy() )
      return false;

    if ( ( source_ptr->get_thread() != target_thread ) && ( source_ptr->has_proxies() ) )
    {
      target_thread = source_ptr->get_thread();
      target_ptr = kernel().node_manager.get_node( target_id, target_thread );
    }

    connect_( *source_ptr, *target_ptr, source_id, target_thread, syn, params );
  }
  else // globally receiving devices iterate over all target threads
  {
    if ( !source_ptr->has_proxies() ) // we do not allow to connect a device to a global receiver at
      // the moment
      return false;
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread t = 0; t < n_threads; t++ )
    {
      target_ptr = kernel().node_manager.get_node( target_id, t );
      connect_( *source_ptr, *target_ptr, source_id, t, syn, params );
    }
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


 The parameters delay and weight have the default value NAN.
 NAN is a special value in cmath, which describes double values that
 are not a number. If delay or weight is omitted in an connect call,
 NAN indicates this and weight/delay are set only, if they are valid.
 */

void
nest::ConnectionBuilderManager::connect_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double_t d,
  double_t w )
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
nest::ConnectionBuilderManager::connect_( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  DictionaryDatum& p,
  double_t d,
  double_t w )
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
nest::ConnectionBuilderManager::disconnect( Node& target,
  index sgid,
  thread target_thread,
  index syn_id )
{

  if ( kernel().node_manager.is_local_gid( target.get_gid() ) )
  {
    // get the ConnectorBase corresponding to the source
    ConnectorBase* conn = validate_pointer( validate_source_entry_( target_thread, sgid, syn_id ) );
    ConnectorBase* c = kernel()
                         .model_manager.get_synapse_prototype( syn_id, target_thread )
                         .delete_connection( target, target_thread, conn, syn_id );
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
nest::ConnectionBuilderManager::divergent_connect( index source_id,
  const TokenArray& target_ids,
  const TokenArray& weights,
  const TokenArray& delays,
  index syn )
{
  bool complete_wd_lists = ( target_ids.size() == weights.size() && weights.size() != 0
    && weights.size() == delays.size() );
  bool short_wd_lists =
    ( target_ids.size() != weights.size() && weights.size() == 1 && delays.size() == 1 );
  bool no_wd_lists = ( weights.size() == 0 && delays.size() == 0 );

  // check if we have consistent lists for weights and delays
  if ( !( complete_wd_lists || short_wd_lists || no_wd_lists ) )
  {
    LOG( M_ERROR,
      "DivergentConnect",
      "If explicitly specified, weights and delays must be either doubles or lists of "
      "equal size. If given as lists, their size must be 1 or the same size as targets." );
    throw DimensionMismatch();
  }

  Node* source = kernel().node_manager.get_node( source_id );

  Subnet* source_comp = dynamic_cast< Subnet* >( source );
  if ( source_comp != 0 )
  {
    LOG( M_INFO, "DivergentConnect", "Source ID is a subnet; I will iterate it." );

    // collect all leaves in source subnet, then divergent-connect each leaf
    LocalLeafList local_sources( *source_comp );
    std::vector< MPIManager::NodeAddressingData > global_sources;
    kernel().mpi_manager.communicate( local_sources, global_sources );
    for ( std::vector< MPIManager::NodeAddressingData >::iterator src = global_sources.begin();
          src != global_sources.end();
          ++src )
      divergent_connect( src->get_gid(), target_ids, weights, delays, syn );

    return;
  }

  // We retrieve pointers for all targets, this implicitly checks if they
  // exist and throws UnknownNode if not.
  std::vector< Node* > targets;
  targets.reserve( target_ids.size() );

  // only bother with local targets - is_local_gid is cheaper than get_node()
  for ( index i = 0; i < target_ids.size(); ++i )
  {
    index gid = getValue< long >( target_ids[ i ] );
    if ( kernel().node_manager.is_local_gid( gid ) )
      targets.push_back( kernel().node_manager.get_node( gid ) );
  }

  for ( index i = 0; i < targets.size(); ++i )
  {
    thread target_thread = targets[ i ]->get_thread();

    if ( source->get_thread() != target_thread )
      source = kernel().node_manager.get_node( source_id, target_thread );

    if ( !targets[ i ]->has_proxies() && source->is_proxy() )
      continue;

    try
    {
      if ( complete_wd_lists )
        connect_( *source,
          *targets[ i ],
          source_id,
          target_thread,
          syn,
          delays.get( i ),
          weights.get( i ) );
      else if ( short_wd_lists )
        connect_( *source,
          *targets[ i ],
          source_id,
          target_thread,
          syn,
          delays.get( 0 ),
          weights.get( 0 ) );
      else
        connect_( *source, *targets[ i ], source_id, target_thread, syn );
    }
    catch ( IllegalConnection& e )
    {
      std::string msg = String::compose(
        "Target with ID %1 does not support the connection. "
        "The connection will be ignored.",
        targets[ i ]->get_gid() );
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
        source->get_gid(),
        targets[ i ]->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "DivergentConnect", msg.c_str() );
      continue;
    }
    catch ( TypeMismatch& e )
    {
      std::string msg = String::compose(
        "In Connection from global source ID %1 to target ID %2: "
        "Expect source and weights of type double. "
        "The connection will be ignored",
        source->get_gid(),
        targets[ i ]->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "DivergentConnect", msg.c_str() );
      continue;
    }
  }
}

// -----------------------------------------------------------------------------


void
nest::ConnectionBuilderManager::divergent_connect( index source_id,
  DictionaryDatum pars,
  index syn )
{
  // We extract the parameters from the dictionary explicitly since getValue() for DoubleVectorDatum
  // copies the data into an array, from which the data must then be copied once more.
  DictionaryDatum par_i( new Dictionary() );
  Dictionary::iterator di_s, di_t;

  // To save time, we first create the parameter dictionary for connect(), then we copy
  // all keys from the original dictionary into the parameter dictionary.
  // We can the later use iterators to change the values inside the parameter dictionary,
  // rather than using the lookup operator.
  // We also do the parameter checking here so that we can later use unsafe operations.
  for ( di_s = ( *pars ).begin(); di_s != ( *pars ).end(); ++di_s )
  {
    par_i->insert( di_s->first, Token( new DoubleDatum() ) );
    DoubleVectorDatum const* tmp = dynamic_cast< DoubleVectorDatum* >( di_s->second.datum() );
    if ( tmp == 0 )
    {

      std::string msg = String::compose(
        "Parameter '%1' must be a DoubleVectorArray or numpy.array. ", di_s->first.toString() );
      LOG( M_DEBUG, "DivergentConnect", msg );
      LOG( M_DEBUG, "DivergentConnect", "Trying to convert, but this takes time." );

      IntVectorDatum const* tmpint = dynamic_cast< IntVectorDatum* >( di_s->second.datum() );
      if ( tmpint )
      {
        std::vector< double >* data =
          new std::vector< double >( ( *tmpint )->begin(), ( *tmpint )->end() );
        DoubleVectorDatum* dvd = new DoubleVectorDatum( data );
        di_s->second = dvd;
        continue;
      }
      ArrayDatum* ad = dynamic_cast< ArrayDatum* >( di_s->second.datum() );
      if ( ad )
      {
        std::vector< double >* data = new std::vector< double >;
        ad->toVector( *data );
        DoubleVectorDatum* dvd = new DoubleVectorDatum( data );
        di_s->second = dvd;
      }
      else
        throw TypeMismatch( DoubleVectorDatum().gettypename().toString() + " or "
            + ArrayDatum().gettypename().toString(),
          di_s->second.datum()->gettypename().toString() );
    }
  }

  const Token target_t = pars->lookup2( names::target );
  DoubleVectorDatum const* ptarget_ids = static_cast< DoubleVectorDatum* >( target_t.datum() );
  const std::vector< double >& target_ids( **ptarget_ids );

  const Token weight_t = pars->lookup2( names::weight );
  DoubleVectorDatum const* pweights = static_cast< DoubleVectorDatum* >( weight_t.datum() );
  const std::vector< double >& weights( **pweights );

  const Token delay_t = pars->lookup2( names::delay );
  DoubleVectorDatum const* pdelays = static_cast< DoubleVectorDatum* >( delay_t.datum() );
  const std::vector< double >& delays( **pdelays );


  bool complete_wd_lists =
    ( target_ids.size() == weights.size() && weights.size() == delays.size() );
  // check if we have consistent lists for weights and delays
  if ( !complete_wd_lists )
  {
    LOG(
      M_ERROR, "DivergentConnect", "All lists in the paramter dictionary must be of equal size." );
    throw DimensionMismatch();
  }

  Node* source = kernel().node_manager.get_node( source_id );

  Subnet* source_comp = dynamic_cast< Subnet* >( source );
  if ( source_comp != 0 )
  {
    LOG( M_INFO, "DivergentConnect", "Source ID is a subnet; I will iterate it." );

    // collect all leaves in source subnet, then divergent-connect each leaf
    LocalLeafList local_sources( *source_comp );
    std::vector< MPIManager::NodeAddressingData > global_sources;
    kernel().mpi_manager.communicate( local_sources, global_sources );
    for ( std::vector< MPIManager::NodeAddressingData >::iterator src = global_sources.begin();
          src != global_sources.end();
          ++src )
      divergent_connect( src->get_gid(), pars, syn );

    return;
  }

  size_t n_targets = target_ids.size();
  for ( index i = 0; i < n_targets; ++i )
  {
    try
    {
      kernel().node_manager.get_node( target_ids[ i ] );
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

    // here we fill a parameter dictionary with the values of the current loop index.
    for ( di_s = ( *pars ).begin(), di_t = par_i->begin(); di_s != ( *pars ).end(); ++di_s, ++di_t )
    {
      DoubleVectorDatum const* tmp = static_cast< DoubleVectorDatum* >( di_s->second.datum() );
      const std::vector< double >& tmpvec = **tmp;
      DoubleDatum* dd = static_cast< DoubleDatum* >( di_t->second.datum() );
      ( *dd ) = tmpvec[ i ]; // We assign the double directly into the double datum.
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


void
nest::ConnectionBuilderManager::random_divergent_connect( index source_id,
  const TokenArray& target_ids,
  index n,
  const TokenArray& weights,
  const TokenArray& delays,
  bool allow_multapses,
  bool allow_autapses,
  index syn )
{
  Node* source = kernel().node_manager.get_node( source_id );

  // check if we have consistent lists for weights and delays
  if ( !( weights.size() == n || weights.size() == 0 ) && ( weights.size() == delays.size() ) )
  {
    LOG( M_ERROR, "RandomDivergentConnect", "weights and delays must be lists of size n." );
    throw DimensionMismatch();
  }

  Subnet* source_comp = dynamic_cast< Subnet* >( source );
  if ( source_comp != 0 )
  {
    LOG( M_INFO, "RandomDivergentConnect", "Source ID is a subnet; I will iterate it." );

    // collect all leaves in source subnet, then divergent-connect each leaf
    LocalLeafList local_sources( *source_comp );
    std::vector< MPIManager::NodeAddressingData > global_sources;
    kernel().mpi_manager.communicate( local_sources, global_sources );

    for ( std::vector< MPIManager::NodeAddressingData >::iterator src = global_sources.begin();
          src != global_sources.end();
          ++src )
      random_divergent_connect(
        src->get_gid(), target_ids, n, weights, delays, allow_multapses, allow_autapses, syn );

    return;
  }

  librandom::RngPtr rng = kernel().rng_manager.get_grng();

  TokenArray chosen_targets;

  std::set< long > ch_ids; // ch_ids used for multapses identification

  long n_rnd = target_ids.size();

  for ( size_t j = 0; j < n; ++j )
  {
    long t_id;

    do
    {
      t_id = rng->ulrand( n_rnd );
    } while ( ( !allow_autapses && ( ( index ) target_ids.get( t_id ) ) == source_id )
      || ( !allow_multapses && ch_ids.find( t_id ) != ch_ids.end() ) );

    if ( !allow_multapses )
      ch_ids.insert( t_id );

    chosen_targets.push_back( target_ids.get( t_id ) );
  }

  divergent_connect( source_id, chosen_targets, weights, delays, syn );
}

/**
 * Connect, using a dictionary with arrays.
 * This variant of connect combines the functionalities of
 * - connect
 * - divergent_connect
 * - convergent_connect
 * The decision is based on the details of the dictionary entries source and target.
 * If source and target are both either a GID or a list of GIDs with equal size, then source and
 * target are connected one-to-one.
 * If source is a gid and target is a list of GIDs then divergent_connect is used.
 * If source is a list of GIDs and target is a GID, then convergent_connect is used.
 * At this stage, the task of connect is to separate the dictionary into one for each thread and
 * then to forward the
 * connect call to the connectors who can then deal with the details of the connection.
 */
bool
nest::ConnectionBuilderManager::connect( ArrayDatum& conns )
{
  // #ifdef _OPENMP
  //     LOG(M_INFO, "ConnectionManager::Connect", msg);
  // #endif

  // #ifdef _OPENMP
  // #pragma omp parallel shared

  // #endif
  {
    for ( Token* ct = conns.begin(); ct != conns.end(); ++ct )
    {
      DictionaryDatum cd = getValue< DictionaryDatum >( *ct );
      index target_gid = static_cast< size_t >( ( *cd )[ names::target ] );
      Node* target_node = kernel().node_manager.get_node( target_gid );
      size_t thr = target_node->get_thread();

      // #ifdef _OPENMP
      // 	    size_t my_thr=omp_get_thread_num();
      // 	    if(my_thr == thr)
      // #endif
      {

        size_t syn_id = 0;
        index source_gid = ( *cd )[ names::source ];

        Token synmodel = cd->lookup( names::synapse_model );
        if ( !synmodel.empty() )
        {
          std::string synmodel_name = getValue< std::string >( synmodel );
          synmodel = kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
          if ( !synmodel.empty() )
            syn_id = static_cast< size_t >( synmodel );
          else
            throw UnknownModelName( synmodel_name );
        }
        Node* source_node = kernel().node_manager.get_node( source_gid );
        //#pragma omp critical
        connect_( *source_node, *target_node, source_gid, thr, syn_id, cd );
      }
    }
  }
  return true;
}

nest::ConnectorBase*
nest::ConnectionBuilderManager::validate_source_entry_( thread tid, index s_gid, synindex syn_id )
{
  kernel().model_manager.assert_valid_syn_id( syn_id );

  // resize sparsetable to full network size
  if ( connections_[ tid ].size() < kernel().node_manager.size() )
    connections_[ tid ].resize( kernel().node_manager.size() );

  // check, if entry exists
  // if not put in zero pointer
  if ( connections_[ tid ].test( s_gid ) )
    return connections_[ tid ].get( s_gid );
  else
    return 0; // if non-existing
}

// -----------------------------------------------------------------------------

void
nest::ConnectionBuilderManager::convergent_connect( const TokenArray& source_ids,
  index target_id,
  const TokenArray& weights,
  const TokenArray& delays,
  index syn )
{
  bool complete_wd_lists = ( source_ids.size() == weights.size() && weights.size() != 0
    && weights.size() == delays.size() );
  bool short_wd_lists =
    ( source_ids.size() != weights.size() && weights.size() == 1 && delays.size() == 1 );
  bool no_wd_lists = ( weights.size() == 0 && delays.size() == 0 );

  // check if we have consistent lists for weights and delays
  if ( !( complete_wd_lists || short_wd_lists || no_wd_lists ) )
  {
    LOG( M_ERROR,
      "ConvergentConnect",
      "weights and delays must be either doubles or lists of equal size. "
      "If given as lists, their size must be 1 or the same size as sources." );
    throw DimensionMismatch();
  }

  if ( !kernel().node_manager.is_local_gid( target_id ) )
    return;

  Node* target = kernel().node_manager.get_node( target_id );

  Subnet* target_comp = dynamic_cast< Subnet* >( target );
  if ( target_comp != 0 )
  {
    LOG( M_INFO, "ConvergentConnect", "Target node is a subnet; I will iterate it." );

    // we only iterate over local leaves, as remote targets are ignored anyways
    LocalLeafList target_nodes( *target_comp );
    for ( LocalLeafList::iterator tgt = target_nodes.begin(); tgt != target_nodes.end(); ++tgt )
      convergent_connect( source_ids, ( *tgt )->get_gid(), weights, delays, syn );

    return;
  }

  for ( index i = 0; i < source_ids.size(); ++i )
  {
    index source_id = source_ids.get( i );
    Node* source = kernel().node_manager.get_node( getValue< long >( source_id ) );

    thread target_thread = target->get_thread();

    if ( !target->has_proxies() )
    {
      // target_thread = sources[i]->get_thread();
      target_thread = source->get_thread();

      // If target is on the wrong thread, we need to get the right one now.
      if ( target->get_thread() != target_thread )
        target = kernel().node_manager.get_node( target_id, target_thread );

      if ( source->is_proxy() )
        continue;
    }

    // The source node may still be on a wrong thread, so we need to get the right
    // one now. As get_node() is quite expensive, so we only call it if we need to
    // if (source->get_thread() != target_thread)
    //  source = get_node(sid, target_thread);

    try
    {
      if ( complete_wd_lists )
        connect_(
          *source, *target, source_id, target_thread, syn, delays.get( i ), weights.get( i ) );
      else if ( short_wd_lists )
        connect_(
          *source, *target, source_id, target_thread, syn, delays.get( 0 ), weights.get( 0 ) );
      else
        connect_( *source, *target, source_id, target_thread, syn );
    }
    catch ( IllegalConnection& e )
    {
      std::string msg = String::compose(
        "Target with ID %1 does not support the connection. "
        "The connection will be ignored.",
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
    catch ( UnknownReceptorType& e )
    {
      std::string msg = String::compose(
        "In Connection from global source ID %1 to target ID %2: "
        "Target does not support requested receptor type. "
        "The connection will be ignored",
        source->get_gid(),
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
    catch ( TypeMismatch& e )
    {
      std::string msg = String::compose(
        "In Connection from global source ID %1 to target ID %2: "
        "Expect source and weights of type double. "
        "The connection will be ignored",
        source->get_gid(),
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
  }
}


/**
 * New and specialized variant of the convergent_connect()
 * function, which takes a vector<Node*> for sources and relies
 * on the fact that target is guaranteed to be on this thread.
 */
void
nest::ConnectionBuilderManager::convergent_connect( const std::vector< index >& source_ids,
  index target_id,
  const TokenArray& weights,
  const TokenArray& delays,
  index syn )
{
  bool complete_wd_lists = ( source_ids.size() == weights.size() && weights.size() != 0
    && weights.size() == delays.size() );
  bool short_wd_lists =
    ( source_ids.size() != weights.size() && weights.size() == 1 && delays.size() == 1 );

  // Check if we have consistent lists for weights and delays
  // already checked in previous RCC call

  Node* target = kernel().node_manager.get_node( target_id );
  for ( index i = 0; i < source_ids.size(); ++i )
  {
    Node* source = kernel().node_manager.get_node( source_ids[ i ] );
    thread target_thread = target->get_thread();

    if ( !target->has_proxies() )
    {
      target_thread = source->get_thread();

      // If target is on the wrong thread, we need to get the right one now.
      if ( target->get_thread() != target_thread )
        target = kernel().node_manager.get_node( target_id, target_thread );

      if ( source->is_proxy() )
        continue;
    }

    try
    {
      if ( complete_wd_lists )
        connect_( *source,
          *target,
          source_ids[ i ],
          target_thread,
          syn,
          delays.get( i ),
          weights.get( i ) );
      else if ( short_wd_lists )
        connect_( *source,
          *target,
          source_ids[ i ],
          target_thread,
          syn,
          delays.get( 0 ),
          weights.get( 0 ) );
      else
        connect_( *source, *target, source_ids[ i ], target_thread, syn );
    }
    catch ( IllegalConnection& e )
    {
      std::string msg = String::compose(
        "Target with ID %1 does not support the connection. "
        "The connection will be ignored.",
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
    catch ( UnknownReceptorType& e )
    {
      std::string msg = String::compose(
        "In Connection from global source ID %1 to target ID %2: "
        "Target does not support requested receptor type. "
        "The connection will be ignored",
        source->get_gid(),
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
    catch ( TypeMismatch& e )
    {
      std::string msg = String::compose(
        "In Connection from global source ID %1 to target ID %2: "
        "Expect source and weights of type double. "
        "The connection will be ignored",
        source->get_gid(),
        target->get_gid() );
      if ( !e.message().empty() )
        msg += "\nDetails: " + e.message();
      LOG( M_WARNING, "ConvergentConnect", msg.c_str() );
      continue;
    }
  }
}


void
nest::ConnectionBuilderManager::random_convergent_connect( const TokenArray& source_ids,
  index target_id,
  index n,
  const TokenArray& weights,
  const TokenArray& delays,
  bool allow_multapses,
  bool allow_autapses,
  index syn )
{
  if ( !kernel().node_manager.is_local_gid( target_id ) )
    return;

  Node* target = kernel().node_manager.get_node( target_id );

  // check if we have consistent lists for weights and delays
  if ( !( weights.size() == n || weights.size() == 0 ) && ( weights.size() == delays.size() ) )
  {
    LOG( M_ERROR, "ConvergentConnect", "weights and delays must be lists of size n." );
    throw DimensionMismatch();
  }

  Subnet* target_comp = dynamic_cast< Subnet* >( target );
  if ( target_comp != 0 )
  {
    LOG( M_INFO, "RandomConvergentConnect", "Target ID is a subnet; I will iterate it." );

    // we only consider local leaves as targets,
    LocalLeafList target_nodes( *target_comp );
    for ( LocalLeafList::iterator tgt = target_nodes.begin(); tgt != target_nodes.end(); ++tgt )
      random_convergent_connect(
        source_ids, ( *tgt )->get_gid(), n, weights, delays, allow_multapses, allow_autapses, syn );

    return;
  }

  librandom::RngPtr rng = kernel().rng_manager.get_rng( target->get_thread() );
  TokenArray chosen_sources;

  std::set< long > ch_ids;

  long n_rnd = source_ids.size();

  for ( size_t j = 0; j < n; ++j )
  {
    long s_id;

    do
    {
      s_id = rng->ulrand( n_rnd );
    } while ( ( !allow_autapses && ( ( index ) source_ids[ s_id ] ) == target_id )
      || ( !allow_multapses && ch_ids.find( s_id ) != ch_ids.end() ) );

    if ( !allow_multapses )
      ch_ids.insert( s_id );

    chosen_sources.push_back( source_ids[ s_id ] );
  }

  convergent_connect( chosen_sources, target_id, weights, delays, syn );
}

// This function loops over all targets, with every thread taking
// care only of its own target nodes
void
nest::ConnectionBuilderManager::random_convergent_connect( TokenArray& source_ids,
  TokenArray& target_ids,
  TokenArray& ns,
  TokenArray& weights,
  TokenArray& delays,
  bool allow_multapses,
  bool allow_autapses,
  index syn )
{
#ifndef _OPENMP
  // It only makes sense to call this function if we have openmp
  LOG( M_ERROR, "ConvergentConnect", "This function can only be called using OpenMP threading." );
  throw KernelException();
#else

  // Collect all nodes on this process and convert the TokenArray with
  // the sources to a std::vector<Node*>. This is needed, because
  // 1. We don't want to call get_node() within the loop for many
  //    neurons several times
  // 2. The function token_array::operator[]() is not thread-safe, so
  //    the threads will possibly access the same element at the same
  //    time, causing segfaults

  std::vector< index > vsource_ids( source_ids.size() );
  for ( index i = 0; i < source_ids.size(); ++i )
  {
    index sid = getValue< long >( source_ids.get( i ) );
    vsource_ids[ i ] = sid;
  }

  // Check if we have consistent lists for weights and delays
  if ( !( weights.size() == ns.size() || weights.size() == 0 )
    && ( weights.size() == delays.size() ) )
  {
    LOG( M_ERROR, "ConvergentConnect", "weights, delays and ns must be same size." );
    throw DimensionMismatch();
  }

  for ( size_t i = 0; i < ns.size(); ++i )
  {
    size_t n;
    // This throws std::bad_cast if the dynamic_cast goes
    // wrong. Throwing in a parallel section is not allowed. This
    // could be solved by only accepting IntVectorDatums for the ns.
    try
    {
      const IntegerDatum& nid = dynamic_cast< const IntegerDatum& >( *ns.get( i ) );
      n = nid.get();
    }
    catch ( const std::bad_cast& e )
    {
      LOG( M_ERROR, "ConvergentConnect", "ns must consist of integers only." );
      throw KernelException();
    }

    // Check if we have consistent lists for weights and delays part two.
    // The inner lists have to be equal to n or be zero.
    if ( weights.size() > 0 )
    {
      TokenArray ws = getValue< TokenArray >( weights.get( i ) );
      TokenArray ds = getValue< TokenArray >( delays.get( i ) );

      if ( !( ws.size() == n || ws.size() == 0 ) && ( ws.size() == ds.size() ) )
      {
        LOG( M_ERROR, "ConvergentConnect", "weights and delays must be lists of size n." );
        throw DimensionMismatch();
      }
    }
  }

#pragma omp parallel
  {
    int nrn_counter = 0;
    int tid = kernel().vp_manager.get_thread_id();

    librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

    for ( size_t i = 0; i < target_ids.size(); i++ )
    {
      index target_id = target_ids.get( i );

      // This is true for neurons on remote processes
      if ( !kernel().node_manager.is_local_gid( target_id ) )
        continue;

      Node* target = kernel().node_manager.get_node( target_id, tid );

      // Check, if target is on our thread
      if ( target->get_thread() != tid )
        continue;

      nrn_counter++;

      // extract number of connections for target i
      const IntegerDatum& nid = dynamic_cast< const IntegerDatum& >( *ns.get( i ) );
      const size_t n = nid.get();

      // extract weights and delays for all connections to target i
      TokenArray ws;
      TokenArray ds;
      if ( weights.size() > 0 )
      {
        ws = getValue< TokenArray >( weights.get( i ) );
        ds = getValue< TokenArray >( delays.get( i ) );
      }

      std::vector< index > chosen_source_ids( n );
      std::set< long > ch_ids;

      long n_rnd = vsource_ids.size();

      for ( size_t j = 0; j < n; ++j )
      {
        long s_id;

        do
        {
          s_id = rng->ulrand( n_rnd );
        } while ( ( !allow_autapses && ( ( index ) vsource_ids[ s_id ] ) == target_id )
          || ( !allow_multapses && ch_ids.find( s_id ) != ch_ids.end() ) );

        if ( !allow_multapses )
          ch_ids.insert( s_id );

        chosen_source_ids[ j ] = vsource_ids[ s_id ];
      }

      convergent_connect( chosen_source_ids, target_id, ws, ds, syn );

    } // of for all targets
  }   // of omp parallel
#endif
}

void
nest::ConnectionBuilderManager::trigger_update_weight( const long_t vt_id,
  const std::vector< spikecounter >& dopa_spikes,
  const double_t t_trig )
{
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    for ( tSConnector::const_nonempty_iterator it = connections_[ t ].nonempty_begin();
          it != connections_[ t ].nonempty_end();
          ++it )
      validate_pointer( *it )->trigger_update_weight(
        vt_id, t, dopa_spikes, t_trig, kernel().model_manager.get_synapse_prototypes( t ) );
}

void
nest::ConnectionBuilderManager::send( thread t, index sgid, Event& e )
{
  if ( sgid < connections_[ t ].size() ) // probably test only fails, if there are no connections
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
        validate_pointer( p )->send( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
      }
    }
  }
}

void
nest::ConnectionBuilderManager::send_secondary( thread t, SecondaryEvent& e )
{

  index sgid = e.get_sender_gid();

  if ( sgid < connections_[ t ].size() ) // probably test only fails, if there are no connections
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
            p->send( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
        }
        else
          p->send_secondary( e, t, kernel().model_manager.get_synapse_prototypes( t ) );
      }
    }
  }
}

size_t
nest::ConnectionBuilderManager::get_num_connections() const
{
  size_t num_connections = 0;
  tVDelayChecker::const_iterator i;
  for ( index t = 0; t < vv_num_connections_.size(); ++t )
    for ( index s = 0; s < vv_num_connections_[ t ].size(); ++s )
      num_connections += vv_num_connections_[ t ][ s ];

  return num_connections;
}

size_t
nest::ConnectionBuilderManager::get_num_connections( synindex syn_id ) const
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
nest::ConnectionBuilderManager::get_connections( DictionaryDatum params ) const
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
    source_a = dynamic_cast< TokenArray const* >( source_t.datum() );
  if ( not target_t.empty() )
    target_a = dynamic_cast< TokenArray const* >( target_t.datum() );

  size_t syn_id = 0;

#ifdef _OPENMP
  std::string msg;
  msg =
    String::compose( "Setting OpenMP num_threads to %1.", kernel().vp_manager.get_num_threads() );
  LOG( M_DEBUG, "ConnectionManager::get_connections", msg );
  omp_set_num_threads( kernel().vp_manager.get_num_threads() );
#endif

  // First we check, whether a synapse model is given.
  // If not, we will iterate all.
  if ( not syn_model_t.empty() )
  {
    Name synmodel_name = getValue< Name >( syn_model_t );
    const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
    if ( !synmodel.empty() )
      syn_id = static_cast< size_t >( synmodel );
    else
      throw UnknownModelName( synmodel_name.toString() );
    get_connections( connectome, source_a, target_a, syn_id, synapse_label );
  }
  else
  {
    for ( syn_id = 0; syn_id < kernel().model_manager.get_num_synapse_prototypes(); ++syn_id )
    {
      ArrayDatum conn;
      get_connections( conn, source_a, target_a, syn_id, synapse_label );
      if ( conn.size() > 0 )
        connectome.push_back( new ArrayDatum( conn ) );
    }
  }

  return connectome;
}

void
nest::ConnectionBuilderManager::get_connections( ArrayDatum& connectome,
  TokenArray const* source,
  TokenArray const* target,
  size_t syn_id,
  long_t synapse_label ) const
{
  size_t num_connections = get_num_connections( syn_id );

  connectome.reserve( num_connections );

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
      ArrayDatum conns_in_thread;
      size_t num_connections_in_thread = 0;
      // Count how many connections we will have.
      for ( tSConnector::const_nonempty_iterator it = connections_[ t ].nonempty_begin();
            it != connections_[ t ].nonempty_end();
            ++it )
      {
        num_connections_in_thread += validate_pointer( *it )->get_num_connections();
      }

#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index source_id = 1; source_id < connections_[ t ].size(); ++source_id )
      {
        if ( connections_[ t ].get( source_id ) != 0 )
          validate_pointer( connections_[ t ].get( source_id ) )
            ->get_connections( source_id, t, syn_id, synapse_label, conns_in_thread );
      }
      if ( conns_in_thread.size() > 0 )
      {
#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
        connectome.append_move( conns_in_thread );
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
      ArrayDatum conns_in_thread;
      size_t num_connections_in_thread = 0;
      // Count how many connections we will have maximally.
      for ( tSConnector::const_nonempty_iterator it = connections_[ t ].nonempty_begin();
            it != connections_[ t ].nonempty_end();
            ++it )
      {
        num_connections_in_thread += validate_pointer( *it )->get_num_connections();
      }

#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index source_id = 1; source_id < connections_[ t ].size(); ++source_id )
      {
        if ( validate_pointer( connections_[ t ].get( source_id ) ) != 0 )
        {
          for ( index t_id = 0; t_id < target->size(); ++t_id )
          {
            size_t target_id = target->get( t_id );
            validate_pointer( connections_[ t ].get( source_id ) )
              ->get_connections( source_id, target_id, t, syn_id, synapse_label, conns_in_thread );
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
      ArrayDatum conns_in_thread;
      size_t num_connections_in_thread = 0;
      // Count how many connections we will have maximally.
      for ( tSConnector::const_nonempty_iterator it = connections_[ t ].nonempty_begin();
            it != connections_[ t ].nonempty_end();
            ++it )
      {
        num_connections_in_thread += validate_pointer( *it )->get_num_connections();
      }

#ifdef _OPENMP
#pragma omp critical( get_connections )
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index s = 0; s < source->size(); ++s )
      {
        size_t source_id = source->get( s );
        if ( source_id < connections_[ t ].size()
          && validate_pointer( connections_[ t ].get( source_id ) ) != 0 )
        {
          if ( target == 0 )
          {
            validate_pointer( connections_[ t ].get( source_id ) )
              ->get_connections( source_id, t, syn_id, synapse_label, conns_in_thread );
          }
          else
          {
            for ( index t_id = 0; t_id < target->size(); ++t_id )
            {
              size_t target_id = target->get( t_id );
              validate_pointer( connections_[ t ].get( source_id ) )
                ->get_connections(
                  source_id, target_id, t, syn_id, synapse_label, conns_in_thread );
            }
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
    }
    return;
  } // else
}


void
nest::ConnectionBuilderManager::get_sources( std::vector< index > targets,
  std::vector< std::vector< index > >& sources,
  index synapse_model )
{
  thread thread_id;
  index source_gid;
  std::vector< std::vector< index > >::iterator source_it;
  std::vector< index >::iterator target_it;
  size_t num_connections;

  sources.resize( targets.size() );
  for ( std::vector< std::vector< index > >::iterator i = sources.begin(); i != sources.end(); i++ )
  {
    ( *i ).clear();
  }

  // loop over the threads
  for ( tVSConnector::iterator it = connections_.begin(); it != connections_.end(); ++it )
  {
    thread_id = it - connections_.begin();
    // loop over the sources (return the corresponding ConnectorBase)
    for ( tSConnector::nonempty_iterator iit = it->nonempty_begin(); iit != it->nonempty_end();
          ++iit )
    {
      source_gid = connections_[ thread_id ].get_pos( iit );

      // loop over the targets/sources
      source_it = sources.begin();
      target_it = targets.begin();
      for ( ; target_it != targets.end(); target_it++, source_it++ )
      {
        num_connections =
          validate_pointer( *iit )->get_num_connections( *target_it, thread_id, synapse_model );
        for ( size_t c = 0; c < num_connections; c++ )
        {
          ( *source_it ).push_back( source_gid );
        }
      }
    }
  }
}

void
nest::ConnectionBuilderManager::get_targets( std::vector< index > sources,
  std::vector< std::vector< index > >& targets,
  index synapse_model )
{
  thread thread_id;
  std::vector< index >::iterator source_it;
  std::vector< std::vector< index > >::iterator target_it;
  targets.resize( sources.size() );
  for ( std::vector< std::vector< index > >::iterator i = targets.begin(); i != targets.end(); i++ )
  {
    ( *i ).clear();
  }

  for ( tVSConnector::iterator it = connections_.begin(); it != connections_.end(); ++it )
  {
    thread_id = it - connections_.begin();
    // loop over the targets/sources
    source_it = sources.begin();
    target_it = targets.begin();
    for ( ; source_it != sources.end(); source_it++, target_it++ )
    {
      if ( ( *it ).get( *source_it ) != 0 )
      {
        validate_pointer( ( *it ).get( *source_it ) )
          ->get_target_gids( ( *target_it ), thread_id, synapse_model );
      }
    }
  }
}
