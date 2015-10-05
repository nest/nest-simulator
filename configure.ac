AC_PREREQ(2.63)
AC_INIT([nest], [git], [nest_user@nest-initiative.org])

# Prevent automatic rebuilding of Makefile.ins and configure if their
# content or timestamp changes. Not doing this lead to failing builds
# from tarballs on platforms with older versions of autotools. See
# stackoverflow.com/questions/18769770 for the problem. Rebuilding can
# still be enabled by configuring with --enable-maintainer-mode.
AM_MAINTAINER_MODE([disable])

# These variables are exported to libnestutil/config.h
# The patchlevel is updated automatically by the script
# releasetools/buildnest
SLI_MAJOR=2
SLI_MINOR=8
SLI_PATCHLEVEL=0-git

SLI_PRGNAME="nest-$SLI_MAJOR.$SLI_MINOR.$SLI_PATCHLEVEL"
SLI_VERSION=$SLI_MAJOR.$SLI_MINOR.$SLI_PATCHLEVEL

# Exporting source and build directories to SLI requires full path names.
# Thus we have to expand. Here, we are in top build dir, since source dir
# must exist, we can just move there and call pwd
if test "x$srcdir" = x ; then
  PKGSRCDIR=`pwd`
else
  PKGSRCDIR=`cd $srcdir && pwd`
fi
PKGBUILDDIR=`pwd`

# If the user did not supply --prefix, we fail loudly
if test $prefix = NONE; then
  echo "Please supply a valid installation prefix, for example ./configure --prefix=\$HOME/opt/nest !"
  exit 1
fi

# If this is not called, install-sh will be put into .. by bootstrap.sh
# moritz, 06-26-06
AC_CONFIG_AUX_DIR(.)

# We might consider switching to tar-pax (2001) in the future, for now
# tar-ustar (1988) is probably good enough. tar-v7 is not an option, since
# we already hit the 99 char filename limit.
AM_INIT_AUTOMAKE([tar-ustar subdir-objects])

# Allow to build NEST using silent rules.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

# Obtain host system type; HEP 2004-12-20
AC_CANONICAL_HOST

# This is necessary to use AC_CHECK_HEADER
AC_USE_SYSTEM_EXTENSIONS

# ------------------------------------------------------------------------
# Handle options
#
# NOTE: No programs/compilations must be run in this section;
#       otherwise CFLAGS and CXXFLAGS may take on funny default
#       values.
#       HEP 2004-12-20
# ------------------------------------------------------------------------

# Use PS_ARRAYS
AC_DEFINE(PS_ARRAYS, 1, [Use PS array construction semantics])

# Debug level
# Note, $SLI_debug is used in acinclude.m4
SLI_debug="unset"
# Flags depend on platform and compiler and will be set later
SLI_debugflags=""

# Optimization level
# Note, $SLI_optimize is used in acinclude.m4
SLI_optimize="set"
# Flags depend on platform and compiler and will be set later
SLI_optimizeflags=""

# Warning level
# Note, $SLI_warning is used in acinclude.m4
SLI_warning="set"
# Flags depend on platform and compiler and will be set later
SLI_warningflags=""

# Select elementary unit of time
# example:
#    ./configure --with-tics-per-ms=1000    // 1 microsecond, default
#    ./configure --with-tics-per-ms=1e3     // the same
#    ./configure --with-tics-per-ms='std::ldexp(1.0,13)'  // 2^13 tics per ms
#
AC_ARG_WITH(tics_per_ms,[  --with-tics-per-ms=nnn	Specify elementary unit of time.],
[if test "$withval" != no; then
 AC_DEFINE(HAVE_TICS_PER_MS,1, [Tics per millisecond specified? ])
 AC_DEFINE_UNQUOTED(CONFIG_TICS_PER_MS, ${withval}, [tics per ms in simulation])
fi])


# Select resolution of simulation
# example:
#    ./configure --with-tics-per-step=100   // 100 tics per step = 0.1ms (default)
#
AC_ARG_WITH(tics_per_step,[  --with-tics-per-step=nnn	Specify resolution.],
[if test "$withval" != no; then
 AC_DEFINE(HAVE_TICS_PER_STEP,1, [Tics per step specified? ])
 AC_DEFINE_UNQUOTED(CONFIG_TICS_PER_STEP, ${withval}, [tics per step in simulation])
fi])


# Select usage of the vampir tool
#
# in order to analyze the result of a program run
# the required environment variables are:
#    PAL_ROOT        e.g. /usr/nld/vampir
#    PAL_LICENSEFILE e.g. /usr/nld/vampir/etc/license.dat
# and path to the binary vampir which needs to be called with the
# trace file .bpv as a parameter.
#
# 28.3.03 Diesmann, Morrison
#
AC_ARG_WITH(vampir,[  --with-vampir			Include support for the Vampir tool.],
[if test "$withval" != no; then
 AC_DEFINE(HAVE_VAMPIR,1, [Vampir support enabled? ])
 VAMPIR_PATH="${withval}"
 VAMPIR_LIBS="-L${VAMPIR_PATH}/lib -lVT -lpmpi"
 VAMPIR_CFLAGS="-I${VAMPIR_PATH}/include"
 else
  unset HAVE_VAMPIR
  unset VAMPIR_PATH
  unset VAMPIR_LIBS
  unset VAMPIR_CFLAGS
fi])


# memory_thisjob_darwin needs the mach/mach.h header to get
# information about memory consumption
# 05 Nov 2014 Tammo Ippen
AC_CHECK_HEADERS(mach/mach.h)


bluegene_architecture="none"
configure_bluegene="no"
AC_MSG_CHECKING(whether we are configuring for Blue Gene)
AC_ARG_ENABLE([bluegene],
	[AS_HELP_STRING([--enable-bluegene],
	[Configure for Blue Gene; the specific BG model must be given as argument (=L/P/Q).])],
	[if test "x$enableval" != "xno" ; then
	   configure_bluegene="yes"
	   enableval_uc=`echo ${enableval} | awk {'print toupper($_)'}`
	   if test "x$enableval_uc" = xL || test "x$enableval_uc" = xP || test "x$enableval_uc" = xQ; then
	     bluegene_architecture="$enableval_uc"
           else
             echo
	     AC_MSG_ERROR([Only L, P, or Q are valid arguments for --enable-bluegene.])
           fi
         fi],[])

