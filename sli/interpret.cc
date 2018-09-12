/*
 *  interpret.cc
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
    Definitions for the SLI Interpreter class
*/

#include "interpret.h"

// C++ includes:
#include <algorithm>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "numerics.h"

// Includes from sli:
#include "booldatum.h"
#include "dictdatum.h"
#include "dictstack.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "functional.h"
#include "functiondatum.h"
#include "integerdatum.h"
#include "iostreamdatum.h"
#include "namedatum.h"
#include "parser.h"
#include "psignal.h"
#include "scanner.h"
#include "stringdatum.h"
#include "tokenutils.h"
#include "triedatum.h"

// This function is the only interface to the driver program
extern void init_slidict( SLIInterpreter* );
extern void init_slicontrol( SLIInterpreter* );
extern void init_sli_io( SLIInterpreter* );
extern void init_slistack( SLIInterpreter* );
extern void init_slimath( SLIInterpreter* );
extern void init_slitypecheck( SLIInterpreter* );
extern void init_slidata( SLIInterpreter* );
extern void init_slistring( SLIInterpreter* );


const int SLIInterpreter::M_ALL = 0;
const int SLIInterpreter::M_DEBUG = 5;
const int SLIInterpreter::M_STATUS = 7;
const int SLIInterpreter::M_INFO = 10;
const int SLIInterpreter::M_DEPRECATED = 18;
const int SLIInterpreter::M_WARNING = 20;
const int SLIInterpreter::M_ERROR = 30;
const int SLIInterpreter::M_FATAL = 40;
const int SLIInterpreter::M_QUIET = 100;

const char* const SLIInterpreter::M_ALL_NAME = "";
const char* const SLIInterpreter::M_DEBUG_NAME = "Debug";
const char* const SLIInterpreter::M_STATUS_NAME = "Status";
const char* const SLIInterpreter::M_INFO_NAME = "Info";
const char* const SLIInterpreter::M_DEPRECATED_NAME = "Deprecated";
const char* const SLIInterpreter::M_WARNING_NAME = "Warning";
const char* const SLIInterpreter::M_ERROR_NAME = "Error";
const char* const SLIInterpreter::M_FATAL_NAME = "Fatal";
const char* const SLIInterpreter::M_QUIET_NAME = "";


SLIType SLIInterpreter::Integertype;
SLIType SLIInterpreter::Doubletype;
SLIType SLIInterpreter::Stringtype;
SLIType SLIInterpreter::Nametype;
SLIType SLIInterpreter::Booltype;
SLIType SLIInterpreter::Literaltype;
SLIType SLIInterpreter::Arraytype;
SLIType SLIInterpreter::Proceduretype;
SLIType SLIInterpreter::Litproceduretype;
SLIType SLIInterpreter::Dictionarytype;
SLIType SLIInterpreter::Symboltype;
SLIType SLIInterpreter::Functiontype;
SLIType SLIInterpreter::Trietype;
SLIType SLIInterpreter::Callbacktype;
SLIType SLIInterpreter::Istreamtype;
SLIType SLIInterpreter::XIstreamtype;
SLIType SLIInterpreter::Ostreamtype;
SLIType SLIInterpreter::IntVectortype;
SLIType SLIInterpreter::DoubleVectortype;
SLIType SLIInterpreter::Iteratortype;

// SLIType SLIInterpreter::IOstreamtype;

// SLIType default actions

DatatypeFunction SLIInterpreter::datatypefunction;
NametypeFunction SLIInterpreter::nametypefunction;
ProceduretypeFunction SLIInterpreter::proceduretypefunction;
LitproceduretypeFunction SLIInterpreter::litproceduretypefunction;
FunctiontypeFunction SLIInterpreter::functiontypefunction;
TrietypeFunction SLIInterpreter::trietypefunction;
CallbacktypeFunction SLIInterpreter::callbacktypefunction;
XIstreamtypeFunction SLIInterpreter::xistreamtypefunction;

// Basic Operations needed to run the default actions

const IlookupFunction SLIInterpreter::ilookupfunction;
const IsetcallbackFunction SLIInterpreter::isetcallbackfunction;
const IiterateFunction SLIInterpreter::iiteratefunction;
const IloopFunction SLIInterpreter::iloopfunction;
const IrepeatFunction SLIInterpreter::irepeatfunction;
const IforFunction SLIInterpreter::iforfunction;
const IforallarrayFunction SLIInterpreter::iforallarrayfunction;
const IforalliterFunction SLIInterpreter::iforalliterfunction;
const IforallindexedarrayFunction SLIInterpreter::iforallindexedarrayfunction;
const IforallindexedstringFunction SLIInterpreter::iforallindexedstringfunction;
const IforallstringFunction SLIInterpreter::iforallstringfunction;

