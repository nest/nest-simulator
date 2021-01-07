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

namespace nest
{

class ProjectionCollection
{
private:
  struct Projection_
  {
    const bool is_spatial;
    const NodeCollectionDatum sources;
    const NodeCollectionDatum targets;
    const DictionaryDatum conn_spec;
    const ArrayDatum syn_spec;

    Projection_( const ArrayDatum& projection, const bool is_spatial );

    void connect() const;
    void print_me( std::ostream& stream ) const;
  };

  std::vector< Projection_ > projections_;

public:
  ProjectionCollection() = delete;
  ProjectionCollection( const ArrayDatum& projections );

  ~ProjectionCollection() = default;

  void connect() const;
};


} // namespace nest

#endif /* #ifndef PROJECTION_COLLECTION_H */
