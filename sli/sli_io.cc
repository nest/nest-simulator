/*
 *  sli_io.cc
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
   SLI's i/o functions
*/

#include "sli_io.h"

// C++ includes:
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <typeinfo>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictstack.h"
#include "doubledatum.h"
#include "fdstream.h"
#include "integerdatum.h"
#include "iostreamdatum.h"
#include "namedatum.h"
#include "stringdatum.h"
#include "tokenutils.h"

// sstream has functions std::?stringstream
// strstream has functions std::?strstream
// HEP 2002-10-06
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif


using namespace std;

// The i/o facilities defined in this file shall map the C++ stream i/o
// facilities to SLI. The PS compatible i/o operations shall be implemented
// in terms of the c++ compatible operators.

// Note: As of GNU libstdc++ 2.7.2, most of the ANSI/ISO manipulators
// are not yet supportet, thus the flags are set and unset explicitely.

// String stream operations are not yet (as of Nov 7 1997) defined
// due to the lack of sstream support in libstdc++-2.7.2.x
// This will change in the future.

extern int SLIsignalflag;


void
MathLinkPutStringFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  // call: string  ->

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL )
  {
    StringDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  std::cout << "sending (" << *sd << ") to Mathematica" << std::endl;

  i->EStack.pop();
  i->OStack.pop();
}


/** @BeginDocumentation
  Name: xifstream - Create an executable input-stream.
  Synopsis: (filename) xifstream -> file true
  -> false
  Description: xifstream first tries to open a file
  by the given name. If this was successful, it creates
  an executable stream-handle.
  If an executable stream is executed (e.g. with exec),
  the interpreter parses this file according to SLI syntax
  and evaluates all contained objects.
  SeeAlso: run
*/
void
XIfstreamFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  // call: string -> ifstreamhandle true
  //              -> false
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL )
  {
    StringDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  std::istream* in = new ifdstream( sd->c_str() );
  i->OStack.pop();
  if ( in->good() )
  {
    Token handle_token( new XIstreamDatum( in ) );

    i->OStack.push_move( handle_token );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

void
IfstreamFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ifstream - Open file for reading.
     Synopsis: string ifstream -> ifstreamhandle true
     -> false
     Description:
     Tries to open the named file for reading.
     If successful an ifstream handle object
     and the boolean true is returned. In case of failure only the boolean
     false is returned. This function provides a direct interface to the
     C++ ifstream constructor. SLI's search path mechanism is not used.
     Remarks: commented 26.3.1999, Diesmann
     SeeAlso: xifstream, ofstream
  */
  i->assert_stack_load( 1 );

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL )
  {
    StringDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  //    std::ifstream *in = new std::ifstream(sd->c_str());
  std::istream* in = new ifdstream( sd->c_str() );
  i->OStack.pop();
  if ( in->good() )
  {
    Token handle_token( new IstreamDatum( in ) );

    i->OStack.push_move( handle_token );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

/** @BeginDocumentation
   Name: ofstream - Open a file stream for writing.
   Synopsis: string ofstream -> ofstreamhandle true
   -> false
   Description:
   Open the file by the supplied name for writing.
   If successful an ofstream handle object
   and the boolean true is returned. In case of failure, only the boolean
   false is returned. This function provides a direct interface to the
   C++ ofstream constructor. SLI's search path mechanism is not used.
   Remarks: commented 26.3.1999, Diesmann
   SeeAlso: ofsopen
*/

void
OfstreamFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  // call: string -> ofstream true
  //              -> false
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL )
  {
    StringDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  //    std::ofstream *out = new std::ofstream(sd->c_str());
  std::ostream* out = new ofdstream( sd->c_str() );
  i->OStack.pop();
  if ( out->good() )
  {
    Token handle_token( new OstreamDatum( out ) );

    i->OStack.push_move( handle_token );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}


// This is the ofstream constructor
// with open mode arguments.
// In the sli we do not implement the ios_base flags
// and rather use the c-mode notation

/** @BeginDocumentation
   Name: ofsopen - Open an existing file for appending or writing.
   Synopsis: (name) (mode) ofsopen -> ofstreamhandle true
   -> false
   Parameters: (name) - name of the file.
   (mode) - string with either (w) or (a) to identify
   the access mode. (w) corresponds to writing
   and (a) to appending.
   Description:
   Open the named file according to the access mode. If the file
   is not existing, it will be created in the current working directory.
   If the file does exists, the access mode decides whether the file will
   be overwritten (w) or whether the new data will be appended (a).
   If successful an ofstream handle object and the boolean true is returned.
   In case of failure, only the boolean
   false is returned. This function provides a direct interface to the
   C++ ofstream constructor. SLI's search path mechanism is not used.
   Remarks: commented 26.3.1999, Diesmann
   SeeAlso: ofstream
*/
void
OfsopenFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );
  // call: string string -> ofstream true
  //                     -> false

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );

  StringDatum* md = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL || md == NULL )
  {
    StringDatum const d;
    Token t1 = i->OStack.pick( 1 );
    Token t2 = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(),
      t1.datum()->gettypename().toString() + " or " + t2.datum()->gettypename().toString() );
  }

  std::ostream* out = NULL;

  if ( static_cast< string >( *md ) == "w" )
  {
    out = new ofdstream( sd->c_str(), ios::out );
  }
  else if ( static_cast< string >( *md ) == "a" )
  {
    out = new ofdstream( sd->c_str(), ios::out | ios::app );
  }
  else
  {
    i->raiseerror( Name( "UnknownFileOpenMode" ) );
    return;
  }

  if ( out != NULL )
  {
    i->OStack.pop( 2 );
    if ( out->good() )
    {
      Token handle_token( new OstreamDatum( out ) );

      i->OStack.push_move( handle_token );
      i->OStack.push( true );
    }
    else
    {
      i->OStack.push( false );
    }
  }

  i->EStack.pop();
}

