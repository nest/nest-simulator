/*
 *  processes.cc
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

#include "processes.h"

// C includes:
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// C++ includes:
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dict.h"      // for TokenMap
#include "dictdatum.h" // for signaldict
#include "dictutils.h"
#include "fdstream.h"
#include "integerdatum.h" // Include the data-types we use!
#include "interpret.h"    // for SLI Interpreter and messaging mechanism
#include "iostreamdatum.h"
#include "namedatum.h"
#include "sliexceptions.h"
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

#ifdef HAVE_ALPHA_CXX_STD_BUG
#undef __PURE_CNAME
#include <cerrno>
#define __PURE_CNAME
#else
#include <cerrno>
#endif

#ifndef _POSIX_SOURCE
#define _SYNOD__SET_POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q
extern "C" {
// These functions are defined in the file "get_mem.c". They need
// to reside in a plain C file, because the #pragmas defined in the
// BG header files interfere with C++, causing "undefined reference
// to non-virtual thunk" MH 12-02-22, redid fix by JME 12-01-27.
long bg_get_heap_mem();
long bg_get_stack_mem();
long bg_get_mmap_mem();
}
#endif

#if defined __APPLE__ && defined HAVE_MACH_MACH_H
extern "C" {
// Similar to the above prototype definitions for BG.
unsigned long darwin_get_used_mem();
}
#endif

// definition of static variables and functions declared in processes.h:
pid_t Processes::children_group = 0;

// The following concernes the new module: -----------------------

// This will be used to produce systemerror-messages

const std::string
Processes::systemerror( SLIInterpreter* i )
{
  Token errordict_t( i->baselookup( i->errordict_name ) );
  assert( errordict_t.datum() != NULL );
  DictionaryDatum errordict_d = *dynamic_cast< DictionaryDatum* >( errordict_t.datum() );

  std::string ErrorMessage( std::strerror( errno ) );

  errordict_d->insert( Name( "sys_errname" ), new LiteralDatum( ErrorMessage ) );
  errordict_d->insert( Name( "sys_errno" ), new IntegerDatum( errno ) );

  return "SystemError";
}

int
Processes::fd( std::istream* s )
{
  if ( s == &std::cin )
  {
    return STDIN_FILENO;
  }
  else
  {
    ifdstream* fs = dynamic_cast< ifdstream* >( s );
    assert( fs != NULL );
    return fs->rdbuf()->fd();
  }
}

int
Processes::fd( std::ostream* s )
{
  if ( s == &std::cout )
  {
    return STDOUT_FILENO;
  }
  else if ( ( s == &std::cerr ) || ( s == &std::clog ) )
  {
    return STDERR_FILENO;
  }
  else
  {
    ofdstream* fs = dynamic_cast< ofdstream* >( s );
    assert( fs != NULL );
    return fs->rdbuf()->fd();
  }
}

// end of definition of static variables and functions

const std::string
Processes::name( void ) const
{
  return std::string( "basic process management" ); // Return name of the module
}

const std::string
Processes::commandstring( void ) const
{
  return std::string( "(processes.sli) run" );
}

void
Processes::init( SLIInterpreter* i )
{
  // Create Dictionary "signaldict", which will contain the system's signal
  // values:
  Dictionary* signaldict = new Dictionary; // get a new dictionary from the heap

  // There is a typeconversion operator Datum<->Token !
  signaldict->insert( SIGABRT_name, new IntegerDatum( SIGABRT ) );
  signaldict->insert( SIGALRM_name, new IntegerDatum( SIGALRM ) );
  signaldict->insert( SIGFPE_name, new IntegerDatum( SIGFPE ) );
  signaldict->insert( SIGHUP_name, new IntegerDatum( SIGHUP ) );
  signaldict->insert( SIGILL_name, new IntegerDatum( SIGILL ) );
  signaldict->insert( SIGINT_name, new IntegerDatum( SIGINT ) );
  signaldict->insert( SIGKILL_name, new IntegerDatum( SIGKILL ) );
  signaldict->insert( SIGPIPE_name, new IntegerDatum( SIGPIPE ) );
  signaldict->insert( SIGQUIT_name, new IntegerDatum( SIGQUIT ) );
  signaldict->insert( SIGSEGV_name, new IntegerDatum( SIGSEGV ) );
  signaldict->insert( SIGTERM_name, new IntegerDatum( SIGTERM ) );
  signaldict->insert( SIGUSR1_name, new IntegerDatum( SIGUSR1 ) );
  signaldict->insert( SIGUSR2_name, new IntegerDatum( SIGUSR2 ) );

  signaldict->insert( SIGCHLD_name, new IntegerDatum( SIGCHLD ) );
  signaldict->insert( SIGCONT_name, new IntegerDatum( SIGCONT ) );
  signaldict->insert( SIGSTOP_name, new IntegerDatum( SIGSTOP ) );
  signaldict->insert( SIGTSTP_name, new IntegerDatum( SIGTSTP ) );
  signaldict->insert( SIGTTIN_name, new IntegerDatum( SIGTTIN ) );
  signaldict->insert( SIGTTOU_name, new IntegerDatum( SIGTTOU ) );

  i->def( signaldict_name, new DictionaryDatum( signaldict ) );
  // DictionaryDatum(signaldict) makes a lockPTR from the ordinary pointer.
  // The datum stored in the signaldict-token will thus be a lockPTR to a
  // dictionary

  // create variables "sys_errname" and "sys_errno"
  //  and all needed errornumbers in errordict
  Token errordict_t( i->baselookup( i->errordict_name ) );
  assert( errordict_t.datum() != NULL );
  DictionaryDatum errordict_d = *dynamic_cast< DictionaryDatum* >( errordict_t.datum() );

  errordict_d->insert( sys_errname, new LiteralDatum( "" ) );
  errordict_d->insert( sys_errno, new IntegerDatum( 0 ) );

  errordict_d->insert( E2BIG_name, new IntegerDatum( E2BIG ) );
  errordict_d->insert( EACCES_name, new IntegerDatum( EACCES ) );
  errordict_d->insert( EAGAIN_name, new IntegerDatum( EAGAIN ) );
  errordict_d->insert( EBADF_name, new IntegerDatum( EBADF ) );
  errordict_d->insert( EBUSY_name, new IntegerDatum( EBUSY ) );
  errordict_d->insert( ECHILD_name, new IntegerDatum( ECHILD ) );
  errordict_d->insert( EDEADLK_name, new IntegerDatum( EDEADLK ) );
  errordict_d->insert( EDOM_name, new IntegerDatum( EDOM ) );
  errordict_d->insert( EEXIST_name, new IntegerDatum( EEXIST ) );
  errordict_d->insert( EFAULT_name, new IntegerDatum( EFAULT ) );
  errordict_d->insert( EFBIG_name, new IntegerDatum( EFBIG ) );
  errordict_d->insert( EINTR_name, new IntegerDatum( EINTR ) );
  errordict_d->insert( EINVAL_name, new IntegerDatum( EINVAL ) );
  errordict_d->insert( EIO_name, new IntegerDatum( EIO ) );
  errordict_d->insert( EISDIR_name, new IntegerDatum( EISDIR ) );
  errordict_d->insert( EMFILE_name, new IntegerDatum( EMFILE ) );
  errordict_d->insert( EMLINK_name, new IntegerDatum( EMLINK ) );
  errordict_d->insert( ENAMETOOLONG_name, new IntegerDatum( ENAMETOOLONG ) );
  errordict_d->insert( ENFILE_name, new IntegerDatum( ENFILE ) );
  errordict_d->insert( ENODEV_name, new IntegerDatum( ENODEV ) );
  errordict_d->insert( ENOENT_name, new IntegerDatum( ENOENT ) );
  errordict_d->insert( ENOEXEC_name, new IntegerDatum( ENOEXEC ) );
  errordict_d->insert( ENOLCK_name, new IntegerDatum( ENOLCK ) );
  errordict_d->insert( ENOMEM_name, new IntegerDatum( ENOMEM ) );
  errordict_d->insert( ENOSPC_name, new IntegerDatum( ENOSPC ) );
  errordict_d->insert( ENOSYS_name, new IntegerDatum( ENOSYS ) );
  errordict_d->insert( ENOTDIR_name, new IntegerDatum( ENOTDIR ) );
  errordict_d->insert( ENOTEMPTY_name, new IntegerDatum( ENOTEMPTY ) );
  errordict_d->insert( ENOTTY_name, new IntegerDatum( ENOTTY ) );
  errordict_d->insert( ENXIO_name, new IntegerDatum( ENXIO ) );
  errordict_d->insert( EPERM_name, new IntegerDatum( EPERM ) );
  errordict_d->insert( EPIPE_name, new IntegerDatum( EPIPE ) );
  errordict_d->insert( ERANGE_name, new IntegerDatum( ERANGE ) );
  errordict_d->insert( EROFS_name, new IntegerDatum( EROFS ) );
  errordict_d->insert( ESPIPE_name, new IntegerDatum( ESPIPE ) );
  errordict_d->insert( ESRCH_name, new IntegerDatum( ESRCH ) );
  errordict_d->insert( EXDEV_name, new IntegerDatum( EXDEV ) );


  // ...don't forget to create the new SLI-commands!
  i->createcommand( "fork", &forkfunction );
  i->createcommand( "sysexec_a", &sysexec_afunction );
  i->createcommand( "waitPID", &waitPIDfunction );
  i->createcommand( "kill", &killfunction );
  i->createcommand( "pipe", &pipefunction );
  i->createcommand( "dup2_is_is", &dup2_is_isfunction );
  i->createcommand( "dup2_os_os", &dup2_os_osfunction );
  i->createcommand( "dup2_is_os", &dup2_is_osfunction );
  i->createcommand( "dup2_os_is", &dup2_os_isfunction );
  i->createcommand( "available", &availablefunction );
  i->createcommand( "getPID", &getpidfunction );
  i->createcommand( "getPPID", &getppidfunction );
  i->createcommand( "getPGRP", &getpgrpfunction );
  i->createcommand( "mkfifo", &mkfifofunction );
#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q
  i->createcommand( ":memory_thisjob_bg", &memorythisjobbgfunction );
#endif
#if defined __APPLE__ && defined HAVE_MACH_MACH_H
  i->createcommand( ":memory_thisjob_darwin", &memorythisjobdarwinfunction );
#endif
  i->createcommand( "setNONBLOCK", &setnonblockfunction );
  i->createcommand( "ctermid", &ctermidfunction );
  i->createcommand( "isatty_os", &isatty_osfunction );
  i->createcommand( "isatty_is", &isatty_isfunction );
}

Processes::~Processes()
{
}


// ---------------------------------------------------------------


// The following concernes the new command(s): -------------------

void
Processes::ForkFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  pid_t pid;
  pid = fork();
  if ( pid < 0 )
  {
    i->raiseerror( systemerror( i ) );
  }
  else
  {
    if ( pid != 0 )
    { // I am the parent. pid is the PID of the new child.
#ifdef HAVE_SSTREAM
      std::stringstream s;
      s << "Child PID: " << pid << "\n";
      i->message( SLIInterpreter::M_DEBUG, "fork", s.str().c_str() );
#endif
      //           if (Processes::children_group == 0)
      //             {
      //               std::cerr << "Parent: Creating and putting child into new
      //               process group ";
      //               int result = setpnode_id(pid,pid);
      //               if (result < 0) i->raiseerror(systemerror(i));
      //               Processes::children_group = pid;
      //               std::cerr << Processes::children_group << std::endl;
      //             }
      //           else
      //             {
      //               std::cerr << "Parent: Putting child into process group "
      //               <<
      //               Processes::children_group << std::endl;
      //               int result = setpnode_id(pid,Processes::children_group);
      //               if (result < 0) i->raiseerror(systemerror(i));
      //             }
    }
    else // for the child
    {
      // In case we are in debug_mode, we need to switch it off
      // Otherwise the debug prompt will disturb further processing.
      i->debug_mode_off();
    }

    i->EStack.pop(); // Don't forget to pop yourself...

    Token result_token( new IntegerDatum( pid ) ); // Make Token, containing
                                                   // IntegerDatum, which is
                                                   // initialized to pid;
    i->OStack.push_move( result_token );
  }
}

void
Processes::Sysexec_aFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() != 0 );

  Token array_token;
  i->OStack.pop_move( array_token ); // move topmost Token into namearray_token

  // this is an array of tokens (to names)
  ArrayDatum* array = dynamic_cast< ArrayDatum* >( array_token.datum() );
  assert( array != NULL );

  assert( array->size() > 0 ); // need at least the commandname

  // The following array is needed to supply the function execvp with a C
  // string array. Thus, the supplied array of SLI strings must be converted.
  // argv is deleted right after the call to execvp(command, const_cast<char *
  // const *> (argv) );
  //
  // **argv denotes an pointer to an array which is allocated dynamically
  // the old formulation char *argv[array->size() + 1]; is no longer legal c++
  // (Ruediger!!)
  char** argv = new char* [ array->size() + 1 ];

  for ( unsigned int j = 0; j < array->size(); j++ ) // forall in array
  {
    StringDatum* nd = dynamic_cast< StringDatum* >( ( *array )[ j ].datum() );
    assert( nd != NULL );
    // StringDatum is derived from class string.
    argv[ j ] = const_cast< char* >( nd->c_str() );
  }

  char* command = argv[ 0 ];
  argv[ array->size() ] = NULL;
  int result = execvp( command, argv );
  //  int result = execvp(command, const_cast<char * const *> (argv) );
  delete[] argv;

  if ( result == -1 )
  {                                     // an error occured!
    i->OStack.push_move( array_token ); // restore operand stack
    i->raiseerror( systemerror( i ) );
  }

} // auto-destroy namearray_token and free its datum.

void
Processes::WaitPIDFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:

  assert( i->OStack.load() >= 2 ); // waitPID takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  IntegerDatum* pidin_d = dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  assert( pidin_d != NULL );

  BoolDatum* nohangflag_d = dynamic_cast< BoolDatum* >( i->OStack.top().datum() );
  assert( nohangflag_d != NULL );

  // call waitpid()
  int stat_value;
  int options = 0;
  if ( *nohangflag_d )
  {
    options = WNOHANG;
  }
  pid_t pidout = waitpid( pidin_d->get(), &stat_value, options );

  // Check for error
  if ( pidout == -1 ) // an Error occured
  {
    i->raiseerror( systemerror( i ) );
  }
  else if ( pidout == 0 ) // NoHangFlag was set, and no information was ready
  {
    i->EStack.pop();
    i->OStack.pop();
    i->OStack.pop();     // We will oly leave one result on stack: 0
    i->OStack.push( 0 ); // Push 0 on stack.
  }
  else // child exited
  {
    // push result
    Token pidout_t( new IntegerDatum( pidout ) ); // Make Token, containing
                                                  // IntegerDatum, which is
                                                  // initialized to pidout;
    // Push on stack by moving the contents of this token.
    i->OStack.push_move( pidout_t );
    // Ostack is now: pidin(int) nohangflag(bool) pidout(int)
    // first 2 Tokens will be reused: status(int) normalexitflag(bool)
    // pidout(int)
    // This is meant to produce clearity, not confusion!
    IntegerDatum* status_d = pidin_d;
    BoolDatum* normalexitflag_d = nohangflag_d; // just a renaming of variables!

    // check status
    if ( WIFEXITED( stat_value ) ) // child exited normally
    {
      i->EStack.pop();
      ( *normalexitflag_d ) = true;
      ( *status_d ) = WEXITSTATUS( stat_value ); // return exit status
    }
    else if ( WIFSIGNALED( stat_value ) ) // child terminated due to a signal
                                          // that was not caught
    {
      i->EStack.pop();
      ( *normalexitflag_d ) = false;
      ( *status_d ) = WTERMSIG( stat_value ); // return number of terminating signal
    }
    else
    {
      i->OStack.pop(); // restore OStack before raising an error
      i->raiseerror( "UnhandledExitOfChild" );
    }
  }
}


void
Processes::KillFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 2 ); // kill takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  IntegerDatum* pid_d = dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  assert( pid_d != NULL );

  IntegerDatum* signal_d = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( signal_d != NULL );

  // call kill()
  int result = kill( pid_d->get(), signal_d->get() );

  if ( result == -1 )
  { // an error occured!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                     // no error
    i->EStack.pop();    // pop command from execution stack
    i->OStack.pop( 2 ); // pop arguments from operand stack
  }
}


void
Processes::PipeFunction::execute( SLIInterpreter* i ) const
{
  // call pipe()
  int filedes[ 2 ];
  int result = pipe( filedes );

  if ( result == -1 )
  { // an error occured!
    i->raiseerror( systemerror( i ) );
  }
  else
  { // no error
    ifdstream* in = new ifdstream( filedes[ 0 ] );
    ofdstream* out = new ofdstream( filedes[ 1 ] );

    Token in_t( new IstreamDatum( in ) );
    Token out_t( new OstreamDatum( out ) );

    i->OStack.push_move( in_t );
    i->OStack.push_move( out_t );

    i->EStack.pop(); // pop command from execution stack
  }
}


void
Processes::Dup2_is_isFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 2 ); // dup2 takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  IstreamDatum* s_d1 = dynamic_cast< IstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( s_d1 != NULL );
  IstreamDatum* s_d2 = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );
  assert( s_d2 != NULL );

  // call dup2();
  // int result = dup2( fd(s_d1->get()) , fd(s_d2->get()) );//using get() on a
  // LockPTR will lock the PointerObject!
  // This would result in a failed assertion at the next operation on that Datum
  // (if we don not unlock it explicitly again)
  // so we use operator* instead
  int result = dup2( fd( **s_d1 ), fd( **s_d2 ) );
  // LockPTRs can be used like ordinary pointers!!! (*s_d1 is a LockPTR on an
  // istream)

  if ( result == -1 )
  { // an error occured!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                     // no error
    i->EStack.pop();    // pop command from execution stack
    i->OStack.pop( 2 ); // pop operands from operand stack
  }
}


void
Processes::Dup2_os_osFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 2 ); // dup2 takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  OstreamDatum* s_d1 = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( s_d1 != NULL );
  OstreamDatum* s_d2 = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );
  assert( s_d2 != NULL );

  // call dup2();
  // for comments on LockPTRs see Dup2_is_isFunction::execute
  int result = dup2( fd( **s_d1 ), fd( **s_d2 ) );

  if ( result == -1 )
  { // an error occurred!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                     // no error
    i->EStack.pop();    // pop command from execution stack
    i->OStack.pop( 2 ); // pop operands from operand stack
  }
}


void
Processes::Dup2_is_osFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 2 ); // dup2 takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  IstreamDatum* s_d1 = dynamic_cast< IstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( s_d1 != NULL );
  OstreamDatum* s_d2 = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );
  assert( s_d2 != NULL );

  // call dup2();
  // int result = dup2( fd(s_d1->get()) , fd(s_d2->get()) );//using get() on a
  // LockPTR will lock the PointerObject!
  // This would result in a failed assertion at the next operation on that Datum
  // (if we don not unlock it explicitly again)
  // so we use operator* instead
  int result = dup2( fd( **s_d1 ), fd( **s_d2 ) );
  // LockPTRs can be used like ordinary pointers!!! (*s_d1 is a LockPTR on an
  // istream)

  if ( result == -1 )
  { // an error occurred!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                     // no error
    i->EStack.pop();    // pop command from execution stack
    i->OStack.pop( 2 ); // pop operands from operand stack
  }
}


void
Processes::Dup2_os_isFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 2 ); // dup2 takes 2 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  OstreamDatum* s_d1 = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( s_d1 != NULL );
  IstreamDatum* s_d2 = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );
  assert( s_d2 != NULL );

  // call dup2();
  // for comments on LockPTRs see Dup2_is_isFunction::execute
  int result = dup2( fd( **s_d1 ), fd( **s_d2 ) );

  if ( result == -1 )
  { // an error occurred!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                     // no error
    i->EStack.pop();    // pop command from execution stack
    i->OStack.pop( 2 ); // pop operands from operand stack
  }
}


void
Processes::AvailableFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 ); // available takes 1 argument

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.top().datum() );

  assert( istreamdatum != 0 );
  assert( istreamdatum->valid() );

  if ( not( **istreamdatum ).good() )
  { // istream not good. Do nothing. Return false.
    i->EStack.pop();
    i->OStack.push( false );
  }
  else
  { // istream is good. Try to read from it.
    // istreamdatum is a pointer to a LockPTR

    // get filedescriptor: -
    int fd = Processes::fd( **istreamdatum ); // Get FileDescriptor
    //-------------------------------

    // Set Non-Blocking-Flag on this fd:
    int flags = fcntl( fd, F_GETFL );
    fcntl( fd, F_SETFL, flags | O_NONBLOCK );
    // ------------------------------

    // Start read attempt on this FILE:


    //   ---- Version 1: using errno to detect errors - not portable?? ----

    //       int peekchar = (**istreamdatum).peek();
    //       // Reset Fileflags --------------
    //       fcntl(fd,F_SETFL,flags); //reset to old value
    //       // ------------------------------

    //       if ( (peekchar==-1) && (errno!=EAGAIN) && (errno!=ESPIPE) )
    //         {// some unexpected error occured!
    //           i->raiseerror(systemerror(i));
    //         }
    //       else
    //         {// no error or EAGAIN or ESPIPE occured
    //           i->EStack.pop();
    //           bool result;
    //           if ( peekchar==-1 ) // errno==EAGAIN or errno==ESPIPE
    //             {// no data is currently available
    //               result = false; // no data is available
    //               (**istreamdatum).clear(); // Lower eof and error Flag
    //             }
    //           else
    //             {// the read attempt was successful.
    //               result = true; // data can be read
    //             }
    //           // leave result on OStack ---------
    //           if (result == true)
    //             {
    //               Token result_t = new BoolDatum( i->true_name);
    //               i->OStack.push_move(result_t);
    //             }
    //           else
    //             {
    //               Token result_t = new BoolDatum( i->false_name);
    //               i->OStack.push_move(result_t);
    //             }
    //         }
    //     }
    // ----- End Version 1 ----

    // ---- Version 2: Using .good() to detect errors. Pure C++ (apart from
    // fcntl) ------

    ( **istreamdatum ).peek();
    // Reset Fileflags --------------
    fcntl( fd, F_SETFL, flags ); // reset to old value
    // ------------------------------

    bool result;
    if ( not( **istreamdatum ).good() )
    { // an error occured. No data can be read.
      // no data is currently available
      result = false;             // no data is available
      ( **istreamdatum ).clear(); // Lower eof and error Flag
    }
    else
    {                // the read attempt was successful.
      result = true; // data can be read
    }

    // leave result on OStack ---------
    i->EStack.pop();
    i->OStack.push( result );
  }
  // ----- End Version 2 ----
}

// ---------------------------------------------------------------

void
Processes::GetPIDFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  pid_t pid;
  pid = getpid();
  if ( pid < 0 )
  {
    i->raiseerror( systemerror( i ) );
  }
  else
  {
    i->EStack.pop(); // Don't forget to pop yourself...

    Token result_token( new IntegerDatum( pid ) ); // Make Token, containing
                                                   // IntegerDatum, which is
                                                   // initialized to pid;
    i->OStack.push_move( result_token );
  }
}

void
Processes::GetPPIDFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  pid_t ppid;
  ppid = getppid();
  if ( ppid < 0 )
  {
    i->raiseerror( systemerror( i ) );
  }
  else
  {
    i->EStack.pop(); // Don't forget to pop yourself...

    Token result_token( new IntegerDatum( ppid ) ); // Make Token, containing
                                                    // IntegerDatum, which is
                                                    // initialized to ppid;
    i->OStack.push_move( result_token );
  }
}


void
Processes::GetPGRPFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  pid_t pgrp;
  pgrp = getpgrp();
  if ( pgrp < 0 )
  {
    i->raiseerror( systemerror( i ) );
  }
  else
  {
    i->EStack.pop(); // Don't forget to pop yourself...

    Token result_token( new IntegerDatum( pgrp ) ); // Make Token, containing
                                                    // IntegerDatum, which is
                                                    // initialized to pgrp;
    i->OStack.push_move( result_token );
  }
}

void
Processes::MkfifoFunction::execute( SLIInterpreter* i ) const
{
  // This is what happens, when the SLI-command is called:
  assert( i->OStack.load() >= 1 ); // mkfifo takes 1 arguments

  // Read arguments from operand Stack, but leave tokens on stack:
  StringDatum* s_d = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( s_d != NULL );

  // call mkfifo();
  mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // Try to give all permissions,
                                             // modified only by the user`s
                                             // umask.
  // StringDatum is derived from string
  int result = mkfifo( s_d->c_str(), mode );

  if ( result == -1 )
  { // an error occurred!
    i->raiseerror( systemerror( i ) );
  }
  else
  {                  // no error
    i->EStack.pop(); // pop command from execution stack
    i->OStack.pop(); // pop operand from operand stack
  }
}

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q
/** @BeginDocumentation
 Name: memory_thisjob_bg - Reports memory usage on Blue Gene/P/Q systems
 Description:
 memory_thisjob_bg returns a dictionary with the heap and stack memory
 usage of a process in Bytes.
 Availability: Processes
 Author: Jochen Martin Eppler
 */
