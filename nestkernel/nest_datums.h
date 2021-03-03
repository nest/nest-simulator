/*
 *  nest_datums.h
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

#ifndef NEST_DATUMS_H
#define NEST_DATUMS_H

/**
 * SLI Datum types related to the NEST kernel.
 */

// Includes from nestkernel:
#include "connection_id.h"
#include "node_collection.h"
#include "nestmodule.h"

// Includes from sli:
#include "aggregatedatum.h"
#include "sharedptrdatum.h"
#include "slitype.h"

#ifdef HAVE_LIBNEUROSIM
#include <neurosim/connection_generator.h>
typedef sharedPtrDatum< ConnectionGenerator, &nest::NestModule::ConnectionGeneratorType > ConnectionGeneratorDatum;
#endif

typedef AggregateDatum< nest::ConnectionID, &nest::NestModule::ConnectionType > ConnectionDatum;
typedef sharedPtrDatum< nest::NodeCollection, &nest::NestModule::NodeCollectionType > NodeCollectionDatum;
typedef sharedPtrDatum< nest::nc_const_iterator, &nest::NestModule::NodeCollectionIteratorType >
  NodeCollectionIteratorDatum;
typedef sharedPtrDatum< nest::Parameter, &nest::NestModule::ParameterType > ParameterDatum;

#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool ConnectionDatum::memory;
#endif

template <>
void ConnectionDatum::print( std::ostream& ) const;

template <>
void ConnectionDatum::pprint( std::ostream& ) const;
template <>
void NodeCollectionDatum::pprint( std::ostream& ) const;
template <>
void NodeCollectionIteratorDatum::pprint( std::ostream& ) const;

#endif /* #ifndef NEST_DATUMS_H */
