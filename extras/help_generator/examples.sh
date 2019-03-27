#!/bin/bash
#
# examples.sh
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

# needs a NEST and source nest_vars.sh
. $HOME/work-nest/nest-git/install-mpi-off/bin/nest_vars.sh

myhome=$PWD
nesthome=$HOME/work-nest/nest-git/nest-simulator
exampleshome=$nesthome/pynest/examples

rm -rf $myhome/py_sample $myhome/py-sample-notebooks

cd $nesthome/extras/help_generator
#$HOME/work-nest/nest-git/nest-simulator/extras/help_generator
#cd ~/work-nest/nest-git/nest-simulator/extras/help_generator

python2 webdoc.py $exampleshome $myhome/py_sample $myhome/py-sample-notebooks $myhome/py_sample

cd $myhome

set -e

imgdir=$myhome/py-original
rm -rf $imgdir
mkdir $imgdir
rm -rf $myhome/more-example-networks
mkdir $myhome/more-example-networks

SKIP_LIST="
    Potjans_2014/
    music_cont_out_proxy_example/ 
    LeNovere_2012/       
"

FAILURES=0

# Trim leading and trailing whitespace and make gaps exactly one space large
SKIP_LIST=$(echo $SKIP_LIST | sed -e 's/ +/ /g' -e 's/^ *//' -e 's/ *$//')

# Create a regular expression for grep that removes the excluded files
case "$SKIP_LIST" in  
    *\ * ) # We have spaces in the list
        SKIP='('$(echo $SKIP_LIST | tr ' ' '|' )')' ;;
    *)
        SKIP=$SKIP_LIST ;;
esac

# Find all examples in the installation directory
EXAMPLES=$(find ${SEARCH_DIR:-$exampleshome} -type f -name \*.py -o -name \*.sli | sort -t. -k3)

# 
if test -n "$SKIP_LIST"; then
    EXAMPLES=$(echo $EXAMPLES | tr ' ' '\n' | grep -vE $SKIP)
fi

cp $EXAMPLES $imgdir

EXAMPLES=$(find ${SEARCH_DIR:-$imgdir} -type f -name \*.py -o -name \*.sli | sort -t. -k3)


echo '<ul>' >> $myhome/more-example-networks/index.tmpl.html
for i in $EXAMPLES ; do

    cd $(dirname $i)

    workdir=$PWD
    
    example=$(basename $i)
    examplename=$(basename $i .py)

    ext=$(echo $example | cut -d. -f2)

    if [ $ext = sli ] ; then
        runner=nest
    elif [ $ext = py ] ; then
		#in letzte Zeile Bild Erzeugung einfügen
		echo "import pylab \npylab.savefig('$imgdir/$examplename.svg')" >> $example
		# Bilder generieren
		#ipython $value
    
        runner=python
    fi

    echo ">>> RUNNING: $imgdir/$example"

    set +e

    # MAKE IMAGES
    $runner $example

	# letzte Zeile in Python Dateien löschen
	sed -i '$d' $example
	# nochmal letzte Zeile in Python Dateien löschen
	sed -i '$d' $example    

    # valides HTML
    mkdir $myhome/py_sample/$examplename
    pandoc -t html5 $myhome/py_sample/$examplename.md -o $myhome/py_sample/$examplename/$examplename.html

    THETITLE=$(awk -vRS="</h2>" '/<h2 .*>/{gsub(/.*<h2 .*>|\n+/,"");print;exit}' $myhome/py_sample/$examplename/$examplename.html)
    if [ "$THETITLE" != "" ] ; then
    	echo '<li><a href="../py_sample/'$examplename'/index.html">'$THETITLE'</a></li>' >> $myhome/more-example-networks/index.tmpl.html
	else
		echo '<li><a href="../py_sample/'$examplename'/index.html">'$examplename'</a></li>' >> $myhome/more-example-networks/index.tmpl.html
	fi

   

    cat $myhome/templates/sample-header.tmpl $myhome/py_sample/$examplename/$examplename.html $myhome/templates/sample-footer.tmpl > $myhome/py_sample/$examplename/index.html


    if [ $? != 0 ] ; then
        echo ">>> FAILURE: $imgdir/$example"
        FAILURES=$(( $FAILURES + 1 ))
        OUTPUT=$(printf "        %s\n        %s\n" "$OUTPUT" "$imgdir/$example")
    else
        echo ">>> SUCCESS: $example"
    fi
    echo
    set -e

    cd $imgdir

done
echo '</ul>' >> $myhome/more-example-networks/index.tmpl.html
cat $myhome/templates/overview-sample-header.tmpl $myhome/more-example-networks/index.tmpl.html $myhome/templates/overview-sample-footer.tmpl > $myhome/more-example-networks/index.html

# rm -rf $imgdir

if [ "x$OUTPUT" != "x" ] ; then
    echo ">>> Failed examples:"
    echo "$OUTPUT"
    echo ""
    exit 1
fi

exit 0
