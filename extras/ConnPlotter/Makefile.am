# Automake file for ConnPlotter

EXTRA_DIST= setup.py \
	doc examples \
	colormaps.py \
	ConnPlotter.py \
	__init__.py \
	MANIFEST \
	tcd_nest.py

if HAVE_PYTHON

install-exec-hook:
	cd $(srcdir); $(PYTHON) setup.py build --build-base=$(abs_builddir)/build install --prefix=$(DESTDIR)$(prefix) --install-lib=$(DESTDIR)$(pythondir) --install-scripts=$(DESTDIR)$(bindir) --install-data=$(DESTDIR)$(pkgdatadir)

examplesdir=@PKGDOCDIR@/examples/ConnPlotter

examples_DATA=\
	examples/connplotter_tutorial.py

tutorialdir=@PKGDOCDIR@/ConnPlotter

tutorial_DATA=\
	doc/connplotter_tutorial.pdf

endif

clean-local:
	-rm -rf $(abs_builddir)/build
