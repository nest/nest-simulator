# -*- coding: utf-8 -*-
#
# webdoc.py
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
Make markdown and notebook
================================

Parse the examples (python) and convert everything to static markdown and
ipynb.
Putting everything together to integrate in documentation.

Note: the notebook files are generated for good reading. They could work as
they are, but this is not a must.
"""

import json
import os
import re
import sys
import textwrap
from string import Template
from helpers import makedirs
from subprocess import check_output

if len(sys.argv) != 5:
    print("Usage: python webdoc.py <html_> <md_dir> <nb_dir> <index_dir>")
    # python webdoc.py ../../pynest/examples ~/000-md ~/000-html ~/000-nb
    sys.exit(1)

html_, md_dir, nb_dir, index_dir = sys.argv[1:]


makedirs(md_dir)
makedirs(nb_dir)

ipynbpath = '../../doc/model_details'
doc_dir = '../../doc'
img_dir = '../userdoc/img'


def examples_to_md(example, index_file, f_index):
    """Parse the examples."""
    if example:
        base = os.path.splitext(example)[0]
        the_name = os.path.basename(base)
        mdfile = ('{}/{}.md'.format(md_dir, the_name))
        """
        Tear the file into lines and let's see where the \'\'\' are.
        """
        f = open(example, 'r')
        fi = ''.join(f.readlines())
        # The following opening and writing into the file is only a
        # workaround. We need to be sure, that some documentation is there
        # at the beginning and the end.
        fi = '\n\n"""\n\n"""\n\n' + fi + '\n\n"""\n\n"""\n\n'
        f.close()
        f = open('{}/{}.tmp'.format(md_dir, the_name), 'w')
        f.write(fi)
        f.close()
        f = open('{}/{}.tmp'.format(md_dir, the_name), 'r')
        iscomment = False
        current = 0
        linenumber = 0
        sepnumber = 1
        allblocks = {}
        for line in f:
            linenumber = (linenumber + 1)
            preg = '\'\'\''
            preg2 = '\"\"\"'
            if line.strip().startswith(preg) or line.strip().startswith(preg2):
                sepnumber = (sepnumber + 1)
                if sepnumber % 2 == 0:
                    iscomment = True
                    current = (current + 1)
                if sepnumber % 2 == 1:
                    iscomment = False
                    current = (current + 1)
                continue
            """
            Create dictionary like blocks = { current : block }
            """
            if line.startswith("# -*- coding: utf-8 -*-"):
                continue
            if iscomment:
                allblocks.update({linenumber: [current, 'comment', line]})
            else:
                allblocks.update({linenumber: [current, 'code', line]})
            if line.startswith('#'):  # hey, you are a docline
                allblocks.update({linenumber: [current, 'comment', '\n' +
                                               line + '']})
        f.close()

        """
        Todo:
        There are files without any block (twoneurons.py,
        one_neuron_with_noise). md, html and ipynb are empty.
        """

        """
        Building blocks
        """
        current = ""
        line = ""
        codelines = ""
        commentlines = ""
        typ = ""
        codeblocks = {}
        commentblocks = {}
        last = dict.keys(allblocks)[-1]
        for linenumber, blockitem in allblocks.items():
            if current != blockitem[0]:
                if typ == 'comment':
                    commentlines += line
                    commentblocks.update({blockitem[0]: commentlines})
                    commentlines = ""
                if typ == 'code':
                    codelines += '    ' + line
                    codeblocks.update({blockitem[0]: codelines})
                    codelines = ""
            if current == blockitem[0]:
                if typ == 'code':
                    codelines += '    ' + line
                    # begin every line with '~~~\n{: .language-python}'
                if typ == 'comment':
                    commentlines += line
            line = blockitem[2]
            typ = blockitem[1]
            current = blockitem[0]
            if linenumber == last:
                if typ == 'comment':
                    current += 1
                    commentlines += line
                    commentblocks.update({current: blockitem[2]})
                if typ == 'code':
                    current += 1
                    codelines += '    ' + line
                    codeblocks.update({current: blockitem[2]})
        """
        Now we have the blocks.
        We have to change the commentblocks a little bit.
        """
        for key, comment in commentblocks.items():
            """
            Remove UTF-8 BOM and marker character in input, if present.
            """
            bom = 'r{^\xEF\xBB\xBF|\x1A}'
            comment = re.sub(bom, r"", commentblocks[key])
            """
            Standardize line endings:
            DOS to Unix and Mac to Unix
            """
            endings = '{\r\n?}'
            comment = re.sub(endings, r"\n", comment)
            """
            licence
            """
            licence = r'\n#( *)(.*?)\n'
            comment = re.sub(licence, r"", comment)
            if comment == "\n":
                commentblocks[key] = ""
            else:
                commentblocks[key] = comment
            commentblocks[key] = textwrap.dedent(comment + "\n")

        """
        Codeblocks
        """
        for key, code in codeblocks.items():
            endings = 'r{\r\n?}'
            code = re.sub(endings, r"\n", code)
            codeblocks[1] = " "
            if code == "\n" or code == "\n\n":
                codeblocks[key] = " "
            if code == " +\n":
                codeblocks[key] = " "
            else:
                codeblocks[key] = code
            if codeblocks[key] == '<pre class="prettyprint linenums">\n</pre>':
                codeblocks[key] = " "
            codeblocks[key] = code

        # all information for jyputer notebook
        gen_notebook(commentblocks, codeblocks, example)
        """
        All together
        """
        commentblocks.update(codeblocks)
        commentblocks[1] = ""
        f = open(mdfile, 'w+')
        for blocknr, comment in commentblocks.items():
            f.write(comment)
        f.close()

        """
        Delete all temp files
        """
        for dirpath, dirnames, files in os.walk(md_dir):
            for tfile in files:
                if tfile.endswith('.tmp'):
                    tfi = os.path.join(dirpath, tfile)
                    os.remove(tfi)

        """
        Create an index file
        """
        link = '- [{}]({})\n'.format(the_name, the_name)

        f_index.write(link)


def gen_examples():
    """
    Find all examples - .py files.

    :return: pyfi - fill path to example file
    """
    index_file = ('{}/index.md'.format(index_dir))
    f_index = open(index_file, 'w')
    for dirpath, dirnames, files in os.walk(html_):
        if 'LeNovere_2012' in dirnames:
            dirnames.remove('LeNovere_2012')
        if 'Potjans_2014' in dirnames:
            dirnames.remove('Potjans_2014')
        for pyfile in files:
            if pyfile.endswith('.py'):
                pyfi = os.path.join(dirpath, pyfile)
                examples_to_md(pyfi, index_file, f_index)
    f_index.close()


def gen_notebook(comblocks, codblocks, example):
    base = os.path.splitext(example)[0]
    nb_name = os.path.basename(base)
    nb_file = ('{}/{}.ipynb'.format(nb_dir, nb_name))
    # Loading Template code json
    ctemplate = open('templates/nb-code.tpl.json', 'r')
    ctempl = ctemplate.read()
    ctemplate.close()
    # Loading Template md json
    mtemplate = open('templates/nb-md.tpl.json', 'r')
    mtempl = mtemplate.read()
    mtemplate.close()
    # Loading Template nb json
    nbtemplate = open('templates/nb.tpl.json', 'r')
    nbtempl = nbtemplate.read()
    nbtemplate.close()
    comblocksnew = {}
    codblocksnew = {}
    for key, co in comblocks.items():
        co = json.dumps(co)
        mstring = Template(mtempl).substitute(md_source=co)
        comblocksnew[key] = mstring
    for key, co in codblocks.items():
        co = json.dumps(co)
        cstring = Template(ctempl).substitute(code_source=co)
        codblocksnew[key] = cstring
    """
    All together
    """
    comblocksnew.update(codblocksnew)
    comblocksnew[1] = ""

    blocklist = []
    for blocknr, block in comblocksnew.items()[:-1]:
        if block:
            block += ","
            blocklist.append(block)
    for blocknr, block in comblocksnew.items()[-1:]:
        if block:
            block += ""
            blocklist.append(block)

    blockstring = ("".join(blocklist))
    blocknb = Template(nbtempl).substitute(cell_blocks=blockstring)
    f = open(nb_file, 'w+')
    f.write(blocknb)
    f.close()


# def get_github_releases_md():
#
#     """
#     Get release information
#     :return:
#     """
#     relmdfile = (doc_dir + '/releases.md')
#     js = requests.get(
#         'https://api.github.com/repos/nest/nest-simulator/releases')
#     lf = open(relmdfile, 'w+')
#     from operator import attrgetter
#     x = sorted(js.json(), key=attrgetter('created_at'))
#     for i in x:
#         lf.write('\n\n## [NEST ' + i['tag_name'] + '](' + i['url'] + ')\n')
#         lf.write(i['body'].encode('utf-8'))
#     lf.close()


