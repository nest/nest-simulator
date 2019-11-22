#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# make_links.py
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
#############################################################################################################

# Insert links (reST roles) to PyNest API functions, model names and glossary items that are in the documentation
# Find the correct names of terms, functions, and models from source files
# Search restructured text files for those terms, functions, and models with specific markup and
# replace with proper link
# >> RUN THIS TO MAKE SURE NO DUPLICATES  make_links.py | sort | uniq -c | sort -g
"""
Usage: make_links.py [options] [<outfile>]

Options:
    -n, --dry-run     do not do anything, just show what would be done
    -v, --verbose     give more details what is going on
    -h, --help        print this text
"""
import docopt
import logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger()

from ruamel.yaml import YAML
yaml = YAML()

from pprint import pformat
import glob, os
import sys
import re
import ast

def replace_rst(keywords, replacement, dryrun=False):
    '''
    Find all occurrences in doc/ of rst files that need to be replaced by
    correct mark up to create a link
    '''
    file_out = "tmp.rst"
    regex = r'``(%s)(\(\))?``'           # HERE FIXME THIS IS BROKEN

    nsubs = 0
    nfiles = 0
    for file in glob.glob("**/*.rst", recursive=True):
        #log.info("replacing keywords in " + file)
        if file == file_out:
            continue
        fnsubs = 0
        with open(file, "rt") as fin, open(file_out, "wt") as fout:
            for lineno, line in enumerate(fin):
                for keyword in sorted(keywords, key=len, reverse=True):
                    badpattern = re.compile(regex % keyword)
                    if not badpattern.search(line): continue
                    log.debug("    %s:%d: %s", file, lineno+1, line.rstrip())
                    log.debug("    %s:%d: %s", file, lineno+1, badpattern.sub(replacement, line).rstrip())
                    fnsubs += 1
                    line = badpattern.sub(replacement, line)
                if not dryrun:
                    fout.write(line)
        if not dryrun:
            os.rename(file_out, file)
        if fnsubs > 0:
            log.info('%4d replacements in %s', fnsubs, file)
            nsubs += fnsubs
            nfiles += 1
    log.info('%4d replacements in %d files', nsubs, nfiles)


def allfuncnames(*fileglob):
    '''
    search the python modules and return the function names
    '''
    for fglob in fileglob:
        for file in glob.glob(fglob):
            with open(file) as f:
                node = ast.parse(f.read())
                functions = [n for n in node.body if isinstance(n, ast.FunctionDef)]
                for function in functions:
                    yield function.name

def allmodelnames(*fileglob):
    '''
    get the names of all the models
    '''
    for fglob in fileglob:
        for file in glob.glob(fglob):
            filename, extension = os.path.splitext( os.path.basename(file))
            yield filename

def allglossterms(*fileglob):
    '''
    get all the glossary terms
    '''
    glossary_terms = re.compile('^ [a-zA-Z]')
    for fglob in fileglob:
        for file in glob.glob(fglob):
            with open(file) as infile:
                for line in infile:
                    if glossary_terms.match(line):
                        terms = [line.strip()]
                        for term in terms:
                            yield term

def main():
    args = docopt.docopt(__doc__)
    if args['--verbose']:
        log.setLevel(logging.DEBUG)
    log.debug(pformat(args))

    # find all terms
    funcnames = set(allfuncnames("../pynest/nest/lib/*.py", "../topology/pynest/*.py"))
    modelnames = set(allmodelnames("../models/*.h"))
    glossterms = set(allglossterms('glossary.rst', 'topology/Topology_UserManual.rst'))
    log.info("found %s function names", len(funcnames))
    log.info("found %s model names", len(modelnames))
    log.info("found %s glossary terms", len(glossterms))

    # dump terms for other uses
    if args['<outfile>']:
        log.info('writing lists to %s...', args['<outfile>'])
        with open(args['<outfile>'], 'w') as outfile:
            yaml.dump({
                    'funcnames': funcnames,
                    'modelnames': modelnames,
                    'glossterms': glossterms,
                    }, outfile)

    # consistency checks
    overlap = funcnames.intersection(modelnames.union(glossterms))
    log.warning("duplicates: %s", overlap)   # FIXME incomplete list!
    assert len(funcnames) + len(modelnames) + len(glossterms) == len(funcnames.union(modelnames, glossterms))
    log.info('no overlapping terms')

    if args["--dry-run"]:
        log.warning('not doing replacements because dry-run mode was selected')
    # do actual replacements
    replace_rst(funcnames, ":py:func:`.\g<1>`", dryrun=args["--dry-run"])
    replace_rst(modelnames, ":cpp:class:`\g<1> <nest::\g<1>>`", dryrun=args["--dry-run"])
    replace_rst(glossterms, ":term:`\g<1>`", dryrun=args["--dry-run"])


if __name__ == '__main__':
    main()




