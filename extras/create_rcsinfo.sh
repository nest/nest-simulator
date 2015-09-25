#!/bin/sh

# Arguments:
#   $1: sourcedir
#   $2: builddir

# default values, if git is not installed or
# if the source directory is not revisioned
branch="no_rcsinfo_available"
version=""

# check if we can run the git command
if command -v git >/dev/null 2>&1; then
  # check if sourcedir is a git repository
    if (cd $1 && git status) >/dev/null 2>&1; then
    # replace branch and version with correct values
    branch=`cd $1; git rev-parse --abbrev-ref HEAD`
    version='@'`cd $1; git rev-parse --short HEAD`
  fi
fi

# create destination directory for rcsinfo.sli
sli_libdir="$2/lib/sli"
mkdir -p $sli_libdir

# create rcsinfo.sli
echo "statusdict /rcsinfo ($branch$version) put" > $sli_libdir/rcsinfo.sli
echo $branch$version

exit 0
