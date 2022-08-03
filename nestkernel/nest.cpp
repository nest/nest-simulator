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

#include "connector_model_impl.h"
#include "iaf_psc_alpha.h"
#include "iaf_psc_delta.h"
#include "iaf_psc_exp.h"
#include "parrot_neuron.h"
#include "static_synapse.h"
#include "weight_recorder.h"

#include "spatial.h"

#include "connection_manager_impl.h"

#include "genericmodel_impl.h"
#include "model_manager.h"
#include "model_manager_impl.h"

#include "config.h"
#include "dictionary.h"

namespace nest
{

void
init_nest( int* argc, char** argv[] )
{
  KernelManager::create_kernel_manager();
  kernel().mpi_manager.init_mpi( argc, argv );
  kernel().initialize();

  kernel().model_manager.register_node_model< iaf_psc_alpha >( "iaf_psc_alpha" );
  kernel().model_manager.register_node_model< iaf_psc_delta >( "iaf_psc_delta" );
  kernel().model_manager.register_node_model< iaf_psc_exp >( "iaf_psc_exp" );
  kernel().model_manager.register_node_model< parrot_neuron >( "parrot_neuron" );

  kernel().model_manager.register_node_model< weight_recorder >( "weight_recorder" );

  kernel().model_manager.register_connection_model< static_synapse >( "static_synapse" );


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

void
enable_dryrun_mode( const index n_procs )
{
  kernel().mpi_manager.set_num_processes( n_procs );
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
  std::stringstream stream;
  nc->print_me( stream );
  return stream.str();
}

size_t
nc_size( NodeCollectionPTR nc )
{
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
get_nc_status( NodeCollectionPTR node_collection )
{
  dictionary result;
  for ( NodeCollection::const_iterator it = node_collection->begin(); it < node_collection->end(); ++it )
  {
    const auto node_status = get_node_status( ( *it ).node_id );
    for ( auto& kv_pair : node_status )
    {
      auto p = result.find( kv_pair.first );
      if ( p != result.end() )
      {
        // key exists
        auto& v = boost::any_cast< std::vector< boost::any >& >( p->second );
        v.push_back( kv_pair.second );
        // *p = v;
      }
      else
      {
        // key does not exist yet
        result[ kv_pair.first ] = std::vector< boost::any > { kv_pair.second };
      }
    }
  }
  return result;
}

void
set_nc_status( NodeCollectionPTR nc, dictionary& params )
{
  params.init_access_flags();
  for ( auto it = nc->begin(); it < nc->end(); ++it )
  {
    kernel().node_manager.set_status( ( *it ).node_id, params );
  }
  params.all_entries_accessed( "NodeCollection.set()", "params" );
}

void
set_connection_status( const std::deque< ConnectionID >& conns, const dictionary& dict )
{
  for ( auto& conn : conns )
  {
    kernel().connection_manager.set_synapse_status( conn.get_source_node_id(),
      conn.get_target_node_id(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port(),
      dict );
  }
}

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

void
set_connection_status( const ConnectionDatum& conn, const dictionary& dict )
{
  // TODO_PYNEST-NG: Get ConnectionDatum dict
  // dictionary conn_dict = conn.get_dict();
  dictionary conn_dict;
  const index source_node_id = conn_dict.get< long >( nest::names::source );
  const index target_node_id = conn_dict.get< long >( nest::names::target );
  const thread tid = conn_dict.get< long >( nest::names::target_thread );
  const synindex syn_id = conn_dict.get< long >( nest::names::synapse_modelid );
  const port p = conn_dict.get< long >( nest::names::port );

  // TODO_PYNEST-NG: Access flags
  // dict->clear_access_flags();

  kernel().connection_manager.set_synapse_status( source_node_id, target_node_id, tid, syn_id, p, dict );

  // ALL_ENTRIES_ACCESSED2( *dict,
  //   "SetStatus",
  //   "Unread dictionary entries: ",
  //   "Maybe you tried to set common synapse properties through an individual "
  //   "synapse?" );
}

dictionary
get_connection_status( const ConnectionDatum& conn )
{
  return kernel().connection_manager.get_synapse_status( conn->get_source_node_id(),
    conn->get_target_node_id(),
    conn->get_target_thread(),
    conn->get_synapse_model_id(),
    conn->get_port() );
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

  if ( not kernel().model_manager.get_modeldict().known( model_name ) )
  {
    std::cerr << "model name: " << model_name << "\n";
    throw UnknownModelName( model_name );
  }

  // create
  const index model_id = static_cast< index >( kernel().model_manager.get_modeldict().get< index >( model_name ) );

  return kernel().node_manager.add_node( model_id, n_nodes );
}

NodeCollectionPTR
create_spatial( const dictionary& layer_dict )
{
  return create_layer( layer_dict );
}

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
set_model_defaults( const std::string& modelname, const dictionary& dict )
{
  kernel().model_manager.set_model_defaults( modelname, dict );
}

dictionary
get_model_defaults( const std::string& modelname )
{
  // const Token nodemodel = kernel().model_manager.get_modeldict()->lookup( modelname );
  // const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( modelname );

  dictionary dict;

  // TODO-PYNEST-NG: fix when updating models get_status()

  // if ( not nodemodel.empty() )
  // {
  //   const long model_id = static_cast< long >( nodemodel );
  //   Model* m = kernel().model_manager.get_model( model_id );
  //   dict = m->get_status();
  // }
  // else if ( not synmodel.empty() )
  // {
  //   const long synapse_id = static_cast< long >( synmodel );
  //   dict = kernel().model_manager.get_connector_defaults( synapse_id );
  // }
  // else
  // {
  //   throw UnknownModelName( modelname.toString() );
  // }

  return dict;
}

std::shared_ptr< Parameter >
create_parameter( const boost::any& value )
{
  if ( is_double( value ) )
  {
    return create_parameter( boost::any_cast< double >( value ) );
  }
  else if ( is_int( value ) )
  {
    return create_parameter( boost::any_cast< int >( value ) );
  }
  else if ( is_dict( value ) )
  {
    return create_parameter( boost::any_cast< dictionary >( value ) );
  }
  else if ( is_parameter( value ) )
  {
    return create_parameter( boost::any_cast< std::shared_ptr< Parameter > >( value ) );
  }
  throw BadProperty( "Parameter must be parametertype, constant or dictionary." );
}

std::shared_ptr< Parameter >
create_parameter( const std::shared_ptr< Parameter > param )
{
  // TODO-PYNEST-NG: do we need this function?
  return param;
}

std::shared_ptr< Parameter >
create_parameter( const double value )
{
  const auto param = new ConstantParameter( value );
  return std::shared_ptr< Parameter >( param );
}

std::shared_ptr< Parameter >
create_parameter( const int value )
{
  const auto param = new ConstantParameter( value );
  return std::shared_ptr< Parameter >( param );
}

std::shared_ptr< Parameter >
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

std::shared_ptr< Parameter >
create_parameter( const std::string& name, const dictionary& d )
{
  // The parameter factory will create the parameter
  return std::shared_ptr< Parameter >( parameter_factory_().create( name, d ) );
}

ParameterFactory&
parameter_factory_( void )
{
  static ParameterFactory factory;
  return factory;
}


double
get_value( const ParameterDatum& param )
{
  RngPtr rng = get_rank_synced_rng();
  return param->value( rng, nullptr );
}

bool
is_spatial( const ParameterDatum& param )
{
  return param->is_spatial();
}

std::vector< double >
apply( const ParameterDatum& param, const NodeCollectionDatum& nc )
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
apply( const ParameterDatum& param, const dictionary& positions )
{
  auto source_nc = positions.get< NodeCollectionPTR >( names::source );
  auto targets = positions.get< std::vector< int > >( names::targets );
  // TODO-PYNEST-NG: fix Parameter::apply()
  // return param->apply( source_nc, targets );
  return {};
}

NodeCollectionPTR
node_collection_array_index( NodeCollectionPTR node_collection, const long* array, unsigned long n )
{
  assert( node_collection->size() >= n );
  std::vector< index > node_ids;
  node_ids.reserve( n );

  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr )
  {
    node_ids.push_back( node_collection->operator[]( *node_ptr ) );
  }
  return NodeCollection::create( node_ids );
}

NodeCollectionPTR
node_collection_array_index( NodeCollectionPTR node_collection, const bool* array, unsigned long n )
{
  assert( node_collection->size() == n );
  std::vector< index > node_ids;
  node_ids.reserve( n );

  auto nc_it = node_collection->begin();
  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr, ++nc_it )
  {
    if ( *node_ptr )
    {
      node_ids.push_back( ( *nc_it ).node_id );
    }
  }
  return NodeCollection::create( node_ids );
}


} // namespace nest
