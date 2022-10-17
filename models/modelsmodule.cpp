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

#include "modelsmodule.h"

// Includes from nestkernel
#include "genericmodel_impl.h"

// Generated includes:
#include "config.h"

// Neuron models
#include "aeif_cond_alpha.h"
#include "aeif_cond_alpha_multisynapse.h"
#include "aeif_cond_beta_multisynapse.h"
#include "aeif_cond_exp.h"
#include "aeif_psc_alpha.h"
#include "aeif_psc_delta.h"
#include "aeif_psc_delta_clopath.h"
#include "aeif_psc_exp.h"
#include "amat2_psc_exp.h"
#include "cm_default.h"
#include "erfc_neuron.h"
#include "gauss_rate.h"
#include "gif_cond_exp.h"
#include "gif_cond_exp_multisynapse.h"
#include "gif_pop_psc_exp.h"
#include "gif_psc_exp.h"
#include "gif_psc_exp_multisynapse.h"
#include "ginzburg_neuron.h"
#include "glif_cond.h"
#include "glif_psc.h"
#include "hh_cond_beta_gap_traub.h"
#include "hh_cond_exp_traub.h"
#include "hh_psc_alpha.h"
#include "hh_psc_alpha_clopath.h"
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
#include "iaf_psc_alpha_canon.h"
#include "iaf_psc_alpha_multisynapse.h"
#include "iaf_psc_alpha_ps.h"
#include "iaf_psc_delta.h"
#include "iaf_psc_delta_ps.h"
#include "iaf_psc_exp.h"
#include "iaf_psc_exp_htum.h"
#include "iaf_psc_exp_multisynapse.h"
#include "iaf_psc_exp_ps.h"
#include "iaf_psc_exp_ps_lossless.h"
#include "izhikevich.h"
#include "lin_rate.h"
#include "mat2_psc_exp.h"
#include "mcculloch_pitts_neuron.h"
#include "parrot_neuron.h"
#include "parrot_neuron_ps.h"
#include "pp_cond_exp_mc_urbanczik.h"
#include "pp_pop_psc_delta.h"
#include "pp_psc_delta.h"
#include "siegert_neuron.h"
#include "sigmoid_rate.h"
#include "sigmoid_rate_gg_1998.h"
#include "tanh_rate.h"
#include "threshold_lin_rate.h"

// Stimulation devices
#include "ac_generator.h"
#include "dc_generator.h"
#include "gamma_sup_generator.h"
#include "inhomogeneous_poisson_generator.h"
#include "mip_generator.h"
#include "noise_generator.h"
#include "poisson_generator.h"
#include "poisson_generator_ps.h"
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
#include "spike_dilutor.h"
#include "spike_recorder.h"
#include "spin_detector.h"
#include "volume_transmitter.h"
#include "weight_recorder.h"

// Prototypes for synapses
#include "bernoulli_synapse.h"
#include "clopath_synapse.h"
#include "common_synapse_properties.h"
#include "cont_delay_synapse.h"
#include "cont_delay_synapse_impl.h"
#include "diffusion_connection.h"
#include "gap_junction.h"
#include "ht_synapse.h"
#include "jonke_synapse.h"
#include "quantal_stp_synapse.h"
#include "quantal_stp_synapse_impl.h"
#include "rate_connection_delayed.h"
#include "rate_connection_instantaneous.h"
#include "static_synapse.h"
#include "static_synapse_hom_w.h"
#include "stdp_dopamine_synapse.h"
#include "stdp_nn_pre_centered_synapse.h"
#include "stdp_nn_restr_synapse.h"
#include "stdp_nn_symm_synapse.h"
#include "stdp_pl_synapse_hom.h"
#include "stdp_synapse.h"
#include "stdp_synapse_facetshw_hom.h"
#include "stdp_synapse_facetshw_hom_impl.h"
#include "stdp_synapse_hom.h"
#include "stdp_triplet_synapse.h"
#include "tsodyks2_synapse.h"
#include "tsodyks_synapse.h"
#include "tsodyks_synapse_hom.h"
#include "urbanczik_synapse.h"
#include "vogels_sprekeler_synapse.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model_impl.h"
#include "genericmodel.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"
#include "target_identifier.h"

#ifdef HAVE_MUSIC
#include "music_cont_in_proxy.h"
#include "music_cont_out_proxy.h"
#include "music_event_in_proxy.h"
#include "music_event_out_proxy.h"
#include "music_message_in_proxy.h"
#include "music_rate_in_proxy.h"
#include "music_rate_out_proxy.h"
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
ModelsModule::name() const
{
  return std::string( "NEST Standard Models Module" ); // Return name of the module
}

