/*
 *  namedatum.cc
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

#include "namedatum.h"

// initialization of static members requires template<>
// see Stroustrup C.13.1 --- HEP 2001-08-09
template <>
sli::pool AggregateDatum< Name, &SLIInterpreter::Nametype >::memory( sizeof( NameDatum ), 10240, 1 );
template <>
sli::pool AggregateDatum< Name, &SLIInterpreter::Literaltype >::memory( sizeof( LiteralDatum ), 10240, 1 );


// explicit template instantiation needed
// because otherwise methods defined in
// numericdatum_impl.h will not be instantiated
// Moritz, 2007-04-16
template class AggregateDatum< Name, &SLIInterpreter::Nametype >;
template class AggregateDatum< Name, &SLIInterpreter::Literaltype >;
