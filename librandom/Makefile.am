# Automake file for the NEST random number library
#
# Marc-Oliver Gewaltig, October 2000
# HONDA R&D EUROPE DEUTSCHLAND (GmbH)
#
# updated for GSL
#  HEP,       7/2002
#  Diesmann,  8/2002
#  HEP,       1/2008
#
# librandom is built as a convenience library.
#
# Since almost all headers of this library should not
# be installed, all headers are for now included in 
# SOURCES

MAKEFLAGS=@MAKE_FLAGS@

noinst_PROGRAMS = randomtest
noinst_LTLIBRARIES= librandom.la

AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
		-I$(top_srcdir)/nestkernel\
		-I$(top_srcdir)/sli\
        @GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

## sources and headers for librandom
librandom_la_SOURCES=\
		exp_randomdev.h \
		knuthlfg.h knuthlfg.cpp \
		mt19937.h mt19937.cpp \
		random_datums.h \
		random_numbers.h random_numbers.cpp \
		randomgen.h randomgen.cpp \
		binomial_randomdev.h binomial_randomdev.cpp \
		exp_randomdev.cpp \
		gamma_randomdev.h gamma_randomdev.cpp \
		normal_randomdev.h normal_randomdev.cpp \
		poisson_randomdev.h poisson_randomdev.cpp \
		randomdev.h randomdev.cpp \
		uniformint_randomdev.h uniformint_randomdev.cpp \
		gslrandomgen.h gslrandomgen.cpp \
		gsl_binomial_randomdev.h gsl_binomial_randomdev.cpp


librandom_la_CXXFLAGS=  @SLI_CXXBACKEND@ @AM_CXXFLAGS@

# stuff relating to randomtest program ------------------------

randomtest_SOURCES= randomtest.cpp

if GSL_1_2_AVAILABLE

  randomtest_LDADD= librandom.la\
	            $(top_builddir)/sli/libsli.la \
	            $(top_builddir)/libnestutil/libnestutil.la \
                    @SLI_LIBS@\
	   	    @MPI_LIBS@\
                    @GSL_LIBS@

  randomtest_AM_CPPFLAGS= @GSL_CFLAGS@

else

  randomtest_LDADD= librandom.la\
	            $(top_builddir)/sli/libsli.la \
	            $(top_builddir)/libnestutil/libnestutil.la \
                    @SLI_LIBS@\
	   	    @MPI_LIBS@

endif
