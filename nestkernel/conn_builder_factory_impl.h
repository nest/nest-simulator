/*
 *  conn_builder_factory_impl.h
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

#ifndef CONN_BUILDER_FACTORY_IMPL_H
#define CONN_BUILDER_FACTORY_IMPL_H

#include "conn_builder_factory.h"

namespace nest
{

/**
 * Factory class for bipartite ConnBuilders
 */
template < typename ConnBuilderType >
class BipartiteConnBuilderFactory : public GenericBipartiteConnBuilderFactory
{
public:
  BipartiteConnBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const override
  {
    return new ConnBuilderType( sources, targets, third_out, conn_spec, syn_specs );
  }
};


/**
 * Factory class for Third-factor ConnBuilders
 */
template < typename ThirdConnBuilderType >
class ThirdConnBuilderFactory : public GenericThirdConnBuilderFactory
{
public:
  ThirdOutBuilder*
  create( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdInBuilder* third_in,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs ) const override
  {
    return new ThirdConnBuilderType( sources, targets, third_in, conn_spec, syn_specs );
  }
};


} // namespace nest

#endif // CONN_BUILDER_FACTORY_IMPL_H
