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
#include "connector_base.h"
#include "connection_label.h"
#include "network.h"
#include "nest_time.h"
#include "nest_datums.h"
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace nest
{

ConnectionManager::ConnectionManager( Network& net )
  : net_( net )
{
}

ConnectionManager::~ConnectionManager()
{
  delete_connections_();
  clear_prototypes_();

  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
      delete *i;
}

void
ConnectionManager::init( Dictionary* synapsedict )
{
  synapsedict_ = synapsedict;
  init_();
}

void
ConnectionManager::init_()
{
  synapsedict_->clear();

  // one list of prototypes per thread
  std::vector< std::vector< ConnectorModel* > > tmp_proto( net_.get_num_threads() );
  prototypes_.swap( tmp_proto );

  // (re-)append all synapse prototypes
  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
    {
      std::string name = ( *i )->get_name();
      for ( thread t = 0; t < net_.get_num_threads(); ++t )
        prototypes_[ t ].push_back( ( *i )->clone( name ) );
      synapsedict_->insert( name, prototypes_[ 0 ].size() - 1 );
    }

  tVSConnector tmp( net_.get_num_threads(), tSConnector() );

  connections_.swap( tmp );

  num_connections_ = 0;
}

void
ConnectionManager::delete_connections_()
{
  for ( tVSConnector::iterator it = connections_.begin(); it != connections_.end(); ++it )
    for ( tSConnector::nonempty_iterator iit = it->nonempty_begin(); iit != it->nonempty_end();
          ++iit )
#ifdef USE_PMA
      validate_pointer( *iit )->~ConnectorBase();
#else
      delete validate_pointer( *iit );
#endif

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

void
ConnectionManager::clear_prototypes_()
{
  for ( std::vector< std::vector< ConnectorModel* > >::iterator it = prototypes_.begin();
        it != prototypes_.end();
        ++it )
  {
    for ( std::vector< ConnectorModel* >::iterator pt = it->begin(); pt != it->end(); ++pt )
      if ( *pt != 0 )
        delete *pt;
    it->clear();
  }
  prototypes_.clear();
}

void
ConnectionManager::reset()
{
  delete_connections_();
  clear_prototypes_();
  init_();
}


synindex
ConnectionManager::register_synapse_prototype( ConnectorModel* cf )
{
  std::string name = cf->get_name();

  if ( synapsedict_->known( name ) )
  {
    delete cf;
    throw NamingConflict("A synapse type called '" + name + "' already exists.\n"
                         "Please choose a different name!");
  }

  pristine_prototypes_.push_back( cf );

  const synindex id = prototypes_[ 0 ].size();
  pristine_prototypes_[ id ]->set_syn_id( id );

  for ( thread t = 0; t < net_.get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( cf->clone( name ) );
    prototypes_[ t ][ id ]->set_syn_id( id );
  }

  synapsedict_->insert( name, id );

  return id;
}

void
ConnectionManager::calibrate( const TimeConverter& tc )
{
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( std::vector< ConnectorModel* >::iterator pt = prototypes_[ t ].begin();
          pt != prototypes_[ t ].end();
          ++pt )
      if ( *pt != 0 )
        ( *pt )->calibrate( tc );
}

const Time
ConnectionManager::get_min_delay() const
{
  Time min_delay = Time::pos_inf();

  std::vector< ConnectorModel* >::const_iterator it;
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( it = prototypes_[ t ].begin(); it != prototypes_[ t ].end(); ++it )
      if ( *it != 0 && ( *it )->get_num_connections() > 0 && ( *it )->has_delay() )
        min_delay = std::min( min_delay, ( *it )->get_min_delay() );

  return min_delay;
}

const Time
ConnectionManager::get_max_delay() const
{
  Time max_delay = Time::get_resolution();

  std::vector< ConnectorModel* >::const_iterator it;
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( it = prototypes_[ t ].begin(); it != prototypes_[ t ].end(); ++it )
      if ( *it != 0 && ( *it )->get_num_connections() > 0 && ( *it )->has_delay() )
        max_delay = std::max( max_delay, ( *it )->get_max_delay() );

  return max_delay;
}

bool
ConnectionManager::get_user_set_delay_extrema() const
{
  bool user_set_delay_extrema = false;
  std::vector< ConnectorModel* >::const_iterator it;
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( it = prototypes_[ t ].begin(); it != prototypes_[ t ].end(); ++it )
      user_set_delay_extrema |= ( *it )->get_user_set_delay_extrema();

  return user_set_delay_extrema;
}

synindex
ConnectionManager::copy_synapse_prototype( synindex old_id, std::string new_name )
{
  // we can assert here, as nestmodule checks this for us
  assert( !synapsedict_->known( new_name ) );

  int new_id = prototypes_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=255), maximal id of synapse_model = 254
  {
    net_.message( SLIInterpreter::M_ERROR,
      "ConnectionManager::copy_synapse_prototype",
      "CopyModel cannot generate another synapse. Maximal synapse model count of 255 exceeded." );
    throw KernelException( "Synapse model count exceeded" );
  }
  assert( new_id != invalid_synindex );

  // if the copied synapse is a secondary connector model the synid of the copy has to
  // be mapped to the corresponding secondary event type
  if ( not get_synapse_prototype( old_id ).is_primary() )
  {
    ( get_synapse_prototype( old_id ).get_event() )->add_syn_id( new_id );
  }

  for ( thread t = 0; t < net_.get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( get_synapse_prototype( old_id ).clone( new_name ) );
    prototypes_[ t ][ new_id ]->set_syn_id( new_id );
  }

  synapsedict_->insert( new_name, new_id );
  return new_id;
}


void
ConnectionManager::get_status( DictionaryDatum& d ) const
{
  size_t n = get_num_connections();
  def< long >( d, "num_connections", n );
}

void
ConnectionManager::set_prototype_status( synindex syn_id, const DictionaryDatum& d )
{
  assert_valid_syn_id( syn_id );
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
  {
    try
    {
      prototypes_[ t ][ syn_id ]->set_status( d );
    }
    catch ( BadProperty& e )
    {
      throw BadProperty( String::compose( "Setting status of prototype '%1': %2",
        prototypes_[ t ][ syn_id ]->get_name(),
        e.message() ) );
    }
  }
}

DictionaryDatum
ConnectionManager::get_prototype_status( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );

  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    prototypes_[ t ][ syn_id ]->get_status( dict ); // each call adds to num_connections

  return dict;
}

