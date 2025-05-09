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
#include <random>

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_parameter.h"
#include "connector_base.h"
#include "connector_model.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "sp_manager_impl.h"
#include "spatial.h"

namespace nest
{

SPManager::SPManager()
  : ManagerInterface()
  , structural_plasticity_update_interval_( 10000. )
  , structural_plasticity_enabled_( false )
  , structural_plasticity_gaussian_kernel_sigma_( -1 )
  , structural_plasticity_cache_probabilities_( false )
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
  structural_plasticity_gaussian_kernel_sigma_ = -1;
  structural_plasticity_cache_probabilities_ = false;
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
    const std::string model = kernel().model_manager.get_connection_model( ( *i )->get_synapse_model(), 0 ).get_name();
    def< std::string >( sp_synapse, names::synapse_model, model );
    def< bool >( sp_synapse, names::allow_autapses, ( *i )->allows_autapses() );
    def< bool >( sp_synapse, names::allow_multapses, ( *i )->allows_multapses() );

    def< DictionaryDatum >( sp_synapses, ( *i )->get_name(), sp_synapse );
  }

  def< double >( d, names::structural_plasticity_update_interval, structural_plasticity_update_interval_ );
  def< double >( d, names::structural_plasticity_gaussian_kernel_sigma, structural_plasticity_gaussian_kernel_sigma_ );
  def< bool >( d, names::structural_plasticity_cache_probabilities, structural_plasticity_cache_probabilities_ );


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
  updateValue< double >(
    d, names::structural_plasticity_gaussian_kernel_sigma, structural_plasticity_gaussian_kernel_sigma_ );
  updateValue< bool >(
    d, names::structural_plasticity_cache_probabilities, structural_plasticity_cache_probabilities_ );

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
    if ( not conn_builder->get_default_delay() and not kernel().connection_manager.get_user_set_delay_extrema() )
    {
      throw BadProperty(
        "Structural Plasticity: to use different delays for synapses you must "
        "specify the min and max delay in the kernel parameters." );
    }
    sp_conn_builders_.push_back( conn_builder );
  }
}

void
SPManager::gather_global_positions_and_ids()
{
  std::vector< double > local_positions;
  std::vector< int > local_ids;
  std::vector< int > displacements;

  // Collect local positions and IDs
  for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    const SparseNodeArray& local_nodes = kernel().node_manager.get_local_nodes( tid );

    for ( auto node_it = local_nodes.begin(); node_it < local_nodes.end(); ++node_it )
    {
      int node_id = node_it->get_node_id();
      if ( node_id < 1 )
      {
        throw std::runtime_error( "Invalid neuron ID (must be >= 1)." );
      }

      std::vector< double > pos = get_position( node_id );

      if ( std::none_of( pos.begin(), pos.end(), []( double v ) { return std::isnan( v ); } ) )
      {
        local_ids.push_back( node_id );
        local_positions.insert( local_positions.end(), pos.begin(), pos.end() );
      }
    }
  }

  // Communicate positions and IDs
  kernel().mpi_manager.communicate( local_positions, global_positions, displacements );
  kernel().mpi_manager.communicate( local_ids, global_ids, displacements );

  // Validate global_positions size consistency with global_ids
  size_t num_neurons = global_ids.size();
  size_t total_positions = global_positions.size();

  if ( num_neurons == 0 )
  {
    throw std::runtime_error(
      "No neurons with valid positions found. Please provide valid positions, or disable distance dependency." );
  }
  if ( total_positions == 0 )
  {
    throw std::runtime_error( "No positions found. Please provide positions, or disable distance dependency." );
  }

  if ( total_positions % num_neurons != 0 )
  {
    throw std::runtime_error( "Mismatch in global positions dimensionality." );
  }

  pos_dim = total_positions / num_neurons;

  // Pair global_ids with their positions
  std::vector< std::pair< int, std::vector< double > > > id_pos_pairs;
  id_pos_pairs.reserve( num_neurons );
  for ( size_t i = 0; i < num_neurons; ++i )
  {
    int node_id = global_ids[ i ];
    std::vector< double > pos( global_positions.begin() + i * pos_dim, global_positions.begin() + ( i + 1 ) * pos_dim );
    id_pos_pairs.emplace_back( node_id, pos );
  }

  // Sort id_pos_pairs based on node_id to ensure ordering from 1 to num_neurons
  std::sort( id_pos_pairs.begin(),
    id_pos_pairs.end(),
    []( const std::pair< int, std::vector< double > >& a, const std::pair< int, std::vector< double > >& b ) -> bool
    { return a.first < b.first; } );

  // Verify that IDs are sequential
  for ( size_t i = 0; i < num_neurons; ++i )
  {
    if ( id_pos_pairs[ i ].first != static_cast< int >( i + 1 ) )
    {
      throw std::runtime_error( "Neuron IDs are not sequential after sorting." );
    }
  }

  // Assign sorted positions to temp_positions
  std::vector< double > temp_positions( num_neurons * pos_dim, 0.0 );
  for ( size_t i = 0; i < num_neurons; ++i )
  {
    std::copy( id_pos_pairs[ i ].second.begin(), id_pos_pairs[ i ].second.end(), temp_positions.begin() + i * pos_dim );
  }

  // Update global_positions with sorted positions
  global_positions = std::move( temp_positions );
}

