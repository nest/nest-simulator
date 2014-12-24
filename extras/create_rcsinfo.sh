#!/bin/sh

# Arguments:
#   $1: sourcedir
#   $2: builddir

# default values, if svn is not installed or
# if the source directory is not revisioned
branch="not_revisioned_source_dir"
version="0"

# check if we can run the svn command
if command -v svn >/dev/null 2>&1
then
  # and check if sourcedir is a repository
  if svn info $1 >/dev/null 2>&1
  then
    # replace branch and version with correct values
    branch=`svn info $1 | grep ^URL\: | rev | cut -d'/' -f1 | rev | uniq`
    version=`svnversion $1`
  fi
fi

# create destination directory for rcsinfo.sli
sli_libdir="$2/lib/sli"
mkdir -p $sli_libdir

# create rcsinfo.sli
echo "statusdict /rcsinfo ($branch@$version) put" > $sli_libdir/rcsinfo.sli
echo $branch@$version

exit 0