void
Processes::MemoryThisjobBgFunction::execute( SLIInterpreter* i ) const
{
  DictionaryDatum dict( new Dictionary );

  unsigned long heap_memory = bg_get_heap_mem();
  ( *dict )[ "heap" ] = heap_memory;
  unsigned long stack_memory = bg_get_stack_mem();
  ( *dict )[ "stack" ] = stack_memory;
  unsigned long mmap_memory = bg_get_mmap_mem();
  ( *dict )[ "mmap" ] = mmap_memory;

  i->OStack.push( dict );
  i->EStack.pop();
}
#endif

#if defined __APPLE__ && defined HAVE_MACH_MACH_H
/** @BeginDocumentation
 Name: memory_thisjob_darwin - Reports memory usage on Darwin/Apple systems
 Description:
 memory_thisjob_darwin returns the resident memory usage of a process in Bytes.
 Availability: Processes
 Author: Tammo Ippen
 */
void
Processes::MemoryThisjobDarwinFunction::execute( SLIInterpreter* i ) const
{
  unsigned long resident_memory = darwin_get_used_mem();

  i->OStack.push( resident_memory );
  i->EStack.pop();
}
#endif

void
Processes::SetNonblockFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 ); // setNONBLOCK takes 2 arguments

  IstreamDatum* istreamdatum = dynamic_cast< IstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( istreamdatum != 0 );
  assert( istreamdatum->valid() );

  BoolDatum* newflag_d = dynamic_cast< BoolDatum* >( i->OStack.top().datum() );
  assert( newflag_d != NULL );

  // get filedescriptor:
  // istreamdatum is a pointer to a LockPTR
  int fd = Processes::fd( **istreamdatum ); // Get FileDescriptor

  //-------------------------------
  // Get filestatus-flags on this fd:
  int flags = fcntl( fd, F_GETFL );
  if ( flags == -1 )
  {
    i->raiseerror( systemerror( i ) ); // an error occured!
  }

  // modify flags to the new value:
  if ( *newflag_d )
  {
    flags |= O_NONBLOCK; // set the flag
  }
  else
  {
    flags &= ~O_NONBLOCK; // erase the flag
  }

  // Set new filestatus-flags:
  int result = fcntl( fd, F_SETFL, flags );
  // ------------------------------


  if ( result == -1 )
  { // an error occured!
    i->raiseerror( systemerror( i ) );
  }
  else
  { // no error
    // leave the istream on OStack
    i->EStack.pop();
    i->OStack.pop();
  }
}

