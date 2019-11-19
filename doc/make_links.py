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

# Find all occurrences in doc/ of rst files that need to be replaced by correct mark up to create a link 
def replace_rst(k,v):
    log.info("replacing " + k)
    return
    file_out = "tmp.rst"

    old_pattern = "``" + k + "\W+"
    for file in glob.glob("**/*.rst", recursive=True):
        with open(file, "rt") as fin:
            with open(file_out, "wt") as fout:
                for line in fin:
                    if re.match(old_pattern, line):
                        #log.debug("Filename:" + file + "\n" + line)
                    #fout.write(line.replace(k,v))
        os.rename(file_out, file)

# search the python modules and return the function names
for file in glob.glob("../pynest/nest/lib/*.py"):
    with open(file) as f:
        node = ast.parse(f.read())
        functions = [n for n in node.body if isinstance(n, ast.FunctionDef)]
        for function in functions:
           pypattern = ":py:func:`." + function.name + "`"
           replace_rst(function.name, pypattern)

# get the names of all the models
for file in glob.glob("../models/*.h"):
    filename_w_ext = os.path.basename(file)
    filename, extension = os.path.splitext(filename_w_ext)
    cpp_pattern = ":cpp:class:`" + filename + " <nest::" + filename + ">`"
    replace_rst(filename, cpp_pattern)

glossary_terms = re.compile('^ [a-zA-Z]')
# get all the glossary terms
with open('glossary.rst') as a, open ('topology/Topology_UserManual.rst') as b:
    for line in a or b:
        if glossary_terms.match(line):
            terms = [line.strip()]
            for term in terms:
                gloss_pattern = ":term:`" + term + "`"
                replace_rst(term, gloss_pattern)



#key = re.compile('^class')
 ## Create the pattern for finding match and replacing it with correct syntax
#def link_pyapi(functionNode):
#    pynest_pattern = "``" + functionNode.name + "``"
#    new_pynest_pattern = ":py:func:`." + functionNode.name + "`"
#    replace_rst(pynest_pattern, new_pynest_pattern)
#    for arg in functionNode.args.args:
#        param_pattern = "``" + arg.arg + "``"
#        new_param_pattern = ":py:func:`" + arg.arg + " <." + functionNode.name +">`"
#        #replace_rst(param_pattern, new_param_pattern)
#
#def link_model(cppModel):
#    model_pattern = "``" + cppModel + "``"
#    new_model_pattern = ":cpp:class:`" + cppModel + " <nest::" + cppModel + ">`"
#    replace_rst(model_pattern, new_model_pattern)
#
#def link_term(item):
#    gloss_pattern = "``" + item + "``"
#    new_gloss_pattern = ":term:`" + item + "`"
##    for i, line in enumerate(open(file)):
#            if key.match(line):
#                i = str(i)
#                models = [line.split(' ')[1]]
#                for model in models:
#                    link_model(model)

#    replace_rst(gloss_pattern, new_gloss_pattern)
