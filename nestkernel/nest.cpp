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

// Includes from sli:
#include "sliexceptions.h"
#include "token.h"

namespace nest
{

void
init_nest( int* argc, char** argv[] )
{
  KernelManager::create_kernel_manager();
  kernel().mpi_manager.init_mpi( argc, argv );
  kernel().initialize();
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

void
print_nodes_to_stream( std::ostream& ostr )
{
  kernel().node_manager.print( ostr );
}

librandom::RngPtr
get_vp_rng( thread tid )
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( kernel().vp_manager.get_num_threads() ) );
  return kernel().rng_manager.get_rng( tid );
}

librandom::RngPtr
get_global_rng()
{
  return kernel().rng_manager.get_grng();
}

void
set_kernel_status( const DictionaryDatum& dict )
{
  dict->clear_access_flags();
  kernel().set_status( dict );
  ALL_ENTRIES_ACCESSED( *dict, "SetKernelStatus", "Unread dictionary entries: " );
}

DictionaryDatum
get_kernel_status()
{
  assert( kernel().is_initialized() );

  DictionaryDatum d( new Dictionary );
  kernel().get_status( d );

  return d;
}

void
set_node_status( const index node_id, const DictionaryDatum& dict )
{
  kernel().node_manager.set_status( node_id, dict );
}

DictionaryDatum
get_node_status( const index node_id )
{
  return kernel().node_manager.get_status( node_id );
}

void
set_connection_status( const ConnectionDatum& conn, const DictionaryDatum& dict )
{
  DictionaryDatum conn_dict = conn.get_dict();
  const index source_node_id = getValue< long >( conn_dict, nest::names::source );
  const index target_node_id = getValue< long >( conn_dict, nest::names::target );
  const thread tid = getValue< long >( conn_dict, nest::names::target_thread );
  const synindex syn_id = getValue< long >( conn_dict, nest::names::synapse_modelid );
  const port p = getValue< long >( conn_dict, nest::names::port );

  dict->clear_access_flags();

  kernel().connection_manager.set_synapse_status( source_node_id, target_node_id, tid, syn_id, p, dict );

  ALL_ENTRIES_ACCESSED2( *dict,
    "SetStatus",
    "Unread dictionary entries: ",
    "Maybe you tried to set common synapse properties through an individual "
    "synapse?" );
}

DictionaryDatum
get_connection_status( const ConnectionDatum& conn )
{
  return kernel().connection_manager.get_synapse_status( conn.get_source_node_id(),
    conn.get_target_node_id(),
    conn.get_target_thread(),
    conn.get_synapse_model_id(),
    conn.get_port() );
}

NodeCollectionPTR
create( const Name& model_name, const index n_nodes )
{
  if ( n_nodes == 0 )
  {
    throw RangeCheck();
  }

  const Token model = kernel().model_manager.get_modeldict()->lookup( model_name );
  if ( model.empty() )
  {
    throw UnknownModelName( model_name );
  }

  // create
  const index model_id = static_cast< index >( model );

  return kernel().node_manager.add_node( model_id, n_nodes );
}

NodeCollectionPTR
get_nodes( const DictionaryDatum& params, const bool local_only )
{
  return kernel().node_manager.get_nodes( params, local_only );
}

