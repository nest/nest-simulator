/*
 *  booldatum.cc
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

#include "booldatum.h"

// Includes from sli:
#include "name.h"
#include "token.h"

sli::pool BoolDatum::memory( sizeof( BoolDatum ), 1024, 1 );

const char* BoolDatum::true_string = "true";
const char* BoolDatum::false_string = "false";

BoolDatum::BoolDatum( const Name& val )
{
  d = ( val == Name( true_string ) );
}

BoolDatum::operator Name() const
{
  return ( d ? Name( true_string ) : Name( false_string ) );
}

BoolDatum::operator std::string() const
{
  return ( d ? std::string( true_string ) : std::string( false_string ) );
}

void
BoolDatum::input_form( std::ostream& out ) const
{
  print( out );
}

void
BoolDatum::pprint( std::ostream& out ) const
{
  print( out );
}

void
BoolDatum::print( std::ostream& out ) const
{
  out << ( d ? true_string : false_string );
}

void* BoolDatum::operator new( size_t size )
{
  if ( size != memory.size_of() )
  {
    return ::operator new( size );
  }
  return memory.alloc();
}

void BoolDatum::operator delete( void* p, size_t size )
{
  if ( p == NULL )
  {
    return;
  }
  if ( size != memory.size_of() )
  {
    ::operator delete( p );
    return;
  }
  memory.free( p );
}
