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
		multirange.h multirange.cpp\
		connection.h connection.cpp\
		connection_het_wd.h connection_het_wd.cpp\
		connection_hom_wd.h connection_hom_wd.cpp\
		connection_manager.h connection_manager.cpp\
		connectiondatum.h connectiondatum.cpp\
		connection_id.h connection_id.cpp\
		connector.h\
		connector_model.h connector_model.cpp\
		device.h device.cpp\
		dynamicloader.h dynamicloader.cpp\
		event.h event.cpp\
		event_priority.h\
		exceptions.h exceptions.cpp\
		generic_connector.h\
		generic_connector_model.h\
		genericmodel.h\
		bg_get_mem.c\
		histentry.h histentry.cpp\
		model.h model.cpp\
		nest.h\
		nest_names.cpp nest_names.h\
		nestmodule.h nestmodule.cpp\
		nest_time.h nest_time.cpp \
		nest_timeconverter.h nest_timeconverter.cpp \
		nest_timemodifier.h nest_timemodifier.cpp\
		net_thread.h net_thread.cpp\
		modelrange.h modelrange.cpp\
		modelrangemanager.h modelrangemanager.cpp\
		network.h network.cpp\
		node.h node.cpp\
		nodelist.h nodelist.cpp\
		proxynode.h proxynode.cpp\
		recording_device.h recording_device.cpp\
		pseudo_recording_device.h\
		ring_buffer.h ring_buffer.cpp\
		scheduler.h scheduler.cpp\
		spikecounter.h spikecounter.cpp\
		stimulating_device.h\
		music_event_handler.h music_event_handler.cpp

libnest_la_LIBADD= @LIBLTDL@ @LIBADD_DL@

AM_CPPFLAGS=  -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/librandom\
		-I$(top_srcdir)/sli\
		@INCLTDL@ @GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@