void
connect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const DictionaryDatum& connectivity,
  const std::vector< DictionaryDatum >& synapse_params )
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
  // Mapping pointers to the first parameter value of each parameter to their respective names.
  std::map< Name, double* > param_pointers;
  if ( p_keys.size() != 0 )
  {
    size_t i = 0;
    for ( auto& key : p_keys )
    {
      // Shifting the pointer to the first value of the parameter.
      param_pointers[ key ] = p_values + i * n;
      ++i;
    }
  }

  // Dictionary holding additional synapse parameters, passed to the connect call.
  std::vector< DictionaryDatum > param_dicts( kernel().vp_manager.get_num_threads(), new Dictionary() );
  index synapse_model_id( kernel().model_manager.get_synapsedict()->lookup( syn_model ) );

  // Increments pointers to weight, delay, and receptor type, if they are specified.
  auto increment_wd = [weights, delays]( decltype( weights ) & w, decltype( delays ) & d )
  {
    if ( weights != nullptr )
    {
      ++w;
    }
    if ( delays != nullptr )
    {
      ++d;
    }
  };
  // Vector for storing exceptions raised by threads.
  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();
    try
    {
      auto s = sources;
      auto t = targets;
      auto w = weights;
      auto d = delays;
      double weight_buffer = numerics::nan;
      double delay_buffer = numerics::nan;
      for ( ; s != sources + n; ++s, ++t )
      {
        if ( 0 >= *s or static_cast< index >( *s ) > kernel().node_manager.size() )
        {
          throw UnknownNode( *s );
        }
        if ( 0 >= *t or static_cast< index >( *t ) > kernel().node_manager.size() )
        {
          throw UnknownNode( *t );
        }
        auto target_node = kernel().node_manager.get_node_or_proxy( *t, tid );
        if ( target_node->is_proxy() )
        {
          increment_wd( w, d );
          continue;
        }
        // If weights or delays are specified, the buffers are replaced with the values.
        // If not, the buffers will be NaN and replaced by a default value by the connect function.
        if ( weights != nullptr )
        {
          weight_buffer = *w;
        }
        if ( delays != nullptr )
        {
          delay_buffer = *d;
        }

        // Store the key-value pair of each parameter in the Dictionary.
        for ( auto& param_pointer_pair : param_pointers )
        {
          // Receptor type must be an integer.
          if ( param_pointer_pair.first == names::receptor_type )
          {
            const auto int_cast_rtype = static_cast< size_t >( *param_pointer_pair.second );
            if ( int_cast_rtype != *param_pointer_pair.second )
            {
              throw BadParameter( "Receptor types must be integers." );
            }
            ( *param_dicts[ tid ] )[ param_pointer_pair.first ] = int_cast_rtype;
          }
          else
          {
            ( *param_dicts[ tid ] )[ param_pointer_pair.first ] = *param_pointer_pair.second;
          }
          // Increment the pointer to the parameter value.
          ++param_pointer_pair.second;
        }

        kernel().connection_manager.connect(
          *s, target_node, tid, synapse_model_id, param_dicts[ tid ], delay_buffer, weight_buffer );
        ALL_ENTRIES_ACCESSED( *param_dicts[ tid ], "connect_arrays", "Unread dictionary entries: " );

        increment_wd( w, d );
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at the end of the catch block.
      exceptions_raised.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
  // check if any exceptions have been raised
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised.at( tid ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised.at( tid ) ) );
    }
  }
}

ArrayDatum
get_connections( const DictionaryDatum& dict )
{
  dict->clear_access_flags();

  ArrayDatum array = kernel().connection_manager.get_connections( dict );

  ALL_ENTRIES_ACCESSED( *dict, "GetConnections", "Unread dictionary entries: " );

  return array;
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
copy_model( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict )
{
  kernel().model_manager.copy_model( oldmodname, newmodname, dict );
}

void
set_model_defaults( const Name& modelname, const DictionaryDatum& dict )
{
  kernel().model_manager.set_model_defaults( modelname, dict );
}

DictionaryDatum
get_model_defaults( const Name& modelname )
{
  const Token nodemodel = kernel().model_manager.get_modeldict()->lookup( modelname );
  const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( modelname );

  DictionaryDatum dict;

  if ( not nodemodel.empty() )
  {
    const long model_id = static_cast< long >( nodemodel );
    Model* m = kernel().model_manager.get_model( model_id );
    dict = m->get_status();
  }
  else if ( not synmodel.empty() )
  {
    const long synapse_id = static_cast< long >( synmodel );
    dict = kernel().model_manager.get_connector_defaults( synapse_id );
  }
  else
  {
    throw UnknownModelName( modelname.toString() );
  }

  return dict;
}

ParameterDatum
multiply_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->multiply_parameter( *param2 );
}

