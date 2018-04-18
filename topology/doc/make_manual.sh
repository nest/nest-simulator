#!/bin/bash

# Requires Matplotlib 1.0.0
python -c "import matplotlib" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Python module 'matplotlib' not found. Exiting."
    exit 1
fi

# Requires nest in PYTHONPATH
python -c "import nest" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Python module 'nest' not found. Exiting."
    exit 1
fi


# Programs to use
LATEX=pdflatex
BIBTEX=bibtex
MKIDX=makeindex
PYTHON=python

# This script builds the user manual from scratch
fname=Topology_UserManual

# Clean-up
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ins,lof,log,lot,pdf,synctex.gz,toc}
rm -rf user_manual_figures
rm -f user_namual_scripts/*.log
mkdir user_manual_figures

# Run scripts / -u ensures unbuffered output 
cd user_manual_scripts
${PYTHON} -u layers.py      > layers.log
${PYTHON} -u connections.py > connections.log
cd ..

# Do the LaTeX Dance
${LATEX}  ${fname}
${BIBTEX} ${fname}
${MKIDX}  ${fname}
${LATEX}  ${fname}
${LATEX}  ${fname}

# remove temporary files and figures
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ind,ins,lof,log,lot,synctex.gz,toc}
rm -rf user_manual_figures