void
SLIInterpreter::inittypes( void )
{
  Iteratortype.settypename( "iteratortype" );
  Iteratortype.setdefaultaction( datatypefunction );
  Integertype.settypename( "integertype" );
  Integertype.setdefaultaction( datatypefunction );
  Doubletype.settypename( "doubletype" );
  Doubletype.setdefaultaction( datatypefunction );
  Stringtype.settypename( "stringtype" );
  Stringtype.setdefaultaction( datatypefunction );
  Nametype.settypename( "nametype" );
  Nametype.setdefaultaction( nametypefunction );
  Booltype.settypename( "booltype" );
  Booltype.setdefaultaction( datatypefunction );
  Literaltype.settypename( "literaltype" );
  Literaltype.setdefaultaction( datatypefunction );
  Arraytype.settypename( "arraytype" );
  Arraytype.setdefaultaction( datatypefunction );
  Proceduretype.settypename( "proceduretype" );
  Proceduretype.setdefaultaction( proceduretypefunction );
  Litproceduretype.settypename( "literalproceduretype" );
  Litproceduretype.setdefaultaction( litproceduretypefunction );
  Dictionarytype.settypename( "dictionarytype" );
  Dictionarytype.setdefaultaction( datatypefunction );
  Symboltype.settypename( "symboltype" );
  Symboltype.setdefaultaction( datatypefunction );
  Functiontype.settypename( "functiontype" );
  Functiontype.setdefaultaction( functiontypefunction );
  Trietype.settypename( "trietype" );
  Trietype.setdefaultaction( trietypefunction );
  Callbacktype.settypename( "callbacktype" );
  Callbacktype.setdefaultaction( callbacktypefunction );
  Istreamtype.settypename( "istreamtype" );
  Istreamtype.setdefaultaction( datatypefunction );
  XIstreamtype.settypename( "xistreamtype" );
  XIstreamtype.setdefaultaction( xistreamtypefunction );
  Ostreamtype.settypename( "ostreamtype" );
  Ostreamtype.setdefaultaction( datatypefunction );
  IntVectortype.settypename( "intvectortype" );
  IntVectortype.setdefaultaction( datatypefunction );
  DoubleVectortype.settypename( "doublevectortype" );
  DoubleVectortype.setdefaultaction( datatypefunction );
}

void
SLIInterpreter::initdictionaries( void )
{
  assert( DStack == NULL );

  DStack = new DictionaryStack();
  assert( DStack != NULL );

  errordict = new Dictionary();
  DictionaryDatum sysdict( new Dictionary() );
  DictionaryDatum userdict( new Dictionary() );

  DStack->push( sysdict );
  DStack->set_basedict();

  def( errordict_name, DictionaryDatum( errordict ) );
  def( systemdict_name, sysdict );
  def( userdict_name, userdict );
  def( "statusdict", DictionaryDatum( statusdict ) );
}

void
SLIInterpreter::initbuiltins( void )
{

  createcommand( ilookup_name, &SLIInterpreter::ilookupfunction );
  createcommand( ipop_name, &SLIInterpreter::ilookupfunction );
  createcommand( isetcallback_name, &SLIInterpreter::isetcallbackfunction );
  createcommand( iiterate_name, &SLIInterpreter::iiteratefunction );
  createcommand( iloop_name, &SLIInterpreter::iloopfunction );
  createcommand( irepeat_name, &SLIInterpreter::irepeatfunction );
  createcommand( ifor_name, &SLIInterpreter::iforfunction );
  createcommand( iforallarray_name, &SLIInterpreter::iforallarrayfunction );
  createcommand( iforalliter_name, &SLIInterpreter::iforalliterfunction );
  createcommand(
    iforallindexedstring_name, &SLIInterpreter::iforallindexedstringfunction );
  createcommand(
    iforallindexedarray_name, &SLIInterpreter::iforallindexedarrayfunction );
  createcommand( iforallstring_name, &SLIInterpreter::iforallstringfunction );

  createdouble( pi_name, numerics::pi );
  createdouble( e_name, numerics::e );
}

void
SLIInterpreter::initexternals( void )
{
  init_slidict( this );
  init_slicontrol( this );
  init_sli_io( this );
  init_slistack( this );
  init_slimath( this );
  init_slitypecheck( this );
  init_slidata( this );
  init_slistring( this );

  ArrayDatum* ad = new ArrayDatum();
  Token at( ad );
  def_move( commandstring_name, at );
}

/**************************************
  The interpreter's dictionary stack is accessed through member functions
  of the interpreter. This is a slight inconsistency with the way the
  other stacks are accessed, however,  class DictionaryStack has
  to know class Interpreter. Thous, the dictionary stack is allocated on the
  free storage.
****************************************/

FunctionDatum*
SLIInterpreter::Ilookup( void ) const
{
  return new FunctionDatum(
    ilookup_name, &SLIInterpreter::ilookupfunction, "" );
}

FunctionDatum*
SLIInterpreter::Iiterate( void ) const
{
  return new FunctionDatum(
    iiterate_name, &SLIInterpreter::iiteratefunction, "" );
}

void
SLIInterpreter::createdouble( Name const& n, double d )
{
  Token t( new DoubleDatum( d ) );
  DStack->def_move( n, t );
}

/** Define a function in the current dictionary.
 *  This function defines a SLI function in the current dictionary.
 *  Note that you may also pass a string as the first argument, as
 *  there is an implicit type conversion operator from string to Name.
 *  Use the Name when a name object for this function already
 *  exists.
 */
void
SLIInterpreter::createcommand( Name const& n,
  SLIFunction const* fn,
  std::string deprecation_info )
{
  if ( DStack->known( n ) )
  {
    throw NamingConflict("A function called '" + std::string(n.toString())
                         + "' exists already.\n"
                         "Please choose a different name!");
  }

  Token t( new FunctionDatum( n, fn, deprecation_info ) );
  DStack->def_move( n, t );
}

/** Define a constant in the current dictionary.
 *  This function defines a SLI constant in the current dictionary.
 *  Note that you may also pass a string as the first argument, as
 *  there is an implicit type conversion operator from string to Name.
 *  Use the Name when a name object for this function already
 *  exists.
 */
