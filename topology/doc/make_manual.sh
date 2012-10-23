#!/bin/bash

# Requires nest in Pythonpath
# Requires Matplotlib 1.0.0

# Programs to use
LATEX=pdflatex
BIBTEX=bibtex
MKIDX=makeindex
PYTHON=python

# This script builds the user manual from scratch
fname=Topology_UserManual

# Clean-up
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ins,lof,log,lot,pdf,synctex.gz,toc}
rm -f user_manual_figures/*
rm -f user_namual_scripts/*.log

# Run scripts
cd user_manual_scripts
${PYTHON} layers.py      > layers.log
${PYTHON} connections.py > connections.log
cd ..

# Do the LaTeX Dance
${LATEX}  ${fname}
${BIBTEX} ${fname}
${MKIDX}  ${fname}
${LATEX}  ${fname}
${LATEX}  ${fname}

# remove temporary files
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ind,ins,lof,log,lot,synctex.gz,toc}