/*
 *  sp_manager.cpp
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

/*
 * File:   sp_updater.cpp
 * Author: naveau
 *
 * Created on November 26, 2013, 2:28 PM
 */

#include "sp_manager.h"
#include "network.h"
#include "connector_model.h"
#include "conn_parameter.h"
#include "conn_builder.h"
#include "connector_base.h"
#include "nest_names.h"
#include <algorithm>

namespace nest
{

long_t SPManager::structural_plasticity_update_interval = 1000;

SPManager::SPManager( Network& net )
  : ConnectionManager( net )
{
}

SPManager::~SPManager()
{
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    delete *i;
  }
  sp_conn_builders.clear();
}

void
SPManager::reset()
{
  ConnectionManager::reset();
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    delete *i;
  }
  sp_conn_builders.clear();
}

/*
 * Methods to retrieve data regarding structural plasticity variables
 */
void
SPManager::get_structural_plasticity_status( DictionaryDatum& d ) const
{
  DictionaryDatum sp_synapses = DictionaryDatum( new Dictionary() );
  DictionaryDatum sp_synapse;


  def< DictionaryDatum >( d, names::structural_plasticity_synapses, sp_synapses );
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    sp_synapse = DictionaryDatum( new Dictionary() );
    def< std::string >(
      sp_synapse, names::pre_synaptic_element, ( *i )->get_pre_synaptic_element_name() );
    def< std::string >(
      sp_synapse, names::post_synaptic_element, ( *i )->get_post_synaptic_element_name() );
    std::stringstream syn_name;
    syn_name << "syn" << ( sp_conn_builders.end() - i );
    def< DictionaryDatum >( sp_synapses, syn_name.str(), sp_synapse );
  }

  def< long_t >(
    d, names::structural_plasticity_update_interval, structural_plasticity_update_interval );
}

/*
 * Methods to retrieve data regarding structural plasticity
 */
void
SPManager::get_status( DictionaryDatum& d ) const
{
  ConnectionManager::get_status( d );
  get_structural_plasticity_status( d );
}

/**
 * Set status of synaptic plasticity variables: synaptic update interval,
 * synapses and synaptic elements.
 * @param d Dictionary containing the values to be set
 */
void
SPManager::set_structural_plasticity_status( const DictionaryDatum& d )
{
  if ( d->known( names::structural_plasticity_update_interval ) )
    updateValue< long_t >(
      d, names::structural_plasticity_update_interval, structural_plasticity_update_interval );
  if ( !d->known( names::structural_plasticity_synapses ) )
    return;
  /*
   * Configure synapses model updated during the simulation.
   */
  Token synmodel;
  DictionaryDatum syn_specs, syn_spec;
  DictionaryDatum conn_spec = DictionaryDatum( new Dictionary() );

  if ( d->known( names::autapses ) )
    def< bool >( conn_spec, names::autapses, getValue< bool >( d, names::autapses ) );
  if ( d->known( names::multapses ) )
    def< bool >( conn_spec, names::multapses, getValue< bool >( d, names::multapses ) );
  GIDCollection sources = GIDCollection();
  GIDCollection targets = GIDCollection();

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    delete ( *i );
  }
  sp_conn_builders.clear();
  updateValue< DictionaryDatum >( d, names::structural_plasticity_synapses, syn_specs );
  for ( Dictionary::const_iterator i = syn_specs->begin(); i != syn_specs->end(); ++i )
  {
    syn_spec = getValue< DictionaryDatum >( syn_specs, i->first );
    // We use a ConnBuilder with dummy values to check the synapse parameters
    SPBuilder* conn_builder = new SPBuilder( net_, sources, targets, conn_spec, syn_spec );
    // check that the user defined the min and max delay properly
    conn_builder->get_min_delay();
    conn_builder->get_max_delay();
    sp_conn_builders.push_back( conn_builder );
  }
}

void
SPManager::set_status( const DictionaryDatum& d )
{
  set_structural_plasticity_status( d );
}

