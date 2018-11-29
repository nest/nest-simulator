# -*- coding: utf-8 -*-
#
# writers.py
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
NEST help writer
================

Collect all the data and write help files.
"""

import glob
import io
import os
import re
import textwrap
from helpers import cut_it
from string import Template


def write_help_html(doc_dic, helpdir, fname, sli_command_list, keywords):
    """
    Write html.

    Write html for integration in NEST Help-System
    """
    # Loading Template for commands
    ftemplate = io.open('templates/cmd.tpl.html', encoding='utf-8')
    templ = ftemplate.read()
    ftemplate.close()
    # Loading Template for CSS
    cssf = io.open('templates/nest.tpl.css', encoding='utf-8')
    csstempl = cssf.read()
    cssf.close()
    # Loading Template for footer
    footerf = io.open('templates/footer.tpl.html', encoding='utf-8')
    footertempl = footerf.read()
    footerf.close()

    s = Template(templ)

    htmllist = []
    hlplist = []

    name = ''
    for key, value in doc_dic.items():
        if key == "Name":
            name = value.strip()

    for key, value in doc_dic.items():
        if key == "FullName":
            fullname = value.strip("\s\n")
            fullname = re.sub("(\n)", " <br/> ", fullname)

            htmllist.append('''<div class="doc_header">Name:</div>
<div class="doc_paragraph">%s - %s</div>''' %
                            (name, fullname))
            hlpfullname = re.sub(' <br\/> ', '\n', fullname).strip()
            hlplist.append('Name: %s - %s\n' % (name, hlpfullname))

    # sorting linked keywords
    for word in keywords:
        word = word.strip(':')
        for key, value in doc_dic.items():
            if key == word:
                if (key != "Name" and key != "FullName" and
                        key != "SeeAlso" and key != "File"):
                    # strip whitespace and paragraph breaks at start of entry
                    value = re.sub("^(\s*(\n))*\s*", "", value)
                    # strip whitespace and paragraph breaks at end of entry
                    value = re.sub("((\n)\s*)*$", "", value)
                    value = re.sub("(\n)", " <br/> ", value)
                    value = re.sub("(^|\n) ", "&nbsp;", value)
                    htmllist.append('<div class="doc_header">%s: </div>' % key)
                    htmllist.append('<div class="doc_paragraph">%s</div>'
                                    % value)
                    hlpvalue = re.sub(' <br/> ', '\n', value).rstrip()
                    hlpvalue = re.sub('\n ', '\n', hlpvalue).rstrip()
                    hlpvalue = hlpvalue.lstrip('\n')
                    hlpvalue = re.sub('\n[\s?]*\n', '\n', hlpvalue).rstrip()
                    # Better looking .hlp files
                    dedented_text = textwrap.dedent(hlpvalue).strip()
                    hlpcontent = ('%s:\n\n%s\n\n' % (key, dedented_text))
                    hlplist.append(hlpcontent)

    for key, value in doc_dic.items():
        if key == "SeeAlso":
            htmllist.append('<div class="doc_header">%s: </div>' % key)
            hlplist.append('%s:\n' % key)
            htmllist.append('<ul>')
            for i in value:
                see = i.strip("\n ~~")
                if see:
                    if see in sli_command_list:
                        htmllist.append('    <li><a href="../sli/' + see +
                                        '.html">' + see + '</a></li>')
                        hlplist.append('%s' % see)
                    else:
                        htmllist.append('    <li><a href="../cc/' + see +
                                        '.html">' + see + '</a></li>')
                        hlplist.append('%s' % see)
            hlplist.append('')
            htmllist.append('</ul>')

    for key, value in doc_dic.items():
        if key == "File":
            value = value.strip("\n \n $$")
            htmllist.append('''<div class="doc_header">Source:</div>
<div class="doc_paragraph">%s</div>''' % value)
            hlplist.append('Source:\n\n%s' % value)

    htmlstring = (u'\n'.join(htmllist))
    cmdindexstring = s.substitute(indexbody=htmlstring, css=csstempl,
                                  title=name, footer=footertempl)

    if name:  # only, if there is a name
        if fname.endswith('.sli'):
            path = os.path.join(helpdir, 'sli')
        else:
            path = os.path.join(helpdir, 'cc')

        f_file_name = io.open(os.path.join(path, '{}.html'.format(name)),
                              mode='w', encoding='utf-8')
        f_file_name.write(cmdindexstring)
        f_file_name.write(u'\n')
        f_file_name.close()

        f_file_name_hlp = io.open(os.path.join(path, '{}.hlp'.format(name)),
                                  mode='w', encoding='utf-8')
        f_file_name_hlp.write(u'\n'.join(hlplist))
        f_file_name_hlp.write(u'\n')
        f_file_name_hlp.close()


def write_helpindex(helpdir):
    """
    Writes helpindex.html and helpindex.hlp
    ---------------------------------------

    """

    # We only have to generate a helpindex if the help directory exists
    if not os.path.exists(helpdir):
        print("Error: Help directory not found: " + helpdir)
        return

    filelist = glob.glob(os.path.join(helpdir, '*', '*.hlp'))
    html_list = []
    hlp_list = []

    # Loading Template for helpindex.html
    ftemplate = io.open(os.path.join('templates', 'helpindex.tpl.html'),
                        encoding='utf-8')
    templ = ftemplate.read()
    ftemplate.close()
    # Loading Template for CSS
    cssf = io.open(os.path.join('templates', 'nest.tpl.css'),
                   encoding='utf-8')
    csstempl = cssf.read()
    cssf.close()
    # Loading Template for footer
    footerf = io.open(os.path.join('templates', 'footer.tpl.html'),
                      encoding='utf-8')
    footertempl = footerf.read()
    footerf.close()

    s = Template(templ)

    alpha = [('A', 'a'), ('B', 'b'), ('C', 'c'), ('D', 'd'), ('E', 'e'),
             ('F', 'f'), ('G', 'g'), ('H', 'h'), ('I', 'i'), ('J', 'j'),
             ('K', 'k'), ('L', 'l'), ('M', 'm'), ('N', 'n'), ('O', 'o'),
             ('P', 'p'), ('Q', 'q'), ('R', 'r'), ('S', 's'), ('T', 't'),
             ('U', 'u'), ('V', 'v'), ('W', 'w'), ('X', 'x'), ('Z', 'z'), '-',
             ':', '<', '=']

    for doubles in alpha:
        html_list.append('<center><table class="alpha">')
        html_list.append('<table class="letteridx"><tr>')
        for x in alpha:
            html_list.append('<td><a href="#%s">%s</a></td>' % (x[0], x[0]))
        html_list.append('</tr></table></center>')
        html_list.append('<center><table class="commands" id="%s">'
                         % doubles[0])
        for item in sorted(filelist,
                           key=lambda name: name.lower().rsplit('/', 1)[1]):
            fitem = io.open(item, encoding='utf-8')
            itemtext = fitem.read()
            fitem.close()
            # only the basename of the file
            name = os.path.basename(item)[:-4]
            # only the first line of itemtext
            name_line = itemtext.splitlines()[0]
            #
            if name_line.rsplit(' - ')[0] == 'Name: ' + name:
                fullname = name_line.rsplit(' - ')[1]
            else:
                fullname = name
            # file extension
            itemext = item.rsplit('/')[-2]
            if name.startswith(doubles) and os.path.isfile(item):
                # check if 'name' is available in folder with os.path.isfile(
                # checkfile)
                html_list.append('<tr><td class="left">')
                html_list.append('<a href="%s/%s.html">%s</a></td>' %
                                 (itemext, name, name))
                html_list.append('<td>%s</td></tr>' % fullname)

                # Better Format for the index.hlp
                c = len(name)
                hlp_list.append(name + '\t' * (16 - min(c, 60) // 4) +
                                fullname)
            elif not os.path.isfile(item):
                print('WARNING: Checkfile ' + item + ' not exist.')

        html_list.append('</table></center>')
        html_list.append('</table></center>')

    # html_list.append(footer)
    htmlstring = (u'\n'.join(html_list))
    indexstring = s.substitute(indexbody=htmlstring, css=csstempl,
                               footer=footertempl)

    f_helpindex = io.open(os.path.join(helpdir, 'helpindex.html'), mode='w',
                          encoding='utf-8')
    f_helpindex.write(indexstring)
    f_helpindex.write(u'\n')
    f_helpindex.close()

    # Todo: using string template for .hlp
    f_helphlpindex = io.open(os.path.join(helpdir, 'helpindex.hlp'), mode='w',
                             encoding='utf-8')
    f_helphlpindex.write(u'\n'.join(hlp_list))
    f_helphlpindex.write(u'\n')
    f_helphlpindex.close()


def coll_data(keywords, documentation, num, helpdir, fname, sli_command_list):
    """
    Collect data.

    Prepare the data for writing the help.
    """
    see = ""
    relfile = fname.strip()
    doc_dic = {"Id": str(num), "File": relfile}
    iname = None
    for k in keywords:
        if k in documentation:
            if k == "Name:":
                iname = documentation[k].split()[0].rstrip("-")
                ifullname = documentation[k].strip(" \n").strip()
                ifullname = ifullname.lstrip(iname).strip()
                ifullname = ifullname.lstrip("- ")
                if iname:
                    iname = iname.strip('~~')
                    doc_dic.update({"Name": iname})
                if ifullname:
                    doc_dic.update({"FullName": ifullname})
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
                for i in cut_it("\n", documentation[k]):
                    text = text + i.strip() + " \n" + ""
                if text:
                    doc_dic.update({name: text})
    write_help_html(doc_dic, helpdir, fname, sli_command_list, keywords)
