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

#include "config.h"
#include "modelsmodule.h"
#include "network.h"
#include "model.h"
#include "genericmodel.h"
#include <string>

// Neuron models
#include "aeif_cond_alpha.h"
#include "aeif_cond_alpha_RK5.h"
#include "aeif_cond_alpha_multisynapse.h"
#include "aeif_cond_exp.h"
#include "amat2_psc_exp.h"
#include "hh_cond_exp_traub.h"
#include "hh_psc_alpha.h"
#include "ht_neuron.h"
#include "iaf_chs_2007.h"
#include "iaf_chxk_2008.h"
#include "iaf_cond_alpha.h"
#include "iaf_cond_alpha_mc.h"
#include "iaf_cond_exp.h"
#include "iaf_cond_exp_sfa_rr.h"
#include "iaf_neuron.h"
#include "iaf_psc_alpha.h"
#include "iaf_psc_alpha_multisynapse.h"
#include "iaf_psc_delta.h"
#include "iaf_psc_exp.h"
#include "iaf_psc_exp_multisynapse.h"
#include "iaf_tum_2000.h"
#include "mat2_psc_exp.h"
#include "parrot_neuron.h"
#include "pp_psc_delta.h"
#include "pp_pop_psc_delta.h"
#include "sli_neuron.h"
#include "ginzburg_neuron.h"
#include "mcculloch_pitts_neuron.h"
#include "izhikevich.h"

// Stimulation devices
#include "ac_generator.h"
#include "dc_generator.h"
#include "spike_generator.h"
#include "poisson_generator.h"
#include "pulsepacket_generator.h"
#include "noise_generator.h"
#include "step_current_generator.h"
#include "mip_generator.h"
#include "sinusoidal_poisson_generator.h"
#include "sinusoidal_gamma_generator.h"
#include "ppd_sup_generator.h"
#include "gamma_sup_generator.h"

// Recording devices
#include "spike_detector.h"
#include "spin_detector.h"
#include "multimeter.h"
#include "correlation_detector.h"
#include "correlomatrix_detector.h"

#include "volume_transmitter.h"

// Generic (template) implementations for synapse prototypes
#include "connector_model_impl.h"

// Prototypes for synapses

#include "common_synapse_properties.h"
#include "static_connection.h"
#include "static_connection_hom_w.h"
#include "cont_delay_connection.h"
#include "cont_delay_connection_impl.h"
#include "tsodyks_connection.h"
#include "tsodyks2_connection.h"
#include "quantal_stp_connection.h"
#include "quantal_stp_connection_impl.h"
#include "stdp_connection.h"
#include "stdp_connection_hom.h"
#include "stdp_connection_facetshw_hom.h"
#include "stdp_connection_facetshw_hom_impl.h"
#include "stdp_pl_connection_hom.h"
#include "stdp_dopa_connection.h"
#include "ht_connection.h"
#include "spike_dilutor.h"

#include "target_identifier.h"

#ifdef HAVE_MUSIC
#include "music_event_in_proxy.h"
#include "music_event_out_proxy.h"
#include "music_cont_in_proxy.h"
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
  return std::string( "NEST Standard Models Module" ); // Return name of the module
}

const std::string
ModelsModule::commandstring( void ) const
{
  return std::string( "(models-init) run" );
}

//-------------------------------------------------------------------------------------

