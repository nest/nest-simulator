# Makefile file for PyNEST
#
# author: Moritz Helias, november 2005
# moritz@bccn.uni-freiburg.de
# ported for prerelease 1.9.0, Moritz Helias, june 2006
#
# Extended and partly rewritten at the CodeJam 2007
# Jochen Martin Eppler, Marc-Oliver Gewaltig, Moritz Helias
#
# Rewritten to make use of Python's distutils package, 01/31/08
# Sven Schrader, Jochen Martin Eppler
#

defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@

EXTRA_DIST= setup.py.in do_tests.py.in \
	nest examples \
	README.PyNEST \
	pynestkernel.cpp \
	checkpython.py \
	pydatum.h \
	pydatum.cpp \
	datumtopythonconverter.h \
	datumtopythonconverter.cpp \
	pynestpycsa.h \
	pynestpycsa.cpp

# 'all' does sometimes miss changes, therefore we 'clean' first
all:	clean
	@PYTHON_EXEC@ setup.py build

install-exec-hook:
	@PYTHON_EXEC@ setup.py install --prefix=$(DESTDIR)@PYNEST_PREFIX@
	if test `echo @PYTHON_VERSION@ | cut -d. -f2` -ge 6; then \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	  cd $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	     ln -f -s ../site-packages/nest; \
	  cd $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	     ln -f -s ../site-packages/PyNEST*.egg-info; \
	fi

install-data-hook: install-exec
	@INSTALL_PROGRAM@ -m 644 @PKGSRCDIR@/pynest/README.PyNEST $(DESTDIR)@PKGDOCDIR@/
	mkdir -p $(DESTDIR)@PKGDATADIR@/extras
	@INSTALL_PROGRAM@ do_tests.py $(DESTDIR)@PKGDATADIR@/extras/
	for dir in @SLI_SUBDIRS@; do \
	  $(MAKE) -k -C @PKGBUILDDIR@/$$dir install-pynest-component 2> /dev/null || true ; done

clean:
	rm -rf build

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
	examples/LeNovere_2012/brunel2000_classes.py \
	examples/LeNovere_2012/brunel2000_interactive.py \
	examples/LeNovere_2012/brunel2000_rand_plastic.py \
	examples/LeNovere_2012/brunel2000_rand.py \
	examples/LeNovere_2012/one_neuron_with_sine_wave.py


