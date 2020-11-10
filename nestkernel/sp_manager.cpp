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

// C++ includes:
#include <algorithm>

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_parameter.h"
#include "connector_base.h"
#include "connector_model.h"
#include "kernel_manager.h"
#include "nest_names.h"

namespace nest
{

template < typename T >
void
print_vector( const std::vector< T >& vec )
{
  std::cout << "#######BEGIN############################\n";
  for ( typename std::vector< T >::const_iterator cit = vec.begin(); cit != vec.end(); ++cit )
  {
    std::cout << *cit << ", ";
  }
  std::cout << "########END############################\n";
}

SPManager::SPManager()
  : ManagerInterface()
  , structural_plasticity_update_interval_( 10000. )
  , structural_plasticity_enabled_( false )
  , sp_conn_builders_()
  , growthcurvedict_( new Dictionary() )
  , growthcurve_factories_()
{
}

SPManager::~SPManager()
{
  finalize();
}

void
SPManager::initialize()
{
  structural_plasticity_update_interval_ = 10000.;
  structural_plasticity_enabled_ = false;
}

void
SPManager::finalize()
{
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    delete *i;
  }
  sp_conn_builders_.clear();
}

/*
 * Methods to retrieve data regarding structural plasticity variables
 */
void
SPManager::get_status( DictionaryDatum& d )
{
  DictionaryDatum sp_synapses = DictionaryDatum( new Dictionary() );
  DictionaryDatum sp_synapse;
  def< DictionaryDatum >( d, names::structural_plasticity_synapses, sp_synapses );
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    sp_synapse = DictionaryDatum( new Dictionary() );
    def< std::string >( sp_synapse, names::pre_synaptic_element, ( *i )->get_pre_synaptic_element_name() );
    def< std::string >( sp_synapse, names::post_synaptic_element, ( *i )->get_post_synaptic_element_name() );
    def< std::string >( sp_synapse,
      names::synapse_model,
      kernel().model_manager.get_synapse_prototype( ( *i )->get_synapse_model(), 0 ).get_name() );
    std::stringstream syn_name;
    syn_name << "syn" << ( sp_conn_builders_.end() - i );
    def< DictionaryDatum >( sp_synapses, syn_name.str(), sp_synapse );
  }

  def< double >( d, names::structural_plasticity_update_interval, structural_plasticity_update_interval_ );
}

/**
 * Set status of synaptic plasticity variables: synaptic update interval,
 * synapses and synaptic elements.
 * @param d Dictionary containing the values to be set
 */
void
SPManager::set_status( const DictionaryDatum& d )
{
  if ( d->known( names::structural_plasticity_update_interval ) )
  {
    updateValue< double >( d, names::structural_plasticity_update_interval, structural_plasticity_update_interval_ );
  }
  if ( not d->known( names::structural_plasticity_synapses ) )
  {
    return;
  } /*
    * Configure synapses model updated during the simulation.
    */
  Token synmodel;
  DictionaryDatum syn_specs, syn_spec;
  DictionaryDatum conn_spec = DictionaryDatum( new Dictionary() );

  if ( d->known( names::allow_autapses ) )
  {
    def< bool >( conn_spec, names::allow_autapses, getValue< bool >( d, names::allow_autapses ) );
  }
  if ( d->known( names::allow_multapses ) )
  {
    def< bool >( conn_spec, names::allow_multapses, getValue< bool >( d, names::allow_multapses ) );
  }
  NodeCollectionPTR sources( new NodeCollectionPrimitive() );
  NodeCollectionPTR targets( new NodeCollectionPrimitive() );

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    delete ( *i );
  }
  sp_conn_builders_.clear();
  updateValue< DictionaryDatum >( d, names::structural_plasticity_synapses, syn_specs );
  for ( Dictionary::const_iterator i = syn_specs->begin(); i != syn_specs->end(); ++i )
  {
    syn_spec = getValue< DictionaryDatum >( syn_specs, i->first );
    // We use a ConnBuilder with dummy values to check the synapse parameters
    SPBuilder* conn_builder = new SPBuilder( sources, targets, conn_spec, { syn_spec } );

    // check that the user defined the min and max delay properly, if the
    // default delay is not used.
    if ( not conn_builder->get_default_delay() and not kernel().connection_manager.get_user_set_delay_extrema() )
    {
      throw BadProperty(
        "Structural Plasticity: to use different delays for synapses you must "
        "specify the min and max delay in the kernel parameters." );
    }
    sp_conn_builders_.push_back( conn_builder );
  }
}

