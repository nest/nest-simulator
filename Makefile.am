SUBDIRS= libltdl @SLI_SUBDIRS@ doc examples extras testsuite @PYNEST_SUBDIR@

# ensure proper distribution
# The present combination of DIST_SUBDIRS, EXTRA_DIST and
# dist-hook cleanup commands is anything but optimal.  We
# should clean this up when we create a proper install
# target.  HEP, 2004-12-02
DIST_SUBDIRS= @SLI_DIST_SUBDIRS@ libltdl
EXTRA_DIST= doc examples testsuite extras lib LICENSE bootstrap.sh libltdl install-sh configure.ac.in
ACLOCAL_AMFLAGS= -I libltdl/m4

# note: the cleaning of libltdl below is not really clean.
# One should use make distclean here, but make gets confused
# then. So we remove files by hand.
dist-hook:
	test -f $(top_srcdir)/releasetools/clear_extra_modules.awk \
	  && gawk -f  $(top_srcdir)/releasetools/clear_extra_modules.awk $(distdir)/configure.ac > $(distdir)/configure.ac.tmp \
	  && mv $(distdir)/configure.ac.tmp $(distdir)/configure.ac || true
	rm -rf `find $(distdir) -name .svn`
	rm -rf $(distdir)/doc/doxygen
	rm -f $(distdir)/doc/doxygen.log
	rm -rf $(distdir)/examples/nest/FacetsBenchmarks/debugging_versions
	rm -f $(distdir)/libltdl/{config.h,config.log,config.status,Makefile,stamp-h1}
	rm -f $(distdir)/extras/logos/*.{ai,svg}
	rm -f $(distdir)/extras/logos/nest-initiative-logo*.*


.PHONY: doc fulldoc install-slidoc-recursive

doc: 
	$(MAKE) -C doc $(AM_MAKEFLAGS) doc

fulldoc: 
	$(MAKE) -C doc $(AM_MAKEFLAGS) fulldoc

if IS_BLUEGENE

install-slidoc-recursive:
	rm -rf $(DESTDIR)@PKGDOCDIR@/help/*-
	mkdir -p $(DESTDIR)@PKGDOCDIR@/help
else

install-slidoc-recursive:
	rm -rf $(DESTDIR)@PKGDOCDIR@/help/*
	mkdir -p $(DESTDIR)@PKGDOCDIR@/help
	SLIDOCDIR=$(DESTDIR)@PKGDOCDIR@ SLIDATADIR=$(DESTDIR)@PKGDATADIR@ NESTRCFILENAME=/dev/null \
	  $(DESTDIR)$(exec_prefix)/bin/sli --userargs="@HELPDIRS@" $(top_srcdir)/lib/sli/install-help.sli 2>&1 > @INSTALL_HELP_LOG@
	$(MAKE) SLIDOCDIR=$(DESTDIR)@PKGDOCDIR@ SLIDATADIR=$(DESTDIR)@PKGDATADIR@ NESTRCFILENAME=/dev/null \
	  -k -C @PKGBUILDDIR@/testsuite install-slidoc 2> /dev/null

endif

if HAVE_PYTHON

installcheck-local:
	PATH=$(exec_prefix)/bin:$(PATH) \
	/bin/sh $(DESTDIR)@PKGDATADIR@/extras/do_tests.sh --test-pynest

else

installcheck-local:
	PATH=$(exec_prefix)/bin:$(PATH) \
	/bin/sh $(DESTDIR)@PKGDATADIR@/extras/do_tests.sh

endif

# This hook runs after make install has finished.
# We use it to replace some distribution paths in extras/emacs/sli.el.
# Note that we can't depend on configure's replacement, because we
# do not have fully expanded variables there (they still contain $prefix).
install-data-hook: install-exec install-slidoc-recursive
	sed -e "s:++PKGDATADIR++:@PKGDATADIR@:"\
	    -e "s:++PKGDOCDIR++:@PKGDOCDIR@:"\
	    -e "s:++PKGSRCDIR++:@PKGSRCDIR@:"\
	    $(DESTDIR)@PKGDATADIR@/extras/emacs/sli.el.in > $(DESTDIR)@PKGDATADIR@/extras/emacs/sli.el
	rm $(DESTDIR)@PKGDATADIR@/extras/emacs/sli.el.in
	@INSTALL_PROGRAM@ @PKGBUILDDIR@/extras/nest-config $(DESTDIR)$(exec_prefix)/bin/
	@INSTALL_PROGRAM@ @PKGSRCDIR@/extras/nest_serial $(DESTDIR)$(exec_prefix)/bin/
	@INSTALL_PROGRAM@ @PKGSRCDIR@/extras/nest_indirect $(DESTDIR)$(exec_prefix)/bin/
	@INSTALL_PROGRAM@ -m 644 @PKGSRCDIR@/README $(DESTDIR)@PKGDOCDIR@/
	@INSTALL_PROGRAM@ -m 644 @PKGSRCDIR@/NEWS $(DESTDIR)@PKGDOCDIR@/

nobase_pkgdata_DATA=\
	extras/emacs/sli.el.in \
	extras/emacs/postscript-sli.el \
	extras/emacs/psvn.el

uninstall:
	@echo "make: uninstall target is not supported, use isolated prefix or a package manager instead!"
	@exit 1
