/*
 *  projection_collection.cpp
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
 *  GNU General Public License for moreconnection_ details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>

#include "connection_creator_impl.h"
#include "kernel_manager.h"
#include "projection_collection.h"
#include "spatial.h"

namespace nest
{

ProjectionCollection::ProjectionCollection( const ArrayDatum& projections )
{
  for ( auto& proj_token : projections )
  {
    const auto projection_array = getValue< ArrayDatum >( proj_token );
    // Normal projection has 4 elements, spatial projection has 3.
    assert( projection_array.size() == 4 or projection_array.size() == 3 );
    const bool is_spatial = projection_array.size() == 3;
    auto sources = getValue< NodeCollectionDatum >( projection_array[ 0 ] );
    auto targets = getValue< NodeCollectionDatum >( projection_array[ 1 ] );
    auto conn_spec = getValue< DictionaryDatum >( projection_array[ 2 ] );
    if ( is_spatial )
    {
      projections_.emplace_back(
        std::make_unique< ConnectionClassWrapper_::SpatialBuilderWrapper_ >( sources, targets, conn_spec ) );
      post_spatial_connector_creation_checks( conn_spec ); // checks of dictionary access flags
    }
    else
    {
      auto syn_spec = getValue< ArrayDatum >( projection_array[ 3 ] );
      std::vector< DictionaryDatum > synapse_params( syn_spec.size() );
      // Convert ArrayDatum of tokens to vector of DictionaryDatums.
      std::transform( syn_spec.begin(),
        syn_spec.end(),
        synapse_params.begin(),
        // Lambda expression that handles the conversion of each element.
        []( Token& token ) -> DictionaryDatum { return getValue< DictionaryDatum >( token ); } );
      // Need to do the same checks of arguments as in ConnectionManager::connect().
      pre_connector_creation_checks( sources, targets, conn_spec, synapse_params );
      const auto rule_name = static_cast< const std::string >( ( *conn_spec )[ names::rule ] );
      projections_.emplace_back(
        kernel().connection_manager.get_conn_builder( rule_name, sources, targets, conn_spec, synapse_params ) );
      post_connector_creation_checks( conn_spec, synapse_params ); // checks of dictionary access flags
    }
  }
}

void
ProjectionCollection::connect()
{
  kernel().connection_manager.set_connections_have_changed();

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();

    try
    {
      // Apply projection connections
      for ( auto& projection : projections_ )
      {
        projection.connect();
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
  for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised.at( tid ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised.at( tid ) ) );
    }
  }
}

void
ProjectionCollection::pre_connector_creation_checks( NodeCollectionPTR& sources,
  NodeCollectionPTR& targets,
  DictionaryDatum& conn_spec,
  std::vector< DictionaryDatum >& syn_specs )
{
  // Copied from ConnectionManager::connect()

  if ( sources->empty() )
  {
    throw IllegalConnection( "Presynaptic nodes cannot be an empty NodeCollection" );
  }
  if ( targets->empty() )
  {
    throw IllegalConnection( "Postsynaptic nodes cannot be an empty NodeCollection" );
  }

  conn_spec->clear_access_flags();

  for ( auto syn_params : syn_specs )
  {
    syn_params->clear_access_flags();
  }

  if ( not conn_spec->known( names::rule ) )
  {
    throw BadProperty( "Connectivity spec must contain connectivity rule." );
  }
  const Name rule_name = static_cast< const std::string >( ( *conn_spec )[ names::rule ] );

  if ( not kernel().connection_manager.valid_connection_rule( rule_name.toString() ) )
  {
    throw BadProperty( String::compose( "Unknown connectivity rule: %1", rule_name ) );
  }
}

void
ProjectionCollection::post_connector_creation_checks( DictionaryDatum& conn_spec,
  std::vector< DictionaryDatum >& syn_specs )
{
  // Copied from ConnectionManager::connect()

  ALL_ENTRIES_ACCESSED( *conn_spec, "Connect", "Unread dictionary entries in conn_spec: " );
  for ( auto syn_params : syn_specs )
  {
    ALL_ENTRIES_ACCESSED( *syn_params, "Connect", "Unread dictionary entries in syn_spec: " );
  }
}
void
ProjectionCollection::post_spatial_connector_creation_checks( DictionaryDatum& connection_dict )
{
  // Copied from connect_layers()

  ALL_ENTRIES_ACCESSED( *connection_dict, "nest::CreateLayers", "Unread dictionary entries: " );
}


ProjectionCollection::ConnectionClassWrapper_::ConnectionClassWrapper_( std::unique_ptr< ConnBuilder > conn_builder )
  : conn_builder_( std::move( conn_builder ) )
  , spatial_conn_creator_( nullptr )
{
}

ProjectionCollection::ConnectionClassWrapper_::ConnectionClassWrapper_(
  std::unique_ptr< SpatialBuilderWrapper_ > spatial_builder )
  : conn_builder_( nullptr )
  , spatial_conn_creator_( std::move( spatial_builder ) )
{
}

void
ProjectionCollection::ConnectionClassWrapper_::connect()
{
  if ( conn_builder_ )
  {
    assert( not spatial_conn_creator_ );
    conn_builder_->connect();
  }
  else
  {
    assert( not conn_builder_ );
    spatial_conn_creator_->connect();
  }
}

ProjectionCollection::ConnectionClassWrapper_::SpatialBuilderWrapper_::SpatialBuilderWrapper_(
  const NodeCollectionDatum sources,
  const NodeCollectionDatum targets,
  const DictionaryDatum conn_dict )
  : sources( sources )
  , targets( targets )
  , source_layer( get_layer( sources ) )
  , target_layer( get_layer( targets ) )
  , spatial_builder( ConnectionCreator( conn_dict ) )
{
  source_layer->create_pool_if_needed( sources, target_layer, spatial_builder );
}

void
ProjectionCollection::ConnectionClassWrapper_::SpatialBuilderWrapper_::connect()
{
  source_layer->connect( sources, target_layer, targets, spatial_builder );
}

} // namespace nest
