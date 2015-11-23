# Automake file for the NEST simulation kernel library
#
# Marc-Oliver Gewaltig, July 2000
# Jochen Martin Eppler, February 2004
#
# Restructured June 2006, Hans Ekkehard Plesser:
#
# - directory renamed
# - main now in nest
# - neststartup has moved out of libnest, and is now part
#   of the nest_SOURCES.  Thus, libnest includes only code
#   truly pertaining to the kernel.  This is necessary to
#   allow other modules to link statically against libnest,
#   and the nest executable to be linked statically against
#   those module libs (example: libmodelmodule.a)
#
# libnestkernel is built as a convenience library and all
# headers are included in the SOURCES.

defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@

AM_CPPFLAGS=  -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/librandom\
		-I$(top_srcdir)/sli\
		@INCLTDL@ @GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

noinst_LTLIBRARIES= libnest.la

libnest_la_CXXFLAGS= @AM_CXXFLAGS@

libnest_la_SOURCES=\
		universal_data_logger_impl.h universal_data_logger.h\
		recordables_map.h\
		archiving_node.h archiving_node.cpp\
		common_synapse_properties.h common_synapse_properties.cpp\
		communicator.h communicator_impl.h communicator.cpp\
		sibling_container.h sibling_container.cpp\
		subnet.h subnet.cpp\
		connection.h\
		connection_label.h\
		common_properties_hom_w.h\
		syn_id_delay.h\
		connector_base.h connector_base.cpp\
		connector_model.h connector_model_impl.h connector_model.cpp\
		connection_manager.h connection_manager.cpp\
		connection_id.h connection_id.cpp\
		device.h device.cpp\
		dynamicloader.h dynamicloader.cpp\
		event.h event.cpp\
		event_priority.h\
		exceptions.h exceptions.cpp\
		genericmodel.h\
		gid_collection.h gid_collection.cpp\
		histentry.h histentry.cpp\
		model.h model.cpp\
		nest.h\
		nest_datums.h nest_datums.cpp\
		nest_names.cpp nest_names.h\
		nestmodule.h nestmodule.cpp\
		nest_time.h nest_time.cpp \
		nest_timeconverter.h nest_timeconverter.cpp \
		nest_timemodifier.h nest_timemodifier.cpp\
		modelrange.h modelrange.cpp\
		modelrangemanager.h modelrangemanager.cpp\
		multirange.h multirange.cpp\
		network.h network_impl.h network.cpp\
		node.h node.cpp\
		nodelist.h nodelist.cpp\
		proxynode.h proxynode.cpp\
		recording_device.h recording_device.cpp\
		pseudo_recording_device.h\
		ring_buffer.h ring_buffer.cpp\
		scheduler.h scheduler.cpp\
		spikecounter.h spikecounter.cpp\
		stimulating_device.h\
		target_identifier.h\
		sparse_node_array.h sparse_node_array.cpp\
		conn_parameter.h conn_parameter.cpp\
		conn_builder.h conn_builder.cpp\
		conn_builder_factory.h\
		music_event_handler.h music_event_handler.cpp\
		synaptic_element.h synaptic_element.cpp\
		growth_curve.h growth_curve.cpp\
		growth_curve_factory.h
libnest_la_LIBADD= @LIBLTDL@ @LIBADD_DL@

EXTRA_DIST= README.md