void
SLIInterpreter::createconstant( Name const& n, Token const& val )
{
  Token t( val );
  DStack->def_move( n, t );
}

const Token&
SLIInterpreter::lookup( const Name& n ) const
{
  return DStack->lookup( n );
}

const Token&
SLIInterpreter::lookup2( const Name& n ) const
{
  return DStack->lookup2( n );
}

const Token&
SLIInterpreter::baselookup( const Name& n ) const
{
  return DStack->baselookup( n );
}

bool
SLIInterpreter::known( const Name& n ) const
{
  return DStack->known( n );
}

bool
SLIInterpreter::baseknown( const Name& n ) const
{
  return DStack->baseknown( n );
}

void
SLIInterpreter::def( Name const& n, Token const& t )
{
  DStack->def( n, t );
}

void
SLIInterpreter::undef( Name const& n )
{
  DStack->undef( n );
}

void
SLIInterpreter::basedef( Name const& n, Token const& t )
{
  DStack->basedef( n, t );
}

void
SLIInterpreter::def_move( Name const& n, Token& t )
{
  DStack->def_move( n, t );
}

void
SLIInterpreter::basedef_move( Name const& n, Token& t )
{
  DStack->basedef_move( n, t );
}

SLIInterpreter::SLIInterpreter( void )
  : debug_mode_( false )
  , show_stack_( false )
  , show_backtrace_( false )
  , catch_errors_( false )
  , opt_tailrecursion_( true )
  , call_depth_( 0 )
  , max_call_depth_( 10 )
  , cycle_count( 0 )
  , cycle_guard( false )
  , cycle_restriction( 0 )
  , verbositylevel( M_INFO )
  , statusdict( 0 )
  , errordict( 0 )
  , DStack( 0 )
  , parse( 0 )
  , ilookup_name( "::lookup" )
  , ipop_name( "::pop" )
  , isetcallback_name( "::setcallback" )
  , iiterate_name( "::executeprocedure" )
  , iloop_name( "::loop" )
  , irepeat_name( "::repeat" )
  , ifor_name( "::for" )
  , iforallarray_name( "::forall_a" )
  , iforalliter_name( "::forall_iter" )
  , iforallindexedarray_name( "::forallindexed_a" )
  , iforallindexedstring_name( "::forallindexed_s" )
  , iforallstring_name( "::forall_s" )

  /** @BeginDocumentation
   Name: Pi - Value of the constant Pi= 3.1415...
   Synopsis:  Pi -> double
   Description: Pi yields an approximation with a precision of 12 digits.
   Author: Diesmann, Hehl
   FirstVersion: 10.6.99
   References:
   SeeAlso: E, sin, cos
  */

  , pi_name( "Pi" )

  /** @BeginDocumentation
   Name: E - Value of the Euler constant E=2.718...
   Synopsis:  E -> double
   Description: E is the result of the builtin function std::exp(1).
   The precision of this value is therefore system-dependent.

   Author: Diesmann, Hehl
   FirstVersion: 10.6.99
   SeeAlso: exp
  */

  , e_name( "E" )
  , iparse_name( "::parse" )
  , stop_name( "stop" )
  , end_name( "end" )
  , null_name( "null" )
  , true_name( "true" )
  , false_name( "false" )
  , mark_name( "mark" )
  , istopped_name( "::stopped" )
  , systemdict_name( "systemdict" )
  , userdict_name( "userdict" )

  /** @BeginDocumentation
   Name: errordict - pushes error dictionary on operand stack
   Synopsis: errordict -> dict
   Description:
    Pushes the dictionary object errordict on the operand stack.
    errordict is not an operator; it is a name in systemdict associated
    with the dictionary object.

    The flag newerror helps to distinguish
    between interrupts caused by call of
    stop and interrupts raised by raiseerror.

    The name command contains the name of the command which
    caused the most recent error.

    The flag recordstacks decides whether the state of the interpreter
    is saved on error.
    If reckordstacks is true, the following state objects are saved

    Operand stack    -> ostack
    Dictionary stack -> dstack
    Execution stack  -> estack

   Parameters: none
   Examples: errordict info -> shows errordict
   Remarks: commented  1.4.1999, Diesmann
   SeeAlso: raiseerror, raiseagain, info
   References: The Red Book 2nd. ed. p. 408
  */
  , errordict_name( "errordict" )
  , quitbyerror_name( "quitbyerror" )
  , newerror_name( "newerror" )
  , errorname_name( "errorname" )
  , commandname_name( "commandname" )
  , signo_name( "sys_signo" )
  , recordstacks_name( "recordstacks" )
  , estack_name( "estack" )
  , ostack_name( "ostack" )
  , dstack_name( "dstack" )
  , commandstring_name( "moduleinitializers" )
  , interpreter_name( "SLIInterpreter::execute" )
  , ArgumentTypeError( "ArgumentType" )
  , StackUnderflowError( "StackUnderflow" )
  , UndefinedNameError( "UndefinedName" )
  , WriteProtectedError( "WriteProtected" )
  , DivisionByZeroError( "DivisionByZero" )
  , RangeCheckError( "RangeCheck" )
  , PositiveIntegerExpectedError( "PositiveIntegerExpected" )
  , BadIOError( "BadIO" )
  , StringStreamExpectedError( "StringStreamExpected" )
  , CycleGuardError( "AllowedCyclesExceeded" )
  , SystemSignal( "SystemSignal" )
  , BadErrorHandler( "BadErrorHandler" )
  , KernelError( "KernelError" )
  , InternalKernelError( "InternalKernelError" )
  , OStack( 100 )
  , EStack( 100 )
{
  inittypes();

  initdictionaries();
  initbuiltins();
  parse = new Parser( std::cin );

  initexternals();

#ifndef HAVE_MPI
  // Set a signal handler if it is not ignored.
  // If the SIGINT is ignored, we are most likely running as
  // a background process.
  // Here, we use a posix conforming substitute for the
  // ISO C signal function. It is defined in psignal.{h,cc}

  if ( posix_signal( SIGINT, ( Sigfunc* ) SIG_IGN ) != ( Sigfunc* ) SIG_IGN )
  {
    posix_signal( SIGINT, ( Sigfunc* ) SLISignalHandler );
  }
  if ( posix_signal( SIGUSR1, ( Sigfunc* ) SIG_IGN ) != ( Sigfunc* ) SIG_IGN )
  {
    posix_signal( SIGUSR1, ( Sigfunc* ) SLISignalHandler );
  }
  if ( posix_signal( SIGUSR2, ( Sigfunc* ) SIG_IGN ) != ( Sigfunc* ) SIG_IGN )
  {
    posix_signal( SIGUSR2, ( Sigfunc* ) SLISignalHandler );
  }
#endif

  errordict->insert( quitbyerror_name, baselookup( false_name ) );
}

