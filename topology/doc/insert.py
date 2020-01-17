#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# insert.py
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
Usage: insert [options] <file>

   Reads <file> and writes it to stdout with all "MISSING" lines replaced as
   stated, implementing a basic templating / include mechanism.

   The input file is expected to have lines with a keyword and insert
   information of the form:

      <words> file: <filename> lines <startID>-<stopID>

   Where `words` can be any text, `filename` is the insertion source file, and
   `startID` and `stopID` are line numbers or keywords identifying the inserted
   snippet.

   Example: Given two files

   Example fileA:
       Some text here
       MISSING TEXT file: fileB lines foo-bar
       Some other text here

   Example fileB:
       a lot of stuff
       other stuff
       foo
       hello world
       bar
       more other stuff

   Calling `insert fileA` will result in the following output:

       Some text here
       hello world
       Some other text here

Options:
    -k, --keyword=<word>
        fills lines only if <word> is in the line. [default: MISSING]

    --fences=<fence>    add fences around the inserted snippets
    -v, --verbose       increase output
    -h, --help          print this text
'''
from docopt import docopt
from textwrap import indent
import re

from pprint import pformat
import logging
log = logging.getLogger()
logging.basicConfig(level=logging.DEBUG)


def snippet(filename, startID, stopID, indent='', **kwargs):
    log.debug("filling snippet %s", kwargs.get('text', '...'))
    pre = True
    start_re = re.compile(r'[\s^]%s[$\s]' % startID)
    stop_re = re.compile(r'[\s^]%s[$\s]' % stopID)
    with open(filename, 'r') as infile:
        for line in infile:
            if pre:
                if start_re.search(line):
                    pre = False
                continue
            if stop_re.search(line):
                break
            yield indent+line


def insert(filename, keyword="", fences=None):
    regex = (r'(?P<indent>\s*)(?P<text>.*%s.*)\s+file:\s+(?P<filename>.+?)\s+'
             r'lines\s+(?P<startID>.+?)-(?P<stopID>.*?)\s*$') % keyword

    log.debug("looking for %s", repr(regex))
    insert_re = re.compile(regex)
    with open(filename, 'r') as infile:
        for line in infile:
            match = insert_re.match(line)
            if match:
                log.debug("Line: %s", line.rstrip())
                log.debug(pformat(match.groupdict()))
                if fences:
                    yield str(fences) + match.group("indent")
                yield from snippet(**match.groupdict())
                if fences:
                    yield str(fences) + match.group("indent")
            else:
                yield line


if __name__ == '__main__':
    args = docopt(__doc__)
    if args['--verbose']:
        log.setLevel(logging.DEBUG)
    log.debug(pformat(args))

    lines = insert(args['<file>'],
                   keyword=args['--keyword'],
                   fences=args.get('--fences', None))

    for line in lines:
        print(line.rstrip("\n"))