delay
SPManager::builder_min_delay() const
{
  delay min_delay = Time::pos_inf().get_steps();
  delay builder_delay = Time::pos_inf().get_steps();

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    ( *i )->update_delay( builder_delay );
    min_delay = std::min( min_delay, builder_delay );
  }
  return min_delay;
}

delay
SPManager::builder_max_delay() const
{
  delay max_delay = Time::neg_inf().get_steps();
  delay builder_delay = Time::neg_inf().get_steps();

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    ( *i )->update_delay( builder_delay );
    max_delay = std::max( max_delay, builder_delay );
  }
  return max_delay;
}

/**
 * Deletes synapses between a source and a target.
 * @param snode_id
 * @param target
 * @param target_thread
 * @param syn_id
 */
void
SPManager::disconnect( const index snode_id, Node* target, thread target_thread, const index syn_id )
{
  Node* const source = kernel().node_manager.get_node_or_proxy( snode_id );
  // normal nodes and devices with proxies
  if ( target->has_proxies() )
  {
    kernel().connection_manager.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
  }
  else if ( target->local_receiver() ) // normal devices
  {
    if ( source->is_proxy() )
    {
      return;
    }
    if ( ( source->get_thread() != target_thread ) and ( source->has_proxies() ) )
    {
      target_thread = source->get_thread();
      target = kernel().node_manager.get_node_or_proxy( target->get_node_id(), target_thread );
    }

    kernel().connection_manager.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
  }
  else // globally receiving devices iterate over all target threads
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return;
    }
    const thread n_threads = kernel().vp_manager.get_num_threads();
    for ( thread t = 0; t < n_threads; t++ )
    {
      target = kernel().node_manager.get_node_or_proxy( target->get_node_id(), t );
      target_thread = target->get_thread();
      kernel().connection_manager.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
    }
  }
}

/**
 * Obtains the right connection builder and performs a synapse deletion
 * according to the specified connection specs.
 * @param sources collection of sources
 * @param targets collection of targets
 * @param conn_spec disconnection specs. For now only all to all and one to one
 * rules are implemented.
 * @param syn_spec synapse specs
 */
void
SPManager::disconnect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  DictionaryDatum& conn_spec,
  DictionaryDatum& syn_spec )
{
  if ( kernel().connection_manager.have_connections_changed() )
  {
    if ( kernel().connection_manager.secondary_connections_exist() )
    {
      kernel().model_manager.create_secondary_events_prototypes(); // necessary before
                                                                   // updating
                                                                   // connection
                                                                   // infrastructure
    }
#pragma omp parallel
    {
      const thread tid = kernel().vp_manager.get_thread_id();
      kernel().simulation_manager.update_connection_infrastructure( tid );
    }
  }

  ConnBuilder* cb = NULL;
  conn_spec->clear_access_flags();
  syn_spec->clear_access_flags();

  if ( not conn_spec->known( names::rule ) )
  {
    throw BadProperty( "Disconnection spec must contain disconnection rule." );
  }
  const std::string rule_name = ( *conn_spec )[ names::rule ];

  if ( not kernel().connection_manager.get_connruledict()->known( rule_name ) )
  {
    throw BadProperty( "Unknown connectivty rule: " + rule_name );
  }

  if ( not sp_conn_builders_.empty() )
  { // Implement a getter for sp_conn_builders_

    for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
    {
      std::string synModel = getValue< std::string >( syn_spec, names::synapse_model );
      if ( ( *i )->get_synapse_model() == ( index )( kernel().model_manager.get_synapsedict()->lookup( synModel ) ) )
      {
        cb = kernel().connection_manager.get_conn_builder( rule_name, sources, targets, conn_spec, { syn_spec } );
        cb->set_post_synaptic_element_name( ( *i )->get_post_synaptic_element_name() );
        cb->set_pre_synaptic_element_name( ( *i )->get_pre_synaptic_element_name() );
      }
    }
  }
  else
  {
    cb = kernel().connection_manager.get_conn_builder( rule_name, sources, targets, conn_spec, { syn_spec } );
  }
  assert( cb != 0 );

  // at this point, all entries in conn_spec and syn_spec have been checked
  ALL_ENTRIES_ACCESSED( *conn_spec, "Connect", "Unread dictionary entries: " );
  ALL_ENTRIES_ACCESSED( *syn_spec, "Connect", "Unread dictionary entries: " );

  cb->disconnect();

  delete cb;
}