void
SLIInterpreter::addmodule( SLIModule* m )
{
  modules.push_back( m );
  try
  {
    m->install( std::cerr, this );
  }
  catch ( SLIException& e )
  {
    message( M_ERROR,
      "SLIInterpreter",
      ( "An error occured while loading module " + m->name() ).c_str() );
    message( M_ERROR, "SLIInterpreter", e.what() );
    message( M_ERROR, "SLIInterpreter", e.message().c_str() );
    return;
  }
  catch ( std::exception& e )
  {
    message( M_ERROR,
      "SLIInterpreter",
      ( "A C++ library exception occured while loading module " + m->name() )
        .c_str() );
    message( M_ERROR, "SLIInterpreter", e.what() );
    return;
  }
  catch ( ... )
  {
    message( M_ERROR,
      "SLIInterpreter",
      ( "An unspecified exception occured while loading module " + m->name() )
        .c_str() );
    return;
  }

  // Add commandstring to list of module initializers. They will be executed
  // by sli-init.sli once all C++ stuff is loaded.
  if ( not( m->commandstring().empty() ) )
  {
    ArrayDatum* ad =
      dynamic_cast< ArrayDatum* >( baselookup( commandstring_name ).datum() );
    assert( ad != NULL );
    ad->push_back( new StringDatum( m->commandstring() ) );
  }
}

void
SLIInterpreter::addlinkedusermodule( SLIModule* m )
{
  m->install( std::cerr, this );

  // Add commandstring to list of module initializers. They will be executed
  // by sli-init.sli once all C++ stuff is loaded.
  if ( not( m->commandstring().empty() ) )
  {
    ArrayDatum* ad =
      dynamic_cast< ArrayDatum* >( baselookup( commandstring_name ).datum() );
    assert( ad != NULL );
    ad->push_back( new StringDatum( m->commandstring() ) );
  }
}


SLIInterpreter::~SLIInterpreter()
{
  // Make sure there is no more data on the stacks
  // before the modules are deleted.
  OStack.clear();
  EStack.clear();

  for_each( modules.rbegin(), modules.rend(), delete_ptr< SLIModule >() );

  DStack->pop();
  delete DStack;
  delete parse;

  Integertype.deletetypename();
  Doubletype.deletetypename();
  Stringtype.deletetypename();
  Nametype.deletetypename();
  Booltype.deletetypename();
  Literaltype.deletetypename();
  Arraytype.deletetypename();
  Proceduretype.deletetypename();
  Litproceduretype.deletetypename();
  Dictionarytype.deletetypename();
  Symboltype.deletetypename();
  Functiontype.deletetypename();
  Trietype.deletetypename();
  Callbacktype.deletetypename();
  Istreamtype.deletetypename();
  XIstreamtype.deletetypename();
  Ostreamtype.deletetypename();
  IntVectortype.deletetypename();
  DoubleVectortype.deletetypename();
}

void
SLIInterpreter::raiseerror( Name err )
{
  Name caller = getcurrentname();
  EStack.pop();
  raiseerror( caller, err );
}

void
SLIInterpreter::raiseerror( std::exception& err )
{
  Name caller = getcurrentname();

  assert( errordict != NULL );
  errordict->insert(
    "command", EStack.top() ); // store the func/trie that caused the error.

  // SLIException provide addtional information
  SLIException* slierr = dynamic_cast< SLIException* >( &err );

  if ( slierr )
  {
    // err is a SLIException
    errordict->insert( Name( "message" ), slierr->message() );
    raiseerror( caller, slierr->what() );
  }
  else
  {
    // plain std::exception: turn what() output into message
    errordict->insert( Name( "message" ), std::string( err.what() ) );
    raiseerror( caller, "C++Exception" );
  }
}