void
Processes::CtermidFunction::execute( SLIInterpreter* i ) const
{
  // ensure term buffer is sufficiently large and safely initialized
  assert( L_ctermid > 0 );
  char term[ L_ctermid ];
  term[ 0 ] = '\0';

  const std::string termid = ctermid( term );

  i->OStack.push( Token( termid ) );
  i->EStack.pop();
}

void
Processes::Isatty_osFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  // Read arguments from operand Stack, but leave tokens on stack:
  OstreamDatum* s_d1 = dynamic_cast< OstreamDatum* >( i->OStack.pick( 0 ).datum() );
  assert( s_d1 != NULL );

  int fd = Processes::fd( **s_d1 ); // Get FileDescriptor

  i->OStack.pop();

  if ( isatty( fd ) )
  {
    i->OStack.push( Token( BoolDatum( true ) ) );
  }
  else
  {
    i->OStack.push( Token( BoolDatum( false ) ) );
  }

  i->EStack.pop();
}

void
Processes::Isatty_isFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  // Read arguments from operand Stack, but leave tokens on stack:
  IstreamDatum* s_d1 = dynamic_cast< IstreamDatum* >( i->OStack.pick( 0 ).datum() );
  assert( s_d1 != NULL );

  int fd = Processes::fd( **s_d1 ); // Get FileDescriptor


  i->OStack.pop();

  if ( isatty( fd ) )
  {
    i->OStack.push( Token( BoolDatum( true ) ) );
  }
  else
  {
    i->OStack.push( Token( BoolDatum( false ) ) );
  }

  i->EStack.pop();
}

#ifdef _SYNOD__SET_POSIX_SOURCE
#undef _SYNOD__SET_POSIX_SOURCE
#undef _POSIX_SOURCE
#endif