ParameterDatum
divide_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->divide_parameter( *param2 );
}

ParameterDatum
add_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->add_parameter( *param2 );
}

ParameterDatum
subtract_parameter( const ParameterDatum& param1, const ParameterDatum& param2 )
{
  return param1->subtract_parameter( *param2 );
}

ParameterDatum
compare_parameter( const ParameterDatum& param1, const ParameterDatum& param2, const DictionaryDatum& d )
{
  return param1->compare_parameter( *param2, d );
}

ParameterDatum
conditional_parameter( const ParameterDatum& param1, const ParameterDatum& param2, const ParameterDatum& param3 )
{
  return param1->conditional_parameter( *param2, *param3 );
}

ParameterDatum
min_parameter( const ParameterDatum& param, const double other_value )
{
  return param->min( other_value );
}

ParameterDatum
max_parameter( const ParameterDatum& param, const double other_value )
{
  return param->max( other_value );
}

ParameterDatum
redraw_parameter( const ParameterDatum& param, const double min, const double max )
{
  return param->redraw( min, max );
}

ParameterDatum
exp_parameter( const ParameterDatum& param )
{
  return param->exp();
}

ParameterDatum
sin_parameter( const ParameterDatum& param )
{
  return param->sin();
}

ParameterDatum
cos_parameter( const ParameterDatum& param )
{
  return param->cos();
}

ParameterDatum
pow_parameter( const ParameterDatum& param, const double exponent )
{
  return param->pow( exponent );
}

ParameterDatum
dimension_parameter( const ParameterDatum& param_x, const ParameterDatum& param_y )
{
  return param_x->dimension_parameter( *param_y );
}

ParameterDatum
dimension_parameter( const ParameterDatum& param_x, const ParameterDatum& param_y, const ParameterDatum& param_z )
{
  return param_x->dimension_parameter( *param_y, *param_z );
}

ParameterDatum
create_parameter( const DictionaryDatum& param_dict )
{
  param_dict->clear_access_flags();

  ParameterDatum datum( NestModule::create_parameter( param_dict ) );

  ALL_ENTRIES_ACCESSED( *param_dict, "nest::CreateParameter", "Unread dictionary entries: " );

  return datum;
}

double
get_value( const ParameterDatum& param )
{
  librandom::RngPtr rng = get_global_rng();
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
  librandom::RngPtr rng = get_global_rng();
  for ( auto it = nc->begin(); it < nc->end(); ++it )
  {
    auto node = kernel().node_manager.get_node_or_proxy( ( *it ).node_id );
    result.push_back( param->value( rng, node ) );
  }
  return result;
}

std::vector< double >
apply( const ParameterDatum& param, const DictionaryDatum& positions )
{
  auto source_tkn = positions->lookup( names::source );
  auto source_nc = getValue< NodeCollectionPTR >( source_tkn );

  auto targets_tkn = positions->lookup( names::targets );
  TokenArray target_tkns = getValue< TokenArray >( targets_tkn );
  return param->apply( source_nc, target_tkns );
}

Datum*
node_collection_array_index( const Datum* datum, const long* array, unsigned long n )
{
  const NodeCollectionDatum node_collection = *dynamic_cast< const NodeCollectionDatum* >( datum );
  assert( node_collection->size() >= n );
  std::vector< index > node_ids;
  node_ids.reserve( n );

  for ( auto node_ptr = array; node_ptr != array + n; ++node_ptr )
  {
    node_ids.push_back( node_collection->operator[]( *node_ptr ) );
  }
  return new NodeCollectionDatum( NodeCollection::create( node_ids ) );
}

Datum*
node_collection_array_index( const Datum* datum, const bool* array, unsigned long n )
{
  const NodeCollectionDatum node_collection = *dynamic_cast< const NodeCollectionDatum* >( datum );
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
  return new NodeCollectionDatum( NodeCollection::create( node_ids ) );
}

} // namespace nest
