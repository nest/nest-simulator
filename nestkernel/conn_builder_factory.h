/*
 *  conn_builder_factory.h
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

#ifndef CONN_BUILDER_FACTORY_H
#define CONN_BUILDER_FACTORY_H

// C++ includes:
#include <map>

// Includes from nestkernel:
#include "conn_builder.h"

// Includes from sli:
#include "dictdatum.h"
#include "sharedptrdatum.h"
#include "name.h"

namespace nest
{
/**
 * Generic factory class for ConnBuilder objects.
 *
 * This factory allows for flexible registration
 * of ConnBuilder subclasses and object creation.
 *
 */
class GenericConnBuilderFactory
{
public:
  virtual ~GenericConnBuilderFactory()
  {
  }
  virtual ConnBuilder* create( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& ) const = 0;
};

/**
 * Factory class for generating objects of type ConnBuilder
 */

template < typename ConnBuilderType >
class ConnBuilderFactory : public GenericConnBuilderFactory
{

public:
  //! create conn builder
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const
  {
    return new ConnBuilderType( sources, targets, conn_spec, syn_specs );
  }
};

} // namespace nest

#endif
