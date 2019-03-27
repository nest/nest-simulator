/*
 *  slistartup.cc
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

#include "slistartup.h"

// C++ includes:
#include <cstdlib>
#include <cstring>
#include <fstream>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dictdatum.h"
#include "integerdatum.h"
#include "interpret.h"
#include "iostreamdatum.h"
#include "namedatum.h"
#include "stringdatum.h"

extern int SLIsignalflag;

// Access to environement variables.
#ifdef __APPLE__
#include <crt_externs.h>
#define environ ( *_NSGetEnviron() )
#else
extern char** environ;
#endif

/*
1.  Propagate commandline to the sli level.
    Commandline options will be handled by the startup file.

2.  Locate startup file and prepare the start symbol
    to run the startup script.
3.  If startup-script cannot be located, issue meaningful diagnostic
    messages and exit gracefuly.
*/

/**
 * Returns true, if the interpreter startup file sli-init.sli
 * is in the supplied path and false otherwise.
 */
bool
SLIStartup::checkpath( std::string const& path, std::string& result ) const
{
  const std::string fullpath = path + slilibpath;
  const std::string fullname = fullpath + "/" + startupfilename;

  std::ifstream in( fullname.c_str() );

  if ( in.good() )
  {
    result = fullname;
    return true;
  }
  else
  {
    result.erase();
    return false;
  }
}


/** @BeginDocumentation
Name: getenv - evaluates if a string is an evironment variable

Synopsis: string getenv -> path true
string getenv -> false

Description: getenv checks if the string is an environment variable. If
this is the case the path of the variable and true is pushed on the stack,
otherwise a false is pushed on the stack and the string is lost.

Examples:

SLI ] (HOME) getenv
SLI [2] pstack
true
(/home/gewaltig)

SLI ] (NONEXISTING) getenv =
false

SLI ] (NEST_DATA_DIR) getenv
SLI [2] { (Using root path: )  =only = }
SLI [3] { (Warning: $NEST_DATA_DIR undefined) =}
SLI [4] ifelse
Using root path: /home/gewaltig/nest/release/release

SLI ] (/home) getenv
false

Remarks: if getenv is used with the wrong argument (e.g. integer),
the SLI Session is terminated

Author: docu by Marc Oliver Gewaltig and Sirko Straube

SeeAlso: environment
 */

std::string
SLIStartup::getenv( const std::string& v ) const
{
  char* s = ::getenv( v.c_str() );
  if ( not s )
  {
    return std::string();
  }
  else
  {
    return std::string( s );
  }
}

void
SLIStartup::GetenvFunction::execute( SLIInterpreter* i ) const
{
  // string getenv -> string true
  //               -> false

  i->assert_stack_load( 1 );

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );
  const char* s = ::getenv( sd->c_str() );
  i->OStack.pop();
  if ( s != NULL )
  {
    Token t( new StringDatum( s ) );
    i->OStack.push_move( t );
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  }

  i->EStack.pop();
}

/**
 * Checks if the environment variable envvar contains a directory. If yes, the
 * path is returned, else an empty string is returned.
 */
std::string
SLIStartup::checkenvpath( std::string const& envvar,
  SLIInterpreter* i,
  std::string defaultval = "" ) const
{
  const std::string envpath = getenv( envvar );

  if ( envpath != "" )
  {
    DIR* dirptr = opendir( envpath.c_str() );
    if ( dirptr != NULL )
    {
      closedir( dirptr );
      return envpath;
    }
    else
    {
      std::string msg;
      switch ( errno )
      {
      case ENOTDIR:
        msg = String::compose( "'%1' is not a directory.", envpath );
        break;
      case ENOENT:
        msg = String::compose( "Directory '%1' does not exist.", envpath );
        break;
      default:
        msg = String::compose(
          "Errno %1 received when trying to open '%2'", errno, envpath );
        break;
      }

      i->message( SLIInterpreter::M_ERROR,
        "SLIStartup",
        String::compose( "%1 is not usable:", envvar ).c_str() );
      i->message( SLIInterpreter::M_ERROR, "SLIStartup", msg.c_str() );
      if ( defaultval != "" )
      {
        i->message( SLIInterpreter::M_ERROR,
          "SLIStartup",
          String::compose( "I'm using the default: %1", defaultval ).c_str() );
      }
    }
  }
  return std::string();
}