void
SLIInterpreter::raiseerror( Name cmd, Name err )
{

  // All error related symbols are now in their correct dictionary,
  // the error dictionary $errordict ( see Bug #4)

  assert( errordict != NULL );

  if ( errordict->lookup( newerror_name ) == baselookup( false_name ) )
  {
    errordict->insert( newerror_name, baselookup( true_name ) );
    errordict->insert( errorname_name, LiteralDatum( err ) );
    errordict->insert( commandname_name, LiteralDatum( cmd ) );
    if ( errordict->lookup( recordstacks_name ) == baselookup( true_name ) )
    {
      Token est( new ArrayDatum( EStack.toArray() ) );
      Token ost( new ArrayDatum( OStack.toArray() ) );
      TokenArray olddstack;
      DStack->toArray( olddstack );
      Token dst( new ArrayDatum( olddstack ) );

      errordict->insert_move( estack_name, est );
      errordict->insert_move( ostack_name, ost );
      errordict->insert_move( dstack_name, dst );
    }

    OStack.push( LiteralDatum( cmd ) );
    EStack.push( baselookup( stop_name ) );
  }
  else // There might be an error in the error-handler
  {
    errordict->insert( newerror_name, baselookup( false_name ) );
    raiseerror( Name( "raiserror" ), BadErrorHandler );
    return;
  }
}

void
SLIInterpreter::print_error( Token cmd )
{
  // Declare the variables where the information
  // about the error is stored.
  std::string errorname;
  std::ostringstream msg;

  // Read errorname from dictionary.
  if ( errordict->known( errorname_name ) )
  {
    errorname = std::string( errordict->lookup( errorname_name ) );
  }

  // Find the correct message for the errorname.

  // If errorname is equal to SystemError no message string
  // is printed. The if-else branching below follows the
  // syntax of the lib/sli/sli-init.sli function
  // /:print_error
  if ( errorname == "SystemError" )
  {
  }
  else if ( errorname == "BadErrorHandler" )
  {
    msg << ": The error handler of a stopped context "
        << "contained itself an error.";
  }
  else
  {
    // Read a pre-defined message from dictionary.
    if ( errordict->known( Name( "message" ) ) )
    {
      msg << errordict->lookup( Name( "message" ) );
      errordict->erase( Name( "message" ) );
    }

    // Print command information for error command.
    if ( errordict->known( Name( "command" ) ) )
    {
      Token command = errordict->lookup( Name( "command" ) );
      errordict->erase( Name( "command" ) );

      // Command information is only printed if the
      // command is of trietype
      if ( command.datum() != NULL )
      {
        if ( command->gettypename() == Name( "trietype" ) )
        {
          msg << "\n\nCandidates for " << command << " are:\n";

          TrieDatum* trie = dynamic_cast< TrieDatum* >( command.datum() );
          assert( trie != NULL );

          trie->get().info( msg );
        }
      }
    }
  }

  // Error message header is defined as "$errorname in $cmd"
  std::string from = std::string( cmd );

  // Print error.
  message( M_ERROR, from.c_str(), msg.str().c_str(), errorname.c_str() );
}

void
SLIInterpreter::raiseagain( void )
{
  assert( errordict != NULL );

  if ( errordict->known( commandname_name ) )
  {
    Token cmd_t = errordict->lookup( commandname_name );
    assert( not cmd_t.empty() );
    errordict->insert( newerror_name, baselookup( true_name ) );
    OStack.push_move( cmd_t );
    EStack.push( baselookup( stop_name ) );
  }
  else
  {
    raiseerror( Name( "raiseagain" ), BadErrorHandler );
  }
}

void
SLIInterpreter::raisesignal( int sig )
{
  Name caller = getcurrentname();

  errordict->insert( signo_name, IntegerDatum( sig ) );

  raiseerror( caller, SystemSignal );
}

void
SLIInterpreter::verbosity( int level )
{
  verbositylevel = level;
}

int
SLIInterpreter::verbosity( void ) const
{
  return verbositylevel;
}

void
SLIInterpreter::terminate( int returnvalue )
{
  if ( returnvalue == -1 )
  {
    assert( statusdict->known( "exitcodes" ) );
    DictionaryDatum exitcodes =
      getValue< DictionaryDatum >( *statusdict, "exitcodes" );
    returnvalue = getValue< long >( exitcodes, "fatal" );
  }

  message( M_FATAL, "SLIInterpreter", "Exiting." );
  delete this;
  std::exit( returnvalue );
}

void
SLIInterpreter::message( int level,
  const char from[],
  const char text[],
  const char errorname[] ) const
{
// Only one thread may write at a time.
#ifdef _OPENMP
#pragma omp critical( message )
  {
#endif
    if ( level >= verbositylevel )
    {
      if ( level >= M_FATAL )
      {
        message( std::cout, M_FATAL_NAME, from, text, errorname );
      }
      else if ( level >= M_ERROR )
      {
        message( std::cout, M_ERROR_NAME, from, text, errorname );
      }
      else if ( level >= M_WARNING )
      {
        message( std::cout, M_WARNING_NAME, from, text, errorname );
      }
      else if ( level >= M_DEPRECATED )
      {
        message( std::cout, M_DEPRECATED_NAME, from, text, errorname );
      }
      else if ( level >= M_INFO )
      {
        message( std::cout, M_INFO_NAME, from, text, errorname );
      }
      else if ( level >= M_STATUS )
      {
        message( std::cout, M_STATUS_NAME, from, text, errorname );
      }
      else if ( level >= M_DEBUG )
      {
        message( std::cout, M_DEBUG_NAME, from, text, errorname );
      }
      else
      {
        message( std::cout, M_ALL_NAME, from, text, errorname );
      }
    }

#ifdef _OPENMP
  }
#endif
}