void
SPManager::update_structural_plasticity()
{
  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); ++i )
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
  std::vector< int > pre_vacant_n;     // number of synaptic elements
  std::vector< int > post_vacant_n;    // number of synaptic elements

  // Index of neuron deleting a synaptic element
  std::vector< index > pre_deleted_id, post_deleted_id;
  std::vector< int > pre_deleted_n, post_deleted_n;

  // Global vector for vacant and deleted synaptic element
  std::vector< index > pre_vacant_id_global, post_vacant_id_global;
  std::vector< int > pre_vacant_n_global, post_vacant_n_global;
  std::vector< index > pre_deleted_id_global, post_deleted_id_global;
  std::vector< int > pre_deleted_n_global, post_deleted_n_global;

  // Vector of displacements for communication
  std::vector< int > displacements;

  // Get pre synaptic elements data from global nodes
  get_synaptic_elements(
    sp_builder->get_pre_synaptic_element_name(), pre_vacant_id, pre_vacant_n, pre_deleted_id, pre_deleted_n );

  // Communicate the number of deleted pre-synaptic elements
  kernel().mpi_manager.communicate( pre_deleted_id, pre_deleted_id_global, displacements );
  kernel().mpi_manager.communicate( pre_deleted_n, pre_deleted_n_global, displacements );

  if ( pre_deleted_id_global.size() > 0 )
  {
    delete_synapses_from_pre( pre_deleted_id_global,
      pre_deleted_n_global,
      sp_builder->get_synapse_model(),
      sp_builder->get_pre_synaptic_element_name(),
      sp_builder->get_post_synaptic_element_name() );
    // update the number of synaptic elements
    get_synaptic_elements(
      sp_builder->get_pre_synaptic_element_name(), pre_vacant_id, pre_vacant_n, pre_deleted_id, pre_deleted_n );
  }
  // Get post synaptic elements data from local nodes
  get_synaptic_elements(
    sp_builder->get_post_synaptic_element_name(), post_vacant_id, post_vacant_n, post_deleted_id, post_deleted_n );
  // Communicate the number of deleted post-synaptic elements
  kernel().mpi_manager.communicate( post_deleted_id, post_deleted_id_global, displacements );
  kernel().mpi_manager.communicate( post_deleted_n, post_deleted_n_global, displacements );

  if ( post_deleted_id_global.size() > 0 )
  {
    delete_synapses_from_post( post_deleted_id_global,
      post_deleted_n_global,
      sp_builder->get_synapse_model(),
      sp_builder->get_pre_synaptic_element_name(),
      sp_builder->get_post_synaptic_element_name() );
    get_synaptic_elements(
      sp_builder->get_pre_synaptic_element_name(), pre_vacant_id, pre_vacant_n, pre_deleted_id, pre_deleted_n );
    get_synaptic_elements(
      sp_builder->get_post_synaptic_element_name(), post_vacant_id, post_vacant_n, post_deleted_id, post_deleted_n );
  }

  // Communicate vacant elements
  kernel().mpi_manager.communicate( pre_vacant_id, pre_vacant_id_global, displacements );
  kernel().mpi_manager.communicate( pre_vacant_n, pre_vacant_n_global, displacements );
  kernel().mpi_manager.communicate( post_vacant_id, post_vacant_id_global, displacements );
  kernel().mpi_manager.communicate( post_vacant_n, post_vacant_n_global, displacements );

  if ( pre_vacant_id_global.size() > 0 and post_vacant_id_global.size() > 0 )
  {
    create_synapses(
      pre_vacant_id_global, pre_vacant_n_global, post_vacant_id_global, post_vacant_n_global, sp_builder );
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
  std::vector< int >& pre_n,
  std::vector< index >& post_id,
  std::vector< int >& post_n,
  SPBuilder* sp_conn_builder )
{
  std::vector< index > pre_id_rnd;
  std::vector< index > post_id_rnd;

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
  sp_conn_builder->sp_connect( pre_id_rnd, post_id_rnd );
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
SPManager::delete_synapses_from_pre( const std::vector< index >& pre_deleted_id,
  std::vector< int >& pre_deleted_n,
  const index synapse_model,
  const std::string& se_pre_name,
  const std::string& se_post_name )
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
  std::vector< index >::const_iterator id_it;
  std::vector< int >::iterator n_it;

  kernel().connection_manager.get_targets( pre_deleted_id, synapse_model, se_post_name, connectivity );

  id_it = pre_deleted_id.begin();
  n_it = pre_deleted_n.begin();
  connectivity_it = connectivity.begin();
  for ( ; id_it != pre_deleted_id.end() and n_it != pre_deleted_n.end(); id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of targets
    kernel().mpi_manager.communicate( *connectivity_it, global_targets, displacements );
    // shuffle only the first n items, n is the number of deleted synaptic
    // elements
    if ( -( *n_it ) > static_cast< int >( global_targets.size() ) )
    {
      *n_it = -global_targets.size();
    }
    global_shuffle( global_targets, -( *n_it ) );

    for ( int i = 0; i < -( *n_it ); ++i ) // n is negative
    {
      delete_synapse( *id_it, global_targets[ i ], synapse_model, se_pre_name, se_post_name );
    }
  }
}

