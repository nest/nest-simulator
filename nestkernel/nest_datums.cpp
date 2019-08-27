/*
 *  nest_datums.cpp
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

#include "nest_datums.h"

// explicit instantiations
template class AggregateDatum< nest::ConnectionID, &nest::NestModule::ConnectionType >;
template class AggregateDatum< nest::GIDCollection, &nest::NestModule::GIDCollectionType >;

// instantiate memory management pool
template <>
sli::pool ConnectionDatum::memory( sizeof( nest::ConnectionID ), 10000, 1 );
template <>
sli::pool GIDCollectionDatum::memory( sizeof( nest::GIDCollection ), 10000, 1 );

// simple type printing
template <>
void
ConnectionDatum::print( std::ostream& out ) const
{
  out << "/connectiontype";
}
template <>
void
GIDCollectionDatum::print( std::ostream& out ) const
{
  out << "/gidcollectiontype";
}

// printing of the objects
template <>
void
ConnectionDatum::pprint( std::ostream& out ) const
{
  print_me( out );
}
template <>
void
GIDCollectionDatum::pprint( std::ostream& out ) const
{
  print_me( out );
}
