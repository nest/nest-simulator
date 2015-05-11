dnl DO NOT USE cvs/rcs keyword replacement here!!
dnl
dnl @synopsis SLI_SET_CXXFLAGS
dnl
dnl Set flags for various C++ compilers.
dnl Each compiler has a different set of options
dnl so we try to identify the compiler and set the
dnl corresponding options.
dnl Moreover, some compilers like to handle the AR stage of
dnl static library creation (SUN and SGI CC). This is also handled
dnl here.
dnl
dnl Please do not handle C flags here, use
dnl SLI_SET_CFLAGS below for this purpose.
dnl here.
dnl
dnl @author Marc-Oliver Gewaltig
dnl rewritten: 2002-11-15

AC_DEFUN([SLI_SET_CXXFLAGS],\
[
# Find which compiler C++ compiler we have to switch compiler flags
# add more conditionals for further compilers:
# However, things are slightly more complicated, since compilers
# on different systems might have the same name (e.g. CC on Irix and Solaris)
# Thus, we first check whether a GNU compiler was diagnosed. If not,
# we assume that the Proprietary compiler is presents by checking the
# OS-name
#
#
#   -ansi removed from C compilation because of problem with GSL
#   Diesmann, 23.08.02 
#

SLI_CXXBACKEND=
SLI_cxxflags=
SLI_threadflags=

# conditional default, 19.11.04 MD
#

if (! test "$CXX_AR") ; then
 CXX_AR="ar cru"
fi

##
## Section for general flags.
##
# compressed target lists ?
if test "$SLI_compression" = yes ; then
 SLI_cxxflags="-DCOMPR_CONMAT"
fi

# We first branch for the plaform and then branch on the compiler.
# Thus, it is possible to enable platform specific optimizations
# even if GCC is used.
echo "Platform: ${host}"
 case ${host} in
   *linux*)
     if test "$GXX" = "yes"; then
       compversion=`$CXX --version`
       if test "${compversion:0:4}" = "icpc"; then
         # Intel compiler pretending to be g++
	 echo "Compiler : icpc"
         if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
           SLI_debugflags="-g -inline-debug-info"
         fi
         if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
           SLI_warningflags="-w1"
         fi
         if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
           SLI_optimizeflags="-O3 -fp-model precise"
         fi
       else
         #real g++
         echo "Compiler : g++"
         if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
           SLI_debugflags="-g"
         fi
         if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
           SLI_warningflags="-W -Wall -pedantic -Wno-long-long"
         fi
         if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
           SLI_optimizeflags="-O3"
         fi
       fi
     fi
     if test "x$enable_bluegene" = xyes; then
	  echo "Compiling for the Blue Gene using compilers specified by CC and CXX environment variables"
	  if test "$SLI_debug" = "set"; then
            SLI_debugflags=" "
          fi
          if test "$SLI_warning" = "set"; then
            SLI_warningflags=" "
          fi
          if test "$SLI_optimize" = "set"; then
            SLI_optimizeflags=" "
          fi
     fi
   ;;
   sparc*-sun-solaris2.*)
     SLI_forte=`$CXX -V 2>&1 | grep Forte`
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2 -mcpu=v9"
       fi
     fi
     if test -n "$SLI_forte"; then
       echo "Compiler : $SLI_forte"
       CXX_AR="$CXX -xar -o"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="+w"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-fast"
       fi
       if test -z "$SLI_threadflags"; then 
         SLI_threadflags="-mt"
       fi
     else # Version 8 is no longer called Forte, but Sun C++
       SLI_forte=`$CXX -V 2>&1 | grep Sun`

       if test -n "$SLI_forte"; then	
         echo "Compiler : $SLI_forte"
         CXX_AR="$CXX -xar -o"
         if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
           SLI_debugflags="-g"
         fi
         if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
           SLI_warningflags="+w"
         fi
         if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
           SLI_optimizeflags="-fast"
         fi
         if test -z "$SLI_threadflags"; then 
           SLI_threadflags="-mt"
         fi
       fi
      fi
     ;;
   *-hp-hpux*)
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       SLI_cxxflags="${SLI_cxxflags} -AA"
     fi
     ;;
   mips-sgi-irix*)
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       SLI_cxxflags="${SLI_cxxflags} -LANG:std"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-64 -mips4"
       fi
     fi
   ;;
   *-dec-osf*)
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O3 -mieee"
       fi
     else
       echo "Compiler: $CXX"
       SLI_cxxflags="${SLI_cxxflags} -std strict_ansi -ieee -denorm -underflow_to_zero -nofp_reorder -pthread -lm -ptr ../cxx_r"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-fast -arch host -tune host "
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-std strict_ansi"
       fi
     fi
   ;;
   hppa1.1-hitachi-hiuxwe2*)
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       echo "Compiler: $CXX"
       SLI_CXXBACKEND="--backend -loopfuse --backend -noparallel"
       CXX_AR="KCC -o"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O3 -lp64"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
     fi
   ;;
   powerpc-ibm-aix5.1*)
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       echo "Compiler: $CXX"
       SLI_cxxflags="${SLI_cxxflags} -qrtti=all"

       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
     fi
   ;;
   *)
     ## For all other OS, we just check for the GNU compiler.
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic -Wno-long-long"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     fi		
     ;;
 esac

##
## Now compose the automake Macro.
## 
## CXXFLAGS now appended instead of prepended, so that it can 
## override default values.
AM_CXXFLAGS="$SLI_threadflags $SLI_cxxflags $SLI_warningflags $SLI_debugflags $SLI_optimizeflags $SLI_SAVE_CXXFLAGS" 

echo "Using AM_CXXFLAGS= $AM_CXXFLAGS"
AC_SUBST(SLI_CXXBACKEND)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(CXX_AR)
])

dnl @synopsis SLI_SET_CFLAGS
dnl
dnl Set flags for various C-compilers.
dnl Each compiler has a different set of options
dnl so we try to identify the compiler and set the
dnl corresponding options.
dnl
dnl Please do not handle C++ flags here, use
dnl SLI_SET_CXXFLAGS for this purpose.
dnl here.
dnl
dnl @author Marc-Oliver Gewaltig
dnl rewritten: 2002-11-15

