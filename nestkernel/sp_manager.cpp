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

#include "sp_manager.h"

// C++ includes:
#include <algorithm>

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_parameter.h"
#include "connection_manager.h"
#include "connector_base.h"
#include "connector_model.h"
#include "kernel_manager.h"
#include "logging.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "nest.h"
#include "nest_names.h"

#include "node_manager.h"

namespace nest
{

SPManager::SPManager()
  : ManagerInterface()
  , structural_plasticity_update_interval_( 10000. )
  , structural_plasticity_enabled_( false )
  , sp_conn_builders_()
  , growthcurve_factories_()
  , growthcurvedict_( new Dictionary() )
{
}

SPManager::~SPManager()
{
}

void
SPManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    // Add MSP growth curves
    register_growth_curve< GrowthCurveSigmoid >( "sigmoid" );
    register_growth_curve< GrowthCurveGaussian >( "gaussian" );
    register_growth_curve< GrowthCurveLinear >( "linear" );
  }

  structural_plasticity_update_interval_ = 10000.;
  structural_plasticity_enabled_ = false;
}

void
SPManager::finalize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    for ( auto spcb : sp_conn_builders_ )
    {
      delete spcb;
    }
    sp_conn_builders_.clear();

    for ( auto gcf : growthcurve_factories_ )
    {
      delete gcf;
    }
    growthcurve_factories_.clear();
    growthcurvedict_->clear();
  }
}

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
    const std::string model =
      kernel::manager< ModelManager >.get_connection_model( ( *i )->get_synapse_model(), 0 ).get_name();
    def< std::string >( sp_synapse, names::synapse_model, model );
    def< bool >( sp_synapse, names::allow_autapses, ( *i )->allows_autapses() );
    def< bool >( sp_synapse, names::allow_multapses, ( *i )->allows_multapses() );

    def< DictionaryDatum >( sp_synapses, ( *i )->get_name(), sp_synapse );
  }

  def< double >( d, names::structural_plasticity_update_interval, structural_plasticity_update_interval_ );

  ArrayDatum growth_curves;
  for ( auto const& element : *growthcurvedict_ )
  {
    growth_curves.push_back( new LiteralDatum( element.first ) );
  }
  def< ArrayDatum >( d, names::growth_curves, growth_curves );
}

void
SPManager::set_status( const DictionaryDatum& d )
{
  updateValue< double >( d, names::structural_plasticity_update_interval, structural_plasticity_update_interval_ );

  if ( not d->known( names::structural_plasticity_synapses ) )
  {
    return;
  }

  // Configure synapses model updated during the simulation.
  Token synmodel;
  DictionaryDatum syn_specs;
  DictionaryDatum syn_spec;
  DictionaryDatum conn_spec = DictionaryDatum( new Dictionary() );
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
    if ( syn_spec->known( names::allow_autapses ) )
    {
      def< bool >( conn_spec, names::allow_autapses, getValue< bool >( syn_spec, names::allow_autapses ) );
    }
    if ( syn_spec->known( names::allow_multapses ) )
    {
      def< bool >( conn_spec, names::allow_multapses, getValue< bool >( syn_spec, names::allow_multapses ) );
    }

    // We use a ConnBuilder with dummy values to check the synapse parameters
    SPBuilder* conn_builder = new SPBuilder( sources, targets, /* third_out */ nullptr, conn_spec, { syn_spec } );
    conn_builder->set_name( i->first.toString() );

    // check that the user defined the min and max delay properly, if the
    // default delay is not used.
    if ( not conn_builder->get_default_delay()
      and not kernel::manager< ConnectionManager >.get_user_set_delay_extrema() )
    {
      throw BadProperty(
        "Structural Plasticity: to use different delays for synapses you must "
        "specify the min and max delay in the kernel parameters." );
    }
    sp_conn_builders_.push_back( conn_builder );
  }
}

long
SPManager::builder_min_delay() const
{
  long min_delay = Time::pos_inf().get_steps();
  long builder_delay = Time::pos_inf().get_steps();

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    ( *i )->update_delay( builder_delay );
    min_delay = std::min( min_delay, builder_delay );
  }
  return min_delay;
}

long
SPManager::builder_max_delay() const
{
  long max_delay = Time::neg_inf().get_steps();
  long builder_delay = Time::neg_inf().get_steps();

  for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
  {
    ( *i )->update_delay( builder_delay );
    max_delay = std::max( max_delay, builder_delay );
  }
  return max_delay;
}

