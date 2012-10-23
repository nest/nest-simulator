/*
 *  pynestmodule.cpp
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

/* 
    This file is part of NEST

    pynestmodule.cpp -- module providing models depending on Python

    Author(s): 
    Hans Ekkehard Plesser, 
    based on work by M.O.Gewaltig, E. Mueller and M. Helias

    First Version: January 2009
*/

#include "config.h"
#include "pynestmodule.h"
#include "network.h"
#include "model.h"
#include "genericmodel.h"
#include <string>

// Node headers


namespace nest
{

  SLIType PynestModule::Pyobjecttype;

  /* At the time when PynestModule is constructed, the SLI Interpreter
     must already be initialized. PynestModule relies on the presence of
     the following SLI datastructures: Name, Dictionary
  */

  PynestModule::PynestModule(Network& net) :
    net_(net)
   {
    Pyobjecttype.settypename("pyobjecttype");
    Pyobjecttype.setdefaultaction(SLIInterpreter::datatypefunction);
   }

   PynestModule::~PynestModule()
   {
     Pyobjecttype.deletetypename();
   }

   const std::string PynestModule::name(void) const
   {
     return std::string("NEST Python-dependent Models Module"); // Return name of the module
   }

   const std::string PynestModule::commandstring(void) const
   {
     return std::string(""); // Run associated SLI startup script
   }

  //-------------------------------------------------------------------------------------

  void PynestModule::init(SLIInterpreter *)
  {
    // register models
  }  // PynestModule::init()


} // namespace nest

