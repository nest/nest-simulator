/*
 *  connectiondatum.cpp
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

#include "connectiondatum.h"
#include "datumconverter.h"

#include "aggregatedatum_impl.h"

// explicit instantiation
template class AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>;

template<> sli::pool 
 AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::memory(
           sizeof(nest::ConnectionID),10000,1);


template<>
void AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::print(std::ostream &out) const
{
    out << "/connectiontype";
}

template<>
void AggregateDatum<nest::ConnectionID, &nest::NestModule::ConnectionType>::pprint(std::ostream &out) const
{
    out << "<" << source_gid_ << ',' << target_gid_ << ',' 
	<< target_thread_ << ',' << synapse_modelid_ << ',' << port_ << ">";
}