if test "x$configure_bluegene" = xno ; then
  AC_MSG_RESULT(no)
else
  AC_MSG_RESULT([yes ($bluegene_architecture)])
fi

# register IS_BLUEGENE to be visible in Makefile.am
# required to suppress slidoc installation, which would fail due to cross-compile
AM_CONDITIONAL(IS_BLUEGENE, test "x$configure_bluegene" != xno)

# Manually activate MPI, allow to specify directory containing MPI
#
# The Scali mpic++ wrapper leads to link problems, so we cannot use it.
# Instead, we use the plain C++ compiler and look for MPI in the scali
# installation directory, by default /opt/scali.
#
# To make this easy for the user, there are two mutually exclusive options:
#   --with-mpi    for non-Scali MPI
#   --with-scali  for Scali MPI
#
# H. E. Plesser, 2007-01-05
#
SLI_distributed="unset"
SLI_mpi_prefix="unset"
SLI_mpi_option="no"
SLI_scali_option="no"

if test "x$configure_bluegene" = xyes ; then
  SLI_distributed=set
  SLI_mpi_option=yes
  AC_DEFINE(IS_BLUEGENE, 1, [Configuring for Blue Gene]) 
  if test "x$bluegene_architecture" = xL; then
    AC_DEFINE(IS_BLUEGENE_L, 1, [Configuring for Blue Gene/L])
    bluegene_dynamic_libs=no # no dynamic libs on BG/L
  elif test "x$bluegene_architecture" = xP; then
    AC_DEFINE(IS_BLUEGENE_P, 1, [Configuring for Blue Gene/P])
    bluegene_dynamic_libs=yes
  elif test "x$bluegene_architecture" = xQ ; then
    AC_DEFINE(IS_BLUEGENE_Q, 1, [Configuring for Blue Gene/Q])
    bluegene_dynamic_libs=yes
  fi
else
  AC_ARG_WITH(mpi,[  --with-mpi[[=directory]]	Request compilation with MPI; optionally give directory with MPI installation.],
  [
    if test "$SLI_scali_option" = "yes" ; then
      AC_MSG_ERROR([The --with-mpi and --with-scali options are mutually exclusive.])
    fi

    if test "$withval" = "yes" ; then
      SLI_distributed="set"
      SLI_mpi_option="yes"
    elif test "$withval" != "no" ; then
      SLI_distributed="set"
      SLI_mpi_prefix=`echo ${withval} | sed 's/\/*$//'` # remove trailing slashes
      SLI_mpi_option="yes"
    fi
  ])

  AC_ARG_WITH(scali, [  --with-scali[[=directory]]	Request compilation with Scali MPI; optionally give directory with Scali [[/opt/scali]]],
  [
    if test "$SLI_mpi_option" = "yes" ; then
      AC_MSG_ERROR([The --with-mpi and --with-scali options are mutually exclusive.])
    fi

    if test "$withval" = "yes" ; then
      SLI_distributed="set"
      SLI_mpi_prefix="/opt/scali"
      SLI_scali_option="yes"
    elif test "$withval" != "no" ; then
      SLI_distributed="set"
      SLI_mpi_prefix=`echo ${withval} | sed 's#\(.*\)/$#\1# ; q'`
      SLI_scali_option="yes"
    fi
  ])
fi

if test "x$configure_k" = xyes ; then
  AC_DEFINE(IS_K, 1, [Configuring for K])
fi

# Use the MUSIC library to allow for simulator interoperation
#
AC_ARG_WITH(music,[  --with-music[[=directory]]       Request the use of MUSIC. Optionally give the directory, where MUSIC is installed],
  [
    if test "$withval" != "no"; then
      if test "$SLI_distributed" != "set"; then
         AC_MSG_ERROR([MUSIC requires --with-mpi.])
      else
        AC_DEFINE(HAVE_MUSIC,1, [MUSIC support enabled?])
        if test "$withval" = "yes"; then
      	  MUSIC_LIBS="-lmusic"
	  MUSIC_INCLUDE=""
        else
          MUSIC_PATH="${withval}"
      	  MUSIC_LIBS="-L${MUSIC_PATH}/lib -lmusic"
	  MUSIC_INCLUDE="-I${MUSIC_PATH}/include"
        fi
      fi
    else
      unset HAVE_MUSIC
    fi
  ])

#
# Use libneurosim?
#
AC_CHECK_LIB(neurosim, libneurosim_version, HAVE_LIBNEUROSIM="auto", HAVE_LIBNEUROSIM="no")
if test "x$HAVE_LIBNEUROSIM" = xauto; then
   LIBNEUROSIM_LIBS="-lneurosim"
   LIBNEUROSIM_PY_LIBS="-lpyneurosim"
   LIBNEUROSIM_INCLUDE=""
fi
AC_ARG_WITH(libneurosim, [  --with-libneurosim[[=directory]]       Request the use of libneurosim. Optionally give the directory, where libneurosim is installed],
  [
    if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
        if test "x$configure_bluegene" = xyes; then
          LIBNEUROSIM_LIBS="${withval}/lib/libneurosim.a"
          LIBNEUROSIM_PY_LIBS="${withval}/lib/libpyneurosim.a"
          LIBNEUROSIM_INCLUDE="-I${withval}/include"
        else
          LIBNEUROSIM_LIBS="-L${withval}/lib -lneurosim"
          LIBNEUROSIM_PY_LIBS="-lpyneurosim"
          LIBNEUROSIM_INCLUDE="-I${withval}/include"
        fi
      fi
      HAVE_LIBNEUROSIM="yes"
    else
      HAVE_LIBNEUROSIM="no"
    fi
  ])

