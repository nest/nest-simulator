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

'''
Examples to HTML
==================

Generate the documentation for the py examples.

Todo: Make it better!

'''

'''
Define some variables:
'''
import glob
import json
import os
import re

header = '<!DOCTYPE html>\n'
header += '<html>\n'
header += '    <head>\n'
header += '    <title></title>\n'
header += '''<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css">
<!-- Optional theme -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap-theme.min.css">
<script src="https://google-code-prettify.googlecode.com/svn/loader/run_prettify.js"></script>'''
header += '    </head>\n'
header += '    <body style="margin: 5% 10%">\n'
footer = '\n    </body>'
footer += '\n</html>'

with open('ipynbdata.json') as data_file:
    data = json.load(data_file)

outfiles = '../build/examples/'
if os.path.isdir(outfiles):
    pass
else:
    os.mkdir(outfiles)
if os.path.isdir(outfiles + 'examples'):
    pass
else:
    os.mkdir(outfiles + 'examples')

dirList = glob.glob('../../../pynest/examples/*.py')

findex = open(outfiles + "index.html", 'w+')
findex.write(header)
findex.write('<ul>\n')

for example in dirList:

    base = os.path.splitext(example)[0]
    the_name = os.path.basename(base)
    htmlfile = ('%s/examples/%s.html' % (outfiles, the_name))
    print(the_name)
    '''
    Generate ipynb
    --------------
    Use nbformat : This results in a ipynb file with only one big block of code.
    Target is splitting it here as in the html file.
    Danger: nbformat in Jupyter is not the same than in Verion 2 or 3, so:
    Todo: Generating the ipynb JSON like the html!

    Found this:
    https://www.webucator.com/blog/2015/07/bulk-convert-python-files-to-ipython-notebook-files-py-to-ipynb-conversion/
    '''

    '''
    Tear the file into lines and let's see where the \'\'\' are.
    '''

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
            allblocks.update({linenumber: [current, 'comment', '<p>' + line + '</p>']})

    f.close()
    '''
    Building blocks
    '''
    current = ""
    line = ""
    codelines = ""
    commentlines = ""
    typ = ""
    codeblocks = {}
    commentblocks = {}
    sumall = len(allblocks)
    last = dict.keys(allblocks)[-1]

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

            if typ == 'code':
                codelines += line
            if typ == 'comment':
                commentlines += line

        line = blockitem[2]
        typ = blockitem[1]
        current = blockitem[0]

        if linenumber == last:
            if typ == 'comment':
                current = current + 1
                commentlines += line
                commentblocks.update({current: blockitem[2]})
            if typ == 'code':
                current = current + 1
                codelines += line
                codeblocks.update({current: blockitem[2]})

    '''
    Now we have the blocks.
    We have to change the commentblocks a little bit.
    '''

    for key, comment in commentblocks.items():

        '''
        Remove UTF-8 BOM and marker character in input, if present.
        '''
        bom = 'r{^\xEF\xBB\xBF|\x1A}'
        comment = re.sub(bom, r"", commentblocks[key])

        '''
        Standardize line endings:
        DOS to Unix and Mac to Unix
        '''
        endings = 'r{\r\n?}'
        comment = re.sub(endings, r"\n", comment)

        '''
        Ticks
        '''
        doubletick = r'\`\`(.*?)\`\`'
        tick = r'\`(.*?)\`'
        first = re.sub(doubletick, r"<b>\1</b>", comment)
        comment = re.sub(tick, r'<a class="api-reference" href="../api/index.html#\1">\1</a>', first)

        '''
        Header
        ------
        '''
        headersingle = '(.*?)\n(---+)\n'
        comment = re.sub(headersingle, r"<h2>\1</h2>", comment)

        '''
        Header
        =====
        '''
        headerdouble = '(.*?)\n(===+)\n'
        comment = re.sub(headersingle, r"<h1>\1</h1>", comment)

        '''
        LISTS

        Todo: NO NESTED LISTS!
        '''
        preg_ul_li = r'(?m)^\- (.*)?'
        preg_ol_li = r'(?m)^\d+[.]+(.*)?'

        preg_ul_end = r'((?m)^\-+(.*)?)[\r\n]+[\s\t]*[\r\n]+'
        preg_ol_end = r'((?m)^\d+[.]+(.*)?)[\r\n]+[\s\t]*[\r\n]+'

        preg_ul_begin = r'[\r\n]+[\s\t]*[\r\n]+((?m)^\-+(.*)?)'
        preg_ol_begin = r'[\r\n]+[\s\t]*[\r\n]+((?m)^\d+[.]+(.*)?)'

        # ul begin
        comment = re.sub(preg_ul_begin, r"\n<ul>\n\1", comment)
        # ul end
        comment = re.sub(preg_ul_end, r"\1\n</ul>\n", comment)
        # ol begin
        comment = re.sub(preg_ol_begin, r"\n<ol>\n\1", comment)
        # ol end
        comment = re.sub(preg_ol_end, r"\1\n</ol>\n", comment)
        # ul li's
        comment = re.sub(preg_ul_li, r"<li>\1</li>", comment)
        # ol li's
        comment = re.sub(preg_ol_li, r"<li>\1</li>", comment)

        '''
        Breaks
        '''
        textbreak = r'\n\n'
        comment = re.sub(textbreak, r"\n<br>\n", comment)

        '''
        Cite
        '''
        cite = r'\*\*(.*?)\*\*'
        comment = re.sub(cite, r"<b>\1</b>", comment)

        '''
        media
        '''
        media = r'\*(.*?)\*'
        comment = re.sub(media, r"<b>\1</b>", comment)

        '''
        Todo: urls http,....
        '''

        if (comment == "\n"):
            commentblocks[key] = ""
        else:
            commentblocks[key] = "<div>" + comment + "</div>"

    '''
    Codeblocks
    '''

    for key, code in codeblocks.items():
        if (code == "\n"):
            codeblocks[key] = ""
        else:
            codeblocks[key] = '<pre  class="prettyprint">' + code + "</pre>"

    '''
    All together
    '''
    commentblocks.update(codeblocks)

    f = open(('./%s' % (htmlfile,)), 'w+')
    f.write(header)
    for blocknr, comment in commentblocks.items():
        f.write(comment)
    f.write(footer)
    f.close()

    '''
    Create an index file
    '''
    link = '<li><a href="examples/%s.html">%s.html</a></li>\n' % (the_name, the_name)
    findex.write(link)
findex.write('</ul>')
findex.write(footer)
findex.close()