SLIStartup::SLIStartup( int argc, char** argv )
  : startupfilename( "sli-init.sli" )
  , slilibpath( "/sli" )
  , slihomepath( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DATADIR )
  , slidocdir( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DOCDIR )
  , sliprefix( NEST_INSTALL_PREFIX )
  , verbosity_( SLIInterpreter::M_INFO ) // default verbosity level
  , debug_( false )
  , argv_name( "argv" )
  , prgname_name( "prgname" )
  , exitcode_name( "exitcode" )
  , prgmajor_name( "prgmajor" )
  , prgminor_name( "prgminor" )
  , prgpatch_name( "prgpatch" )
  , prgbuilt_name( "built" )
  , prefix_name( "prefix" )
  , prgdatadir_name( "prgdatadir" )
  , prgdocdir_name( "prgdocdir" )
  , host_name( "host" )
  , hostos_name( "hostos" )
  , hostvendor_name( "hostvendor" )
  , hostcpu_name( "hostcpu" )
  , getenv_name( "getenv" )
  , statusdict_name( "statusdict" )
  , start_name( "start" )
  , intsize_name( "int" )
  , longsize_name( "long" )
  , havelonglong_name( "have_long_long" )
  , longlongsize_name( "long_long" )
  , doublesize_name( "double" )
  , pointersize_name( "void_ptr" )
  , architecturedict_name( "architecture" )
  , platform_name( "platform" )
  , threading_name( "threading" )
  , have_mpi_name( "have_mpi" )
  , ismpi_name( "is_mpi" )
  , have_gsl_name( "have_gsl" )
  , have_music_name( "have_music" )
  , have_libneurosim_name( "have_libneurosim" )
  , ndebug_name( "ndebug" )
  , exitcodes_name( "exitcodes" )
  , exitcode_success_name( "success" )
  , exitcode_skipped_name( "skipped" )
  , exitcode_skipped_no_mpi_name( "skipped_no_mpi" )
  , exitcode_skipped_have_mpi_name( "skipped_have_mpi" )
  , exitcode_skipped_no_threading_name( "skipped_no_threading" )
  , exitcode_skipped_no_gsl_name( "skipped_no_gsl" )
  , exitcode_skipped_no_music_name( "skipped_no_music" )
  , exitcode_scripterror_name( "scripterror" )
  , exitcode_abort_name( "abort" )
  , exitcode_userabort_name( "userabort" )
  , exitcode_segfault_name( "segfault" )
  , exitcode_exception_name( "exception" )
  , exitcode_fatal_name( "fatal" )
  , exitcode_unknownerror_name( "unknownerror" )
  , environment_name( "environment" )
{
  ArrayDatum ad;

  // argv[0] is the name of the program that was given to the shell.
  // This name must be given to SLI, otherwise initialization fails.
  // If we import NEST directly from the Python interpreter, that is, not from
  // a script but by using an interactive session in Python, argv[0] is an
  // empty string, see documentation for argv in
  // https://docs.python.org/3/library/sys.html
  for ( int i = 0; i < argc; ++i )
  {
    StringDatum* sd = new StringDatum( argv[ i ] );
    ad.push_back( Token( sd ) );

    if ( *sd == "-d" || *sd == "--debug" )
    {
      debug_ = true;
      verbosity_ = SLIInterpreter::M_ALL; // make the interpreter verbose.
      continue;
    }

    if ( *sd == "--verbosity=ALL" )
    {
      verbosity_ = SLIInterpreter::M_ALL;
      continue;
    }
    if ( *sd == "--verbosity=DEBUG" )
    {
      verbosity_ = SLIInterpreter::M_DEBUG;
      continue;
    }
    if ( *sd == "--verbosity=STATUS" )
    {
      verbosity_ = SLIInterpreter::M_STATUS;
      continue;
    }
    if ( *sd == "--verbosity=INFO" )
    {
      verbosity_ = SLIInterpreter::M_INFO;
      continue;
    }
    if ( *sd == "--verbosity=DEPRECATED" )
    {
      verbosity_ = SLIInterpreter::M_DEPRECATED;
      continue;
    }
    if ( *sd == "--verbosity=WARNING" )
    {
      verbosity_ = SLIInterpreter::M_WARNING;
      continue;
    }
    if ( *sd == "--verbosity=ERROR" )
    {
      verbosity_ = SLIInterpreter::M_ERROR;
      continue;
    }
    if ( *sd == "--verbosity=FATAL" )
    {
      verbosity_ = SLIInterpreter::M_FATAL;
      continue;
    }
    if ( *sd == "--verbosity=QUIET" )
    {
      verbosity_ = SLIInterpreter::M_QUIET;
      continue;
    }
  }
  targs = ad;
}

