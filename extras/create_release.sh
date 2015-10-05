#!/bin/csh -e

#  Shell script to roll a NEST tarball from the git repository

# first, we check for autoconf >= 2.60, as tarballs built with older versions
# may cause problems on many machines. See trac #122 for details.
set ac_ver = `autoconf --version | head -n 1 | cut -d' ' -f 4`
set ac_ver_major = `echo $ac_ver | cut -d\. -f 1`
set ac_ver_minor = `echo $ac_ver | cut -d\. -f 2`

if ( ! ( ( $ac_ver_major == 2 ) & ( $ac_ver_minor >= 60 ) ) ) then
  echo ""
  echo "ERROR: You need autoconf >= 2.60 to build a tarball."
  echo ""
  exit 1
endif

set workdir = `pwd`
endif

set git_url = "https://github.com/nest/nest-simulator.git"
set nest_srcdir = __NEST_SOURCE
set nest_blddir = __NEST_BUILD

# set umask to 022 (gives 755/644 permission in tarball)
umask 022

# remove any existing checkout
rm -rf $nest_srcdir
rm -rf $nest_blddir

echo ""
echo "NEST Tarball Creation Script"
echo "----------------------------"
echo ""
echo "You are about to create a tarball of the master branch of NEST."
echo "In order to include local changes, you have to commit them first."
echo ""
echo -n "Do you want to continue [Y/n]? "
set ans=`head -1`

if ( $ans =~ [nN]* ) then
  exit 0
endif

echo -n "Git tag for the tarball [empty to skip tagging]: "
set tag=`head -1`

echo -n "Add git revision hash to version number [y/N]? "
set add_rev=`head -1`

echo ""
echo "Creating clone ..."
git clone $git_url $nest_srcdir
cd $nest_srcdir

if ( "$tag" != "" ) then
  git checkout releases
  git merge --commit master
endif

set label = ""
if ( $add_rev =~ [yY]* ) then
  set label = "-"`git rev-parse --short HEAD`
endif

set nest_major = `grep SLI_MAJOR= configure.ac | cut -d= -f2`
set nest_minor = `grep SLI_MINOR= configure.ac | cut -d= -f2`
set nest_patch = `grep SLI_PATCHLEVEL= configure.ac | cut -d= -f2 | cut -d- -f1`$label
set nest_version = "$nest_major.$nest_minor.$nest_patch"

sed -i'' -e "s/SLI_PATCHLEVEL=.*/SLI_PATCHLEVEL=$nest_patch/" configure.ac
sed -i'' -e "s/AC_INIT.*/AC_INIT([nest], [$nest_version], [nest_user@nest-initiative.org])/" configure.ac

if ( "$tag" != "" ) then
  sed -i'' -e "s/version=\x22\x22/version=\x22$tag\x22/" extras/create_rcsinfo.sh
  git commit -a -m "$tag"
  git push
  git tag -a $tag -m "Release of $tag"
  git push --tags
endif

echo "Bootstrapping clone ..."
./bootstrap.sh

cd $workdir
mkdir $nest_blddir
cd $nest_blddir

echo "Configuring build directory ..." 
../$nest_srcdir/configure --prefix=$HOME/opt/nest > /dev/null

echo "Precompile Cython files ..."
make -C pynest pynestkernel.cpp

echo "Rolling tarball ..."
make -s dist

cd $workdir
mv $nest_blddir/*.tar.gz $workdir

echo "Cleaning up ..."
rm -rf $nest_srcdir
rm -rf $nest_blddir
	
echo ""
echo "Done."
echo ""
