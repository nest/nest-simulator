# -*- coding: utf-8 -*-
#
# colormaps.py
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

import glob
import json
from json.encoder import JSONEncoder
import os
# import pprint
# from pprint import pprint
# import re
# import sys


with open('ipynbdata.json') as data_file:
    data = json.load(data_file)

exfiles = '../build/examples'
outfiles = '../build/examples/examples/'
dirList = glob.glob('../../../pynest/examples/*.py')
# dirList = {'../../../pynest/examples/twoneurons.py'}
if os.path.isdir(exfiles):
    pass
else:
    os.mkdir(exfiles)
if os.path.isdir(outfiles):
    pass
else:
    os.mkdir(outfiles)


'''
Generate ipynb
--------------
Using nbformat : This results in a ipynb file with only one big block of code.
Target is splitting it here as in the html file.
Danger: nbformat in Jupyter is not the same then in Verion 2 or 3, so:
Using it for py to ipynb conversion is outdatet!

My Hack: Using string conversions

'''

for example in dirList:
    base = os.path.splitext(example)[0]
    the_name = os.path.basename(base)
    ipyfile = ('%s.ipynb' % (outfiles + the_name))
    print(the_name)

    '''
    Tear the file into lines and let's see where the \'\'\' are.
    '''

    # What is the line for?
    f = open(('%s' % (example,)), 'r')
    linenumber = 0
    iscomment = False
    current = 0
    linenumber = 0
    sepnumber = 1

    commentlines = {}
    commenttypes = {}
    commentcurrent = {}
    codelines = {}
    codetypes = {}
    codecurrent = {}

    allblocks = {}

    for line in f:
        linenumber = (linenumber + 1)
        preg = '\'\'\''
        # if line.strip() == preg:
        if line.strip().startswith(preg):
            sepnumber = (sepnumber + 1)
            if sepnumber % 2 == 0:
                iscomment = True
                current = (current + 1)
            if sepnumber % 2 == 1:
                iscomment = False
                current = (current + 1)
            continue

        '''
        Create dictionary like blocks = { current : block }
        '''
        if line.startswith("# -*- coding: utf-8 -*-"):
                continue

        if iscomment:
                allblocks.update({linenumber: [current, 'comment', line]})

        if not(iscomment):
                allblocks.update({linenumber: [current, 'code', line]})

        if line.startswith('#'):  # hey, you are a docline
            allblocks.update({linenumber: [current, 'comment', '\\' + line]})
    # print(allblocks)
    '''
    Building blocks
    '''
    current = ""
    line = ""
    codelines = ""
    commentlines = ""
    codelines0 = ""
    commentlines0 = ""
    typ = ""
    codeblocks = {}
    commentblocks = {}
    block = 0

    sumall = len(allblocks)
    # print sumall
    last = dict.keys(allblocks)[-1]
    # print last
    # print(allblocks)

    for linenumber, blockitem in allblocks.items():
        if current != blockitem[0]:
            if typ == 'comment':
                commentlines += line
                commentblocks.update({blockitem[0]: commentlines})
                commentlines = ""

            if typ == 'code':
                codelines += line
                codeblocks.update({blockitem[0]: codelines})
                codelines = ""

        if current == blockitem[0]:
            if blockitem[0] == 0:  # There is a problem with files without doc. blockitem[0] is then everytime 0

                if typ == 'comment':
                    commentlines0 += line
                if typ == 'code':
                    codelines0 += line
            else:
                if typ == 'code':
                    codelines += line
                if typ == 'comment':
                    commentlines += line

        line = blockitem[2]
        typ = blockitem[1]
        current = blockitem[0]
        lastcommentline = ""
        lastcodeline = ""

        if linenumber == last:
            if typ == 'comment':
                current = current + 1
                commentlines += line
                lastcommentline = commentlines
                commentblocks.update({current: blockitem[2]})
            if typ == 'code':
                current = current + 1
                codelines += line
                lastcodeline = codelines
                codeblocks.update({current: blockitem[2]})

    if commentlines0:
        commentlines0 = commentlines0 + lastcommentline
        # commentblocks.update({0: commentlines0}) # License text disabled
    if codelines0:
        codelines0 = codelines0 + lastcodeline
        codeblocks.update({1: codelines0})
    '''
    Commentblocks
    ----------
    '''

    for key, comment in commentblocks.items():
        comment = JSONEncoder().encode(comment)
        commentblocks[key] = data['markdown'] + comment

    '''
    Codeblocks
    ----------
    '''
    for key, code in codeblocks.items():
        if code:
            code = JSONEncoder().encode(code)
            codeblocks[key] = data['code'] + code

    '''
    All together
    ------------
    '''
    commentblocks.update(codeblocks)

    mid = ''
    for block in commentblocks.items():
        mid += block[1] + data['between']

    f = open(('./%s' % (ipyfile,)), 'w+')
    f.write(data['begin'] + mid + data['end'])
    f.close()

    print(('CONVERTED:  %s' % (example,)))
