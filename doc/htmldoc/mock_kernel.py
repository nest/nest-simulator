# -*- coding: utf-8 -*-
#
# mock_kernel.py
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
Mock pynestkernel.pyx into dummy python file.
"""

import ast
import re


def has_return(ast_func):
    b = False

    for node in ast.walk(ast_func):
        if isinstance(node, ast.Return):
            b = True

    return b


def convert(infile):
    """Turn cython file into python

    Munge the cython file into parsable python and return the converted
    result as a string.

    The conversion is not correct but it can then be parsed by ast and
    thus coverted to a fully mocked file with dummy classes and fuctions
    (either pass or return MagicMock)
    """
    res, res_tmp = "", ""

    cdef_in_classes_re = re.compile(r'    +cdef')
    cdef_class_re = re.compile(r'cdef class (.*)')
    rmdatatype_re = re.compile(r'\bint\b|\bnew\b|\&(?=\w)|<[^>]+>')

    inclass = False

    for line in infile:
        if "__cinit__" in line:
            line = line.replace("__cinit__", "__init__")
        if inclass is True:
            if cdef_in_classes_re.match(line):
                continue
            if not (line.startswith(" ") or line == "\n"):
                inclass = False
            else:
                line = rmdatatype_re.sub("", line)
                res_tmp += line

        if inclass is False:
            m = cdef_class_re.match(line)
            if m is not None:
                res_tmp += line[5:]   # remove "cdef "
                inclass = True
            else:
                if line.startswith("cdef"):
                    continue

    tree = ast.parse(res_tmp)

    for klass in tree.body:
        bases = ""

        if klass.bases:
            if len(klass.bases) == 1:
                bases = "(" + klass.bases[0].id + ")"
            else:
                bases = "(" + ", ".join([k.id for k in klass.bases]) + ")"

        res += "class {name}{bases}:\n".format(name=klass.name, bases=bases)

        for child in klass.body:
            if isinstance(child, ast.FunctionDef):
                args = ""

                if len(child.args.args) == 1:
                    args = child.args.args[0].arg
                else:
                    args = ", ".join([a.arg for a in child.args.args])

                res += "    def {name}({args}):\n".format(name=child.name, args=args)

                if has_return(child):
                    res += "        return MagicMock()\n"
                else:
                    res += "        pass\n"

        res += "\n\n"

    return res
