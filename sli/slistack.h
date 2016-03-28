/*
 *  slistack.h
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

#ifndef SLISTACK_H
#define SLISTACK_H
/*
    Stack manipulation functions
*/

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "interpret.h"

/************************************************
  Stack manipulation functions
  ********************************************/
class PopFunction : public SLIFunction
{
public:
  PopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class NpopFunction : public SLIFunction
{
public:
  NpopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DupFunction : public SLIFunction
{
public:
  DupFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ExchFunction : public SLIFunction
{
public:
  ExchFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IndexFunction : public SLIFunction
{
public:
  IndexFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RollFunction : public SLIFunction
{
public:
  RollFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RolluFunction : public SLIFunction
{
public:
  RolluFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RolldFunction : public SLIFunction
{
public:
  RolldFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RotFunction : public SLIFunction
{
public:
  RotFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OverFunction : public SLIFunction
{
public:
  OverFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CountFunction : public SLIFunction
{
public:
  CountFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CopyFunction : public SLIFunction
{
public:
  CopyFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ClearFunction : public SLIFunction
{
public:
  ClearFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ExecstackFunction : public SLIFunction
{
public:
  ExecstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RestoreestackFunction : public SLIFunction
{
public:
  RestoreestackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RestoreostackFunction : public SLIFunction
{
public:
  RestoreostackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OperandstackFunction : public SLIFunction
{
public:
  OperandstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};
#endif
