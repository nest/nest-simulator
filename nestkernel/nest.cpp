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

// C++ includes:
#include <cassert>

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "parameter.h"

#include "sp_manager.h"
#include "sp_manager_impl.h"

#include "connector_model_impl.h"

#include "ac_generator.h"
#include "aeif_cond_alpha.h"
#include "aeif_cond_alpha_multisynapse.h"
#include "aeif_cond_beta_multisynapse.h"
#include "aeif_psc_delta_clopath.h"
#include "cm_default.h"
#include "dc_generator.h"
#include "erfc_neuron.h"
#include "glif_cond.h"
#include "glif_psc.h"
#include "hh_psc_alpha_gap.h"
#include "ht_neuron.h"
#include "iaf_cond_alpha.h"
#include "iaf_cond_alpha_mc.h"
#include "lin_rate.h"
#include "multimeter.h"
#include "noise_generator.h"
#include "poisson_generator.h"
#include "poisson_generator_ps.h"
#include "pp_psc_delta.h"
#include "spike_generator.h"
#include "spike_train_injector.h"
#include "spike_recorder.h"
#include "tanh_rate.h"

#include "aeif_cond_exp.h"
#include "hh_psc_alpha_clopath.h"
#include "iaf_cond_exp.h"
#include "parrot_neuron_ps.h"
#include "siegert_neuron.h"
#include "sigmoid_rate_gg_1998.h"
#include "step_current_generator.h"
#include "step_rate_generator.h"

#include "aeif_psc_alpha.h"
#include "aeif_psc_delta.h"
#include "aeif_psc_exp.h"
#include "threshold_lin_rate.h"

#include "iaf_psc_alpha.h"
#include "iaf_psc_delta.h"
#include "iaf_psc_exp.h"
#include "iaf_psc_exp_multisynapse.h"

#include "pp_cond_exp_mc_urbanczik.h"
#include "spin_detector.h"

#include "parrot_neuron.h"

#include "bernoulli_synapse.h"
#include "clopath_synapse.h"
#include "common_synapse_properties.h"
#include "cont_delay_synapse.h"
#include "cont_delay_synapse_impl.h"
#include "diffusion_connection.h"
#include "gap_junction.h"
#include "ht_synapse.h"
#include "jonke_synapse.h"
#include "quantal_stp_synapse.h"
#include "quantal_stp_synapse_impl.h"
#include "rate_connection_delayed.h"
#include "rate_connection_instantaneous.h"
#include "static_synapse.h"
#include "static_synapse_hom_w.h"
#include "stdp_dopamine_synapse.h"
#include "stdp_nn_pre_centered_synapse.h"
#include "stdp_nn_restr_synapse.h"
#include "stdp_nn_symm_synapse.h"
#include "stdp_pl_synapse_hom.h"
#include "stdp_synapse.h"
#include "stdp_synapse_facetshw_hom.h"
#include "stdp_synapse_facetshw_hom_impl.h"
#include "stdp_synapse_hom.h"
#include "stdp_triplet_synapse.h"
#include "tsodyks2_synapse.h"
#include "tsodyks_synapse.h"
#include "tsodyks_synapse_hom.h"
#include "urbanczik_synapse.h"
#include "vogels_sprekeler_synapse.h"

#include "volume_transmitter.h"
#include "weight_recorder.h"

#include "conn_builder_conngen.h"

#include "grid_mask.h"
#include "spatial.h"

#include "connection_manager_impl.h"

#include "genericmodel_impl.h"
#include "model_manager.h"
#include "model_manager_impl.h"

#include "config.h"
#include "dictionary.h"