/**
 * Retrieve the min delay for the dynamic creation of synapses using
 * structural plasticity
 * @return the min delay to use in the synapse creation
 */
const Time
SPManager::get_min_delay() const
{
  // Get min_delay of all existing synapses
  Time min_delay = ConnectionManager::get_min_delay();

  // Get min_delay of the synapse connectors registered for the structural plasticity update
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    min_delay = std::min( min_delay, ( *i )->get_min_delay() );
  }
  return min_delay;
}

/**
 * Retrieve the max delay for the dynamic creation of synapses using
 * structural plasticity
 * @return the max delay to use in the synapse creation
 */
const Time
SPManager::get_max_delay() const
{
  // Get max_delay of all existing synapses
  Time max_delay = ConnectionManager::get_max_delay();

  // Get max_delay of the synapse connectors registered for the structural plasticity update
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    max_delay = std::max( max_delay, ( *i )->get_max_delay() );
  }
  return max_delay;
}

void
SPManager::update_structural_plasticity()
{
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders.begin();
        i != sp_conn_builders.end();
        i++ )
  {
    update_structural_plasticity( ( *i ) );
  }
}

/**
 * Handles the general dynamic creation and deletion of synapses when
 * structural plasticity is enabled. Retrieves the number of available
 * synaptic elements to create new synapses. Retrieves the number of
 * deleted synaptic elements to delete already created synapses.
 * @param sp_builder The structural plasticity connection builder to use
 */
void
SPManager::update_structural_plasticity( SPBuilder* sp_builder )
{
  // Index of neurons having a vacant synaptic element
  std::vector< index > pre_vacant_id;  // pre synaptic elements (e.g Axon)
  std::vector< index > post_vacant_id; // post synaptic element (e.g Den)
  std::vector< int_t > pre_vacant_n;   // number of synaptic elements
  std::vector< int_t > post_vacant_n;  // number of synaptic elements

  // Index of neuron deleting a synaptic element
  std::vector< index > pre_deleted_id, post_deleted_id;
  std::vector< int_t > pre_deleted_n, post_deleted_n;

  // Global vector for vacant and deleted synaptic element
  std::vector< index > pre_vacant_id_global, post_vacant_id_global;
  std::vector< int_t > pre_vacant_n_global, post_vacant_n_global;
  std::vector< index > pre_deleted_id_global, post_deleted_id_global;
  std::vector< int_t > pre_deleted_n_global, post_deleted_n_global;

  // Vector of displacements for communication
  std::vector< int > displacements;

  // Get pre synaptic elements data from global nodes
  get_synaptic_elements( sp_builder->get_pre_synaptic_element_name(),
    pre_vacant_id,
    pre_vacant_n,
    pre_deleted_id,
    pre_deleted_n );
  // Get post synaptic elements data from local nodes
  get_synaptic_elements( sp_builder->get_post_synaptic_element_name(),
    post_vacant_id,
    post_vacant_n,
    post_deleted_id,
    post_deleted_n );
  // Communicate the number of deleted pre-synaptic elements
  Communicator::communicate( pre_deleted_id, pre_deleted_id_global, displacements );
  Communicator::communicate( pre_deleted_n, pre_deleted_n_global, displacements );

  if ( pre_deleted_id_global.size() > 0 )
  {
    delete_synapses_from_pre( pre_deleted_id_global,
      pre_deleted_n_global,
      sp_builder->get_synapse_model(),
      sp_builder->get_pre_synaptic_element_name(),
      sp_builder->get_post_synaptic_element_name() );
    // update the number of synaptic elements
    get_synaptic_elements( sp_builder->get_pre_synaptic_element_name(),
      pre_vacant_id,
      pre_vacant_n,
      pre_deleted_id,
      pre_deleted_n );
    get_synaptic_elements( sp_builder->get_post_synaptic_element_name(),
      post_vacant_id,
      post_vacant_n,
      post_deleted_id,
      post_deleted_n );
  }

  // Communicate the number of deleted post-synaptic elements
  Communicator::communicate( post_deleted_id, post_deleted_id_global, displacements );
  Communicator::communicate( post_deleted_n, post_deleted_n_global, displacements );

  if ( post_deleted_id_global.size() > 0 )
  {
    delete_synapses_from_post( post_deleted_id_global,
      post_deleted_n_global,
      sp_builder->get_synapse_model(),
      sp_builder->get_pre_synaptic_element_name(),
      sp_builder->get_post_synaptic_element_name() );
    get_synaptic_elements( sp_builder->get_pre_synaptic_element_name(),
      pre_vacant_id,
      pre_vacant_n,
      pre_deleted_id,
      pre_deleted_n );
    get_synaptic_elements( sp_builder->get_post_synaptic_element_name(),
      post_vacant_id,
      post_vacant_n,
      post_deleted_id,
      post_deleted_n );
  }

  // Communicate vacant elements
  Communicator::communicate( pre_vacant_id, pre_vacant_id_global, displacements );
  Communicator::communicate( pre_vacant_n, pre_vacant_n_global, displacements );
  Communicator::communicate( post_vacant_id, post_vacant_id_global, displacements );
  Communicator::communicate( post_vacant_n, post_vacant_n_global, displacements );

  if ( pre_vacant_id_global.size() > 0 && post_vacant_id_global.size() > 0 )
  {
    create_synapses( pre_vacant_id_global,
      pre_vacant_n_global,
      post_vacant_id_global,
      post_vacant_n_global,
      sp_builder );
  }
}

