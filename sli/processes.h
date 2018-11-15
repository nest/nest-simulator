/*
 *  processes.h
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

#ifndef PROCESSES_H
#define PROCESSES_H

/*
    SLI's basic process management capabilities
*/

// C includes:
#include <sys/types.h>

// C++ includes:
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "name.h"
#include "slifunction.h"
#include "slimodule.h"

// A new SLI-Module:
class Processes : public SLIModule
{
  // The following concernes the new module: -----------------------

public:
  static const std::string systemerror(
    SLIInterpreter* ); // This will be used to produce systemerror-messages

  static int fd( std::istream* s );
  static int fd( std::ostream* s );
  static int
  fd( std::ostream& s )
  {
    return fd( &s );
  }
  static int
  fd( std::istream& s )
  {
    return fd( &s );
  }

  static pid_t children_group; // the declaration of a static variable. It will
  // persist as long as the instantiation of the Processes module lives. Never
  // confuse declaration and definition of a variable, by the way...

  const Name signaldict_name; // The name of the signal dictionary

  // The Names of the signals contained in signal dictionary:
  // These are the signals defined by the POSIX standard
  const Name SIGABRT_name;
  const Name SIGALRM_name;
  const Name SIGFPE_name;
  const Name SIGHUP_name;
  const Name SIGILL_name;
  const Name SIGINT_name;
  const Name SIGKILL_name;
  const Name SIGPIPE_name;
  const Name SIGQUIT_name;
  const Name SIGSEGV_name;
  const Name SIGTERM_name;
  const Name SIGUSR1_name;
  const Name SIGUSR2_name;

  const Name SIGCHLD_name;
  const Name SIGCONT_name;
  const Name SIGSTOP_name;
  const Name SIGTSTP_name;
  const Name SIGTTIN_name;
  const Name SIGTTOU_name;


  const Name sys_errname; // The name of the variable in errordict, in which the
                          // name of a system error will be stored
  const Name sys_errno;   // The corresponding error-number

  // The Names of the system's error-numbers contained in errordict
  const Name E2BIG_name;
  const Name EACCES_name;
  const Name EAGAIN_name;
  const Name EBADF_name;
  const Name EBUSY_name;
  const Name ECHILD_name;
  const Name EDEADLK_name;
  const Name EDOM_name;
  const Name EEXIST_name;
  const Name EFAULT_name;
  const Name EFBIG_name;
  const Name EINTR_name;
  const Name EINVAL_name;
  const Name EIO_name;
  const Name EISDIR_name;
  const Name EMFILE_name;
  const Name EMLINK_name;
  const Name ENAMETOOLONG_name;
  const Name ENFILE_name;
  const Name ENODEV_name;
  const Name ENOENT_name;
  const Name ENOEXEC_name;
  const Name ENOLCK_name;
  const Name ENOMEM_name;
  const Name ENOSPC_name;
  const Name ENOSYS_name;
  const Name ENOTDIR_name;
  const Name ENOTEMPTY_name;
  const Name ENOTTY_name;
  const Name ENXIO_name;
  const Name EPERM_name;
  const Name EPIPE_name;
  const Name ERANGE_name;
  const Name EROFS_name;
  const Name ESPIPE_name;
  const Name ESRCH_name;
  const Name EXDEV_name;

  // The constructor and destructor for our module object (-if we need them-):
  Processes( void )
    : signaldict_name( "signaldict" )
    , SIGABRT_name( "SIGABRT" )
    , SIGALRM_name( "SIGALRM" )
    , SIGFPE_name( "SIGFPE" )
    , SIGHUP_name( "SIGHUP" )
    , SIGILL_name( "SIGILL" )
    , SIGINT_name( "SIGINT" )
    , SIGKILL_name( "SIGKILL" )
    , SIGPIPE_name( "SIGPIPE" )
    , SIGQUIT_name( "SIGQUIT" )
    , SIGSEGV_name( "SIGSEGV" )
    , SIGTERM_name( "SIGTERM" )
    , SIGUSR1_name( "SIGUSR1" )
    , SIGUSR2_name( "SIGUSR2" )
    , SIGCHLD_name( "SIGCHLD" )
    , SIGCONT_name( "SIGCONT" )
    , SIGSTOP_name( "SIGSTOP" )
    , SIGTSTP_name( "SIGTSTP" )
    , SIGTTIN_name( "SIGTTIN" )
    , SIGTTOU_name( "SIGTTOU" )
    , sys_errname( "sys_errname" ) // The name of the variable in errordict, in
                                   // which the name of a system error will be
                                   // stored
    , sys_errno( "sys_errno" )     // The corresponding error-number

