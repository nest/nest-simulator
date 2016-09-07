/*
 *  sliactions.h
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

#ifndef __DEFAULTACTIONS
#define __DEFAULTACTIONS
/*
    Actions associated with SLI types.
*/

// Includes from sli:
#include "slifunction.h"

class DatatypeFunction : public SLIFunction
{
public:
  DatatypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class NametypeFunction : public SLIFunction
{
public:
  NametypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ProceduretypeFunction : public SLIFunction
{
public:
  ProceduretypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class LitproceduretypeFunction : public SLIFunction
{
public:
  LitproceduretypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class FunctiontypeFunction : public SLIFunction
{
public:
  FunctiontypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CallbacktypeFunction : public SLIFunction
{
public:
  CallbacktypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class XIstreamtypeFunction : public SLIFunction
{
public:
  XIstreamtypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class XIfstreamtypeFunction : public SLIFunction
{
public:
  XIfstreamtypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class TrietypeFunction : public SLIFunction
{
public:
  TrietypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

#endif
