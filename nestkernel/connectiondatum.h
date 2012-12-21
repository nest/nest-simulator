/*
 *  connectiondatum.h
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

#ifndef CONNECTIONDATUM_H
#define CONNECTIONDATUM_H

/**
 * SLI Datum types related to the NEST kernel.
 */

#include "nestmodule.h"
#include "connection_id.h"
#include "aggregatedatum.h"


#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template<>
sli::pool AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::memory;
#endif
template<>
void AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::print(std::ostream &) const;
template<>
void AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::pprint(std::ostream &) const;

typedef AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType> ConnectionDatum;


#endif
