
## Define doc targets as .PHONY so they are run
## if requested; HEP

## Documentation is created in the source directory.
## Both fulldoc and doc write to doxygen/html.
## fulldoc is painfully slow.

.PHONY: doc fulldoc

doc:
if DOXYGEN_AVAILABLE
	TOPSRCDIR=$(top_srcdir) $(DOXYGEN) $(top_srcdir)/doc/normaldoc.conf
endif

fulldoc:
if DOXYGEN_AVAILABLE
if DOT_AVAILABLE

	TOPSRCDIR=$(top_srcdir) $(DOXYGEN) $(top_srcdir)/doc/fulldoc.conf

## is it possible to issue a message here if DOT is not available?

endif
endif

EXTRA_DIST= doc_header.txt

data_DATA= ../LICENSE

datadir=@PKGDOCDIR@

nobase_data_DATA=\
	conngen/conngen.txt \
	index.html\
	quickref.html\
	doc_header.txt
