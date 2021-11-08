/*
 *  arraydatum.h
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

#ifndef ARRAYDATUM_H
#define ARRAYDATUM_H

// C++ includes:
#include <vector>

// Includes from sli:
#include "aggregatedatum.h"
#include "interpret.h"
#include "lockptrdatum.h"
#include "tokenarray.h"

/* These are declarations to specialize the static memory pool BEFORE
   we instantiate the AggregateDatum. Note, that this is only a declaration,
   because we do not provide an initializer (see ISO14882 Sec.  14.7.3.15.)
   The definition is given in the *.CC file with the appropriate
   initializer.

   Note that SUN's Forte 6.2 does not handle this correctly, so we have
   to use a compiler-switch.
*/


#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool AggregateDatum< TokenArray, &SLIInterpreter::Arraytype >::memory;
template <>
sli::pool AggregateDatum< TokenArray, &SLIInterpreter::Proceduretype >::memory;
template <>
sli::pool AggregateDatum< TokenArray, &SLIInterpreter::Litproceduretype >::memory;
#endif

template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Arraytype >::print( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Arraytype >::pprint( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Proceduretype >::print( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Proceduretype >::list( std::ostream&, std::string, int ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Proceduretype >::pprint( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Litproceduretype >::print( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Litproceduretype >::pprint( std::ostream& ) const;
template <>
void AggregateDatum< TokenArray, &SLIInterpreter::Litproceduretype >::list( std::ostream&, std::string, int ) const;
template <>
void lockPTRDatum< std::vector< long >, &SLIInterpreter::IntVectortype >::pprint( std::ostream& out ) const;
template <>
void lockPTRDatum< std::vector< double >, &SLIInterpreter::DoubleVectortype >::pprint( std::ostream& out ) const;

/**
 * @remark This type was introduced to pass numeric arrays between
 *         Python and nodes. It is *not* meant for general use. The
 *         current implementation is minimal and this was done on
 *         purpose. While it is useful to have numeric arrays at the
 *         level of SLI, this would require the implementation of many
 *         functions, to make them useful. For example there are no
 *         functions to access the data in such arrays. (MOG 2009-01-23, see
 *         #253)
 */
typedef lockPTRDatum< std::vector< long >, &SLIInterpreter::IntVectortype > IntVectorDatum;

/**
 * @remark This type was introduced to pass numeric arrays between
 *         Python and nodes. It is *not* meant for general use. The
 *         current implementation is minimal and this was done on
 *         purpose. While it is useful to have numeric arrays at the
 *         level of SLI, this would require the implementation of many
 *         functions, to make them useful. For example there are no
 *         functions to access the data in such arrays. (MOG 2009-01-23, see
 *         #253)
 */
typedef lockPTRDatum< std::vector< double >, &SLIInterpreter::DoubleVectortype > DoubleVectorDatum;

typedef AggregateDatum< TokenArray, &SLIInterpreter::Arraytype > ArrayDatum;
typedef AggregateDatum< TokenArray, &SLIInterpreter::Proceduretype > ProcedureDatum;
typedef AggregateDatum< TokenArray, &SLIInterpreter::Litproceduretype > LitprocedureDatum;


#endif
