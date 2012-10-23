# Automake file for TopologyModule
#
# Headers from this directory are not to be installed upon
# make install. They are therefore included in _SOURCES.

# BEGIN: do not change these lines ---------------------------------------
defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@
# END  : do not change these lines ---------------------------------------

SUBDIRS= doc examples

noinst_LTLIBRARIES= libtopologymodule.la

# Add one line for each h-cpp-file pair containing code for your
# module. Remember the \ at the end of each but the last line!
libtopologymodule_la_SOURCES= \
			topologymodule.cpp \
			topologymodule.h \
			topologyconnector.cpp \
			topologyconnector.h \
			connection_creator.cpp \
			connection_creator.h \
			layer.cpp \
			layer.h \
			layer_regular.cpp \
			layer_regular.h \
			position.h \
			quadtree.cpp \
			quadtree.h \
			quadrant.cpp \
			quadrant.h \
			layer_unrestricted.cpp \
			layer_unrestricted.h \
			topology_names.cpp \
			topology_names.h \
			parameters.cpp \
			parameters.h \
			region.cpp \
			region.h \
			nodewrapper.cpp \
			nodewrapper.h \
			selector.cpp \
			selector.h \
			layer_3d.cpp \
			layer_3d.h \
			octtree.cpp \
			octtree.h \
			octant.cpp \
			octant.h \
			walker.cpp \
			walker.h \
			binomial.cpp \
			binomial.h \
			layer_slice.h

# do not change anything below this line ------------------------------
# libtopologymodule_la_LDFLAGS= -module

libtopologymodule_la_CXXFLAGS= @AM_CXXFLAGS@

AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
	  -I$(top_srcdir)/librandom\
	  -I$(top_srcdir)/sli\
	  -I$(top_srcdir)/nestkernel\
	  @INCLTDL@ \
	  @GSL_CFLAGS@ @MUSIC_INCLUDE@ @MPI_INCLUDE@

nobase_pkgdata_DATA=\
  sli/topology-interface.sli

HELPDIRS="@PKGSRCDIR@/$(subdir):@PKGSRCDIR@/$(subdir)/sli:@PKGSRCDIR@/$(subdir)/testsuite/manualtests"

install-slidoc:
	mkdir -p $(DESTDIR)@PKGDOCDIR@/help
	$(DESTDIR)$(exec_prefix)/bin/sli --userargs=$(HELPDIRS) $(top_srcdir)/lib/sli/install-help.sli 2>&1 > @INSTALL_HELP_LOG@

install-data-hook:
	mkdir -p $(DESTDIR)@PKGDOCDIR@/topology
	rm -rf $(DESTDIR)@PKGDOCDIR@/topology/testsuite
	cp -r @PKGSRCDIR@/$(subdir)/testsuite $(DESTDIR)@PKGDOCDIR@/topology
	rm -rf `find $(DESTDIR)@PKGDOCDIR@/topology -name .svn`

install-pynest-component:
	if test `echo @PYTHON_VERSION@ | cut -d. -f2` -ge 6; then \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages/nest/topology; \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages/nest/topology/tests; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/hl_api.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages/nest/topology; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/__init__.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages/nest/topology; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/tests/*.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages/nest/topology/tests; \
	else \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/site-packages/nest/topology; \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/site-packages/nest/topology/tests; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/hl_api.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/site-packages/nest/topology; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/__init__.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/site-packages/nest/topology; \
	  cp @PKGSRCDIR@/$(subdir)/pynest/tests/*.py $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/site-packages/nest/topology/tests; \
	fi

EXTRA_DIST= testsuite sli doc pynest examples
