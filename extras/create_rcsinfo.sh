#!/bin/sh

# Arguments:
#   $1: sourcedir
#   $2: builddir

# default values
branch="master"
codename=""

# check if we can run the git command
if command -v git >/dev/null 2>&1; then
  # check if sourcedir is a git repository
  if (cd $1 && git status) >/dev/null 2>&1; then
    branch=`cd $1; git rev-parse --abbrev-ref HEAD`
    githash=@`cd $1; git rev-parse --short HEAD`
  fi
fi

version="$branch${codename:--$codename}${githash:-}"

mkdir -p $2/libnestutil
echo "#define NEST_VERSION $version" > $2/libnestutil/version.h
