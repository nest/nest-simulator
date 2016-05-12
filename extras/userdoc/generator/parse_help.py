# -*- coding: utf-8 -*-
#
# parse_help.py
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

"""
Generate NEST help files
========================

Parse all NEST files for documentation and build the help.
"""

import os
import re

from modules.writers import coll_data, check_ifdef

path = '../../../'
path = os.path.abspath(path)
allfiles = [os.path.join(dirpath, f) for dirpath, dirnames, files in
            os.walk(path) for f in files if
            f.endswith((".sli", ".cpp", ".cc", ".h", ".py"))]
num = 0
full_list = []
sli_command_list = []


def cut_it(trenner, text):
    """
    Cut it.

    Cut text by trenner.
    """
    import re
    return re.split(trenner, text)

keywords = ["Name:", "Synopsis:", "Parameters:", "Description:",
            "Options:", "Examples:", "Variants:", "Bugs:",
            "Diagnostics:", "Author:", "FirstVersion:", "Remarks:",
            "Availability:", "References:", "SeeAlso:", "Source:",
            "Sends:", "Receives:", "Transmits:", "Requires:" "Require:"]


# List of all .sli files. Needed for the SeeAlso part.
for file in allfiles:
    if file.endswith('.sli'):
        docstring = r'\/\*[ *\n]?BeginDocumentation\n(.*?)\n*?\*\/'
        f = open(('%s' % (file,)), 'r')
        filetext = f.read()
        f.close()
        items = re.findall(docstring, filetext, re.DOTALL)
        for item in items:
            namestring = r'([ *]?Name[ *]?\:[ *]?)(.*?)([ *]?\-)'
            docnames = re.findall(namestring, item, re.DOTALL)
            for docname in docnames:
                sli_command_list.append(docname[1].strip())

# Now begin to coll the data for the help files
for file in allfiles:
    # .py is for future use
    if not file.endswith('.py'):
        docstring = r'\/\*[ *\n]?BeginDocumentation\n(.*?)\n*?\*\/'
        f = open(('%s' % (file,)), 'r')
        filetext = f.read()
        f.close()
        # Multiline matiching
        items = re.findall(docstring, filetext, re.DOTALL)
        for item in items:
            require = check_ifdef(item, filetext, docstring)
            if require:
                item = '\n\nRequire: ' + require + item
            alllines = []
            s = " ###br###\n"
            for line in item.splitlines():
                line = re.sub(r"(\s\s)", '$$', line)
                alllines.append(line)
            item = s.join(alllines)
            num = num + 1
            documentation = {}
            for token in item.split():
                if token in keywords:
                    keyword_curr = token
                    documentation[keyword_curr] = ""
                else:
                    if keyword_curr in documentation:
                        documentation[keyword_curr] += " " + token
            all_data = coll_data(keywords, documentation, num, file,
                                 sli_command_list)