/**
 * Dynamic creation of synapses
 * @param pre_id source id
 * @param pre_n number of available synaptic elements in the pre node
 * @param post_id target id
 * @param post_n number of available synaptic elements in the post node
 * @param sp_conn_builder structural plasticity connection builder to use
 */
void
SPManager::create_synapses( std::vector< index >& pre_id,
  std::vector< int_t >& pre_n,
  std::vector< index >& post_id,
  std::vector< int_t >& post_n,
  SPBuilder* sp_conn_builder )
{
  std::vector< index > pre_id_rnd;
  std::vector< index > post_id_rnd;
  std::vector< index >::iterator pre_it, post_it;

  // shuffle the vacant element
  serialize_id( pre_id, pre_n, pre_id_rnd );
  serialize_id( post_id, post_n, post_id_rnd );
  // Shuffle only the largest vector
  if ( pre_id_rnd.size() > post_id_rnd.size() )
  {
    // we only shuffle the n first items,
    // where n is the number of post synaptic elements
    global_shuffle( pre_id_rnd, post_id_rnd.size() );
    pre_id_rnd.resize( post_id_rnd.size() );
  }
  else
  {
    // we only shuffle the n first items,
    // where n is the number of pre synaptic elements
    global_shuffle( post_id_rnd, pre_id_rnd.size() );
    post_id_rnd.resize( pre_id_rnd.size() );
  }

  // create synapse
  GIDCollection sources = GIDCollection( pre_id_rnd );
  GIDCollection targets = GIDCollection( post_id_rnd );

  sp_conn_builder->connect( sources, targets );
}

/**
 * Deletion of synapses due to the loss of a pre synaptic element. The
 * corresponding pre synaptic element will still remain available for a new
 * connection on the following updates in connectivity
 * @param pre_deleted_id Id of the node with the deleted pre synaptic element
 * @param pre_deleted_n number of deleted pre synaptic elements
 * @param synapse_model model name
 * @param se_pre_name pre synaptic element name
 * @param se_post_name post synaptic element name
 */
