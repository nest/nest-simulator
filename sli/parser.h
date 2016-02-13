/*
 *  parser.h
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

#ifndef PARSER_H
#define PARSER_H
/*
    SLI's parser.
*/

// C++ includes:
#include <iostream>
#include <typeinfo>

// Includes from sli:
#include "scanner.h"
#include "token.h"
#include "tokenstack.h"


class Parser
{
  Scanner* s;

  Token arraytoken;
  Token proctoken;
  TokenStack ParseStack;

  enum ParseResult
  {
    tokencontinue,
    scancontinue,
    tokencompleted,
    noopenproc,
    endprocexpected,
    noopenarray,
    endarrayexpected,
    unexpectedeof
  };

  void init( std::istream& );

public:
  Parser( void );
  Parser( std::istream& );

  bool operator()( Token& );
  bool
  readToken( std::istream& is, Token& t )
  {
    s->source( &is );
    return operator()( t );
  }

  bool
  readSymbol( std::istream& is, Token& t )
  {
    s->source( &is );
    return s->operator()( t );
  }

  Scanner const*
  scan( void ) const
  {
    return s;
  }

  void
  clear_context()
  {
    if ( s != NULL )
    {
      s->clear_context();
    }
  }
};

bool operator==( Parser const&, Parser const& );

std::ostream& operator<<( std::ostream&, const Parser& );


#endif