#ifdef HAVE_SSTREAM


void
IsstreamFunction::execute( SLIInterpreter* i ) const
{
  // This operator does not work yet, due to the fact that
  // the string, used to initialize the stringstream is not copied
  // but is used as the actual buffer. Thus, in our
  // context this leads to a loss of the buffer, as
  // the string object is destroyed at the end of the function!!

  // call: string isstream -> isstream true
  //                       -> false

  i->assert_stack_load( 1 );

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  if ( sd == NULL )
  {
    StringDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

#ifdef HAVE_SSTREAM
  std::istringstream* in = new std::istringstream( sd->c_str() );
#else
  std::istrstream* in = new std::istrstream( sd->c_str() );
#endif
  i->OStack.pop();
  if ( in->good() )
  {
    i->OStack.push( new IstreamDatum( in ) );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

// old style output string stream
/** @BeginDocumentation
  Name: osstream - Create a string-stream object.
  Synopsis:
  osstream -> osstream-handle true
  -> false
  Description:
  osstream creates a stream object which can later be turned into a
  string. If creation of a usable string-stream object failed for any
  reason, false is returned. Replaces obsolete ostrstream.
  Remarks: commented 6.11.2003, Diesmann
  SeeAlso: str, ofstream, ofsopen
*/
void
OsstreamFunction::execute( SLIInterpreter* i ) const
{
// call: osstream -> osstream true
//                -> false

#ifdef HAVE_SSTREAM
  std::ostringstream* out = new std::ostringstream();
#else
  std::ostrtream* out = new std::ostrstream();
#endif

  if ( out->good() )
  {
    i->OStack.push( new OstreamDatum( out ) );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}


void
StrSStreamFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  // call: ostrstream str -> string

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == NULL )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

#ifdef HAVE_SSTREAM
  std::ostringstream* out = dynamic_cast< std::ostringstream* >( ostreamdatum->get() );
#else
  std::ostrstream* out = dynamic_cast< std::ostrstream* >( ostreamdatum->get() );
#endif
  assert( out != NULL );
  ostreamdatum->unlock();

  if ( out != NULL )
  {
    if ( out->good() )
    {
      // *out << ends;  // this causes a 0 as the last character of the string
      // on the alpha compiler. It is probably a bug to
      // use this command with <sstream>. 25.2.02 Diesmann

      string s = out->str();           // following Stroustrup
      Token t( new StringDatum( s ) ); //


      i->OStack.pop();
      i->OStack.push_move( t );
      i->EStack.pop();
    }
    else
    {
      i->raiseerror( i->BadIOError ); // new style more throw like
    }
  }
  else
  {
    i->raiseerror( i->StringStreamExpectedError );
  }
}

#else

// old style output string stream
/** @BeginDocumentation
  Name: ostrstream - Create a string-stream object.
  Synopsis:
  ostrstream -> ostreamhandle true
  -> false
  Description:
  Obsolete, use osstream instead.
  SeeAlso: str, osstream
*/
void
OstrstreamFunction::execute( SLIInterpreter* i ) const
{
// call: ostrstream -> ostrstream true
//                  -> false

#ifdef HAVE_SSTREAM
  std::ostringstream* out = new std::ostringstream();
#else
  std::ostrstream* out = new std::ostrstream();
#endif
  assert( out != NULL );

  if ( out->good() )
  {
    i->OStack.push( new OstreamDatum( out ) );
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

/** @BeginDocumentation
  Name: str - Retrieve a string from a string-stream.
  Synopsis: osstream str -> string
  Description: Retrieves the string data from a string-stream object
  SeeAlso: osstream
*/
void
StrFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  // call: std::ostrstream str -> string

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == NULL )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

#ifdef HAVE_SSTREAM
  std::ostringstream* out = dynamic_cast< std::ostringstream* >( ostreamdatum->get() );
#else
  std::ostrstream* out = dynamic_cast< std::ostrstream* >( ostreamdatum->get() );
#endif
  assert( out != NULL );
  ostreamdatum->unlock();

  if ( out != NULL )
  {
    if ( out->good() )
    {
      *out << ends;                    // following Josuttis, Nicolai 1996
      char* s = out->str();            // sec. 13.11.2 page 493
      Token t( new StringDatum( s ) ); //
      delete[] s;                      //

      i->OStack.pop();
      i->OStack.push_move( t );
      i->EStack.pop();
    }
    else
    {
      i->raiseerror( i->BadIOError ); // new style more throw like
    }
  }
  else
  {
    i->raiseerror( i->StringStreamExpectedError );
  }
}

#endif


/** @BeginDocumentation
  Name: print - Print object to a stream

  Synopsis: ostream any <- -> ostream

  Description: Alternatives: You can use <- (undocumented),
  which is the same as print.

  Examples:
  cerr [1 2 3] print endl ; -> <arraytype>

  Author: docu by Marc Oliver Gewaltig and Sirko Straube

  SeeAlso: pprint, =, ==
*/
void
PrintFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  // call: ostream obj -> ostream

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ostreamdatum == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  assert( ostreamdatum->valid() );

  if ( ( *ostreamdatum )->good() )
  {
    i->OStack.top()->print( **ostreamdatum );
    if ( SLIsignalflag != 0 )
    {
      ( *ostreamdatum )->clear();
    }

    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

/** @BeginDocumentation
  Name: pprint - pretty print: Print object to a stream

  Synopsis: ostream any <- -> ostream

  Description: Alternatives: You can use <-- (undocumented),
  which is the same as pprint.

  Examples:
  cerr [1 2 3] pprint endl ; -> [1 2 3]

  Author: docu by Marc Oliver Gewaltig and Sirko Straube

  SeeAlso: print, =, ==
*/

void
PrettyprintFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream obj -> ostream

  i->assert_stack_load( 2 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.pick( 1 );
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    i->OStack.top()->pprint( **ostreamdatum );
    if ( SLIsignalflag != 0 )
    {
      ( *ostreamdatum )->clear();
    }
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

/** @BeginDocumentation
  Name: flush - Force the buffer of a stream to be flushed.
  Synopsis: ostream flush -> ostream
*/
void
FlushFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  // call: ostream -> ostream

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->flush();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

/** @BeginDocumentation
  Name: endl - line break

  Examples: Watch the difference:
  cerr (hello) print
  cerr (hello) print endl

  Author: docu by Sirko Straube

  SeeAlso: print, pprint, cout, cerr
*/

void
EndlFunction::execute( SLIInterpreter* i ) const
{

  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << std::endl;
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
EndsFunction::execute( SLIInterpreter* i ) const
{

  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.pick( 1 );
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << ends;
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
EatwhiteFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == NULL || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *istreamdatum )->good() )
  {
    if ( not( *istreamdatum )->eof() )
    {
      ( **istreamdatum ) >> ws;
    }
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}


void
CloseistreamFunction::execute( SLIInterpreter* i ) const
{
  // call: istream -> -

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == NULL || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( istreamdatum->get() != &std::cin )
  {
    istreamdatum->unlock();

    // The following dynamic cast yields a seg-fault if
    // the datum conatains &std::cin !!
    ifdstream* ifs = dynamic_cast< ifdstream* >( istreamdatum->get() );
    istreamdatum->unlock();
    if ( ifs != NULL )
    {
      ifs->close();
      // iostreamhandle->destroy();
      i->OStack.pop();
      i->EStack.pop();
    }
    else
    {
      i->raiseerror( i->ArgumentTypeError );
    }
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}


void
CloseostreamFunction::execute( SLIInterpreter* i ) const
{
  // This function causes a segmentation fault on linux systems.
  // There the problem is that you cannot close an ofstream over
  // its base-class ostream pointer.
  // This applies to g++ 2.7.2.x

  // call: ostream -> -

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == NULL )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ostreamdatum->get() != &std::cout )
  {
    ostreamdatum->unlock();
    // The following dynamic cast yields a seg-fault if
    // the datum conatains &std::cout !!
    ofdstream* ofs = dynamic_cast< ofdstream* >( ostreamdatum->get() );
    ostreamdatum->unlock();

    if ( ofs != NULL )
    {
      ofs->close();
      i->OStack.pop();
      i->EStack.pop();
    }
    else
    {
      i->raiseerror( i->ArgumentTypeError );
    }
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
SetwFunction::execute( SLIInterpreter* i ) const
{
  // call: ostreamhandle num -> ostreamhandle

  i->assert_stack_load( 2 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.pick( 1 );
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );

  if ( id == NULL )
  {
    IntegerDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << setw( id->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

/** @BeginDocumentation
   Name: setprecision - set precision for decimal place of a stream

   Synopsis: ostream int setprecision -> ostream

   Examples:
   cerr 3 setprecision 3.12345678901 pprint endl -> 3.123
   cerr 9 setprecision 3.12345678901 pprint endl -> 3.123456789

   Author: docu by Sirko Straube

   SeeAlso: cerr, endl, pprint
*/

void
SetprecisionFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream num -> ostream

  i->assert_stack_load( 2 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.pick( 1 );
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );

  if ( id == NULL )
  {
    IntegerDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << std::setprecision( id->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSFixedFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.pick( 1 );
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->setf( ios::fixed );
    ( *ostreamdatum )->unsetf( ios::scientific );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSScientificFunction::execute( SLIInterpreter* i ) const
{
  // call: ostreamhandle -> ostreamhandle

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::fixed );
    ( *ostreamdatum )->setf( ios::scientific );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSDefaultFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::fixed );
    ( *ostreamdatum )->unsetf( ios::scientific );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSShowpointFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->setf( ios::showpoint );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSNoshowpointFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::showpoint );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}


void
IOSOctFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << oct;
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSHexFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << hex;
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSDecFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    **ostreamdatum << dec;
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSShowbaseFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->setf( ios::showbase );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSNoshowbaseFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::showbase );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSLeftFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->setf( ios::left );
    ( *ostreamdatum )->unsetf( ios::right );
    ( *ostreamdatum )->unsetf( ios::internal );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSRightFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::left );
    ( *ostreamdatum )->setf( ios::right );
    ( *ostreamdatum )->unsetf( ios::internal );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
IOSInternalFunction::execute( SLIInterpreter* i ) const
{
  // call: ostream -> ostream

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );

  if ( ostreamdatum == NULL || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    ( *ostreamdatum )->unsetf( ios::left );
    ( *ostreamdatum )->unsetf( ios::right );
    ( *ostreamdatum )->setf( ios::internal );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

/** @BeginDocumentation
   Name: getc - Read single character from input stream.
   Synopsis: istream getc -> istream integer
   Description: getc reads a single character from the
   supplied input stream. The character is
   returned as integer. The mapping between
   characters and its numerical value is
   determined by the C++ compiler.
   Diagnostics: Raises BadIOError
   SeeAlso: gets, getline
*/

void
GetcFunction::execute( SLIInterpreter* i ) const
{
  // call: istream -> istream char

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == NULL || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  char c;
  if ( ( *istreamdatum )->get( c ) )
  {
    Token int_token( new IntegerDatum( c ) );

    i->OStack.push_move( int_token );
    i->EStack.pop();
  }
  else
  {
    if ( SLIsignalflag != 0 )
    {
      // else a SignalError will be raised by the interpreter cycle
      ( *istreamdatum )->clear();
      i->EStack.pop();
    }
    else
    {
      i->raiseerror( i->BadIOError );
    }
  }
}

/** @BeginDocumentation
   Name: gets - Read white space terminated string from stream
   Synopsis: istream gets -> istream string
   Description: gets reads a single string from the istream.
   The stream argument is not removed from the stack
   to support successive application of gets.
   Diagnostics: Raises BadIOError if the read was not successful.
   SeeAlso: getline, getc
*/

void
GetsFunction::execute( SLIInterpreter* i ) const
{
  // call: istream -> istream string

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  //    char buffer[256];
  string s;

  if ( *( *istreamdatum ) >> s )
  {
    Token str_token( new StringDatum( s ) );

    i->OStack.push_move( str_token );
    i->EStack.pop();
  }
  else
  {
    if ( SLIsignalflag == 0 )
    {
      i->raiseerror( i->BadIOError );
    }
    else
    {
      // else a SignalError will be raised by the interpreter cycle
      ( *istreamdatum )->clear();
      i->EStack.pop();
    }
  }
}


void
GetlineFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: getline - Read a newline terminated string from an input stream.
     Synopsis: istreamhandle getline -> istreamhandle string true
     -> istreamhandle false
     Description: getline reads a line from the supplied stream.
     If the read process was successful, a result string and
     the boolean true are returned.
     If an error occured while reading from the stream, only the
     stream and boolean false is returned.
     Diagnostics: No errors are raised. If getline is applied to an
     invalid stream, the return value is false. The return value
     false indicates that the state of the stream is no longer
     "good".

     Author: Diesmann & Gewaltig
     Remarks: commented 26.3.1999, Diesmann
     SeeAlso: getc, gets, file
  */
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *istreamdatum )->good() && not( *istreamdatum )->eof() )
  {
    string s;
    getline( **istreamdatum, s );
    if ( not( *istreamdatum )->good() )
    {
      if ( SLIsignalflag == 0 )
      {
        i->OStack.push( false );
      }
      else
      {
        // else a SignalError will be raised by the interpreter cycle
        ( *istreamdatum )->clear();
        return;
      }
    }
    else
    {
      Token string_token( new StringDatum( s ) );

      i->OStack.push_move( string_token );
      i->OStack.push( true );
    }
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

void
IGoodFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: igood - check the "good"-flag of a stream.
     Synopsis: istreamhandle igood -> istreamhandle true
     -> istreamhandle false
     Description:
     This function provides a direct interface to
     the C++ istream::good() member function.
     Parameters:
     Examples:
     Bugs:
     Author: Diesmann
     FirstVersion: 26.3.1999
     Remarks:
     SeeAlso: ogood, good
  */

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *istreamdatum )->good() )
  {
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

void
IClearFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: iclear - Clear the state-flags of input stream.
     Synopsis: istream iclear -> istream

     Description:
     This function provides a direct interface to
     the C++ istream::clear() member function.

     Bugs:
     Author: Gewaltig

     Remarks:
     SeeAlso: oclear, good
  */

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  ( *istreamdatum )->clear();

  i->EStack.pop();
}

void
OClearFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: oclear - Clear the state-flags of an output stream.
     Synopsis: ostream oclear -> ostream

     Description:
     This function provides a direct interface to
     the C++ ostream::clear() member function.

     Bugs:
     Author: Gewaltig

     Remarks:
     SeeAlso: iclear, good
  */

  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == 0 || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  ( *ostreamdatum )->clear();
  i->EStack.pop();
}

void
IFailFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ifail - Check the "fail"-flag of an input stream.
     Synopsis: istreamhandle ifail -> istreamhandle true
     -> istreamhandle false
     Description:
     This function provides a direct interface to
     the C++ istream::fail() member function.
     If true, the next operation on the stream will fail.
     Parameters:
     Examples:
     Bugs:
     Author: Gewaltig
     FirstVersion: 21.5.1999
     Remarks:
     SeeAlso: ogood, good
  */

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *istreamdatum )->fail() )
  {
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}


