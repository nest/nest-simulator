/*
 *  scanner.cc
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
    scanner.cc
*/

#include "scanner.h"

// C++ includes:
#include <cmath>
#include <limits>
#include <sstream>

// Includes from sli:
#include "doubledatum.h"
#include "integerdatum.h"
#include "namedatum.h"
#include "stringdatum.h"
#include "symboldatum.h"

/*************************************************************************/
/** Scanner   (implemented as a DFA)                                     */
/* -------                                                               */
/*         Scanner uses a symbol processor which controls all operations */
/*         on symbols (strings with a unique id) instead of a static     */
/*         symbol table.                                                 */
/*                                                                       */
/*          still experimental...                                        */
/*             - uses an explicit transition function, which can         */
/*               not be constructed from regular expression              */
/*               (unlike in lex).                                        */
/*             - no clear interface between the basic functions and a    */
/*               a particular application.                               */
/*             - once we are sure about the final structure of this file */
/*               we will as usual move this comment to scanner.h         */
/*                                                                       */
/* Problems:                                                             */
/*           The scanner puts the newline character back into the        */
/*           stream at the end of a line, thus the next read             */
/*           operation performed on that stream returns the newline      */
/*           character. This case, i.e. a token is terminated by endoln  */
/*           should be treated separately to avoid this.                 */
/*           The current workarount checks for endoln at each unget()    */
/*           call!                                                       */
/*           24. June 1997, Gewaltig                                     */
/*                                                                       */
/*                                                                       */
/* History:                                                              */
/*         (7) 080505, Diesmann. minusst can now be followed by minus    */
/*             to enable right arrows like -->                           */
/*         (6) 071002, Diesmann. Replaced own conversion to double by    */
/*             library function std::atof(). Now compatible with cvd.    */
/*         (5) 8.4.1997, Adaption debuged. Special explicit start entr   */
/*            in switch, to reserve default for errors. Previous version */
/*            did not eat white spaces.                                  */
/*         (4) March 1997, Gewaltig; Adapted for SLI 2.0                 */
/*         (3) 5.8.1994, Diesmann                                        */
/*             We are now trying to bring the project to a state where   */
/*             it can be used by the public.                             */
/*             We noticed that on some compilers the 'put to' operator   */
/*             is not defined for type (long double) and we are not sure */
/*             that it is part of the ANSI standard. As we are currently */
/*             not using this type we haven't implemented it. The        */
/*             'put to' operator for class Token now returns the string  */
/*             "sorry, no 'put to' for (long double)\n" instead.         */
/*         (2) Oct. 1993, applied to sns Gewaltig, Diesmann              */
/*         (1) string introduced by mog. 5.10.93                         */
/*         (0) first version, Summer 1993 diesmann                       */
/*                                                                       */
/*************************************************************************/

/***************************************************************************/

