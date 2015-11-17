#!/bin/sh
#
# ##Generate some pictures from the python examples
# Needs a fully installed NEST.
# Start with: sh generate_example2png.sh

# rm -rf ../build/examples
# rm -rf ../build/temp

set -e
mypath="pwd"
clear
read -p "Please enter your NEST installation path (eg "/home/yourname/opt") : " nestpath
echo ""

export PYTHONPATH=$nestpath/nest/lib/python2.7/site-packages

mkdir -p ../build/examples
mkdir -p ../build/temp/

cp ../../../pynest/examples/*.py  ../build/temp/

unset $orig
orig=$(ls ../build/temp/*.py)

for value in $orig
do
	echo $value
	recode -f utf-8 $value
	# append after last line
    	echo "import pylab \npylab.savefig('$value.png')" >> $value
        sed '/savefig/s/\.py//' -i $value
    	# generate
    	ipython $value
	# delete last line
	sed -i '$d' $value
	# agaiin
	sed -i '$d' $value
	# sed '1,5d' -i $value
done

cp ../build/temp/*.png ../build/examples/examples
cp ../../../pynest/examples/*.py ../build/examples/examples
rm -rf ../build/temp