// This method uses a formula based on triangular numbers
// to map two ids to one index indepndent of theirs order
int
SPManager::get_neuron_pair_index( int id1, int id2 )
{
  int max_id = std::max( id1, id2 );
  int min_id = std::min( id1, id2 );
  int index = ( ( max_id ) * ( max_id - 1 ) ) / 2 + ( min_id - 1 );
  return index;
}


// Method to perform roulette wheel selection
int
SPManager::roulette_wheel_selection( const std::vector< double >& probabilities, double rnd )
{
  if ( probabilities.empty() )
  {
    throw std::runtime_error( "Probabilities vector is empty." );
  }

  std::vector< double > cumulative( probabilities.size() );
  std::partial_sum( probabilities.begin(), probabilities.end(), cumulative.begin() );

  // Ensure the sum of probabilities is greater than zero
  double sum = cumulative.back();
  if ( sum < 0.0 )
  {
    throw std::runtime_error( "Sum of probabilities must be greater than zero." );
  }


  // Generate a random number in the range [0, sum)
  double randomValue = rnd * sum;

  // Perform binary search to find the selected index
  auto it = std::lower_bound( cumulative.begin(), cumulative.end(), randomValue );
  return static_cast< int >( std::distance( cumulative.begin(), it ) );
}


double
SPManager::gaussian_kernel( const std::vector< double >& pos1, const std::vector< double >& pos2, const double sigma )
{
  double distanceSquared = 0.0;
  for ( size_t i = 0; i < pos1.size(); ++i )
  {
    double diff = pos2[ i ] - pos1[ i ];
    distanceSquared += diff * diff;
  }
  return std::exp( -distanceSquared / ( sigma * sigma ) );
}