AC_DEFUN([SLI_SET_CFLAGS],\
[
# Find which compiler C we have to switch compiler flags
# add more conditionals for further compilers:
# However, things are slightly more complicated, since compilers
# on different systems might have the same name (e.g. CC on Irix and Solaris)
# Thus, we first check whether a GNU compiler was diagnosed. If not,
# we assume that the Proprietary compiler is presents by checking the
# OS-name
#
#
SLI_cflags=

# commented emptying CFLAGS, 19.11.04 MD
#
#CFLAGS=

##
## Section for general flags.
##
# compressed target lists ?

if test "$SLI_compression" = yes ; then
 SLI_cflags="-DCOMPR_CONMAT"
fi

# We first branch for the plaform and then branch on the compiler.
# Thus, it is possible to enable platform specific optimizations
# even if GCC is used.
echo "Platform: ${host}"
 case ${host} in
   *linux*)
     if test "$GCC" = "yes"; then
       compversion=`$CC --version`
       if test "${compversion:0:3}" = "icc"; then
         # Intel compiler pretending to be gcc
	 echo "Compiler : icc"
         if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
           SLI_debugflags="-g -inline-debug-info"
         fi
         if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
           SLI_warningflags="-w1"
         fi
         if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
           SLI_optimizeflags="-O3 -fp-model precise"
         fi
       else
         #real g++
         echo "Compiler : gcc"
         if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
           SLI_debugflags="-g"
         fi
         if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
           SLI_warningflags="-W -Wall -pedantic -Wno-long-long"
         fi
         if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
           SLI_optimizeflags="-O2"
         fi
       fi
     fi
     if test "x$enable_bluegene" = xyes; then
	 echo "Compiling for the Blue Gene using compilers specified by CC and CXX environment variables"	
       	  if test "$SLI_debug" = "set"; then
            SLI_debugflags=" "
          fi
          if test "$SLI_warning" = "set"; then
            SLI_warningflags=" "
          fi
          if test "$SLI_optimize" = "set"; then
            SLI_optimizeflags=" "
          fi
      fi
   ;;
   sparc*-sun-solaris2.*)
     SLI_forte=`$CC -V 2>&1 | grep Forte`
     if test "$GCC" = "yes"; then
       echo "Compiler : gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2 -mcpu=v9"
       fi
     fi
    if test -n "$SLI_forte"; then
       echo "Compiler: $SLI_forte"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="+w"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-fast"
        fi
       if test -z "$SLI_threadflags"; then 
         SLI_threadflags="-mt"
       fi
    fi
    ;;
   *-hp-hpux*)
     if test "$GCC" = "yes"; then
       echo "Compiler: gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       SLI_cflags="-AA"
     fi
   ;;
   mips-sgi-irix*)
     if test "$GCC" = "yes"; then
       echo "Compiler: gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-64 -mips4"
       fi
     fi
   ;;
   *-dec-osf*)
     if test "$GCC" = "yes"; then
       echo "Compiler: gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O3 -mieee"
       fi
     else
       echo "Compiler: $CC"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-fast -arch host -tune host -ieee -nofp_reorder "
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
     fi
   ;;
   hppa1.1-hitachi-hiuxwe2*)
     if test "$GCC" = "yes"; then
       echo "Compiler: gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       echo "Compiler: $CC"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O3 -lp64"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
     fi
   ;;
   powerpc-ibm-aix5.1*)
     if test "$GCC" = "yes"; then
       echo "Compiler : gcc"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     else
       echo "Compiler: $CC"

       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags=""
       fi
     fi
   ;;
   *)
     ## For all other OS, we just check for the GNU compiler.
     if test "$GXX" = "yes"; then
       echo "Compiler : g++"
       if test "$SLI_debug" = "set" -a -z "$SLI_debugflags"; then
         SLI_debugflags="-g"
       fi
       if test "$SLI_warning" = "set" -a -z "$SLI_warningflags"; then 
         SLI_warningflags="-W -Wall -pedantic -Wno-long-long"
       fi
       if test "$SLI_optimize" = "set" -a -z "$SLI_optimizeflags"; then 
         SLI_optimizeflags="-O2"
       fi
     fi		
     ;;
 esac

##
## Now compose the automake Macro.
##
## CFLAGS now appended instead of prepended, so that it can 
## override default values.

AM_CFLAGS="$SLI_threadflags $SLI_cflags $SLI_warningflags $SLI_debugflags $SLI_optimizeflags $SLI_SAVE_CFLAGS" 
echo "Using AM_CFLAGS= $AM_CFLAGS"
AC_SUBST(AM_CFLAGS)
])



dnl @synopsis SLI_ALPHA_CXX_STD_BUG
dnl
dnl The Alpha cxx v6.3 and later (at least up to v6.5-014) include
dnl some standard headers (*.h, cerrno) in a non-standard conformant
dnl way when compiling with -std strict_ansi.  In some situations, 
dnl one thus has do undefine __PURE_CNAME before including said
dnl headers, and redefine it afterwards.
dnl
dnl @author Markus Diesmann
dnl @author Hans Ekkehard Plesser (renamed, 2004-12-01)

AC_DEFUN([SLI_ALPHA_CXX_STD_BUG],
[
AC_CACHE_CHECK(whether the compiler does NOT include <*.h> headers ISO conformant,
sli_cv_alpha_cxx_std_bug,
[AC_LANG_SAVE 
 errno_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <cerrno>  
 int e( E2BIG );],, 
 sli_cv_alpha_cxx_std_bug=no, sli_cv_alpha_cxx_std_bug=yes)
 CXXFLAGS=$errno_help
 AC_LANG_RESTORE
])
if test "$sli_cv_alpha_cxx_std_bug" = yes; then
  AC_DEFINE(HAVE_ALPHA_CXX_STD_BUG,,
            [define if the compiler does not include *.h headers ISO conformant])
fi
])

