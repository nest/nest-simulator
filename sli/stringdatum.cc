/*
 *  stringdatum.cc
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

#include "stringdatum.h"

// C++ includes:
#include <algorithm>
#include <cctype>

// Includes from sli:
#include "tokenutils.h"

// initialization of static members requires template<>
// see Stroustrup C.13.1 --- HEP 2001-08-09
template <>
sli::pool AggregateDatum< std::string, &SLIInterpreter::Stringtype >::memory(
  sizeof( AggregateDatum< std::string, &SLIInterpreter::Stringtype > ),
  100,
  1 );

template <>
void
AggregateDatum< std::string, &SLIInterpreter::Stringtype >::pprint( std::ostream& out ) const
{
  out << '(';
  print( out );
  out << ')';
}

// explicit template instantiation needed
// because otherwise methods defined in
// numericdatum_impl.h will not be instantiated
// Moritz, 2007-04-16
template class AggregateDatum< std::string, &SLIInterpreter::Stringtype >;

const ToUppercase_sFunction touppercase_sfunction;
const ToLowercase_sFunction tolowercase_sfunction;

/** @BeginDocumentation
   Name: ToUppercase - Convert a string to upper case.
   Synopsis:
   (string) ToUppercase -> (string)
   Description:
   ToUppercase converts a string to upper case. If no upper case
   representation of a letter exists, the letter is kept unchanged.

   Examples:
   SLI ] (MiXeD cAsE) ToUppercase
   (MIXED CASE)

   Author: Jochen Martin Eppler
   SeeAlso: ToLowercase
*/
void
ToUppercase_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum sd = getValue< StringDatum >( i->OStack.top() );
  std::string* str = dynamic_cast< std::string* >( &sd );
  std::transform( str->begin(), str->end(), str->begin(), toupper );

  i->OStack.pop();
  i->OStack.push( new StringDatum( str->c_str() ) );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: ToLowercase - Convert a string to lower case.
   Synopsis:
   (string) ToLowercase -> (string)
   Description:
   ToLowercase converts a string to lower case. If no lower case
   representation of a letter exists, the letter is kept unchanged.

   Examples:
   SLI ] (MiXeD cAsE) ToLowercase
   (mixed case)

   Author: Jochen Martin Eppler
   SeeAlso: ToUppercase
*/
void
ToLowercase_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum sd = getValue< StringDatum >( i->OStack.top() );
  std::string* str = dynamic_cast< std::string* >( &sd );
  std::transform( str->begin(), str->end(), str->begin(), tolower );

  i->OStack.pop();
  i->OStack.push( new StringDatum( str->c_str() ) );
  i->EStack.pop();
}

void
init_slistring( SLIInterpreter* i )
{
  i->createcommand( "ToUppercase", &touppercase_sfunction );
  i->createcommand( "ToLowercase", &tolowercase_sfunction );
}
