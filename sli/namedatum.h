/*
 *  namedatum.h
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

#ifndef NAMEDATUM_H
#define NAMEDATUM_H
/*
    Defines Datum classes which are derived from Names:
    class NameDatum;
    class LiteralDatum;
    class BoolDatum;
*/

// Include all headers, needed to use token and datum objects

// C++ includes:
#include <string>
#include <typeinfo>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "aggregatedatum.h"
#include "interpret.h"
#include "name.h"

/* These are declarations to specialize the static memory pool BEFORE
   we instantiate the AggregateDatum. Note, that this is only a declaration,
   because we do not provide an initializer (see ISO14882 Sec.  14.7.3.15.)
   The definition is given in the *.CC file with the appropriate
   initializer.

   Note that SUN's Forte 6.2 and 7 does not handle this correctly,
   so we have to use a compiler-switch. 11/2002 Gewaltig

   The Alpha cxx V6.3-002 says that storage class extern is not allowed here,
   so I removed it. 15.2.2002 Diesmann
*/
#ifndef HAVE_STATIC_TEMPLATE_DECLARATION_FAILS
template <>
sli::pool AggregateDatum< Name, &SLIInterpreter::Nametype >::memory;

template <>
sli::pool AggregateDatum< Name, &SLIInterpreter::Literaltype >::memory;
#endif


class NameDatum : public AggregateDatum< Name, &SLIInterpreter::Nametype >
{
  Datum*
  clone( void ) const
  {
    return new NameDatum( *this );
  }

  Datum*
  get_ptr()
  {
    Datum::addReference();
    return this;
  }

public:
  NameDatum( const Name& n )
    : AggregateDatum< Name, &SLIInterpreter::Nametype >( n )
  {
    set_executable();
  }
  NameDatum( const NameDatum& n )
    : AggregateDatum< Name, &SLIInterpreter::Nametype >( n )
  {
  }
  ~NameDatum()
  {
    set_executable();
  }
};

class LiteralDatum : public AggregateDatum< Name, &SLIInterpreter::Literaltype >
{
  Datum*
  clone( void ) const
  {
    return new LiteralDatum( *this );
  }

  Datum*
  get_ptr()
  {
    Datum::addReference();
    return this;
  }

public:
  LiteralDatum( const Name& n )
    : AggregateDatum< Name, &SLIInterpreter::Literaltype >( n )
  {
    set_executable();
  }
  LiteralDatum( const LiteralDatum& n )
    : AggregateDatum< Name, &SLIInterpreter::Literaltype >( n )
  {
    set_executable();
  }
  ~LiteralDatum()
  {
  }
  void pprint( std::ostream& ) const;
};

#endif
