#!/bin/bash

cd ../extras/help_generator
python webdoc.py ../../pynest/examples ../../doc/examples ../../doc/examples
# cd ../../doc/ipynb/
# ipython nbconvert --to markdown exact-integration.ipynb
