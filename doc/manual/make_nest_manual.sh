#!/bin/bash

# Programs to use
LATEX=pdflatex
BIBTEX=bibtex
MKIDX=makeindex
PYTHON=python

# This script builds the user manual from scratch
fname=nest_manual

# Clean-up
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ins,lof,log,lot,pdf,synctex.gz,toc}

# Do the LaTeX Dance
${LATEX}  ${fname}
${BIBTEX} ${fname}
${MKIDX}  ${fname}
${LATEX}  ${fname}
${LATEX}  ${fname}

# remove temporary files
rm -f ${fname}.{aux,bbl,blg,idx,ilg,ind,ins,lof,log,lot,synctex.gz,toc,out}
rm -f *.*~