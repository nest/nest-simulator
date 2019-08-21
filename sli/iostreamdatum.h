/*
 *  iostreamdatum.h
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

#ifndef IOSTREAMDATUM_H
#define IOSTREAMDATUM_H
/*
    Datum classes encapsulating c++ streams
*/

// Include all headers, needed to use token and datum objects

// C++ includes:
#include <iostream>
#include <typeinfo>

// Includes from sli:
#include "interpret.h"
#include "lockptrdatum.h"

typedef lockPTRDatum< std::istream, &SLIInterpreter::Istreamtype > IstreamDatum;
typedef lockPTRDatum< std::istream, &SLIInterpreter::XIstreamtype > XIstreamDatum;
typedef lockPTRDatum< std::ostream, &SLIInterpreter::Ostreamtype > OstreamDatum;
// typedef lockPTRDatum<std::iostream,&SLIInterpreter::IOstreamtype>
// IOstreamDatum;

#endif