void
SLIInterpreter::message( std::ostream& out,
  const char levelname[],
  const char from[],
  const char text[],
  const char errorname[] ) const
{
  const unsigned buflen = 30;
  char timestring[ buflen + 1 ] = "";
  const time_t tm = std::time( NULL );

  std::strftime( timestring, buflen, "%b %d %H:%M:%S", std::localtime( &tm ) );

  std::string msg =
    String::compose( "%1 %2 [%3]: ", timestring, from, levelname );
  out << std::endl
      << msg << errorname;

  // Set the preferred line indentation.
  const size_t indent = 4;

  // Get size of the output window. The message text will be
  // adapted to the width of the window.
  //
  // The COLUMNS variable should preferably be extracted
  // from the environment dictionary set up by the
  // Processes class. getenv("COLUMNS") works only on
  // the created NEST executable (not on the messages
  // printed by make install).
  char const* const columns = std::getenv( "COLUMNS" );
  size_t max_width = 78;
  if ( columns )
  {
    max_width = std::atoi( columns );
  }
  if ( max_width < 3 * indent )
  {
    max_width = 3 * indent;
  }
  const size_t width = max_width - indent;

  // convert char* to string to be able to use the string functions
  std::string text_str( text );

  // Indent first message line
  if ( text_str.size() != 0 )
  {
    std::cout << std::endl
              << std::string( indent, ' ' );
  }

  size_t pos = 0;

  for ( size_t i = 0; i < text_str.size(); ++i )
  {
    if ( text_str.at( i ) == '\n' && i != text_str.size() - 1 )
    {
      // Print a lineshift followed by an indented whitespace
      // Manually inserted lineshift at the end of the message
      // are suppressed.
      out << std::endl
          << std::string( indent, ' ' );
      pos = 0;
    }
    else
    {
      // If we've reached the width of the output we'll print
      // a lineshift regardless of whether '\n' is found or not.
      // The printing is done so that no word splitting occurs.
      size_t space = text_str.find( ' ', i ) < text_str.find( '\n' )
        ? text_str.find( ' ', i )
        : text_str.find( '\n' );
      // If no space is found (i.e. the last word) the space
      // variable is set to the end of the string.
      if ( space == std::string::npos )
      {
        space = text_str.size();
      }

      // Start on a new line if the next word is longer than the
      // space available (as long as the word is shorter than the
      // total width of the printout).
      if ( i != 0 && text_str.at( i - 1 ) == ' '
        && static_cast< int >( space - i ) > static_cast< int >( width - pos ) )
      {
        out << std::endl
            << std::string( indent, ' ' );
        pos = 0;
      }

      // Only print character if we're not at the end of the
      // line and the last character is a space.
      if ( not( width - pos == 0 && text_str.at( i ) == ' ' ) )
      {
        // Print the actual character.
        out << text_str.at( i );
      }

      ++pos;
    }
  }
  out << std::endl;
}

Name
SLIInterpreter::getcurrentname( void ) const
{
  FunctionDatum* func = dynamic_cast< FunctionDatum* >( EStack.top().datum() );
  if ( func != NULL )
  {
    return ( func->getname() );
  }
  TrieDatum* trie = dynamic_cast< TrieDatum* >( EStack.top().datum() );
  if ( trie != NULL )
  {
    return ( trie->getname() );
  }
  return interpreter_name;
}

void
SLIInterpreter::setcycleguard( Index c )
{
  cycle_guard = true;
  cycle_restriction = cycles() + c;
}

void
SLIInterpreter::removecycleguard( void )
{
  cycle_guard = false;
}

void
SLIInterpreter::toggle_stack_display()
{
  show_stack_ = not show_stack_;
  std::string msg =
    std::string( "Stack display is now " ) + ( show_stack_ ? "On" : "Off" );
  message( M_INFO, "SLIInterpreter", msg.c_str() );
}

void
SLIInterpreter::backtrace_on()
{
  show_backtrace_ = true;
  opt_tailrecursion_ = false;
  std::string msg =
    "Showing stack backtrace on error.  Disabling tail recursion optimization.";
  message( M_INFO, "SLIInterpreter", msg.c_str() );
}

void
SLIInterpreter::backtrace_off()
{
  show_backtrace_ = false;
  opt_tailrecursion_ = true;
  std::string msg =
    "Stack backtrace on error in now off. Re-enabling tail recursion "
    "optimization.";
  message( M_INFO, "SLIInterpreter", msg.c_str() );
}

/**
 * List the execution stack from level n-1 downwards to level 0. If you want the
 * entire stack to be displayed, call
 * the function as stack_backtrace(EStack.load());
 */
void
SLIInterpreter::stack_backtrace( int n )
{
  for ( int p = n - 1; p >= 0; --p )
  {
    if ( ( size_t ) p > EStack.load() )
    {
      continue;
    }

    FunctionDatum* fd =
      dynamic_cast< FunctionDatum* >( EStack.pick( p ).datum() );
    if ( fd != 0 )
    {
      fd->backtrace( this, p );
      continue;
    }
    NameDatum* nd = dynamic_cast< NameDatum* >( EStack.pick( p ).datum() );
    if ( nd != 0 )
    {
      std::cerr << "While executing: ";
      nd->print( std::cerr );
      std::cerr << std::endl;
      continue;
    }
    TrieDatum* td = dynamic_cast< TrieDatum* >( EStack.pick( p ).datum() );
    if ( td != 0 )
    {
      std::cerr << "While executing: ";
      td->print( std::cerr );
      std::cerr << std::endl;
      continue;
    }
  }
}

