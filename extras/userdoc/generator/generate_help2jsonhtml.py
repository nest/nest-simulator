# -*- coding: utf-8 -*-
#
# generate_help2jsonhtml.py
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

import json
import os
import re
import shutil

# import urllib
# path = '/home/steffen/work/nest-2.8.0'
# docpath = '../userdoc'
buildpath = '../build/json/json'
buildpath_html = '../build/reference'
if os.path.isdir(buildpath_html):
    pass
else:
    os.mkdir(buildpath_html)
if os.path.isdir(buildpath_html + '/cmds'):
    pass
else:
    os.mkdir(buildpath_html + '/cmds')
if os.path.isdir(buildpath):
    pass
else:
    jsonpath = '../build/json'
    shutil.copytree('assets/json', jsonpath, symlinks=False, ignore=None)
    os.mkdir(jsonpath + '/json')
# List of all files in "path"
path = '../../../'
path = os.path.abspath(path)
allfiles = [os.path.join(dirpath, f) for dirpath, dirnames, files in os.walk(path) for f in files if f.endswith((".sli", ".cpp", ".cc", ".h"))]

num = 0
full_list = []
name_list = []
f_file = open(('%s/0_index.json' % (buildpath)), 'w')
f_file.write("[\n")
f_html = open(('%s/cmd_index.html' % (buildpath_html)), 'w')
f_html.write('<html><body><ul>')


# little helper
def cut_it(trenner, text):
    return re.split(trenner, text)


def write_help_html(doc_dic):
    name = ""
    htmllist = ['<html><body style="padding: 5% 10%;">']
    # namelist = []
    for key, value in doc_dic.iteritems():
        if key == "Name":
            name = value.strip()
            htmllist.append('<h1>%s</h1>' % name)
    for key, value in doc_dic.iteritems():
        if (key != "Name" and key != "SeeAlso" and key != "Id"):
            htmllist.append('<h2>%s</h2>' % key)
            htmllist.append('<p>%s<p>' % value)
    for key, value in doc_dic.iteritems():
        if key == "SeeAlso":
            htmllist.append('<h2>%s</h2>' % key)
            htmllist.append('<ul>')
            for i in value:
                see = i.strip()
                if see:
                    htmllist.append('    <li><a href="' + see + '.html">' + see + '</a></li>')
            htmllist.append('</ul>')
    htmllist.append('</body></html>')
    if name:  # only, if there is a name
        f_file_name = open(('%s/cmds/%s.html' % (buildpath_html, name)), 'w')
        f_file_name.write('\n'.join(htmllist))
        f_file_name.close()
        return name


def coll_data(keywords, documentation, num, file, f_file):
    iname = ""
    see = ""
    relfile = cut_it("/nest-2.8.0/", file)
    relfile = relfile[1].strip()
    doc_dic = {"Id": str(num), "File": relfile}
    for k in keywords:
        if k in documentation:
            if k == "Name:":
                iname = cut_it("-", documentation[k])
                iname = iname[0].strip()
                if iname:
                    doc_dic.update({"Name": iname})
                    full_list = ('{"Name": ' + json.dumps(iname) + ', "Id": ' + json.dumps(str(num)) + '},' + "\n")
                    f_file.write((full_list))
            elif k == "SeeAlso:" or k == "See also:" or k == "See Also:":
                doc_list = []
                see_alsos = cut_it(",", documentation[k])
                for i in see_alsos:
                    see = i.strip()
                    if see:
                        doc_list.append(see)
                doc_dic.update({"SeeAlso": doc_list})
            else:
                text = ""
                name = k.replace(":", "")
                for i in cut_it(",", documentation[k]):
                    text = text + i.strip() + "\n"
                if text:
                    doc_dic.update({name: text})
    f_file_one = open(('%s/%s.json' % (buildpath, iname)), 'w')
    f_file_one.write(json.dumps(doc_dic))
    f_file_one.close()
    return(write_help_html(doc_dic))


for file in allfiles:
    docstring = r'\/\*[ *\n]?BeginDocumentation\n(.*?)\n*?\*\/'
    f = open(('%s' % (file,)), 'r')
    filetext = f.read()
    f.close()
    items = re.findall(docstring, filetext, re.DOTALL)  # Multiline matiching
    for item in items:
        num = num + 1
        keywords = ["Name:", "Synopsis:", "Parameters:", "Description:", "Options:", "Examples:", "Variants:", "Bugs:", "Diagnostics:", "Author:", "FirstVersion:", "Remarks:", "Availability:", "References:", "SeeAlso:", "Source:", "Sends:", "Receives:", "Transmits:"]
        documentation = {}
        for token in item.split():
            if token in keywords:
                keyword_curr = token
                documentation[keyword_curr] = ""
            else:
                if keyword_curr in documentation:
                    documentation[keyword_curr] += " " + token
        all_data = coll_data(keywords, documentation, num, file, f_file)
        name_list.append(all_data)
sorted_name_list = sorted(name_list)
# print(sorted_name_list)
for i in sorted_name_list:
    if i:
        f_html.write('    <li><a href="cmds/' + i + '.html">' + i + '</a></li>')
f_html.write("</ul></body></html>")
f_html.close()
f_file.write('{}\n]')
f_file.close()
