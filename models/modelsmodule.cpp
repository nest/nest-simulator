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
#include "aeif_cond_alpha.h"
#include "aeif_cond_alpha_multisynapse.h"
#include "aeif_cond_beta_multisynapse.h"
#include "aeif_cond_alpha_RK5.h"
#include "aeif_cond_exp.h"
#include "aeif_psc_alpha.h"
#include "aeif_psc_exp.h"
#include "aeif_psc_delta.h"
#include "amat2_psc_exp.h"
#include "erfc_neuron.h"
#include "gauss_rate.h"
#include "ginzburg_neuron.h"
#include "hh_cond_exp_traub.h"
#include "hh_cond_beta_gap_traub.h"
#include "hh_psc_alpha.h"
#include "hh_psc_alpha_gap.h"
#include "ht_neuron.h"
#include "iaf_chs_2007.h"
#include "iaf_chxk_2008.h"
#include "iaf_cond_alpha.h"
#include "iaf_cond_alpha_mc.h"
#include "iaf_cond_beta.h"
#include "iaf_cond_exp.h"
#include "iaf_cond_exp_sfa_rr.h"
#include "iaf_psc_alpha.h"
#include "iaf_psc_alpha_multisynapse.h"
#include "iaf_psc_delta.h"
#include "iaf_psc_exp.h"
#include "iaf_psc_exp_multisynapse.h"
#include "iaf_tum_2000.h"
#include "izhikevich.h"
#include "lin_rate.h"
#include "tanh_rate.h"
#include "threshold_lin_rate.h"
#include "mat2_psc_exp.h"
#include "mcculloch_pitts_neuron.h"
#include "parrot_neuron.h"
#include "pp_pop_psc_delta.h"
#include "pp_psc_delta.h"
#include "siegert_neuron.h"
#include "sigmoid_rate.h"
#include "sigmoid_rate_gg_1998.h"
#include "gif_psc_exp.h"
#include "gif_psc_exp_multisynapse.h"
#include "gif_cond_exp.h"
#include "gif_cond_exp_multisynapse.h"
#include "gif_pop_psc_exp.h"

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
#include "common_synapse_properties.h"
#include "cont_delay_connection.h"
#include "cont_delay_connection_impl.h"
#include "diffusion_connection.h"
#include "gap_junction.h"
#include "ht_connection.h"
#include "quantal_stp_connection.h"
#include "quantal_stp_connection_impl.h"
#include "rate_connection_instantaneous.h"
#include "rate_connection_delayed.h"
#include "spike_dilutor.h"
#include "static_connection.h"
#include "static_connection_hom_w.h"
#include "stdp_connection.h"
#include "stdp_connection_facetshw_hom.h"
#include "stdp_connection_facetshw_hom_impl.h"
#include "stdp_connection_hom.h"
#include "stdp_triplet_connection.h"
#include "stdp_dopa_connection.h"
#include "stdp_pl_connection_hom.h"
#include "tsodyks2_connection.h"
#include "tsodyks_connection.h"
#include "tsodyks_connection_hom.h"
#include "vogels_sprekeler_connection.h"

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
  // rate models with input noise
  kernel().model_manager.register_node_model< gauss_rate_ipn >(
    "gauss_rate_ipn" );
  kernel().model_manager.register_node_model< lin_rate_ipn >( "lin_rate_ipn" );
  kernel().model_manager.register_node_model< sigmoid_rate_ipn >(
    "sigmoid_rate_ipn" );
  kernel().model_manager.register_node_model< sigmoid_rate_gg_1998_ipn >(
    "sigmoid_rate_gg_1998_ipn" );
  kernel().model_manager.register_node_model< tanh_rate_ipn >(
    "tanh_rate_ipn" );
  kernel().model_manager.register_node_model< threshold_lin_rate_ipn >(
    "threshold_lin_rate_ipn" );

  // rate models with output noise
  kernel().model_manager.register_node_model< lin_rate_opn >( "lin_rate_opn" );
  kernel().model_manager.register_node_model< tanh_rate_opn >(
    "tanh_rate_opn" );
  kernel().model_manager.register_node_model< threshold_lin_rate_opn >(
    "threshold_lin_rate_opn" );

  // rate transformer nodes
  kernel().model_manager.register_node_model< rate_transformer_gauss >(
    "rate_transformer_gauss" );
  kernel().model_manager.register_node_model< rate_transformer_lin >(
    "rate_transformer_lin" );
  kernel().model_manager.register_node_model< rate_transformer_sigmoid >(
    "rate_transformer_sigmoid" );
  kernel()
    .model_manager.register_node_model< rate_transformer_sigmoid_gg_1998 >(
      "rate_transformer_sigmoid_gg_1998" );
  kernel().model_manager.register_node_model< rate_transformer_tanh >(
    "rate_transformer_tanh" );
  kernel().model_manager.register_node_model< rate_transformer_threshold_lin >(
    "rate_transformer_threshold_lin" );

  kernel().model_manager.register_node_model< iaf_chs_2007 >( "iaf_chs_2007" );
  kernel().model_manager.register_node_model< iaf_psc_alpha >(
    "iaf_psc_alpha" );
  kernel().model_manager.register_node_model< iaf_psc_alpha_multisynapse >(
    "iaf_psc_alpha_multisynapse" );
  kernel().model_manager.register_node_model< iaf_psc_delta >(
    "iaf_psc_delta" );
  kernel().model_manager.register_node_model< iaf_psc_exp >( "iaf_psc_exp" );
  kernel().model_manager.register_node_model< iaf_psc_exp_multisynapse >(
    "iaf_psc_exp_multisynapse" );
  kernel().model_manager.register_node_model< iaf_tum_2000 >( "iaf_tum_2000" );
  kernel().model_manager.register_node_model< amat2_psc_exp >(
    "amat2_psc_exp" );
  kernel().model_manager.register_node_model< mat2_psc_exp >( "mat2_psc_exp" );
  kernel().model_manager.register_node_model< parrot_neuron >(
    "parrot_neuron" );
  kernel().model_manager.register_node_model< pp_psc_delta >( "pp_psc_delta" );
  kernel().model_manager.register_node_model< pp_pop_psc_delta >(
    "pp_pop_psc_delta" );
  kernel().model_manager.register_node_model< gif_psc_exp >( "gif_psc_exp" );
  kernel().model_manager.register_node_model< gif_psc_exp_multisynapse >(
    "gif_psc_exp_multisynapse" );

  kernel().model_manager.register_node_model< ac_generator >( "ac_generator" );
  kernel().model_manager.register_node_model< dc_generator >( "dc_generator" );
  kernel().model_manager.register_node_model< spike_generator >(
    "spike_generator" );
  kernel().model_manager.register_node_model< inhomogeneous_poisson_generator >(
    "inhomogeneous_poisson_generator" );
  kernel().model_manager.register_node_model< poisson_generator >(
    "poisson_generator" );
  kernel().model_manager.register_node_model< pulsepacket_generator >(
    "pulsepacket_generator" );
  kernel().model_manager.register_node_model< noise_generator >(
    "noise_generator" );
  kernel().model_manager.register_node_model< step_current_generator >(
    "step_current_generator" );
  kernel().model_manager.register_node_model< mip_generator >(
    "mip_generator" );
  kernel().model_manager.register_node_model< sinusoidal_poisson_generator >(
    "sinusoidal_poisson_generator" );
  kernel().model_manager.register_node_model< ppd_sup_generator >(
    "ppd_sup_generator" );
  kernel().model_manager.register_node_model< gamma_sup_generator >(
    "gamma_sup_generator" );
  kernel().model_manager.register_node_model< erfc_neuron >( "erfc_neuron" );
  kernel().model_manager.register_node_model< ginzburg_neuron >(
    "ginzburg_neuron" );
  kernel().model_manager.register_node_model< mcculloch_pitts_neuron >(
    "mcculloch_pitts_neuron" );
  kernel().model_manager.register_node_model< izhikevich >( "izhikevich" );
  kernel().model_manager.register_node_model< spike_dilutor >(
    "spike_dilutor" );

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
  kernel().model_manager.register_node_model< iaf_chxk_2008 >(
    "iaf_chxk_2008" );
  kernel().model_manager.register_node_model< iaf_cond_alpha >(
    "iaf_cond_alpha" );
  kernel().model_manager.register_node_model< iaf_cond_beta >(
    "iaf_cond_beta" );
  kernel().model_manager.register_node_model< iaf_cond_exp >( "iaf_cond_exp" );
  kernel().model_manager.register_node_model< iaf_cond_exp_sfa_rr >(
    "iaf_cond_exp_sfa_rr" );
  kernel().model_manager.register_node_model< iaf_cond_alpha_mc >(
    "iaf_cond_alpha_mc" );
  kernel().model_manager.register_node_model< hh_cond_beta_gap_traub >(
    "hh_cond_beta_gap_traub" );
  kernel().model_manager.register_node_model< hh_psc_alpha >( "hh_psc_alpha" );
  kernel().model_manager.register_node_model< hh_psc_alpha_gap >(
    "hh_psc_alpha_gap" );
  kernel().model_manager.register_node_model< hh_cond_exp_traub >(
    "hh_cond_exp_traub" );
  kernel().model_manager.register_node_model< sinusoidal_gamma_generator >(
    "sinusoidal_gamma_generator" );
  kernel().model_manager.register_node_model< gif_cond_exp >( "gif_cond_exp" );
  kernel().model_manager.register_node_model< gif_cond_exp_multisynapse >(
    "gif_cond_exp_multisynapse" );
  kernel().model_manager.register_node_model< gif_pop_psc_exp >(
    "gif_pop_psc_exp" );

  kernel().model_manager.register_node_model< aeif_cond_alpha >(
    "aeif_cond_alpha" );
  kernel().model_manager.register_node_model< aeif_cond_exp >(
    "aeif_cond_exp" );
  kernel().model_manager.register_node_model< aeif_psc_alpha >(
    "aeif_psc_alpha" );
  kernel().model_manager.register_node_model< aeif_psc_exp >( "aeif_psc_exp" );
  kernel().model_manager.register_node_model< aeif_psc_delta >(
    "aeif_psc_delta" );
  kernel().model_manager.register_node_model< ht_neuron >( "ht_neuron" );
  kernel().model_manager.register_node_model< aeif_cond_beta_multisynapse >(
    "aeif_cond_beta_multisynapse" );
  kernel().model_manager.register_node_model< aeif_cond_alpha_multisynapse >(
    "aeif_cond_alpha_multisynapse" );
  kernel().model_manager.register_node_model< siegert_neuron >(
    "siegert_neuron" );