//-------------------------------------------------------------------------------------

void
ModelsModule::init( SLIInterpreter* )
{
  // rate models with input noise
  kernel().model_manager.register_node_model< gauss_rate_ipn >( "gauss_rate_ipn" );
  kernel().model_manager.register_node_model< lin_rate_ipn >( "lin_rate_ipn" );
  kernel().model_manager.register_node_model< sigmoid_rate_ipn >( "sigmoid_rate_ipn" );
  kernel().model_manager.register_node_model< sigmoid_rate_gg_1998_ipn >( "sigmoid_rate_gg_1998_ipn" );
  kernel().model_manager.register_node_model< tanh_rate_ipn >( "tanh_rate_ipn" );
  kernel().model_manager.register_node_model< threshold_lin_rate_ipn >( "threshold_lin_rate_ipn" );

  // rate models with output noise
  kernel().model_manager.register_node_model< lin_rate_opn >( "lin_rate_opn" );
  kernel().model_manager.register_node_model< tanh_rate_opn >( "tanh_rate_opn" );
  kernel().model_manager.register_node_model< threshold_lin_rate_opn >( "threshold_lin_rate_opn" );

  // rate transformer nodes
  kernel().model_manager.register_node_model< rate_transformer_gauss >( "rate_transformer_gauss" );
  kernel().model_manager.register_node_model< rate_transformer_lin >( "rate_transformer_lin" );
  kernel().model_manager.register_node_model< rate_transformer_sigmoid >( "rate_transformer_sigmoid" );
  kernel().model_manager.register_node_model< rate_transformer_sigmoid_gg_1998 >( "rate_transformer_sigmoid_gg_1998" );
  kernel().model_manager.register_node_model< rate_transformer_tanh >( "rate_transformer_tanh" );
  kernel().model_manager.register_node_model< rate_transformer_threshold_lin >( "rate_transformer_threshold_lin" );

  kernel().model_manager.register_node_model< cm_default >( "cm_default" );

  kernel().model_manager.register_node_model< iaf_chs_2007 >( "iaf_chs_2007" );
  kernel().model_manager.register_node_model< iaf_psc_alpha >( "iaf_psc_alpha" );
  kernel().model_manager.register_node_model< iaf_psc_alpha_canon >(
    "iaf_psc_alpha_canon", /*deprecation_info*/ "a future version of NEST" );
  kernel().model_manager.register_node_model< iaf_psc_alpha_multisynapse >( "iaf_psc_alpha_multisynapse" );
  kernel().model_manager.register_node_model< iaf_psc_alpha_ps >( "iaf_psc_alpha_ps" );
  kernel().model_manager.register_node_model< iaf_psc_delta >( "iaf_psc_delta" );
  kernel().model_manager.register_node_model< iaf_psc_delta_ps >( "iaf_psc_delta_ps" );
  kernel().model_manager.register_node_model< iaf_psc_exp >( "iaf_psc_exp" );
  kernel().model_manager.register_node_model< iaf_psc_exp_htum >( "iaf_psc_exp_htum" );
  kernel().model_manager.register_node_model< iaf_psc_exp_multisynapse >( "iaf_psc_exp_multisynapse" );

  kernel().model_manager.register_node_model< iaf_psc_exp_ps >( "iaf_psc_exp_ps" );
  kernel().model_manager.register_node_model< iaf_psc_exp_ps_lossless >( "iaf_psc_exp_ps_lossless" );

  kernel().model_manager.register_node_model< amat2_psc_exp >( "amat2_psc_exp" );
  kernel().model_manager.register_node_model< mat2_psc_exp >( "mat2_psc_exp" );
  kernel().model_manager.register_node_model< parrot_neuron >( "parrot_neuron" );
  kernel().model_manager.register_node_model< parrot_neuron_ps >( "parrot_neuron_ps" );
  kernel().model_manager.register_node_model< pp_psc_delta >( "pp_psc_delta" );
  kernel().model_manager.register_node_model< pp_pop_psc_delta >(
    "pp_pop_psc_delta", /*deprecation_info*/ "a future version of NEST" );
  kernel().model_manager.register_node_model< gif_psc_exp >( "gif_psc_exp" );
  kernel().model_manager.register_node_model< gif_psc_exp_multisynapse >( "gif_psc_exp_multisynapse" );
  kernel().model_manager.register_node_model< glif_psc >( "glif_psc" );

  kernel().model_manager.register_node_model< ac_generator >( "ac_generator" );
  kernel().model_manager.register_node_model< dc_generator >( "dc_generator" );
  kernel().model_manager.register_node_model< spike_generator >( "spike_generator" );
  kernel().model_manager.register_node_model< inhomogeneous_poisson_generator >( "inhomogeneous_poisson_generator" );
  kernel().model_manager.register_node_model< poisson_generator >( "poisson_generator" );
  kernel().model_manager.register_node_model< poisson_generator_ps >( "poisson_generator_ps" );
  kernel().model_manager.register_node_model< pulsepacket_generator >( "pulsepacket_generator" );
  kernel().model_manager.register_node_model< noise_generator >( "noise_generator" );
  kernel().model_manager.register_node_model< step_current_generator >( "step_current_generator" );
  kernel().model_manager.register_node_model< step_rate_generator >( "step_rate_generator" );
  kernel().model_manager.register_node_model< mip_generator >( "mip_generator" );
  kernel().model_manager.register_node_model< sinusoidal_poisson_generator >( "sinusoidal_poisson_generator" );
  kernel().model_manager.register_node_model< ppd_sup_generator >( "ppd_sup_generator" );
  kernel().model_manager.register_node_model< gamma_sup_generator >( "gamma_sup_generator" );
  kernel().model_manager.register_node_model< erfc_neuron >( "erfc_neuron" );
  kernel().model_manager.register_node_model< ginzburg_neuron >( "ginzburg_neuron" );
  kernel().model_manager.register_node_model< mcculloch_pitts_neuron >( "mcculloch_pitts_neuron" );
  kernel().model_manager.register_node_model< izhikevich >( "izhikevich" );
  kernel().model_manager.register_node_model< spike_dilutor >(
    "spike_dilutor", /*deprecation_info*/ "a future version of NEST" );

  kernel().model_manager.register_node_model< spike_recorder >( "spike_recorder" );
  kernel().model_manager.register_node_model< weight_recorder >( "weight_recorder" );
  kernel().model_manager.register_node_model< spin_detector >( "spin_detector" );
  kernel().model_manager.register_node_model< multimeter >( "multimeter" );
  kernel().model_manager.register_node_model< voltmeter >( "voltmeter" );
  kernel().model_manager.register_node_model< correlation_detector >( "correlation_detector" );
  kernel().model_manager.register_node_model< correlomatrix_detector >( "correlomatrix_detector" );
  kernel().model_manager.register_node_model< correlospinmatrix_detector >( "correlospinmatrix_detector" );
  kernel().model_manager.register_node_model< volume_transmitter >( "volume_transmitter" );

#ifdef HAVE_GSL
  kernel().model_manager.register_node_model< iaf_chxk_2008 >( "iaf_chxk_2008" );
  kernel().model_manager.register_node_model< iaf_cond_alpha >( "iaf_cond_alpha" );
  kernel().model_manager.register_node_model< iaf_cond_beta >( "iaf_cond_beta" );
  kernel().model_manager.register_node_model< iaf_cond_exp >( "iaf_cond_exp" );
  kernel().model_manager.register_node_model< iaf_cond_exp_sfa_rr >( "iaf_cond_exp_sfa_rr" );
  kernel().model_manager.register_node_model< iaf_cond_alpha_mc >( "iaf_cond_alpha_mc" );
  kernel().model_manager.register_node_model< hh_cond_beta_gap_traub >( "hh_cond_beta_gap_traub" );
  kernel().model_manager.register_node_model< hh_psc_alpha >( "hh_psc_alpha" );
  kernel().model_manager.register_node_model< hh_psc_alpha_clopath >( "hh_psc_alpha_clopath" );
  kernel().model_manager.register_node_model< hh_psc_alpha_gap >( "hh_psc_alpha_gap" );
  kernel().model_manager.register_node_model< hh_cond_exp_traub >( "hh_cond_exp_traub" );
  kernel().model_manager.register_node_model< sinusoidal_gamma_generator >( "sinusoidal_gamma_generator" );
  kernel().model_manager.register_node_model< gif_cond_exp >( "gif_cond_exp" );
  kernel().model_manager.register_node_model< gif_cond_exp_multisynapse >( "gif_cond_exp_multisynapse" );
  kernel().model_manager.register_node_model< gif_pop_psc_exp >( "gif_pop_psc_exp" );
  kernel().model_manager.register_node_model< glif_cond >( "glif_cond" );

  kernel().model_manager.register_node_model< aeif_psc_delta_clopath >( "aeif_psc_delta_clopath" );
  kernel().model_manager.register_node_model< aeif_cond_alpha >( "aeif_cond_alpha" );
  kernel().model_manager.register_node_model< aeif_cond_exp >( "aeif_cond_exp" );
  kernel().model_manager.register_node_model< aeif_psc_alpha >( "aeif_psc_alpha" );
  kernel().model_manager.register_node_model< aeif_psc_exp >( "aeif_psc_exp" );
  kernel().model_manager.register_node_model< aeif_psc_delta >( "aeif_psc_delta" );
  kernel().model_manager.register_node_model< ht_neuron >( "ht_neuron" );
  kernel().model_manager.register_node_model< aeif_cond_beta_multisynapse >( "aeif_cond_beta_multisynapse" );
  kernel().model_manager.register_node_model< aeif_cond_alpha_multisynapse >( "aeif_cond_alpha_multisynapse" );
  kernel().model_manager.register_node_model< siegert_neuron >( "siegert_neuron" );
  kernel().model_manager.register_node_model< pp_cond_exp_mc_urbanczik >( "pp_cond_exp_mc_urbanczik" );
#endif

#ifdef HAVE_MUSIC
  //// proxies for inter-application communication using MUSIC
  kernel().model_manager.register_node_model< music_event_in_proxy >( "music_event_in_proxy" );
  kernel().model_manager.register_node_model< music_event_out_proxy >( "music_event_out_proxy" );
  kernel().model_manager.register_node_model< music_cont_in_proxy >( "music_cont_in_proxy" );
  kernel().model_manager.register_node_model< music_cont_out_proxy >( "music_cont_out_proxy" );
  kernel().model_manager.register_node_model< music_message_in_proxy >( "music_message_in_proxy" );
  kernel().model_manager.register_node_model< music_rate_in_proxy >( "music_rate_in_proxy" );
  kernel().model_manager.register_node_model< music_rate_out_proxy >( "music_rate_out_proxy" );
#endif

  // register all synapse models
  register_connection_model< bernoulli_synapse >( "bernoulli_synapse" );
  register_connection_model< clopath_synapse >(
    "clopath_synapse", default_connection_model_flags | RegisterConnectionModelFlags::REQUIRES_CLOPATH_ARCHIVING );
  register_connection_model< cont_delay_synapse >( "cont_delay_synapse" );
  register_connection_model< ht_synapse >( "ht_synapse" );
  register_connection_model< jonke_synapse >( "jonke_synapse" );
  register_connection_model< quantal_stp_synapse >( "quantal_stp_synapse" );
  register_connection_model< static_synapse >( "static_synapse" );
  register_connection_model< static_synapse_hom_w >( "static_synapse_hom_w" );
  register_connection_model< stdp_synapse >( "stdp_synapse" );
  register_connection_model< stdp_synapse_hom >( "stdp_synapse_hom" );
  register_connection_model< stdp_dopamine_synapse >( "stdp_dopamine_synapse" );
  register_connection_model< stdp_facetshw_synapse_hom >( "stdp_facetshw_synapse_hom" );
  register_connection_model< stdp_nn_restr_synapse >( "stdp_nn_restr_synapse" );
  register_connection_model< stdp_nn_symm_synapse >( "stdp_nn_symm_synapse" );
  register_connection_model< stdp_nn_pre_centered_synapse >( "stdp_nn_pre_centered_synapse" );
  register_connection_model< stdp_pl_synapse_hom >( "stdp_pl_synapse_hom" );
  register_connection_model< stdp_triplet_synapse >( "stdp_triplet_synapse" );
  register_connection_model< tsodyks_synapse >( "tsodyks_synapse" );
  register_connection_model< tsodyks_synapse_hom >( "tsodyks_synapse_hom" );
  register_connection_model< tsodyks2_synapse >( "tsodyks2_synapse" );
  register_connection_model< urbanczik_synapse >(
    "urbanczik_synapse", default_connection_model_flags | RegisterConnectionModelFlags::REQUIRES_URBANCZIK_ARCHIVING );
  register_connection_model< vogels_sprekeler_synapse >( "vogels_sprekeler_synapse" );

  // register secondary connection models
  register_secondary_connection_model< GapJunction >(
    "gap_junction", RegisterConnectionModelFlags::REQUIRES_SYMMETRIC | RegisterConnectionModelFlags::SUPPORTS_WFR );

  register_secondary_connection_model< RateConnectionInstantaneous >(
    "rate_connection_instantaneous", RegisterConnectionModelFlags::SUPPORTS_WFR );
  register_secondary_connection_model< RateConnectionDelayed >(
    "rate_connection_delayed", RegisterConnectionModelFlags::HAS_DELAY );
  register_secondary_connection_model< DiffusionConnection >(
    "diffusion_connection", RegisterConnectionModelFlags::SUPPORTS_WFR );
}

} // namespace nest