/**
 * Handles the deletion of synapses between source and target nodes. The
 * deletion is defined by the pre and post synaptic elements and the synapse
 * type. Updates the number of connected synaptic elements in the source and
 * target.
 * @param snode_id source id
 * @param tnode_id target id
 * @param syn_id synapse type
 * @param se_pre_name name of the pre synaptic element
 * @param se_post_name name of the post synaptic element
 */
void
SPManager::delete_synapse( const index snode_id,
  const index tnode_id,
  const long syn_id,
  const std::string se_pre_name,
  const std::string se_post_name )
{
  // get thread id
  const int tid = kernel().vp_manager.get_thread_id();
  if ( kernel().node_manager.is_local_node_id( snode_id ) )
  {
    Node* const source = kernel().node_manager.get_node_or_proxy( snode_id );
    const thread source_thread = source->get_thread();
    if ( tid == source_thread )
    {
      source->connect_synaptic_element( se_pre_name, -1 );
    }
  }

  if ( kernel().node_manager.is_local_node_id( tnode_id ) )
  {
    Node* const target = kernel().node_manager.get_node_or_proxy( tnode_id );
    const thread target_thread = target->get_thread();
    if ( tid == target_thread )
    {
      kernel().connection_manager.disconnect( tid, syn_id, snode_id, tnode_id );

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
  std::vector< int >& post_deleted_n,
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
  std::vector< int >::iterator n_it;

  // Retrieve the connected sources
  kernel().connection_manager.get_sources( post_deleted_id, synapse_model, connectivity );

  id_it = post_deleted_id.begin();
  n_it = post_deleted_n.begin();
  connectivity_it = connectivity.begin();

  for ( ; id_it != post_deleted_id.end() and n_it != post_deleted_n.end(); id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of sources
    kernel().mpi_manager.communicate( *connectivity_it, global_sources, displacements );
    // shuffle only the first n items, n is the number of deleted synaptic
    // elements
    if ( -( *n_it ) > static_cast< int >( global_sources.size() ) )
    {
      *n_it = -global_sources.size();
    }
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
  std::vector< int >& se_vacant_n,
  std::vector< index >& se_deleted_id,
  std::vector< int >& se_deleted_n )
{
  // local nodes
  index n_vacant_id = 0;
  index n_deleted_id = 0;
  index node_id;
  int n;
  size_t n_nodes = kernel().node_manager.size();
  se_vacant_id.clear();
  se_vacant_n.clear();
  se_deleted_id.clear();
  se_deleted_n.clear();

  se_vacant_id.resize( n_nodes );
  se_vacant_n.resize( n_nodes );
  se_deleted_id.resize( n_nodes );
  se_deleted_n.resize( n_nodes );

  std::vector< index >::iterator vacant_id_it = se_vacant_id.begin();
  std::vector< int >::iterator vacant_n_it = se_vacant_n.begin();
  std::vector< index >::iterator deleted_id_it = se_deleted_id.begin();
  std::vector< int >::iterator deleted_n_it = se_deleted_n.begin();
  SparseNodeArray::const_iterator node_it;

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    const SparseNodeArray& local_nodes = kernel().node_manager.get_local_nodes( tid );
    SparseNodeArray::const_iterator node_it;
    for ( node_it = local_nodes.begin(); node_it < local_nodes.end(); node_it++ )
    {
      node_id = node_it->get_node_id();
      Node* node = node_it->get_node();
      n = node->get_synaptic_elements_vacant( se_name );
      if ( n > 0 )
      {
        ( *vacant_id_it ) = node_id;
        ( *vacant_n_it ) = n;
        n_vacant_id++;
        vacant_id_it++;
        vacant_n_it++;
      }
      if ( n < 0 )
      {
        ( *deleted_id_it ) = node_id;
        ( *deleted_n_it ) = n;
        n_deleted_id++;
        deleted_id_it++;
        deleted_n_it++;
      }
    }
  }
  se_vacant_id.resize( n_vacant_id );
  se_vacant_n.resize( n_vacant_id );
  se_deleted_id.resize( n_deleted_id );
  se_deleted_n.resize( n_deleted_id );
}

void
nest::SPManager::serialize_id( std::vector< index >& id, std::vector< int >& n, std::vector< index >& res )
{
  // populate res with indexes of nodes corresponding to the number of elements
  res.clear();
  std::vector< index >::iterator id_it;
  std::vector< int >::iterator n_it;
  int j;
  id_it = id.begin();
  n_it = n.begin();
  for ( ; id_it != id.end() and n_it != n.end(); id_it++, n_it++ )
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
  unsigned int N = v.size();
  std::vector< index > v2;
  index tmp;
  unsigned int rnd;
  std::vector< index >::iterator rndi;
  for ( unsigned int i = 0; i < n; i++ )
  {
    N = v.size();
    rnd = kernel().rng_manager.get_grng()->ulrand( N );
    tmp = v[ rnd ];
    v2.push_back( tmp );
    rndi = v.begin();
    v.erase( rndi + rnd );
  }
  v = v2;
}


/*
 * Enable structural plasticity
 */
void
nest::SPManager::enable_structural_plasticity()
{
  if ( kernel().vp_manager.get_num_threads() > 1 )
  {
    throw KernelException( "Structural plasticity can not be used with multiple threads" );
  }
  if ( not kernel().connection_manager.get_keep_source_table() )
  {
    throw KernelException(
      "Structural plasticity can not be enabled if keep_source_table has been "
      "set to false." );
  }
  if ( not kernel().connection_manager.get_sort_connections_by_source() )
  {
    throw KernelException(
      "Structural plasticity can not be enabled if sort_connections_by_source "
      "has been set to false." );
  }
  structural_plasticity_enabled_ = true;
}

/*
 Disable  structural plasticity
 */
void
nest::SPManager::disable_structural_plasticity()
{
  structural_plasticity_enabled_ = false;
}

} // namespace nest
