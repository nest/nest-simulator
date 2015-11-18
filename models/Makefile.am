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
		ac_generator.h ac_generator.cpp\
		aeif_cond_alpha.h aeif_cond_alpha.cpp\
		aeif_cond_alpha_RK5.h aeif_cond_alpha_RK5.cpp\
		aeif_cond_alpha_multisynapse.h aeif_cond_alpha_multisynapse.cpp\
		aeif_cond_exp.h aeif_cond_exp.cpp\
		amat2_psc_exp.h amat2_psc_exp.cpp\
		binary_neuron.h binary_neuron_impl.h\
		cont_delay_connection.h cont_delay_connection_impl.h\
		correlation_detector.h correlation_detector.cpp\
		correlomatrix_detector.h correlomatrix_detector.cpp\
		correlospinmatrix_detector.h correlospinmatrix_detector.cpp\
		dc_generator.h dc_generator.cpp\
		gamma_sup_generator.h gamma_sup_generator.cpp\
		gap_junction.h\
		ginzburg_neuron.h ginzburg_neuron.cpp\
		hh_cond_exp_traub.h hh_cond_exp_traub.cpp\
		hh_psc_alpha.h hh_psc_alpha.cpp\
		hh_psc_alpha_gap.h hh_psc_alpha_gap.cpp\
		ht_connection.h\
		ht_neuron.h ht_neuron.cpp\
		iaf_chxk_2008.cpp iaf_chxk_2008.h\
		iaf_chs_2007.cpp iaf_chs_2007.h\
		iaf_cond_alpha.h iaf_cond_alpha.cpp\
		iaf_cond_alpha_mc.h iaf_cond_alpha_mc.cpp\
		iaf_cond_exp.h iaf_cond_exp.cpp\
		iaf_cond_exp_sfa_rr.h iaf_cond_exp_sfa_rr.cpp\
		iaf_neuron.h iaf_neuron.cpp\
		iaf_psc_alpha.h iaf_psc_alpha.cpp\
		iaf_psc_alpha_multisynapse.h iaf_psc_alpha_multisynapse.cpp\
		iaf_psc_exp_multisynapse.h iaf_psc_exp_multisynapse.cpp\
		iaf_psc_delta.h iaf_psc_delta.cpp\
		iaf_psc_exp.h iaf_psc_exp.cpp\
		iaf_tum_2000.h iaf_tum_2000.cpp\
		izhikevich.h izhikevich.cpp\
		mat2_psc_exp.h mat2_psc_exp.cpp\
		mcculloch_pitts_neuron.h mcculloch_pitts_neuron.cpp\
		mip_generator.h mip_generator.cpp\
		modelsmodule.h modelsmodule.cpp\
		multimeter.h multimeter.cpp\
		music_cont_in_proxy.h music_cont_in_proxy.cpp\
		music_event_in_proxy.h music_event_in_proxy.cpp\
		music_event_out_proxy.h music_event_out_proxy.cpp\
		music_message_in_proxy.h music_message_in_proxy.cpp\
		noise_generator.h noise_generator.cpp\
		parrot_neuron.h parrot_neuron.cpp\
		poisson_generator.h poisson_generator.cpp\
		pp_psc_delta.h pp_psc_delta.cpp\
		pp_pop_psc_delta.h pp_pop_psc_delta.cpp\
		ppd_sup_generator.h ppd_sup_generator.cpp\
		pulsepacket_generator.h pulsepacket_generator.cpp\
		quantal_stp_connection.h quantal_stp_connection_impl.h\
		sli_neuron.h sli_neuron.cpp\
		sinusoidal_poisson_generator.h sinusoidal_poisson_generator.cpp \
		sinusoidal_gamma_generator.h sinusoidal_gamma_generator.cpp \
		spike_detector.h spike_detector.cpp\
		spike_generator.h spike_generator.cpp\
		spin_detector.h spin_detector.cpp\
		static_connection.h\
		static_connection_hom_w.h\
		stdp_connection.h\
		stdp_connection_facetshw_hom.h stdp_connection_facetshw_hom_impl.h\
		stdp_connection_hom.h stdp_connection_hom.cpp\
		stdp_dopa_connection.h stdp_dopa_connection.cpp\
		stdp_connection_facetshw_hom.h stdp_connection_facetshw_hom_impl.h\
		stdp_pl_connection_hom.h stdp_pl_connection_hom.cpp\
		step_current_generator.h step_current_generator.cpp\
		tsodyks2_connection.h\
		tsodyks_connection.h\
		tsodyks_connection_hom.h tsodyks_connection_hom.cpp\
		volume_transmitter.h volume_transmitter.cpp\
		spike_dilutor.h spike_dilutor.cpp



AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/librandom\
		-I$(top_srcdir)/sli\
		-I$(top_srcdir)/nestkernel\
		@INCLTDL@ \
		@GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

nobase_pkgdata_DATA= sli/models-init.sli

EXTRA_DIST= sli README.md
