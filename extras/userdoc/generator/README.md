# README for the new NEST Help Generator

Processing more in less time with less code.

The parser go through all sli and cc files to find documentation and 
convert it into .html and .hlp files.
This is a replacement for the old help generation mechanism in NEST.

CMAKE integration is also available.

## Usage

    python parse_help.py

## In addition the parser solves issue \#363

    'ISSUE 363
    Currently, the help extractor reads all source files and extracts 
    the documentation, no matter if the function is enclosed in a #ifdef
    block or not. This means that the helpdesk may contain documentation
    for functions or models that have not been compiled, which 
    frequently confuses users.

    This is a follow-up to trac 713.'
