
## Define doc targets as .PHONY so they are run
## if requested; HEP

## Documentation is created in the source directory.
## Both fulldoc and doc write to doxygen/html.
## fulldoc is painfully slow.

.PHONY: doc fulldoc

doc:
if DOXYGEN_AVAILABLE

	$(eval RCSINFO:=`@PKGSRCDIR@/extras/create_rcsinfo.sh @PKGSRCDIR@ /tmp`) \
	  $(SED) -i "/^PROJECT_NUMBER/ s/=.*/= @SLI_VERSION@, $(RCSINFO)/" @PKGBUILDDIR@/doc/normaldoc.conf
	$(DOXYGEN) @PKGBUILDDIR@/doc/normaldoc.conf

endif

fulldoc:
if DOXYGEN_AVAILABLE
if DOT_AVAILABLE

	$(eval RCSINFO:=`@PKGSRCDIR@/extras/create_rcsinfo.sh @PKGSRCDIR@ /tmp`) \
	  $(SED) -i "/^PROJECT_NUMBER/ s/=.*/= @SLI_VERSION@, $(RCSINFO)/" @PKGBUILDDIR@/doc/fulldoc.conf
	$(DOXYGEN) @PKGBUILDDIR@/doc/fulldoc.conf

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