void
OGoodFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ogood - Check the "good"-flag of an output stream.
     Synopsis: ostreamhandle ogood -> ostreamhandle true
     -> ostreamhandle false
     Description:
     This function provides a direct interface to
     the C++ ostream::good() member function.
     Parameters:
     Examples:
     Bugs:
     Author: Diesmann
     FirstVersion: 26.3.1999
     Remarks:
     SeeAlso: igood, good
  */
  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == 0 || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->good() )
  {
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}

void
IEofFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ieof - Check the "eof"-flag of an input stream.
     Synopsis: istreamhandle ieof -> istreamhandle true
     -> istreamhandle false
     Description:
     This function provides a direct interface to
     the C++ istream::eof() member function.
     Parameters:
     Examples:
     Bugs:
     Author: Diesmann, Hehl
     FirstVersion: 19.4.1999
     Remarks:
     SeeAlso: oeof, eof
  */
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *istreamdatum )->eof() )
  {
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}


void
OEofFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: oeof - Check the "eof"-flag of an output stream.
     Synopsis: ostreamhandle oeof -> ostreamhandle true
     -> ostreamhandle false
     Description:
     This function provides a direct interface to
     the C++ ostream::eof() member function.
     Parameters:
     Examples:
     Bugs:
     Author: Diesmann, Hehl
     FirstVersion: 19.4.1999
     Remarks:
     SeeAlso: ieof, eof
  */
  i->assert_stack_load( 1 );

  OstreamDatum* ostreamdatum = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );

  if ( ostreamdatum == 0 || not ostreamdatum->valid() )
  {
    OstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( ( *ostreamdatum )->eof() )
  {
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }

  i->EStack.pop();
}


