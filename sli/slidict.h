/*
 *  slidict.h
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

#ifndef SLIDICT_H
#define SLIDICT_H
/*
    SLI dictionary access
*/

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "interpret.h"

/**************************************
  All SLI dictionary functions are
  defined in this module
  *************************************/

void init_slidict( SLIInterpreter* );

class DictFunction : public SLIFunction
{
public:
  DictFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictputFunction : public SLIFunction
{
public:
  DictputFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictgetFunction : public SLIFunction
{
public:
  DictgetFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictbeginFunction : public SLIFunction
{
public:
  DictbeginFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictendFunction : public SLIFunction
{
public:
  DictendFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

// MH fx 2010-09-28
// This function is not implemented.
// Definition commented out because it prevents compilation on PETA fx.
//
// class DictundefFunction: public SLIFunction
//{
// public:
// DictundefFunction() {}
//    void execute(SLIInterpreter *) const;
//};

class DictinfoFunction : public SLIFunction
{
public:
  DictinfoFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DicttopinfoFunction : public SLIFunction
{
public:
  DicttopinfoFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class WhoFunction : public SLIFunction
{
public:
  WhoFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictconstructFunction : public SLIFunction
{
public:
  DictconstructFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class DictstackFunction : public SLIFunction
{
public:
  DictstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CurrentdictFunction : public SLIFunction
{
public:
  CurrentdictFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CountdictstackFunction : public SLIFunction
{
public:
  CountdictstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CleardictstackFunction : public SLIFunction
{
public:
  CleardictstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Length_dFunction : public SLIFunction
{
public:
  Length_dFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Empty_DFunction : public SLIFunction
{
public:
  Empty_DFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CleardictFunction : public SLIFunction
{
public:
  CleardictFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ClonedictFunction : public SLIFunction
{
public:
  ClonedictFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class UndefFunction : public SLIFunction
{
public:
  UndefFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class KnownFunction : public SLIFunction
{
public:
  KnownFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cva_dFunction : public SLIFunction
{
public:
  Cva_dFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class KeysFunction : public SLIFunction
{
public:
  KeysFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ValuesFunction : public SLIFunction
{
public:
  ValuesFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RestoredstackFunction : public SLIFunction
{
public:
  RestoredstackFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

#endif