Scanner::Scanner( std::istream* is )
  : in( is )
  , code( std::numeric_limits< unsigned char >::max(), invalid )
  , line( 0 )
  , col( 0 )
  , space( 32 )
  , tab( 9 )
  , endoln( 10 )
  , cr( 13 )
  , endof( 4 )
  , BeginArraySymbol( "/BeginArraySymbol" )
  , // these symbol-names cannot be entered
  EndArraySymbol( "/EndArraySymbol" )
  , // by keyboard! This is important to ensure
  BeginProcedureSymbol( "/BeginProcedureSymbol" )
  , // the integrity of the scanner/parser
  EndProcedureSymbol( "/EndProcedureSymbol" )
  , // interaction: Non-terminal symbols
  EndSymbol( "/EndSymbol" )
{
  for ( size_t s = start; s < lastscanstate; ++s )
  {
    for ( size_t c = invalid; c < lastcode; ++c )
    {
      trans[ s ][ c ] = error;
    }
  }

  code[ space ] = whitespace;
  code[ tab ] = whitespace;
  code[ endof ] = eof;

  code[ '+' ] = plus;
  code[ '-' ] = minus;

  code[ '[' ] = openbracket;
  code[ ']' ] = closebracket; // see Kernighan p.7
  code[ '{' ] = openbrace;
  code[ '}' ] = closebrace;
  code[ '(' ] = openparenth;  // for string implementation
  code[ ')' ] = closeparenth; // for string implementation

  code[ '.' ] = decpoint;
  code[ '0' ] = null;
  code.Group( expntl, "Ee" );
  code.Group( digit, "123456789" );

  code.Group( alpha, "ABCDFGHIJKLMNOPQRSTUVWXYZ" );
  code.Group( alpha, "abcdfghijklmopqrsuvwxyz" );
  code.Range( alpha, ( char ) 161, ( char ) 255 );
  code[ '_' ] = alpha;
  code.Group( alpha, "~`!@#$^&=|:;'<,>?\"" ); // according to PS

  code[ '/' ] = slash;
  code[ '\\' ] = backslash; // used for escapes in strings
  code[ 'n' ] = newline;    // newline escape \n
  code[ 't' ] = tabulator;  // tabulator escape \t

  code[ '*' ] = asterisk;
  code[ '%' ] = percent;
  code[ endoln ] = eoln;
  code[ cr ] = eoln;


  trans[ start ][ whitespace ] = start;
  trans[ start ][ eoln ] = start;
  trans[ start ][ minus ] = minusst;
  trans[ start ][ plus ] = plusst;
  trans[ start ][ digit ] = intdgtst;
  trans[ start ][ null ] = nullst;
  trans[ start ][ decpoint ] = decpfirstst;
  trans[ start ][ openbracket ] = openbracketst;
  trans[ start ][ closebracket ] = closebracketst;
  trans[ start ][ openbrace ] = openbracest;
  trans[ start ][ closebrace ] = closebracest;
  trans[ start ][ alpha ] = alphast;
  trans[ start ][ asterisk ] = alphast;
  trans[ start ][ newline ] = alphast;
  trans[ start ][ tabulator ] = alphast;
  trans[ start ][ backslash ] = alphast;
  trans[ start ][ expntl ] = alphast;
  trans[ start ][ slash ] = slashst;
  trans[ start ][ percent ] = percentst;
  trans[ start ][ eof ] = eofst;
  trans[ start ][ openparenth ] = startstringst;

  trans[ minusst ][ digit ] = intdgtst;
  trans[ minusst ][ null ] = nullst;
  trans[ minusst ][ decpoint ] = decpfirstst;
  trans[ minusst ][ alpha ] = sgalphast;
  trans[ minusst ][ minus ] = sgalphast; // must be name
  trans[ minusst ][ newline ] = sgalphast;
  trans[ minusst ][ tabulator ] = sgalphast;
  trans[ minusst ][ backslash ] = sgalphast;
  trans[ minusst ][ expntl ] = alphast;
  trans[ minusst ][ whitespace ] = aheadsgst;
  trans[ minusst ][ eoln ] = aheadsgst;
  trans[ minusst ][ openbracket ] = aheadsgst;
  trans[ minusst ][ openbrace ] = aheadsgst;
  trans[ minusst ][ closebracket ] = aheadsgst;
  trans[ minusst ][ closebrace ] = aheadsgst;
  trans[ minusst ][ percent ] = aheadsgst;
  trans[ minusst ][ openparenth ] = aheadsgst;
  trans[ minusst ][ slash ] = aheadsgst;
  trans[ minusst ][ eof ] = aheadsgst;

  trans[ plusst ][ digit ] = intdgtst;
  trans[ plusst ][ null ] = nullst;
  trans[ plusst ][ decpoint ] = decpfirstst;
  trans[ plusst ][ alpha ] = sgalphast;
  trans[ plusst ][ newline ] = sgalphast;
  trans[ plusst ][ tabulator ] = sgalphast;
  trans[ plusst ][ backslash ] = sgalphast;
  trans[ plusst ][ expntl ] = alphast;
  trans[ plusst ][ whitespace ] = aheadsgst;
  trans[ plusst ][ eoln ] = aheadsgst;
  trans[ plusst ][ openbracket ] = aheadsgst;
  trans[ plusst ][ openbrace ] = aheadsgst;
  trans[ plusst ][ closebracket ] = aheadsgst;
  trans[ plusst ][ closebrace ] = aheadsgst;
  trans[ plusst ][ percent ] = aheadsgst;
  trans[ plusst ][ openparenth ] = aheadsgst;
  trans[ plusst ][ slash ] = aheadsgst;
  trans[ plusst ][ eof ] = aheadsgst;


  trans[ startstringst ][ closeparenth ] = closeparst; // empty string
  trans[ startstringst ][ openparenth ] = openparst;
  trans[ startstringst ][ backslash ] = backslashst; // string escape
  trans[ startstringst ][ digit ] = stringst;
  trans[ startstringst ][ null ] = stringst;
  trans[ startstringst ][ expntl ] = stringst;
  trans[ startstringst ][ decpoint ] = stringst;
  trans[ startstringst ][ plus ] = stringst;
  trans[ startstringst ][ minus ] = stringst;
  trans[ startstringst ][ whitespace ] = stringst;
  trans[ startstringst ][ eoln ] = stringst; // eoln is included!
  trans[ startstringst ][ openbracket ] = stringst;
  trans[ startstringst ][ closebracket ] = stringst;
  trans[ startstringst ][ openbrace ] = stringst;
  trans[ startstringst ][ closebrace ] = stringst;
  trans[ startstringst ][ alpha ] = stringst;
  trans[ startstringst ][ newline ] = stringst;
  trans[ startstringst ][ tabulator ] = stringst;
  trans[ startstringst ][ slash ] = stringst;
  trans[ startstringst ][ percent ] = stringst;
  trans[ startstringst ][ asterisk ] = stringst;

  trans[ stringst ][ closeparenth ] = closeparst;
  trans[ stringst ][ openparenth ] = openparst;
  trans[ stringst ][ backslash ] = backslashst; // string escape
  trans[ stringst ][ digit ] = stringst;
  trans[ stringst ][ null ] = stringst;
  trans[ stringst ][ expntl ] = stringst;
  trans[ stringst ][ decpoint ] = stringst;
  trans[ stringst ][ plus ] = stringst;
  trans[ stringst ][ minus ] = stringst;
  trans[ stringst ][ whitespace ] = stringst;
  trans[ stringst ][ eoln ] = stringst;
  trans[ stringst ][ openbracket ] = stringst;
  trans[ stringst ][ closebracket ] = stringst;
  trans[ stringst ][ openbrace ] = stringst;
  trans[ stringst ][ closebrace ] = stringst;
  trans[ stringst ][ alpha ] = stringst;
  trans[ stringst ][ newline ] = stringst;
  trans[ stringst ][ tabulator ] = stringst;
  trans[ stringst ][ slash ] = stringst;
  trans[ stringst ][ percent ] = stringst;
  trans[ stringst ][ asterisk ] = stringst;

  // Escape sequences inside a string
  trans[ backslashst ][ newline ] = newlinest;
  trans[ backslashst ][ tabulator ] = tabulatorst;
  trans[ backslashst ][ backslash ] = backslashcst;
  trans[ backslashst ][ openparenth ] = oparenthcst;
  trans[ backslashst ][ closeparenth ] = cparenthcst;

  trans[ intdgtst ][ digit ] = intdgtst;
  trans[ intdgtst ][ null ] = intdgtst;
  trans[ intdgtst ][ expntl ] = intexpst;
  trans[ intdgtst ][ decpoint ] = decpointst;
  trans[ intdgtst ][ whitespace ] = aheadintst;
  trans[ intdgtst ][ openbracket ] = aheadintst;
  trans[ intdgtst ][ openbrace ] = aheadintst;
  trans[ intdgtst ][ closebrace ] = aheadintst;
  trans[ intdgtst ][ closebracket ] = aheadintst;
  trans[ intdgtst ][ percent ] = aheadintst;
  trans[ intdgtst ][ slash ] = aheadintst;
  // this is a bit questionable, but still unique
  trans[ intdgtst ][ alpha ] = aheadintst;
  trans[ intdgtst ][ newline ] = aheadintst;
  trans[ intdgtst ][ tabulator ] = aheadintst;
  trans[ intdgtst ][ backslash ] = aheadintst;
  trans[ intdgtst ][ openparenth ] = aheadintst;
  trans[ intdgtst ][ eoln ] = aheadintst;
  trans[ intdgtst ][ eof ] = aheadintst;

  trans[ nullst ][ decpoint ] = decpointst;
  trans[ nullst ][ expntl ] = expntlst;
  trans[ nullst ][ whitespace ] = aheadintst;
  trans[ nullst ][ openbracket ] = aheadintst;
  trans[ nullst ][ openbrace ] = aheadintst;
  trans[ nullst ][ closebrace ] = aheadintst;
  trans[ nullst ][ closebracket ] = aheadintst;
  trans[ nullst ][ percent ] = aheadintst;
  trans[ nullst ][ slash ] = aheadintst;
  trans[ nullst ][ openparenth ] = aheadintst;
  // this is a bit questionable, but still unique
  trans[ nullst ][ alpha ] = aheadintst;
  trans[ nullst ][ tabulator ] = aheadintst;
  trans[ nullst ][ newline ] = aheadintst;
  trans[ nullst ][ backslash ] = aheadintst;
  trans[ nullst ][ eoln ] = aheadintst;
  trans[ nullst ][ eof ] = aheadintst;


  trans[ decpfirstst ][ digit ] = decpdgtst;
  trans[ decpfirstst ][ alpha ] = dotalphast;
  trans[ decpfirstst ][ asterisk ] = dotalphast;
  trans[ decpfirstst ][ null ] = decpdgtst;

  trans[ decpointst ][ digit ] = fracdgtst;
  trans[ decpointst ][ null ] = fracdgtst;
  trans[ decpointst ][ expntl ] = expntlst;
  trans[ decpointst ][ whitespace ] = aheadfracst;
  trans[ decpointst ][ eoln ] = aheadfracst;
  trans[ decpointst ][ openbracket ] = aheadfracst;
  trans[ decpointst ][ openbrace ] = aheadfracst;
  trans[ decpointst ][ closebracket ] = aheadfracst;
  trans[ decpointst ][ closebrace ] = aheadfracst;
  trans[ decpointst ][ percent ] = aheadfracst;
  trans[ decpointst ][ slash ] = aheadfracst;
  trans[ decpointst ][ openparenth ] = aheadfracst;
  // this is a bit questionable, but still unique
  trans[ decpointst ][ alpha ] = aheadfracst;
  trans[ decpointst ][ tabulator ] = aheadfracst;
  trans[ decpointst ][ newline ] = aheadfracst;
  trans[ decpointst ][ backslash ] = aheadfracst;
  trans[ decpointst ][ eof ] = aheadfracst;


  trans[ fracdgtst ][ digit ] = fracdgtst;
  trans[ fracdgtst ][ null ] = fracdgtst;
  trans[ fracdgtst ][ expntl ] = expntlst;
  trans[ fracdgtst ][ whitespace ] = aheadfracst;
  trans[ fracdgtst ][ eoln ] = aheadfracst;
  trans[ fracdgtst ][ openbracket ] = aheadfracst;
  trans[ fracdgtst ][ openbrace ] = aheadfracst;
  trans[ fracdgtst ][ closebracket ] = aheadfracst;
  trans[ fracdgtst ][ closebrace ] = aheadfracst;
  trans[ fracdgtst ][ percent ] = aheadfracst;
  trans[ fracdgtst ][ slash ] = aheadfracst;
  trans[ fracdgtst ][ openparenth ] = aheadfracst;
  // this is a bit questionable, but still unique
  trans[ fracdgtst ][ alpha ] = aheadfracst;
  trans[ fracdgtst ][ tabulator ] = aheadfracst;
  trans[ fracdgtst ][ newline ] = aheadfracst;
  trans[ fracdgtst ][ backslash ] = aheadfracst;
  trans[ fracdgtst ][ eof ] = aheadfracst;

  trans[ expntlst ][ digit ] = expdigst;
  trans[ expntlst ][ null ] = expdigst;
  trans[ expntlst ][ plus ] = plexpst;
  trans[ expntlst ][ minus ] = mnexpst;

  trans[ plexpst ][ digit ] = expdigst;
  trans[ plexpst ][ null ] = expdigst;

  trans[ mnexpst ][ digit ] = expdigst;
  trans[ mnexpst ][ null ] = expdigst;

  trans[ expdigst ][ digit ] = expdigst;
  trans[ expdigst ][ null ] = expdigst;
  trans[ expdigst ][ whitespace ] = aheadfracst;
  trans[ expdigst ][ eoln ] = aheadfracst;
  trans[ expdigst ][ openbracket ] = aheadfracst;
  trans[ expdigst ][ openbrace ] = aheadfracst;
  trans[ expdigst ][ closebracket ] = aheadfracst;
  trans[ expdigst ][ closebrace ] = aheadfracst;
  trans[ expdigst ][ percent ] = aheadfracst;
  trans[ expdigst ][ slash ] = aheadfracst;
  trans[ expdigst ][ openparenth ] = aheadfracst;
  // this is a bit questionable, but still unique
  trans[ expdigst ][ alpha ] = aheadfracst;
  trans[ expdigst ][ newline ] = aheadfracst;
  trans[ expdigst ][ tabulator ] = aheadfracst;
  trans[ expdigst ][ backslash ] = aheadfracst;
  trans[ expdigst ][ eof ] = aheadfracst;


  trans[ alphast ][ whitespace ] = aheadalphst;
  trans[ alphast ][ eoln ] = aheadalphst;
  trans[ alphast ][ alpha ] = alphast;
  trans[ alphast ][ asterisk ] = alphast;
  trans[ alphast ][ newline ] = alphast;
  trans[ alphast ][ tabulator ] = alphast;
  trans[ alphast ][ backslash ] = alphast;
  trans[ alphast ][ expntl ] = alphast;
  trans[ alphast ][ digit ] = alphast;
  trans[ alphast ][ null ] = alphast;
  trans[ alphast ][ plus ] = alphast;
  trans[ alphast ][ minus ] = alphast;
  trans[ alphast ][ decpoint ] = alphast;
  trans[ alphast ][ openbracket ] = aheadalphst;
  trans[ alphast ][ openbrace ] = aheadalphst;
  trans[ alphast ][ closebracket ] = aheadalphst;
  trans[ alphast ][ closebrace ] = aheadalphst;
  trans[ alphast ][ percent ] = aheadalphst;
  trans[ alphast ][ openparenth ] = aheadalphst;
  trans[ alphast ][ slash ] = aheadalphst;
  trans[ alphast ][ eof ] = aheadalphst;

  // PostScript comments are like white space

  trans[ percentst ][ eoln ] = start;
  trans[ percentst ][ backslash ] = percentst;
  trans[ percentst ][ whitespace ] = percentst;
  trans[ percentst ][ openparenth ] = percentst;
  trans[ percentst ][ closeparenth ] = percentst;
  trans[ percentst ][ digit ] = percentst;
  trans[ percentst ][ null ] = percentst;
  trans[ percentst ][ decpoint ] = percentst;
  trans[ percentst ][ plus ] = percentst;
  trans[ percentst ][ minus ] = percentst;
  trans[ percentst ][ openbracket ] = percentst;
  trans[ percentst ][ closebracket ] = percentst;
  trans[ percentst ][ openbrace ] = percentst;
  trans[ percentst ][ closebrace ] = percentst;
  trans[ percentst ][ alpha ] = percentst;
  trans[ percentst ][ newline ] = percentst;
  trans[ percentst ][ tabulator ] = percentst;
  trans[ percentst ][ expntl ] = percentst;
  trans[ percentst ][ slash ] = percentst;
  trans[ percentst ][ percent ] = percentst;
  trans[ percentst ][ asterisk ] = percentst;
  trans[ percentst ][ eof ] = eofst;

  // ccommentst treats c like comments.

  trans[ slashst ][ asterisk ] = ccommentst;
  trans[ slashst ][ backslash ] = literalst;
  trans[ slashst ][ alpha ] = literalst;
  trans[ slashst ][ newline ] = literalst;
  trans[ slashst ][ tabulator ] = literalst;
  trans[ slashst ][ minus ] = literalst;
  trans[ slashst ][ plus ] = literalst;
  trans[ slashst ][ expntl ] = literalst;
  trans[ slashst ][ digit ] = literalst;
  trans[ slashst ][ decpoint ] = literalst;
  trans[ slashst ][ null ] = literalst;

  trans[ literalst ][ whitespace ] = aheadlitst;
  trans[ literalst ][ eoln ] = aheadlitst;
  trans[ literalst ][ alpha ] = literalst;
  trans[ literalst ][ asterisk ] = literalst;
  trans[ literalst ][ newline ] = literalst;
  trans[ literalst ][ tabulator ] = literalst;
  trans[ literalst ][ backslash ] = literalst;
  trans[ literalst ][ expntl ] = literalst;
  trans[ literalst ][ digit ] = literalst;
  trans[ literalst ][ null ] = literalst;
  trans[ literalst ][ plus ] = literalst;
  trans[ literalst ][ minus ] = literalst;
  trans[ literalst ][ decpoint ] = literalst;
  trans[ literalst ][ openbracket ] = aheadlitst;
  trans[ literalst ][ closebracket ] = aheadlitst;
  trans[ literalst ][ openbrace ] = aheadlitst;
  trans[ literalst ][ closebrace ] = aheadlitst;
  trans[ literalst ][ openparenth ] = aheadlitst;
  trans[ literalst ][ percent ] = aheadlitst;
  trans[ literalst ][ slash ] = aheadlitst;
  trans[ literalst ][ eof ] = aheadlitst;


  trans[ ccommentst ][ eoln ] = ccommentst;
  trans[ ccommentst ][ whitespace ] = ccommentst;
  trans[ ccommentst ][ openparenth ] = ccommentst;
  trans[ ccommentst ][ closeparenth ] = ccommentst;
  trans[ ccommentst ][ backslash ] = ccommentst;
  trans[ ccommentst ][ digit ] = ccommentst;
  trans[ ccommentst ][ null ] = ccommentst;
  trans[ ccommentst ][ decpoint ] = ccommentst;
  trans[ ccommentst ][ plus ] = ccommentst;
  trans[ ccommentst ][ minus ] = ccommentst;
  trans[ ccommentst ][ percent ] = ccommentst;
  trans[ ccommentst ][ openbracket ] = ccommentst;
  trans[ ccommentst ][ closebracket ] = ccommentst;
  trans[ ccommentst ][ openbrace ] = ccommentst;
  trans[ ccommentst ][ closebrace ] = ccommentst;
  trans[ ccommentst ][ alpha ] = ccommentst;
  trans[ ccommentst ][ newline ] = ccommentst;
  trans[ ccommentst ][ tabulator ] = ccommentst;
  trans[ ccommentst ][ expntl ] = ccommentst;
  trans[ ccommentst ][ slash ] = ccommentst;
  trans[ ccommentst ][ asterisk ] = asteriskst;

  trans[ asteriskst ][ slash ] = start;
  trans[ asteriskst ][ eoln ] = ccommentst;
  trans[ asteriskst ][ backslash ] = ccommentst;
  trans[ asteriskst ][ whitespace ] = ccommentst;
  trans[ asteriskst ][ openparenth ] = ccommentst;
  trans[ asteriskst ][ closeparenth ] = ccommentst;
  trans[ asteriskst ][ digit ] = ccommentst;
  trans[ asteriskst ][ null ] = ccommentst;
  trans[ asteriskst ][ decpoint ] = ccommentst;
  trans[ asteriskst ][ plus ] = ccommentst;
  trans[ asteriskst ][ minus ] = ccommentst;
  trans[ asteriskst ][ openbracket ] = ccommentst;
  trans[ asteriskst ][ closebracket ] = ccommentst;
  trans[ asteriskst ][ openbrace ] = ccommentst;
  trans[ asteriskst ][ closebrace ] = ccommentst;
  trans[ asteriskst ][ alpha ] = ccommentst;
  trans[ asteriskst ][ newline ] = ccommentst;
  trans[ asteriskst ][ tabulator ] = ccommentst;
  trans[ asteriskst ][ expntl ] = ccommentst;
  trans[ asteriskst ][ percent ] = ccommentst;
  // changed from ccommentst, 25.8.1995
  trans[ asteriskst ][ asterisk ] = asteriskst;
}

