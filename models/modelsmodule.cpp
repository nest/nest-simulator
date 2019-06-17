/*
 *  modelsmodule.cpp
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
    This file is part of NEST.

    modelsmodule.cpp -- sets up the modeldict with all models included
    with the NEST distribution.

    Author(s):
    Marc-Oliver Gewaltig
    R"udiger Kupper
    Hans Ekkehard Plesser

    First Version: June 2006
*/

#include "modelsmodule.h"

// Includes from nestkernel
#include "genericmodel_impl.h"

// Generated includes:
#include "config.h"

// Neuron models
#include "hh_psc_alpha.h"
#include "hh_psc_alpha_clopath.h"
#include "hh_psc_alpha_gap.h"
#include "iaf_cond_exp.h"
#include "iaf_psc_alpha.h"

// Stimulation devices
#include "ac_generator.h"
#include "dc_generator.h"
#include "gamma_sup_generator.h"
#include "mip_generator.h"
#include "noise_generator.h"
#include "poisson_generator.h"
#include "inhomogeneous_poisson_generator.h"
#include "ppd_sup_generator.h"
#include "pulsepacket_generator.h"
#include "sinusoidal_gamma_generator.h"
#include "sinusoidal_poisson_generator.h"
#include "spike_generator.h"
#include "step_current_generator.h"
#include "step_rate_generator.h"

// Recording devices
#include "correlation_detector.h"
#include "correlomatrix_detector.h"
#include "correlospinmatrix_detector.h"
#include "multimeter.h"
#include "spike_detector.h"
#include "spin_detector.h"
#include "weight_recorder.h"

#include "volume_transmitter.h"

// Prototypes for synapses
#include "bernoulli_connection.h"
#include "clopath_connection.h"
#include "common_synapse_properties.h"
#include "cont_delay_connection_impl.h"
#include "diffusion_connection.h"
#include "ht_connection.h"
#include "quantal_stp_connection.h"
#include "quantal_stp_connection_impl.h"
#include "spike_dilutor.h"
#include "static_connection.h"
#include "static_connection_hom_w.h"
#include "stdp_connection.h"
#include "stdp_connection_facetshw_hom.h"
#include "stdp_connection_facetshw_hom_impl.h"
#include "stdp_connection_hom.h"
#include "stdp_dopa_connection.h"
#include "stdp_pl_connection_hom.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model_impl.h"
#include "genericmodel.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"
#include "target_identifier.h"

#ifdef HAVE_MUSIC
#include "music_event_in_proxy.h"
#include "music_event_out_proxy.h"
#include "music_cont_in_proxy.h"
#include "music_cont_out_proxy.h"
#include "music_message_in_proxy.h"
#endif

