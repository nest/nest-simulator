# nestkernel/CMakeLists.txt
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

set ( nestkernel_sources
      universal_data_logger_impl.h universal_data_logger.h
      recordables_map.h
      archiving_node.h archiving_node.cpp
      clopath_archiving_node.h clopath_archiving_node.cpp
      urbanczik_archiving_node.h urbanczik_archiving_node_impl.h
      common_synapse_properties.h common_synapse_properties.cpp
      connection.h
      connection_label.h
      common_properties_hom_w.h
      syn_id_delay.h
      connector_base.h connector_base_impl.h
      connector_model.h connector_model_impl.h connector_model.cpp
      connection_id.h connection_id.cpp
      deprecation_warning.h deprecation_warning.cpp
      device.h device.cpp
      device_node.h
      event.h event.cpp
      exceptions.h exceptions.cpp
      genericmodel.h genericmodel_impl.h
      node_collection.h node_collection.cpp
      generic_factory.h
      histentry.h histentry.cpp
      model.h model.cpp
      model_manager.h model_manager_impl.h model_manager.cpp
      nest_types.h
      nest_names.h
      nest_time.h nest_time.cpp
      nest_timeconverter.h nest_timeconverter.cpp
      modelrange.h modelrange.cpp
      modelrange_manager.h modelrange_manager.cpp
      node.h node.cpp
      parameter.h parameter.cpp
      per_thread_bool_indicator.h per_thread_bool_indicator.cpp
      proxynode.h proxynode.cpp
      random_generators.h
      recording_device.h recording_device.cpp
      pseudo_recording_device.h
      ring_buffer.h ring_buffer_impl.h ring_buffer.cpp
      secondary_event.h
      slice_ring_buffer.cpp slice_ring_buffer.h
      spikecounter.h spikecounter.cpp
      stimulation_device.h stimulation_device.cpp
      target_identifier.h
      sparse_node_array.h sparse_node_array.cpp
      conn_parameter.h conn_parameter.cpp
      conn_builder.h conn_builder_impl.h conn_builder.cpp
      conn_builder_factory.h
      conn_builder_conngen.h conn_builder_conngen.cpp
      music_event_handler.h music_event_handler.cpp
      music_rate_in_handler.h music_rate_in_handler.cpp
      music_manager.cpp music_manager.h
      nest.h nest_impl.h nest.cpp
      ../models/iaf_psc_alpha.h ../models/iaf_psc_alpha.cpp
      ../models/iaf_psc_delta.h ../models/iaf_psc_delta.cpp
      ../models/iaf_psc_exp.h ../models/iaf_psc_exp.cpp
      ../models/iaf_psc_exp_multisynapse.h ../models/iaf_psc_exp_multisynapse.cpp
      ../models/parrot_neuron.h ../models/parrot_neuron.cpp
      ../models/weight_recorder.h ../models/weight_recorder.cpp
      ../models/stdp_pl_synapse_hom.h ../models/stdp_pl_synapse_hom.cpp
      ../models/stdp_dopamine_synapse.h ../models/stdp_dopamine_synapse.cpp
      ../models/stdp_synapse_hom.h ../models/stdp_synapse_hom.cpp
      ../models/volume_transmitter.h ../models/volume_transmitter.cpp
      ../models/tsodyks_synapse_hom.h ../models/tsodyks_synapse_hom.cpp
      ../models/ac_generator.h ../models/ac_generator.cpp
      ../models/dc_generator.h ../models/dc_generator.cpp
      ../models/spike_generator.h ../models/spike_generator.cpp
      ../models/spike_recorder.h ../models/spike_recorder.cpp
      ../models/poisson_generator.h ../models/poisson_generator.cpp
      ../models/poisson_generator_ps.h ../models/poisson_generator_ps.cpp
      ../models/multimeter.h ../models/multimeter.cpp
      ../models/noise_generator.h ../models/noise_generator.cpp
      ../models/aeif_cond_alpha.h ../models/aeif_cond_alpha.cpp
      ../models/aeif_cond_alpha_multisynapse.h ../models/aeif_cond_alpha_multisynapse.cpp
      ../models/aeif_cond_beta_multisynapse.h ../models/aeif_cond_beta_multisynapse.cpp
      ../models/aeif_psc_delta_clopath.h ../models/aeif_psc_delta_clopath.cpp
      ../models/cm_default.h ../models/cm_default.cpp
      ../models/cm_tree.h ../models/cm_tree.cpp
      ../models/cm_compartmentcurrents.h ../models/cm_compartmentcurrents.cpp
      ../models/erfc_neuron.h ../models/erfc_neuron.cpp
      ../models/glif_cond.h ../models/glif_cond.cpp
      ../models/glif_psc.h ../models/glif_psc.cpp
      ../models/hh_psc_alpha_gap.h ../models/hh_psc_alpha_gap.cpp
      ../models/ht_neuron.h ../models/ht_neuron.cpp
      ../models/iaf_cond_alpha_mc.h ../models/iaf_cond_alpha_mc.cpp
      ../models/pp_psc_delta.h ../models/pp_psc_delta.cpp
      ../models/lin_rate.h ../models/lin_rate.cpp
      ../models/tanh_rate.h ../models/tanh_rate.cpp
      ../models/threshold_lin_rate.h ../models/threshold_lin_rate.cpp
      ../models/iaf_cond_alpha.h ../models/iaf_cond_alpha.cpp
      ../models/parrot_neuron_ps.h ../models/parrot_neuron_ps.cpp
      ../models/step_rate_generator.h ../models/step_rate_generator.cpp
      ../models/step_current_generator.h ../models/step_current_generator.cpp
      ../models/hh_psc_alpha_clopath.h ../models/hh_psc_alpha_clopath.cpp
      ../models/iaf_cond_exp.h ../models/iaf_cond_exp.cpp
      ../models/aeif_cond_exp.h ../models/aeif_cond_exp.cpp
      ../models/aeif_psc_alpha.h ../models/aeif_psc_alpha.cpp
      ../models/aeif_psc_delta.h ../models/aeif_psc_delta.cpp
      ../models/aeif_psc_exp.h ../models/aeif_psc_exp.cpp
      ../models/spin_detector.h ../models/spin_detector.cpp
      ../models/pp_cond_exp_mc_urbanczik.h ../models/pp_cond_exp_mc_urbanczik.cpp
      ../models/siegert_neuron.h ../models/siegert_neuron.cpp
      ../models/sigmoid_rate_gg_1998.h ../models/sigmoid_rate_gg_1998.cpp
      ../models/gap_junction.h
      ../models/rate_connection_instantaneous.h
      ../models/rate_connection_delayed.h
      ../models/diffusion_connection.h

      synaptic_element.h synaptic_element.cpp
      growth_curve.h growth_curve.cpp
      growth_curve_factory.h
      kernel_manager.h kernel_manager.cpp
      vp_manager.h vp_manager_impl.h vp_manager.cpp
      io_manager.h io_manager_impl.h io_manager.cpp
      mpi_manager.h mpi_manager_impl.h mpi_manager.cpp
      simulation_manager.h simulation_manager.cpp
      connection_manager.h connection_manager_impl.h connection_manager.cpp
      sp_manager.h sp_manager_impl.h sp_manager.cpp
      delay_checker.h delay_checker.cpp
      random_manager.h random_manager.cpp
      event_delivery_manager.h event_delivery_manager_impl.h
      event_delivery_manager.cpp
      node_manager.h node_manager.cpp
      logging_manager.h logging_manager.cpp
      recording_backend.h recording_backend.cpp
      recording_backend_ascii.h recording_backend_ascii.cpp
      recording_backend_memory.h recording_backend_memory.cpp
      recording_backend_screen.h recording_backend_screen.cpp
      manager_interface.h
      target_table.h target_table.cpp
      target_table_devices.h target_table_devices.cpp target_table_devices_impl.h
      target.h target_data.h static_assert.h
      send_buffer_position.h
      source.h
      source_table.h source_table.cpp
      source_table_position.h
      spike_data.h
      structural_plasticity_node.h structural_plasticity_node.cpp
      connection_creator.h connection_creator.cpp connection_creator_impl.h
      free_layer.h
      grid_layer.h
      grid_mask.h
      layer.h layer.cpp layer_impl.h
      mask.h mask.cpp mask_impl.h
      ntree.h ntree_impl.h
      position.h
      spatial.h spatial.cpp
      stimulation_backend.h
      )


