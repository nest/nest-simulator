# Automake file for ConnPlotter

defs= @SLI_THREAD_DEFS@ $defs
MAKEFLAGS= @MAKE_FLAGS@

EXTRA_DIST= setup.py \
	doc examples \
	colormaps.py \
	ConnPlotter.py \
	__init__.py \
	LICENSE.ConnPlotter \
	MANIFEST \
	tcd_nest.py

# 'all' does sometimes miss changes, therefore we 'clean' first
all:	clean
	cd @PKGSRCDIR@/$(subdir); @PYTHON_EXEC@ setup.py build

install-exec-hook:
	cd @PKGSRCDIR@/$(subdir); @PYTHON_EXEC@ setup.py install --prefix=$(DESTDIR)@PYNEST_PREFIX@
	if test `echo @PYTHON_VERSION@ | cut -d. -f2` -ge 6; then \
	  mkdir -p $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	  cd $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	     ln -f -s ../site-packages/ConnPlotter; \
	  cd $(DESTDIR)@PYNEST_PREFIX@/lib/python@PYTHON_VERSION@/dist-packages; \
	     ln -f -s ../site-packages/ConnPlotter*.egg-info; \
	fi

install-data-hook: install-exec
	@INSTALL_PROGRAM@ -m 644 @PKGSRCDIR@/$(subdir)/LICENSE.ConnPlotter $(DESTDIR)@PKGDOCDIR@/
	@INSTALL_PROGRAM@ -m 755 -d $(DESTDIR)@PKGDOCDIR@/ConnPlotter
	@INSTALL_PROGRAM@ -m 644 @PKGSRCDIR@/$(subdir)/doc/connplotter_tutorial.pdf $(DESTDIR)@PKGDOCDIR@/ConnPlotter

clean:
	rm -rf build

datadir=@PKGDOCDIR@/examples/ConnPlotter

data_DATA=\
	examples/connplotter_tutorial.py
