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
import sys
import shutil
import textwrap

from modules.writers import coll_data, write_helpindex
from modules.helpers import check_ifdef

path = '../../../'
path = os.path.abspath(path)
allfiles = []
for dirpath, dirnames, files in os.walk(path):
    if not re.findall(r'[.?]*MyModule[.?]*', dirpath):
        for f in files:
            if f.endswith((".sli", ".cpp", ".cc", ".h", ".py")):
                allfiles.append(os.path.join(dirpath, f))


num = 0
full_list = []
sli_command_list = []
cc_command_list = []
index_dic_list = []

keywords = ["Name:", "Synopsis:", "Examples:", "Description:", "Parameters:",
            "Options:", "Requires:", "Require:", "Receives:", "Transmits:",
            "Sends:", "Variants:", "Bugs:", "Diagnostics:", "Remarks:",
            "Availability:", "References:", "SeeAlso:", "Author:",
            "FirstVersion:", "Source:"]

# TODO combine the following 2 loops to make it faster (and shorter)

# Now begin to collect the data for the helpindex.html

dcs = r'\/\*[\s?]*[\n?]*BeginDocumentation[\s?]*\:?[\s?]*[.?]*\n(.*?)\n*?\*\/'
for file in allfiles:
    if not file.endswith('.py'):
        f = open(('%s' % (file,)), 'r')
        filetext = f.read()
        f.close()
        items = re.findall(dcs, filetext, re.DOTALL)
        # List of all .sli files. Needed for the SeeAlso part.
        # if file.endswith('.sli'):
        index_dic = {}
        fullname = ""
        for item in items:
            namestring = r'([\s*]?Name[\s*]?\:[\s*]?)(.*?)([\s*]?\n)'
            docnames = re.findall(namestring, item, re.DOTALL)
            for docname in docnames:
                if docname[1].strip():
                    fullname = docname[1].strip()
                    docname = fullname.split()[0].rstrip("-")
                    fullname = fullname.lstrip(docname).strip()
                    fullname = fullname.lstrip("-").strip()
                if file.endswith('.sli'):
                    sli_command_list.append(docname.strip())
                    index_dic = {'name': docname, 'ext': 'sli'}
                else:
                    index_dic = {'name': docname, 'ext': 'cc'}
                filename_dic = {'file': file}
                if fullname:
                    fullname_dic = {'fullname': fullname}
                    index_dic.update(fullname_dic)
                    filename_dic = {'file': file}
                else:
                    fullname_dic = {'fullname': ''}
                    index_dic.update(fullname_dic)
                index_dic.update(filename_dic)
            index_dic_list.append(index_dic)
write_helpindex(index_dic_list)

# Now begin to collect the data for the help files and start generating.
dcs = r'\/\*[\s?]*[\n?]*BeginDocumentation[\s?]*\:?[\s?]*[.?]*\n(.*?)\n*?\*\/'
for file in allfiles:
    # .py is for future use
    if not file.endswith('.py'):
        f = open(('%s' % (file,)), 'r')
        filetext = f.read()
        f.close()
        # Multiline matiching to find codeblock
        items = re.findall(dcs, filetext, re.DOTALL)
        for item in items:
            # Check the ifdef in code
            require = check_ifdef(item, filetext, dcs)
            if require:
                item = '\n\nRequire: ' + require + item
            alllines = []
            s = " ######\n"
            for line in item.splitlines():
                name_line = re.findall(r"([\s*]?Name[\s*]?\:)(.*)", line)
                if name_line:
                    # Clean the Name: line!
                    name_line_0 = name_line[0][0].strip()
                    name_line_1 = name_line[0][1].strip()
                    line = name_line_0 + ' ' + name_line_1
                line = textwrap.dedent(line).strip()
                # Tricks for the blanks
                line = re.sub(r"(\s){5,}", '~~~ ', line)
                line = re.sub(r"(\s){3,4}", '~~ ', line)
                line = re.sub(r"(\s){2}", '~ ', line)
                alllines.append(line)
            item = s.join(alllines)
            num += 1
            documentation = {}
            keyword_curr = ""
            for token in item.split():
                if token in keywords:
                    keyword_curr = token
                    documentation[keyword_curr] = ""
                else:
                    if keyword_curr in documentation:
                        documentation[keyword_curr] += " " + token

            all_data = coll_data(keywords, documentation, num, file,
                                 sli_command_list)
if len(sys.argv) > 1:
    shutil.rmtree(sys.argv[1], ignore_errors=True)
    shutil.copytree("../cmds", sys.argv[1])