void
SLIInterpreter::debug_options() const
{
  std::cerr << "Type one of the following commands:\n"
            << "\nInspection:\n"
            << "  n)ext       - Trace (execute) next command.\n"
            << "  l)ist       - list current procedure or loop.\n"
            << "  w)here      - show backtrace of execution stack.\n"
            << "  c)ontinue   - Continue this level without debugging\n"
            << "  step        - Step over deeper levels.\n"
            << "  stack       - show operand stack.\n"
            << "  estack      - show execution stack.\n"
            << "  e)dit       - enter interactive mode.\n"
            << "  stop        - raise an exception.\n"
            << "  h)elp       - display this list.\n"
            << "  q)uit       - quit debug mode.\n\n"
            << "  show next   - show next command.\n"
            << "  show stack  - show operand stack.\n"
            << "  show backtrace- same as 'where'.\n"
            << "  show estack - show execution stack.\n\n"
            << "  toggle stack     - toggle stack display.\n"
            << "  toggle catch     - toggle debug on error.\n"
            << "  toggle backtrace - toggle stack backtrace on error.\n"
            << "  toggle tailrecursion - toggle tail-recursion optimisation.\n";
}


char
SLIInterpreter::debug_commandline( Token& next )
{
  char c = '\n';

  std::string command;
  std::string arg;

  // /dev/tty is the UNIX  file representing the keyboard. We directly read from
  // it to be able to close the input
  // with CTRL-D. If std::cin is closed with ctrl-D we cannot re-open it again
  // and the debugger would be dysfunctional for the remainder of the session.
  std::ifstream tty( "/dev/tty" );
  if ( show_stack_ )
  {
    OStack.dump( std::cerr );
  }
  std::cerr << "Next token: ";
  next.pprint( std::cerr );
  std::cerr << std::endl;

  do
  {
    std::cerr << call_depth_ << "/" << max_call_depth_ << ">";

    tty >> command;
    if ( tty.eof() )
    {
      std::cerr << std::endl;
      debug_mode_off();
      return c;
    }

    if ( SLIsignalflag != 0 )
    {
      std::cerr << "Caught Signal Number " << SLIsignalflag << std::endl;
      SLIsignalflag = 0;
      tty.clear();
      continue;
    }

    if ( command == "show" )
    {
      tty >> arg;
      if ( arg == "stack" )
      {
        OStack.dump( std::cerr );
      }
      else if ( arg == "estack" )
      {
        EStack.dump( std::cerr );
      }
      else if ( arg == "backtrace" )
      {
        stack_backtrace( EStack.load() );
      }
      else if ( arg == "next" || arg == "n" )
      {
        std::cerr << "Next token: ";
        next.pprint( std::cerr );
        std::cerr << std::endl;
      }
      else
      {
        std::cerr << "show: Unknown argument. Type 'help' for help."
                  << std::endl;
      }
      continue;
    }
    else if ( command == "toggle" )
    {
      tty >> arg;
      if ( arg == "backtrace" )
      {
        show_backtrace_ = not show_backtrace_;
        std::cerr << "Stack backtrace is now "
                  << ( show_backtrace_ ? " On." : "Off." ) << std::endl;
      }
      else if ( arg == "stack" )
      {
        show_stack_ = not show_stack_;
        std::cerr << "Stack display is now "
                  << ( show_stack_ ? " On." : "Off." ) << std::endl;
      }
      else if ( arg == "catch" )
      {
        catch_errors_ = not catch_errors_;
        std::cerr << "Catch error mode is now "
                  << ( catch_errors_ ? " On." : "Off." ) << std::endl;
      }
      else if ( arg == "tailrecursion" || arg == "tail" )
      {
        opt_tailrecursion_ = not opt_tailrecursion_;
        std::cerr << "Tail-recursion optimization is now "
                  << ( opt_tailrecursion_ ? " On." : "Off." ) << std::endl;
      }
    }
    else if ( command == "list" || command == "l" )
    {
      c = 'l';
      break;
    }
    else if ( command == "stop" )
    {
      debug_mode_off();
      EStack.push( new NameDatum( stop_name ) );
      break;
    }
    else if ( command == "catch" )
    {
      catch_errors_ = true;
      std::cerr << "Catch error mode is now "
                << ( catch_errors_ ? " On." : "Off." ) << std::endl;
    }
    else if ( command == "where" || command == "w" )
    {
      stack_backtrace( EStack.load() );
    }
    else if ( command == "edit" || command == "break" || command == "e" )
    {
      debug_mode_off();
      std::cerr << "Type 'continue', to exit interactive mode." << std::endl;
      EStack.push( new NameDatum( "debugon" ) ); // restart debugging mode
      EStack.push( baselookup( mark_name ) );
      EStack.push( new XIstreamDatum( std::cin ) );
      EStack.push( baselookup( iparse_name ) );
      c = 'i';
      break;
    }
    else if ( command == "stack" )
    {
      OStack.dump( std::cerr );
    }
    else if ( command == "estack" )
    {
      EStack.dump( std::cerr );
    }
    else if ( command == "help" || command == "?" || command == "h" )
    {
      debug_options();
    }
    else if ( command == "next" || command == "n" )
    {
      break;
    }
    else if ( command == "continue" || command == "cont" || command == "c" )
    {
      max_call_depth_ = call_depth_; // will show lower levels only
    }
    else if ( command == "step" )
    {
      max_call_depth_ = call_depth_ + 1; // will this level and lower.
    }
    else if ( command == "quit" || command == "q" )
    {
      debug_mode_ = false;
      break;
    }
    else
    {
      std::cerr
        << "Unknown command. Type 'help' for help, or 'quit' to leave debugger."
        << std::endl;
    }
  } while ( true );

  return c;
}