void
SPManager::build_probability_list()
{
  size_t num_neurons = global_ids.size();

  if ( global_positions.size() % num_neurons != 0 )
  {
    throw std::runtime_error( "Mismatch in global positions dimensionality." );
  }

  // Resize the probability list to accommodate all neuron pairs.
  size_t total_pairs = ( num_neurons * ( num_neurons + 1 ) ) / 2;
  probability_list.resize( total_pairs, -1.0 );

  // Calculate probabilities for connections between all pairs of neurons.
  for ( size_t i = 0; i < num_neurons; ++i )
  {
    size_t id_i = i + 1;
    if ( id_i < 1 || id_i > num_neurons )
    {
      std::cerr << "Error: Neuron ID " << id_i << " out of valid range." << std::endl;
      continue;
    }

    std::vector< double > pos_i(
      global_positions.begin() + pos_dim * ( id_i - 1 ), global_positions.begin() + pos_dim * id_i );

    for ( size_t j = i; j < num_neurons; ++j )
    {
      size_t id_j = j + 1;
      if ( id_j < 1 || id_j > num_neurons )
      {
        std::cerr << "Error: Neuron ID " << id_j << " out of valid range." << std::endl;
        continue;
      }

      size_t index = get_neuron_pair_index( id_i, id_j );

      if ( index >= probability_list.size() )
      {
        std::cerr << "Error: Index out of bounds: " << index << " for ids " << id_i << " and " << id_j << std::endl;
        continue;
      }

      if ( id_i == id_j )
      {
        probability_list[ index ] = 0.0; // Assign zero probability for self-connections
      }
      else
      {
        std::vector< double > pos_j(
          global_positions.begin() + pos_dim * ( id_j - 1 ), global_positions.begin() + pos_dim * id_j );

        double prob = gaussian_kernel( pos_i, pos_j, structural_plasticity_gaussian_kernel_sigma_ );
        probability_list[ index ] = prob;
      }
    }
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
    const size_t n_threads = kernel().vp_manager.get_num_threads();
    for ( size_t t = 0; t < n_threads; t++ )
    {
      target = kernel().node_manager.get_node_or_proxy( target->get_node_id(), t );
      target_thread = target->get_thread();
      kernel().connection_manager.disconnect( target_thread, syn_id, snode_id, target->get_node_id() );
    }
  }
}

