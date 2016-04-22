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
#include "gid_collection.h"
#include "nestmodule.h"

// Includes from sli:
#include "aggregatedatum.h"

typedef AggregateDatum< nest::ConnectionID, &nest::NestModule::ConnectionType >
  ConnectionDatum;
typedef AggregateDatum< nest::GIDCollection,
  &nest::NestModule::GIDCollectionType >
  GIDCollectionDatum;

#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool ConnectionDatum::memory;
template <>
sli::pool GIDCollectionDatum::memory;
#endif

template <>
void ConnectionDatum::print( std::ostream& ) const;
template <>
void GIDCollectionDatum::print( std::ostream& ) const;

template <>
void ConnectionDatum::pprint( std::ostream& ) const;
template <>
void GIDCollectionDatum::pprint( std::ostream& ) const;

#endif /* #ifndef NEST_DATUMS_H */
