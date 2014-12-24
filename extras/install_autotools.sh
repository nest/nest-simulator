#!/bin/sh

# This script installs modern versions of GNU Autotools
# in $HOME/autotools. The install path can be changed below.
# For installation, a directory AutotoolsBuild is created in the
# current working directory.
#
#  install_autotools.sh /this/path  --> install to /this/path
#  install_autotools.sh             --> install to $HOME/autotools
#   
#  - This script requires wget to work.
#  - Only minimal error checking.
#  - You must delete AutotoolsBuild before running the script
#    a second time.
#  - Under OSX, a crash report window may pop up while m4 is configured.
#    There is no need to worry about this, just click "Ignore".

# Install path
# prefix to $HOME if you want to put the binaries into 
# $HOME/bin and other files into $HOME/{lib,include,share, ...}
if [ $# -gt 0 ] ; then
  prefix=$1
else
  prefix=$HOME/autotools
fi

# Package versions to install
pkgs="m4 autoconf automake libtool"
m4_ver=1.4.17
autoconf_ver=2.69
automake_ver=1.14.1
libtool_ver=2.4.3

wgetprog=`which wget`
if [ -z $wgetprog ] ; then
  echo
  echo "This script requires wget, which seems unavailable on you machine."
  echo "Please download the following files"
  echo
  for pkg in $pkgs ; do
    vname=${pkg}_ver
    eval pkgfull=${pkg}-"\$$vname"
    pkgtar=${pkgfull}.tar.gz
    echo "  ftp://anonymous@ftp.gnu.org/gnu/${pkg}/${pkgtar}"
  done
  echo
  echo "unpack them and run"
  echo 
  echo "  ./configure --prefix=xxx && make && make install" 
  echo
  echo "in each directory in the order given above."
  echo "Make sure that prefix/bin is in your PATH!"
  echo
  exit 1
fi

# current directory
currdir=`pwd`

# Work dir
wrkdir=$currdir/AutotoolsBuild
mkdir -p $wrkdir
cd $wrkdir

# ensure we see newly installed pkgs when building next in line
savepath=$PATH
export PATH=$prefix/bin:$PATH

# fetch, configure, build
installed_pkg=""
for pkg in $pkgs ; do
  vname=${pkg}_ver
  eval pkgfull=${pkg}-"\$$vname"
  pkgtar=${pkgfull}.tar.gz
  $wgetprog ftp://anonymous@ftp.gnu.org/gnu/${pkg}/${pkgtar}
  tar zxf $pkgtar
  gunzip $pkgtar
  cd $pkgfull
  ./configure --prefix=$prefix && make && make install && installed_pkg="${installed_pkg} ${pkgfull}"  
  if [ $? -ne 0 ] ; then
    echo
    echo "Something went wrong while building $pkg"
    echo "Only these packages are installed: $installed_pkg"
    echo
    exit 2
  fi
  cd ..
done

cd $currdir
rm -rf $wrkdir

# link libtoolize to glibtoolize
cd $prefix/bin
ln -s libtoolize glibtoolize
cd $currdir

# restore old path
export PATH=$savepath

echo
echo 
echo " ********************************************** "
echo
echo " Installed $installed_pkg "
echo " to ${prefix}."
echo
echo " Remember to add"
echo " $prefix/bin" 
echo " to the start of your PATH!"
echo
echo " ********************************************** "
echo
