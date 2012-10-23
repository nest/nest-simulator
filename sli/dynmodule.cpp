/*
 *  dynmodule.cpp
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

#include "dynmodule.h"
#include "interpret.h"

void DynModule::install(std::ostream &, SLIInterpreter *i, nest::Network *net)
{
  // Output stream for all messages are now decided by the message
  // level.
  //  i->message(out,5, name().c_str(), "Initializing.");
  i->message(5, name().c_str(), "Initializing.");
  init(i, net);
}

const std::string DynModule::commandstring(void) const
{
  return std::string();
}
