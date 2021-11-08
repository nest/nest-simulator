/*
 *  interpret.h
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

#ifndef INTERPRETER_H
#define INTERPRETER_H
/*
    interpret.h defines the SLI Interpreter class
*/

// C++ includes:
#include <list>
#include <typeinfo>

// Includes from sli:
#include "sliactions.h"
#include "slibuiltins.h"
#include "slimodule.h"
#include "slitype.h"
#include "token.h"
#include "tokenstack.h"

/**
 * @defgroup SLIOutput How to notify the SLI user
 */

/**
 * @defgroup SLIMessaging Displaying messages to the user
 * @ingroup SLIOutput
 */

/**
 * @defgroup SLIError Raising (and displaying) errors
 * @ingroup SLIOutput
 */

class Parser;
class Scanner;
class DictionaryStack;
class Dictionary;
class FunctionDatum;
class BoolDatum;

extern "C" {
void SLIthrowsignal( int s );
}

class SLIInterpreter
{
  std::list< SLIModule* > modules;

  /* Flags and variables to control debugging and
   * optimizations.
   */
  bool debug_mode_;        //!< True, if SLI level debugging is enabled.
  bool show_stack_;        //!< Show stack in debug mode.
  bool show_backtrace_;    //!< Show stack-backtrace on error.
  bool catch_errors_;      //!< Enter debugger on error.
  bool opt_tailrecursion_; //!< Optimize tailing recursion.
  int call_depth_;         //!< Current depth of procedure calls.
  int max_call_depth_;     //!< Depth until which procedure calls are debugged.


  unsigned long cycle_count;
  bool cycle_guard;
  unsigned long cycle_restriction;


  int verbositylevel;
  void inittypes( void );
  void initdictionaries( void );
  void initbuiltins( void );
  void initexternals( void );

public:
  unsigned long code_accessed; // for code coverage analysis.
  unsigned long code_executed; // ration should be coverage


  Dictionary* statusdict;
  Dictionary* errordict;

  DictionaryStack* DStack;
  Parser* parse;


  // Names of basics functions
  Name ilookup_name;
  Name ipop_name;
  Name isetcallback_name;
  Name iiterate_name;
  Name iloop_name;
  Name irepeat_name;
  Name ifor_name;
  Name iforallarray_name;
  Name iforallindexedarray_name;
  Name iforallindexedstring_name;
  Name iforallstring_name;

  Name pi_name;
  Name e_name;

  Name iparse_name;
  Name stop_name;
  Name end_name;

  // Names of symbols and objects
  Name null_name;
  Name true_name;
  Name false_name;
  Name mark_name;
  Name istopped_name;
  Name systemdict_name;
  Name userdict_name;
  Name errordict_name;
  Name quitbyerror_name;
  Name newerror_name;
  Name errorname_name;
  Name commandname_name;
  Name signo_name;
  Name recordstacks_name;
  Name estack_name;
  Name ostack_name;
  Name dstack_name;
  Name commandstring_name;
  Name interpreter_name;

  // Names of basic errors
  Name ArgumentTypeError;
  Name StackUnderflowError;
  Name UndefinedNameError;
  Name WriteProtectedError;
  Name DivisionByZeroError;
  Name RangeCheckError;
  Name PositiveIntegerExpectedError;
  Name BadIOError;
  Name StringStreamExpectedError;
  Name CycleGuardError;
  Name SystemSignal;
  Name BadErrorHandler;
  Name KernelError;
  Name InternalKernelError;

  Token execbarrier_token;