if test "x$HAVE_LIBNEUROSIM" != xno; then
  AC_DEFINE(HAVE_LIBNEUROSIM, 1, [libneurosim support enabled?])
fi

# Select optimization mode
# note: see debugging mode selection
#
AC_ARG_WITH(optimize,[  --with-optimize[[=flags]]	Select optimization level.],
[ SLI_debug="unset"
  SLI_optimize="unset"
  if test "$withval" != no; then
    SLI_optimize="set"
   if test "$withval" != "yes"; then
    SLI_optimizeflags="$withval"
   fi
fi])

# select debugging mode
# note: --with-debug     -> withval = yes
#       --without-debug  -> withval = no
#       --with-debug=foo -> withval = foo
#
AC_ARG_WITH(debug,[  --with-debug[[=flags]]		Enable debugging information.],
[SLI_debug="unset"
 if test "$withval" != "no"; then
   SLI_debug="set"
   ## Debugging should be done without optimizations
   SLI_optimize="unset"
   if test "$withval" != "yes"; then
     SLI_debugflags="$withval"
   fi
fi])

# select warning level
#
AC_ARG_WITH(warnings,[  --with-warnings[[=flags]]	Enable warnings.],
[SLI_warning="unset"
 if test "$withval" != "no"; then
   SLI_warning="set"
   SLI_warningflags="$withval"
fi])

# Set the platform-dependent compiler flags based on the canonical
# host string.  These flags are placed in AM_{C,CXX}FLAGS.  If
# {C,CXX}FLAGS are given as environment variables, then they are
# appended to the set of automatically chosen flags.  After
# {C,CXX}FLAGS have been read out, they must be cleared, since
# system-dependent defaults will otherwise be placed into the
# Makefiles.  HEP 2004-12-20.

# Before we can determine the proper compiler flags, we must know
# which compiler we are using.  Since the pertaining AC macros run the
# compiler and set CFLAGS, CXXFLAGS to system-dependent values, we
# need to save command line/enviroment settings of these variables
# first. AC_AIX must run before the compiler is run, so we must run it
# here.
# HEP 2004-12-21

# Compiler selection:
# - C compiler is chosen using AC_PROG_CC. C code occurs only in a
#   few files in librandom. None of that code is MPI related.
#
# - C++ compiler is chosen as follows if distributed simulation is
#   chosen:
#
#   1. If simulation is not distributed, use AC_PROC_CXX.
#   2. Otherwise, if no prefix is given, search for mpiCC or equivalent
#      using AC_PROG_CXX(mpiCC).
#      Search order can be influenced by setting PATH before calling
#      configure.
#   3. If distributed simulation is requested and a prefix given, use
#      standard compiler from AC_PROG_CXX and check for libraries in
#      prefix directory.
#   4. Setting CXX overrides compiler selection brute force. AC_PROG_CXX
#      handles this.
#   5. The final configuration is tested for if it works.
#
# HEP 2007-01-03

SLI_SAVE_CFLAGS=$CFLAGS
SLI_SAVE_CXXFLAGS=$CXXFLAGS

AC_AIX

AC_PROG_CC

if test "$SLI_distributed" = unset ; then

  # no-distributed simulation
  AC_PROG_CXX

else

  if test "$SLI_scali_option" = "yes" ; then

    # We cannot use the mpic++ wrapper for Scali, since it
    # leads to a linker error due to "hidden symbol __dso_handle"
    # on several systems. So we just choose the default C++ compiler
    # and handle MPI via compiler flags later.
    AC_PROG_CXX

  else

    # Non-Scali MPI. Here we assume that the wrappers work. If an
    # explicit mpi-path is given, we add its bin to PATH first, then
    # test, and then redefine the compiler variables to full path
    # names. This is necessary since AC_PROG_CXX internally searches
    # through the entire PATH; since AC_PROG_CXX contains many checks
    # in addition to the search, we cannot simply write our own
    # absolute-path version.
    #
    # Search for MPI C++ compiler wrapper:
    # 1. Search for mpicxx, since mpicc and mpiCC are the same file
    #    on non-case-sensitive file systems (most OSX filesystems).
    # 2. Search for mpiCC, which is MPICH standard
    # 3. Do not look for Scali's mpic++, since the Scali compiler wrapper
    #    does not work.

    if test "$SLI_mpi_prefix" = unset ; then
      AC_PROG_CXX(mpicxx mpiCC)
    else
      SLI_SAVE_PATH=$PATH
      # add /bin, avoiding duplicate //
      mpi_bin=${SLI_mpi_prefix}/bin

      PATH=${mpi_bin}:$PATH
      AC_PROG_CXX(mpicxx mpiCC)

      # If mpicxx or mpiCC are chosen, we check if they exists in the prefix
      # path. If so, we add the prefix path. Otherwise, we leave CXX untouched.
      if test $CXX = mpicxx -o $CXX = mpiCC ; then
        if test -x ${mpi_bin}/$CXX ; then
          CXX=$mpi_bin/$CXX
          ac_ct_CXX=$CXX
          ac_cv_prog_ac_ct_CXX=$CXX
        fi
      fi
      PATH=$SLI_SAVE_PATH
    fi

  fi

fi
# further processing of distributed case below (see SLI_NEW_PATH_MPI)

##-- use ltdl library for dynamic loading of modules
##   if not Cygwin or blue gene
AC_MSG_CHECKING([whether to build shared libraries])
if test "$host_os" != "cygwin"; then
  if test "x$configure_bluegene" = xno ; then
    BUILD_SHARED=yes
  else
    BUILD_SHARED=$bluegene_dynamic_libs
  fi
else
  BUILD_SHARED=no
  SLI_debug="unset"
  SLI_debugflags="-g"
  SLI_optimize="set"
  SLI_optimizeflags="-O"
fi
if test "x$BUILD_SHARED" != xno ; then
   AC_ENABLE_SHARED    ## do not build static libraries by default, use
   AC_DISABLE_STATIC   ## --enable-static to build them too
