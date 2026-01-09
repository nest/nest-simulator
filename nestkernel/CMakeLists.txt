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
      universal_data_logger.h
      recordables_map.h
      archiving_node.h archiving_node.cpp
      clopath_archiving_node.h clopath_archiving_node.cpp
      urbanczik_archiving_node.h
      eprop_archiving_node.h eprop_archiving_node_impl.h
      eprop_archiving_node_readout.h eprop_archiving_node_readout_impl.h
      eprop_archiving_node_recurrent.h eprop_archiving_node_recurrent_impl.h
      common_synapse_properties.h common_synapse_properties.cpp
      connection.h connection_impl.h
      connection_label.h connection_label_impl.h
      common_properties_hom_w.h common_properties_hom_w.cpp
      syn_id_delay.h syn_id_delay.cpp
      connector_base.h connector_base.cpp connector_base_impl.h
      connector_model.h connector_model.cpp connector_model_impl.h
      connection_id.h connection_id.cpp
      deprecation_warning.h deprecation_warning.cpp
      device.h device.cpp
      device_node.h device_node.cpp
      module_manager.h module_manager.cpp
      event.h event.cpp
      exceptions.h exceptions.cpp
      genericmodel.h
      node_collection.h node_collection.cpp
      generic_factory.h
      generic_factory_impl.h
      histentry.h histentry.cpp
      model.h model.cpp
      model_manager.h model_manager.cpp model_manager_impl.h
      nest_datums.h nest_datums.cpp
      nest_names.cpp nest_names.h
      nestmodule.h nestmodule.cpp
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
      pseudo_recording_device.h pseudo_recording_device.cpp
      ring_buffer.h ring_buffer.cpp ring_buffer_impl.h
      secondary_event.h secondary_event.cpp secondary_event_impl.h
      slice_ring_buffer.cpp slice_ring_buffer.h
      spikecounter.h spikecounter.cpp
      stimulation_device.h stimulation_device.cpp
      target_identifier.h target_identifier.cpp
      sparse_node_array.h sparse_node_array.cpp
      conn_parameter.h conn_parameter.cpp
      conn_builder.h conn_builder.cpp
      conn_builder_factory.h
      conn_builder_factory.cpp
      conn_builder_factory_impl.h
      conn_builder_conngen.h conn_builder_conngen.cpp
      sonata_connector.h sonata_connector.cpp
      music_event_handler.h music_event_handler.cpp
      music_rate_in_handler.h music_rate_in_handler.cpp
      music_manager.cpp music_manager.h
      nest.h nest.cpp nest_impl.h
      synaptic_element.h synaptic_element.cpp
      growth_curve.h growth_curve.cpp
      growth_curve_factory.h
      kernel_manager.h kernel_manager.cpp
      vp_manager.h vp_manager.cpp
      io_manager.h io_manager.cpp
      mpi_manager.h mpi_manager.cpp mpi_manager_impl.h
      simulation_manager.h simulation_manager.cpp
      connection_manager.h connection_manager.cpp connection_manager_impl.h
      sp_manager.h sp_manager.cpp
      delay_checker.h delay_checker.cpp
      random_manager.h random_manager.cpp
      event_delivery_manager.h
      event_delivery_manager_impl.h
      event_delivery_manager.cpp
      node_manager.h node_manager.cpp
      logging_manager.h logging_manager.cpp
      recording_backend.h recording_backend.cpp
      recording_backend_ascii.h recording_backend_ascii.cpp
      recording_backend_memory.h recording_backend_memory.cpp
      recording_backend_screen.h recording_backend_screen.cpp
      manager_interface.h
      target_table.h target_table.cpp
      target_table_devices.h target_table_devices.cpp
      target.h target.cpp
      static_assert.h
      target_data.h target_data.cpp
      send_buffer_position.h send_buffer_position.cpp
      source.h source.cpp
      source_table.h source_table.cpp
      source_table_position.h source_table_position.cpp
      spike_data.h spike_data.cpp
      structural_plasticity_node.h structural_plasticity_node.cpp
      connection_creator.h connection_creator.cpp
      free_layer.h free_layer_impl.h
      grid_layer.h grid_layer_impl.h
      grid_mask.h grid_mask_impl.h grid_mask.cpp
      layer.h layer.cpp layer_impl.h
      mask.h mask.cpp mask_impl.h
      ntree.h ntree.cpp ntree_impl.h
      position.h position_impl.h
      spatial.h spatial.cpp
      stimulation_backend.h
      buffer_resize_log.h buffer_resize_log.cpp
      nest_extension_interface.h
      stopwatch.h
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


# Prevent problems with Mamba path substitution (see #2348)
set_source_files_properties( dynamicloader.cpp PROPERTIES COMPILE_OPTIONS "-O0" )


add_library( nestkernel STATIC ${nestkernel_sources} )
set_target_properties( nestkernel
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )

target_link_libraries( nestkernel
    nestutil sli_lib models
    ${LTDL_LIBRARIES} ${MPI_CXX_LIBRARIES} ${MUSIC_LIBRARIES} ${SIONLIB_LIBRARIES} ${LIBNEUROSIM_LIBRARIES} ${HDF5_LIBRARIES}
    )

target_include_directories( nestkernel PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/models
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/nestkernel/spatial
    )

FILTER_HEADERS("${nestkernel_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
