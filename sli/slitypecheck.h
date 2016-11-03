/*
 *  slitypecheck.h
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

#ifndef SLITYPECHECK_H
#define SLITYPECHECK_H

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "slifunction.h"

class TrieFunction : public SLIFunction
{
public:
  TrieFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class TrieInfoFunction : public SLIFunction
{
public:
  TrieInfoFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class AddtotrieFunction : public SLIFunction
{
public:
  AddtotrieFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cva_tFunction : public SLIFunction
{
public:
  Cva_tFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvt_aFunction : public SLIFunction
{
public:
  Cvt_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class TypeFunction : public SLIFunction
{
public:
  TypeFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


void init_slitypecheck( SLIInterpreter* );

#endif