else
   AC_DISABLE_SHARED
   AC_ENABLE_STATIC
fi

AC_MSG_RESULT([$BUILD_SHARED])

SLI_SET_CFLAGS
CFLAGS=
SLI_SET_CXXFLAGS
CXXFLAGS=

# -------------------------------------------
# END Handle options
# -------------------------------------------

## search for the Doxygen executable
AC_PATH_PROG([DOXYGEN], [doxygen])
AM_CONDITIONAL(DOXYGEN_AVAILABLE, test x$DOXYGEN != x)

## see if dot from graphviz is available
## this is only needed to choose between
## Doxygen files, see doc/Makefile.am; HEP
AC_CHECK_PROG(HAVE_DOT, dot, "yes")
AM_CONDITIONAL(DOT_AVAILABLE, test x$HAVE_DOT = xyes)

## Configure C environment

AC_PROG_INSTALL

LT_PATH_LD
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen])                 ## use libtool
LTDL_INIT([convenience subproject])

if test "x$BUILD_SHARED" != xno ; then
  if test "x$LIBLTDL" != x ; then
    AC_DEFINE(HAVE_LIBLTDL, 1, [Havel libltdl, can load dynamic modules])
  fi
fi

#-- Stuff for GSL -----------------------------------------
# NOTE: MUST come BEFORE switching to AC_LANG_CPLUSPLUS !!!
# HEP 2001-08-07
# NOTE: Adding @GSL_CFLAGS@ to the AM_CPPFLAGS variable
#       in Makefile.am will result in warnings about re-ordering
#       the search path.
# HEP 2002-07-10
# GSL Related HAVE_* variables:
#
#   HAVE_GSL     indicates that some GSL release version is available
#                (i.e., version 1.0 or higher)
#   HAVE_GSL_X_Y indicates that the GSL is version >= X.Y
#
# Currently implemented:
#   HAVE_GSL
#   HAVE_GSL_1_2   # minimum requirement for GSL Random Generators
#   HAVE_GSL_1_11  # fixes bug in ODE solvers, see #313 and #88
#
# Add further HAVE_GSL_MAJOR_MINOR if necessary according to scheme for
# HAVE_GSL_1_2 below.
#
# For each HAVE_GSL_X_Y, define also an automake conditional
# GSL_1_2_AVAILABLE to allow for conditional compilation.  See example
# below, and librandom for an example of how to use it in Makefile.am.
#
# HEP 2002-08-21
AM_PATH_GSL(1.0, ac_have_gsl="yes", ac_have_gsl="no")

if test "$ac_have_gsl" != no ; then
  AC_DEFINE(HAVE_GSL,1, [Is the GNU Science Library available (ver. >= 1.0)?])
  AC_SUBST(HAVE_GSL)
  AC_SUBST(GSL_LIBS)
  AC_SUBST(GSL_CFLAGS)
  AM_PATH_GSL(1.2, ac_have_gsl_1_2="yes", ac_have_gsl_1_2="no")
  if test "x$ac_have_gsl_1_2" = xyes ; then
    AC_DEFINE(HAVE_GSL_1_2, 1, [Is GSL Version >= 1.2?])

    AM_PATH_GSL(1.11, ac_have_gsl_1_11="yes", ac_have_gsl_1_11="no")
    if test "x$ac_have_gsl_1_11" = xyes ; then
      AC_DEFINE(HAVE_GSL_1_11, 1, [Is GSL Version >= 1.11?])
    else
      ac_have_gsl_1_11="no"
    fi
  fi
else
  unset GSL_LIBS
  unset GSL_CFLAGS
  unset GSL_VERSION
  ac_have_gsl_1_2="no"
fi
# AM_CONDITIONAL must not be invoked conditionally
AM_CONDITIONAL(GSL_1_2_AVAILABLE, test x$ac_have_gsl_1_2 = xyes)
AM_CONDITIONAL(GSL_1_11_AVAILABLE, test x$ac_have_gsl_1_11 = xyes)

# C Inline stuff revised, HEP 2003-03-11
SLI_C_INLINE

if test "$ac_cv_c_inline" != no ; then
  AC_DEFINE(HAVE_INLINE,1, [Do we have the inline macro in C?])
  AC_SUBST(HAVE_INLINE)
fi

## --- end GSL STUFF -----------------------------------

# fails to find available functions if run with language
# set to C++; HEP 2003-03-13
AC_CHECK_FUNCS(popen pclose execv fork setenv sigaction)

#-- Set the language to C++
AC_LANG_CPLUSPLUS

# check if expm1 is available after we have switched to C++
AC_MSG_CHECKING([for expm1 ])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <math.h>]],
[[double x = ::expm1(0.1);]])],
ac_have_expm1="yes", ac_have_expm1="no")
if test "$ac_have_expm1" = yes ; then
  AC_DEFINE(HAVE_EXPM1,1, ["Define if expm1() is available"])
fi
AC_MSG_RESULT($ac_have_expm1)

#-- Look for programs needed in the Makefile
AC_PROG_CXXCPP

AC_PATH_PROGS([MAKE],[gmake make],[make])

SLI_LIBS="$SLI_LIBS $LDFLAGS"

# Check for readline if not configuring for Blue Gene
if test "x$configure_bluegene" = "xno" ; then
  AC_ARG_WITH([readline], [AS_HELP_STRING([--without-readline], [disable support for GNU readline])],
  [],
  [with_readline=yes])

  if test "x$with_readline" != xno ; then
    AC_CHECK_LIB(curses,tgetstr, SLI_LIBS="${SLI_LIBS} -lcurses",\
   [AC_CHECK_LIB(termcap,tgetstr, SLI_LIBS="${SLI_LIBS} -ltermcap",\
   [AC_CHECK_LIB(ncursesw,tgetstr,  SLI_LIBS="${SLI_LIBS} -lncursesw",\
   [AC_CHECK_LIB(ncurses,tgetstr,  SLI_LIBS="${SLI_LIBS} -lncurses",\
   [AC_MSG_ERROR([You need either libncurses, libcurses, or libtermcap to compile NEST.
   ])])])])])
   AC_CHECK_LIB(readline, readline,\
   [SLI_LIBS="${SLI_LIBS} -lreadline"\
   AC_DEFINE(HAVE_READLINE, 1,[Use GNU libreadline])],\
   [AC_MSG_ERROR([No readline found, --without-readline to disable])],\
   [${SLI_LIBS}])
  fi
