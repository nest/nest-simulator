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

import os
import re
import textwrap
from helpers import cut_it
from string import Template

if os.path.isdir('../cmds'):
    pass
else:
    os.mkdir('../cmds')
if os.path.isdir('../cmds/sli'):
    pass
else:
    os.mkdir('../cmds/sli')
if os.path.isdir('../cmds/cc'):
    pass
else:
    os.mkdir('../cmds/cc')


def write_help_html(doc_dic, file, sli_command_list, keywords):
    """
    Write html.

    Write html for integration in NEST Help-System
    """
    # Loading Template for commands
    ftemplate = open('template/cmd.tpl.html', 'r')
    templ = ftemplate.read()
    ftemplate.close()
    # Loading Template for CSS
    cssf = open('template/nest.tpl.css', 'r')
    csstempl = cssf.read()
    cssf.close()
    # Loading Template for footer
    footerf = open('template/footer.tpl.html', 'r')
    footertempl = footerf.read()
    footerf.close()

    s = Template(templ)

    htmllist = []
    hlplist = []
    name = ''

    for key, value in doc_dic.iteritems():
        if key == "Name":
            name = value.strip()
            # htmllist.append('<title>NEST Command Index: %s</title>%s</head>'
            #                 % (name, header_style) + '<body>')
            # htmllist.append('<h1>Command: %s</h1>' % name)
            # htmllist.append('<div class="wrap">')
    for key, value in doc_dic.iteritems():
        if key == "FullName":
            fullname = value.strip("###### ######")
            fullname = re.sub("(######)", " <br/> ", fullname)
            fullname = re.sub("(\~\~\~)", '\t\t', fullname)
            fullname = re.sub("(\~\~)", '\t', fullname)
            fullname = re.sub("(\~)", ' ', fullname)

            htmllist.append('<b>Name:</b><pre>%s - %s</pre>' %
                            (name, fullname))
            hlpfullname = re.sub(' <br\/> ', '\n', fullname).strip()
            hlplist.append('Name: %s - %s\n' % (name, hlpfullname))

    # print keywords
    # sorting linked keywords
    for word in keywords:
        word = word.strip(':')
        for key, value in doc_dic.iteritems():
            if key == word:
                if (key != "Name" and key != "FullName" and key != "SeeAlso"
                   and key != "File"):
                    value = re.sub("(######)", " <br/> ", value)
                    # value = re.sub("(\~\~)", '  ', value)
                    value = re.sub("(\~\~\~)", '\t', value)
                    value = re.sub("(\~\~)", '  ', value)
                    value = re.sub("(\~)", ' ', value)
                    value = re.sub(' - ', '\t- ', value)

                    htmllist.append('<b>%s: </b>' % key)
                    htmllist.append('<pre>%s</pre>' % value)
                    hlpvalue = re.sub(' <br\/> ', '\n', value).rstrip()
                    hlpvalue = re.sub('\n ', '\n', hlpvalue).rstrip()
                    hlpvalue = hlpvalue.lstrip('\n')
                    hlpvalue = re.sub('\n[\s?]*\n', '\n', hlpvalue).rstrip()
                    # Better looking .hlp files
                    dedented_text = textwrap.dedent(hlpvalue).strip()
                    hlpcontent = ('%s:\n\n%s\n\n' % (key, dedented_text))
                    hlplist.append(hlpcontent)

    for key, value in doc_dic.iteritems():
        if key == "SeeAlso":
            htmllist.append('<b>%s: </b>' % key)
            hlplist.append('%s:\n' % key)
            htmllist.append('<ul>')
            for i in value:
                see = i.strip("###### ###### $$")
                see = i.strip("###### ~~")
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

    for key, value in doc_dic.iteritems():
        if key == "File":
            value = value.strip("###### ###### $$")
            htmllist.append('<b>Source:</b><pre>%s</pre>' % value)
            hlplist.append('Source:\n\n%s' % value)
    htmllist.append('</div>')
    htmlstring = ('\n'.join(htmllist))
    cmdindexstring = s.substitute(indexbody=htmlstring, css=csstempl,
                                  title=name, footer=footertempl)

    if name:  # only, if there is a name
        if file.endswith('.sli'):
            f_file_name = open(('../cmds/sli/%s.html' % name), 'w')
            f_file_name.write(cmdindexstring)
            f_file_name.close()

            f_file_name_hlp = open(('../cmds/sli/%s.hlp' % name), 'w')
            f_file_name_hlp.write(cmdindexstring)
            f_file_name_hlp.close()
        else:
            f_file_name = open(('../cmds/cc/%s.html' % name), 'w')
            f_file_name.write(cmdindexstring)
            f_file_name.close()

            f_file_name_hlp = open(('../cmds/cc/%s.hlp' % name), 'w')
            f_file_name_hlp.write('\n'.join(hlplist))
            f_file_name_hlp.close()
        # return name


