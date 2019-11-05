# models/CMakeLists.txt
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

set( models_sources
    ac_generator.h ac_generator.cpp
    aeif_cond_alpha.h aeif_cond_alpha.cpp
    aeif_cond_alpha_multisynapse.h aeif_cond_alpha_multisynapse.cpp
    aeif_cond_beta_multisynapse.h aeif_cond_beta_multisynapse.cpp
    aeif_cond_exp.h aeif_cond_exp.cpp
    aeif_psc_alpha.h aeif_psc_alpha.cpp
    aeif_psc_exp.h aeif_psc_exp.cpp
    aeif_psc_delta.h aeif_psc_delta.cpp
    aeif_psc_delta_clopath.h aeif_psc_delta_clopath.cpp
    amat2_psc_exp.h amat2_psc_exp.cpp
    bernoulli_connection.h
    binary_neuron.h
    clopath_connection.h
    cont_delay_connection.h cont_delay_connection_impl.h
    correlation_detector.h correlation_detector.cpp
    correlomatrix_detector.h correlomatrix_detector.cpp
    correlospinmatrix_detector.h correlospinmatrix_detector.cpp
    dc_generator.h dc_generator.cpp
    diffusion_connection.h
    erfc_neuron.h erfc_neuron.cpp
    gamma_sup_generator.h gamma_sup_generator.cpp
    gap_junction.h
    gauss_rate.h gauss_rate.cpp
    gif_psc_exp.h gif_psc_exp.cpp
    gif_psc_exp_multisynapse.h gif_psc_exp_multisynapse.cpp
    gif_cond_exp.h gif_cond_exp.cpp
    gif_cond_exp_multisynapse.h gif_cond_exp_multisynapse.cpp
    gif_pop_psc_exp.h gif_pop_psc_exp.cpp
    ginzburg_neuron.h ginzburg_neuron.cpp
    hh_cond_exp_traub.h hh_cond_exp_traub.cpp
    hh_cond_beta_gap_traub.h hh_cond_beta_gap_traub.cpp
    hh_psc_alpha.h hh_psc_alpha.cpp
    hh_psc_alpha_clopath.h hh_psc_alpha_clopath.cpp
    hh_psc_alpha_gap.h hh_psc_alpha_gap.cpp
    ht_connection.h
    ht_neuron.h ht_neuron.cpp
    iaf_chxk_2008.cpp iaf_chxk_2008.h
    iaf_chs_2007.cpp iaf_chs_2007.h
    iaf_cond_alpha.h iaf_cond_alpha.cpp
    iaf_cond_alpha_mc.h iaf_cond_alpha_mc.cpp
    iaf_cond_beta.h iaf_cond_beta.cpp
    iaf_cond_exp.h iaf_cond_exp.cpp
    iaf_cond_exp_sfa_rr.h iaf_cond_exp_sfa_rr.cpp
    iaf_psc_alpha.h iaf_psc_alpha.cpp
    iaf_psc_alpha_multisynapse.h iaf_psc_alpha_multisynapse.cpp
    iaf_psc_exp_multisynapse.h iaf_psc_exp_multisynapse.cpp
    iaf_psc_delta.h iaf_psc_delta.cpp
    iaf_psc_exp.h iaf_psc_exp.cpp
    iaf_tum_2000.h iaf_tum_2000.cpp
    izhikevich.h izhikevich.cpp
    lin_rate.h lin_rate.cpp
    mat2_psc_exp.h mat2_psc_exp.cpp
    mcculloch_pitts_neuron.h mcculloch_pitts_neuron.cpp
    mip_generator.h mip_generator.cpp
    modelsmodule.h modelsmodule.cpp
    multimeter.h multimeter.cpp
    music_cont_in_proxy.h music_cont_in_proxy.cpp
    music_cont_out_proxy.h music_cont_out_proxy.cpp
    music_event_in_proxy.h music_event_in_proxy.cpp
    music_event_out_proxy.h music_event_out_proxy.cpp
    music_message_in_proxy.h music_message_in_proxy.cpp
    noise_generator.h noise_generator.cpp
    parrot_neuron.h parrot_neuron.cpp
    inhomogeneous_poisson_generator.h inhomogeneous_poisson_generator.cpp
    poisson_generator.h poisson_generator.cpp
    pp_psc_delta.h pp_psc_delta.cpp
    pp_pop_psc_delta.h pp_pop_psc_delta.cpp
    ppd_sup_generator.h ppd_sup_generator.cpp
    pulsepacket_generator.h pulsepacket_generator.cpp
    quantal_stp_connection.h quantal_stp_connection_impl.h
    rate_connection_delayed.h
    rate_connection_instantaneous.h
    rate_neuron_opn.h rate_neuron_opn_impl.h
    rate_neuron_ipn.h rate_neuron_ipn_impl.h
    rate_transformer_node.h rate_transformer_node_impl.h
    siegert_neuron.h siegert_neuron.cpp
    sigmoid_rate.h sigmoid_rate.cpp
    sigmoid_rate_gg_1998.h sigmoid_rate_gg_1998.cpp
    sinusoidal_poisson_generator.h sinusoidal_poisson_generator.cpp
    sinusoidal_gamma_generator.h sinusoidal_gamma_generator.cpp
    spike_detector.h spike_detector.cpp
    spike_generator.h spike_generator.cpp
    spin_detector.h spin_detector.cpp
    static_connection.h
    static_connection_hom_w.h
    stdp_connection.h
    stdp_nn_pre-centered_connection.h
    stdp_nn_restr_connection.h
    stdp_nn_symm_connection.h
    stdp_connection_facetshw_hom.h stdp_connection_facetshw_hom_impl.h
    stdp_connection_hom.h stdp_connection_hom.cpp
    stdp_dopa_connection.h stdp_dopa_connection.cpp
    stdp_connection_facetshw_hom.h stdp_connection_facetshw_hom_impl.h
    stdp_pl_connection_hom.h stdp_pl_connection_hom.cpp
    stdp_triplet_connection.h
    step_current_generator.h step_current_generator.cpp
    step_rate_generator.h step_rate_generator.cpp
    tanh_rate.h tanh_rate.cpp
    threshold_lin_rate.h threshold_lin_rate.cpp
    tsodyks2_connection.h
    tsodyks_connection.h
    tsodyks_connection_hom.h tsodyks_connection_hom.cpp
    volume_transmitter.h volume_transmitter.cpp
    vogels_sprekeler_connection.h
    weight_recorder.h weight_recorder.cpp
    spike_dilutor.h spike_dilutor.cpp
    )

add_library( models ${models_sources} )
target_link_libraries( models nestutil sli_lib nestkernel random )

target_include_directories( models PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    )

install( TARGETS models
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

FILTER_HEADERS("${models_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
