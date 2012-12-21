TESTSUBDIRS= selftests unittests regressiontests manualtests mpitests mpi_selftests mpi_selftests/pass mpi_selftests/fail

install-data-hook:
	for dir in $(TESTSUBDIRS) ; do \
          rm -rf $(DESTDIR)@PKGDOCDIR@/$$dir/*; \
	  mkdir -p $(DESTDIR)@PKGDOCDIR@/$$dir; \
	  cp @PKGSRCDIR@/$(subdir)/$$dir/* $(DESTDIR)@PKGDOCDIR@/$$dir; \
	done
	mkdir -p $(DESTDIR)@PKGDATADIR@/extras
	@INSTALL_PROGRAM@ do_tests.sh $(DESTDIR)@PKGDATADIR@/extras

install-slidoc:
	mkdir -p $(DESTDIR)@PKGDOCDIR@/help
	for dir in $(TESTSUBDIRS) ; do \
	  $(DESTDIR)$(exec_prefix)/bin/sli --userargs="@PKGSRCDIR@/$(subdir)/$$dir" $(top_srcdir)/lib/sli/install-help.sli 2>&1 >> @INSTALL_HELP_LOG@; \
	done

EXTRA_DIST= $(TESTSUBDIRS) README