    // The Names of the system's error-numbers contained in errordict
    , E2BIG_name( "E2BIG" )
    , EACCES_name( "EACCES" )
    , EAGAIN_name( "EAGAIN" )
    , EBADF_name( "EBADF" )
    , EBUSY_name( "EBUSY" )
    , ECHILD_name( "ECHILD" )
    , EDEADLK_name( "EDEADLK" )
    , EDOM_name( "EDOM" )
    , EEXIST_name( "EEXIST" )
    , EFAULT_name( "EFAULT" )
    , EFBIG_name( "EFBIG" )
    , EINTR_name( "EINTR" )
    , EINVAL_name( "EINVAL" )
    , EIO_name( "EIO" )
    , EISDIR_name( "EISDIR" )
    , EMFILE_name( "EMFILE" )
    , EMLINK_name( "EMLINK" )
    , ENAMETOOLONG_name( "ENAMETOOLONG" )
    , ENFILE_name( "ENFILE" )
    , ENODEV_name( "ENODEV" )
    , ENOENT_name( "ENOENT" )
    , ENOEXEC_name( "ENOEXEC" )
    , ENOLCK_name( "ENOLCK" )
    , ENOMEM_name( "ENOMEM" )
    , ENOSPC_name( "ENOSPC" )
    , ENOSYS_name( "ENOSYS" )
    , ENOTDIR_name( "ENOTDIR" )
    , ENOTEMPTY_name( "ENOTEMPTY" )
    , ENOTTY_name( "ENOTTY" )
    , ENXIO_name( "ENXIO" )
    , EPERM_name( "EPERM" )
    , EPIPE_name( "EPIPE" )
    , ERANGE_name( "ERANGE" )
    , EROFS_name( "EROFS" )
    , ESPIPE_name( "ESPIPE" )
    , ESRCH_name( "ESRCH" )
    , EXDEV_name( "EXDEV" )
  {
  } // Processes constructor

  ~Processes( void ); // clean up dynmem for static variables...

  // The Module is registered by a call to this Function:
  void init( SLIInterpreter* );

  // This function will return the name of our module:
  const std::string name( void ) const;

  // This function -may- return a string of SLI-commands to be executed for
  // initialization
  const std::string commandstring( void ) const;


  // ---------------------------------------------------------------


  // The following concernes the new command(s): -------------------

public:
  // Module contains classes defining new SLI-functions:
  class ForkFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Sysexec_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class WaitPIDFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class KillFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class PipeFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Dup2_is_isFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Dup2_os_osFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Dup2_is_osFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Dup2_os_isFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class AvailableFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class GetPIDFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class GetPPIDFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class GetPGRPFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class MkfifoFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q
  class MemoryThisjobBgFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  };
#endif

#if defined __APPLE__ && defined HAVE_MACH_MACH_H
  class MemoryThisjobDarwinFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  };
#endif

  class SetNonblockFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class CtermidFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Isatty_isFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };
  class Isatty_osFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const; // This is all we need.
  };

  // Module contains -one- instantiation of each new function-class:

public:
  ForkFunction forkfunction;
  Sysexec_aFunction sysexec_afunction;
  WaitPIDFunction waitPIDfunction;
  KillFunction killfunction;
  PipeFunction pipefunction;
  Dup2_is_isFunction dup2_is_isfunction;
  Dup2_os_osFunction dup2_os_osfunction;
  Dup2_is_osFunction dup2_is_osfunction;
  Dup2_os_isFunction dup2_os_isfunction;
  AvailableFunction availablefunction;
  GetPIDFunction getpidfunction;
  GetPPIDFunction getppidfunction;
  GetPGRPFunction getpgrpfunction;
  MkfifoFunction mkfifofunction;

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q
  MemoryThisjobBgFunction memorythisjobbgfunction;
#endif

#if defined __APPLE__ && defined HAVE_MACH_MACH_H
  MemoryThisjobDarwinFunction memorythisjobdarwinfunction;
#endif

  SetNonblockFunction setnonblockfunction;
  CtermidFunction ctermidfunction;
  Isatty_osFunction isatty_osfunction;
  Isatty_isFunction isatty_isfunction;
};


// Description of new SLI-commands:

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: fork - create a child process of SLI

Synopsis: fork -> PID