  // Debug Message levels as static consts
  /** @ingroup SLIMessaging
   *  @name Predefined error levels
   *  @{
   */
  static const int M_ALL;    //!< Predefined error level for turning on
                             //!< the display of all messages;
                             //!< for use with verbosity(int).
  static const int M_DEBUG;  //!< Predefined error level for debugging messages
  static const int M_STATUS; //!< Predefined error level for status messages
  //! Predefined error level for informational messages
  static const int M_INFO;
  static const int M_DEPRECATED; //!< Predefined error level for deprecation
                                 //!< warnings
  static const int M_PROGRESS;   //!< Predefined error level for progress messages
  static const int M_WARNING;    //!< Predefined error level for warning messages
  static const int M_ERROR;      //!< Predefined error level for error messages
  static const int M_FATAL;      //!< Predefined error level for failure messages
  static const int M_QUIET;      //!< An error level above all others. Use to turn
                                 //!< off messages completely.
  /** @} */

private:
  static char const* const M_ALL_NAME;
  static char const* const M_DEBUG_NAME;
  static char const* const M_STATUS_NAME;
  static char const* const M_INFO_NAME;
  static char const* const M_PROGRESS_NAME;
  static char const* const M_DEPRECATED_NAME;
  static char const* const M_WARNING_NAME;
  static char const* const M_ERROR_NAME;
  static char const* const M_FATAL_NAME;
  static char const* const M_QUIET_NAME;

public:
  // These static members must be accessible from
  // the Datum constructors

  static SLIType Integertype;
  static SLIType Doubletype;
  static SLIType Stringtype;
  static SLIType Nametype;
  static SLIType Booltype;
  static SLIType Literaltype;
  static SLIType Arraytype;
  static SLIType Proceduretype;
  static SLIType Litproceduretype;
  static SLIType Dictionarytype;
  static SLIType Symboltype;
  static SLIType Functiontype;
  static SLIType Trietype;
  static SLIType Callbacktype;
  static SLIType Istreamtype;
  static SLIType XIstreamtype;
  static SLIType Ostreamtype;
  static SLIType IntVectortype;
  static SLIType DoubleVectortype;

  // SLIType default actions
  static DatatypeFunction datatypefunction;
  static NametypeFunction nametypefunction;
  static ProceduretypeFunction proceduretypefunction;
  static LitproceduretypeFunction litproceduretypefunction;
  static FunctiontypeFunction functiontypefunction;
  static TrietypeFunction trietypefunction;
  static CallbacktypeFunction callbacktypefunction;
  static XIstreamtypeFunction xistreamtypefunction;

  // Basic Operations needed to run the default actions
  static const IlookupFunction ilookupfunction;
  static const IsetcallbackFunction isetcallbackfunction;
  static const IiterateFunction iiteratefunction;
  static const IloopFunction iloopfunction;
  static const IrepeatFunction irepeatfunction;
  static const IforFunction iforfunction;
  static const IforallarrayFunction iforallarrayfunction;
  static const IforallindexedarrayFunction iforallindexedarrayfunction;
  static const IforallindexedstringFunction iforallindexedstringfunction;
  static const IforallstringFunction iforallstringfunction;

  // State variables of the Interpreter


  Token ct; // callback; see comments in execute(void)

  TokenStack OStack;
  TokenStack EStack;

  // public member functions:
  SLIInterpreter( void );
  ~SLIInterpreter();

  //! Initialise the interpreter by reading in the startup files.
  int startup();

  /**
   * Execute the supplied command string.
   */
  int execute( const std::string& );

  /**
   * Execute the supplied token.
   */
  int execute( const Token& );

  /**
   * Start the interpreter and run the startup code.
   */
  int execute( int v = 0 );

  /**
   * Run the interpreter with a prepared execution stack.
   * The function returns, if the execution stack has reached the given level.
   */
  int execute_( size_t exitlevel = 0 );
  int execute_debug_( size_t exitlevel = 0 );

  void createdouble( Name const&, double );
  void createcommand( Name const&, SLIFunction const*, std::string deprecation_info = std::string() );
  void createconstant( Name const&, const Token& );


  /** Lookup a name searching all dictionaries on the stack.
   *  The first occurrence is reported. If the Name is not found,
   *  @a VoidToken is returned.
   */
  const Token& lookup( const Name& n ) const;


  /** Lookup a name searching all dictionaries on the stack.
   *  The first occurrence is reported. If the Name is not found,
   *  an UndefinedName exceptiopn is thrown.
   */
  const Token& lookup2( const Name& n ) const;

  /** Lookup a name searching only the bottom level dictionary.
   *  If the Name is not found,
   *  @a VoidToken is returned.
   */
  const Token& baselookup( const Name& n ) const; // lookup in a specified

  /** Test for a name searching all dictionaries on the stack.
   */
  bool known( const Name& n ) const;