void
SLIStartup::init( SLIInterpreter* i )
{
  i->verbosity( verbosity_ );

  i->createcommand( getenv_name, &getenvfunction );
  std::string fname;

  // Check for supplied NEST_DATA_DIR
  std::string slihomepath_env = checkenvpath( "NEST_DATA_DIR", i, slihomepath );
  if ( slihomepath_env != "" )
  {
    slihomepath = slihomepath_env; // absolute path & directory exists
    i->message( SLIInterpreter::M_DEBUG,
      "SLIStartup",
      String::compose( "Using NEST_DATA_DIR=%1", slihomepath ).c_str() );
  }

  // check for supplied NEST_DOC_DIR
  std::string slidocdir_env = checkenvpath( "NEST_DOC_DIR", i, slidocdir );
  if ( slidocdir_env != "" )
  {
    slidocdir = slidocdir_env; // absolute path & directory exists
    i->message( SLIInterpreter::M_DEBUG,
      "SLIStartup",
      String::compose( "Using NEST_DOC_DIR=%1", slidocdir ).c_str() );
  }

  // check for supplied NEST_INSTALL_DIR
  std::string sliprefix_env = checkenvpath( "NEST_INSTALL_DIR", i, sliprefix );
  if ( sliprefix_env != "" )
  {
    sliprefix = sliprefix_env; // absolute path & directory exists
    i->message( SLIInterpreter::M_DEBUG,
      "SLIStartup",
      String::compose( "Using NEST_INSTALL_DIR=%1", sliprefix ).c_str() );
  }

  if ( not checkpath( slihomepath, fname ) )
  {
    i->message( SLIInterpreter::M_FATAL,
      "SLIStartup",
      "Your NEST installation seems broken. \n" );
    i->message( SLIInterpreter::M_FATAL,
      "SLIStartup",
      "I could not find the startup file that" );
    i->message( SLIInterpreter::M_FATAL,
      "SLIStartup",
      std::string( std::string( "should have been in " ) + slihomepath )
        .c_str() );
    i->message( SLIInterpreter::M_FATAL,
      "SLIStartup",
      "Please re-build NEST and try again." );
    i->message( SLIInterpreter::M_FATAL,
      "SLIStartup",
      "The file install.html in NEST's doc directory tells you how." );

    i->message( SLIInterpreter::M_FATAL, "SLIStartup", "Bye." );

    SLIsignalflag = 255; // this exits the interpreter.
    debug_ = false;      // switches off the -d/--debug switch!
    i->verbosity( SLIInterpreter::M_QUIET ); // suppress all further output.
  }
  else
  {
    std::string fname_msg = std::string( "Initialising from file: " ) + fname;
    i->message( SLIInterpreter::M_DEBUG, "SLIStartup", fname_msg.c_str() );
  }

  Token cin_token( new XIstreamDatum( std::cin ) );
  Token start_token( new NameDatum( start_name ) );

  DictionaryDatum statusdict( new Dictionary() );
  i->statusdict = &( *statusdict );
  assert( statusdict.valid() );

  statusdict->insert_move( argv_name, targs );
  statusdict->insert(
    prgname_name, Token( new StringDatum( NEST_VERSION_PRGNAME ) ) );
  statusdict->insert(
    exitcode_name, Token( new IntegerDatum( EXIT_SUCCESS ) ) );
  statusdict->insert(
    prgmajor_name, Token( new IntegerDatum( NEST_VERSION_MAJOR_REVISION ) ) );
  statusdict->insert(
    prgminor_name, Token( new IntegerDatum( NEST_VERSION_MINOR_REVISION ) ) );
  statusdict->insert(
    prgpatch_name, Token( new StringDatum( NEST_VERSION_PATCHLEVEL ) ) );
  statusdict->insert( prgbuilt_name,
    Token( new StringDatum(
      String::compose( "%1 %2", __DATE__, __TIME__ ) ) ) );
  statusdict->insert(
    prgdatadir_name, Token( new StringDatum( slihomepath ) ) );
  statusdict->insert( prgdocdir_name, Token( new StringDatum( slidocdir ) ) );
  statusdict->insert( prefix_name, Token( new StringDatum( sliprefix ) ) );
  statusdict->insert( host_name, Token( new StringDatum( NEST_HOST ) ) );
  statusdict->insert( hostos_name, Token( new StringDatum( NEST_HOSTOS ) ) );
  statusdict->insert(
    hostvendor_name, Token( new StringDatum( NEST_HOSTVENDOR ) ) );
  statusdict->insert( hostcpu_name, Token( new StringDatum( NEST_HOSTCPU ) ) );

  // expose platform model for code branching without assuming
  // configure leads to a unique setting
  std::string platform;

#ifdef IS_BLUEGENE_L
  platform += "bg/l";
#endif

#ifdef IS_BLUEGENE_P
  platform += "bg/p";
#endif

#ifdef IS_BLUEGENE_Q
  platform += "bg/q";
#endif

#ifdef IS_K
  platform = "k";
#endif
  if ( platform == "" )
  {
    platform = "default";
  }

  statusdict->insert( platform_name, Token( new StringDatum( platform ) ) );

  // expose threading model without assuming configure leads to a
  // unique setting
  std::string threading;

#ifdef _OPENMP
  threading += "openmp";
#endif
  if ( threading == "" )
  {
    threading = "no";
  }

  statusdict->insert( threading_name, Token( new StringDatum( threading ) ) );

#ifdef HAVE_MPI
  statusdict->insert( have_mpi_name, Token( new BoolDatum( true ) ) );
#else
  statusdict->insert( have_mpi_name, Token( new BoolDatum( false ) ) );
#endif

#ifdef HAVE_GSL
  statusdict->insert( have_gsl_name, Token( new BoolDatum( true ) ) );
#else
  statusdict->insert( have_gsl_name, Token( new BoolDatum( false ) ) );
#endif

#ifdef HAVE_MUSIC
  statusdict->insert( have_music_name, Token( new BoolDatum( true ) ) );
#else
  statusdict->insert( have_music_name, Token( new BoolDatum( false ) ) );
#endif

#ifdef HAVE_LIBNEUROSIM
  statusdict->insert( have_libneurosim_name, Token( new BoolDatum( true ) ) );
#else
  statusdict->insert( have_libneurosim_name, Token( new BoolDatum( false ) ) );
#endif

#ifdef NDEBUG
  statusdict->insert( ndebug_name, Token( new BoolDatum( true ) ) );
#else
  statusdict->insert( ndebug_name, Token( new BoolDatum( false ) ) );
#endif

  DictionaryDatum architecturedict( new Dictionary() );
  assert( architecturedict.valid() );

  architecturedict->insert(
    doublesize_name, Token( new IntegerDatum( sizeof( double ) ) ) );
  architecturedict->insert(
    pointersize_name, Token( new IntegerDatum( sizeof( void* ) ) ) );
  architecturedict->insert(
    intsize_name, Token( new IntegerDatum( sizeof( int ) ) ) );
  architecturedict->insert(
    longsize_name, Token( new IntegerDatum( sizeof( long ) ) ) );
  architecturedict->insert(
    "Token", Token( new IntegerDatum( sizeof( Token ) ) ) );
  architecturedict->insert(
    "TokenMap", Token( new IntegerDatum( sizeof( TokenMap ) ) ) );
  architecturedict->insert(
    "Dictionary", Token( new IntegerDatum( sizeof( Dictionary ) ) ) );
  architecturedict->insert(
    "DictionaryDatum", Token( new IntegerDatum( sizeof( DictionaryDatum ) ) ) );
  architecturedict->insert(
    "IntegerDatum", Token( new IntegerDatum( sizeof( IntegerDatum ) ) ) );
  architecturedict->insert(
    "ArrayDatum", Token( new IntegerDatum( sizeof( ArrayDatum ) ) ) );
  architecturedict->insert(
    "TokenArray", Token( new IntegerDatum( sizeof( TokenArray ) ) ) );
  architecturedict->insert(
    "TokenArrayObj", Token( new IntegerDatum( sizeof( TokenArrayObj ) ) ) );

  statusdict->insert( architecturedict_name, architecturedict );

  DictionaryDatum exitcodes( new Dictionary() );
  assert( exitcodes.valid() );

  exitcodes->insert(
    exitcode_success_name, Token( new IntegerDatum( EXIT_SUCCESS ) ) );
  exitcodes->insert(
    exitcode_skipped_name, Token( new IntegerDatum( EXITCODE_SKIPPED ) ) );
  exitcodes->insert( exitcode_skipped_no_mpi_name,
    Token( new IntegerDatum( EXITCODE_SKIPPED_NO_MPI ) ) );
  exitcodes->insert( exitcode_skipped_have_mpi_name,
    Token( new IntegerDatum( EXITCODE_SKIPPED_HAVE_MPI ) ) );
  exitcodes->insert( exitcode_skipped_no_threading_name,
    Token( new IntegerDatum( EXITCODE_SKIPPED_NO_THREADING ) ) );
  exitcodes->insert( exitcode_skipped_no_gsl_name,
    Token( new IntegerDatum( EXITCODE_SKIPPED_NO_GSL ) ) );
  exitcodes->insert( exitcode_skipped_no_music_name,
    Token( new IntegerDatum( EXITCODE_SKIPPED_NO_MUSIC ) ) );
  exitcodes->insert( exitcode_scripterror_name,
    Token( new IntegerDatum( EXITCODE_SCRIPTERROR ) ) );
  exitcodes->insert(
    exitcode_abort_name, Token( new IntegerDatum( NEST_EXITCODE_ABORT ) ) );
  exitcodes->insert(
    exitcode_userabort_name, Token( new IntegerDatum( EXITCODE_USERABORT ) ) );
  exitcodes->insert( exitcode_segfault_name,
    Token( new IntegerDatum( NEST_EXITCODE_SEGFAULT ) ) );
  exitcodes->insert(
    exitcode_exception_name, Token( new IntegerDatum( EXITCODE_EXCEPTION ) ) );
  exitcodes->insert(
    exitcode_fatal_name, Token( new IntegerDatum( EXITCODE_FATAL ) ) );
  exitcodes->insert( exitcode_unknownerror_name,
    Token( new IntegerDatum( EXITCODE_UNKNOWN_ERROR ) ) );

  statusdict->insert( exitcodes_name, exitcodes );

  // Copy environment variables
  // The environ pointer is defined at the head of the file.
  DictionaryDatum environment( new Dictionary() );
  for ( char* const* envptr = environ; *envptr; ++envptr )
  {
    std::string const envstr( *envptr );

    // It is safe to assume that all entries contain the character '='
    const size_t pos = envstr.find( '=' );
    const Name varname = envstr.substr( 0, pos );
    const std::string varvalue = envstr.substr( pos + 1 );
    environment->insert( varname, Token( new StringDatum( varvalue ) ) );
  }
  statusdict->insert( environment_name, environment );

#ifdef HAVE_LONG_LONG
  architecturedict->insert( havelonglong_name, Token( new BoolDatum( true ) ) );
  architecturedict->insert(
    longlongsize_name, Token( new IntegerDatum( sizeof( long long ) ) ) );
#else
  architecturedict->insert(
    havelonglong_name, Token( new BoolDatum( false ) ) );
#endif

  i->def( statusdict_name, statusdict );

  if ( not fname.empty() )
  {
    std::ifstream* input = new std::ifstream( fname.c_str() );
    Token input_token( new XIstreamDatum( input ) );

    i->EStack.push_move( input_token );
    i->EStack.push( i->baselookup( i->iparse_name ) );
  }

  // If we start with debug option, we set the debugging mode, but disable
  // stepmode. This way, the debugger is entered only on error.
  if ( debug_ )
  {
    i->debug_mode_on();
    i->backtrace_on();
  }
}