else
  AC_MSG_WARN("Skipping configuration of libreadline")
fi

# Test if to use multithreading with either OpenMP or POSIX threads
# JME 2011/09/29
AC_MSG_CHECKING(which threading library to use)

# Testing for OpenMP
AC_ARG_WITH([openmp], [AS_HELP_STRING([--without-openmp], [disable OpenMP multithreading])],
            [with_openmp=$withval],     # withval=yes, if configured with --with-openmp, no with --without-openmp
            [with_openmp=default])      # withval would be yes, if configured without args

# Check if the OpenMP setup is usable
if test "$with_openmp" = yes || test "$with_openmp" = default; then
  AC_MSG_RESULT(openmp)
  if test "x$configure_bluegene" = xyes; then
    # On Blue Gene, the macro AC_OPENMP is not defined, so we set the flags manually
    OPENMP_CXXFLAGS="-qsmp=omp"
  elif test "x$configure_k" = xyes; then
    # On K, the macro AC_OPENMP is not defined, so we set the flags manually
    OPENMP_CXXFLAGS="-Kopenmp"
  else
    m4_ifdef([AC_OPENMP], [AC_OPENMP])
    if test "$OPENMP_CXXFLAGS" = ""; then
      AC_MSG_ERROR([OpenMP not found, please use --without-openmp to disable.])
    fi
  fi
  AM_CFLAGS="$AM_CFLAGS $OPENMP_CXXFLAGS"
  AM_CXXFLAGS="$AM_CXXFLAGS $OPENMP_CXXFLAGS"
  with_openmp=yes
else
  AC_MSG_RESULT(none)
fi

AM_CONDITIONAL([LINK_OPENMP],  test "x$with_openmp"  = xyes)

# End of thread library processing

AC_ARG_WITH([python],
    [AS_HELP_STRING([--without-python], [ignore the presence of Python and disable PyNEST])])

AS_IF([test "x$with_python" != "xno"],
      [AM_PATH_PYTHON([2.6], [have_python=yes], [have_python=no])],
      [have_python=no])

AS_IF([test "x$have_python" = "xyes"], [
    PYTHON_INC=`$PYTHON -c 'import sys; from distutils import sysconfig; sys.stdout.write(sysconfig.get_python_inc())'`
    AC_CHECK_FILE(["${PYTHON_INC}/Python.h"], [PYNEST_CPPFLAGS="-I${PYTHON_INC}"], [have_python=no])
])

AC_MSG_CHECKING([whether to build PyNEST])
AS_IF([test "x$have_python" = "xyes"], [],
      [AS_IF([test "x$with_python" = "xyes"],
             [AC_MSG_ERROR([PyNEST requested, but Python not found])])])
AC_MSG_RESULT([$have_python])

AX_CHECK_COMPILE_FLAG([-fno-strict-aliasing],
                      [PYNEST_CXXFLAGS="-fno-strict-aliasing"], [])

# FIXME: this means that --without-python make will not recurse into pynest subdir
if test "x$have_python" = "xyes"; then
    PYNEST_SUBDIR="pynest"
else
    PYNEST_SUBDIR=""
fi

AM_CONDITIONAL([HAVE_PYTHON], [test "x$have_python" = "xyes"])

AC_SUBST([HAVE_PYTHON])
AC_SUBST([PYNEST_SUBDIR])
AC_SUBST([PYEXECDIR], ${pyexecdir})

AC_SUBST([PYTHON])
AC_SUBST([PYTHON_VERSION])

AC_SUBST([PYNEST_CPPFLAGS])
AC_SUBST([PYNEST_CXXFLAGS])

# is long long available for type representation and fully usable?
SLI_HAVE_LONG_LONG

# Check whether Alpha cxx includes *.h standard headers
# and C-style macros correctly
SLI_ALPHA_CXX_STD_BUG

# For a description of MPI detection, see comment on compiler selection.
if test "$SLI_distributed" = "set"; then
  if test "x$configure_bluegene" = xyes ; then
    BLUEGENE_MPI
  else
    SLI_NEW_PATH_MPI
  fi
fi

# Makros M_E and M_PI
#
# !HAVE_M_E && HAVE_CMATH_MAKROS_IGNORED
#    --> cmath needs to be included with #undef __PURE_CNAME
#
# HAVE_M_E  || HAVE_CMATH_MAKROS_IGNORED  --> M_E  is present
# HAVE_M_PI || HAVE_CMATH_MAKROS_IGNORED  --> M_PI is present
#
# Diesmann, 28.8.02
#
SLI_CXX_HAVE_M_E
SLI_CXX_HAVE_M_PI

SLI_CHECK_STATIC_TEMPLATE_DECLARATION

SLI_ALPHA_CMATH_MAKROS_IGNORED
SLI_ALPHA_CXX_SIGUSR_IGNORED

SLI_CHECK_ISTREAM_HEADER
SLI_CHECK_OSTREAM_HEADER
SLI_CHECK_SSTREAM_HEADER

SLI_CHECK_STL_VECTOR_CAPACITY_BASE_UNITY
SLI_CHECK_STL_VECTOR_CAPACITY_DOUBLING

SLI_CHECK_XLC_ICE_ON_USING

SLI_CHECK_EXITCODES

# ----------------------------------------------------
# Configure external modules to be linked into nest
# ----------------------------------------------------
# SLI_USER_MODULES collects module names for informative messages
SLI_USER_MODULES=""
# SLI_USER_MODULES_LDADD collects what needs to be added to LDADD
SLI_USER_MODULES_LDADD=""

