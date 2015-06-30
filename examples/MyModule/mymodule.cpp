/*
 *  mymodule.cpp
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

// include necessary NEST headers
#include "config.h"
#include "network.h"
#include "model.h"
#include "dynamicloader.h"
#include "genericmodel.h"
#include "booldatum.h"
#include "integerdatum.h"
#include "tokenarray.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include "nestmodule.h"
#include "connector_model_impl.h"
#include "target_identifier.h"

// include headers with your own stuff
#include "mymodule.h"
#include "pif_psc_alpha.h"
#include "drop_odd_spike_connection.h"

// -- Interface to dynamic module loader ---------------------------------------

/*
 * The dynamic module loader must be able to find your module.
 * You make the module known to the loader by defining an instance of your
 * module class in global scope. This instance must have the name
 *
 * <modulename>_LTX_mod
 *
 * The dynamicloader can then load modulename and search for symbol "mod" in it.
 */

mynest::MyModule mymodule_LTX_mod;

// -- DynModule functions ------------------------------------------------------

mynest::MyModule::MyModule()
{
#ifdef LINKED_MODULE
  // register this module at the dynamic loader
  // this is needed to allow for linking in this module at compile time
  // all registered modules will be initialized by the main app's dynamic loader
  nest::DynamicLoaderModule::registerLinkedModule( this );
#endif
}

mynest::MyModule::~MyModule()
{
}

const std::string
mynest::MyModule::name( void ) const
{
  return std::string( "My NEST Module" ); // Return name of the module
}

const std::string
mynest::MyModule::commandstring( void ) const
{
  // Instruct the interpreter to load mymodule-init.sli
  return std::string( "(mymodule-init) run" );
}

/* BeginDocumentation
   Name: StepPatternConnect - Connect sources and targets with a stepping pattern

   Synopsis:
   [sources] source_step [targets] target_step synmod StepPatternConnect -> n_connections

   Parameters:
   [sources]     - Array containing GIDs of potential source neurons
   source_step   - Make connection from every source_step'th neuron
   [targets]     - Array containing GIDs of potential target neurons
   target_step   - Make connection to every target_step'th neuron
   synmod        - The synapse model to use (literal, must be key in synapsedict)
   n_connections - Number of connections made

   Description:
   This function subsamples the source and target arrays given with steps
   source_step and target_step, beginning with the first element in each array,
   and connects the selected nodes.

   Example:
   /first_src 0 /network_size get def
   /last_src /iaf_neuron 20 Create def  % nodes  1 .. 20
   /src [first_src last_src] Range def
   /last_tgt /iaf_neuron 10 Create def  % nodes 21 .. 30
   /tgt [last_src 1 add last_tgt] Range def

   src 6 tgt 4 /drop_odd_spike StepPatternConnect

   This connects nodes [1, 7, 13, 19] as sources to nodes [21, 25,
   29] as targets using synapses of type drop_odd_spike, and
   returning 12 as the number of connections.  The following
   command will print the connections (you must paste the SLI
   command as one long line):

   src { /s Set << /source s >> GetConnections { cva 1 get } Map dup length 0 gt { cout s <- ( -> )
   <- exch <-- endl } if ; } forall
   1 -> [21 25 29]
   7 -> [21 25 29]
   13 -> [21 25 29]
   19 -> [21 25 29]

   Remark:
   This function is only provided as an example for how to write your own
   interface function.

   Author:
   Hans Ekkehard Plesser

   SeeAlso: Connect
*/
void
mynest::MyModule::StepPatternConnect_Vi_i_Vi_i_lFunction::execute( SLIInterpreter* i ) const
{
  // Check if we have (at least) five arguments on the stack.
  i->assert_stack_load( 5 );

  // Retrieve source, source step, target, target step from the stack
  const TokenArray sources = getValue< TokenArray >( i->OStack.pick( 4 ) ); // bottom
  const long src_step = getValue< long >( i->OStack.pick( 3 ) );
  const TokenArray targets = getValue< TokenArray >( i->OStack.pick( 2 ) );
  const long tgt_step = getValue< long >( i->OStack.pick( 1 ) );
  const Name synmodel_name = getValue< std::string >( i->OStack.pick( 0 ) ); // top

  // Obtain synapse model index
  const Token synmodel = nest::NestModule::get_network().get_synapsedict().lookup( synmodel_name );
  if ( synmodel.empty() )
    throw nest::UnknownSynapseType( synmodel_name.toString() );
  const nest::index synmodel_id = static_cast< nest::index >( synmodel );

  // Build a list of targets with the given step
  TokenArray selected_targets;
  for ( size_t t = 0; t < targets.size(); t += tgt_step )
    selected_targets.push_back( targets[ t ] );

  // Now connect all appropriate sources to this list of targets
  size_t Nconn = 0; // counts connections
  for ( size_t s = 0; s < sources.size(); s += src_step )
  {
    // We must first obtain the GID of the source as integer
    const nest::long_t sgid = getValue< nest::long_t >( sources[ s ] );

    // nest::network::divergent_connect() requires weight and delay arrays. We want to use
    // default values from the synapse model, so we pass empty arrays.
    nest::NestModule::get_network().divergent_connect(
      sgid, selected_targets, TokenArray(), TokenArray(), synmodel_id );
    Nconn += selected_targets.size();
  }

  // We get here only if none of the operations above throws and exception.
  // Now we can safely remove the arguments from the stack and push Nconn
  // as our result.
  i->OStack.pop( 5 );
  i->OStack.push( Nconn );

  // Finally, we pop the call to this functions from the execution stack.
  i->EStack.pop();
}

//-------------------------------------------------------------------------------------

void
mynest::MyModule::init( SLIInterpreter* i )
{
  /* Register a neuron or device model.
     Give node type as template argument and the name as second argument.
     The first argument is always a reference to the network.
  */
  nest::register_model< pif_psc_alpha >( nest::NestModule::get_network(), "pif_psc_alpha" );

  /* Register a synapse type.
     Give synapse type as template argument and the name as second argument.
     The first argument is always a reference to the network.

     There are two choices for the template argument:
         - nest::TargetIdentifierPtrRport
         - nest::TargetIdentifierIndex
     The first is the standard and you should usually stick to it.
     nest::TargetIdentifierIndex reduces the memory requirement of synapses
     even further, but limits the number of available rports. Please see
     Kunkel et al, Front Neurofinfom 8:78 (2014), Sec 3.3.2, for details.
  */
  nest::register_connection_model< DropOddSpikeConnection< nest::TargetIdentifierPtrRport > >(
    nest::NestModule::get_network(), "drop_odd_synapse" );

  /* Register a SLI function.
     The first argument is the function name for SLI, the second a pointer to
     the function object. If you do not want to overload the function in SLI,
     you do not need to give the mangled name. If you give a mangled name, you
     should define a type trie in the mymodule-init.sli file.
  */
  i->createcommand( "StepPatternConnect_Vi_i_Vi_i_l", &stepPatternConnect_Vi_i_Vi_i_lFunction );

} // MyModule::init()
