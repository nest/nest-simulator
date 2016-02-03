/*
 *  symboldatum.h
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

#ifndef SYMBOLDATUM_H
#define SYMBOLDATUM_H

// Include all headers, needed to use token and datum objects

// C++ includes:
#include <string>
#include <typeinfo>

// Includes from sli:
#include "aggregatedatum.h"
#include "interpret.h"
#include "name.h"

/* These are declarations to specialize the static memory pool BEFORE
   we instantiate the AggregateDatum. Note, that this is onlz a declaration,
   because we do not provide an initializer (see ISO14882 Sec.  14.7.3.15.)
   The definition is given in the *.CC file with the appropriate
   initializer.

   Note that SUN's Forte 6.2 does not handle this correctly, so we have
   to use a compiler-switch.
*/
#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool AggregateDatum< Name, &SLIInterpreter::Symboltype >::memory;
#endif

class SymbolDatum : public AggregateDatum< Name, &SLIInterpreter::Symboltype >
{
  Datum*
  clone( void ) const
  {
    return new SymbolDatum( *this );
  }

public:
  SymbolDatum( const Name& n )
    : AggregateDatum< Name, &SLIInterpreter::Symboltype >( n )
  {
  }
  SymbolDatum( const SymbolDatum& n )
    : AggregateDatum< Name, &SLIInterpreter::Symboltype >( n )
  {
  }
  ~SymbolDatum()
  {
  }
};

#endif
