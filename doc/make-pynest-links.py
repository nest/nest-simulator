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

# Insert links (reST roles) to PyNest API functions that are in the documentation
# Find the correct names of functions from source files
# Search restructured text files for those functions with specific markup and
# replace with proper link
# >> RUN THIS TO MAKE SURE NO DUPLICATES  make_links.py | sort | uniq -c | sort -g
import logging
log = logging.getLogger()
logging.basicConfig(level= logging.DEBUG)

import glob, os
import re
import ast

#TODO
#
# consider functions that also have brackets

# create a dictionary of old_pattern (function name), new_pattern
# open a rst file and see if old pattern exists and replace with new pattern
func_dict = {}

for file in glob.glob("../pynest/nest/**/*.py", recursive=True):
    with open(file) as f:
        node = ast.parse(f.read())
        functions = [n for n in node.body if isinstance(n, ast.FunctionDef)]
        for function in functions:
           old_pattern = "``" + function.name + "()``"
           pypattern = ":py:func:`." + function.name + "`"
           # Adding a new key value pair
           func_dict.update({old_pattern: pypattern})

file_out = 'temp.rst'

#def multiple_replace(adict, text):
for file in glob.glob("userdoc/**/*.rst", recursive=True):
    with open(file) as fin:
#with open('test_files/recording_from_simulations.rst', 'r') as f:
        data = fin.read()
        with open(file_out, "wt") as fout:
            # Create a regular expression from all of the dictionary keys
            regex = re.compile("|".join(map(re.escape, func_dict.keys(  ))))
            # For each match, look up the corresponding value in the dictionary
            fout.write(regex.sub(lambda match: func_dict[match.group(0)], data))

    os.rename(file_out, file)

#        for line in data:
#            for k, v in func_dict.items():
#                if re.findall(k, line):
#                    fout.write(line.replace(k, v))
#   # line.strip() in func_dict:
#            print(line)

#with open('test_files/recording_from_simulations.rst', 'wt') as f:
##overrite the input file with the resulting data
#    f.writelines(data)
#
#            with open('file', 'w') as f:
#        f.write(line)
#        if line.strip() in block:
#            f.write(block[line.strip()])
#for k, v in func_dict.items():
#
#with open(file, 'w') as f:
#    f.writelines(data)
##def replace_rst(k,v):
#    log.info("replacing " + k + " with " + v)
#    #return
#
#    old_pattern = "``" + k + "\W+"
#    file_out = "temp.rst"
#
#    for file in glob.glob("test_files/*.rst", recursive=True):
#        with open(file, "rt") as fin:
#            with open(file_out, "wt") as fout:
#                for line in fin:
#                    if re.match(old_pattern, line):
#                    #log.debug("Filename:" + file + "\n" + line)
#                        fout.write(line.replace(old_pattern, v))
#        os.rename(file_out, file)