void
Scanner::source( std::istream* in_s )
{
  if ( in != in_s )
  {
    in = in_s;
    line = 0;
    col = 0;
    old_context.clear();
    context.clear();
    context.reserve( 255 );
  }
}

bool Scanner::operator()( Token& t )
{
  static const int base = 10;
  ScanStates state = start;
  std::string s;
  s.reserve( 255 );
  std::string ds;
  context.reserve( 255 );

  unsigned char c = '\0';
  unsigned char sgc = '\0';

  long lng = 0L;
  double d = 0.0;
  int sg = 1;
  int e = 0;
  int parenth = 0; // to handle PS parenthesis in strings
  double p = 1.;


  t.clear();

  do
  {

    if ( not in->eof() && not in->good() )
    {
      std::cout << "I/O Error in scanner input stream." << std::endl;
      state = error;
      break;
    }

    // according to Stroustrup 21.3.4, std::istream::get(char&),
    // so we cannot use unsigned char c as argument.  The
    // get() is not picky.  --- HEP 2001-08-09
    //     in->get(c);
    c = in->get();
    if ( col++ == 0 )
    {
      ++line;
    }

    if ( c == '\0' || in->bad() )
    {
      c = endof;
    }

    if ( in->eof() )
    {
      c = endof;
    }
    else
    {
      assert( in->good() );
    }
    if ( c != endof )
    {
      context += c;
    }

    if ( c == endoln )
    {
      col = 0;
      old_context.clear();
      old_context.swap( context );
      context.reserve( 256 );
    }

    state = trans[ state ][ code( c ) ];

    switch ( state )
    {
    case intdgtst:
      lng = sg * ( std::labs( lng ) * base + digval( c ) );
      ds.push_back( c );
      break;

    case aheadintst:
    {
      IntegerDatum id( lng );
      t = id;
      if ( c != endoln && c != endof )
      {
        in->unget();
        --col;
      }

      ds.clear();
      state = end;
    }
    break;


    case expntlst:
      ds.push_back( 'e' );
      break;

    case intexpst:
      d = ( double ) lng;
      ds.push_back( 'e' );
      state = expntlst;
      break;

    case decpointst:
      d = ( double ) lng;
      ds.push_back( '.' );
      break;

    case decpdgtst:
      /* This state is entered when a number starts with a decimal point.
         in this case the next character must be null or digit, everything
         else is an invalid transition. This is why decpdgtst and fracdgtst
         are separate states. */
      ds.push_back( '.' );
      state = fracdgtst;
    case fracdgtst:
      p /= base;
      d += sg * p * digval( c );
      ds.push_back( c );
      break;

    case aheadfracst:
    {
      // cast base to double to help aCC with overloading
      // the first arg to pow is always a double, Stroustrup 22.3
      // HEP 2001-08--09

      // traditional
      // Token doubletoken(new DoubleDatum(d * std::pow((double)base,es*e)));

      Token doubletoken( new DoubleDatum( std::atof( ds.c_str() ) ) );
      ds.clear();

      t.move( doubletoken );
      if ( c != endoln && c != endof )
      {
        in->unget();
        --col;
      }
      state = end;
    }
    break;

    case minusst:
      sg = -1;
      ds.push_back( '-' );
    case plusst:
      sgc = c;
      break;

    case mnexpst:
      ds.push_back( '-' );
      break;

    case expdigst:
      e = e * base + digval( c );
      ds.push_back( c );
      break;

    case openparst:
      parenth++;
      s.append( 1, c );
      state = stringst;
      break;
    case closeparst:      // this is not meant for a DEA!
      if ( parenth )      // if parenth>0 we are still
      {                   //  inside the string
        s.append( 1, c ); // the last ) is not included.
        parenth--;
        state = stringst;
      }
      else
      {
        Token temptoken( new StringDatum( s ) );
        t.move( temptoken );
        state = end;
      }
      break;
    case dotalphast:
      s.append( 1, '.' );
      s.append( 1, c );
      state = alphast;
      break;
    case sgalphast:
      assert( sgc == '+' || sgc == '-' );
      s.append( 1, sgc );
      state = alphast;
    case literalst:
    case stringst:
    case alphast:       // let's optimize this at some point
      s.append( 1, c ); // string of fixed length sl
      break;            // append to s every sl characters
    case newlinest:
      s.append( "\n" );
      state = stringst;
      break;
    case tabulatorst:
      s.append( "\t" );
      state = stringst;
      break;
    case backslashcst:
      s.append( "\\" );
      state = stringst;
      break;
    case oparenthcst:
      s.append( "(" );
      state = stringst;
      break;
    case cparenthcst:
      s.append( ")" );
      state = stringst;
      break;
    case aheadsgst:
      s.append( 1, sgc );
    case aheadalphst:
    {
      if ( c != endoln && c != endof )
      {
        in->unget();
        --col;
      }
      NameDatum nd( s );
      t = nd;
    }
      state = end;
      break;

    case aheadlitst:
    {
      if ( c != endoln && c != endof )
      {
        in->unget();
        --col;
      }
      LiteralDatum nd( s );
      t = nd;
      state = end;
    }
    break;

    case openbracest:
    {
      t = BeginProcedureSymbol;
      state = end;
    }
    break;

    case openbracketst:
    {
      t = BeginArraySymbol;
      state = end;
    }
    break;

    case closebracest:
    {
      t = EndProcedureSymbol;
      state = end;
    }
    break;

    case closebracketst:
    {
      t = EndArraySymbol;
      state = end;
    }
    break;

    case eofst:
    {
      t = EndSymbol;
      state = end;
    }
    break;

    case error:
      print_error( "" );
      break;
    default:
      break;
    }
  } while ( ( state != error ) && ( state != end ) );
  return ( state == end );
}

void
Scanner::print_error( const char* msg )
{
  std::cout << "% parser: At line " << line << " position " << col << ".\n"
            << "% parser: Syntax Error: " << msg << "\n";
  std::cout << "% parser: Context preceding the error follows:\n" << old_context << std::endl
            << context << std::endl;
}
