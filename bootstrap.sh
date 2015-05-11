#!/bin/sh

echo "Bootstrapping NEST"

# configure.ac.in contains code that only runs on libtool 2.2 and
# higher. We copy it to configure.ac and patch it, if a lower version
# of libtool is used.
cp configure.ac.in configure.ac

if test -d autom4te.cache ; then
# we must remove this cache, because it may screw up things if
# configure is run for different platforms.
  echo "  -> Removing old automake cache ..."
  rm -rf autom4te.cache
fi

echo "  -> Running aclocal ..."
aclocal

echo "  -> Running libtoolize ..."
if [ `uname -s` = Darwin ] ; then
# libtoolize is glibtoolize on OSX
  LIBTOOLIZE=glibtoolize
else  
  LIBTOOLIZE=libtoolize
fi

libtool_major=`$LIBTOOLIZE --version | head -n1 | cut -d\) -f2 | cut -d\. -f1`
if test $libtool_major -lt 2; then
  echo "  -> Patching configure.ac for libtoolize 1.5 ..."
  patch -s -f -p0 < extras/libtool-1.5-fix.patch
  $LIBTOOLIZE --force --copy --ltdl
else
  $LIBTOOLIZE --force --copy --quiet --ltdl
fi

echo "  -> Re-running aclocal ..."
aclocal --force

echo "  -> Running autoconf ..."
autoconf

# autoheader must run before automake 
echo "  -> Running autoheader ..."
autoheader

# patch configure.ac for old (pre 1.14) versions of automake
am_major=`automake --version | head -n1 | cut -d\) -f2 | cut -d. -f1`
am_minor=`automake --version | head -n1 | cut -d. -f2`
if test $am_major -lt 1 -o \( $am_major -le 1 -a $am_minor -lt 14 \); then
  echo "  -> Patching configure.ac for automake < 1.14 ..."
  patch -s -f -p0 < extras/automake-pre-1.14-fix.patch
fi

echo "  -> Running automake ..."
automake --foreign --add-missing --force-missing --copy

echo "Done."
