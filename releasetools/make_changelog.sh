#!/bin/bash

# make_changelog.sh SCRIPTDIR SOURCEDIR SVNID
#
# This script generates a ChangeLog from SVN data.
# SCRIPTDIR : directory containing 
#  - svn2cl.sh
#  - svn2html.xsl
#  - NestChangeLog.css
# SOURCEDIR: source of fresh checkout
# SVNID: ID string for SVN
#
# NOTE:
# This script should be run immediately after the version number
# has been hiked in configure.ac.
#
# The change log will be placed in ./doc/ChangeLog.html
#
# SVN Version:
# Hans Ekkehard Plesser, 2006-07-11
# First Version:
# Hans Ekkehard Plesser, 2004-11-17

SCRIPTDIR=$1
BUILDDIR=$2
SVNID=$3
NESTVERSION=$4

# include revisions as far back as the following SVN revision
# 3559 is first after 1.8.2004 / circa Obidos release
start_revision=3559

chglog_tmphtml=$BUILDDIR/__ChangeLog.html 
chglog_html=$BUILDDIR/doc/ChangeLog.html 

echo "  -> Running svn2cl.sh ..."

# run once to get HTML change log
${SCRIPTDIR}/svn2cl.sh \
    --output=$chglog_tmphtml \
    --html \
    --group-by-day \
    --revision HEAD:$start_revision \
    $SVNID> /dev/null 2>&1

echo "  -> Postprocessing ..."
gawk \
  --file ${SCRIPTDIR}/BeautifyChangeLog.awk \
  --assign cssfile=${SCRIPTDIR}/NestChangeLog.css \
  --assign nestversion=${NESTVERSION} \
  ${chglog_tmphtml} > ${chglog_html}

rm -f ${chglog_tmphtml}

echo "Done. Changelog saved as ${chglog_html}"