void
SPManager::delete_synapses_from_pre( std::vector< index >& pre_deleted_id,
  std::vector< int_t >& pre_deleted_n,
  index synapse_model,
  std::string se_pre_name,
  std::string se_post_name )
{
  /*
   * Synapses deletion due to the loss of a pre-synaptic element need a
   * communication of the lists of target
   */

  // Connectivity
  std::vector< std::vector< index > > connectivity;
  std::vector< index > global_targets;
  std::vector< int > displacements;

  // iterators
  std::vector< std::vector< index > >::iterator connectivity_it;
  std::vector< index >::iterator id_it;
  std::vector< int_t >::iterator n_it;

  get_targets( pre_deleted_id, connectivity, synapse_model );

  id_it = pre_deleted_id.begin();
  n_it = pre_deleted_n.begin();
  connectivity_it = connectivity.begin();
  for ( ; id_it != pre_deleted_id.end() && n_it != pre_deleted_n.end();
        id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of targets
    Communicator::communicate( *connectivity_it, global_targets, displacements );
    // shuffle only the first n items, n is the number of deleted synaptic elements
    if ( -( *n_it ) > global_targets.size() )
      *n_it = -global_targets.size();
    global_shuffle( global_targets, -( *n_it ) );

    for ( int i = 0; i < -( *n_it ); i++ ) // n is negative
    {
      delete_synapse( *id_it, global_targets[ i ], synapse_model, se_pre_name, se_post_name );
    }
  }
}

/**
 * Handles the deletion of synapses between source and target nodes. The
 * deletion is defined by the pre and post synaptic elements and the synapse type.
 * Updates the number of connected synaptic elements in the source and target.
 * @param sgid source id
 * @param tgid target id
 * @param syn_id synapse type
 * @param se_pre_name name of the pre synaptic element
 * @param se_post_name name of the post synaptic element
 */
void
SPManager::delete_synapse( index sgid,
  index tgid,
  long syn_id,
  std::string se_pre_name,
  std::string se_post_name )
{
  // get thread id
  const int tid = net_.get_thread_id();
  if ( net_.is_local_gid( sgid ) )
  {
    Node* const source = net_.local_nodes_.get_node_by_gid( sgid );
    const thread source_thread = source->get_thread();
    if ( tid == source_thread )
    {
      source->connect_synaptic_element( se_pre_name, -1 );
    }
  }

  if ( net_.is_local_gid( tgid ) )
  {
    Node* const target = net_.local_nodes_.get_node_by_gid( tgid );
    thread target_thread = target->get_thread();
    if ( tid == target_thread )
    {
      // get the ConnectorBase corresponding to the source
      ConnectorBase* conn = connections_[ target_thread ].get( sgid );
      ConnectorBase* c = prototypes_[ target_thread ][ syn_id ]->delete_connection(
        *target, target_thread, conn, syn_id );
      if ( c == 0 )
      {
        connections_[ target_thread ].erase( sgid );
      }
      else
      {
        connections_[ target_thread ].set( sgid, c );
      }
      target->connect_synaptic_element( se_post_name, -1 );
    }
  }
}

/**
 * Deletion of synapses due to the loss of a post synaptic element. The
 * corresponding pre synaptic element will still remain available for a new
 * connection on the following updates in connectivity
 * @param post_deleted_id Id of the node with the deleted post synaptic element
 * @param post_deleted_n number of deleted post synaptic elements
 * @param synapse_model model name
 * @param se_pre_name pre synaptic element name
 * @param se_post_name post synaptic element name
 */
void
SPManager::delete_synapses_from_post( std::vector< index >& post_deleted_id,
  std::vector< int_t >& post_deleted_n,
  index synapse_model,
  std::string se_pre_name,
  std::string se_post_name )
{
  /*
   * TODO: Synapses deletion due to the loss of a post-synaptic element can
   * be done locally (except for the update of the number of pre-synaptic
   * element)
   */

  // Connectivity
  std::vector< std::vector< index > > connectivity;
  std::vector< index > global_sources;
  std::vector< int > displacements;

  // iterators
  std::vector< std::vector< index > >::iterator connectivity_it;
  std::vector< index >::iterator id_it;
  std::vector< int_t >::iterator n_it;

  // Retrieve the connected sources
  get_sources( post_deleted_id, connectivity, synapse_model );

  id_it = post_deleted_id.begin();
  n_it = post_deleted_n.begin();
  connectivity_it = connectivity.begin();

  for ( ; id_it != post_deleted_id.end() && n_it != post_deleted_n.end();
        id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of sources
    Communicator::communicate( *connectivity_it, global_sources, displacements );
    // shuffle only the first n items, n is the number of deleted synaptic elements
    if ( -( *n_it ) > global_sources.size() )
      *n_it = -global_sources.size();
    global_shuffle( global_sources, -( *n_it ) );

    for ( int i = 0; i < -( *n_it ); i++ ) // n is negative
    {
      delete_synapse( global_sources[ i ], *id_it, synapse_model, se_pre_name, se_post_name );
    }
  }
}