DictionaryDatum
ConnectionManager::get_synapse_status( index gid, synindex syn_id, port p, thread tid )
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );
  validate_pointer( connections_[ tid ].get( gid ) )->get_synapse_status( syn_id, dict, p );
  ( *dict )[ names::source ] = gid;
  ( *dict )[ names::synapse_model ] = LiteralDatum( get_synapse_prototype( syn_id ).get_name() );

  return dict;
}

void
ConnectionManager::set_synapse_status( index gid,
  synindex syn_id,
  port p,
  thread tid,
  const DictionaryDatum& dict )
{
  assert_valid_syn_id( syn_id );
  try
  {
    validate_pointer( connections_[ tid ].get( gid ) )
      ->set_synapse_status( syn_id, *( prototypes_[ tid ][ syn_id ] ), dict, p );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty(
      String::compose( "Setting status of '%1' connecting from GID %2 to port %3: %4",
        prototypes_[ tid ][ syn_id ]->get_name(),
        gid,
        p,
        e.message() ) );
  }
}

ArrayDatum
ConnectionManager::get_connections( DictionaryDatum params ) const
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
  msg = String::compose( "Setting OpenMP num_threads to %1.", net_.get_num_threads() );
  net_.message( SLIInterpreter::M_DEBUG, "ConnectionManager::get_connections", msg );
  omp_set_num_threads( net_.get_num_threads() );
