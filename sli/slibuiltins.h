/*
 *  slibuiltins.h
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

#ifndef SLIBUILTINS_H
#define SLIBUILTINS_H
/*
    The interpreter's basic operators
*/

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "slifunction.h"

/*********************************************************
  This module contains only those functions which are
  needed by the intereter's default actions. All other
  built-in or user supplied functions must be defined
  either in builtins.{h,cc} or in user-defined modules
  *******************************************************/

class IlookupFunction : public SLIFunction
{
public:
  IlookupFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IsetcallbackFunction : public SLIFunction
{
public:
  IsetcallbackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IiterateFunction : public SLIFunction
{
public:
  IiterateFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IloopFunction : public SLIFunction
{
public:
  IloopFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IrepeatFunction : public SLIFunction
{
public:
  IrepeatFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IforFunction : public SLIFunction
{
public:
  IforFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IforallarrayFunction : public SLIFunction
{
public:
  IforallarrayFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IforallindexedarrayFunction : public SLIFunction
{
public:
  IforallindexedarrayFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IforallindexedstringFunction : public SLIFunction
{
public:
  IforallindexedstringFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};

class IforallstringFunction : public SLIFunction
{
public:
  IforallstringFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
  void backtrace( SLIInterpreter*, int ) const;
};


#endif