dnl Check if we have long long, and whether it is fully usable.
dnl Tru64 cxx on Alpha is a bit difficult here ...
dnl H E Plesser, 2006-11-30
AC_DEFUN([SLI_HAVE_LONG_LONG],
[
AC_CACHE_CHECK(whether we have usable long long,
sli_cv_have_long_long,
[AC_LANG_SAVE
 longlong_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <iostream>  
void f(long) {}
void f(long long) {}
void g() { long long ll; std::cout << ll; }
],, 
 sli_cv_have_long_long=yes, sli_cv_have_long_long=no)
 CXXFLAGS=$longlong_help
 AC_LANG_RESTORE
])

dnl It is important to define the value as 1 to avoid
dnl conflicts with the definition of HAVE_LONG_LONG from mpi.h
if test "$sli_cv_have_long_long" = yes; then
  AC_DEFINE(HAVE_LONG_LONG, 1,
            [define if we have usable long long type.])
fi
])

AC_DEFUN([SLI_CXX_HAVE_M_E],
[
AC_CACHE_CHECK(whether M_E is defined,
sli_cv_cxx_m_e,
[AC_LANG_SAVE 
 m_e_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <cmath>  
 double e( M_E );],, 
 sli_cv_cxx_m_e=yes, sli_cv_cxx_m_e=no)
 CXXFLAGS=$m_e_help
 AC_LANG_RESTORE
])
if test "$sli_cv_cxx_m_e" = yes; then
  AC_DEFINE(HAVE_M_E,,
            [define if M_E is defined])
fi
])

AC_DEFUN([SLI_CXX_HAVE_M_PI],
[
AC_CACHE_CHECK([whether M_PI is defined],
sli_cv_cxx_m_pi_ignored,
[AC_LANG_SAVE 
 m_pi_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <cmath>  
 double pi( M_PI );],, 
 sli_cv_cxx_m_pi=yes, sli_cv_cxx_m_pi=no)
 CXXFLAGS=$errno_help
 AC_LANG_RESTORE
])
if test "$sli_cv_cxx_m_pi" = yes; then
  AC_DEFINE(HAVE_M_PI,,
            [define if M_PI is defined])
fi
])


AC_DEFUN([SLI_ALPHA_CMATH_MAKROS_IGNORED],
[
AC_CACHE_CHECK(whether the compiler ignores cmath makros,
sli_cv_alpha_cxx_makros_ignored,
[AC_LANG_SAVE 
 m_e_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#undef __PURE_CNAME
#include <cmath>  
#define __PURE_CNAME
 double e( M_E );
 double pi( M_PI );],, 
 sli_cv_alpha_cxx_makros_ignored=yes, sli_cv_alpha_cxx_makros_ignored=no)
 CXXFLAGS=$errno_help
 AC_LANG_RESTORE
])
if test "$sli_alpha_cxx_makros_ignored" = yes; then
  AC_DEFINE(HAVE_CMATH_MAKROS_IGNORED,,
            [define if the compiler ignores cmath makros])
fi
])


AC_DEFUN([SLI_ALPHA_CXX_SIGUSR_IGNORED],
[
AC_CACHE_CHECK(whether the compiler ignores symbolic signal names in signal.h,
sli_cv_alpha_cxx_sigusr_ignored,
[AC_LANG_SAVE 
 sigusr_help=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <signal.h>  
 int e( SIGUSR1 );],, 
 sli_cv_alpha_cxx_sigusr_ignored=no, sli_cv_alpha_cxx_sigusr_ignored=yes)
 CXXFLAGS=$sigusr_help
 AC_LANG_RESTORE
])
if test "$sli_cv_alpha_cxx_sigusr_ignored" = yes; then
  AC_DEFINE(HAVE_SIGUSR_IGNORED,,
            [define if the compiler ignores symbolic signal names in signal.h])
fi
])

AC_DEFUN([SLI_CHECK_ISTREAM_HEADER],
[
AC_CACHE_CHECK(for istream,
sli_cv_check_istream_header,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <istream>  
 ],, 
 sli_cv_check_istream_header=yes, sli_cv_check_istream_header=no)
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_check_istream_header" = yes; then
  AC_DEFINE(HAVE_ISTREAM,,
            [define for istream])
fi
])

AC_DEFUN([SLI_CHECK_OSTREAM_HEADER],
[
AC_CACHE_CHECK(for ostream,
sli_cv_check_ostream_header,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <ostream>  
 ],, 
 sli_cv_check_ostream_header=yes, sli_cv_check_ostream_header=no)
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_check_ostream_header" = yes; then
  AC_DEFINE(HAVE_OSTREAM,,
            [define for ostream])
fi
])


AC_DEFUN([SLI_CHECK_SSTREAM_HEADER],
[
AC_CACHE_CHECK(for sstream,
sli_cv_check_sstream_header,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <sstream>  
 ],, 
 sli_cv_check_sstream_header=yes, sli_cv_check_sstream_header=no)
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_check_sstream_header" = yes; then
  AC_DEFINE(HAVE_SSTREAM,,
            [define for sstream])
fi
])



AC_DEFUN([SLI_CHECK_STL_VECTOR_CAPACITY_BASE_UNITY],
[
AC_CACHE_CHECK(for STL vector capacity base unity,
sli_cv_check_stl_vector_capacity_base_unity,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <vector>
int main(void)
{
  std:: vector<int> v;
  v.push_back(7);
  return v.capacity() > 1;     
} 
 ])], 
 sli_cv_check_stl_vector_capacity_base_unity=yes, 
 sli_cv_check_stl_vector_capacity_base_unity=no,
 [echo "cross compiling, assuming no"
  sli_cv_check_stl_vector_capacity_base_unity=no])
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_check_stl_vector_capacity_base_unity" = yes; then
  AC_DEFINE(HAVE_STL_VECTOR_CAPACITY_BASE_UNITY,,
            [define for STL vector capacity base unity])
fi
])