def write_helpindex(index_dic_list):
    """
    Write helpindex.html.

    Collect the long list of dicts and transform it toa sorted html file.
    """

    alpha = [('A', 'a'), ('B', 'b'), ('C', 'c'), ('D', 'd'), ('E', 'e'),
             ('F', 'f'), ('G', 'g'), ('H', 'h'), ('I', 'i'), ('J' 'j'),
             ('K', 'k'), ('L', 'l'), ('M', 'm'), ('N', 'n'), ('O', 'o'),
             ('P', 'p'), ('Q', 'q'), ('R', 'r'), ('S', 's'), ('T', 't'),
             ('U', 'u'), ('V', 'v'), ('W', 'w'), ('X', 'x'), ('Z', 'z'), '-',
             ':', '<', '=']

    html_list = []
    hlp_list = []

    # Loading Template for helpindex.html
    ftemplate = open('template/helpindex.tpl.html', 'r')
    templ = ftemplate.read()
    ftemplate.close()
    # Loading Template for CSS
    cssf = open('template/nest.tpl.css', 'r')
    csstempl = cssf.read()
    cssf.close()
    # Loading Template for footer
    footerf = open('template/footer.tpl.html', 'r')
    footertempl = footerf.read()
    footerf.close()

    s = Template(templ)

    from operator import itemgetter
    index_dic_list = sorted(index_dic_list, key=itemgetter('name'))
    for doubles in alpha:
        html_list.append('<center><table class="alpha">')
        html_list.append('<table class="letteridx"><tr>')
        for x in alpha:
            html_list.append('<td><a href="#%s">%s</a></td>' % (x[0], x[0]))
        html_list.append('</tr></table></center>')
        html_list.append('<center><table class="commands" id="%s">'
                         % doubles[0])
        for item in index_dic_list:
            if item['name'].startswith(doubles):
                html_list.append('<tr><td class="left">')
                html_list.append('<a href="%s/%s.html">%s</a></td>' %
                                 (item['ext'], item['name'], item['name']))
                html_list.append('<td>%s</td></tr>' % item['fullname'])
                # Better Format for the index.hlp
                c = len(item['name'])
                if (c < 8):
                    hlp_list.append("%s\t\t\t\t%s" % (item['name'],
                                                      item['fullname']))
                elif (c < 16):
                    hlp_list.append("%s\t\t\t%s" % (item['name'],
                                                    item['fullname']))
                elif(c < 24):
                    hlp_list.append("%s\t\t%s" % (item['name'],
                                                  item['fullname']))
                elif(c < 32):
                    hlp_list.append("%s\t%s" % (item['name'],
                                                item['fullname']))
                else:
                    hlp_list.append("%s %s" % (item['name'],
                                               item['fullname']))
        html_list.append('</table></center>')
        html_list.append('</table></center>')
    #html_list.append(footer)
    htmlstring = ('\n'.join(html_list))
    indexstring = s.substitute(indexbody=htmlstring, css=csstempl,
                               footer=footertempl)

    f_helpindex = open('../cmds/helpindex.html', 'w')
    f_helpindex.write(indexstring)
    f_helpindex.close()

    # Todo: using string template for .hlp
    f_helphlpindex = open('../cmds/helpindex.hlp', 'w')
    f_helphlpindex.write('\n'.join(hlp_list))
    f_helphlpindex.close()


def coll_data(keywords, documentation, num, file, sli_command_list):
    """
    Collect data.

    Prepare the data for writing the help.
    """
    see = ""
    relfile = file.strip()
    doc_dic = {"Id": str(num), "File": relfile}
    for k in keywords:
        if k in documentation:
            if k == "Name:":
                iname = documentation[k].split()[0].rstrip("-")
                ifullname = documentation[k].strip(" ######").strip()
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
    write_help_html(doc_dic, file, sli_command_list, keywords)
    # return(write_help_md(doc_dic))