void
SPManager::disconnect( const size_t snode_id, Node* target, size_t target_thread, const size_t syn_id )
{
  Node* const source = kernel::manager< NodeManager >.get_node_or_proxy( snode_id );
  // normal nodes and devices with proxies
  if ( target->has_proxies() )
  {
    kernel::manager< ConnectionManager >.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
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
      target = kernel::manager< NodeManager >.get_node_or_proxy( target->get_node_id(), target_thread );
    }

    kernel::manager< ConnectionManager >.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
  }
  else // globally receiving devices iterate over all target threads
  {
    // we do not allow to connect a device to a global receiver at the moment
    if ( not source->has_proxies() )
    {
      return;
    }
    const size_t n_threads = kernel::manager< VPManager >.get_num_threads();
    for ( size_t t = 0; t < n_threads; t++ )
    {
      target = kernel::manager< NodeManager >.get_node_or_proxy( target->get_node_id(), t );
      target_thread = target->get_thread();
      kernel::manager< ConnectionManager >.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
    }
  }
}

void
SPManager::disconnect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  DictionaryDatum& conn_spec,
  DictionaryDatum& syn_spec )
{
  // probably not strictly necessarye here, but does nothing if all is up to date
  kernel::manager< NodeManager >.update_thread_local_node_data();

  if ( kernel::manager< ConnectionManager >.connections_have_changed() )
  {
#pragma omp parallel
    {
      const size_t tid = kernel::manager< VPManager >.get_thread_id();
      kernel::manager< SimulationManager >.update_connection_infrastructure( tid );
    }
  }

  BipartiteConnBuilder* cb = nullptr;
  conn_spec->clear_access_flags();
  syn_spec->clear_access_flags();

  if ( not conn_spec->known( names::rule ) )
  {
    throw BadProperty( "Disconnection spec must contain disconnection rule." );
  }
  const std::string rule_name = ( *conn_spec )[ names::rule ];

  if ( not kernel::manager< ConnectionManager >.valid_connection_rule( rule_name ) )
  {
    throw BadProperty( "Unknown connectivity rule: " + rule_name );
  }

  if ( not sp_conn_builders_.empty() )
  { // Implement a getter for sp_conn_builders_

    for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
    {
      std::string synModel = getValue< std::string >( syn_spec, names::synapse_model );
      if ( ( *i )->get_synapse_model() == kernel::manager< ModelManager >.get_synapse_model_id( synModel ) )
      {
        cb = kernel::manager< ConnectionManager >.get_conn_builder( rule_name,
          sources,
          targets,
          /* third_out */ nullptr,
          conn_spec,
          { syn_spec } );
        cb->set_synaptic_element_names(
          ( *i )->get_pre_synaptic_element_name(), ( *i )->get_post_synaptic_element_name() );
      }
    }
  }
  else
  {
    cb = kernel::manager< ConnectionManager >.get_conn_builder( rule_name,
      sources,
      targets,
      /* third_out */ nullptr,
      conn_spec,
      { syn_spec } );
  }
  assert( cb );

  // at this point, all entries in conn_spec and syn_spec have been checked
  ALL_ENTRIES_ACCESSED( *conn_spec, "Connect", "Unread dictionary entries: " );
  ALL_ENTRIES_ACCESSED( *syn_spec, "Connect", "Unread dictionary entries: " );

  // Set flag before calling cb->disconnect() in case exception is thrown after some connections have been removed.
  kernel::manager< ConnectionManager >.set_connections_have_changed();
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

void
SPManager::update_structural_plasticity( SPBuilder* sp_builder )
{
  // Index of neurons having a vacant synaptic element
  std::vector< size_t > pre_vacant_id;  // pre synaptic elements (e.g Axon)
  std::vector< size_t > post_vacant_id; // postsynaptic element (e.g Den)
  std::vector< int > pre_vacant_n;      // number of synaptic elements
  std::vector< int > post_vacant_n;     // number of synaptic elements

  // Index of neuron deleting a synaptic element
  std::vector< size_t > pre_deleted_id, post_deleted_id;
  std::vector< int > pre_deleted_n, post_deleted_n;

  // Global vector for vacant and deleted synaptic element
  std::vector< size_t > pre_vacant_id_global, post_vacant_id_global;
  std::vector< int > pre_vacant_n_global, post_vacant_n_global;
  std::vector< size_t > pre_deleted_id_global, post_deleted_id_global;
  std::vector< int > pre_deleted_n_global, post_deleted_n_global;

  // Vector of displacements for communication
  std::vector< int > displacements;

  // Get pre synaptic elements data from global nodes
  get_synaptic_elements(
    sp_builder->get_pre_synaptic_element_name(), pre_vacant_id, pre_vacant_n, pre_deleted_id, pre_deleted_n );

  // Communicate the number of deleted pre-synaptic elements
  kernel::manager< MPIManager >.communicate( pre_deleted_id, pre_deleted_id_global, displacements );
  kernel::manager< MPIManager >.communicate( pre_deleted_n, pre_deleted_n_global, displacements );

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
  // Get postsynaptic elements data from local nodes
  get_synaptic_elements(
    sp_builder->get_post_synaptic_element_name(), post_vacant_id, post_vacant_n, post_deleted_id, post_deleted_n );
  // Communicate the number of deleted postsynaptic elements
  kernel::manager< MPIManager >.communicate( post_deleted_id, post_deleted_id_global, displacements );
  kernel::manager< MPIManager >.communicate( post_deleted_n, post_deleted_n_global, displacements );

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
  kernel::manager< MPIManager >.communicate( pre_vacant_id, pre_vacant_id_global, displacements );
  kernel::manager< MPIManager >.communicate( pre_vacant_n, pre_vacant_n_global, displacements );
  kernel::manager< MPIManager >.communicate( post_vacant_id, post_vacant_id_global, displacements );
  kernel::manager< MPIManager >.communicate( post_vacant_n, post_vacant_n_global, displacements );

  bool synapses_created = false;
  if ( pre_vacant_id_global.size() > 0 and post_vacant_id_global.size() > 0 )
  {
    synapses_created = create_synapses(
      pre_vacant_id_global, pre_vacant_n_global, post_vacant_id_global, post_vacant_n_global, sp_builder );
  }
  if ( synapses_created or post_deleted_id.size() > 0 or pre_deleted_id.size() > 0 )
  {
    kernel::manager< ConnectionManager >.set_connections_have_changed();
  }
}

bool
SPManager::create_synapses( std::vector< size_t >& pre_id,
  std::vector< int >& pre_n,
  std::vector< size_t >& post_id,
  std::vector< int >& post_n,
  SPBuilder* sp_conn_builder )
{
  std::vector< size_t > pre_id_rnd;
  std::vector< size_t > post_id_rnd;

  // shuffle the vacant element
  serialize_id( pre_id, pre_n, pre_id_rnd );
  serialize_id( post_id, post_n, post_id_rnd );

  // Shuffle only the largest vector
  if ( pre_id_rnd.size() > post_id_rnd.size() )
  {
    // we only shuffle the n first items,
    // where n is the number of postsynaptic elements
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

  return not pre_id_rnd.empty();
}

void
SPManager::delete_synapses_from_pre( const std::vector< size_t >& pre_deleted_id,
  std::vector< int >& pre_deleted_n,
  const size_t synapse_model,
  const std::string& se_pre_name,
  const std::string& se_post_name )
{
  // Synapses deletion due to the loss of a pre-synaptic element need a
  // communication of the lists of target

  // Connectivity
  std::vector< std::vector< size_t > > connectivity;
  std::vector< size_t > global_targets;
  std::vector< int > displacements;

  // iterators
  std::vector< std::vector< size_t > >::iterator connectivity_it;
  std::vector< size_t >::const_iterator id_it;
  std::vector< int >::iterator n_it;

  kernel::manager< ConnectionManager >.get_targets( pre_deleted_id, synapse_model, se_post_name, connectivity );

  id_it = pre_deleted_id.begin();
  n_it = pre_deleted_n.begin();
  connectivity_it = connectivity.begin();
  for ( ; id_it != pre_deleted_id.end() and n_it != pre_deleted_n.end(); id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of targets
    kernel::manager< MPIManager >.communicate( *connectivity_it, global_targets, displacements );
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

void
SPManager::delete_synapse( const size_t snode_id,
  const size_t tnode_id,
  const long syn_id,
  const std::string se_pre_name,
  const std::string se_post_name )
{
  // get thread id
  const size_t tid = kernel::manager< VPManager >.get_thread_id();
  if ( kernel::manager< NodeManager >.is_local_node_id( snode_id ) )
  {
    Node* const source = kernel::manager< NodeManager >.get_node_or_proxy( snode_id );
    const size_t source_thread = source->get_thread();
    if ( tid == source_thread )
    {
      source->connect_synaptic_element( se_pre_name, -1 );
    }
  }

  if ( kernel::manager< NodeManager >.is_local_node_id( tnode_id ) )
  {
    Node* const target = kernel::manager< NodeManager >.get_node_or_proxy( tnode_id );
    const size_t target_thread = target->get_thread();
    if ( tid == target_thread )
    {
      kernel::manager< ConnectionManager >.disconnect( tid, syn_id, snode_id, tnode_id );

      target->connect_synaptic_element( se_post_name, -1 );
    }
  }
}

void
SPManager::delete_synapses_from_post( std::vector< size_t >& post_deleted_id,
  std::vector< int >& post_deleted_n,
  size_t synapse_model,
  std::string se_pre_name,
  std::string se_post_name )
{
  // TODO: Synapses deletion due to the loss of a postsynaptic element can
  // be done locally (except for the update of the number of pre-synaptic
  // element)

  // Connectivity
  std::vector< std::vector< size_t > > connectivity;
  std::vector< size_t > global_sources;
  std::vector< int > displacements;

  // iterators
  std::vector< std::vector< size_t > >::iterator connectivity_it;
  std::vector< size_t >::iterator id_it;
  std::vector< int >::iterator n_it;

  // Retrieve the connected sources
  kernel::manager< ConnectionManager >.get_sources( post_deleted_id, synapse_model, connectivity );

  id_it = post_deleted_id.begin();
  n_it = post_deleted_n.begin();
  connectivity_it = connectivity.begin();

  for ( ; id_it != post_deleted_id.end() and n_it != post_deleted_n.end(); id_it++, n_it++, connectivity_it++ )
  {
    // Communicate the list of sources
    kernel::manager< MPIManager >.communicate( *connectivity_it, global_sources, displacements );
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
  std::vector< size_t >& se_vacant_id,
  std::vector< int >& se_vacant_n,
  std::vector< size_t >& se_deleted_id,
  std::vector< int >& se_deleted_n )
{
  // local nodes
  size_t n_vacant_id = 0;
  size_t n_deleted_id = 0;
  size_t node_id;
  int n;
  size_t n_nodes = kernel::manager< NodeManager >.size();
  se_vacant_id.clear();
  se_vacant_n.clear();
  se_deleted_id.clear();
  se_deleted_n.clear();

  se_vacant_id.resize( n_nodes );
  se_vacant_n.resize( n_nodes );
  se_deleted_id.resize( n_nodes );
  se_deleted_n.resize( n_nodes );

  std::vector< size_t >::iterator vacant_id_it = se_vacant_id.begin();
  std::vector< int >::iterator vacant_n_it = se_vacant_n.begin();
  std::vector< size_t >::iterator deleted_id_it = se_deleted_id.begin();
  std::vector< int >::iterator deleted_n_it = se_deleted_n.begin();
  SparseNodeArray::const_iterator node_it;

  for ( size_t tid = 0; tid < kernel::manager< VPManager >.get_num_threads(); ++tid )
  {
    const SparseNodeArray& local_nodes = kernel::manager< NodeManager >.get_local_nodes( tid );
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
nest::SPManager::serialize_id( std::vector< size_t >& id, std::vector< int >& n, std::vector< size_t >& res )
{
  // populate res with indexes of nodes corresponding to the number of elements
  res.clear();
  std::vector< size_t >::iterator id_it;
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
nest::SPManager::global_shuffle( std::vector< size_t >& v )
{
  global_shuffle( v, v.size() );
}

void
nest::SPManager::global_shuffle( std::vector< size_t >& v, size_t n )
{
  assert( n <= v.size() );

  // shuffle res using the global random number generator
  unsigned int N = v.size();
  std::vector< size_t > v2;
  size_t tmp;
  unsigned int rnd;
  std::vector< size_t >::iterator rndi;
  for ( unsigned int i = 0; i < n; i++ )
  {
    N = v.size();
    rnd = get_rank_synced_rng()->ulrand( N );
    tmp = v[ rnd ];
    v2.push_back( tmp );
    rndi = v.begin();
    v.erase( rndi + rnd );
  }
  v = v2;
}


void
nest::SPManager::enable_structural_plasticity()
{
  if ( kernel::manager< VPManager >.get_num_threads() > 1 )
  {
    throw KernelException( "Structural plasticity can not be used with multiple threads" );
  }
  if ( not kernel::manager< ConnectionManager >.get_keep_source_table() )
  {
    throw KernelException(
      "Structural plasticity can not be enabled if keep_source_table has been "
      "set to false." );
  }
  if ( not kernel::manager< ConnectionManager >.use_compressed_spikes() )
  {
    throw KernelException(
      "Structural plasticity can not be enabled if use_compressed_spikes "
      "has been set to false." );
  }
  structural_plasticity_enabled_ = true;
}

void
nest::SPManager::disable_structural_plasticity()
{
  structural_plasticity_enabled_ = false;
}


double
SPManager::get_structural_plasticity_update_interval() const
{

  return structural_plasticity_update_interval_;
}

bool
SPManager::is_structural_plasticity_enabled() const
{

  return structural_plasticity_enabled_;
}

GrowthCurve*
SPManager::new_growth_curve( Name name )
{

  const long nc_id = ( *growthcurvedict_ )[ name ];
  return growthcurve_factories_.at( nc_id )->create();
}
} // namespace nest