AC_ARG_WITH(modules,[  --with-modules=MODULES external modules to be linked in],
[
if test "$withval" != no; then
  for mod in $withval; do
    SLI_USER_MODULES_LDADD="$SLI_USER_MODULES_LDADD $prefix/lib/nest/lib$mod.la"
    SLI_USER_MODULES="$SLI_USER_MODULES $mod"
  done
fi
])


# ---------------------------------------------------------------
# Configure directories to be built
# ---------------------------------------------------------------

# these are modules from the NEST code base, ie, in the source directory
# and dependency tracking shall happen for these modules
SLI_MODULES="models precise topology"

if test "x$HAVE_LIBNEUROSIM" != xno; then
  SLI_MODULES="$SLI_MODULES conngen"
fi

SLI_MODULES_LDADD=""
MODULE_INCLUDES=""
ADD_STATIC_MODULES=''

# collect directories for which AC_CONFIG_FILES needs to be run
MODULE_MAKEFILE_DIRS=""


# check all modules: must contain Makefile.am
for mod in $SLI_MODULES; do
  MODULE_MAKEFILE_DIRS="$MODULE_MAKEFILE_DIRS $mod"
  SLI_MODULES_LDADD="$SLI_MODULES_LDADD ${PKGBUILDDIR}/${mod}/lib${mod}module.la"

  # check if we need to create Makefiles in doc, examples, tests, pynest
  for sdir in doc examples tests pynest; do
    SLI_CHECK_SOURCE_FILE([$srcdir/$mod/$sdir/Makefile.am],
                          [MODULE_MAKEFILE_DIRS="$MODULE_MAKEFILE_DIRS $mod/$sdir"], [printf ""])
  done

  # build include and addmodule lines for use in (py)neststartup
  modheader="$mod/${mod}module.h"
  # get module name from the module's main header
  Mod=`grep SLIModule ${srcdir}/${modheader} | sed -n 's/class *\(.*\) *:.*/\1/p' | tr -d ' '`
  SLI_CHECK_SOURCE_FILE([$srcdir/$mod/Makefile.am],
    [MODULE_INCLUDES="${MODULE_INCLUDES}#include \"../${modheader}\"\n"
     ADD_STATIC_MODULES="${ADD_STATIC_MODULES}  engine.addmodule(new nest::${Mod}(net));\n"],
    [printf ""])

done

# On Bluegene, we need to link user modules statically and add them to startup
# We require that the module is installed and has its header file in $prefix/include
if test x$configure_bluegene == xyes ; then
  for umod in $SLI_USER_MODULES ; do
    modheader="${umod}.h"
    fullmodheader="${prefix}/include/${modheader}"
    # get module namespace and class from the module's main header
    # the following chain of commands (feel free to beautify!)
    # 1. removes all block comments
    # 2. removes all // comments
    # 3. removes all "namespace nest" lines
    # 4. selects all "namespace" lines (should only be one!!!)
    # 5. extracts the name of the module namespace 
    # 6. appends :: if modnamespace is not empty
    modnamespace=`sed "/^ *\/\*/,/*\/ *$/d" ${fullmodheader} | sed "/\/\/.*$/d" | grep -v "namespace *nest" | grep namespace | sed -n 's/namespace *\(.*\)/\1/p' | tr -d ' {'`
    if test x$modnamespace != x ; then
      modnamespace="${modnamespace}::"
    fi
    modclass=`grep SLIModule ${fullmodheader} | sed -n 's/class *\(.*\) *:.*/\1/p' | tr -d ' '`
    # Do NOT pass network to user modules, it is outdated
    SLI_CHECK_SOURCE_FILE([$prefix/include/$modheader],
      [MODULE_INCLUDES="${MODULE_INCLUDES}#include \"${fullmodheader}\"\n"
       ADD_STATIC_MODULES="${ADD_STATIC_MODULES}  engine.addmodule(new ${modnamespace}${modclass}());\n"],
      [printf ""])
  done
fi


# Now generate $PKGBUILDDIR/nest/static_modules.h, including the module headers
# and providing the add_static_modules() function.
# This way of doing it is quite a hack. However, AC_CONFIG_FILES() cannot substitute
# the multiline variables we want to create here.
AC_CONFIG_COMMANDS([nest/static_modules.h],
	           [printf "$guard$module_includes$standard_includes$function_signature$module_loaders$end" > nest/static_modules.h],
		   [guard="#ifndef STATIC_MODULES_H\n#define STATIC_MODULES_H\n\n"
		    module_includes='$MODULE_INCLUDES\n'
		    standard_includes="#include \"interpret.h\"\n#include \"network.h\"\n\n"
		    function_signature="void add_static_modules(SLIInterpreter& engine, nest::Network& net)\n{\n"
		    module_loaders='$ADD_STATIC_MODULES'
		    end="}\n\n#endif\n"])

# directories to include in distribution tarballs
SLI_SUBDIRS="libnestutil sli librandom lib nestkernel $SLI_MODULES nest"
SLI_DIST_SUBDIRS="$SLI_SUBDIRS conngen pynest"

# generate a list of absolute paths where to search for help files
for dir in $SLI_SUBDIRS; do
  HELPDIRS="$HELPDIRS $PKGSRCDIR/$dir"
  # Automatically include sli directory if it exists
  if test -d "$PKGSRCDIR/$dir/sli" ; then
    HELPDIRS="$HELPDIRS $PKGSRCDIR/$dir/sli"
  fi
done

HELPDIRS=`echo $HELPDIRS | tr " " ":"`
INSTALL_HELP_LOG="$PKGBUILDDIR/install-help.log"

PKGDATADIR=$datadir/$PACKAGE
PKGDOCDIR=$datadir/doc/$PACKAGE
TESTSUITE_BASEDIR=`eval eval echo $PKGDOCDIR`

SLI_PREFIX=$prefix

# This hack resolves '${prefix}' in the variables
eval eval eval PKGDOCDIR_AS_CONFIGURED=$PKGDOCDIR
eval eval eval PKGDATADIR_AS_CONFIGURED=$PKGDATADIR

