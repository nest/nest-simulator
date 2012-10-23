nobase_pkgdata_DATA=\
  sli/FormattedIO.sli\
  sli/arr.sli\
  sli/debug.sli\
  sli/gdftools.sli\
  sli/helpinit.sli\
  sli/install-help.sli\
  sli/librandom.sli\
  sli/library.sli\
  sli/mathematica.sli\
  sli/misc_helpers.sli\
  sli/oosupport.sli\
  sli/processes.sli\
  sli/ps-lib.sli\
  sli/regexp.sli\
  sli/sli-init.sli\
  sli/typeinit.sli\
  sli/version.sli\
  sli/XDR.sli\
  sli/nest-init.sli\
  sli/unittest.sli\
  sli/filesystem.sli

HELPDIRS="@PKGSRCDIR@/$(subdir)/sli"

install-slidoc:
	mkdir -p $(DESTDIR)@PKGDOCDIR@/help
	$(DESTDIR)$(exec_prefix)/bin/sli --userargs=$(HELPDIRS) $(top_srcdir)/lib/sli/install-help.sli 2>&1 > @INSTALL_HELP_LOG@