#endif

  // This version of the AdEx model does not depend on GSL.
  kernel().model_manager.register_node_model< aeif_cond_alpha_RK5 >(
    "aeif_cond_alpha_RK5",
    /*private_model*/ false,
    /*deprecation_info*/ "NEST 3.0" );

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

  /** @BeginDocumentation
     Name: gap_junction - Connection model for gap junctions.
     SeeAlso: synapsedict
  */
  kernel()
    .model_manager
    .register_secondary_connection_model< GapJunction< TargetIdentifierPtrRport > >(
      "gap_junction",
      /*has_delay=*/false,
      /*requires_symmetric=*/true,
      /*supports_wfr=*/true );
  kernel()
    .model_manager
    .register_secondary_connection_model< RateConnectionInstantaneous< TargetIdentifierPtrRport > >(
      "rate_connection_instantaneous",
      /*has_delay=*/false,
      /*requires_symmetric=*/false,
      /*supports_wfr=*/true );
  kernel()
    .model_manager
    .register_secondary_connection_model< RateConnectionDelayed< TargetIdentifierPtrRport > >(
      "rate_connection_delayed",
      /*has_delay=*/true,
      /*requires_symmetric=*/false,
      /*supports_wfr=*/false );
  kernel()
    .model_manager
    .register_secondary_connection_model< DiffusionConnection< TargetIdentifierPtrRport > >(
      "diffusion_connection",
      /*has_delay=*/false,
      /*requires_symmetric=*/false,
      /*supports_wfr=*/true );


  /** @BeginDocumentation
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


  /** @BeginDocumentation
     Name: stdp_triplet_synapse_hpc - Variant of stdp_triplet_synapse with low
     memory consumption.
     SeeAlso: synapsedict, stdp_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPTripletConnection< TargetIdentifierPtrRport > >(
      "stdp_triplet_synapse" );
  kernel()
    .model_manager
    .register_connection_model< STDPTripletConnection< TargetIdentifierIndex > >(
      "stdp_triplet_synapse_hpc" );


  /** @BeginDocumentation
     Name: quantal_stp_synapse_hpc - Variant of quantal_stp_synapse with low
     memory consumption.
     SeeAlso: synapsedict, quantal_stp_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< Quantal_StpConnection< TargetIdentifierPtrRport > >(
      "quantal_stp_synapse" );
  kernel()
    .model_manager
    .register_connection_model< Quantal_StpConnection< TargetIdentifierIndex > >(
      "quantal_stp_synapse_hpc" );


  /** @BeginDocumentation
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


  /** @BeginDocumentation
     Name: stdp_facetshw_synapse_hom_hpc - Variant of stdp_facetshw_synapse_hom
     with low memory consumption.
     SeeAlso: synapsedict, stdp_facetshw_synapse_hom, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPFACETSHWConnectionHom< TargetIdentifierPtrRport > >(
      "stdp_facetshw_synapse_hom" );
  kernel()
    .model_manager
    .register_connection_model< STDPFACETSHWConnectionHom< TargetIdentifierIndex > >(
      "stdp_facetshw_synapse_hom_hpc" );


  /** @BeginDocumentation
     Name: cont_delay_synapse_hpc - Variant of cont_delay_synapse with low
     memory consumption.
     SeeAlso: synapsedict, cont_delay_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< ContDelayConnection< TargetIdentifierPtrRport > >(
      "cont_delay_synapse" );
  kernel()
    .model_manager
    .register_connection_model< ContDelayConnection< TargetIdentifierIndex > >(
      "cont_delay_synapse_hpc" );


  /** @BeginDocumentation
     Name: tsodyks_synapse_hpc - Variant of tsodyks_synapse with low memory
     consumption.
     SeeAlso: synapsedict, tsodyks_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< TsodyksConnection< TargetIdentifierPtrRport > >(
      "tsodyks_synapse" );
  kernel()
    .model_manager
    .register_connection_model< TsodyksConnection< TargetIdentifierIndex > >(
      "tsodyks_synapse_hpc" );


  /** @BeginDocumentation
     Name: tsodyks_synapse_hom_hpc - Variant of tsodyks_synapse_hom with low
     memory consumption.
     SeeAlso: synapsedict, tsodyks_synapse_hom, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< TsodyksConnectionHom< TargetIdentifierPtrRport > >(
      "tsodyks_synapse_hom" );
  kernel()
    .model_manager
    .register_connection_model< TsodyksConnectionHom< TargetIdentifierIndex > >(
      "tsodyks_synapse_hom_hpc" );


  /** @BeginDocumentation
     Name: tsodyks2_synapse_hpc - Variant of tsodyks2_synapse with low memory
     consumption.
     SeeAlso: synapsedict, tsodyks2_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< Tsodyks2Connection< TargetIdentifierPtrRport > >(
      "tsodyks2_synapse" );
  kernel()
    .model_manager
    .register_connection_model< Tsodyks2Connection< TargetIdentifierIndex > >(
      "tsodyks2_synapse_hpc" );


  /** @BeginDocumentation
     Name: ht_synapse_hpc - Variant of ht_synapse with low memory consumption.
     SeeAlso: synapsedict, ht_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< HTConnection< TargetIdentifierPtrRport > >(
      "ht_synapse" );
  kernel()
    .model_manager
    .register_connection_model< HTConnection< TargetIdentifierIndex > >(
      "ht_synapse_hpc" );


  /** @BeginDocumentation
     Name: stdp_dopamine_synapse_hpc - Variant of stdp_dopamine_synapse with low
     memory consumption.
     SeeAlso: synapsedict, stdp_dopamine_synapse, static_synapse_hpc
  */
  kernel()
    .model_manager
    .register_connection_model< STDPDopaConnection< TargetIdentifierPtrRport > >(
      "stdp_dopamine_synapse" );
  kernel()
    .model_manager
    .register_connection_model< STDPDopaConnection< TargetIdentifierIndex > >(
      "stdp_dopamine_synapse_hpc" );

  /** @BeginDocumentation
     Name: vogels_sprekeler_synapse_hpc - Variant of vogels_sprekeler_synapse
     with low memory
     consumption.
     SeeAlso: synapsedict, vogels_sprekeler_synapse
  */
  kernel()
    .model_manager
    .register_connection_model< VogelsSprekelerConnection< TargetIdentifierPtrRport > >(
      "vogels_sprekeler_synapse" );
  kernel()
    .model_manager
    .register_connection_model< VogelsSprekelerConnection< TargetIdentifierIndex > >(
      "vogels_sprekeler_synapse_hpc" );

  /** @BeginDocumentation
     Name: bernoulli_synapse - Static synapse with stochastic transmission
     SeeAlso: synapsedict, static_synapse, static_synapse_hom_w
  */
  kernel()
    .model_manager
    .register_connection_model< BernoulliConnection< TargetIdentifierPtrRport > >(
      "bernoulli_synapse" );
}

} // namespace nest
