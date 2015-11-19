#!/bin/sh
#
# ##Generate some pictures from the python examples
# Needs a fully installed NEST.
# Start with: sh generate_example2png.sh

# rm -rf ../build/examples
# rm -rf ../build/temp

set -e

os=$(uname)
clear
read -p "Please enter your NEST installation path (eg "/home/yourname/opt") : " nestpath
echo ""

# see /nest/bin/nest_vars.sh
# There are problems with executing it directly. So using this:
export NEST_INSTALL_DIR=$nestpath/nest
export PYTHONPATH=$NEST_INSTALL_DIR/lib/python2.7/site-packages:$PYTHONPATH
export PYTHONPATH=$NEST_INSTALL_DIR/lib64/python2.7/site-packages:$PYTHONPATH
export PATH=$NEST_INSTALL_DIR/bin:$PATH

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
		if [$os == 'FreeBSD'] || [$os == 'Darwin']; then
			sed -i '' '/savefig/s/\.py//' $value
		else
			sed '/savefig/s/\.py//' -i $value
		fi
    	# generate
    	ipython $value
done

cp ../build/temp/*.png ../build/examples/examples
cp ../../../pynest/examples/*.py ../build/examples/examples
rm -rf ../build/temp