void
ModelsModule::init( SLIInterpreter* )
{
  register_model< iaf_neuron >( "iaf_neuron" );
  register_model< iaf_chs_2007 >( "iaf_chs_2007" );
  register_model< iaf_psc_alpha >( "iaf_psc_alpha" );
  register_model< iaf_psc_alpha_multisynapse >( "iaf_psc_alpha_multisynapse" );
  register_model< iaf_psc_delta >( "iaf_psc_delta" );
  register_model< iaf_psc_exp >( "iaf_psc_exp" );
  register_model< iaf_psc_exp_multisynapse >( "iaf_psc_exp_multisynapse" );
  register_model< iaf_tum_2000 >( "iaf_tum_2000" );
  register_model< amat2_psc_exp >( "amat2_psc_exp" );
  register_model< mat2_psc_exp >( "mat2_psc_exp" );
  register_model< parrot_neuron >( "parrot_neuron" );
  register_model< pp_psc_delta >( "pp_psc_delta" );
  register_model< pp_pop_psc_delta >( "pp_pop_psc_delta" );

  register_model< ac_generator >( "ac_generator" );
  register_model< dc_generator >( "dc_generator" );
  register_model< spike_generator >( "spike_generator" );
  register_model< poisson_generator >( "poisson_generator" );
  register_model< pulsepacket_generator >( "pulsepacket_generator" );
  register_model< noise_generator >( "noise_generator" );
  register_model< step_current_generator >( "step_current_generator" );
  register_model< mip_generator >( "mip_generator" );
  register_model< sinusoidal_poisson_generator >( "sinusoidal_poisson_generator" );
  register_model< ppd_sup_generator >( "ppd_sup_generator" );
  register_model< gamma_sup_generator >( "gamma_sup_generator" );
  register_model< sli_neuron >( "sli_neuron" );
  register_model< ginzburg_neuron >( "ginzburg_neuron" );
  register_model< mcculloch_pitts_neuron >( "mcculloch_pitts_neuron" );
  register_model< izhikevich >( "izhikevich" );
  register_model< spike_dilutor >( "spike_dilutor" );

  register_model< spike_detector >( "spike_detector" );
  register_model< spin_detector >( "spin_detector" );
  register_model< Multimeter >( "multimeter" );
  register_model< correlation_detector >( "correlation_detector" );
  register_model< correlomatrix_detector >( "correlomatrix_detector" );
  register_model< volume_transmitter >( "volume_transmitter" );

  // Create voltmeter as a multimeter pre-configured to record V_m.
  Dictionary vmdict;
  ArrayDatum ad;
  ad.push_back( LiteralDatum( names::V_m.toString() ) );
  vmdict[ names::record_from ] = ad;
  register_preconf_model< Multimeter >( "voltmeter", vmdict );

#ifdef HAVE_GSL
  register_model< iaf_chxk_2008 >( "iaf_chxk_2008" );
  register_model< iaf_cond_alpha >( "iaf_cond_alpha" );
  register_model< iaf_cond_exp >( "iaf_cond_exp" );
  register_model< iaf_cond_exp_sfa_rr >( "iaf_cond_exp_sfa_rr" );
  register_model< iaf_cond_alpha_mc >( "iaf_cond_alpha_mc" );
  register_model< hh_psc_alpha >( "hh_psc_alpha" );
  register_model< hh_cond_exp_traub >( "hh_cond_exp_traub" );
  register_model< sinusoidal_gamma_generator >( "sinusoidal_gamma_generator" );
#endif

#ifdef HAVE_GSL_1_11
  register_model< aeif_cond_alpha >( "aeif_cond_alpha" );
  register_model< aeif_cond_exp >( "aeif_cond_exp" );
  register_model< ht_neuron >( "ht_neuron" );
#endif
  // This version of the AdEx model does not depend on GSL.
  register_model< aeif_cond_alpha_RK5 >( "aeif_cond_alpha_RK5" );
  register_model< aeif_cond_alpha_multisynapse >( "aeif_cond_alpha_multisynapse" );

#ifdef HAVE_MUSIC
  //// proxies for inter-application communication using MUSIC
  register_model< music_event_in_proxy >( "music_event_in_proxy" );
  register_model< music_event_out_proxy >( "music_event_out_proxy" );
  register_model< music_cont_in_proxy >( "music_cont_in_proxy" );
  register_model< music_message_in_proxy >( "music_message_in_proxy" );
#endif

  // register synapses

  /* BeginDocumentation
     Name: static_synapse_hpc - Variant of static_synapse with low memory consumption.

     Description:
     hpc synapses store the target neuron in form of a 2 Byte index instead of an
     8 Byte pointer. This limits the number of thread local neurons to 65,536.
     No support for different receptor types.
     Otherwise identical to static_synapse.

     SeeAlso: synapsedict, static_synapse
  */
  register_connection_model< StaticConnection< TargetIdentifierPtrRport > >(
    "static_synapse" );
  register_connection_model< StaticConnection< TargetIdentifierIndex > >(
    "static_synapse_hpc" );


  /* BeginDocumentation
     Name: static_synapse_hom_w_hpc - Variant of static_synapse_hom_w with low memory consumption.
     SeeAlso: synapsedict, static_synapse_hom_w, static_synapse_hpc
  */
  register_connection_model< StaticConnectionHomW< TargetIdentifierPtrRport > >(
    "static_synapse_hom_w" );
  register_connection_model< StaticConnectionHomW< TargetIdentifierIndex > >(
    "static_synapse_hom_w_hpc" );


  /* BeginDocumentation
     Name: stdp_synapse_hpc - Variant of stdp_synapse with low memory consumption.
     SeeAlso: synapsedict, stdp_synapse, static_synapse_hpc
  */
  register_connection_model< STDPConnection< TargetIdentifierPtrRport > >( "stdp_synapse" );
  register_connection_model< STDPConnection< TargetIdentifierIndex > >( "stdp_synapse_hpc" );


  /* BeginDocumentation
     Name: stdp_pl_synapse_hom_hpc - Variant of stdp_pl_synapse_hom with low memory consumption.
     SeeAlso: synapsedict, stdp_pl_synapse_hom, static_synapse_hpc
  */
  register_connection_model< STDPPLConnectionHom< TargetIdentifierPtrRport > >(
    "stdp_pl_synapse_hom" );
  register_connection_model< STDPPLConnectionHom< TargetIdentifierIndex > >(
    "stdp_pl_synapse_hom_hpc" );


  /* BeginDocumentation
     Name: quantal_stp_synapse_hpc - Variant of quantal_stp_synapse with low memory consumption.
     SeeAlso: synapsedict, quantal_stp_synapse, static_synapse_hpc
  */
  register_connection_model< Quantal_StpConnection< TargetIdentifierPtrRport > >(
    "quantal_stp_synapse" );
  register_connection_model< Quantal_StpConnection< TargetIdentifierIndex > >(
    "quantal_stp_synapse_hpc" );


  /* BeginDocumentation
     Name: stdp_synapse_hom_hpc - Variant of quantal_stp_synapse with low memory consumption.
     SeeAlso: synapsedict, stdp_synapse_hom, static_synapse_hpc
  */
  register_connection_model< STDPConnectionHom< TargetIdentifierPtrRport > >(
    "stdp_synapse_hom" );
  register_connection_model< STDPConnectionHom< TargetIdentifierIndex > >(
    "stdp_synapse_hom_hpc" );


  /* BeginDocumentation
     Name: stdp_facetshw_synapse_hom_hpc - Variant of stdp_facetshw_synapse_hom with low memory
     consumption.
     SeeAlso: synapsedict, stdp_facetshw_synapse_hom, static_synapse_hpc
  */
  register_connection_model< STDPFACETSHWConnectionHom< TargetIdentifierPtrRport > >(
    "stdp_facetshw_synapse_hom" );
  register_connection_model< STDPFACETSHWConnectionHom< TargetIdentifierIndex > >(
    "stdp_facetshw_synapse_hom_hpc" );


  /* BeginDocumentation
     Name: cont_delay_synapse_hpc - Variant of cont_delay_synapse with low memory consumption.
     SeeAlso: synapsedict, cont_delay_synapse, static_synapse_hpc
  */
  register_connection_model< ContDelayConnection< TargetIdentifierPtrRport > >(
    "cont_delay_synapse" );
  register_connection_model< ContDelayConnection< TargetIdentifierIndex > >(
    "cont_delay_synapse_hpc" );


  /* BeginDocumentation
     Name: tsodyks_synapse_hpc - Variant of tsodyks_synapse with low memory consumption.
     SeeAlso: synapsedict, tsodyks_synapse, static_synapse_hpc
  */
  register_connection_model< TsodyksConnection< TargetIdentifierPtrRport > >(
    "tsodyks_synapse" );
  register_connection_model< TsodyksConnection< TargetIdentifierIndex > >(
    "tsodyks_synapse_hpc" );


  /* BeginDocumentation
     Name: tsodyks2_synapse_hpc - Variant of tsodyks2_synapse with low memory consumption.
     SeeAlso: synapsedict, tsodyks2_synapse, static_synapse_hpc
  */
  register_connection_model< Tsodyks2Connection< TargetIdentifierPtrRport > >(
    "tsodyks2_synapse" );
  register_connection_model< Tsodyks2Connection< TargetIdentifierIndex > >(
    "tsodyks2_synapse_hpc" );


  /* BeginDocumentation
     Name: ht_synapse_hpc - Variant of ht_synapse with low memory consumption.
     SeeAlso: synapsedict, ht_synapse, static_synapse_hpc
  */
  register_connection_model< HTConnection< TargetIdentifierPtrRport > >( "ht_synapse" );
  register_connection_model< HTConnection< TargetIdentifierIndex > >( "ht_synapse_hpc" );


  /* BeginDocumentation
     Name: stdp_dopamine_synapse_hpc - Variant of stdp_dopamine_synapse with low memory consumption.
     SeeAlso: synapsedict, stdp_dopamine_synapse, static_synapse_hpc
  */
  register_connection_model< STDPDopaConnection< TargetIdentifierPtrRport > >(
    "stdp_dopamine_synapse" );
  register_connection_model< STDPDopaConnection< TargetIdentifierIndex > >(
    "stdp_dopamine_synapse_hpc" );
}

} // namespace nest
