/*
 *  slicontrol.h
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

#ifndef SLICONTROL_H
#define SLICONTROL_H
/*
    SLI's control structures
*/

// Includes from sli:
#include "dictdatum.h"
#include "interpret.h"

/**************************************
  All SLI control functions are
  defined in this module
  *************************************/

void init_slicontrol( SLIInterpreter* );

class Backtrace_onFunction : public SLIFunction
{
public:
  Backtrace_onFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Backtrace_offFunction : public SLIFunction
{
public:
  Backtrace_offFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OStackdumpFunction : public SLIFunction
{
public:
  OStackdumpFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class EStackdumpFunction : public SLIFunction
{
public:
  EStackdumpFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class LoopFunction : public SLIFunction
{
public:
  LoopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ExitFunction : public SLIFunction
{
public:
  ExitFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class QuitFunction : public SLIFunction
{
public:
  QuitFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IfFunction : public SLIFunction
{
public:
  IfFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IfelseFunction : public SLIFunction
{
public:
  IfelseFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RepeatFunction : public SLIFunction
{
public:
  RepeatFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CloseinputFunction : public SLIFunction
{
public:
  CloseinputFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class StoppedFunction : public SLIFunction
{
public:
  StoppedFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class StopFunction : public SLIFunction
{
public:
  StopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CurrentnameFunction : public SLIFunction
{
public:
  CurrentnameFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IparsestdinFunction : public SLIFunction
{
public:
  IparsestdinFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class StartFunction : public SLIFunction
{
public:
  StartFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ParsestdinFunction : public SLIFunction
{
public:
  ParsestdinFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IparseFunction : public SLIFunction
{
public:
  IparseFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DefFunction : public SLIFunction
{
public:
  DefFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class SetFunction : public SLIFunction
{
public:
  SetFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class LoadFunction : public SLIFunction
{
public:
  LoadFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class LookupFunction : public SLIFunction
{
public:
  LookupFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ForFunction : public SLIFunction
{
public:
  ForFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Forall_aFunction : public SLIFunction
{
public:
  Forall_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Forallindexed_aFunction : public SLIFunction
{
public:
  Forallindexed_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Forallindexed_sFunction : public SLIFunction
{
public:
  Forallindexed_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Forall_sFunction : public SLIFunction
{
public:
  Forall_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RaiseerrorFunction : public SLIFunction
{
public:
  RaiseerrorFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class PrinterrorFunction : public SLIFunction
{
public:
  PrinterrorFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RaiseagainFunction : public SLIFunction
{
public:
  RaiseagainFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CyclesFunction : public SLIFunction
{
public:
  CyclesFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CodeAccessedFunction : public SLIFunction
{
public:
  CodeAccessedFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CodeExecutedFunction : public SLIFunction
{
public:
  CodeExecutedFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ExecFunction : public SLIFunction
{
public:
  ExecFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


class TypeinfoFunction : public SLIFunction
{
public:
  TypeinfoFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

/** @BeginDocumentation
 Name: switch - finish a case ... switch structure

 Synopsis: mark proc1...procn switch-> -

 Description:
   switch executes proc1 ... procn and removes the mark. If any executed
   proc containes an exit command, switch will remove the other procs without
   execution. switch is used together with case.

 Parameters:
   proc1...procn: executable procedure tokens.

 Examples:

   mark
     false {1 == exit} case
     false {2 == exit} case
     true  {3 == exit} case
     false {4 == exit} case
   switch
   --> 3

   mark {1 ==} {2 ==} switch -->  1 2

 Author: Gewaltig

 SeeAlso: case, switchdefault, exit, mark
*/


class SwitchFunction : public SLIFunction
{
public:
  SwitchFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

/** @BeginDocumentation
 Name: switchdefault - finish a case ... switchdefault structure

 Synopsis: mark proc1...procn procdefault switchdefault -> -

 Description:
   Like switch, switchdefault executes any of proc1...procn.
   If an execution it meets an exit command, no further procs are executed.
   If n=0, e.g. no procedure other than procdefault is found, procdefault
   will be executed. Thus, procdefault will be skipped if any other proc
   exists.

 Parameters:
   proc1...procn: executable procedure tokens.
   procdefault  : execulable procedure called if no other proc is present.

 Examples:

   mark
     false {1 == exit} case
     false {2 == exit} case
     true  {3 == exit} case
     false {4 == exit} case
     {(default) ==}
   switchdefault
   -->  3

   mark
     false {1 == exit} case
     false {2 == exit} case
     false {3 == exit} case
     false {4 == exit} case
     {(default) ==}
   switchdefault
   --> default

 Author: Hehl

 FirstVersion: April 16, 1999

 SeeAlso: case, switch, exit, mark
*/

class SwitchdefaultFunction : public SLIFunction
{
public:
  SwitchdefaultFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

/** @BeginDocumentation
 Name: case - like if, but test a series of conditions.
 Synopsis: bool proc case --> proc
                              -
 Description:
   case tests the bool and pushes proc if true, else does nothing.

 Parameters:
   bool : condition for case to test
   proc : procedure to be executed if case is true

 Examples:

   true {(hello) ==} case --> hello
   false {(hello) ==} case --> -

   1 0 gt {(1 bigger than 0) ==} case --> 1 bigger than 0
   1 0 lt {(0 bigger than 1) ==} case --> -

   mark
     false {1 == exit} case
     false {2 == exit} case
     true  {3 == exit} case
     false {4 == exit} case
   switch
   --> 3

 Author: Gewaltig

 Remarks: Use exit to make sure that switch is exited.

 SeeAlso: switch, switchdefault, exit, mark, if
*/

class CaseFunction : public SLIFunction
{
public:
  CaseFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CounttomarkFunction : public SLIFunction
{
public:
  CounttomarkFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class PclocksFunction : public SLIFunction
{
public:
  PclocksFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class PclockspersecFunction : public SLIFunction
{
public:
  PclockspersecFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class PgetrusageFunction : public SLIFunction
{
public:
  PgetrusageFunction()
  {
  }
  void execute( SLIInterpreter* ) const;

private:
  bool getinfo_( int, DictionaryDatum& ) const;
};

class TimeFunction : public SLIFunction
{
public:
  TimeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Sleep_dFunction : public SLIFunction
{
public:
  Sleep_dFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Token_sFunction : public SLIFunction
{
public:
  Token_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Token_isFunction : public SLIFunction
{
public:
  Token_isFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Symbol_sFunction : public SLIFunction
{
public:
  Symbol_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class SetGuardFunction : public SLIFunction
{
public:
  SetGuardFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RemoveGuardFunction : public SLIFunction
{
public:
  RemoveGuardFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class MessageFunction : public SLIFunction
{
public:
  MessageFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class SetVerbosityFunction : public SLIFunction
{
public:
  SetVerbosityFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class VerbosityFunction : public SLIFunction
{
public:
  VerbosityFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


class NoopFunction : public SLIFunction
{
public:
  NoopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DebugOnFunction : public SLIFunction
{
public:
  DebugOnFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DebugOffFunction : public SLIFunction
{
public:
  DebugOffFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DebugFunction : public SLIFunction
{
public:
  DebugFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


#endif