int
SLIInterpreter::startup()
{
  static bool is_initialized = false;
  int exitcode = EXIT_SUCCESS;

  if ( not is_initialized && EStack.load() > 0 )
  {
    exitcode = execute_(); // run the interpreter
    is_initialized = true;
  }
  return exitcode;
}

int
SLIInterpreter::execute( const std::string& cmdline )
{
  int exitcode = startup();
  if ( exitcode != EXIT_SUCCESS )
  {
    return -1;
  }

  OStack.push( new StringDatum( cmdline ) );
  EStack.push( new NameDatum( "::evalstring" ) );
  return execute_(); // run the interpreter
}

int
SLIInterpreter::execute( const Token& cmd )
{
  int exitcode = startup();
  if ( exitcode != EXIT_SUCCESS )
  {
    return -1;
  }

  EStack.push( cmd );
  return execute_(); // run the interpreter
}


int
SLIInterpreter::execute( int v )
{
  startup();
  EStack.push( new NameDatum( "start" ) );
  switch ( v )
  {
  case 0:
  case 1:
    return execute_(); // run the interpreter
  case 2:
    return execute_debug_();
  default:
    return -1;
  }
}

int
SLIInterpreter::execute_debug_( size_t exitlevel )
{
  int exitcode;
  assert( statusdict->known( "exitcodes" ) );
  DictionaryDatum exitcodes =
    getValue< DictionaryDatum >( *statusdict, "exitcodes" );

  if ( SLIsignalflag != 0 )
  {
    exitcode = getValue< long >( exitcodes, "unknownerror" );
    return exitcode;
  }

  try
  {
    do
    { // loop1  this double loop to keep the try/catch outside the inner loop
      try
      {
        while ( EStack.load() > exitlevel ) // loop 2
        {
          ++cycle_count;
          EStack.top()->execute( this );
        }
      }
      catch ( std::exception& exc )
      {
        raiseerror( exc );
      }
    } while ( EStack.load() > exitlevel );
  }
  catch ( std::exception& e )
  {
    message( M_FATAL, "SLIInterpreter", "A C++ library exception occured." );
    OStack.dump( std::cerr );
    EStack.dump( std::cerr );
    message( M_FATAL, "SLIInterpreter", e.what() );
    exitcode = getValue< long >( *exitcodes, "exception" );
    terminate( exitcode );
  }
  catch ( ... )
  {
    message( M_FATAL, "SLIInterpreter", "An unknown c++ exception occured." );
    OStack.dump( std::cerr );
    EStack.dump( std::cerr );
    exitcode = getValue< long >( *exitcodes, "exception" );
    terminate( exitcode );
  }

  assert( statusdict->known( "exitcode" ) );
  exitcode = getValue< long >( *statusdict, "exitcode" );
  if ( exitcode != 0 )
  {
    errordict->insert( quitbyerror_name, baselookup( true_name ) );
  }

  return exitcode;
}

int
SLIInterpreter::execute_( size_t exitlevel )
{
  int exitcode;
  assert( statusdict->known( "exitcodes" ) );
  DictionaryDatum exitcodes =
    getValue< DictionaryDatum >( *statusdict, "exitcodes" );

  if ( SLIsignalflag != 0 )
  {
    exitcode = getValue< long >( exitcodes, "unknownerror" );
    return exitcode;
  }

  try
  {
    do
    { // loop1  this double loop to keep the try/catch outside the inner loop
      try
      {
        while ( not SLIsignalflag and ( EStack.load() > exitlevel ) ) // loop 2
        {
          ++cycle_count;
          EStack.top()->execute( this );
        }
        if ( SLIsignalflag != 0 )
        {
          SLIsignalflag = 0;
          raisesignal( SLIsignalflag );
        }
      }
      catch ( std::exception& exc )
      {
        raiseerror( exc );
      }
    } while ( EStack.load() > exitlevel );
  }
  catch ( std::exception& e )
  {
    message( M_FATAL, "SLIInterpreter", "A C++ library exception occured." );
    OStack.dump( std::cerr );
    EStack.dump( std::cerr );
    message( M_FATAL, "SLIInterpreter", e.what() );
    exitcode = getValue< long >( *exitcodes, "exception" );
    terminate( exitcode );
  }
  catch ( ... )
  {
    message( M_FATAL, "SLIInterpreter", "An unknown c++ exception occured." );
    OStack.dump( std::cerr );
    EStack.dump( std::cerr );
    exitcode = getValue< long >( *exitcodes, "exception" );
    terminate( exitcode );
  }

  assert( statusdict->known( "exitcode" ) );
  exitcode = getValue< long >( *statusdict, "exitcode" );
  if ( exitcode != 0 )
  {
    errordict->insert( quitbyerror_name, baselookup( true_name ) );
  }

  return exitcode;
}
