# -*- coding: utf-8 -*-
#
# include_checker.py
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

import os
import re
import sys

"""
This script suggest C/CPP include orders that conform to the NEST coding style
guidelines. Call the script like (from NEST sources):

For one file:
    python3 extras/include_checker.py -nest $PWD -f nest/main.cpp

For one directory:
    python3 extras/include_checker.py -nest $PWD -d nest

If everything is OK, or only few includes are in the wrong order, it will print
something like:

    Includes for main.cpp are OK! Includes in wrong order: 0

If something is wrong, it will print the suggestion:

    Includes for neststartup.h are WRONG! Includes in wrong order: 5

    ##############################
    Suggested includes for neststartup.h:
    ##############################


    // C includes:
    #include <neurosim/pyneurosim.h>

    // C++ includes:
    #include <string>

    // Generated includes:
    #include "config.h"

    // Includes from conngen:
    #include "conngenmodule.h"

    // Includes from sli:
    #include "datum.h"
"""

# We would like to have files that are not actually provided by
# the NEST Initiative, e.g. implementing the Google Sparsetable,
# to be exactly like they come from the upstream source.
excludes_files = []


class IncludeInfo():
    filename = ""
    name = ""
    spiky = False
    origin = "a_unknown"

    def __init__(self, filename, name, spiky, all_headers):
        self.filename = filename
        self.name = name
        self.spiky = spiky
        self.set_origin(all_headers)

    def is_header_include(self):
        return (self.name.split('.')[0] == self.filename.split('.')[0] or
                self.name.split('.')[0] == self.filename.split('_impl.')[0])

    def is_cpp_include(self):
        return (not self.name.endswith('.h') and
                not self.name.endswith('.hpp') and self.spiky)

    def is_c_include(self):
        return self.name.endswith('.h') and self.spiky

    def is_project_include(self):
        return (not self.spiky and
                (self.name.endswith('.h') or self.name.endswith('.hpp')))

    def set_origin(self, includes):
        for k, v in includes.iteritems():
            if self.name in v:
                self.origin = k
                break

    def cmp_value(self):
        v = 8 if self.is_header_include() else 0
        v += 4 if self.is_c_include() else 0
        v += 2 if self.is_cpp_include() else 0
        v += 1 if self.is_project_include() else 0
        return v

    def __cmp__(self, other):
        s = self.cmp_value()
        o = other.cmp_value()
        val = o - s
        if val == 0:
            val = cmp(self.origin, other.origin)
            if val == 0:
                return cmp(self.name, other.name)
            else:
                return val
        else:
            return val

    def to_string(self):
        l_guard = '<' if self.spiky else '"'
        r_guard = '>' if self.spiky else '"'
        return '#include ' + l_guard + self.name + r_guard


def all_includes(path):
    result = {}
    dirs = [d for d in next(os.walk(path))[1] if d[0] != '.']
    for d in dirs:
        for root, dirs, files in os.walk(os.path.join(path, d)):
            tmp = [f for f in files if f.endswith(".h") or f.endswith(".hpp")]
            if len(tmp) > 0:
                result[d] = tmp

    return result


def create_include_info(line, filename, all_headers):
    match = re.search('^#include ([<"])(.*)([>"])', line)
    name = match.group(2)
    spiky = match.group(1) == '<'
    return IncludeInfo(filename, name, spiky, all_headers)


def get_includes_from(file, all_headers):
    includes = []
    with open(file, 'r') as f:
        for line in f:
            if line.startswith('#include'):
                includes += [create_include_info(line,
                                                 os.path.basename(file),
                                                 all_headers)]
    return includes


def is_include_order_ok(includes):
    s_incs = sorted(includes)
    return len(includes) - len([i for i, s in zip(includes, s_incs)
                                if i.name == s.name])


def print_includes(includes):
    s_incs = sorted(includes)

    is_c = False
    is_cpp = False
    origin = ""

    for i in s_incs:
        if not i.is_header_include():
            if not is_c and i.is_c_include():
                is_c = True
                is_cpp = False
                origin = ""
                print("\n// C includes:")

            if not is_cpp and i.is_cpp_include():
                is_c = False
                is_cpp = True
                origin = ""
                print("\n// C++ includes:")

            if i.is_project_include() and origin != i.origin:
                is_c = False
                is_cpp = False
                origin = i.origin
                if i.origin == "a_unknown":
                    print("\n// Generated includes:")
                else:
                    print("\n// Includes from " + i.origin + ":")

        print(i.to_string())


def process_source(path, f, all_headers, print_suggestion):
    if f in excludes_files:
        print("Not checking file " + f + " as it is in the exclude list. " +
              "Please do not change the order of includes.")
        return 0
    includes = get_includes_from(os.path.join(path, f), all_headers)
    order_ok = is_include_order_ok(includes)
    if order_ok <= 2:
        print("Includes for " + f + " are OK! Includes in wrong order: " +
              str(order_ok))
    if order_ok > 2:
        print("Includes for " + f + " are WRONG! Includes in wrong order: " +
              str(order_ok))
        if print_suggestion:
            print("\n##############################")
            print("Suggested includes for " + f + ":")
            print("##############################\n")
            print_includes(includes)
            print("\n##############################")

    return order_ok


def process_all_sources(path, all_headers, print_suggestion):
    count = 0
    for root, dirs, files in os.walk(path):
        for f in files:
            if re.search("\.h$|\.hpp$|\.c$|\.cc|\.cpp$", f):
                # valid source file
                count += process_source(root, f, all_headers, print_suggestion)
        for d in dirs:
            count += process_all_sources(os.path.join(root, d), all_headers,
                                         print_suggestion)
    return count


def usage(exitcode):
    print("Use like:")
    print("  " + sys.argv[0] + " -nest <nest-base-dir>" +
                               " (-f <filename> | -d <base-directory>)")
    sys.exit(exitcode)


if __name__ == '__main__':
    print_suggestion = True
    if len(sys.argv) != 5:
        usage(1)

    if sys.argv[1] == '-nest' and os.path.isdir(sys.argv[2]):
        all_headers = all_includes(sys.argv[2])
    else:
        usage(2)

    if sys.argv[3] == '-f' and os.path.isfile(sys.argv[4]):
        path = os.path.dirname(sys.argv[4])
        file = os.path.basename(sys.argv[4])
        process_source(path, file, all_headers, print_suggestion)

    elif sys.argv[3] == '-d' and os.path.isdir(sys.argv[4]):
        dir = sys.argv[4]
        process_all_sources(dir, all_headers, print_suggestion)

    else:
        usage(3)
