# Automake file for the NEST general purpose library - libnestutil
#
#
# Jochen Martin Eppler, February 2004
#

MAKEFLAGS=@MAKE_FLAGS@

noinst_LTLIBRARIES= libnestutil.la

libnestutil_la_CXXFLAGS=  @SLI_CXXBACKEND@ @AM_CXXFLAGS@

## sources and headers for libnestutil

libnestutil_la_SOURCES= \
		stopwatch.h stopwatch.cpp \
		numerics.h numerics.cpp \
		allocator.h allocator.cpp \
		instance.h \
		lockptr.h \
		sparseconfig.h \
		template_util.h \
		type_traits.h \
		libc_allocator_with_realloc.h \
		hashtable-common.h \
		sparsetable.h \
		compose.hpp


common_cppflags= -I$(top_srcdir)/librandom \
		-I$(top_srcdir)/nestkernel \
		-I$(top_srcdir)/sli \
		@INCLTDL@ @MUSIC_INCLUDE@ @MPI_INCLUDE@


# GSL_CFLAGS are required to provide
# access to the GSL headers
#  Diesmann, 6.4.04
if GSL_1_2_AVAILABLE
  AM_CPPFLAGS= $(common_cppflags) \
               @GSL_CFLAGS@
else
  AM_CPPFLAGS= $(common_cppflags)
endif

EXTRA_DIST= COPYING.compose \
	config.h.in \
	sliconfig.h.in