AC_DEFUN([SLI_CHECK_STL_VECTOR_CAPACITY_DOUBLING],
[
AC_CACHE_CHECK(for STL vector capacity doubling strategy,
sli_cv_check_stl_vector_capacity_doubling,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_RUN_IFELSE([AC_LANG_SOURCE([
 #include <vector>
int main(void)
{
  std:: vector<int> v;
  long l;
  int i;
  v.push_back(7);
  l=v.capacity();
  for(i=0; i<4; i++)
  {
   while (1)
   { 
    v.push_back(7);
    if (v.capacity()!=l) 
     {
      if (v.capacity()/2 != l)
	return 1;
      l=v.capacity();
      break;
     }
   }
  } 
  return 0;
}
 ])], 
 sli_cv_check_stl_vector_capacity_doubling=yes, 
 sli_cv_check_stl_vector_capacity_doubling=no,
 [echo "cross compiling, assuming no"
  sli_cv_check_stl_vector_capacity_doubling=no])
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_check_stl_vector_capacity_doubling" = yes; then
  AC_DEFINE(HAVE_STL_VECTOR_CAPACITY_DOUBLING,,
            [define for STL vector capacity doubling strategy])
fi
])


#
#
#
#
AC_DEFUN([SLI_TIMEBASE],
[
 j=0
 for i in $SLI_timebase
 do
  eval "pp[$j]=$i"
  let j++
 done

 AC_DEFINE_UNQUOTED(NON_DEFAULT_MS_PER_TIC_d,  ${pp[0]}, [ms per tic in simulation ])
 AC_DEFINE_UNQUOTED(NON_DEFAULT_TICS_PER_MS_d,  ${pp[1]}, [tics per ms in simulation ])
 AC_DEFINE_UNQUOTED(NON_DEFAULT_TIMEBASE,       ${pp[2]}, [integer base 10 exponent of tics per ms])
 AC_DEFINE_UNQUOTED(NON_DEFAULT_tics_per_step,  ${pp[3]}, [computation time step in tics])
])


AC_DEFUN([SLI_CHECK_STATIC_TEMPLATE_DECLARATION],
[
AC_CACHE_CHECK(whether static template member declaration fails,
sli_cv_static_template_declaration_fails,
[AC_LANG_SAVE 
 h=$CXXFLAGS
 CXXFLAGS=$AM_CXXFLAGS
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
class A 
{
  int c;
public:
  A(int a)
      :c(a)
  {}
};

template <typename T, typename L>
class B 
{
 protected:
  static A f;
};
template<> A B<int, double>::f;
template<> A B<int,double>::f(9);
 ],, 
 sli_cv_static_template_declaration_fails=no, sli_cv_static_template_declaration_fails=yes)
 CXXFLAGS=$h
 AC_LANG_RESTORE
])
if test "$sli_cv_static_template_declaration_fails" = yes; then
  AC_DEFINE(HAVE_STATIC_TEMPLATE_DECLARATION_FAILS,,
            [define if the compiler fails on static template member declaration])
fi
])