#-- Replace these variables in *.in
AC_SUBST(HAVE_ISTREAM)
AC_SUBST(HAVE_OSTREAM)
AC_SUBST(HAVE_SSTREAM)
AC_SUBST(HAVE_STL_VECTOR_CAPACITY_BASE_UNITY)
AC_SUBST(HAVE_STL_VECTOR_CAPACITY_DOUBLING)
AC_SUBST(HAVE_ERRNO_IGNORED)
AC_SUBST(HAVE_M_E)
AC_SUBST(HAVE_M_PI)
AC_SUBST(HAVE_CMATH_MAKROS_IGNORED)
AC_SUBST(HAVE_SIGUSR_IGNORED)
AC_SUBST(HAVE_READLINE)
AC_SUBST(HAVE_VAMPIR)
AC_SUBST(VAMPIR_LIBS)
AC_SUBST(VAMPIR_CFLAGS)
AC_SUBST(IS_BLUEGENE)
AC_SUBST(IS_BLUEGENE_L)
AC_SUBST(IS_BLUEGENE_P)
AC_SUBST(IS_BLUEGENE_Q)
AC_SUBST(IS_K)
AC_SUBST(SLI_MAJOR)
AC_SUBST(SLI_MINOR)
AC_SUBST(SLI_PATCHLEVEL)
AC_SUBST(SLI_VERSION)
AC_SUBST(SLI_PRGNAME)
AC_SUBST(SLI_PREFIX)
AC_SUBST(PKGSRCDIR)
AC_SUBST(PKGBUILDDIR)
AC_SUBST(PKGDATADIR)
AC_SUBST(PKGDATADIR_AS_CONFIGURED)
AC_SUBST(PKGDOCDIR)
AC_SUBST(PKGDOCDIR_AS_CONFIGURED)
AC_SUBST(TESTSUITE_BASEDIR)
AC_SUBST(SLI_EXITCODE_ABORT)
AC_SUBST(SLI_EXITCODE_SEGFAULT)
AC_SUBST(KERNEL)
AC_SUBST(HOST)
AC_SUBST(SED)
AC_SUBST(LD)
AC_SUBST(host_os)
AC_SUBST(host_cpu)
AC_SUBST(host_vendor)
AC_SUBST(AS)
AC_SUBST(CC)
AC_SUBST(CXX)
AC_SUBST(AR)
AC_SUBST(ARFLAGS)
AC_SUBST(CXX_AR)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(OPENMP_CXXFLAGS)
AC_SUBST(MAKE)
AC_SUBST(MAKE_FLAGS)
AC_SUBST(SLI_SUBDIRS)
AC_SUBST(HELPDIRS)
AC_SUBST(INSTALL_HELP_LOG)
AC_SUBST(SLI_DIST_SUBDIRS)
AC_SUBST(SLI_THREAD_DEFS)
AC_SUBST(SLI_LIBS)
AC_SUBST(MPI_LIBS)
AC_SUBST(MPI_INCLUDE)
AC_SUBST(HAVE_MUSIC)
AC_SUBST(MUSIC_LIBS)
AC_SUBST(MUSIC_INCLUDE)
AC_SUBST(LIBNEUROSIM_LIBS)
AC_SUBST(LIBNEUROSIM_PY_LIBS)
AC_SUBST(LIBNEUROSIM_INCLUDE)
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(HAVE_LIBLTDL)
AC_SUBST(LIBADD_DL)
AC_SUBST(SLI_MODULES_LDADD)
AC_SUBST(SLI_USER_MODULES_LDADD)
AC_SUBST(configure_bluegene)

AC_CONFIG_HEADER(libnestutil/config.h)
AC_CONFIG_FILES(libnestutil/sliconfig.h)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(doc/fulldoc.conf)
AC_CONFIG_FILES(doc/normaldoc.conf)
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(examples/Makefile)
AC_CONFIG_FILES(examples/nest/Makefile)
AC_CONFIG_FILES(examples/sli/Makefile)
AC_CONFIG_FILES(testsuite/Makefile)
AC_CONFIG_FILES(libnestutil/Makefile)
AC_CONFIG_FILES(librandom/Makefile)
AC_CONFIG_FILES(sli/Makefile)
AC_CONFIG_FILES(lib/Makefile)
AC_CONFIG_FILES(nestkernel/Makefile)
AC_CONFIG_FILES(nest/Makefile)
AC_CONFIG_FILES(extras/nest-config)
AC_CONFIG_FILES(testsuite/do_tests.sh)
AC_CONFIG_FILES(extras/nest_vars.sh)

# conngen is special in that it might be omitted from build when
# libneurosim is not present, yet it always has to be configured
# in order to be distributed with the tarballs
AC_CONFIG_FILES(conngen/Makefile)

# In the following loop, we need to have a case statement and list all
# directories with Makefile.am explicitly. Otherwise, automake does
# not generate Makefile.in.
for dir in $MODULE_MAKEFILE_DIRS; do
  case $dir in
    models)
      AC_CONFIG_FILES(models/Makefile)
      ;;
    precise)
      AC_CONFIG_FILES(precise/Makefile)
      ;;
    topology)
      AC_CONFIG_FILES(topology/Makefile)
      if test "x$have_python" = "xyes" ; then
        AC_CONFIG_FILES(topology/setup.py)
      fi
      ;;
    topology/doc)
      AC_CONFIG_FILES(topology/doc/Makefile)
      ;;
  esac
done

EXTRAS_SUBDIRS=

# if Python is found, configure and compile PyNEST
if test "x$have_python" = "xyes" ; then
 AC_CONFIG_FILES(pynest/Makefile)
 AC_CONFIG_FILES(pynest/setup.py)
 AC_CONFIG_FILES(pynest/do_tests.py)

 AC_CONFIG_FILES(extras/ConnPlotter/Makefile)
 EXTRAS_SUBDIRS="$EXTRAS_SUBDIRS ConnPlotter"
fi

