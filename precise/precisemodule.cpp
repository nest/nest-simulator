/*
 *  precisemodule.cpp
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

    precisemodule.cpp -- module providing models with precise spike timing

    Author(s): 
    Hans Ekkehard Plesser, 
    based on work by M.O.Gewaltig, E. Mueller and M. Helias

    First Version: June 2006
*/

#include "config.h"
#include "precisemodule.h"
#include "network.h"
#include "model.h"
#include "genericmodel.h"
#include <string>

// Node headers
#include "iaf_psc_delta_canon.h"
#include "iaf_psc_alpha_canon.h"
#include "iaf_psc_alpha_presc.h"
#include "iaf_psc_exp_ps.h"
#include "poisson_generator_ps.h"
#include "parrot_neuron_ps.h"

namespace nest
{

  /* At the time when PreciseModule is constructed, the SLI Interpreter
     must already be initialized. PreciseModule relies on the presence of
     the following SLI datastructures: Name, Dictionary
  */

  PreciseModule::PreciseModule(Network& net) :
    net_(net)
   {
   }

   PreciseModule::~PreciseModule()
   {
   }

   const std::string PreciseModule::name(void) const
   {
     return std::string("NEST Precise Spike-Timing Models Module"); // Return name of the module
   }

   const std::string PreciseModule::commandstring(void) const
   {
     return std::string(""); // Run associated SLI startup script
   }

  //-------------------------------------------------------------------------------------

  void PreciseModule::init(SLIInterpreter *)
  {
    // register models
    register_model<iaf_psc_delta_canon>(net_, "iaf_psc_delta_canon");
    register_model<iaf_psc_alpha_canon>(net_, "iaf_psc_alpha_canon");
    register_model<iaf_psc_alpha_presc>(net_, "iaf_psc_alpha_presc");
    register_model<iaf_psc_exp_ps>(net_, "iaf_psc_exp_ps");
    register_model<poisson_generator_ps>(net_, "poisson_generator_ps");
    register_model<parrot_neuron_ps>(net_, "parrot_neuron_ps");
  }  // PreciseModule::init()


} // namespace nest

