# Automake file for Topology module
#
# Headers from this directory are not to be installed upon
# make install. They are therefore included in _SOURCES.

# BEGIN: do not change these lines ---------------------------------------
defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@
# END  : do not change these lines ---------------------------------------

EXTRA_DIST= testsuite sli doc pynest examples README.md

noinst_LTLIBRARIES= libtopologymodule.la

# Add one line for each h-cpp-file pair containing code for your
# module. Remember the \ at the end of each but the last line!
libtopologymodule_la_SOURCES= \
	topologymodule.h \
	topologymodule.cpp \
	topology_names.h \
	topology_names.cpp \
	connection_creator.h \
	connection_creator_impl.h \
	connection_creator.cpp \
	generic_factory.h \
	position.h \
	layer.h \
	layer_impl.h \
	layer.cpp \
	free_layer.h \
	grid_layer.h \
	mask.h \
	mask_impl.h \
	grid_mask.h \
	ntree.h \
	ntree_impl.h \
	vose.h \
	vose.cpp \
	parameter.h \
	parameter.cpp \
	selector.h \
	selector.cpp

nobase_pkgdata_DATA=\
	sli/topology-interface.sli

if HAVE_PYTHON

install-exec-hook:
	$(PYTHON) setup.py build --build-base=$(abs_builddir)/build install --prefix=$(DESTDIR)$(prefix) --install-lib=$(DESTDIR)$(pyexecdir) --install-scripts=$(DESTDIR)$(bindir) --install-data=$(DESTDIR)$(pkgdatadir)

examplesdir=@PKGDOCDIR@/examples/topology

dist_examples_DATA=\
	examples/README \
	examples/conncomp.py \
	examples/conncon_sources.py \
	examples/conncon_targets.py \
	examples/connex.py \
	examples/connex_ew.py \
	examples/ctx_2n.py \
	examples/gaussex.py \
	examples/grid_iaf.py \
	examples/grid_iaf_irr.py \
	examples/grid_iaf_oc.py \
	examples/hill_tononi_Vp.py \
	examples/test_3d.py \
	examples/test_3d_exp.py \
	examples/test_3d_gauss.py

endif

libtopologymodule_la_CXXFLAGS= @AM_CXXFLAGS@

AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
	  -I$(top_srcdir)/librandom\
	  -I$(top_srcdir)/sli\
	  -I$(top_srcdir)/nestkernel\
	  @INCLTDL@ \
	  @GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

install-data-hook:
	mkdir -p $(DESTDIR)@PKGDOCDIR@/topology
	rm -rf $(DESTDIR)@PKGDOCDIR@/topology/testsuite
	cp -r @PKGSRCDIR@/$(subdir)/testsuite $(DESTDIR)@PKGDOCDIR@/topology

clean-local:
	-rm -rf $(abs_builddir)/build
