#!/bin/csh -e

#  Shell script to roll a NEST tarball from the repository
#
#  The script will:
#
#  - make a clean checkout to __NEST_SOURCE in the local directory
#  - set SLI_PATCHLEVEL to current SVN revision in configure.ac.in
#  - ask if to tag the release in the repository
#  - create an up-to-date doc/ChangeLog.html
#  - run bootstrap
#  - configure in __NEST_BUILD
#  - roll a tarball
#  - move the tarball to the local directory
#  - copy the ChangeLog.html to the local directory
#  - remove __NEST_SOURCE and __NEST_BUILD
#
#  The script requires SVN access and releasetools is sourcedir
#
#  11/2004 first version by Hans Ekkehard Plesser
#  06/2006 modified by Moritz Helias for prerelease
#  07/2006 modified by Hans Ekkehard Plesser to use SVN
#  08/2007 modified by Jochen Eppler to use SVN revision as patchlevel
#  03/2009 modified by Jochen Eppler to allow tarballing branches
#  11/2009 modified by Jochen Eppler to use the HTTPS repository


# first, we check for autoconf >= 2.60, as tarballs built with older versions
# may cause problems on many machines. See #122 for details.
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

# commandline parsing
if ( $#argv != 0 ) then
  if ( $#argv == 2 & $1 == "--branch" ) then
    echo "Using branch '"$2"'"
    set svndir="branches/"$2
    set branch="_$2"
  else
    echo "buildnest.sh options:"
    echo "  --branch <branch>    Make a tarball of <branch> instead of trunk"
    echo "  --help               Print this help information and exit"
    echo
    exit 0
  endif
else
  set svndir="branches/nest-2.4/2.4.1"
  set branch=""
endif

# need to find releasetools path
set scriptpath = `pwd`/$0
set scriptdir = `echo $scriptpath | gawk '{ split($1, a, /\/buildnest.sh/); print a[1]; }'`

# absolute pathnames give trouble with make
set nest_srcdir = __NEST_SOURCE$branch
set nest_blddir = __NEST_BUILD$branch

# check that helpers are available
if ( ! ( -f $scriptdir/make_changelog.sh ) ) then
  echo ""  
  echo "ERROR: Helper scripts not available in $scriptdir!"
  echo ""
  exit 1
endif

# SVN ID to use
set svn_id = https://svn.nest-initiative.org/nest/$svndir
set svn_tags = https://svn.nest-initiative.org/nest/tags

# set umask to 022 (gives 755/644 permission in tarball)
umask 022

# remove any existing checkout
rm -rf $nest_srcdir
rm -rf $nest_blddir

# print a nice welcome screen
echo ""
echo "NEST Tarball Creation Script"
echo "----------------------------"
echo ""
echo "You are about to create a tarball of the current SVN revision of trunk."
echo "In order to have local changes included, you have to commit them first."
echo ""
echo -n "Do you want to continue [y/N]? "
set ans=`head -1`
echo ""

set label="-pre"

if ( $ans =~ [yY]* ) then
  echo -n "Do you want to tag this tarball in the SVN repository [y/N]? "
  set ans=`head -1`
  if ( $ans =~ [yY]* ) then
    set label=""
  endif
else
  exit 0
endif

set label="$branch$label"

# check out
echo ""
echo "Creating fresh checkout ..."
svn --quiet checkout $svn_id $nest_srcdir

# add svn revision number to tarball name
set svnver=`svnversion $nest_srcdir`
set patchlevel = "1"

# move to nest_srcdir
cd $nest_srcdir

# edit configure.ac.in
echo ""
echo -n "Setting patch level in configure.ac"
sed -i -e "s/SLI_PATCHLEVEL=svn/SLI_PATCHLEVEL=$patchlevel/" configure.ac.in

set nest_minor   = "4"
set nest_major   = "2"
set nest_version = "$nest_major.$nest_minor.$patchlevel"
set nest_nprog   = "nest-$nest_version"

sed -i -e "s/AC_INIT(\[nest\], \[$nest_major\.$nest_minor\.svn\]/AC_INIT([nest], [$nest_version]/" configure.ac.in

echo " to $nest_nprog"

# tag new version if real run
#if ( "$label" == "" ) then
#  echo ""
#  echo "Tagging new version as $nest_nprog" 
#  svn copy $svn_id $svn_tags/$nest_nprog -r $svnver -m "Tagging release as $nest_nprog"
#endif

# remove all candidate modules from configure.ac---this change MUST NOT be checked in
gawk -f $scriptdir/clear_extra_modules.awk configure.ac.in > configure.ac.tmp \
  && mv configure.ac.tmp configure.ac.in

# bootstrap after modification of configure.ac.in
echo ""
./bootstrap.sh

# create ChangeLog; done in source dir, since this is a source file
echo ""
echo "Creating ChangeLog ..."
$scriptdir/make_changelog.sh $scriptdir $workdir/$nest_srcdir $svn_id $nest_version

# now configure and roll in separate directory
# build dir must be parallel to source dir!
cd $workdir
mkdir $nest_blddir
cd $nest_blddir

# must have relative path here to avoid spurious 
# directories to be created on rolling tarball
echo ""
echo "Configuring build directory ..." 
../$nest_srcdir/configure --prefix=$HOME/opt/nest > /dev/null

# pre-compile Cython code
make -C pynest pynestkernel.cpp

echo ""
echo "Rolling tarball ..."
make -s dist

# leave build dir
cd $workdir

# move/copy files to work directory
mv $nest_blddir/$nest_nprog.tar.gz $workdir
cp $nest_srcdir/doc/ChangeLog.html $workdir

echo ""
echo "Done."
echo ""
echo "NEST Tarball   : ${workdir}/${nest_nprog}.tar.gz"
echo "NEST ChangeLog : ${workdir}/ChangeLog.html"
echo ""

# clean up
rm -rf $nest_srcdir
rm -rf $nest_blddir
