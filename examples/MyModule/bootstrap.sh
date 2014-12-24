#!/bin/sh

echo "Bootstrapping MyModule"

# Choose correct libtoolize executable
if [ `uname -s` = Darwin ] ; then
  # libtoolize is glibtoolize on OSX
  LIBTOOLIZE=glibtoolize
else
  LIBTOOLIZE=libtoolize
fi

# We require libtool 2 or newer, stop and inform if it is missing
libtool_major=`$LIBTOOLIZE --version | head -n1 | cut -d\) -f2 | cut -d\. -f1`
libtool_minor=`$LIBTOOLIZE --version | head -n1 | cut -d\) -f2 | cut -d\. -f2`
if test $libtool_major -lt 2 -o \( $libtool_major -le 2 -a $libtool_minor -lt 2 \); then
  echo "\nLibtool 2.2 or later is required to build MyModule."
  echo "\nextras/install_autotools.sh will automatically install recent versions of autotools in your home directory.\n"
  exit 1
fi

# We require automake 1.14 or newer, stop and inform if it is missing
am_major=`automake --version | head -n1 | cut -d\) -f2 | cut -d. -f1`
am_minor=`automake --version | head -n1 | cut -d. -f2`
if test $am_major -lt 1 -o \( $am_major -le 1 -a $am_minor -lt 14 \); then
  echo "\nAutomake 1.14 or later is required to build MyModule."
  echo "\nextras/install_autotools.sh will automatically install recent versions of autotools in your home directory.\n"
  exit 1
fi

if test -d autom4te.cache ; then
# we must remove this cache, because it
# may screw up things if configure is run for
# different platforms.
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