  /** Test for a name in the bottom level dictionary.
   */
  bool baseknown( const Name& n ) const;

  /** Bind a Token to a Name.
   *  The token is copied. This can be an expensive operation for large
   *  objects. Also, if the token is popped off one of the stacks after
   *  calling def, it is more reasonable to use SLIInterpreter::def_move.
   */
  void def( Name const&, Token const& );

  /** Unbind a previously bound Token from a Name.
   * Throws UnknownName Exception.
   */
  void undef( Name const& );

  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is copied.
   */
  void basedef( const Name& n, const Token& t );

  /** Bind a Token to a Name.
   *  like def, however, the Datum object is moved from the token into the
   *  dictionary, thus, no memory allocation or copying is needed.
   */
  void def_move( Name const&, Token& );

  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is moved.
   */
  void basedef_move( const Name& n, Token& t );

  void setcycleguard( Index );
  void removecycleguard( void );


  /**
   * Increment call depth level.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  void
  inc_call_depth()
  {
    ++call_depth_;
  }

  /**
   * Decrement call depth level.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  void
  dec_call_depth()
  {
    --call_depth_;
  }

  /**
   * Set call depth level to a specific value.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  void
  set_call_depth( int depth )
  {
    call_depth_ = depth;
  }

  /**
   * Return current call depth level.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  int
  get_call_depth() const
  {
    return call_depth_;
  }

  /**
   * Set maximal call depth level to a specific value.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  void
  set_max_call_depth( int d )
  {
    max_call_depth_ = d;
  }

  /**
   * Return value of maximal call depth level.
   * The value of call_depth_ is used to control
   * the step mode.
   * Step mode is disabled for call_depth_ >= max_call_depth_.
   * This gives the user the opportunity to skip over nested
   * calls during debugging.
   */
  int
  get_max_call_depth() const
  {
    return max_call_depth_;
  }

  /**
   * Returns true, if step mode is active.
   * The step mode is active in debug mode if
   * call_depth_ < max_call_depth_
   */
  bool
  step_mode() const
  {
    return debug_mode_ && ( call_depth_ < max_call_depth_ );
  }

  /**
   * Returns true, if debug mode is turned on.
   */
  bool
  get_debug_mode() const
  {
    return debug_mode_;
  }

  /**
   * Turn debug mode on.
   */
  void
  debug_mode_on()
  {
    debug_mode_ = true;
  }

  /**
   * Turn debug mode off.
   */
  void
  debug_mode_off()
  {
    debug_mode_ = false;
  }

  /**
   * Switch stack display on or off in debug mode.
   */
  void toggle_stack_display();


  /**
   * Show Debug options.
   */
  void debug_options() const;

  /**
   * Prompt user for commands during debug mode.
   * In this function, the user can enter simple commands
   * to debug code executed by the interpreter.
   */
  char debug_commandline( Token& );


  /**
   * Returns true, if tailing recursion optimization is done.
   */
  bool
  optimize_tailrecursion() const
  {
    return opt_tailrecursion_;
  }

  /**
   * Enable tail-recursion optimization.
   * Tail-recursion can be optimizes in such a way
   * that the execution stack is not growing with each
   * recursion level.
   * This optimization may improve performance for
   * applications which heavily rely on deep recusions.
   * However, during debugging, tail-recursion
   * optimization removes important information from the
   * execution stack.
   */
  void
  optimize_tailrecursion_on()
  {
    opt_tailrecursion_ = true;
  }

  /**
   * Disable tail-recursion optimization.
   * Tail-recursion can be optimizes in such a way
   * that the execution stack is not growing with each
   * recursion level.
   * This optimization may improve performance for
   * applications which heavily rely on deep recusions.
   * However, during debugging, tail-recursion
   * optimization removes important information from the
   * execution stack.
   */
  void
  optimize_tailrecursion_off()
  {
    opt_tailrecursion_ = false;
  }

  /**
   * True, if a stack backtrace should be shown on error.
   * Whenever an error or stop is raised, the execution stack is
   * unrolled up to the nearest stopped context.
   * In this process it is possible to display a stack backtrace
   * which allows the user to diagnose the origin and possible
   * cause of the error.
   * For applications which handle themselfs, this backtrace may be
   * disturbing. So it is possible to switch this behavior on and
   * off.
   */
  bool
  show_backtrace() const
  {
    return show_backtrace_;
  }

