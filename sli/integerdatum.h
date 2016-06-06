/*
 *  integerdatum.h
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

#ifndef INTEGERDATUM_H
#define INTEGERDATUM_H
/*
    class IntegerDatum
*/

// Includes from sli:
#include "interpret.h"
#include "numericdatum.h"


/* These are declarations to specialize the static memory pool BEFORE
   we instantiate the AggregateDatum. Note, that this is only a declaration,
   because we do not provide an initializer (see ISO14882 Sec.  14.7.3.15.)
   The definition is given in the *.CC file with the appropriate
   initializer.

   Note that SUN's Forte 6.2 does not handle this correctly, so we have
   to use a compiler-switch. 1.2002 Gewaltig

   The Alpha cxx V6.3-002 says that storage class extern is not allowed here,
   so I removed it. 15.2.2002 Diesmann
*/
#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool NumericDatum< long, &SLIInterpreter::Integertype >::memory;
#endif

typedef NumericDatum< long, &SLIInterpreter::Integertype > IntegerDatum;

#endif
