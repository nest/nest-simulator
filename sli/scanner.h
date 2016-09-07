/*
 *  scanner.h
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

#ifndef Scanner_H
#define Scanner_H
/*
    SLI's lexical analyzer

*/

// C++ includes:
#include <iostream>
#include <string>
#include <typeinfo>

// Includes from sli:
#include "charcode.h"
#include "symboldatum.h"
#include "token.h"


class Scanner
{
  std::istream* in;

  CharCode code; // some Chars cause the same Transition

  unsigned long line;  //!< Current line number
  unsigned long col;   //!< Current column in line
  std::string context; //!< Current context.
  std::string old_context;
  enum ScanStates
  {
    start = 0,
    intdgtst,
    intexpst,
    nullst,
    aheadintst,
    decpfirstst,
    decpdgtst,
    decpointst,
    fracdgtst,
    minusst,
    plusst,
    aheadsgst,
    sgalphast,
    aheadfracst,
    expntlst,
    plexpst,
    mnexpst,
    openbracketst,
    closebracketst,
    openbracest,
    closebracest,
    openparst,
    closeparst,
    expdigst,
    dotalphast,
    alphast,
    aheadalphst,
    slashst,
    literalst,
    aheadlitst,
    percentst,
    whitest,
    asteriskst,
    ccommentst,
    startstringst,
    stringst,
    backslashst,
    newlinest,
    tabulatorst,
    backslashcst,
    oparenthcst,
    cparenthcst,
    end,
    eofst,
    error,
    lastscanstate
  };

  enum Codes
  {
    invalid = 0,
    whitespace,
    eof,
    digit,
    null,
    expntl,
    decpoint,
    plus,
    minus,
    openbracket,
    closebracket,
    openbrace,
    closebrace,
    openparenth,
    closeparenth,
    alpha,
    slash,
    backslash,
    newline,
    tabulator,
    asterisk,
    percent,
    eoln,
    other,
    lastcode
  };

  enum ScanStates trans[ lastscanstate ][ lastcode ]; // DFA transition-function


  const unsigned char space;
  const unsigned char tab;
  const unsigned char endoln;
  const unsigned char cr; // carriage return (ctrl M)
  const unsigned char endof;
  int
  digval( unsigned char c ) const
  {
    return c - '0';
  }


public:
  SymbolDatum BeginArraySymbol;
  SymbolDatum EndArraySymbol;
  SymbolDatum BeginProcedureSymbol;
  SymbolDatum EndProcedureSymbol;
  SymbolDatum EndSymbol;

  /**
   * Initialise scanner with a stream.
   */
  Scanner( std::istream* );

  /**
   * Set new input stream.
   */
  void source( std::istream* );

  /**
   * Return input stream.
   */
  std::istream*
  GetSource( void )
  {
    return in;
  }

  /**
   * Scan input until the next Token.
   */
  bool operator()( Token& );

  /**
   * Issue an error message.
   */
  void print_error( const char* );

  /**
   * Return current line number in input stream.
   */
  unsigned long
  get_line() const
  {
    return line;
  }

  /**
   * Return position in  current line in input stream.
   */
  unsigned long
  get_col() const
  {
    return col;
  }

  void
  clear_context()
  {
    line = 0;
    col = 0;
    context.clear();
    old_context.clear();
  }
};

#endif