# def get_osb_projects():
#     """
#     Append opensourcebrain projects with NEST support (>1) to index.md
#
#     You need the python OSB API:
#     https: // github.com / OpenSourceBrain / OSB_API
#     Extended:
#     /osb/Project.py class (Project(OSBEntity):attrs =
#     {...'NEST_SUPPORT': 'NEST support',...}
#     """
#     import sys
#     import osb
#     # import json
#
#     # passed_projects = 0
#     # projects = 0
#     attrdump = []
#     a = ["## Nest Models on [Open Source Brain]("
#          "http://www.opensourcebrain.org/)\n"]
#     if __name__ == "__main__":
#         project_num = 1000
#         if len(sys.argv) == 2:
#             project_num = int(sys.argv[1])
#         for project in osb.get_projects(min_curation_level="Low",
#                                         limit=project_num):
#             if project.nest_support > 1:
#                 dump = ({'osb_id': project.id,'osb_slug': project.identifier,
#                          'osb_title': project.name,
#                          'osb_nest': project.nest_support})
#                 attrdump.append(dump)
#
#                 # write html
#                 title = project.name
#                 link = 'http://www.opensourcebrain.org/projects/' + str(
#                     project.id)
#                 a.append('-   ' + '[' + title + '](' + link + ')')
#                 # Pretty printing JSON
#                 # print json.dumps(attrdump, sort_keys=True, indent=4,
#                 # separators=(',', ': '))
#
#     # append to index.md
#     hfile = open(doc_dir + "/index.md", "a")
#     hfile.write("\n".join(a))
#     hfile.close()


def convert_notebook_to_md(ipysdir):
    for dirpath, dirnames, files in os.walk(ipysdir):
        for ipyfile in files:
            if ipyfile.endswith('.ipynb'):
                # ipyfi = os.path.join(dirpath, ipyfile)
                out = check_output(
                  ["jupyter-nbconvert", "{}/{}".format(ipynbpath, ipyfile),
                   "--to", "markdown"])


# get_osb_projects()
gen_examples()
convert_notebook_to_md(ipynbpath)
# get_github_releases_md()