AC_SUBST(EXTRAS_SUBDIRS)
AC_CONFIG_FILES(extras/Makefile)

# -------------------------------------------------------------------
# Create output
# -------------------------------------------------------------------
AC_OUTPUT


# -------------------------------------------------------------------
# Patch libtool.
# See https://bugs.launchpad.net/ubuntu/+source/kdevelop/+bug/380367
# -------------------------------------------------------------------
if grep -s -q \$echo libtool; then
  grep -s -q echo\= libtool || patch -f -p0 < $PKGSRCDIR/extras/libtool-echo-fix.patch
fi


# -------------------------------------------------------------------
# Report, after output at end of configure run Must come after
# AC_OUTPUT, so that it is displayed after libltdl has been configured
# -------------------------------------------------------------------

echo
echo "--------------------------------------------------------------------------------"
echo "NEST Configuration Summary"
echo "--------------------------------------------------------------------------------"
echo
echo "C compiler          : $CC"
echo "C compiler flags    : "`echo $AM_CFLAGS | sed 's/^[ ]*//g'`
echo "C++ compiler        : $CXX"
echo "C++ compiler flags  : "`echo $AM_CXXFLAGS | sed 's/^[ ]*//g'`
echo

if test x`echo $SLI_MODULES | tr -d " "` != x; then
  echo "Built-in modules    : "`echo $SLI_MODULES | sed 's/^[ ]*//g'`
else
  echo "Built-in modules    : None"
fi

if test x`echo $SLI_USER_MODULES | tr -d " "` != x; then
  echo "User modules        : "`echo $SLI_USER_MODULES | sed 's/^[ ]*//g'`
else
  echo "User modules        : None"
fi

echo

if test "x$have_python" = "xyes" ; then
  echo "Python bindings     : Yes (Python $PYTHON_VERSION: $PYTHON)"
else
  echo "Python bindings     : No"
fi

if test "$with_openmp" = yes; then
  echo "Use threading       : Yes (OpenMP)"
else
  echo "Use threading       : No"
fi

echo "Use GSL             :" `echo $ac_have_gsl | tr "[ny]" "[NY]"`

if test $SLI_distributed = set; then
  if test $SLI_mpi_prefix = "unset"; then
    echo "Use MPI             : Yes"
  else
    echo "Use MPI             : Yes ($SLI_mpi_prefix)"
  fi
else
  echo "Use MPI             : No"
fi

if test "x$MUSIC_LIBS" != x; then
  echo "Use MUSIC           : Yes"
  echo "  MUSIC inlude      : $MUSIC_INCLUDE"
  echo "  MUSIC libs        : $MUSIC_LIBS"
else
  echo "Use MUSIC           : No"
fi

if test "x$HAVE_LIBNEUROSIM" != xno; then
  echo "Use libneurosim     : Yes"
  if test "x$LIBNEUROSIM_INCLUDE" != x; then
    echo "  libneurosim inlude: $LIBNEUROSIM_INCLUDE"
  fi
  if test "x$LIBNEUROSIM_LIBS" != x; then
    echo "  libneurosim libs  : $LIBNEUROSIM_LIBS $LIBNEUROSIM_PY_LIBS"
  fi
else
  echo "Use libneurosim     : No"
fi


echo
echo "--------------------------------------------------------------------------------"
echo
echo "The NEST executable will be installed to:"
printf "  "; eval eval echo "$exec_prefix/bin/"
echo
echo "Documentation and examples will be installed to:"
echo  "  ${PKGDOCDIR_AS_CONFIGURED}/"
echo
if test "x$have_python" = "xyes"; then
    echo "PyNEST will be installed to:"
    printf "  "; eval eval echo "  ${pyexecdir}"
    echo
fi
echo "To set necessary environment variables, add the following line"
echo "to your ~/.bashrc :"
printf "  source "; eval eval echo "$exec_prefix/bin/nest_vars.sh"
echo
echo "--------------------------------------------------------------------------------"
echo

if test "$SLI_scali_option" = yes; then
  echo
  echo "WARNING: We advise against using the SCALI MPI Library for the time being,"
  echo "WARNING: because of instabilities observed when simulating with 28 or more"
  echo "WARNING: processes. We recommend using OpenMPI (www.open-mpi.org) instead!"
  echo
  echo "--------------------------------------------------------------------------------"
  echo
fi

if test "$ac_have_gsl" != no; then
  if test "$ac_have_gsl_1_2" != yes ; then
    echo
    echo "Your GNU Scientific Library is earlier than GSL v 1.2."
    echo "GSL Random Number Generators will not be available."
    echo "See http://www.gnu.org/software/gsl for newer versions."
    echo
    echo "--------------------------------------------------------------------------------"
    echo
  fi
  if test "$ac_have_gsl_1_11" != yes ; then
    echo
    echo "Your GNU Scientific Library is earlier than GSL v 1.11."
    echo "The aeif_cond_alpha neuron will not be available."
    echo "See http://www.gnu.org/software/gsl for newer versions."
    echo
    echo "--------------------------------------------------------------------------------"
    echo
  fi
else
  echo
  echo "ATTENTION!"
  echo "You are about to compile NEST without the GNU Scientific"
  echo "Library. This means that conductance-based neuron models"
  echo "and some random number generators will not be available."
  echo
  echo "--------------------------------------------------------------------------------"
  echo
fi

echo "You can now build and install NEST with"
echo "  make"
echo "  make install"
echo "  make installcheck"
echo

if test $SLI_distributed = set; then
  echo "You have configured NEST with support for distributed computing."
  echo "After running make install, please tell NEST how to start a"
  echo "distributed job on your system before running make installcheck."
  echo "You can do this by modifying the template for command /mpirun in"
  echo "~/.nestrc, which is created by the first start of NEST."
  echo
fi

echo "If you experience problems with the installation or the use of NEST,"
echo "please see http://www.nest-simulator.org/frequently_asked_questions"
echo "or go to http://www.nest-simulator.org/community to find out how to"
echo "join the user mailing list."
echo
