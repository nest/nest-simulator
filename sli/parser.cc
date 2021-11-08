/*
 *  parser.cc
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
    parser.cc
*/

#include "parser.h"

// Generated includes:
#include "config.h"

// Includes from sli:
#include "arraydatum.h"
#include "namedatum.h"
#include "scanner.h"
#include "symboldatum.h"

/*****************************************************************/
/* parse                                                         */
/* --------            Token --> Token                           */
/*                                                               */
/* Errors:                                                       */
/*                                                               */
/*                                                               */
/*                                                               */
/*****************************************************************/

// kann der ParseStack ein Stack von ArrayDatums sein ?????
// Token ist besser weil dann verschoben werden kann
void
Parser::init( std::istream& is )
{
  s = new Scanner( &is );

  arraytoken = ArrayDatum();
}

Parser::Parser( std::istream& is )
  : s( NULL )
  , ParseStack( 128 )
{
  init( is );
  assert( s != NULL );
}

Parser::Parser( void )
  : s( NULL )
  , ParseStack( 128 )
{
  init( std::cin );
  assert( s != NULL );
}


bool Parser::operator()( Token& t )
{
  assert( s != NULL );

  Token pt;

  bool ok;
  ParseResult result = scancontinue;

  do
  {
    if ( result == scancontinue )
    {
      ok = ( *s )( t );
    }
    else
    {
      ok = true;
    }


    if ( ok )
    {

      if ( t.contains( s->BeginProcedureSymbol ) )
      {
        ParseStack.push( new LitprocedureDatum() );
        ParseStack.top()->set_executable();
        result = scancontinue;
      }
      else if ( t.contains( s->BeginArraySymbol ) )
      {
        Token cb( new NameDatum( "[" ) );
        t.move( cb );
        result = tokencontinue;
      }
      else if ( t.contains( s->EndProcedureSymbol ) )
      {
        if ( not ParseStack.empty() )
        {
          ParseStack.pop_move( pt );
          if ( pt->isoftype( SLIInterpreter::Litproceduretype ) )
          {
            t.move( pt ); // procedure completed
            result = tokencontinue;
          }
          else
          {
            result = endarrayexpected;
          }
        }
        else
        {
          result = noopenproc;
        }
      }
      else if ( t.contains( s->EndArraySymbol ) )
      {
        Token ob( new NameDatum( "]" ) );
        t.move( ob );
        result = tokencontinue;
      }
      else if ( t.contains( s->EndSymbol ) )
      {
        if ( not ParseStack.empty() )
        {
          result = unexpectedeof;
          ParseStack.clear();
        }
        else
        {
          result = tokencompleted;
        }
      }
      else
      {
        // Now we should be left with a "simple" Token
        assert( not t->isoftype( SLIInterpreter::Symboltype ) );
        if ( not ParseStack.empty() )
        {
          // append token to array on stack
          ParseStack.pop_move( pt );
          if ( pt->isoftype( SLIInterpreter::Arraytype ) )
          {
            ArrayDatum* pa = dynamic_cast< ArrayDatum* >( pt.datum() );
            assert( pa != NULL );
            pa->push_back( t );
          }
          else // now it must be a procedure
          {
            LitprocedureDatum* pp = dynamic_cast< LitprocedureDatum* >( pt.datum() );
            assert( pp != NULL );
            pp->set_executable();
            pp->push_back( t );
          }
          ParseStack.push_move( pt );
          result = scancontinue;
        }
        else
        {
          result = tokencompleted;
        }
      }

    } // if(ok)
    //      else std::cerr << "<Scanner> : unable to scan input, Result:" << ok
    //      << '\n';
  } while ( ( result == tokencontinue ) || ( result == scancontinue ) );

  if ( result != tokencompleted )
  {
    switch ( result )
    {
    case noopenproc:
      s->print_error( "Open brace missing." );
      break;
    case endprocexpected:
      s->print_error( "Closed brace missing." );
      break;
    case noopenarray:
      s->print_error( "Open bracket missing." );
      break;
    case endarrayexpected:
      s->print_error( "Closed bracket missing." );
      break;
    case unexpectedeof:
      s->print_error( "Unexpected end of input." );
      break;
    default:
      break;
    }
    t = s->EndSymbol; // clear erroneous input
    return false;
  }
  return ( result == tokencompleted );
}

bool operator==( Parser const& p1, Parser const& p2 )
{
  return &p1 == &p2;
}

std::ostream& operator<<( std::ostream& out, const Parser& p )
{
  out << "Parser(" << p.scan() << ')' << std::endl;
  return out;
}
