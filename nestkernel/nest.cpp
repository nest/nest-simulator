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

  // TODO: register_parameter() and register_mask() should be moved, see #3149
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
  register_parameter< GaborParameter >( "gabor" );

  register_mask< BallMask< 2 > >();
  register_mask< BallMask< 3 > >();
  register_mask< EllipseMask< 2 > >();
  register_mask< EllipseMask< 3 > >();
  register_mask< BoxMask< 2 > >();
  register_mask< BoxMask< 3 > >();
  register_mask( "doughnut", create_doughnut );
  register_mask< GridMask< 2 > >();
}

void
shutdown_nest( int exitcode )
{
  // We must MPI_Finalize before the KernelManager() destructor runs, because
  // both MusicManager and MPIManager may be involved, with mpi_finalize()
  // delegating to MusicManager, which is deleted long before MPIManager.
  kernel().mpi_manager.mpi_finalize( exitcode );
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
  assert( nc );
  std::stringstream stream;
  nc->print_me( stream );
  return stream.str();
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
        auto& v = boost::any_cast< std::vector< boost::any >& >( p->second.item );
        v[ node_index ] = kv_pair.second.item;
      }
      else
      {
        // key does not exist yet
        auto new_entry = std::vector< boost::any >( nc->size(), nullptr );
        new_entry[ node_index ] = kv_pair.second.item;
        result[ kv_pair.first ] = new_entry;
      }
    }
  }
  return result;
}

void
set_nc_status( NodeCollectionPTR nc, std::vector< dictionary >& params )
{
  /*
   PYNEST-NG TODO:

   The following does NOT work because the rank_local does not "see" the siblings of devices

  const auto rank_local_begin = nc->rank_local_begin();
  if ( rank_local_begin == nc->end() )
  {
    return; // no local nodes, nothing to do --- more efficient and avoids params access check problems
  }
  */

  if ( params.size() == 1 )
  {
    // PyNEST-NG TODO: Until we have solved the rank_local iteration problem, we need
    // to do the access checking on the individual local node because we otherwise
    // will falsely claim "non read" if a NC has no member on a given rank.

    // params[ 0 ].init_access_flags();

    // We must iterate over all nodes here because we otherwise miss "siblings" of devices
    // May consider ways to fix this.
    for ( auto const& node : *nc )
    {
      kernel().node_manager.set_status( node.node_id, params[ 0 ] );
    }
    // params[ 0 ].all_entries_accessed( "NodeCollection.set()", "params" );
  }
  else if ( nc->size() == params.size() )
  {
    size_t idx = 0;
    for ( auto const& node : *nc )
    {
      // params[ idx ].init_access_flags();
      kernel().node_manager.set_status( node.node_id, params[ idx ] );
      // params[ idx ].all_entries_accessed( "NodeCollection.set()", "params" );
      ++idx;
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

void
set_connection_status( const std::deque< ConnectionID >& conns, const std::vector< dictionary >& dicts )
{
  // PYNEST-NG: Access checks?
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
set_node_status( const size_t node_id, const dictionary& dict )
{
  kernel().node_manager.set_status( node_id, dict );
}

dictionary
get_node_status( const size_t node_id )
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

  if ( step < 1 )
  {
    throw BadParameterValue( "Slicing step must be strictly positive." );
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
create( const std::string& model_name, const size_t n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw BadParameterValue( "n_nodes > 0 expected" );
  }

  const size_t model_id = kernel().model_manager.get_node_model_id( model_name );
  return kernel().node_manager.add_node( model_id, n_nodes );
}

NodeCollectionPTR
create_spatial( const dictionary& layer_dict )
{
  return create_layer( layer_dict );
}

NodeCollectionPTR
make_nodecollection( const std::vector< size_t >& node_ids )
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
  return nc->get_nc_index( node_id );
}

dictionary
get_metadata( const NodeCollectionPTR nc )
{
  dictionary status_dict;
  const auto meta = nc->get_metadata();
  // Fill the status dictionary only if the NodeCollection has valid metadata.
  if ( meta.get() )
  {
    meta->get_status( status_dict, nc );
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
  const std::vector< dictionary >& synapse_params )
{
  kernel().sp_manager.disconnect( sources, targets, connectivity, synapse_params );
}

void
connect_tripartite( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  NodeCollectionPTR third,
  const dictionary& connectivity,
  const dictionary& third_connectivity,
  const std::map< std::string, std::vector< dictionary > >& synapse_specs )
{
  kernel().connection_manager.connect_tripartite(
    sources, targets, third, connectivity, third_connectivity, synapse_specs );
}

void
connect_arrays( long* sources,
  long* targets,
  double* weights,
  double* delays,
  const std::vector< std::string >& p_keys,
  double* p_values,
  size_t n,
  const std::string& syn_model )
{
  kernel().connection_manager.connect_arrays( sources, targets, weights, delays, p_keys, p_values, n, syn_model );
}

void
connect_sonata( const dictionary& graph_specs, const long hyperslab_size )
{
  kernel().connection_manager.connect_sonata( graph_specs, hyperslab_size );
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
  // probably not strictly necessary here, but does nothing if all is up to date
  kernel().node_manager.update_thread_local_node_data();

  for ( auto& conn : conns )
  {
    const auto target_node = kernel().node_manager.get_node_or_proxy( conn.get_target_node_id() );
    kernel().sp_manager.disconnect(
      conn.get_source_node_id(), target_node, conn.get_target_thread(), conn.get_synapse_model_id() );
  }
}

void
simulate( const double t )
{
  prepare();
  run( t );
  cleanup();
}

void
run( const double time )
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
    throw BadParameter( "The simulation time must be a multiple of the simulation resolution." );
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
    const size_t model_id = kernel().model_manager.get_node_model_id( component );
    return kernel().model_manager.get_node_model( model_id )->get_status();
  }
  catch ( UnknownModelName& )
  {
    // ignore errors; throw at the end of the function if that's reached
  }

  try
  {
    const size_t synapse_model_id = kernel().model_manager.get_synapse_model_id( component );
    return kernel().model_manager.get_connector_defaults( synapse_model_id );
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
    return create_parameter( static_cast< long >( boost::any_cast< int >( value ) ) );
  }
  else if ( is_type< long >( value ) )
  {
    return create_parameter( boost::any_cast< long >( value ) );
  }
  else if ( is_type< dictionary >( value ) )
  {
    return create_parameter( boost::any_cast< dictionary >( value ) );
  }
  else if ( is_type< ParameterPTR >( value ) )
  {
    return boost::any_cast< ParameterPTR >( value );
  }
  throw BadProperty(
    std::string( "Parameter must be parametertype, constant or dictionary, got " ) + debug_type( value ) );
}

ParameterPTR
create_parameter( const double value )
{
  const auto param = new ConstantParameter( value );
  return ParameterPTR( param );
}

ParameterPTR
create_parameter( const long value )
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
  std::vector< size_t > node_ids;
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
  std::vector< size_t > node_ids;
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

std::vector< size_t >
node_collection_to_array( NodeCollectionPTR node_collection, const std::string& selection )
{
  return node_collection->to_array( selection );
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