void
nest::SPManager::get_synaptic_elements( std::string se_name,
  std::vector< index >& se_vacant_id,
  std::vector< int_t >& se_vacant_n,
  std::vector< index >& se_deleted_id,
  std::vector< int_t >& se_deleted_n )
{
  // local nodes
  index n_vacant_id = 0;
  index n_deleted_id = 0;
  index gid;
  int_t n;
  size_t n_nodes = net_.local_nodes_.size();
  se_vacant_id.clear();
  se_vacant_n.clear();
  se_deleted_id.clear();
  se_deleted_n.clear();

  se_vacant_id.resize( n_nodes );
  se_vacant_n.resize( n_nodes );
  se_deleted_id.resize( n_nodes );
  se_deleted_n.resize( n_nodes );

  std::vector< index >::iterator vacant_id_it = se_vacant_id.begin();
  std::vector< int_t >::iterator vacant_n_it = se_vacant_n.begin();
  std::vector< index >::iterator deleted_id_it = se_deleted_id.begin();
  std::vector< int_t >::iterator deleted_n_it = se_deleted_n.begin();
  index node_it;

  for ( node_it = 0; node_it < n_nodes; node_it++ )
  {
    gid = net_.local_nodes_.get_node_by_index( node_it )->get_gid();
    n = net_.local_nodes_.get_node_by_index( node_it )->get_synaptic_elements_vacant( se_name );
    if ( n > 0 )
    {
      ( *vacant_id_it ) = gid;
      ( *vacant_n_it ) = n;
      n_vacant_id++;
      vacant_id_it++;
      vacant_n_it++;
    }
    if ( n < 0 )
    {
      ( *deleted_id_it ) = gid;
      ( *deleted_n_it ) = n;
      n_deleted_id++;
      deleted_id_it++;
      deleted_n_it++;
    }
  }
  se_vacant_id.resize( n_vacant_id );
  se_vacant_n.resize( n_vacant_id );
  se_deleted_id.resize( n_deleted_id );
  se_deleted_n.resize( n_deleted_id );
}

void
nest::SPManager::get_sources( std::vector< index > targets,
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
nest::SPManager::get_targets( std::vector< index > sources,
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

void
nest::SPManager::serialize_id( std::vector< index >& id,
  std::vector< int_t >& n,
  std::vector< index >& res )
{
  // populate res with indexes of nodes corresponding to the number of elements
  res.clear();
  std::vector< index >::iterator id_it;
  std::vector< int_t >::iterator n_it;
  int_t j;
  id_it = id.begin();
  n_it = n.begin();
  for ( ; id_it != id.end() && n_it != n.end(); id_it++, n_it++ )
  {
    for ( j = 0; j < ( *n_it ); j++ )
    {
      res.push_back( *id_it );
    }
  }
}

void
nest::SPManager::global_shuffle( std::vector< index >& v )
{
  global_shuffle( v, v.size() );
}

/*
 * Shuffles the n first items of the vector v
 */
void
nest::SPManager::global_shuffle( std::vector< index >& v, size_t n )
{
  assert( n <= v.size() );

  // shuffle res using the global random number generator
  uint N = v.size();
  std::vector< index > v2;
  index tmp;
  uint rnd;
  std::vector< index >::iterator rndi;
  for ( uint i = 0; i < n; i++ )
  {
    N = v.size();
    rnd = net_.scheduler_.get_grng()->ulrand( N );
    tmp = v[ rnd ];
    v2.push_back( tmp );
    rndi = v.begin();
    v.erase( rndi + rnd );
  }
  v = v2;
}

} // namespace nest
