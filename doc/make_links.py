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
import logging
log = logging.getLogger()
logging.basicConfig(level= logging.DEBUG)

import glob, os
import re
import ast

def replace_rst(keywords, replacement):
    '''
    Find all occurrences in doc/ of rst files that need to be replaced by
    correct mark up to create a link
    '''
    file_out = "tmp.rst"
    regex = r'``(%s)(\(\))?``'           # HERE FIXME THIS IS BROKEN

    for file in glob.glob("**/*.rst", recursive=True):
        #log.info("replacing keywords in " + file)
        with open(file, "rt") as fin, open(file_out, "wt") as fout:
            for lineno, line in enumerate(fin):
                for keyword in sorted(keywords, key=len, reverse=True):
                    badpattern = re.compile(regex % keyword)
                    if not badpattern.search(line): continue
                    log.debug("    %s:%d: %s", file, lineno+1, line.rstrip())
                    log.debug("    %s:%d: %s", file, lineno+1, badpattern.sub(replacement, line).rstrip())
                    #fout.write(line.replace(keyword, replacement))
        #os.rename(file_out, file)

def allfuncnames(fileglob="../pynest/nest/lib/*.py"):
    '''
    search the python modules and return the function names
    '''
    for file in glob.glob(fileglob):
        with open(file) as f:
            node = ast.parse(f.read())
            functions = [n for n in node.body if isinstance(n, ast.FunctionDef)]
            for function in functions:
                yield function.name

def allmodelnames(fileglob="../models/*.h"):
    '''
    get the names of all the models
    '''
    for file in glob.glob(fileglob):
        filename, extension = os.path.splitext( os.path.basename(file))
        yield filename

def allglossterms():
    '''
    get all the glossary terms
    '''
    glossary_terms = re.compile('^ [a-zA-Z]')
    with open('glossary.rst') as a, open ('topology/Topology_UserManual.rst') as b:
        for line in a or b:
            if glossary_terms.match(line):
                terms = [line.strip()]
                for term in terms:
                    yield term

funcnames = set(allfuncnames())
modelnames = set(allmodelnames())
glossterms = set(allglossterms())

overlap = funcnames.intersection(modelnames.union(glossterms))
print("duplicates: %s" % overlap)
assert len(funcnames) + len(modelnames) + len(glossterms) == len(funcnames.union(modelnames, glossterms))
replace_rst(funcnames, ":py:func:`.\g<1>`")
replace_rst(modelnames, ":cpp:class:`\g<1> <nest::\g<1>>`")
replace_rst(glossterms, ":term:`\g<1>`")


