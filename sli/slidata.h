/*
 *  slidata.h
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

#ifndef SLIDATA_H
#define SLIDATA_H
/*
    SLI's array access functions
*/

// Includes from sli:
#include "slifunction.h"

/*
 Operators will be implemented as described in the PS Reference Manual
 for the types
 array
 string
 NOTE: dictionary operators are defined in slidict.{h,cc}
 */

class Get_aFunction : public SLIFunction
{
public:
  Get_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Get_a_aFunction : public SLIFunction
{
public:
  Get_a_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Allocations_aFunction : public SLIFunction
{
public:
  Allocations_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Get_pFunction : public SLIFunction
{
public:
  Get_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Get_lpFunction : public SLIFunction
{
public:
  Get_lpFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Append_aFunction : public SLIFunction
{
public:
  Append_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Append_pFunction : public SLIFunction
{
public:
  Append_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Append_sFunction : public SLIFunction
{
public:
  Append_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Prepend_aFunction : public SLIFunction
{
public:
  Prepend_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Prepend_pFunction : public SLIFunction
{
public:
  Prepend_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Prepend_sFunction : public SLIFunction
{
public:
  Prepend_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Join_sFunction : public SLIFunction
{
public:
  Join_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Join_aFunction : public SLIFunction
{
public:
  Join_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Join_pFunction : public SLIFunction
{
public:
  Join_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Insert_sFunction : public SLIFunction
{
public:
  Insert_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Insert_aFunction : public SLIFunction
{
public:
  Insert_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class InsertElement_sFunction : public SLIFunction
{
public:
  InsertElement_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class InsertElement_aFunction : public SLIFunction
{
public:
  InsertElement_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Replace_sFunction : public SLIFunction
{
public:
  Replace_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Replace_aFunction : public SLIFunction
{
public:
  Replace_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Erase_sFunction : public SLIFunction
{
public:
  Erase_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Erase_aFunction : public SLIFunction
{
public:
  Erase_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Erase_pFunction : public SLIFunction
{
public:
  Erase_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


class Length_sFunction : public SLIFunction
{
public:
  Length_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Length_aFunction : public SLIFunction
{
public:
  Length_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Length_pFunction : public SLIFunction
{
public:
  Length_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Length_lpFunction : public SLIFunction
{
public:
  Length_lpFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Capacity_aFunction : public SLIFunction
{
public:
  Capacity_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Size_aFunction : public SLIFunction
{
public:
  Size_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Reserve_aFunction : public SLIFunction
{
public:
  Reserve_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Resize_aFunction : public SLIFunction
{
public:
  Resize_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Empty_aFunction : public SLIFunction
{
public:
  Empty_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class References_aFunction : public SLIFunction
{
public:
  References_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Shrink_aFunction : public SLIFunction
{
public:
  Shrink_aFunction()
  {
  }

  void execute( SLIInterpreter* ) const;
};

class Capacity_sFunction : public SLIFunction
{
public:
  Capacity_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Size_sFunction : public SLIFunction
{
public:
  Size_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Reserve_sFunction : public SLIFunction
{
public:
  Reserve_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Resize_sFunction : public SLIFunction
{
public:
  Resize_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Empty_sFunction : public SLIFunction
{
public:
  Empty_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Getinterval_sFunction : public SLIFunction
{
public:
  Getinterval_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Getinterval_aFunction : public SLIFunction
{
public:
  Getinterval_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvx_aFunction : public SLIFunction
{
public:
  Cvx_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvlit_nFunction : public SLIFunction
{
public:
  Cvlit_nFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvlit_pFunction : public SLIFunction
{
public:
  Cvlit_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvlp_pFunction : public SLIFunction
{
public:
  Cvlp_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvn_sFunction : public SLIFunction
{
public:
  Cvn_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvn_lFunction : public SLIFunction
{
public:
  Cvn_lFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

//---------------------------------------------------------------------------------
//(C84) is maximum width for LaTeX-include1
/** @BeginDocumentation
Name: cvi_s - convert string to integer

Synopsis: string cvi_s -> integer

Description: This is a wrapper to the standard C "atoi"-routine.
If cvi_s is executed with a string that contains letters 0 is returned.

Examples: (23)    cvi_s -> 23
          (23.5)  cvi_s -> 23
          (NEST)  cvi_s -> 0

Diagnostics: No errors are raised.
             In case of impossible conversion, zero is returned.

Bugs: -

Author: R Kupper

FirstVersion: Nov 05 1999

SeeAlso: cvi, cvd, cvs

*/
class Cvi_sFunction : public SLIFunction
{
public:
  Cvi_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

//---------------------------------------------------------------------------------
//(C84) is maximum width for LaTeX-include1
/** @BeginDocumentation

Name: cvd_s - convert string to double

Synopsis: string cvd_s -> double

Description: This is a wrapper to the standard C "atof"-routine.
If cvd_s is executed with a string that contains letters 0 is returned.

Examples: (23.5)  cvi_s -> 23.5
          (23)    cvi_s -> 23   % doubletype!
          (NEST)  cvi_s -> 0

Diagnostics: No errors are raised.
             In case of impossible conversion, zero is returned.

Bugs: -

Author: R Kupper

FirstVersion: Nov 05 1999

SeeAlso: cvi, cvd, cvs

*/
class Cvd_sFunction : public SLIFunction
{
public:
  Cvd_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Get_sFunction : public SLIFunction
{
public:
  Get_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Put_sFunction : public SLIFunction
{
public:
  Put_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Put_aFunction : public SLIFunction
{
public:
  Put_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Put_pFunction : public SLIFunction
{
public:
  Put_pFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Put_lpFunction : public SLIFunction
{
public:
  Put_lpFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Search_sFunction : public SLIFunction
{
public:
  Search_sFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Search_aFunction : public SLIFunction
{
public:
  Search_aFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

// This function is not implemented
// needed to remove it in order to compile NEST
// on PETA project fx machine. 2010-10-28 MH
// class RangeFunction: public SLIFunction
//{
// public:
// RangeFunction() {}
//    void execute(SLIInterpreter *) const;
//};

class IrepeatanyFunction : public SLIFunction
{
public:
  IrepeatanyFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class RepeatanyFunction : public SLIFunction
{
public:
  RepeatanyFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};


void init_slidata( SLIInterpreter* );


#endif
