# Makefile file for PyNEST

EXTRA_DIST= setup.py.in do_tests.py.in \
	nest examples \
	README.PyNEST \
	pynestkernel.pyx \
	pynestkernel.pxd

BUILT_SOURCES = pynestkernel.cpp

pynestkernel.cpp: pynestkernel.pyx pynestkernel.pxd
	cd $(top_srcdir)/pynest; $(PYTHON) -c \
	"from Cython.Build import cythonize; \
	cythonize('pynestkernel.pyx', verbose=1)"

# Threading libraries should always be linked last (or, at least, after
# the standard C++ library), so that the resulting shared object doesn't
# segfault when dlopen'ed from inside Python
#
# Additionally, libtools up to 2.4.2 filter out OpenMP flags, so passing
# them directly to the complier driver improves compatibility with older
# versions

if LINK_OPENMP
THREADING_FLAGS = -XCClinker $(OPENMP_CXXFLAGS)
endif

pkgpyexec_LTLIBRARIES = pynestkernel.la

pynestkernel_la_SOURCES = pynestkernel.cpp ../nest/neststartup.cpp ../nest/neststartup.h

pynestkernel_la_LDFLAGS = -shared -module -avoid-version -export-dynamic $(THREADING_FLAGS)

pynestkernel_la_CPPFLAGS = \
	-D_IS_PYNEST \
	-I@PKGSRCDIR@/libnestutil \
	-I@PKGSRCDIR@/librandom \
	-I@PKGSRCDIR@/sli \
	-I@PKGSRCDIR@/nestkernel \
	-I@PKGBUILDDIR@/libnestutil \
	-I@PKGSRCDIR@/nest \
	-I@PKGBUILDDIR@/nest \
	-I@PKGSRCDIR@/topology \
	-I@PKGSRCDIR@/conngen \
	@GSL_CFLAGS@ \
	@MPI_INCLUDE@ \
	@MUSIC_INCLUDE@ \
	@LIBNEUROSIM_INCLUDE@ \
	$(PYNEST_CPPFLAGS)

pynestkernel_la_CXXFLAGS = $(PYNEST_CXXFLAGS)

pynestkernel_la_LIBADD= \
	@LINKED_USER_MODULES@ \
	@LINKED_MODULES@ \
	$(top_builddir)/nestkernel/libnest.la \
	$(top_builddir)/librandom/librandom.la \
	$(top_builddir)/libnestutil/libnestutil.la \
	$(top_builddir)/sli/libsli.la \
	@SLI_LIBS@ \
	@GSL_LIBS@ \
	@MPI_LIBS@ \
	@MUSIC_LIBS@ \
	@LIBNEUROSIM_LIBS@ @LIBNEUROSIM_PY_LIBS@

# See comments in ../nest/Makefile.am that explain it:
pynestkernel_la_DEPENDENCIES= \
	@LINKED_USER_MODULES@ \
	@LINKED_MODULES@ \
	$(top_builddir)/nestkernel/libnest.la \
	$(top_builddir)/librandom/librandom.la \
	$(top_builddir)/libnestutil/libnestutil.la \
	$(top_builddir)/sli/libsli.la

install-exec-hook:
	$(PYTHON) setup.py build --build-base=$(abs_builddir)/build install --prefix=$(DESTDIR)$(prefix) --install-lib=$(DESTDIR)$(pyexecdir) --install-scripts=$(DESTDIR)$(bindir) --install-data=$(DESTDIR)$(pkgdatadir)
	mkdir -p $(DESTDIR)@PKGDATADIR@/extras
	@INSTALL_PROGRAM@ do_tests.py $(DESTDIR)@PKGDATADIR@/extras/

clean-local:
	-rm -rf $(abs_builddir)/build

pkgdocdir=@PKGDOCDIR@

pkgdoc_DATA= \
	README.PyNEST

datadir=@PKGDOCDIR@/examples/pynest

data_DATA=\
	examples/balancedneuron.py \
	examples/brunel-alpha-nest.py \
	examples/brunel-alpha-numpy.py \
	examples/brunel-delta-nest.py \
	examples/BrodyHopfield.py \
	examples/brette-gerstner-fig-2c.py \
	examples/brette-gerstner-fig-3d.py \
	examples/CampbellSiegert.py \
	examples/csa_example.py \
	examples/mc_neuron.py \
	examples/multimeter.py \
	examples/one-neuron.py \
	examples/one-neuron-with-noise.py \
	examples/pulsepacket.py \
	examples/sinusoidal_poisson_generator.py\
	examples/sinusoidal_gamma_generator.py\
	examples/testiaf.py \
	examples/tsodyks_facilitating.py \
	examples/tsodyks_depressing.py \
	examples/vinit_example.py

lenoveredir=@PKGDOCDIR@/examples/pynest/LeNovere_2012

lenovere_DATA=\
	examples/LeNovere_2012/README.txt

hilltononidir=@PKGDOCDIR@/examples/pynest/HillTononi

hilltononi_DATA=\
	examples/HillTononi/ht_current.py \
	examples/HillTononi/ht_poisson.py
