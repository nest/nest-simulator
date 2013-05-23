# Automake file for the NEST simulation kernel
#
# Marc-Oliver Gewaltig, July 2000
# Jochen Martin Eppler, February 2004
#
# modelsmodule is not to be installed, so it is build as
# a convenience library and header files are listed
# under SOURCES

defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@

noinst_LTLIBRARIES= libmodelsmodule.la

libmodelsmodule_la_CXXFLAGS= @AM_CXXFLAGS@

libmodelsmodule_la_SOURCES= \
		music_event_in_proxy.cpp music_event_in_proxy.h\
		music_event_out_proxy.cpp music_event_out_proxy.h\
		music_cont_in_proxy.cpp music_cont_in_proxy.h\
		music_message_in_proxy.cpp music_message_in_proxy.h\
		iaf_neuron.h iaf_neuron.cpp\
		iaf_psc_alpha.h iaf_psc_alpha.cpp\
		iaf_psc_alpha_multisynapse.h iaf_psc_alpha_multisynapse.cpp\
		iaf_psc_exp_multisynapse.h iaf_psc_exp_multisynapse.cpp\
		iaf_psc_delta.h iaf_psc_delta.cpp\
		iaf_psc_exp.h iaf_psc_exp.cpp\
		iaf_tum_2000.h iaf_tum_2000.cpp\
		iaf_cond_alpha.h iaf_cond_alpha.cpp\
		iaf_cond_exp.h iaf_cond_exp.cpp\
		iaf_cond_exp_sfa_rr.h iaf_cond_exp_sfa_rr.cpp\
		iaf_cond_alpha_mc.cpp iaf_cond_alpha_mc.h\
		aeif_cond_alpha.h aeif_cond_alpha.cpp\
		aeif_cond_exp.h aeif_cond_exp.cpp\
		hh_psc_alpha.h hh_psc_alpha.cpp\
		hh_cond_exp_traub.cpp hh_cond_exp_traub.h\
		mat2_psc_exp.h mat2_psc_exp.cpp\
		parrot_neuron.h parrot_neuron.cpp\
		pp_psc_delta.h pp_psc_delta.cpp\
		ac_generator.h ac_generator.cpp\
		dc_generator.h dc_generator.cpp\
		spike_generator.h spike_generator.cpp\
		poisson_generator.h poisson_generator.cpp\
		noise_generator.h noise_generator.cpp\
		mip_generator.h mip_generator.cpp\
		step_current_generator.h step_current_generator.cpp\
		pulsepacket_generator.h pulsepacket_generator.cpp\
		ppd_sup_generator.cpp ppd_sup_generator.h\
		gamma_sup_generator.cpp gamma_sup_generator.h\
		sinusoidal_gamma_generator.h sinusoidal_gamma_generator.cpp \
		spike_detector.h spike_detector.cpp\
		spin_detector.h spin_detector.cpp\
		multimeter.cpp multimeter.h\
		correlation_detector.h correlation_detector.cpp\
		volume_transmitter.h volume_transmitter.cpp\
		static_connection.h\
		static_connection_hom_wd.h\
		cont_delay_connection.h cont_delay_connection.cpp\
		tsodyks_connection.h tsodyks_connection.cpp\
		tsodyks2_connection.h tsodyks2_connection.cpp\
		stdp_connection.h stdp_connection.cpp\
		stdp_connection_hom.h stdp_connection_hom.cpp\
		stdp_connection_facetshw_hom.h stdp_connection_facetshw_hom.cpp\
		stdp_pl_connection_hom.h stdp_pl_connection_hom.cpp\
		stdp_dopa_connection.h stdp_dopa_connection.cpp\
		ht_neuron.cpp ht_neuron.h \
		ht_connection.cpp ht_connection.h \
		sinusoidal_poisson_generator.h sinusoidal_poisson_generator.cpp \
		sli_neuron.cpp sli_neuron.h\
		binary_neuron.h binary_neuron_impl.h\
		ginzburg_neuron.cpp ginzburg_neuron.h\
		mcculloch_pitts_neuron.cpp mcculloch_pitts_neuron.h\
		izhikevich.cpp izhikevich.h\
	 	modelsmodule.h modelsmodule.cpp

AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/librandom\
		-I$(top_srcdir)/sli\
		-I$(top_srcdir)/nestkernel\
		@INCLTDL@ \
		@GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

nobase_pkgdata_DATA= sli/models-init.sli

EXTRA_DIST= sli
