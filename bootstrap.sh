#!/bin/sh

echo "Bootstrapping NEST"

# check for up-to-date versions of libtool and automake

if [ `uname -s` = Darwin ] ; then
# libtoolize is glibtoolize on OSX
  LIBTOOLIZE=glibtoolize
else  
  LIBTOOLIZE=libtoolize
fi

libtool_major=`$LIBTOOLIZE --version | head -n1 | cut -d\) -f2 | cut -d\. -f1`
if test $libtool_major -lt 2; then
    echo "libtool v 1.x is no longer supported."
    echo "Use extras/install_autotools.sh to install up-to-date tools."
    echo "Aborting."
    exit 1
fi

# ensure automake >= v1.14
am_major=`automake --version | head -n1 | cut -d\) -f2 | cut -d. -f1`
am_minor=`automake --version | head -n1 | cut -d. -f2`
if test $am_major -lt 1 -o \( $am_major -le 1 -a $am_minor -lt 14 \); then
    echo "Automake versions < 1.14 are no longer supported."
    echo "Use extras/install_autotools.sh to install up-to-date tools."
    echo "Aborting."
    exit 1
fi

if test -d autom4te.cache ; then
# we must remove this cache, because it may screw up things if
# configure is run for different platforms.
  echo "  -> Removing old automake cache ..."
  rm -rf autom4te.cache
fi

echo "  -> Running aclocal ..."
aclocal

echo "  -> Running libtoolize ..."

$LIBTOOLIZE --force --copy --quiet --ltdl

echo "  -> Re-running aclocal ..."
aclocal --force

echo "  -> Running autoconf ..."
autoconf

# autoheader must run before automake 
echo "  -> Running autoheader ..."
autoheader

echo "  -> Running automake ..."
automake --foreign --add-missing --force-missing --copy

echo "Done."
