# Automake file for the Precise Module
#
# Hans Ekkehard Plesser, June 2006 based on
# Marc-Oliver Gewaltig, July 2000
# Jochen Martin Eppler, February 2004
#
# Precisemodule is built as a convenience library,
# with header files included in SOURCES

defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@

noinst_LTLIBRARIES= libprecisemodule.la

libprecisemodule_la_CXXFLAGS= @AM_CXXFLAGS@

libprecisemodule_la_SOURCES=\
	slice_ring_buffer.cpp   slice_ring_buffer.h\
	iaf_psc_delta_canon.cpp iaf_psc_delta_canon.h\
	iaf_psc_alpha_canon.cpp iaf_psc_alpha_canon.h\
	iaf_psc_alpha_presc.cpp iaf_psc_alpha_presc.h\
	iaf_psc_exp_ps.cpp      iaf_psc_exp_ps.h\
	poisson_generator_ps.cpp poisson_generator_ps.h\
	parrot_neuron_ps.cpp parrot_neuron_ps.h\
	precisemodule.cpp precisemodule.h




AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/librandom\
		-I$(top_srcdir)/sli\
		-I$(top_srcdir)/nestkernel\
		@INCLTDL@ \
		@GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@
