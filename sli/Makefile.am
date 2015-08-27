bin_PROGRAMS= sli

AM_CPPFLAGS= -I$(top_srcdir)/libnestutil\
	-I$(top_srcdir)/librandom\
	-I$(top_srcdir)/nestkernel\
	@GSL_CFLAGS@\
	-DPKGDATADIR=\"@PKGDATADIR@\"\
	-DPKGDOCDIR=\"@PKGDOCDIR@\"\
	-DPKGSOURCEDIR=\"@PKGSRCDIR@\"

sli_SOURCES= puresli.cc

sli_LDADD= libsli.la \
	$(top_builddir)/libnestutil/libnestutil.la \
	@SLI_LIBS@

noinst_LTLIBRARIES= libsli.la

libsli_la_SOURCES=\
		aggregatedatum.h\
		arraydatum.cc arraydatum.h\
		booldatum.cc booldatum.h\
		callbackdatum.h\
		charcode.cc charcode.h\
		datum.cc datum.h\
		dict.cc dict.h\
		dictdatum.cc dictdatum.h\
		dictstack.cc dictstack.h\
		dictutils.cc dictutils.h\
		doubledatum.cc doubledatum.h\
		fdstream.cc fdstream.h\
		filesystem.cc filesystem.h\
		functional.h\
		functiondatum.cc functiondatum.h\
		genericdatum.h\
		get_mem.c\
		gnureadline.cc gnureadline.h\
		integerdatum.cc integerdatum.h\
		interpret.cc interpret.h\
		iostreamdatum.cc iostreamdatum.h\
		iteratordatum.h iteratordatum.cc\
		literaldatum.cc\
		lockptrdatum.h lockptrdatum_impl.h\
		name.cc name.h\
		slinames.h slinames.cpp\
		namedatum.cc namedatum.h\
		numericdatum.h numericdatum_impl.h\
		oosupport.cc oosupport.h\
		parser.cc parser.h\
		parserdatum.h\
		processes.cc processes.h\
		psignal.c psignal.h\
		scanner.cc scanner.h\
		sli_io.cc sli_io.h\
		sliactions.cc sliactions.h\
		sliarray.cc sliarray.h\
		slibuiltins.cc slibuiltins.h\
		slicontrol.cc slicontrol.h\
		slidata.cc slidata.h\
		slidict.cc slidict.h\
		sliexceptions.cc sliexceptions.h\
		slifunction.h\
		sligraphics.cc sligraphics.h\
		slimath.cc slimath.h\
		slimodule.cc slimodule.h\
		sliregexp.cc sliregexp.h\
		slistack.cc slistack.h\
		slistartup.cc slistartup.h\
		slitype.cc slitype.h\
		slitypecheck.cc slitypecheck.h\
		specialfunctionsmodule.cc specialfunctionsmodule.h\
		stringdatum.cc stringdatum.h\
		symboldatum.cc symboldatum.h\
		tarrayobj.cc tarrayobj.h\
		token.cc token.h\
		tokenarray.cc tokenarray.h\
		tokenstack.cc tokenstack.h\
		tokenutils.cc tokenutils.h\
		triedatum.cc triedatum.h\
		typearray.h\
		typechk.cc typechk.h\
		utils.cc utils.h

EXTRA_DIST= README.md