namespace nest
{
// At the time when ModelsModule is constructed, the SLI Interpreter
// must already be initialized. ModelsModule relies on the presence of
// the following SLI datastructures: Name, Dictionary
ModelsModule::ModelsModule()
{
}

ModelsModule::~ModelsModule()
{
}

const std::string
ModelsModule::name( void ) const
{
  return std::string(
    "NEST Standard Models Module" ); // Return name of the module
}

const std::string
ModelsModule::commandstring( void ) const
{
  // TODO: Move models-init.sli to sli_neuron....
  return std::string( "(models-init) run" );
}

//-------------------------------------------------------------------------------------

void
ModelsModule::init( SLIInterpreter* )
{
  kernel().model_manager.register_node_model< iaf_psc_alpha >(
    "iaf_psc_alpha" );

  kernel().model_manager.register_node_model< poisson_generator >(
    "poisson_generator" );

  kernel().model_manager.register_node_model< spike_detector >(
    "spike_detector" );
  kernel().model_manager.register_node_model< weight_recorder >(
    "weight_recorder" );
  kernel().model_manager.register_node_model< spin_detector >(
    "spin_detector" );
  kernel().model_manager.register_node_model< Multimeter >( "multimeter" );
  kernel().model_manager.register_node_model< correlation_detector >(
    "correlation_detector" );
  kernel().model_manager.register_node_model< correlomatrix_detector >(
    "correlomatrix_detector" );
  kernel().model_manager.register_node_model< correlospinmatrix_detector >(
    "correlospinmatrix_detector" );
  kernel().model_manager.register_node_model< volume_transmitter >(
    "volume_transmitter" );

  // Create voltmeter as a multimeter pre-configured to record V_m.
  /** @BeginDocumentation
  Name: voltmeter - Device to record membrane potential from neurons.
  Synopsis: voltmeter Create

  Description:
  A voltmeter records the membrane potential (V_m) of connected nodes
  to memory, file or stdout.

  By default, voltmeters record values once per ms. Set the parameter
  /interval to change this. The recording interval cannot be smaller
  than the resolution.

  Results are returned in the /events entry of the status dictionary,
  which contains membrane potential as vector /V_m and pertaining
  times as vector /times and node GIDs as /senders, if /withtime and
  /withgid are set, respectively.

  Accumulator mode:
  Voltmeter can operate in accumulator mode. In this case, values for all
  recorded variables are added across all recorded nodes (but kept separate in
  time). This can be useful to record average membrane potential in a
  population.

  To activate accumulator mode, either set /to_accumulator to true, or set
  /record_to [ /accumulator ].  In accumulator mode, you cannot record to file,
  to memory, to screen, with GID or with weight. You must activate accumulator
  mode before simulating. Accumulator data is never written to file. You must
  extract it from the device using GetStatus.

  Remarks:
   - The voltmeter model is implemented as a multimeter preconfigured to
     record /V_m.
   - The set of variables to record and the recording interval must be set
     BEFORE the voltmeter is connected to any node, and cannot be changed
     afterwards.
   - A voltmeter cannot be frozen.
   - If you record with voltmeter in accumulator mode and some of the nodes
     you record from are frozen and others are not, data will only be collected
     from the unfrozen nodes. Most likely, this will lead to confusing results,
     so you should not use voltmeter with frozen nodes.

  Parameters:
       The following parameter can be set in the status dictionary:
       interval     double - Recording interval in ms

  Examples:
  SLI ] /iaf_cond_alpha Create /n Set
  SLI ] /voltmeter Create /vm Set
  SLI ] vm << /interval 0.5 >> SetStatus
  SLI ] vm n Connect
  SLI ] 10 Simulate
  SLI ] vm /events get info
  --------------------------------------------------
  Name                     Type                Value
  --------------------------------------------------
  senders                  intvectortype       <intvectortype>
  times                    doublevectortype    <doublevectortype>
  V_m                      doublevectortype    <doublevectortype>
  --------------------------------------------------
  Total number of entries: 3


  Sends: DataLoggingRequest

  SeeAlso: Device, RecordingDevice, multimeter
  */
  DictionaryDatum vmdict = DictionaryDatum( new Dictionary );
  ArrayDatum ad;
  ad.push_back( LiteralDatum( names::V_m.toString() ) );
  ( *vmdict )[ names::record_from ] = ad;
  const Name name = "voltmeter";
  kernel().model_manager.register_preconf_node_model< Multimeter >(
    name, vmdict, false );

#ifdef HAVE_GSL
  kernel().model_manager.register_node_model< iaf_cond_exp >( "iaf_cond_exp" );
#endif

  // This version of the AdEx model does not depend on GSL.
  //kernel().model_manager.register_node_model< aeif_cond_alpha_RK5 >(
  //  "aeif_cond_alpha_RK5",
  //  /*private_model*/ false,
  //  /*deprecation_info*/ "NEST 3.0" );

#ifdef HAVE_MUSIC
  //// proxies for inter-application communication using MUSIC
  kernel().model_manager.register_node_model< music_event_in_proxy >(
    "music_event_in_proxy" );
  kernel().model_manager.register_node_model< music_event_out_proxy >(
    "music_event_out_proxy" );
  kernel().model_manager.register_node_model< music_cont_in_proxy >(
    "music_cont_in_proxy" );
  kernel().model_manager.register_node_model< music_cont_out_proxy >(
    "music_cont_out_proxy" );
  kernel().model_manager.register_node_model< music_message_in_proxy >(
    "music_message_in_proxy" );
#endif

  // register synapses

  /** @BeginDocumentation
     Name: static_synapse_hpc - Variant of static_synapse with low memory
     consumption.

     Description:
     hpc synapses store the target neuron in form of a 2 Byte index instead of
     an 8 Byte pointer. This limits the number of thread local neurons to
     65,536. No support for different receptor types. Otherwise identical to
     static_synapse.

     SeeAlso: synapsedict, static_synapse
  */
  kernel()
    .model_manager
    .register_connection_model< StaticConnection< TargetIdentifierPtrRport > >(
      "static_synapse" );
  kernel()
    .model_manager
    .register_connection_model< StaticConnection< TargetIdentifierIndex > >(
      "static_synapse_hpc" );


  /** @BeginDocumentation
     Name: static_synapse_hom_w_hpc - Variant of static_synapse_hom_w with low
     memory consumption.
     SeeAlso: synapsedict, static_synapse_hom_w, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< StaticConnectionHomW< TargetIdentifierPtrRport > >(
      "static_synapse_hom_w" );
  kernel()
    .model_manager
    .register_connection_model< StaticConnectionHomW< TargetIdentifierIndex > >(
      "static_synapse_hom_w_hpc" );

  /* BeginDocumentation
     Name: stdp_synapse_hpc - Variant of stdp_synapse with low memory
     consumption.
     SeeAlso: synapsedict, stdp_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPConnection< TargetIdentifierPtrRport > >(
      "stdp_synapse" );
  kernel()
    .model_manager
    .register_connection_model< STDPConnection< TargetIdentifierIndex > >(
      "stdp_synapse_hpc" );

  kernel()
    .model_manager
    .register_connection_model< ClopathConnection< TargetIdentifierPtrRport > >(
      "clopath_synapse",
      /*requires_symmetric=*/false,
      /*requires_clopath_archiving=*/true );

  /** @BeginDocumentation
     Name: stdp_pl_synapse_hom_hpc - Variant of stdp_pl_synapse_hom with low
     memory consumption.
     SeeAlso: synapsedict, stdp_pl_synapse_hom, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPPLConnectionHom< TargetIdentifierPtrRport > >(
      "stdp_pl_synapse_hom" );
  kernel()
    .model_manager
    .register_connection_model< STDPPLConnectionHom< TargetIdentifierIndex > >(
      "stdp_pl_synapse_hom_hpc" );

  /* BeginDocumentation
     Name: stdp_synapse_hom_hpc - Variant of quantal_stp_synapse with low memory
     consumption.
     SeeAlso: synapsedict, stdp_synapse_hom, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPConnectionHom< TargetIdentifierPtrRport > >(
      "stdp_synapse_hom" );
  kernel()
    .model_manager
    .register_connection_model< STDPConnectionHom< TargetIdentifierIndex > >(
      "stdp_synapse_hom_hpc" );

  /* BeginDocumentation
     Name: bernoulli_synapse - Static synapse with stochastic transmission
     SeeAlso: synapsedict, static_synapse, static_synapse_hom_w
  */
  kernel()
    .model_manager
    .register_connection_model< BernoulliConnection< TargetIdentifierPtrRport > >(
      "bernoulli_synapse" );
}

} // namespace nest
