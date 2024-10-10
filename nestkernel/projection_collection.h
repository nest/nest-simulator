/*
 *  projection_collection.h
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

#ifndef PROJECTION_COLLECTION_H
#define PROJECTION_COLLECTION_H

#include <vector>

#include "connection_creator.h"
#include "dictutils.h"
#include "nest_datums.h"
#include "spatial.h"


namespace nest
{

class ProjectionCollection
{
private:
  class ConnectionClassWrapper_
  {
  public:
    /**
     * @brief Wrapper to adapt ConnectionCreator to behave like a ConnBuilder.
     */
    struct SpatialBuilderWrapper_
    {
      SpatialBuilderWrapper_() = delete;
      SpatialBuilderWrapper_( const NodeCollectionDatum, const NodeCollectionDatum, const DictionaryDatum );

      void connect();

      const NodeCollectionPTR sources;
      const NodeCollectionPTR targets;

      AbstractLayerPTR source_layer;
      AbstractLayerPTR target_layer;

      ConnectionCreator spatial_builder;
    };
    ConnectionClassWrapper_() = delete;
    ConnectionClassWrapper_( const ConnectionClassWrapper_& ) = delete;
    ConnectionClassWrapper_( ConnectionClassWrapper_&& ) = default;
    // Regular connections
    ConnectionClassWrapper_( std::unique_ptr< ConnBuilder > );
    // Spatial connections
    ConnectionClassWrapper_( std::unique_ptr< SpatialBuilderWrapper_ > );

    void connect();

  private:
    // Regular connections
    std::unique_ptr< ConnBuilder > conn_builder_;
    // Spatial connections
    std::unique_ptr< SpatialBuilderWrapper_ > spatial_conn_creator_;
  };

  std::vector< ConnectionClassWrapper_ > projections_;

public:
  ProjectionCollection() = delete;
  ProjectionCollection( const ArrayDatum& projections );

  ~ProjectionCollection() = default;

  void pre_connector_creation_checks( NodeCollectionPTR&,
    NodeCollectionPTR&,
    DictionaryDatum&,
    std::vector< DictionaryDatum >& );
  void post_connector_creation_checks( DictionaryDatum&, std::vector< DictionaryDatum >& );
  void post_spatial_connector_creation_checks( DictionaryDatum& );
  void connect();
};


} // namespace nest

#endif /* #ifndef PROJECTION_COLLECTION_H */
