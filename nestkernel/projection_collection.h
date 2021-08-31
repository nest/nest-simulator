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

#include "dictutils.h"
#include "nest_datums.h"
#include "connection_creator.h"

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

      void compute_ntree();

      const NodeCollectionPTR sources;
      const NodeCollectionPTR targets;
      ConnectionCreator spatial_builder;
    };
    ConnectionClassWrapper_() = delete;
    // Regular connections
    ConnectionClassWrapper_( ConnBuilder* const );
    // Spatial connections
    ConnectionClassWrapper_( SpatialBuilderWrapper_* const );

    void connect();
    void compute_ntree();

  private:
    // Regular connections
    ConnBuilder* const conn_builder_;
    // Spatial connections
    SpatialBuilderWrapper_* const spatial_conn_creator_;
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