  /**
   * Switch stack backtrace on.
   * Whenever an error or stop is raised, the execution stack is
   * unrolled up to the nearest stopped context.
   * In this process it is possible to display a stack backtrace
   * which allows the user to diagnose the origin and possible
   * cause of the error.
   * For applications which handle themselfs, this backtrace may be
   * disturbing. So it is possible to switch this behavior on and
   * off.
   */
  void backtrace_on();


  /**
   * Switch stack backtrace off.
   * Whenever an error or stop is raised, the execution stack is
   * unrolled up to the nearest stopped context.
   * In this process it is possible to display a stack backtrace
   * which allows the user to diagnose the origin and possible
   * cause of the error.
   * For applications which handle themselfs, this backtrace may be
   * disturbing. So it is possible to switch this behavior on and
   * off.
   */
  void backtrace_off();


  bool
  catch_errors() const
  {
    return catch_errors_;
  }

  void
  catch_errors_on()
  {
    catch_errors_ = true;
  }

  void
  catch_errors_off()
  {
    catch_errors_ = false;
  }

  void stack_backtrace( int n );

  /** Cause the SLI interpreter to raise an error.
   *  This function is used by classes derived from SLIFunction to raise
   *  an error.
   *  \n
   *  raiseerror() is an interface to the SLI interpreter's error
   *  handling mechanism (see The Red Book for details). If an error
   *  is raised, the following actions are performed:
   *  - the value of errordict /newerror is set to true
   *  - the value of errordict /command is set to the name of the command
   *    which raised the error
   *  - If the value of errordict /recorstack is true,
   *    the state of the interpreter is saved:
   *    - the operand stack is copied to errordict /ostack
   *    - the execution stack is copied to errordict /estack
   *    - the dictionary stack is copied to errordict /dstack
   *  - the dictionary stack is cleared.
   *  - stop is called. Stop then tries to find an enclosing stopped
   *    context and calls the associated function.
   *
   *  This mechanism is explained in detail in The PostScript Reference Manual.
   *  \n
   *  If the user did not establish any stopped context, the default
   *  stopped context for the SLI interpreter will be executed, which
   *  includes display of an error message and stopping program
   *  execution.
   *  \n
   *  Please note that before raiserror() is called, the state of the
   *  operand and execution stack shall be restored to their initial
   *  state.
   *
   *  @param err  The argument is the name of the error, specified as
   *  a string.
   *  The name of the currently active function will be used as the
   *  function name.
   *
   *  @ingroup SLIError
   *  @see raiseerror(Name),
   *  raiseerror(Name,Name), raiseagain()
   */
  void
  raiseerror( const char* err )
  {
    raiseerror( Name( err ) );
  }

  /** Cause the SLI interpreter to raise an error.
   *  This function is used by classes derived from SLIFunction to raise
   *  an error.
   *  \n
   *  raiseerror() is an interface to the SLI interpreter's error
   *  handling mechanism (see The Red Book for details). If an error
   *  is raised, the following actions are performed:
   *  - the value of errordict /newerror is set to true
   *  - the value of errordict /command is set to the name of the command
   *    which raised the error
   *  - If the value of errordict /recorstack is true,
   *    the state of the interpreter is saved:
   *    - the operand stack is copied to errordict /ostack
   *    - the execution stack is copied to errordict /estack
   *    - the dictionary stack is copied to errordict /dstack
   *  - the dictionary stack is cleared.
   *  - stop is called. Stop then tries to find an enclosing stopped
   *    context and calls the associated function.
   *
   *  This mechanism is explained in detail in The PostScript Reference Manual.
   *  \n
   *  If the user did not establish any stopped context, the default
   *  stopped context for the SLI interpreter will be executed, which
   *  includes display of an error message and stopping program
   *  execution.
   *  \n
   *  Please note that before raiserror() is called, the state of the
   *  operand and execution stack shall be restored to their initial
   *  state.
   *
   *  @param err  The argument is the name of the error.
   *  For conveniency, there is also a variant of this function that takes a
   *  string as the argument.
   *
   *  @ingroup SLIError
   *  @see raiseerror(const char*),
   *  raiseerror(Name,Name), raiseagain()
   */
  void raiseerror( Name err );

