# -*- coding: utf-8 -*-
#
# ticket-659-copyright.py
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
import sys

EXIT_SUCCESS = 0
EXIT_BAD_HEADER = 20
EXIT_NO_SOURCE = 126

try:
    source_dir = os.environ['NEST_SOURCE']
except KeyError:
    print("Please make NEST_SOURCE environment variable to point to the source tree you want to check!")
    sys.exit(EXIT_NO_SOURCE)

exclude_dirs = [
    'libltdl',
    '.svn',
]

exclude_files = [
    'doc/copyright_header.cpp',
    'doc/copyright_header.py',
    'do_tests.py',
    'libnestutil/config.h',
    'libnestutil/hashtable-common.h',
    'libnestutil/libc_allocator_with_realloc.h',
    'libnestutil/sliconfig.h',
    'libnestutil/sparseconfig.h',
    'libnestutil/sparsetable.h',
    'libnestutil/template_util.h',
    'libnestutil/type_traits.h',
    'librandom/mt19937.cpp',
    'librandom/mt19937.h',
    'nestrc.sli',
    'nest/static_modules.h',
    'pynest/pynestkernel.cpp',
    'rcsinfo.sli',
    'get-pip.py'
]

templates = {
    ('cc', 'cpp', 'h', 'sli'): 'cpp',
    ('py', 'pyx', 'pxd'): 'py',
}

template_contents = {}

for extensions, template_ext in templates.items():
    with open("{0}/doc/copyright_header.{1}".format(source_dir, template_ext)) as template_file:
        template = template_file.readlines()
        for ext in extensions:
            template_contents[ext] = template

total_files = 0
total_errors = 0
for dirpath, _, fnames in os.walk(source_dir):

    if any([exclude_dir in dirpath for exclude_dir in exclude_dirs]):
        continue

    for fname in fnames:

        extension = os.path.splitext(fname)[1][1:]
        if not (extension and extension in template_contents.keys()):
            continue

        tested_file = os.path.join(dirpath, fname)

        if any([exclude_file in tested_file for exclude_file in exclude_files]):
            continue

        with open(tested_file) as source_file:
            total_files += 1
            for template_line in template_contents[extension]:
                try:
                    line_src = source_file.readline()
                except UnicodeDecodeError as err:
                    print("Unable to decode bytes in '{0}': {1}".format(tested_file, err))
                    total_errors += 1
                    break
                if extension == 'py' and line_src.strip() == '#!/usr/bin/env python':
                    line_src = source_file.readline()
                line_exp = template_line.replace('{{file_name}}', fname)
                if line_src != line_exp:
                    print("Incorrect copyright header in '{0}':".format(tested_file))
                    print("    expected -> '{0}', actual -> '{1}'\n".format(line_exp.strip(), line_src.strip()))
                    total_errors += 1
                    break

print("Files with errors '{0}' out of '{1}'!".format(total_errors, total_files))

if total_errors > 0:
    sys.exit(EXIT_BAD_HEADER)
else:
    sys.exit(EXIT_SUCCESS)