void
SPManager::disconnect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  DictionaryDatum& conn_spec,
  DictionaryDatum& syn_spec )
{
  if ( kernel().connection_manager.connections_have_changed() )
  {
#pragma omp parallel
    {
      const size_t tid = kernel().vp_manager.get_thread_id();
      kernel().simulation_manager.update_connection_infrastructure( tid );
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

  if ( not kernel().connection_manager.valid_connection_rule( rule_name ) )
  {
    throw BadProperty( "Unknown connectivity rule: " + rule_name );
  }

  if ( not sp_conn_builders_.empty() )
  { // Implement a getter for sp_conn_builders_

    for ( std::vector< SPBuilder* >::const_iterator i = sp_conn_builders_.begin(); i != sp_conn_builders_.end(); i++ )
    {
      std::string synModel = getValue< std::string >( syn_spec, names::synapse_model );
      if ( ( *i )->get_synapse_model() == kernel().model_manager.get_synapse_model_id( synModel ) )
      {
        cb = kernel().connection_manager.get_conn_builder( rule_name,
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
    cb = kernel().connection_manager.get_conn_builder( rule_name,
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
  kernel().connection_manager.set_connections_have_changed();
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
  // Get postsynaptic elements data from local nodes
  get_synaptic_elements(
    sp_builder->get_post_synaptic_element_name(), post_vacant_id, post_vacant_n, post_deleted_id, post_deleted_n );
  // Communicate the number of deleted postsynaptic elements
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

  bool synapses_created = false;
  if ( pre_vacant_id_global.size() > 0 and post_vacant_id_global.size() > 0 )
  {
    synapses_created = create_synapses(
      pre_vacant_id_global, pre_vacant_n_global, post_vacant_id_global, post_vacant_n_global, sp_builder );
  }
  if ( synapses_created or post_deleted_id.size() > 0 or pre_deleted_id.size() > 0 )
  {
    kernel().connection_manager.set_connections_have_changed();
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

  std::vector< size_t > pre_ids_results;
  std::vector< size_t > post_ids_results;

  if ( structural_plasticity_gaussian_kernel_sigma_ <= 0 )
  {
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

    pre_ids_results = pre_id_rnd;
    post_ids_results = post_id_rnd;
  }
  else
  {
    global_shuffle_spatial( pre_id_rnd, post_id_rnd, pre_ids_results, post_ids_results );
  }

  // create synapse
  sp_conn_builder->sp_connect( pre_ids_results, post_ids_results );

  return not pre_ids_results.empty();
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

void
SPManager::delete_synapse( const size_t snode_id,
  const size_t tnode_id,
  const long syn_id,
  const std::string se_pre_name,
  const std::string se_post_name )
{
  // get thread id
  const size_t tid = kernel().vp_manager.get_thread_id();
  if ( kernel().node_manager.is_local_node_id( snode_id ) )
  {
    Node* const source = kernel().node_manager.get_node_or_proxy( snode_id );
    const size_t source_thread = source->get_thread();
    if ( tid == source_thread )
    {
      source->connect_synaptic_element( se_pre_name, -1 );
    }
  }

  if ( kernel().node_manager.is_local_node_id( tnode_id ) )
  {
    Node* const target = kernel().node_manager.get_node_or_proxy( tnode_id );
    const size_t target_thread = target->get_thread();
    if ( tid == target_thread )
    {
      kernel().connection_manager.disconnect( tid, syn_id, snode_id, tnode_id );

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
  size_t n_nodes = kernel().node_manager.size();
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

  for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
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
SPManager::global_shuffle_spatial( std::vector< size_t >& pre_ids,
  std::vector< size_t >& post_ids,
  std::vector< size_t >& pre_ids_results,
  std::vector< size_t >& post_ids_results )
{
  size_t maxIterations = std::min( pre_ids.size(), post_ids.size() );

  for ( size_t iteration = 0; iteration < maxIterations; ++iteration )
  {
    if ( pre_ids.empty() || post_ids.empty() )
    {
      break; // Stop if either vector is empty
    }

    size_t pre_id = pre_ids.back();
    pre_ids.pop_back();

    std::vector< double > probabilities;
    std::vector< size_t > valid_post_ids;
    double rnd;
    for ( size_t post_id : post_ids )
    {
      if ( post_id == pre_id )
      {
        continue; // Skip self-connections
      }

      double prob;
      if ( structural_plasticity_cache_probabilities_ )
      {
        // Retrieve cached probability for the neuron pair
        int pair_index = get_neuron_pair_index( pre_id, post_id );
        if ( pair_index < 0 || pair_index >= static_cast< int >( probability_list.size() ) )
        {
          std::cerr << "Error: index out of bounds for pair (" << pre_id << ", " << post_id << ")" << std::endl;
          continue;
        }
        prob = probability_list[ pair_index ];
      }
      else
      {
        size_t pre_index = pre_id - 1;
        std::vector< double > pre_pos(
          global_positions.begin() + pre_index * pos_dim, global_positions.begin() + ( pre_index + 1 ) * pos_dim );

        size_t post_index = post_id - 1;
        std::vector< double > post_pos(
          global_positions.begin() + post_index * pos_dim, global_positions.begin() + ( post_index + 1 ) * pos_dim );

        prob = gaussian_kernel( pre_pos, post_pos, structural_plasticity_gaussian_kernel_sigma_ );
      }
      if ( prob > 0 )
      {
        probabilities.push_back( prob );
        valid_post_ids.push_back( post_id );
      }
    }

    if ( probabilities.empty() )
    {
      continue; // Skip if no valid connections are found
    }

    rnd = get_rank_synced_rng()->drand();

    // Select a post-synaptic neuron using roulette wheel selection
    int selected_post_idx = roulette_wheel_selection( probabilities, rnd );
    size_t selected_post_id = valid_post_ids[ selected_post_idx ];

    // Remove the selected post-synaptic neuron from the list
    auto post_it = std::find( post_ids.begin(), post_ids.end(), selected_post_id );
    if ( post_it != post_ids.end() )
    {
      post_ids.erase( post_it );
    }

    pre_ids_results.push_back( pre_id );
    post_ids_results.push_back( selected_post_id );
  }
}


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
  if ( not kernel().connection_manager.use_compressed_spikes() )
  {
    throw KernelException(
      "Structural plasticity can not be enabled if use_compressed_spikes "
      "has been set to false." );
  }
  structural_plasticity_enabled_ = true;
  if ( structural_plasticity_gaussian_kernel_sigma_ > 0 )
  {
    gather_global_positions_and_ids();
    if ( structural_plasticity_cache_probabilities_ )
    {
      build_probability_list();
    }
  }
}

void
nest::SPManager::disable_structural_plasticity()
{
  structural_plasticity_enabled_ = false;
}

} // namespace nest