  /**
   * Handle exceptions thrown by any execute().
   * This raiseerror is the first step in handling C++ exceptions
   * thrown by an execute() call. In particular,
   * - the name of the calling function is recorded;
   * - the command is popped from the execution stack;
   * - the error message is extracted from those exceptions that are
   *   derived from SLIException.
   * - handling is forwarded to raiserror(Name, Name).
   */
  void raiseerror( std::exception& err );

  /** Cause the SLI interpreter to raise an error.
   *  This function is used by classes derived from SLIFunction to raise
   *  an error.
   *  \n
   *  raiseerror() is an interface to the SLI interpreter's error
   *  handling mechanism (see The Red Book for details). If an error
   *  is raised, the following actions are performed:
   *  - the value of errordict /newerror is set to true
   *  - the value of errordict /command is set to the name of the command
   *    which raised the error
   *  - If the value of errordict /recorstack is true,
   *    the state of the interpreter is saved:
   *    - the operand stack is copied to errordict /ostack
   *    - the execution stack is copied to errordict /estack
   *    - the dictionary stack is copied to errordict /dstack
   *  - the dictionary stack is cleared.
   *  - stop is called. Stop then tries to find an enclosing stopped
   *    context and calls the associated function.
   *
   *  This mechanism is explained in detail in The PostScript Reference Manual.
   *  \n
   *  If the user did not establish any stopped context, the default
   *  stopped context for the SLI interpreter will be executed, which
   *  includes display of an error message and stopping program
   *  execution.
   *  \n
   *  Please note that before raiserror() is called, the state of the
   *  operand and execution stack shall be restored to their initial
   *  state.
   *
   *  @param cmd  The first argument is the name of the calling function.
   *  @param err  The second argument is the name of the error.
   *
   *  @ingroup SLIError
   *  @see raiseerror(const char*), raiseerror(Name),
   *  raiseagain()
   */
  void raiseerror( Name cmd, Name err );

  /** Print a description of a raised error.
   *  The errordict members errorname, command and message together
   *  with the function input parameters decides the nature of the
   *  output message. The function use the message() function to
   *  print the error.
   *  Replaces the SLI :print_error function.
   *
   *  @param cmd  The name of the function that raised the error.
   *
   *  @see raiseerror(const char*), raiseerror(Name),
   *  raiseerror(Name,Name)
   */
  void print_error( Token cmd );

  /** Re-raise the last error.
   *  raiseagain re-raises a previously raised error. This is useful
   *  if an error handler cannot cope with a particular error (e.g. a signal)
   *  and wants to pass it to an upper level handler. Thus, nestet error
   *  handlers are possible.
   *
   *  @ingroup SLIError
   *  @see raiseerror(const char*), raiseerror(Name),
   *  raiseerror(Name,Name)
   */
  void raiseagain( void );

  /** TO BE DOCUMENTED.
   *  @todo Document this function.
   *
   *  @ingroup SLIError
   */
  void raisesignal( int );


  // Message loging mechanism

  /** Set the verbosity level of the SLI messaging mechanism.
   *  Only messages having an error level that is equal to or greater
   *  than this level will be displayed by the interpreter.
   *
   *  @see verbosity(void), message(), debug(), status(), info(), warning(),
   *  error(), fatal()
   *  @ingroup SLIMessaging
   */
  void verbosity( int );