namespace nest
{


AbstractMask* create_doughnut( const dictionary& d );

void
init_nest( int* argc, char** argv[] )
{
  KernelManager::create_kernel_manager();
  kernel().mpi_manager.init_mpi( argc, argv );
  kernel().initialize();


  kernel().model_manager.register_node_model< ac_generator >( "ac_generator" );
  kernel().model_manager.register_node_model< dc_generator >( "dc_generator" );
  kernel().model_manager.register_node_model< spike_generator >( "spike_generator" );
  kernel().model_manager.register_node_model< spike_train_injector >( "spike_train_injector" );
  kernel().model_manager.register_node_model< spike_recorder >( "spike_recorder" );
  kernel().model_manager.register_node_model< poisson_generator >( "poisson_generator" );
  kernel().model_manager.register_node_model< poisson_generator_ps >( "poisson_generator_ps" );
  kernel().model_manager.register_node_model< voltmeter >( "voltmeter" );
  kernel().model_manager.register_node_model< multimeter >( "multimeter" );
  kernel().model_manager.register_node_model< noise_generator >( "noise_generator" );
  kernel().model_manager.register_node_model< aeif_cond_alpha >( "aeif_cond_alpha" );
  kernel().model_manager.register_node_model< aeif_cond_alpha_multisynapse >( "aeif_cond_alpha_multisynapse" );
  kernel().model_manager.register_node_model< aeif_cond_beta_multisynapse >( "aeif_cond_beta_multisynapse" );
  kernel().model_manager.register_node_model< aeif_psc_delta_clopath >( "aeif_psc_delta_clopath" );
  kernel().model_manager.register_node_model< cm_default >( "cm_default" );
  kernel().model_manager.register_node_model< erfc_neuron >( "erfc_neuron" );
  kernel().model_manager.register_node_model< glif_cond >( "glif_cond" );
  kernel().model_manager.register_node_model< glif_psc >( "glif_psc" );
  kernel().model_manager.register_node_model< hh_psc_alpha_gap >( "hh_psc_alpha_gap" );
  kernel().model_manager.register_node_model< ht_neuron >( "ht_neuron" );
  kernel().model_manager.register_node_model< iaf_cond_alpha_mc >( "iaf_cond_alpha_mc" );
  kernel().model_manager.register_node_model< pp_psc_delta >( "pp_psc_delta" );
  kernel().model_manager.register_node_model< lin_rate_ipn >( "lin_rate_ipn" );
  kernel().model_manager.register_node_model< iaf_cond_alpha >( "iaf_cond_alpha" );
  kernel().model_manager.register_node_model< rate_transformer_sigmoid_gg_1998 >( "rate_transformer_sigmoid_gg_1998" );

  kernel().model_manager.register_node_model< tanh_rate_ipn >( "tanh_rate_ipn" );
  kernel().model_manager.register_node_model< lin_rate_opn >( "lin_rate_opn" );
  kernel().model_manager.register_node_model< parrot_neuron_ps >( "parrot_neuron_ps" );
  kernel().model_manager.register_node_model< step_rate_generator >( "step_rate_generator" );
  kernel().model_manager.register_node_model< step_current_generator >( "step_current_generator" );
  kernel().model_manager.register_node_model< hh_psc_alpha_clopath >( "hh_psc_alpha_clopath" );
  kernel().model_manager.register_node_model< iaf_cond_exp >( "iaf_cond_exp" );
  kernel().model_manager.register_node_model< aeif_cond_exp >( "aeif_cond_exp" );
  kernel().model_manager.register_node_model< siegert_neuron >( "siegert_neuron" );

  kernel().model_manager.register_node_model< aeif_psc_alpha >( "aeif_psc_alpha" );
  kernel().model_manager.register_node_model< aeif_psc_delta >( "aeif_psc_delta" );
  kernel().model_manager.register_node_model< aeif_psc_exp >( "aeif_psc_exp" );
  kernel().model_manager.register_node_model< threshold_lin_rate_ipn >( "threshold_lin_rate_ipn" );

  kernel().model_manager.register_node_model< iaf_psc_alpha >( "iaf_psc_alpha" );
  kernel().model_manager.register_node_model< iaf_psc_delta >( "iaf_psc_delta" );
  kernel().model_manager.register_node_model< iaf_psc_exp >( "iaf_psc_exp" );
  kernel().model_manager.register_node_model< iaf_psc_exp_multisynapse >( "iaf_psc_exp_multisynapse" );
  kernel().model_manager.register_node_model< parrot_neuron >( "parrot_neuron" );

  kernel().model_manager.register_node_model< spin_detector >( "spin_detector" );
  kernel().model_manager.register_node_model< pp_cond_exp_mc_urbanczik >( "pp_cond_exp_mc_urbanczik" );

  kernel().model_manager.register_node_model< weight_recorder >( "weight_recorder" );
  kernel().model_manager.register_node_model< volume_transmitter >( "volume_transmitter" );

  kernel().model_manager.register_connection_model< bernoulli_synapse >( "bernoulli_synapse" );
  kernel().model_manager.register_connection_model< clopath_synapse >( "clopath_synapse" );
  kernel().model_manager.register_connection_model< cont_delay_synapse >( "cont_delay_synapse" );
  kernel().model_manager.register_connection_model< ht_synapse >( "ht_synapse" );
  kernel().model_manager.register_connection_model< jonke_synapse >( "jonke_synapse" );
  kernel().model_manager.register_connection_model< quantal_stp_synapse >( "quantal_stp_synapse" );
  kernel().model_manager.register_connection_model< static_synapse >( "static_synapse" );
  kernel().model_manager.register_connection_model< static_synapse_hom_w >( "static_synapse_hom_w" );
  kernel().model_manager.register_connection_model< stdp_synapse >( "stdp_synapse" );
  kernel().model_manager.register_connection_model< stdp_synapse_hom >( "stdp_synapse_hom" );
  kernel().model_manager.register_connection_model< stdp_dopamine_synapse >( "stdp_dopamine_synapse" );
  kernel().model_manager.register_connection_model< stdp_facetshw_synapse_hom >( "stdp_facetshw_synapse_hom" );
  kernel().model_manager.register_connection_model< stdp_nn_restr_synapse >( "stdp_nn_restr_synapse" );
  kernel().model_manager.register_connection_model< stdp_nn_symm_synapse >( "stdp_nn_symm_synapse" );
  kernel().model_manager.register_connection_model< stdp_nn_pre_centered_synapse >( "stdp_nn_pre_centered_synapse" );
  kernel().model_manager.register_connection_model< stdp_pl_synapse_hom >( "stdp_pl_synapse_hom" );
  kernel().model_manager.register_connection_model< stdp_triplet_synapse >( "stdp_triplet_synapse" );
  kernel().model_manager.register_connection_model< tsodyks_synapse >( "tsodyks_synapse" );
  kernel().model_manager.register_connection_model< tsodyks_synapse_hom >( "tsodyks_synapse_hom" );
  kernel().model_manager.register_connection_model< tsodyks2_synapse >( "tsodyks2_synapse" );
  kernel().model_manager.register_connection_model< urbanczik_synapse >( "urbanczik_synapse" );
  kernel().model_manager.register_connection_model< vogels_sprekeler_synapse >( "vogels_sprekeler_synapse" );

  // register secondary connection models
  kernel().model_manager.register_connection_model< GapJunction >( "gap_junction" );
  kernel().model_manager.register_connection_model< RateConnectionInstantaneous >( "rate_connection_instantaneous" );
  kernel().model_manager.register_connection_model< RateConnectionDelayed >( "rate_connection_delayed" );
  kernel().model_manager.register_connection_model< DiffusionConnection >( "diffusion_connection" );

  // Add connection rules
  kernel().connection_manager.register_conn_builder< OneToOneBuilder >( "one_to_one" );
  kernel().connection_manager.register_conn_builder< AllToAllBuilder >( "all_to_all" );
  kernel().connection_manager.register_conn_builder< FixedInDegreeBuilder >( "fixed_indegree" );
  kernel().connection_manager.register_conn_builder< FixedOutDegreeBuilder >( "fixed_outdegree" );
  kernel().connection_manager.register_conn_builder< BernoulliBuilder >( "pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< SymmetricBernoulliBuilder >( "symmetric_pairwise_bernoulli" );
  kernel().connection_manager.register_conn_builder< FixedTotalNumberBuilder >( "fixed_total_number" );
#ifdef HAVE_LIBNEUROSIM
  kernel().connection_manager.register_conn_builder< ConnectionGeneratorBuilder >( "conngen" );
#endif

  register_parameter< ConstantParameter >( "constant" );
  register_parameter< UniformParameter >( "uniform" );
  register_parameter< UniformIntParameter >( "uniform_int" );
  register_parameter< NormalParameter >( "normal" );
  register_parameter< LognormalParameter >( "lognormal" );
  register_parameter< ExponentialParameter >( "exponential" );
  register_parameter< NodePosParameter >( "position" );
  register_parameter< SpatialDistanceParameter >( "distance" );
  register_parameter< GaussianParameter >( "gaussian" );
  register_parameter< Gaussian2DParameter >( "gaussian2d" );
  register_parameter< GammaParameter >( "gamma" );
  register_parameter< ExpDistParameter >( "exp_distribution" );

  register_mask< BallMask< 2 > >();
  register_mask< BallMask< 3 > >();
  register_mask< EllipseMask< 2 > >();
  register_mask< EllipseMask< 3 > >();
  register_mask< BoxMask< 2 > >();
  register_mask< BoxMask< 3 > >();
  register_mask( "doughnut", create_doughnut );
  register_mask< GridMask< 2 > >();

  kernel().sp_manager.register_growth_curve< GrowthCurveSigmoid >( "sigmoid" );
  kernel().sp_manager.register_growth_curve< GrowthCurveGaussian >( "gaussian" );
  kernel().sp_manager.register_growth_curve< GrowthCurveLinear >( "linear" );
}

void
fail_exit( int )
{
}

void
install_module( const std::string& )
{
}

void
reset_kernel()
{
  kernel().reset();
}

severity_t
get_verbosity()
{
  return kernel().logging_manager.get_logging_level();
}

void
set_verbosity( severity_t s )
{
  kernel().logging_manager.set_logging_level( s );
}

void
enable_dryrun_mode( const index n_procs )
{
  kernel().mpi_manager.set_num_processes( n_procs );
}

void
enable_structural_plasticity()
{
  kernel().sp_manager.enable_structural_plasticity();
}


void
disable_structural_plasticity()
{
  kernel().sp_manager.disable_structural_plasticity();
}

void
register_logger_client( const deliver_logging_event_ptr client_callback )
{
  kernel().logging_manager.register_logging_client( client_callback );
}

int
get_rank()
{
  return kernel().mpi_manager.get_rank();
}

int
get_num_mpi_processes()
{
  return kernel().mpi_manager.get_num_processes();
}

std::string
print_nodes_to_string()
{
  std::stringstream string_stream;
  kernel().node_manager.print( string_stream );
  return string_stream.str();
}

std::string
pprint_to_string( NodeCollectionPTR nc )
{
  if ( nc )
  {
    std::stringstream stream;
    nc->print_me( stream );
    return stream.str();
  }
  else
  {
    // PYNEST-ng: added this, not sure why this can happen now, but could not previously
    std::cout << "pprint_to_string: nc is not assigned" << std::endl;
    return "";
  }
}

size_t
nc_size( NodeCollectionPTR nc )
{
  assert( nc && "NodeCollectionPTR must be initialized." );
  return nc->size();
}

void
set_kernel_status( const dictionary& dict )
{
  dict.init_access_flags();
  kernel().set_status( dict );
  dict.all_entries_accessed( "SetKernelStatus", "params" );
}

dictionary
get_kernel_status()
{
  assert( kernel().is_initialized() );

  dictionary d;
  kernel().get_status( d );

  return d;
}

dictionary
get_nc_status( NodeCollectionPTR nc )
{
  dictionary result;
  size_t node_index = 0;
  for ( NodeCollection::const_iterator it = nc->begin(); it < nc->end(); ++it, ++node_index )
  {
    const auto node_status = get_node_status( ( *it ).node_id );
    for ( auto& kv_pair : node_status )
    {
      auto p = result.find( kv_pair.first );
      if ( p != result.end() )
      {
        // key exists
        auto& v = boost::any_cast< std::vector< boost::any >& >( p->second );
        v[ node_index ] = kv_pair.second;
      }
      else
      {
        // key does not exist yet
        auto new_entry = std::vector< boost::any >( nc->size(), nullptr );
        new_entry[ node_index ] = kv_pair.second;
        result[ kv_pair.first ] = new_entry;
      }
    }
  }
  return result;
}

void
set_nc_status( NodeCollectionPTR nc, std::vector< dictionary >& params )
{
  if ( params.size() == 1 )
  {
    params[ 0 ].init_access_flags();
    for ( auto it = nc->begin(); it < nc->end(); ++it )
    {
      kernel().node_manager.set_status( ( *it ).node_id, params[ 0 ] );
    }
    params[ 0 ].all_entries_accessed( "NodeCollection.set()", "params" );
  }
  else if ( nc->size() == params.size() )
  {
    for ( auto it = nc->begin(); it < nc->end(); ++it )
    {
      size_t i = ( *it ).lid;
      params[ i ].init_access_flags();
      kernel().node_manager.set_status( ( *it ).node_id, params[ i ] );
      params[ i ].all_entries_accessed( "NodeCollection.set()", "params" );
    }
  }
  else
  {
    std::string msg = String::compose(
      "List of dictionaries must be the same size as the NodeCollection (%1), %2 given.", nc->size(), params.size() );
    throw BadParameter( msg );
  }
}

void
set_connection_status( const std::deque< ConnectionID >& conns, const dictionary& dict )
{
  dict.init_access_flags();
  for ( auto& conn : conns )
  {
    kernel().connection_manager.set_synapse_status( conn.get_source_node_id(),
      conn.get_target_node_id(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port(),
      dict );
  }
  dict.all_entries_accessed( "connection.set()", "params" );
}

//// void
//// set_connection_status( const std::deque< ConnectionID >& conn, const dictionary& dict )
//// {
////   // TODO_PYNEST-NG: Get ConnectionDatum dict
////   // dictionary conn_dict = conn.get_dict();
////   dictionary conn_dict;
////   const index source_node_id = conn_dict.get< long >( nest::names::source );
////   const index target_node_id = conn_dict.get< long >( nest::names::target );
////   const thread tid = conn_dict.get< long >( nest::names::target_thread );
////   const synindex syn_id = conn_dict.get< long >( nest::names::synapse_modelid );
////   const port p = conn_dict.get< long >( nest::names::port );
////
////   // TODO_PYNEST-NG: Access flags
////   // dict->clear_access_flags();
////
////   kernel().connection_manager.set_synapse_status( source_node_id, target_node_id, tid, syn_id, p, dict );
////
////   // ALL_ENTRIES_ACCESSED2( *dict,
////   //   "SetStatus",
////   //   "Unread dictionary entries: ",
////   //   "Maybe you tried to set common synapse properties through an individual "
////   //   "synapse?" );
//// }


void
set_connection_status( const std::deque< ConnectionID >& conns, const std::vector< dictionary >& dicts )
{
  if ( conns.size() != dicts.size() )
  {
    throw BadParameter( "List of dictionaries must contain one dictionary per connection" );
  }

  for ( size_t i = 0; i < conns.size(); ++i )
  {
    const auto conn = conns[ i ];
    const auto dict = dicts[ i ];
    kernel().connection_manager.set_synapse_status( conn.get_source_node_id(),
      conn.get_target_node_id(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port(),
      dict );
  }
}

std::vector< dictionary >
get_connection_status( const std::deque< ConnectionID >& conns )
{
  std::vector< dictionary > result;
  result.reserve( conns.size() );

  for ( auto& conn : conns )
  {
    const auto d = kernel().connection_manager.get_synapse_status( conn.get_source_node_id(),
      conn.get_target_node_id(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port() );
    result.push_back( d );
  }
  return result;
}

void
set_node_status( const index node_id, const dictionary& dict )
{
  kernel().node_manager.set_status( node_id, dict );
}

dictionary
get_node_status( const index node_id )
{
  return kernel().node_manager.get_status( node_id );
}

dictionary
get_connection_status( const ConnectionID& conn )
{
  return kernel().connection_manager.get_synapse_status( conn.get_source_node_id(),
    conn.get_target_node_id(),
    conn.get_target_thread(),
    conn.get_synapse_model_id(),
    conn.get_port() );
}

NodeCollectionPTR
slice_nc( const NodeCollectionPTR nc, long start, long stop, long step )
{
  const size_t g_size = nc->size();

  // TODO-PYNEST-NG: Zero-based indexing?
  if ( step < 1 )
  {
    throw BadParameter( "Slicing step must be strictly positive." );
  }

  if ( start >= 0 )
  {
    start -= 1; // adjust from 1-based to 0-based indexing
  }
  else
  {
    start += g_size; // automatically correct for 0-based indexing
  }

  if ( stop >= 0 )
  {
    // no adjustment necessary: adjustment from 1- to 0- based indexing
    // and adjustment from last- to stop-based logic cancel
  }
  else
  {
    stop += g_size + 1; // adjust from 0- to 1- based indexin
  }

  return nc->slice( start, stop, step );
}

NodeCollectionPTR
create( const std::string model_name, const index n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const index model_id = kernel().model_manager.get_node_model_id( model_name );
  return kernel().node_manager.add_node( model_id, n_nodes );
}

NodeCollectionPTR
create_spatial( const dictionary& layer_dict )
{
  return create_layer( layer_dict );
}

// std::vector< std::vector< double > >
// get_position( NodeCollectionPTR layer_nc )
//{
//   return get_position( layer );  // PYNEST-NG: is this call creating a copy?
// }


NodeCollectionPTR
make_nodecollection( const std::vector< index > node_ids )
{
  return NodeCollection::create( node_ids );
}

NodeCollectionPTR
get_nodes( const dictionary& params, const bool local_only )
{
  return kernel().node_manager.get_nodes( params, local_only );
}

bool
equal( const NodeCollectionPTR lhs, const NodeCollectionPTR rhs )
{
  return lhs->operator==( rhs );
}

bool
contains( const NodeCollectionPTR nc, const size_t node_id )
{
  return nc->contains( node_id );
}

long
find( const NodeCollectionPTR nc, size_t node_id )
{
  return nc->find( node_id );
}

dictionary
get_metadata( const NodeCollectionPTR nc )
{
  dictionary status_dict;
  const auto meta = nc->get_metadata();
  // Fill the status dictionary only if the NodeCollection has valid metadata.
  if ( meta.get() )
  {
    meta->get_status( status_dict );
    slice_positions_if_sliced_nc( status_dict, nc );
    status_dict[ names::network_size ] = nc->size();
  }
  return status_dict;
}

void
connect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const dictionary& connectivity,
  const std::vector< dictionary >& synapse_params )
{
  kernel().connection_manager.connect( sources, targets, connectivity, synapse_params );
}

void
disconnect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const dictionary& connectivity,
  const dictionary& synapse_params )
{
  kernel().sp_manager.disconnect( sources, targets, connectivity, synapse_params );
}


void
connect_arrays( long* sources,
  long* targets,
  double* weights,
  double* delays,
  std::vector< std::string >& p_keys,
  double* p_values,
  size_t n,
  std::string syn_model )
{
  kernel().connection_manager.connect_arrays( sources, targets, weights, delays, p_keys, p_values, n, syn_model );
}

std::deque< ConnectionID >
get_connections( const dictionary& dict )
{
  dict.init_access_flags();

  const auto& connectome = kernel().connection_manager.get_connections( dict );

  dict.all_entries_accessed( "GetConnections", "params" );

  return connectome;
}

void
disconnect( const std::deque< ConnectionID >& conns )
{
  for ( auto& conn : conns )
  {
    const auto target_node = kernel().node_manager.get_node_or_proxy( conn.get_target_node_id() );
    kernel().sp_manager.disconnect(
      conn.get_source_node_id(), target_node, conn.get_target_thread(), conn.get_synapse_model_id() );
  }
}

void
simulate( const double& t )
{
  prepare();
  run( t );
  cleanup();
}

void
run( const double& time )
{
  const Time t_sim = Time::ms( time );

  if ( time < 0 )
  {
    throw BadParameter( "The simulation time cannot be negative." );
  }
  if ( not t_sim.is_finite() )
  {
    throw BadParameter( "The simulation time must be finite." );
  }
  if ( not t_sim.is_grid_time() )
  {
    throw BadParameter(
      "The simulation time must be a multiple "
      "of the simulation resolution." );
  }

  kernel().simulation_manager.run( t_sim );
}

void
prepare()
{
  kernel().prepare();
}

void
cleanup()
{
  kernel().cleanup();
}

void
copy_model( const std::string& oldmodname, const std::string& newmodname, const dictionary& dict )
{
  kernel().model_manager.copy_model( oldmodname, newmodname, dict );
}

void
set_model_defaults( const std::string& component, const dictionary& dict )
{
  if ( kernel().model_manager.set_model_defaults( component, dict ) )
  {
    return;
  }

  if ( kernel().io_manager.is_valid_recording_backend( component ) )
  {
    kernel().io_manager.set_recording_backend_status( component, dict );
    return;
  }

  throw UnknownComponent( component );
}

dictionary
get_model_defaults( const std::string& component )
{
  try
  {
    const index model_id = kernel().model_manager.get_node_model_id( component );
    return kernel().model_manager.get_node_model( model_id )->get_status();
  }
  catch ( UnknownModelName& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  try
  {
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( component );
    const auto ret = kernel().model_manager.get_connector_defaults( synapse_model_id );
    return ret;
  }
  catch ( UnknownSynapseType& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  if ( kernel().io_manager.is_valid_recording_backend( component ) )
  {
    return kernel().io_manager.get_recording_backend_status( component );
  }

  throw UnknownComponent( component );
  return dictionary(); // supress missing return value warning; never reached
}

ParameterPTR
create_parameter( const boost::any& value )
{
  if ( is_type< double >( value ) )
  {
    return create_parameter( boost::any_cast< double >( value ) );
  }
  else if ( is_type< int >( value ) )
  {
    return create_parameter( boost::any_cast< int >( value ) );
  }
  else if ( is_type< long >( value ) )
  {
    return create_parameter( static_cast< int >( boost::any_cast< long >( value ) ) );
  }
  else if ( is_type< dictionary >( value ) )
  {
    return create_parameter( boost::any_cast< dictionary >( value ) );
  }
  else if ( is_type< ParameterPTR >( value ) )
  {
    return create_parameter( boost::any_cast< ParameterPTR >( value ) );
  }
  throw BadProperty(
    std::string( "Parameter must be parametertype, constant or dictionary, got " ) + debug_type( value ) );
}

ParameterPTR
create_parameter( const ParameterPTR param )
{
  // TODO-PYNEST-NG: do we need this function?
  return param;
}

ParameterPTR
create_parameter( const double value )
{
  const auto param = new ConstantParameter( value );
  return ParameterPTR( param );
}

ParameterPTR
create_parameter( const int value )
{
  const auto param = new ConstantParameter( value );
  return ParameterPTR( param );
}

ParameterPTR
create_parameter( const dictionary& param_dict )
{
  // The dictionary should only have a single key, which is the name of
  // the parameter type to create.
  if ( param_dict.size() != 1 )
  {
    throw BadProperty( "Parameter definition dictionary must contain one single key only." );
  }

  const auto n = param_dict.begin()->first;
  const auto pdict = param_dict.get< dictionary >( n );
  pdict.init_access_flags();
  auto parameter = create_parameter( n, pdict );
  pdict.all_entries_accessed( "create_parameter", "param" );
  return parameter;
}

ParameterPTR
create_parameter( const std::string& name, const dictionary& d )
{
  // The parameter factory will create the parameter
  return ParameterPTR( parameter_factory_().create( name, d ) );
}

ParameterFactory&
parameter_factory_( void )
{
  static ParameterFactory factory;
  return factory;
}

MaskFactory&
mask_factory_( void )
{
  static MaskFactory factory;
  return factory;
}

double
get_value( const ParameterPTR param )
{
  RngPtr rng = get_rank_synced_rng();
  return param->value( rng, nullptr );
}

bool
is_spatial( const ParameterPTR param )
{
  return param->is_spatial();
}

std::vector< double >
apply( const ParameterPTR param, const NodeCollectionPTR nc )
{
  std::vector< double > result;
  result.reserve( nc->size() );
  RngPtr rng = get_rank_synced_rng();
  for ( auto it = nc->begin(); it < nc->end(); ++it )
  {
    auto node = kernel().node_manager.get_node_or_proxy( ( *it ).node_id );
    result.push_back( param->value( rng, node ) );
  }
  return result;
}

std::vector< double >
apply( const ParameterPTR param, const dictionary& positions )
{
  auto source_nc = positions.get< NodeCollectionPTR >( names::source );
  auto targets = positions.get< std::vector< std::vector< double > > >( names::targets );
  return param->apply( source_nc, targets );
}

NodeCollectionPTR
node_collection_array_index( NodeCollectionPTR nc, const long* array, unsigned long n )
{
  assert( nc->size() >= n );
  std::vector< index > node_ids;
  node_ids.reserve( n );

  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr )
  {
    node_ids.push_back( nc->operator[]( *node_ptr ) );
  }
  return NodeCollection::create( node_ids );
}

NodeCollectionPTR
node_collection_array_index( NodeCollectionPTR nc, const bool* array, unsigned long n )
{
  assert( nc->size() == n );
  std::vector< index > node_ids;
  node_ids.reserve( n );

  auto nc_it = nc->begin();
  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr, ++nc_it )
  {
    if ( *node_ptr )
    {
      node_ids.push_back( ( *nc_it ).node_id );
    }
  }
  return NodeCollection::create( node_ids );
}

void
slice_positions_if_sliced_nc( dictionary& dict, const NodeCollectionPTR nc )
{
  // If metadata contains node positions and the NodeCollection is sliced, get only positions of the sliced nodes.
  if ( dict.known( names::positions ) )
  {
    // PyNEST-NG: Check if TokenArray is the correct type here
    const auto positions = dict.get< std::vector< std::vector< double > > >( names::positions );
    if ( nc->size() != positions.size() )
    {
      std::vector< std::vector< double > > sliced_points;
      // Iterate only local nodes
      NodeCollection::const_iterator nc_begin = nc->has_proxies() ? nc->MPI_local_begin() : nc->begin();
      NodeCollection::const_iterator nc_end = nc->end();
      for ( auto node = nc_begin; node < nc_end; ++node )
      {
        // Because the local ID also includes non-local nodes, it must be adapted to represent
        // the index for the local node position.
        const auto index =
          static_cast< size_t >( std::floor( ( *node ).lid / kernel().mpi_manager.get_num_processes() ) );
        sliced_points.push_back( positions[ index ] );
      }
      dict[ names::positions ] = sliced_points;
    }
  }
}

AbstractMask*
create_doughnut( const dictionary& d )
{
  // The doughnut (actually an annulus) is created using a DifferenceMask
  Position< 2 > center( 0, 0 );
  if ( d.known( names::anchor ) )
  {
    center = d.get< std::vector< double > >( names::anchor );
  }

  const double outer = d.get< double >( names::outer_radius );
  const double inner = d.get< double >( names::inner_radius );
  if ( inner >= outer )
  {
    throw BadProperty(
      "nest::create_doughnut: "
      "inner_radius < outer_radius required." );
  }

  BallMask< 2 > outer_circle( center, outer );
  BallMask< 2 > inner_circle( center, inner );

  return new DifferenceMask< 2 >( outer_circle, inner_circle );
}


} // namespace nest
