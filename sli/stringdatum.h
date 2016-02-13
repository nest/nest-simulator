/*
 *  stringdatum.h
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

#ifndef STRINGDATUM_H
#define STRINGDATUM_H

// C++ includes:
#include <string>
//  #include <typeinfo>

// Includes from sli:
#include "aggregatedatum.h"
#include "interpret.h"
#include "slifunction.h"

//  class StringDatum: public AggregateDatum<string,&SLIInterpreter::Stringtype>
//  {
//      Datum * clone(void) const
//      {
//          return new StringDatum(*this);
//      }
//  public:
//      StringDatum():
//              AggregateDatum<string,&SLIInterpreter::Stringtype>() {}
//      StringDatum(const string &s):
//              AggregateDatum<string,&SLIInterpreter::Stringtype>(s) {}
//      StringDatum(const StringDatum &d):
//              AggregateDatum<string,&SLIInterpreter::Stringtype>(d) {}
//      ~StringDatum()
//      {}

//      void pprint(ostream &) const;
//  };


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
sli::pool AggregateDatum< std::string, &SLIInterpreter::Stringtype >::memory;
#endif


template <>
void AggregateDatum< std::string, &SLIInterpreter::Stringtype >::pprint( std::ostream& out ) const;


typedef AggregateDatum< std::string, &SLIInterpreter::Stringtype > StringDatum;

void init_slistring( SLIInterpreter* );

class ToUppercase_sFunction : public SLIFunction
{
public:
  ToUppercase_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ToLowercase_sFunction : public SLIFunction
{
public:
  ToLowercase_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

#endif
