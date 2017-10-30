#!/bin/sh

# Arguments:
#   $1: sourcedir
#   $2: builddir

# default value, if git is not installed or if the source directory is
# not under version control
version="v2.14.0"

# check if we can run the git command
if command -v git >/dev/null 2>&1; then
  # check if sourcedir is a git repository
  if (cd $1 && git status) >/dev/null 2>&1; then
    branch=`cd $1; git rev-parse --abbrev-ref HEAD`
    if [ $branch = "HEAD" ] || [ $branch = "master" ]; then
      branch=""
    else
      branch=$branch'@'
    fi
    version=$branch`cd $1; git rev-parse --short HEAD`
  fi
fi

# create destination directory for rcsinfo.sli
sli_libdir="$2/lib/sli"
mkdir -p $sli_libdir

# create rcsinfo.sli
echo "statusdict /rcsinfo ($version) put" > $sli_libdir/rcsinfo.sli
echo $version

exit 0