Description: Thin wrapper to the fork() system function

Parameters: In : -none-
            Out: PID(integer) : 0 (for the child)
                                the child's process ID (for the parent)

Examples: 1. fork
             (Well, just kidding...)

          2. fork 0 eq {(I'm the child!) = quit} {(I'm the parent!) =} ifelse
             Try this several times. You will notice the child message to appear
             before or after the parent message "by chance". (Even after the
             parent's SLI-prompt...)

Bugs: -

Author: R Kupper

FirstVersion: Mar 17 1999

Remarks: A full parallel process of SLI is forked.
         Parent and child will execute in parallel. There is no way to know
         which will start being executed first.
         Child inherits all open files, including stdin and stdout, from parent!
         Thus, calling fork interactively from the SLI-prompt will result in
         command-line-confusion if both processes end up without quitting.
         If fork() cannot be executed, an error is raised.

         SLI-function spoon (processes.sli) is a more convenient wrapper
          to fork!

SeeAlso: spoon, sysexec, getPID, getPPID, getPGRP, wait, waitPID
*/

//-----------------------------------------------------------------------------
/* Now Documented with sysexec!

Name: sysexec_a - execute a UNIX command

Synopsis: CommandArray  sysexec -> -

Description: Transfer control to a UNIX-Command.

Parameters:  In : CommandArray (array of strings):
                  An array containing the command to execute. The first
                  element is interpreted as the command, the remaining elements
                  as it's parameters.

             Out: -whatever will be will be-

Examples: -see command sysexec-

Bugs: -see command sysexec-

Author: R Kupper

FirstVersion: Mar 1999

Remarks: There is a SLI-wrapper called "sysexec" to this function.
         -see command sysexec-

SeeAlso: sysexec, fork, spoon

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: waitPID - wait or check for a child process to terminate

Synopsis: PIDin NoHangFlag waitPID -> Status NormalExitFlag PIDout
                                   -> 0

Description: The waitPID function suspends execution of the calling process
              until status information for the given child is available,
              or until delivery of a signal whose action is either to
              execute a signal-catching function or to terminate the
              process. If status information is available prior to the call
              to waitPID, it returns immediately.

              The waitPID function returns the process ID of the child for
              which status is being reported.
              Zero is returned immediately, if the NoHangFlag is set
              and no status is available.

              Alternatives: Function waitPID_i_b (undocumented)
              -> behaviour and synopsis are the same.

Parameters: In : PIDin(integer):      -1: Wait for any child process.
                                positive: Wait for the specific child whose
                                          process ID is equal to PIDin.
                           (           0: -not supported-)
                           (less than -1: -not supported-)

                 NoHangFlag(boolean):
                  If true, causes waitPID function not to suspend execution
                  of the calling process if status is not immediately
                  available for any of the child processes specified by
                  PIDin. In this case, zero is returned as only result.

            Out: PIDout(integer):
                   The process ID of the child for which status is being
                   reported.

                 NormalExitFlag(boolean):
                   True, if status is returned for a child that terminated
                   normally, i.e. by a call to exit() or by returning from
                   main(). In that case, the exit code is reported in the Status
                   argument (see below).
                   False, if status is returned for a child that terminated due
                   to a signal that was not caught. In that case, the number of
                   that signal is reported in the Status argument (see below).

                 Status(integer):
                   If NormalExitFlag is true, this reports the child's exit
                   code, i.e. the low-order eight bits of the status argument
                   that the child passed to exit(), or the value the child
                   process returned from main().
                   If NormalExitFlag is false, this reports the number of the
                   signal that caused the termination of the child process. Look
                   up this number in signaldict, to know what it means.

Examples:1. {(sleep 3) sysexec} spoon false waitPID
         2. {(ls) sysexec} spoon false waitPID
            Note the parent process' SLI-Prompt appearing AFTER execution of the
            command finished.
            This is different to the call "{(ls) sysexec} spoon" only.
         3. 23 false waitPID

Bugs: -

Author: R Kupper

FirstVersion: Apr 13 1999

Remarks: The features normally used only by the UNIX-shell-program
          (such as the WUNTRACED-option) are currently not supported,
          although values <=0 can be passed through PIDin.
         See any documentation of POSIX-function waitpid().
         Discription text is mainly taken from
          Donald A. Lewine, "POSIX programmer's guide", O'Reilly&Associates,Inc.

SeeAlso: wait, spoon, signaldict, getPGRP

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: kill - send a signal to another process

Synopsis: PID  SIGNAL kill -> -
          PID /SIGNAL kill -> -

Description: The "kill" function sends a signal to a process or a group
              of processes specified by "PID". If the signal is zero,
              error checking is performed but no signal is actually sent.
              This can be used to check for a valid PID.
             SIGNAL may be given either as an integer value or as the
              literal name of the signal, as found in "signaldict".

              Alternative: Functions kill_i_i for integer (SIGNAL),
              kill_i_l for literal (SIGNAL) (both undocumented)
              -> behaviour and synopsis are the same.

Parameters: In : PID(integer):
                   The ID of the process that shall be signalled.
                   If "PID" is greater than zero, "SIGNAL" is sent to the
                    process whose ID is "PID".
                   (If "PID" is negative, "SIGNAL" is sent to all processes
                    whose process group ID is equal to the absolute value
                    of "PID". - Process groups are usually used by the
                    shell program only.)

                 SIGNAL(integer):
                   The number of the signal to be sent.
                   Signal codes are machine-dependent values, so do not
                    rely on any given value! The valid signal codes for
                    your machine are compiled into the "signaldict"
                    dictionary, where they can be looked up by
                    their literal names.
                   The only value you may rely on is zero:
                    If SIGNAL is zero, error checking is performed
                    but no signal is actually sent.
                    This can be used to check for a valid PID.

                  /SIGNAL(literal):
                   The literal name of the signal to be sent.
                   This name is automatically looked up in "signaldict"
                    and substituted to it's corresponding value prior
                    to a call to wait.

            Out: -none-.

Examples: 1. 23 /SIGTERM kill %send TERM signal to Process 23

          2. %This is equivalent to 1. :
             signaldict begin
             23 SIGTERM kill
             end

          3. (xterm) 2 system %star an xterm-process
             /SIGKILL kill    %kill it again

Bugs: -

Author: R Kupper

FirstVersion: Apr 27 1999

Remarks: "kill" can be used to send ANY signal, but it's most oftenly used
          to terminate another process. Hence the name.
         Resolution of literal signal names is done by a trie defined
          in file processes.sli.
         Description taken mainly from "POSIX Programmer's Guide",
          D. Lewine, O'Reilly & Assoc. Inc.


SeeAlso: signaldict, system, sysexec, wait, waitPID, spoon, fork, getPPID,
getPGRP

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: signaldict - Dictionary containing the machine-dependent signal codes.

Synopsis: signaldict -> signaldict

Description: A SLI dictionary containing the system's valid signal codes.
             "signaldict" is used in combination with the "kill",
              "wait" or "waitPID" commands.
             Signal codes are machine-dependent values, so do not
              rely on any given value! The valid signal codes for
              your machine are compiled into the "signaldict"
              dictionary, where they can be looked up by
              their literal names.

Examples: 1. signaldict /SIGTERM get %get the value for signal SIGTERM

          2. (xterm) 2 system %start an xterm-prcess
             signaldict begin %open signal dictionary
             SIGKILL kill     %kill process
             end              %close dictionary

          3. %This is equivalent to 2. (see description of "kill"):
             (xterm) 2 system %start an xterm-process
             /SIGKILL kill    %kill it again

Bugs: -

Author: R Kupper

FirstVersion: Apr 16 1999

SeeAlso: kill, wait, waitPID, system, sysexec, spoon, fork

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: pipe - Open up a pipe

Synopsis: pipe -> read_end write_end

Description:  The pipe function creates a pipe, placing a filestream
             for the read end and a filestream for the write end of
             the pipe on the stack.
              Data can be written to "write_end" and read from "read_end".
             A read on "read_end" accesses the data written to "write_end"
             on a first-in-first-out basis.

Parameters: In : -none-

            Out: read_end(ifstream):
                   A filestream open for reading, connected to the read-
                   end of the newly created pipe.

                 write_end(ofstream):
                   A filestream open for writing, connected to the write-
                   end of the newly created pipe.

Examples: pipe
          (Hello Pipe) <- std::endl
          pop
          getline =

Diagnostics: If a system-error occurs, a code is stored in "sys_errno"
              (contained in errordict) to identify the error, and
              "sys_errname" is set to the error message. Then a
              "SystemError" is raised.

Bugs: -

Author: R Kupper

FirstVersion: May 02 1999

Remarks:  Description-text taken mainly from "POSIX Programmer's Guide",
         D. Lewine, O'Reilly & Assoc. Inc.

          The O_NONBLOCK and FD_CLOEXEC flags are clear on the
         file descriptors of both streams.

          Opening a pipe in a single process is next to useless (however,
         it might be used for buffering data). The usual application
         is for inter-process-communication: A pipe is opened, and fork
         is called. The child process inherits both filestreams from
         the parent. The child will then close one of the streams (say:
         the read-end), the parent will close the other (say: the write-
         end). Data may then be transfered from the child to the parent
         process through the pipe.

          If the child is to "sysexec" a UNIX command, it may duplicate
         the pipes's write-end onto its standard "cout" stream using "dup2",
         thus directing any data written to "cout" into the pipe. It then
         calles "sysexec". The parent process is thus enabled to read
         the UNIX-command's standard output from the pipe.

          Pipes are unidirectional communication channels.
         For bidirectional communication, two separate pipes must be opened.
         The "spawn" command provides this functionality.

SeeAlso: dup2, available, spawn

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: available - check if data is available from an istream

Synopsis: istream available -> istream {true|false}

Description: "available" gives the answer to one question:
             --Is there at least one character waiting to be read
               from the istream?--
             If "available" returns true, it can be safely assumed that
             reading one character from the given istream is safe,
             i.e. it will NEITHER BLOCK nor yield EOF or an error.

             Alternative: Functions available_is (undocumented)
             -> behavior and synopsis are the same.

Parameters: In:  istream: The istream to check.
            Out: true or false, indicating if data is waiting on the stream.

Examples: myfifo available { getline } if % read it data is available.

Diagnostics: If a system-error occurs, a code is stored in "sys_errno"
             (contained in errordict) to identify the error, and "sys_errname"
             is set to the error message. Then a "SystemError" is raised.
             The following system errors may be issued, according to the
             POSIX standard (errors in parentheses are not
             expected to occur in this routines' context):

             (EACCES)  Search permission is denied for a
                       directory in a files path prefix.
             (EAGAIN)  The ON_NONBLOCK flag is set for a file
                       descriptor and the process would be
                       delayed in the I/O operation.
              EBADF    Invalid file descriptor. (With the current
                       implementation, this indicates trouble
                       getting a fildescriptor from a stream. If
                       it occurs, ask the author for a proper
                       solution!)
             (EDEADLK) A fcntl with function F_SETLKW would
                       cause a deadlock.
              EINTR    Function was interrupted by a signal.
             (EINVAL)  Invalid argument.
             (EMFILE   Too many file descriptors are in use by
                       this process.
             (ENOLCK)  No locks available.

Bugs: -

Author: R Kupper

FirstVersion: May 10 1999

Remarks: "available" will be typically used with pipes or fifos.

         There are two possible reasons why "available" may return false:
          1. There are processes writing to the pipe/fifo, but none
             of the is currently writing data to it.
              A subsequent read attempt will block until data becomes
             available.
          2. There are no processes writing to the pipe (any longer).
              A subsequent read attempt will yield EOF.
         It is NOT possible to tell these possibilities apart! This is
         not a fault of the implementation of this function. It is generally
         impossible to do this. The only way to know is to start a read
         attempt. If it blocks, you know the answer - but you could wait
         forever. Anyway, there normally is no need to distinguish between
         these alternatives: Just NEVER try a read attempt, if "available"
         returned false. Even if temporarily no process was connected to
         the stream, it will return true as soon as the connection is re-
         established and data is waiting.

         "available" just tells you if -one- character may be read safely.
          It is left to the programmer to assure that a given amount of
         data (e.g. upto the next linefeed) may be read.

SeeAlso: pipe, mkfifo, spawn, eof, in_avail

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: getPID - Get ID of the current process

Synopsis: getPID -> -

Description: Returns the process ID for the calling process.

Parameters: -

Examples: (I am process #) =only getPID =

Diagnostics: A call to "getPID" should never produce an error.

Bugs: -

Author: R Kupper

FirstVersion: May 26 1999

SeeAlso: getPPID, getPGRP, fork, spoon, waitPID, kill, system, spawn, shpawn

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: getPPID - Get parent ID of the current process

Synopsis: getPPID -> -

Description: Returns the process parent ID for the calling process.

Parameters: -

Examples: (I was called by process #) =only getPPID =

Bugs: -

Author: S Schrader, taken from getPID

FirstVersion: Nov 11 2005

SeeAlso: getPID, getPGRP, fork, spoon, waitPID, kill, system, spawn, shpawn

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: getPGRP - Get process group ID of the current process

Synopsis: getPGRP -> -

Description: Returns the process group ID for the calling process.

Parameters: -

Examples: (I am member of process group #) =only getPGRP =

Diagnostics: A call to "getPGRP" should never produce an error.

Bugs: -

Author: R Kupper

FirstVersion: Apr 18 2000

SeeAlso: fork, getPID, kill

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: mkfifo - Create a FIFO special file (named pipe)

Synopsis: path mkfifo -> -

Description: The "mkfifo" command creates a new FIFO special file named
             "path". The permission bits are set to "rwx rwx rwx" (full access
             for anyone). Note that these bits may be modified by the process'
             file creation mask. (See remarks below.)

             Alternative: Functions mkfifo_s (undocumented)
             -> behavior and synopsis are the same.

Parameters: In: path(string):
                 Path name of the FIFO to create.

Examples: (/home/kupper/my_fifo) mkfifo

Diagnostics: If a system-error occurs, a code is stored in "sys_errno"
             (contained in errordict) to identify the error, and "sys_errname"
             is set to the error message. Then a "SystemError" is raised.
             The following system errors may be issued, according to the
             POSIX standard:

              EACCES  Search permission denied for a directory in a file's path
                      prefix.
              EEXIST  The named file already exists.
              ENOENT  A file or directory does not exist.
              ENOSPC  No space left on disk.
              ENOTDIR A component of the specified pathname was not a directory
                      when a directory was expected.
              EROFS   Read-only file system.

Bugs: -

Author: R. Kupper

FirstVersion: Aug 13 1999 (Friday 13th!)

Remarks: The FIFO may be used (and has to be opened) like any regular file.

         In special cases, it might be desireable to change the FIFO's file
         permission bits. This is not supported by SLI commands.
         Use UNIX-command "chmod" to change file permissions on the newly
         created file, or use UNIX-command "umask" to set the process' file
         creation mask. (See command "system" for issuing UNIX-commands from
         SLI).

SeeAlso: pipe, mkfifo, ifstream, available, ignore, dup2

*/

//-----------------------------------------------------------------------------
/** @BeginDocumentation
Name: setNONBLOCK - Switch between blocking and non-blocking I/O.

Synopsis: ifstream {true|false} setNONBLOCK -> ifstream

Description: "setNONBLOCK" sets or erases the O_NONBLOCK flag on
             an input stream. The O_NONBLOCK flag determines, if
             a read attempt will block when no data is currently
             available from the stream. By default, a newly
             created stream has the O_NONBLOCK-Flag erased,
             meaning that blocking I/O is selected. By erasing
             O_NONBLOCK, a subsequent read attempt on the stream
             will yield EOF if no data is available.

             Alternatives: Function setNONBLOCK_is_b (undocumented)
             -> behavior and synopsis are the same.

Parameters: In : ifstream: The stream to change the flag on.
            Out: -

Examples: cin    false setNONBLOCK
          myfifo true  setNONBLOCK % set non-blocking I/O for my fifo.

Diagnostics: If a system-error occurs, a code is stored in "sys_errno"
             (contained in errordict) to identify the error, and "sys_errname"
             is set to the error message. Then a "SystemError" is raised.
             The following system errors may be issued, according to the
             POSIX standard (errors in parentheses are not
             expected to occur in this routines' context):

             (EACCES)  Search permission is denied for a
                       directory in a files path prefix.
             (EAGAIN)  The ON_NONBLOCK flag is set for a file
                       descriptor and the process would be
                       delayed in the I/O operation.
              EBADF    Invalid file descriptor. (With the current
                       implementation, this indicates trouble
                       getting a filedescriptor from a stream. If
                       it occurs, ask the author for a proper
                       solution!)
             (EDEADLK) An fcntl with function F_SETLKW would
                       cause a deadlock.
              EINTR    Function was interrupted by a signal.
             (EINVAL)  Invalid argument.
             (EMFILE   Too many file descriptors are in use by
                       this process.
             (ENOLCK)  No locks available.

Bugs: -

Author: R Kupper

FirstVersion: Oct 20 1999

SeeAlso: available, ignore

*/


/** @BeginDocumentation
Name: ctermid - Return the path to the controlling terminal of the process.

Synopsis: ctermid -> (pathname)

Remarks: This is a wrapper to the POSIX kernel function ctermid().

SeeAlso: isatty
*/

#endif
