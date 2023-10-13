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
#include "name.h"
#include "sharedptrdatum.h"

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
  virtual ConnBuilder* create( NodeCollectionPTR,
    NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const DictionaryDatum& ) const = 0;
};

/**
 * Factory class for normal ConnBuilders
 */
template < typename ConnBuilderType, bool is_tripartite = ConnBuilderType::is_tripartite >
class ConnBuilderFactory : public GenericConnBuilderFactory
{
public:
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const override
  {
    assert( false ); // only specialisations should be called
  }

  //! create tripartite builder
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_specs ) const override
  {
    assert( false ); // only specialisations should be called
  }
};


// Specialisation for normal ConnBuilders
template < typename ConnBuilderType >
class ConnBuilderFactory< ConnBuilderType, false > : public GenericConnBuilderFactory
{
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const override
  {
    return new ConnBuilderType( sources, targets, conn_spec, syn_specs );
  }

  //! create tripartite builder
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_specs ) const override
  {
    throw IllegalConnection( String::compose(
      "Connection rule '%1' does not support tripartite connections.", ( *conn_spec )[ names::rule ] ) );
  }
};

// Specialisation for tripartite ConnBuilders
template < typename ConnBuilderType >
class ConnBuilderFactory< ConnBuilderType, true > : public GenericConnBuilderFactory
{
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const override
  {
    throw BadProperty(
      String::compose( "Connection rule %1 only supports tripartite connections.", ( *conn_spec )[ names::rule ] ) );
  }

  //! create tripartite builder
  ConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_specs ) const override
  {
    return new ConnBuilderType( sources, targets, third, conn_spec, syn_specs );
  }
};

} // namespace nest

#endif
