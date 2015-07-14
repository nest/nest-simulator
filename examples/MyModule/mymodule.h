/*
 *  mymodule.h
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

#ifndef MYMODULE_H
#define MYMODULE_H

#include "dynmodule.h"
#include "slifunction.h"

// Put your stuff into your own namespace.
namespace mynest
{

/**
 * Class defining your model.
 * @note For each model, you must define one such class, with a unique name.
 */
class MyModule : public DynModule
{
public:
  // Interface functions ------------------------------------------

  /**
   * @note The constructor registers the module with the dynamic loader.
   *       Initialization proper is performed by the init() method.
   */
  MyModule();

  /**
   * @note The destructor does not do much in modules. Proper "downrigging"
   *       is the responsibility of the unregister() method.
   */
  ~MyModule();

  /**
   * Initialize module.
   * @param SLIInterpreter* SLI interpreter
   */
  void init( SLIInterpreter* );

  /**
   * Return the name of your model.
   */
  const std::string name( void ) const;

  /**
   * Return the name of a sli file to execute when mymodule is loaded.
   * This mechanism can be used to define SLI commands associated with your
   * module, in particular, set up type tries for functions you have defined.
   */
  const std::string commandstring( void ) const;

public:
  // Classes implementing your functions -----------------------------

  /**
   * Implement a function for a step-pattern-based connection.
   * @note What this function does is described in the SLI documentation
   *       in the cpp file.
   * @note The mangled name indicates this function expects the following
   *       arguments on the stack (bottom first): vector of int, int,
   *       vector of int, int.
   * @note You must define a member object in your module class
   *       of the function class. execute() is later invoked on this
   *       member.
   */
  class StepPatternConnect_Vi_i_Vi_i_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  StepPatternConnect_Vi_i_Vi_i_lFunction stepPatternConnect_Vi_i_Vi_i_lFunction;
};
} // namespace mynest

#endif
