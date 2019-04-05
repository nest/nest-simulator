/*
 *  token.cc
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

#include "token.h"

// C++ includes:
#include <algorithm>

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "datum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "name.h"
#include "namedatum.h"
#include "stringdatum.h"
#include "tokenarray.h"
#include "tokenutils.h"


/***********************************************************/
/* Definitions for Token                                       */
/***********************************************************/

// The copy-contructor must perform a kind of bootstrapping,
// since we cannot use the copy-contructor of the datum to
// create the new entry.
// Thus, this constructor must only be called by the
// (virtual) Datum members who create new Datums


Token::Token( int value )
{
  p = new IntegerDatum( value );
}

Token::Token( unsigned int value )
{
  p = new IntegerDatum( value );
}

Token::Token( long value )
{
  p = new IntegerDatum( value );
}

Token::Token( unsigned long value )
{
  p = new IntegerDatum( value );
}

#ifdef HAVE_32BIT_ARCH
Token::Token( uint64_t value )
{
  p = new IntegerDatum( value );
}
#endif

Token::Token( double value )
{
  p = new DoubleDatum( value );
}

Token::Token( bool value )
{
  p = new BoolDatum( value );
}

Token::Token( const char* value )
{
  p = new StringDatum( value );
}

Token::Token( std::string value )
{
  p = new StringDatum( value );
}

Token::Token( const std::vector< long >& value )
{
  p = new ArrayDatum( value );
}

Token::Token( const std::vector< size_t >& value )
{
  p = new ArrayDatum( value );
}

Token::Token( const std::vector< double >& value )
{
  p = new ArrayDatum( value );
}

/*
Token::operator Datum* () const
{
  return p;
}
*/

Token::operator long() const
{
  return getValue< long >( *this );
}

Token::operator size_t() const
{
  return getValue< long >( *this );
}

Token::operator double() const
{
  return getValue< double >( *this );
}

Token::operator float() const
{
  return getValue< float >( *this );
}

Token::operator bool() const
{
  return getValue< bool >( *this );
}

Token::operator std::string() const
{
  return getValue< std::string >( *this );
}

void
Token::info( std::ostream& out ) const
{
  out << "Token::info\n";
  if ( p )
  {
    p->Datum::info( out );

    out << "p    = " << p << std::endl;

    out << "Type = " << type().name() << std::endl;
    p->info( out );
  }
  else
  {
    out << "<NULL token>\n";
  }
}

void
Token::pprint( std::ostream& out ) const
{
  if ( not p )
  {
    out << "<Null token>";
  }
  else
  {
    p->pprint( out );
  }
}

std::ostream& operator<<( std::ostream& out, const Token& c )
{
  if ( not c )
  {
    out << "<Null token>";
  }
  else
  {
    c->print( out );
  }
  return out;
}

bool
Token::matches_as_string( const Token& rhs ) const
{
  try
  {
    const std::string& left = getValue< std::string >( *this );
    const std::string& right = getValue< std::string >( rhs );
    return left == right;
  }
  catch ( TypeMismatch& )
  {
    return false;
  }
  return false;
}