# configure use of MPI
#
# 0. Must be run after GSL_CFLAGS, GSL_LIBS and SLI_LIBS have their final values.
# 1. We may have a compiler wrapper that fixes everything for us.
#    Thus, test first if an mpi program links out of the box.
# 2. If not, try various compiler and linker flags.
#    If successful, test final setup.
# 3. If all tests pass, export HAVE_MPI, MPI_INCLUDE, MPI_LIBS
# HEP 2007-01-03
AC_DEFUN([SLI_NEW_PATH_MPI],
[
  SLI_mpi_include=""
  SLI_mpi_libs=""

  # In particular GSL may introduce -L flags that upset which MPI libs
  # are loaded. To catch such cases, we need to compile our tests with
  # GSL_CFLAGS, GSL_LIBS and SLI_LIBS.
  tmpcxx=$CXXFLAGS
  tmpld=$LDFLAGS
  CXXFLAGS="$AM_CXXFLAGS $GSL_CFLAGS"
  LDFLAGS="$AM_LDFLAGS $SLI_LIBS $GSL_LIBS"

  # initial test, detects working wrappers
  AC_MSG_CHECKING([whether $CXX links MPI programs out of the box])
  AC_TRY_LINK([
#include <mpi.h>
 ],[int* a=0; char*** b=0; MPI_Init(a,b);], 
   SLI_mpi_link_ok=yes,
   SLI_mpi_link_ok=no)
  AC_MSG_RESULT($SLI_mpi_link_ok)

  CXXFLAGS=$tmpcxx
  LDFLAGS=$tmpld

  if test $SLI_mpi_link_ok != yes ; then

    # try to find a candidate setup
    SLI_have_mpi_candidate=no

    if test $SLI_mpi_prefix = unset ; then
      AC_CHECK_LIB(mpi,MPI_Init, SLI_mpi_libs="-lmpi" \
	           SLI_have_mpi_candidate=yes, 
                   [                  

                   # nothing found in default location, try via mpirun
                   AC_MSG_CHECKING(for MPI location using mpirun)

                   # Do not test "which" results for emptyness, many shells return
                   # diagnostic messages if no program is found. Rather, check 
                   # that the result is an executable file. 
                   # Redirect diagnostic output from which to /dev/null
                   # HEP 2006-06-30
                   e=`which mpirun 2> /dev/null`
                   if test -n "$e" && test -x $e ; then
                     p=${e:0:${#e}-11}
                     AC_MSG_RESULT($p) 

                     # Scali needs this macro
                     AC_CHECK_PROG(rp,mpimon,[ -D_REENTRANT],[]) 

                     tmpcxx=$CXXFLAGS
                     tmpld=$LDFLAGS
                     CXXFLAGS="$AM_CXXFLAGS $GSL_CFLAGS $rp -I$p/include"
                     LDFLAGS="$AM_LDFLAGS $SLI_LIBS $GSL_LIBS -L$p/lib64 -L$p/lib -lmpi"
                     AC_CHECK_LIB(mpi,MPI_Init, 
                                  SLI_mpi_include="$rp -I$p/include" \
                                  SLI_mpi_libs="-L$p/lib64 -L$p/lib -lmpi" \
  	                          SLI_have_mpi_candidate=yes)
                     CXXFLAGS=$tmpcxx
                     LDFLAGS=$tmpld
                    fi
                   ]
                  )

    else

      # check in given location
      # Scali needs this macro
      AC_CHECK_PROG(rp,mpimon,[ -D_REENTRANT],[])

      tmpcxx=$CXXFLAGS
      tmpld=$LDFLAGS
      CXXFLAGS="$AM_CXXFLAGS $GSL_CFLAGS $rp -I${SLI_mpi_prefix}/include"
      LDFLAGS="$AM_LDFLAGS $SLI_LIBS $GSL_LIBS -L${SLI_mpi_prefix}/lib64 -L${SLI_mpi_prefix}/lib -lmpi"
      AC_CHECK_LIB(mpi,MPI_Init, \
                   SLI_mpi_include="$rp -I${SLI_mpi_prefix}/include" \
                   SLI_mpi_libs="-L${SLI_mpi_prefix}/lib64 -L${SLI_mpi_prefix}/lib -lmpi" \
                   SLI_have_mpi_candidate=yes)
      CXXFLAGS=$tmpcxx
      LDFLAGS=$tmpld

    fi
   
    if test $SLI_have_mpi_candidate = no ; then
      AC_MSG_ERROR(No sensible MPI setup found. Check your installation!)
    fi 

    # we now have a candidat setup, test it
    AC_MSG_CHECKING([whether MPI candidate $CXX $SLI_mpi_inclue $SLI_mpi_libs works])

    tmpcxx=$CXXFLAGS
    tmpld=$LDFLAGS
    CXXFLAGS="$AM_CXXFLAGS $GSL_CFLAGS $SLI_mpi_include"
    LDFLAGS="$AM_LDFLAGS $SLI_LIBS $GSL_LIBS $SLI_mpi_libs"
    SLI_mpi_link_ok=no

    AC_TRY_LINK([
#include <mpi.h>
    ],[int* a=0; char*** b=0; MPI_Init(a,b);], 
    SLI_mpi_link_ok=yes,
    [
      # Brute force attempt to salvage things
      # according to mpicc, the repetition -lmpich -lpmpich -lmpich is required
      # it also says that on IRIX this may fail
      #
      # MPICH uses non-standard C++ long long
      # gcc-3.3.1 with -pedantic reports this as an error.
      # -Wno-long-long prohibits the generation of an error.
      # Sep 23. 2003, Diesmann
      CXXFLAGS="${CXXFLAGS} -Wno-long-long"
      if test $SLI_mpi_prefix = unset ; then
        retest_ld="-lmpich -lpmpich -lmpich" 
      else
        retest_ld="-L${SLI_mpi_prefix}/lib -lmpich -lpmpich -lmpich" 
      fi
      LDFLAGS="${AM_LDFLAGS} $SLI_LIBS $GSL_LIBS ${retest_ld}"
      AC_TRY_LINK([
#include <mpi.h>
      ],[int* a=0; char*** b=0; MPI_Init(a,b);], 
      SLI_mpi_link_ok=yes \
      SLI_mpi_include="${SLI_mpi_include} -Wno-long-long" \
      SLI_mpi_libs="$retest_ld")
      ])
    AC_MSG_RESULT($SLI_mpi_link_ok)

    CXXFLAGS=$tmpcxx
    LDFLAGS=$tmpld

  fi

  if test $SLI_mpi_link_ok = yes ; then
    AC_DEFINE(HAVE_MPI,1, [MPI is available.])
    AC_SUBST(HAVE_MPI)
    MPI_INCLUDE=$SLI_mpi_include
    AC_SUBST(MPI_INCLUDE)
    MPI_LIBS=$SLI_mpi_libs
    AC_SUBST(MPI_LIBS)
  else
    AC_MSG_ERROR(The MPI candidate did not work. Check your installation!)
  fi 
])



# configure use of MPI for Blue Gene
#
#    We require a compiler wrapper that links the correct libraries
#    therefore, mpi programs should link out of the box.
# AM 2008-05-15
AC_DEFUN([BLUEGENE_MPI],
[
 
  # detect C++ wrapper
  AC_MSG_CHECKING([whether $CXX links MPI programs out of the box])
  AC_LINK_IFELSE([AC_LANG_PROGRAM(
  [[#include <mpi.h>]],
  [int* a=0; char*** b=0; MPI_Init(a,b);])], 
  [BG_CXX_mpi_link_ok=yes],
  [BG_CXX_mpi_link_ok=no])
  AC_MSG_RESULT($BG_CXX_mpi_link_ok)	
 
  if test $BG_CXX_mpi_link_ok = yes ; then
    AC_DEFINE(HAVE_MPI,1, [MPI is available.])
    AC_SUBST(HAVE_MPI)
  else
    AC_MSG_ERROR(Your CXX environment variable must be set to a valid wrapper script to compile C++ programs on Blue Gene.)
  fi 
])



# configure path for MPI library
# strategy:
#    1. evaluate command line option --with-mpi-prefix (not implemented)
#    2. search for native MPI
#    3. search for alternative MPI (determined by location of mpirun)
#
# Diesmann, 18.08.02
#           07.10.02  code for Hitachi added
#
# DEPRECATED
AC_DEFUN([SLI_PATH_MPI_PREFIXED],
[
 AC_MSG_NOTICE([preparing for prefixed MPI])
 MPI_LIBS="${MPI_LIBS} -L${SLI_mpi_prefix}/lib -lmpi"
 AM_CXXFLAGS="${AM_CXXFLAGS} -I${SLI_mpi_prefix}/include"

# flags back-and-forth not necessary if AC_CHECK_LIB not done
# h=$CXXFLAGS
# hl=$LDFLAGS
# CXXFLAGS="${AM_CXXFLAGS}" 
# LDFLAGS="${MPI_LIBS}"
# AC_CHECK_LIB(mpi,MPI_Init,[])
# LDFLAGS=$hl
# CXXFLAGS=$h  

 AC_CHECK_PROG(rp,mpimon,[ -D_REENTRANT],[])
 AM_CXXFLAGS="${AM_CXXFLAGS} $rp"
# AC_DEFINE(HAVE_MPI,1, [MPI is available.])
 AC_MSG_WARN([HAVE_MPI defined without testing whether MPI works as configured.])
 AC_SUBST(HAVE_MPI)
])



#
#
#
#
# DEPRECATED
AC_DEFUN([SLI_PATH_MPI_LIB],
[
 AC_MSG_NOTICE([explicitly specifying path of MPI library])
 MPI_LIBS="${MPI_LIBS} ${SLI_mpi_lib}"
 if test "${SLI_mpi_prefix}" != unset ; then
  AM_CXXFLAGS="${AM_CXXFLAGS} -I${SLI_mpi_prefix}/include"
 else
  p=`expr "${SLI_mpi_lib}" : "\(.*\)/lib/"`
  AM_CXXFLAGS="${AM_CXXFLAGS} -I${p}/include"
 fi

 AC_CHECK_PROG(rp,mpimon,[ -D_REENTRANT],[])
 AM_CXXFLAGS="${AM_CXXFLAGS} $rp"
 AC_DEFINE(HAVE_MPI,1, [MPI is available.])
 AC_SUBST(HAVE_MPI)
])

# DEPRECATED
AC_DEFUN([SLI_PATH_MPI],
[
 if test -e /usr/include/mpi ; then
  AM_CXXFLAGS="${AM_CXXFLAGS}  -I/usr/include/mpi"
 fi

 AC_CHECK_LIB(mpi,MPI_Init, MPI_LIBS="${MPI_LIBS} -lmpi" \
    AC_DEFINE(HAVE_MPI,1, [MPI is available.])
    AC_SUBST(HAVE_MPI), \
 #
 if test "${host}" = "hppa1.1-hitachi-hiuxwe2"; then 
  AC_MSG_WARN("configuring for MPI on Hitachi without checking")
  AM_CXXFLAGS="${AM_CXXFLAGS}  -I/usr/mpi/include -I/usr/local/tools/VAMPIRtrace/lib/lib64"
  MPI_LIBS="${MPI_LIBS}  -L/usr/mpi/lib/lib64/ -L/usr/kai/KCC_BASE/lib64 -L/usr/local/GNU/lib/lib64s/ -L/usr/local/tools/VAMPIRtrace/lib/lib64 -lVT -lpmpi -lm -lmpi"
  #
  AC_MSG_NOTICE(["MPI requires: ${AM_CXXFLAGS}"])
  AC_MSG_NOTICE(["MPI requires: ${MPI_LIBS}"])
 else
  AC_MSG_CHECKING(for MPI location using mpirun)
# Do not test which results for emptyness, many shells return
# diagnostic messages if no program is found. Rather, check 
# that the result is an executable file. 
# Redirect diagnostic output from which to /dev/null
# HEP 2006-06-30
  e=`which mpirun 2> /dev/null`
  if test -n "$e" && test -x $e ; then
    p=${e:0:${#e}-11}
#
# p=`expr "$e" : "\(.*\)/bin/mpirun"`
#
   AC_MSG_RESULT($p) 

# Scali needs this macro
   AC_CHECK_PROG(rp,mpimon,[ -D_REENTRANT],[])

   AM_CXXFLAGS="$AM_CXXFLAGS $rp -I$p/include"
   #
   # according to mpicc, the repetition -lmpich -lpmpich -lmpich is required
   # it also says that on IRIX this may fail
   #
   # MPICH uses non-standard C++ long long
   # gcc-3.3.1 with -pedantic reports this as an error.
   # -Wno-long-long prohibits the generation of an error.
   # Sep 23. 2003, Diesmann
   #
  h=$CXXFLAGS
  hl=$LDFLAGS
  AC_MSG_CHECKING(for MPI library name)
  CXXFLAGS="$AM_CXXFLAGS" 
  LDFLAGS=" -L$p/lib64 -L$p/lib -lmpi"
  AC_TRY_LINK([
#include <mpi.h>
 ],[int* a=0; char*** b=0; MPI_Init(a,b);], 
   pn=" -L$p/lib64 -L$p/lib -lmpi", 
   [
    pn=" -L$p/lib -lmpich -lpmpich -lmpich" 
    AM_CXXFLAGS="$AM_CXXFLAGS -Wno-long-long"
   ])
   AC_MSG_RESULT($pn)
   MPI_LIBS="${MPI_LIBS} $pn"
   LDFLAGS=$hl
   CXXFLAGS=$h              
   AC_DEFINE(HAVE_MPI,1, [MPI is used.])
   AC_SUBST(HAVE_MPI)
  else
   # just warn if MPI is missing so people without it can compile HEP 2002-08-19
   AC_MSG_RESULT("none")
   AC_MSG_WARN("NEST requires an MPI implementation for distributed computing")
  fi
 fi
)
])



# Configure path for the GNU Scientific Library
# Christopher R. Gabriel <cgabriel@linux.it>, April 2000
# 
# Adapted November 2009 by Jochen M. Eppler to also support
# --without-gsl and use --with-gsl instead of --with-gsl-prefix
#
AC_DEFUN([AM_PATH_GSL],
[
AC_ARG_WITH(gsl,[  --with-gsl=PFX		Prefix where GSL is installed (optional)],
            gsl="$withval", gsl="")
AC_ARG_WITH(gsl-exec-prefix,[  --with-gsl-exec-prefix=PFX	Exec prefix where GSL is installed (optional)],
            gsl_exec_prefix="$withval", gsl_exec_prefix="")
AC_ARG_ENABLE(gsltest, [  --disable-gsltest       Do not try to compile and run a test GSL program],
		    , enable_gsltest=yes)

  if test x$gsl == xno ; then
    AC_MSG_WARN(Compiling without GSL. Some models will not be available.)
    ac_have_gsl="no"
  else

  if test x$gsl_exec_prefix != x ; then
     gsl_args="$gsl_args --exec-prefix=$gsl_exec_prefix"
     if test x${GSL_CONFIG+set} != xset ; then
        GSL_CONFIG=$gsl_exec_prefix/bin/gsl-config
     fi
  fi
  if test x$gsl != x ; then
     gsl_args="$gsl_args --prefix=$gsl"
     if test x${GSL_CONFIG+set} != xset ; then
        GSL_CONFIG=$gsl/bin/gsl-config
     fi
  fi

  AC_PATH_PROG(GSL_CONFIG, gsl-config, no)
  min_gsl_version=ifelse([$1], ,0.2.5,$1)
  AC_MSG_CHECKING(for GSL - version >= $min_gsl_version)
  no_gsl=""
  if test "$GSL_CONFIG" = "no" ; then
    no_gsl=yes
  else

dnl See if we can compile without GSL_CFLAGS, since g++ 3.2 issues warnings
dnl about reordering the include path if GSL_CFLAGS includes the standard
dnl search path.  Hans E. Plesser 2002-10-06
    AC_TRY_COMPILE([#include <gsl/gsl_errno.h>], [void foo(void) {}], 
                   [GSL_CFLAGS=],
                   [GSL_CFLAGS=`$GSL_CONFIG $gslconf_args --cflags`])

dnl Only set GSL_LIBS if it is not set previously
    if test x${GSL_LIBS+set} = x ; then 
      GSL_LIBS=`$GSL_CONFIG $gslconf_args --libs`
    fi

    gsl_major_version=`$GSL_CONFIG $gsl_args --version | \
           sed 's/^\([[0-9]]*\).*/\1/'`
    if test "x${gsl_major_version}" = "x" ; then
       gsl_major_version=0
    fi

    gsl_minor_version=`$GSL_CONFIG $gsl_args --version | \
           sed 's/^\([[0-9]]*\)\.\{0,1\}\([[0-9]]*\).*/\2/'`
    if test "x${gsl_minor_version}" = "x" ; then
       gsl_minor_version=0
    fi

    gsl_micro_version=`$GSL_CONFIG $gsl_config_args --version | \
           sed 's/^\([[0-9]]*\)\.\{0,1\}\([[0-9]]*\)\.\{0,1\}\([[0-9]]*\).*/\3/'`
    if test "x${gsl_micro_version}" = "x" ; then
       gsl_micro_version=0
    fi

    if test "x$enable_gsltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GSL_CFLAGS"
      LIBS="$LIBS $GSL_LIBS"

      rm -f conf.gsltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (const char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (void)
{
  int major = 0, minor = 0, micro = 0;
  int n;
  char *tmp_version;

  system ("touch conf.gsltest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_gsl_version");

  n = sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) ;

  if (n != 2 && n != 3) {
     printf("%s, bad version string\n", "$min_gsl_version");
     exit(1);
   }

   if (($gsl_major_version > major) ||
      (($gsl_major_version == major) && ($gsl_minor_version > minor)) ||
      (($gsl_major_version == major) && ($gsl_minor_version == minor) && ($gsl_micro_version >= micro)))
    {
      exit(0);
    }
  else
    {
      printf("\n*** 'gsl-config --version' returned %d.%d.%d, but the minimum version\n", $gsl_major_version, $gsl_minor_version, $gsl_micro_version);
      printf("*** of GSL required is %d.%d.%d. If gsl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If gsl-config was wrong, set the environment variable GSL_CONFIG\n");
      printf("*** to point to the correct copy of gsl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      exit(1);
    }
}

],, no_gsl=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gsl" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GSL_CONFIG" = "no" ; then
       echo "*** The gsl-config script installed by GSL could not be found"
       echo "*** If GSL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GSL_CONFIG environment variable to the"
       echo "*** full path to gsl-config."
     else
       if test -f conf.gsltest ; then
        :
       else
          echo "*** Could not run GSL test program, checking why..."
          CFLAGS="$CFLAGS $GSL_CFLAGS"
          LIBS="$LIBS $GSL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GSL or finding the wrong"
          echo "*** version of GSL. If it is not finding GSL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GSL was incorrectly installed"
          echo "*** or that you have moved GSL since it was installed. In the latter case, you"
          echo "*** may want to edit the gsl-config script: $GSL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
#     GSL_CFLAGS=""
#     GSL_LIBS=""
     ifelse([$3], , :, [$3])
    fi
    AC_SUBST(GSL_CFLAGS)
    AC_SUBST(GSL_LIBS)
    rm -f conf.gsltest
  fi
])


dnl @synopsis SLI_CXX_SPECIALIZATION_BUG
dnl
dnl Tests for a bug in some compilers which can not handle
dnl specialization of function templates if two templates
dnl only differ in the number of template arguments.  If bug
dnl is detected, defines
dnl HAVE_SPECIALIZATION_BUG.
dnl
dnl @author Hans E. Plesser
dnl

AC_DEFUN([SLI_CXX_SPECIALIZATION_BUG],
[AC_CACHE_CHECK(whether the compiler has specialization bug,
hep_cv_cxx_specialization_bug,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
class Foo {} ;
template <class T, int i> void tfunc(T const &value) {}
template <class T> void tfunc(T const &value) {}
template <> void tfunc<Foo>(Foo const &value) {}],, 
 hep_cv_cxx_specialization_bug=no, hep_cv_cxx_specialization_bug=yes)
 AC_LANG_RESTORE
])
if test "$hep_cv_cxx_specialization_bug" = yes; then
  AC_DEFINE(HAVE_SPECIALIZATION_BUG,,
            [define if the compiler has specialization bug])
fi
])

dnl @synopsis SLI_CHECK_XLC_ICE_ON_USING
dnl
dnl Tests for a an internal compiler error obeserved in IBM xlC.
dnl If bug the ICE is detected, defines
dnl HAVE_XLC_ICE_ON_USING
dnl
dnl @author Hans E. Plesser
dnl

AC_DEFUN([SLI_CHECK_XLC_ICE_ON_USING],
[AC_CACHE_CHECK(whether the compiler fails with ICE,
hep_cv_xlc_ice_on_using,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_LINK([
class RandomDev 
{
public:
  virtual double operator()(void) =0; 
  virtual double operator()(double x) =0; 
};

class GenericRandomDevFactory 
{
public:
  virtual RandomDev* create() const =0;
};

template <typename DevType>
class RandomDevFactory: public GenericRandomDevFactory 
{  
public:
  RandomDev* create() const
  {
    return new DevType();
  }      
};

template <typename T>
class Wrapper: public T 
{
public:  
  using RandomDev::operator();
  double operator()(void) { return 0.0; }
  double operator()(double x) { return x; }
};
],
[
RandomDevFactory<Wrapper<RandomDev> > r;
], 
 hep_cv_xlc_ice_on_using=no, hep_cv_xlc_ice_on_using=yes)
 AC_LANG_RESTORE
])
if test "$hep_cv_xlc_ice_on_using" = yes; then
  AC_DEFINE(HAVE_XLC_ICE_ON_USING,,
            [define if the compiler fails with ICE])
fi
])

# SLI_C_INLINE
# ------------
# Do nothing if the compiler accepts the inline keyword.
# Otherwise define inline to __inline__ or __inline if one of those work,
# otherwise define inline to be empty.
#
# HP C version B.11.11.04 doesn't allow a typedef as the return value for an
# inline function, only builtin types.
#
# Modified version of AC_C_INLINE that works gracefully in mixed
# C/C++ environments.  HEP 2003-03-13  
#
AC_DEFUN([SLI_C_INLINE],
[AH_VERBATIM([inline],
[/* Define as `__inline' or `__inline__' if that's what the
    C compiler calls it or to nothing if it is not supported. */
@%:@ifndef __cplusplus
@%:@ undef inline
@%:@endif])dnl
AC_CACHE_CHECK([for inline], ac_cv_c_inline,
[ac_cv_c_inline=no
for ac_kw in inline __inline__ __inline; do
  AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[#ifndef __cplusplus
typedef int foo_t;
static $ac_kw foo_t static_foo () {return 0; }
$ac_kw foo_t foo () {return 0; }
#endif
])],
                    [ac_cv_c_inline=$ac_kw; break])
done
])
case $ac_cv_c_inline in
  inline | yes) ;;
  no) AC_DEFINE(inline) ;;
  *)  echo '#ifndef __cplusplus' >>confdefs.h
      AC_DEFINE_UNQUOTED(inline, $ac_cv_c_inline) 
      echo '#endif'              >>confdefs.h ;;
