#!/bin/bash

# This script converts the Jupyter Notebook to PDF

# Programs to use
LATEX=pdflatex
BIBTEX=bibtex
CONVERTER="jupyter nbconvert"

fname=NEST_by_Example
template=latex_template

# Convert from Jupyter Notebook to LaTeX
${CONVERTER} --to latex --template ${template} "${fname}"

# Do the LaTeX Dance
${LATEX}  "${fname}"
${BIBTEX} "${fname}"
${LATEX}  "${fname}"
${LATEX}  "${fname}"

# Remove temporary files
rm -f "${fname}".{aux,bbl,blg,log,out,tex}
rm -fr "${fname}_files"
