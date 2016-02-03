/*
 *  slimodule.h
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

#ifndef SLIMODULE_H
#define SLIMODULE_H

// C++ includes:
#include <iostream>
#include <string>

class SLIInterpreter;
class Dictionary;

/**
 * Base class for all SLI Interpreter modules.
 */
class SLIModule
{
public:
  virtual ~SLIModule(){};

  /**
   * Initialize the module.
   * When this function is called, most of the
   * interpreter's facilities are up and running.
   * However, depending on where in the interpreter's
   * bootstrap sequence the module is initialized, not
   * all services may be available.
   */
  virtual void init( SLIInterpreter* ) = 0;

  /**
   * Return name of the module.
   */
  virtual const std::string name( void ) const = 0;

  /**
   * Return sli command sequence to be executed for initialization.
   */
  virtual const std::string commandstring( void ) const;

  /**
   * Print installation message via interpreter message command.
   */
  void install( std::ostream&, SLIInterpreter* );
};
#endif
