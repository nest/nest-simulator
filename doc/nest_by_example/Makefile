MAIN   = NEST_By_Example

LATEX  = pdflatex
BIBTEX = bibtex
INDEX = makeindex
# DVIPS = dvips
# DVIPDF = dvipdf


all:
	$(LATEX) $(MAIN).tex
	$(BIBTEX) $(MAIN)
	$(INDEX) $(MAIN)
	$(LATEX) $(MAIN).tex
	$(LATEX) $(MAIN).tex
        # $(DVIPS) $(MAIN).dvi -o $(MAIN).ps
        # $(DVIPDF) $(MAIN).dvi