void
Cvx_fFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  IstreamDatum* sd = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );
  if ( sd != NULL )
  {
    Token handle_token( new XIstreamDatum( *sd ) );
    i->OStack.pop();

    i->OStack.push_move( handle_token );
  }

  i->EStack.pop();
}

void
In_AvailFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: in_avail - Return the number of available in an input stream's
     buffer.
     Synopsis: istreamhandle in_avail -> istreamhandle available_characters
     Description:
     This function provides a direct interface to
     the C++ istream::rdbuf()->in_avail() member function.
     Author: R Kupper
     FirstVersion: May 05 1999
     SeeAlso:
  */
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  Token result_t( new IntegerDatum( ( *istreamdatum )->rdbuf()->in_avail() ) );
  i->OStack.push_move( result_t );

  i->EStack.pop();
}

void
ReadDoubleFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ReadDouble - Read a double number from an input stream.
     Synopsis: istream ReadDouble -> istream double true
     -> istream false
     SeeAlso: ReadInt, ReadWord
  */
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( istreamdatum->valid() )
  {
    double d;
    if ( *( *istreamdatum ) >> d )
    {
      Token result_t( new DoubleDatum( d ) );
      i->OStack.push_move( result_t );
      i->OStack.push( true );
      i->EStack.pop();
    }
    else
    {
      if ( SLIsignalflag == 0 )
      {
        i->OStack.push( false );
        i->EStack.pop();
      }
      else
      {
        // else a SignalError will be raised by the interpreter cycle
        ( *istreamdatum )->clear();
      }
    }
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
ReadIntFunction::execute( SLIInterpreter* i ) const
{
  /** @BeginDocumentation
     Name: ReadInt - Read an integer number from an input stream.
     Synopsis: istream ReadInt -> istream int true
     -> istream false
     SeeAlso: ReadDouble, ReadWord
  */
  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  if ( istreamdatum->valid() )
  {
    long val;
    if ( *( *istreamdatum ) >> val )
    {
      Token result_t( new IntegerDatum( val ) );
      i->OStack.push_move( result_t );
      i->OStack.push( true );
      i->EStack.pop();
    }
    else
    {
      if ( SLIsignalflag == 0 )
      {
        i->OStack.push( false );
        i->EStack.pop();
      }
      else
      {
        // else a SignalError will be raised by the interpreter cycle
        ( *istreamdatum )->clear();
      }
    }
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}


/** @BeginDocumentation
   Name: ReadWord - read white space terminated string from stream
   Synopsis: istream ReadWord -> istream string true
   -> istream false
   Description: ReadWord reads a single word from the istream.
   The stream argument is not removed from the stack
   to support successive application of gets.
   SeeAlso: getline, gets, getc, ReadInt, ReadDouble
*/

void
ReadWordFunction::execute( SLIInterpreter* i ) const
{
  // call: istream -> istream string

  i->assert_stack_load( 1 );

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  if ( istreamdatum == 0 || not istreamdatum->valid() )
  {
    IstreamDatum const d;
    Token t = i->OStack.top();
    throw TypeMismatch( d.gettypename().toString(), t.datum()->gettypename().toString() );
  }

  //    char buffer[256];
  string s;

  if ( *( *istreamdatum ) >> s )
  {
    Token str_token( s );

    i->OStack.push_move( str_token );
    i->OStack.push( true );
    i->EStack.pop();
  }
  else
  {
    if ( SLIsignalflag == 0 )
    {
      i->OStack.push( false );
      i->EStack.pop();
    }
    else
    {
      // else a SignalError will be raised by the interpreter cycle
      ( *istreamdatum )->clear();
    }
  }
}


const MathLinkPutStringFunction mathlinkputstringfunction;

const XIfstreamFunction xifstreamfunction;
const IfstreamFunction ifstreamfunction;
const OfstreamFunction ofstreamfunction;
const OfsopenFunction ofsopenfunction;
const Cvx_fFunction cvx_ffunction;

#ifdef HAVE_SSTREAM
const IsstreamFunction isstreamfunction;
const OsstreamFunction osstreamfunction;

const StrSStreamFunction strsstreamfunction;
#else
const OstrstreamFunction ostrstreamfunction;
const StrFunction strfunction;
#endif

const CloseistreamFunction closeistreamfunction;
const CloseostreamFunction closeostreamfunction;
const PrintFunction printfunction;
const PrettyprintFunction prettyprintfunction;

const FlushFunction flushfunction;
const EndlFunction endlfunction;
const EndsFunction endsfunction;
const EatwhiteFunction eatwhitefunction;
const SetwFunction setwfunction;
const SetprecisionFunction setprecisionfunction;
const IOSScientificFunction iosscientificfunction;
const IOSFixedFunction iosfixedfunction;
const IOSDefaultFunction iosdefaultfunction;
const IOSShowpointFunction iosshowpointfunction;
const IOSNoshowpointFunction iosnoshowpointfunction;

const IOSShowbaseFunction iosshowbasefunction;
const IOSNoshowbaseFunction iosnoshowbasefunction;
const IOSDecFunction iosdecfunction;
const IOSHexFunction ioshexfunction;
const IOSOctFunction iosoctfunction;

const IOSLeftFunction iosleftfunction;
const IOSRightFunction iosrightfunction;
const IOSInternalFunction iosinternalfunction;

const GetcFunction getcfunction;
const GetsFunction getsfunction;
const GetlineFunction getlinefunction;

const OClearFunction oclearfunction;
const IClearFunction iclearfunction;
const IFailFunction ifailfunction;
const IGoodFunction igoodfunction;
const OGoodFunction ogoodfunction;

const IEofFunction ieoffunction;
const OEofFunction oeoffunction;

const In_AvailFunction in_availfunction;

const ReadDoubleFunction readdoublefunction;
const ReadIntFunction readintfunction;
const ReadWordFunction readwordfunction;
// const ReadListFunction      readlistfunction;


void
init_sli_io( SLIInterpreter* i )
{
  // the following objects should be placed in the
  // ios dictionary.
  Token t_cin( new IstreamDatum( std::cin ) );
  Token t_cout( new OstreamDatum( std::cout ) );
  Token t_cerr( new OstreamDatum( std::cerr ) );

  /** @BeginDocumentation
    Name: cin - Standard input stream
    Synopsis: cin -> istream
    Description: cin corresponds to the C++ object with the
    same name.
    SeeAlso: cout, cerr
  */

  i->def_move( "cin", t_cin );
  /** @BeginDocumentation
    Name: cout - Standard output stream
    Synopsis: cout -> ostream
    Description: cout corresponds to the C++ object with the
    same name.
    SeeAlso: cin, cerr
  */
  i->def_move( "cout", t_cout );
  /** @BeginDocumentation
    Name: cerr - Standard error output stream
    Synopsis: cerr -> ostream
    Description: cerr corresponds to the C++ object with the
    same name.
    SeeAlso: cin, cout
  */
  i->def_move( "cerr", t_cerr );

  // these objects belong to the system dictionary

  i->createcommand( "MathLinkPutString", &mathlinkputstringfunction );

  i->createcommand( "ifstream", &ifstreamfunction );
  i->createcommand( "xifstream", &xifstreamfunction );
  i->createcommand( "ofstream", &ofstreamfunction );
  i->createcommand( "ofsopen", &ofsopenfunction );
  i->createcommand( "cvx_f", &cvx_ffunction );

#ifdef HAVE_SSTREAM
  i->createcommand( "isstream", &isstreamfunction );
  i->createcommand( "osstream", &osstreamfunction );

  i->createcommand( "ostrstream", &osstreamfunction );
  i->createcommand( "str", &strsstreamfunction );
#else
  i->createcommand( "ostrstream", &ostrstreamfunction );
  i->createcommand( "str", &strfunction );
#endif


  i->createcommand( "closeistream", &closeistreamfunction );
  i->createcommand( "closeostream", &closeostreamfunction );
  i->createcommand( "<-", &printfunction );
  i->createcommand( "<--", &prettyprintfunction );
  i->createcommand( "print", &printfunction );
  i->createcommand( "pprint", &prettyprintfunction );

  i->createcommand( "flush", &flushfunction );
  i->createcommand( "endl", &endlfunction );
  i->createcommand( "ends", &endsfunction );
  i->createcommand( "ws", &eatwhitefunction );
  i->createcommand( "setw", &setwfunction );
  i->createcommand( "setprecision", &setprecisionfunction );
  i->createcommand( "fixed", &iosfixedfunction );
  i->createcommand( "scientific", &iosscientificfunction );
  i->createcommand( "default", &iosdefaultfunction );
  i->createcommand( "showpoint", &iosshowpointfunction );
  i->createcommand( "noshowpoint", &iosnoshowpointfunction );

  i->createcommand( "noshowbase", &iosnoshowbasefunction );
  i->createcommand( "showbase", &iosshowbasefunction );
  i->createcommand( "dec", &iosdecfunction );
  i->createcommand( "hex", &ioshexfunction );
  i->createcommand( "oct", &iosoctfunction );
  i->createcommand( "left", &iosleftfunction );
  i->createcommand( "right", &iosrightfunction );
  i->createcommand( "internal", &iosinternalfunction );
  i->createcommand( "getc", &getcfunction );
  i->createcommand( "gets", &getsfunction );
  i->createcommand( "getline_is", &getlinefunction );
  i->createcommand( "ifail", &ifailfunction );
  i->createcommand( "iclear", &iclearfunction );
  i->createcommand( "oclear", &oclearfunction );
  i->createcommand( "igood", &igoodfunction );
  i->createcommand( "ogood", &ogoodfunction );
  i->createcommand( "ieof", &ieoffunction );
  i->createcommand( "oeof", &oeoffunction );
  i->createcommand( "in_avail", &in_availfunction );
  i->createcommand( "ReadDouble", &readdoublefunction );
  i->createcommand( "ReadInt", &readintfunction );
  i->createcommand( "ReadWord", &readwordfunction );
  //    i->createcommand("ReadList",&readlistfunction);
}