#endif

  // First we check, whether a synapse model is given.
  // If not, we will iterate all.
  if ( not syn_model_t.empty() )
  {
    Name synmodel_name = getValue< Name >( syn_model_t );
    const Token synmodel = synapsedict_->lookup( synmodel_name );
    if ( !synmodel.empty() )
      syn_id = static_cast< size_t >( synmodel );
    else
      throw UnknownModelName( synmodel_name.toString() );
    get_connections( connectome, source_a, target_a, syn_id, synapse_label );
  }
  else
  {
    for ( syn_id = 0; syn_id < prototypes_[ 0 ].size(); ++syn_id )
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
ConnectionManager::get_connections( ArrayDatum& connectome,
  TokenArray const* source,
  TokenArray const* target,
  size_t syn_id,
  long_t synapse_label ) const
{
  size_t num_connections = 0;

  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    num_connections += prototypes_[ t ][ syn_id ]->get_num_connections();

  connectome.reserve( num_connections );

  if ( source == 0 and target == 0 )
  {
#ifdef _OPENMP
#pragma omp parallel
    {
      thread t = net_.get_thread_id();
#else
    for ( thread t = 0; t < net_.get_num_threads(); ++t )
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
#pragma omp critical
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
#pragma omp critical
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
      thread t = net_.get_thread_id();
#else
    for ( thread t = 0; t < net_.get_num_threads(); ++t )
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
#pragma omp critical
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index source_id = 1; source_id < connections_[ t ].size(); ++source_id )
      {
        if ( connections_[ t ].get( source_id ) != 0 )
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
#pragma omp critical
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
      size_t t = net_.get_thread_id();
#else
    for ( thread t = 0; t < net_.get_num_threads(); ++t )
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
#pragma omp critical
#endif
      conns_in_thread.reserve( num_connections_in_thread );
      for ( index s = 0; s < source->size(); ++s )
      {
        size_t source_id = source->get( s );
        if ( source_id < connections_[ t ].size() && connections_[ t ].get( source_id ) != 0 )
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
#pragma omp critical
#endif
        connectome.append_move( conns_in_thread );
      }
    }
    return;
  } // else
}

ConnectorBase*
ConnectionManager::validate_source_entry( thread tid, index s_gid, synindex syn_id )
{
  assert_valid_syn_id( syn_id );

  // resize sparsetable to full network size
  if ( connections_[ tid ].size() < net_.size() )
    connections_[ tid ].resize( net_.size() );

  // check, if entry exists
  // if not put in zero pointer
  if ( connections_[ tid ].test( s_gid ) )
    return connections_[ tid ].get(
      s_gid ); // returns non-const reference to stored type, here ConnectorBase*
  else
    return 0; // if non-existing
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
ConnectionManager::connect( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  double_t d,
  double_t w )
{
  // see comment above for explanation
  ConnectorBase* conn = validate_source_entry( tid, s_gid, syn );
  ConnectorBase* c = prototypes_[ tid ][ syn ]->add_connection( s, r, conn, syn, d, w );
  connections_[ tid ].set( s_gid, c );
}

void
ConnectionManager::connect( Node& s,
  Node& r,
  index s_gid,
  thread tid,
  index syn,
  DictionaryDatum& p,
  double_t d,
  double_t w )
{
  // see comment above for explanation
  ConnectorBase* conn = validate_source_entry( tid, s_gid, syn );
  ConnectorBase* c = prototypes_[ tid ][ syn ]->add_connection( s, r, conn, syn, p, d, w );
  connections_[ tid ].set( s_gid, c );
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
ConnectionManager::connect( ArrayDatum& conns )
{
  // #ifdef _OPENMP
  //     net_.message(SLIInterpreter::M_INFO, "ConnectionManager::Connect", msg);
  // #endif

  // #ifdef _OPENMP
  // #pragma omp parallel shared

  // #endif
  {
    for ( Token* ct = conns.begin(); ct != conns.end(); ++ct )
    {
      DictionaryDatum cd = getValue< DictionaryDatum >( *ct );
      index target_gid = static_cast< size_t >( ( *cd )[ names::target ] );
      Node* target_node = net_.get_node( target_gid );
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
          synmodel = synapsedict_->lookup( synmodel_name );
          if ( !synmodel.empty() )
            syn_id = static_cast< size_t >( synmodel );
          else
            throw UnknownModelName( synmodel_name );
        }
        Node* source_node = net_.get_node( source_gid );
        //#pragma omp critical
        connect( *source_node, *target_node, source_gid, thr, syn_id, cd );
      }
    }
  }
  return true;
}

void
ConnectionManager::trigger_update_weight( const long_t vt_id,
  const vector< spikecounter >& dopa_spikes,
  const double_t t_trig )
{
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( tSConnector::const_nonempty_iterator it = connections_[ t ].nonempty_begin();
          it != connections_[ t ].nonempty_end();
          ++it )
      validate_pointer( *it )->trigger_update_weight(
        vt_id, t, dopa_spikes, t_trig, prototypes_[ t ] );
}

void
ConnectionManager::send( thread t, index sgid, Event& e )
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
        validate_pointer( p )->send( e, t, prototypes_[ t ] );
      }
    }
  }
}


void
ConnectionManager::send_secondary( thread t, SecondaryEvent& e )
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
            p->send( e, t, prototypes_[ t ] );
        }
        else
          p->send_secondary( e, t, prototypes_[ t ] );
      }
    }
  }
}


size_t
ConnectionManager::get_num_connections() const
{
  num_connections_ = 0;
  for ( thread t = 0; t < net_.get_num_threads(); ++t )
    for ( std::vector< ConnectorModel* >::const_iterator i = prototypes_[ t ].begin();
          i != prototypes_[ t ].end();
          ++i )
      num_connections_ += ( *i )->get_num_connections();

  return num_connections_;
}

} // namespace
