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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>

#include "projection_collection.h"
#include "kernel_manager.h"
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
    projections_.emplace_back( projection_array, is_spatial );
  }
}

void
ProjectionCollection::connect() const
{
  // TODO: Process projections here

  // Apply projection connections
  for ( auto& projection : projections_ )
  {
    projection.connect();
  }
}

ProjectionCollection::Projection_::Projection_( const ArrayDatum& projection, const bool is_spatial )
  : is_spatial( is_spatial )
  , sources( getValue< NodeCollectionDatum >( projection[ 0 ] ) )
  , targets( getValue< NodeCollectionDatum >( projection[ 1 ] ) )
  , conn_spec( getValue< DictionaryDatum >( projection[ 2 ] ) )
  , syn_spec( is_spatial ? ArrayDatum() : getValue< ArrayDatum >( projection[ 3 ] ) )
{
}

void
ProjectionCollection::Projection_::connect() const
{
  if ( is_spatial )
  {
    connect_layers( sources, targets, conn_spec );
  }
  else
  {
    // Convert synapse parameters to vector
    std::vector< DictionaryDatum > synapse_params( syn_spec.size() );
    std::transform( syn_spec.begin(),
      syn_spec.end(),
      synapse_params.begin(),
      []( Token& token ) -> DictionaryDatum
      {
        return getValue< DictionaryDatum >( token );
      } );
    kernel().connection_manager.connect( sources, targets, conn_spec, synapse_params );
  }
}

void
ProjectionCollection::Projection_::print_me( std::ostream& stream ) const
{
  stream << "Projection:\nSources: ";
  sources->print_me( stream );
  stream << "\nTargets: ";
  targets->print_me( stream );
  stream << "\n";
  conn_spec->info( stream );
  if ( not is_spatial )
  {
    for ( auto& spec : syn_spec )
    {
      spec->info( stream );
    }
  }
}

} // namespace nest
