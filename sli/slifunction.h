/*
 *  slifunction.h
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

#ifndef SLIFUNCTION_H
#define SLIFUNCTION_H
/*
    Base class for all SLI functions.
*/

class SLIInterpreter;

/*
  class SLICommand replaces the old class Function from SYNOD 1.x.
 */

class SLIFunction
{
public:
  SLIFunction()
  {
  }
  virtual void execute( SLIInterpreter* ) const = 0;
  virtual ~SLIFunction()
  {
  }

  /**
   * Show stack backtrace information on error.
   * This function tries to extract and display useful
   * information from the execution stack if an error occurs.
   * This function should be implemented for all functions which
   * store administrative information on the execution stack.
   * Examples are: loops and procedure iterations.
   * backtrace() is only called, if the interpreter flag
   * show_backtrace is set.
   */
  virtual void
  backtrace( SLIInterpreter*, int ) const
  {
  }
};

#endif