  /** Retrieve the current verbosity level of the SLI messaging mechanism.
   *  Only messages having an error level that is equal to or greater
   *  than this level will be displayed by the interpreter.
   *  \n
   *  You may use any positive integer here. For conveniency,
   *  there exist five predifined error levels:  \n
   *  SLIInterpreter::M_ALL=0,  display all messages \n
   *  SLIInterpreter::M_DEBUG=5,  display debugging messages and above \n
   *  SLIInterpreter::M_STATUS=7,  display status messages and above \n
   *  SLIInterpreter::M_INFO=10, display information messages and above \n
   *  SLIInterpreter::M_PROGRESS=15, display test-related messages and above \n
   *  SLIInterpreter::M_DEPRECATED=18, display deprecation warnings and above \n
   *  SLIInterpreter::M_WARNING=20, display warning messages and above \n
   *  SLIInterpreter::M_ERROR=30, display error messages and above \n
   *  SLIInterpreter::M_FATAL=40, display failure messages and above \n
   *  SLIInterpreter::M_QUIET=100, suppress all messages \n
   *  Thus, by calling verbosity(SLIInterpreter::M_WARNING) you
   *  indicate that you are interested in seeing error messages and
   *  more important messages only.
   *
   *  @see verbosity(void), message(), debug(), status(), info(), warning(),
   *  error(), fatal()
   *  @ingroup SLIMessaging
   */
  int verbosity( void ) const;

  /** Display a message.
   *  @param level  The error level that shall be associated with the
   *  message. You may use any positive integer here. For convenience,
   *  there exist five predefined error levels:  \n
   * (SLIInterpreter::M_ALL=0, for use with verbosity(int) only, see there), \n
   *  SLIInterpreter::M_DEBUG=5, a debugging message \n
   *  SLIInterpreter::M_STATUS=7, a status message \n
   *  SLIInterpreter::M_INFO=10, an informational message \n
   *  SLIInterpreter::M_PROGRESS=15, a test-related message \n
   *  SLIInterpreter::M_DEPRECATED=18, a deprecation warning \n
   *  SLIInterpreter::M_WARNING=20, a warning message \n
   *  SLIInterpreter::M_ERROR=30, an error message \n
   *  SLIInterpreter::M_FATAL=40, a failure message. \n
   * (SLIInterpreter::M_QUIET=100, for use with verbosity(int) only, see there),
   *
   *  @param from   A string specifying the name of the function that
   *  sends the message.
   *  @param test   A string specifying the message text.
   *
   *  The message will ony be displayed if the current verbosity level
   *  is greater than or equal to the specified level.
   *  \n
   *  If two or more messages are issued after each other, that have
   *  the same <I>from</I> and <I>level</I> argument, the messages will
   *  be grouped together in the output.
   *
   *  @see verbosity(void), verbosity(int)
   *  @ingroup SLIMessaging
   */
  void message( int level, const char from[], const char text[], const char errorname[] = "" ) const;

  /** Function used by the message(int, const char*, const char*) function.
   *  Prints a message to the specified output stream.
   *  @param out        output stream
   *  @param levelname  name associated with input level
   */
  void message( std::ostream& out,
    const char levelname[],
    const char from[],
    const char text[],
    const char errorname[] = "" ) const;

  void terminate( int returnvalue = -1 );

  //*******************************************************
  Name getcurrentname( void ) const;

  unsigned long
  cycles( void ) const
  {
    return cycle_count;
  }


  template < class T >
  void addmodule( void );
  void addmodule( SLIModule* );

  /*
   * Add a linked user module to the interpreter.
   * Initializers (commandstrings) for linked dynamic modules are executed
   * by sli-init.sli after all C++ initialization is done.
   * Do not use this for modules loaded at runtime!
   */
  void addlinkedusermodule( SLIModule* );

  FunctionDatum* Ilookup( void ) const;
  FunctionDatum* Iiterate( void ) const;

  /**
   * Throw StackUnderflow exception if too few elements on stack.
   * @param n Minimum number of elements required on stack.
   * @throw StackUnderflow if fewer that @c n elements on stack.
   */
  void assert_stack_load( size_t n );
};

// This function template is a workaround for the parameterless
// template member function, given below. As of
// egcs-2.91.16 980328 (gcc-2.8.0 release)
// the compiler is not able to parse a call like
// engine.addmodule<ModuleX>();
// (Stroustrup97), Sec 13.3.1 (p335)

template < class T >
void
addmodule( SLIInterpreter& i )
{
  i.addmodule( new T );
}

template < class T >
void
SLIInterpreter::addmodule( void )
{
  SLIModule* m = new T();

  modules.push_back( m );
  m->install( std::cout, this );
}

inline void
SLIInterpreter::assert_stack_load( size_t n )
{
  if ( OStack.load() < n )
  {
    throw StackUnderflow( n, OStack.load() );
  }
}


#endif
