/*
 *  main.cpp
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

#include <config.h>

#include "neststartup.h"

#include "network.h"
#include "interpret.h"

int main(int argc, char *argv[])
{
  nest::Network *pNet = 0;

  /** 
   * Create the interpreter object. Due to its dependence
   * on various static objects (e.g. of class Name), the
   * interpreter engine MUST NOT be global.
   */
  SLIInterpreter engine;

  neststartup(argc, argv, engine, pNet);

  // start the interpreter session
  int exitcode = engine.execute();

  nestshutdown();

  // delete the Network before modules are deleted by interpreter's destructor
  // because otherwise models defined in a module might still be referenced by
  // the Network
  delete pNet;

  return exitcode;
}