if ( HAVE_SIONLIB )
  set( nestkernel_sources
       ${nestkernel_sources}
       recording_backend_sionlib.h recording_backend_sionlib.cpp
       )
endif ()

if ( HAVE_MPI )
  set( nestkernel_sources
    ${nestkernel_sources}
    recording_backend_mpi.h recording_backend_mpi.cpp
    stimulation_backend_mpi.h stimulation_backend_mpi.cpp
  )
endif ()


# Prevent problems with Conda path substitution (see #2348)
set_source_files_properties( dynamicloader.cpp PROPERTIES COMPILE_OPTIONS "-O0" )


add_library( nestkernel ${nestkernel_sources} )
set_target_properties( nestkernel
    PROPERTIES
    VERSION ${NEST_VERSION}
    SOVERSION 3
    )
target_link_libraries( nestkernel
    nestutil sli_lib
    ${LTDL_LIBRARIES} ${MPI_CXX_LIBRARIES} ${MUSIC_LIBRARIES} ${SIONLIB_LIBRARIES} ${LIBNEUROSIM_LIBRARIES}
    )

target_include_directories( nestkernel PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/nestkernel/spatial
    ${PROJECT_SOURCE_DIR}/models
    )

install( TARGETS nestkernel
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/nest
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/nest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

FILTER_HEADERS("${nestkernel_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
