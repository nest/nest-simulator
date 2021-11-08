/*
 *  oosupport.cc
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

/*
    SLI's data access functions
*/

#include "oosupport.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictstack.h"
#include "namedatum.h"

void
OOSupportModule::init( SLIInterpreter* i )
{
  i->createcommand( "call", &callmemberfunction );
}

const std::string
OOSupportModule::commandstring( void ) const
{
  return std::string( "(oosupport.sli) run" );
}

const std::string
OOSupportModule::name( void ) const
{
  return std::string( "OOSupport" );
}

void
OOSupportModule::CallMemberFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict key call -> unknown

  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.pick( 1 ).datum() );
  assert( dict != NULL );
  LiteralDatum* key = dynamic_cast< LiteralDatum* >( i->OStack.pick( 0 ).datum() );
  assert( key != NULL );

  Token value = ( *dict )->lookup( *key );

  if ( value.datum() != NULL )
  {
    Token nt( new NameDatum( *key ) );
    i->DStack->push( *dict );
    i->EStack.pop(); // never forget me
    i->EStack.push( i->baselookup( i->end_name ) );
    i->EStack.push_move( nt );
    i->OStack.pop( 2 );
  }
  else
  {
    i->raiseerror( "UnknownMember" );
  }
}