esac
])# SLI_C_INLINE


AC_DEFUN([SLI_CHECK_EXITCODES],
[
  # under OSX, turn off pop-up window from crash reporter
  if test `uname -s` = "Darwin" ; then
    tmp_crstate=`defaults read com.apple.CrashReporter DialogType`
    defaults write com.apple.CrashReporter DialogType server
  fi

  AC_MSG_CHECKING([for exit code of failed assertions])
  AC_TRY_RUN([
#include <assert.h>
int main()
{
  assert(false);
  return 0; //never reached
}
  ],,
  SLI_EXITCODE_ABORT=$?
  AC_MSG_RESULT($SLI_EXITCODE_ABORT),
  AC_MSG_RESULT(cross-compiling))

  AC_MSG_CHECKING([for exit code of segmentation faults])
  AC_TRY_RUN([
#include <stdio.h>
int main()
{
  char* c = 0;
  sprintf(c, "this operation should provoke a segfault!");
  return 0; //never reached
}
  ],,
  SLI_EXITCODE_SEGFAULT=$?
  AC_MSG_RESULT($SLI_EXITCODE_SEGFAULT),
  AC_MSG_RESULT(cross-compiling))

  # under OSX, set old crash reporter state again
  if test `uname -s` = "Darwin" ; then
    defaults write com.apple.CrashReporter DialogType $tmp_crstate
  fi
])

