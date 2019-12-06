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

#include "precisemodule.h"

// C++ includes:
#include <string>

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "genericmodel.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"

// Includes from precise:
#include "iaf_psc_alpha_canon.h"
#include "iaf_psc_alpha_ps.h"
#include "iaf_psc_delta_ps.h"
#include "iaf_psc_exp_ps.h"
#include "parrot_neuron_ps.h"
#include "poisson_generator_ps.h"
#include "iaf_psc_exp_ps_lossless.h"

namespace nest
{

/* At the time when PreciseModule is constructed, the SLI Interpreter
   must already be initialized. PreciseModule relies on the presence of
   the following SLI datastructures: Name, Dictionary
*/

PreciseModule::PreciseModule()
{
}

PreciseModule::~PreciseModule()
{
}

const std::string
PreciseModule::name( void ) const
{
  return std::string( "NEST Precise Spike-Timing Models Module" ); // Return name of the module
}

const std::string
PreciseModule::commandstring( void ) const
{
  return std::string( "" ); // Run associated SLI startup script
}

//-------------------------------------------------------------------------------------

void
PreciseModule::init( SLIInterpreter* )
{
  kernel().model_manager.register_node_model< iaf_psc_alpha_canon >(
    "iaf_psc_alpha_canon", /*private_model*/ false, /*deprecation_info*/ "a future version of NEST" );
  kernel().model_manager.register_node_model< iaf_psc_alpha_ps >( "iaf_psc_alpha_ps" );
  kernel().model_manager.register_node_model< iaf_psc_delta_ps >( "iaf_psc_delta_ps" );
  kernel().model_manager.register_node_model< iaf_psc_exp_ps >( "iaf_psc_exp_ps" );
  kernel().model_manager.register_node_model< iaf_psc_exp_ps_lossless >( "iaf_psc_exp_ps_lossless" );
  kernel().model_manager.register_node_model< parrot_neuron_ps >( "parrot_neuron_ps" );
  kernel().model_manager.register_node_model< poisson_generator_ps >( "poisson_generator_ps" );
} // PreciseModule::init()


} // namespace nest