# Check if source file exists
# Similar to AC_CHECK_FILE, but works also for cross-compilation,
# since we are only checking source files
# SLI_CHECK_SOURCE_FILE([filename], [action if found], [action if not found]) 
AC_DEFUN([SLI_CHECK_SOURCE_FILE],
[
 if test -f $1 ; then
   $2
 else
   $3
 fi
])

# ===========================================================================
#   http://www.gnu.org/software/autoconf-archive/ax_check_compile_flag.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CHECK_COMPILE_FLAG(FLAG, [ACTION-SUCCESS], [ACTION-FAILURE], [EXTRA-FLAGS])
#
# DESCRIPTION
#
#   Check whether the given FLAG works with the current language's compiler
#   or gives an error.  (Warnings, however, are ignored)
#
#   ACTION-SUCCESS/ACTION-FAILURE are shell commands to execute on
#   success/failure.
#
#   If EXTRA-FLAGS is defined, it is added to the current language's default
#   flags (e.g. CFLAGS) when the check is done.  The check is thus made with
#   the flags: "CFLAGS EXTRA-FLAGS FLAG".  This can for example be used to
#   force the compiler to issue an error when a bad flag is given.
#
#   NOTE: Implementation based on AX_CFLAGS_GCC_OPTION. Please keep this
#   macro in sync with AX_CHECK_{PREPROC,LINK}_FLAG.
#
# LICENSE
#
#   Copyright (c) 2008 Guido U. Draheim <guidod@gmx.de>
#   Copyright (c) 2011 Maarten Bosmans <mkbosmans@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ(2.59)dnl for _AC_LANG_PREFIX
AS_VAR_PUSHDEF([CACHEVAR],[ax_cv_check_[]_AC_LANG_ABBREV[]flags_$4_$1])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler accepts $1], CACHEVAR, [
  ax_check_save_flags=$[]_AC_LANG_PREFIX[]FLAGS
  _AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $4 $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])
  _AC_LANG_PREFIX[]FLAGS=$ax_check_save_flags])
AS_IF([test x"AS_VAR_GET(CACHEVAR)" = xyes],
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl AX_CHECK_COMPILE_FLAGS
