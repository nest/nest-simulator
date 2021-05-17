#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# find_imports.py
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
import os
import sys
import re
from collections import defaultdict

"""Script to report all Python modules imported in NEST

This script parses the NEST code base to find all external
modules imported by NEST, i.e., modules not included in the
Python Standard Library or as part of NEST itself.

For scripts not included by core parts of NEST (PyNEST
itself, its examples and tests), the script also reports
which part of the NEST codebase requires the module.

The script scans the directory given by the NEST_SOURCE
environment variable if defined and the current working
directory otherwise.

This may be useful to define requirements.

This is still work in progress and may return false positives.
"""

if len(sys.argv) == 1:
    if 'NEST_SOURCE' in os.environ:
        source_dir = os.environ['NEST_SOURCE']
    else:
        print('I do not know which directory to parse: ')
        print('No directory given on the command line and NEST_SOURCE undefined.')
        sys.exit(1)
elif len(sys.argv) == 2:
    source_dir = sys.argv[1]
else:
    print(f'\nUsage: {sys.argv[0]} [path to scan]\n')
    sys.exit(2)

# subdirectories to ignore
exclude_dirs = [
    '.git',
    '.github',
    '.settings',
    'cmake',
    'debian',
    'doc',
]

# modules included in Python Standard Library (https://docs.python.org/3/py-modindex.html)
py_modules = set(['__future__', '__main__', '_thread', 'abc', 'aifc', 'argparse', 'array', 'ast',
                  'asynchat', 'asyncio', 'asyncore', 'atexit', 'audioop', 'base64', 'bdb', 'binascii',
                  'binhex', 'bisect', 'builtins', 'bz2', 'cProfile', 'calendar', 'cgi', 'cgitb', 'chunk',
                  'cmath', 'cmd', 'code', 'codecs', 'codeop', 'collections', 'colorsys', 'compileall',
                  'concurrent', 'configparser', 'contextlib', 'contextvars', 'copy', 'copyreg', 'crypt',
                  'csv', 'ctypes', 'curses', 'dataclasses', 'datetime', 'dbm', 'decimal', 'difflib',
                  'dis', 'distutils', 'doctest', 'email', 'encodings', 'ensurepip', 'enum', 'errno',
                  'faulthandler', 'fcntl', 'filecmp', 'fileinput', 'fnmatch', 'formatter', 'fractions',
                  'ftplib', 'functools', 'gc', 'getopt', 'getpass', 'gettext', 'glob', 'graphlib', 'grp',
                  'gzip', 'hashlib', 'heapq', 'hmac', 'html', 'http', 'imaplib', 'imghdr', 'imp',
                  'importlib', 'inspect', 'io', 'ipaddress', 'itertools', 'json', 'keyword', 'lib2to3',
                  'linecache', 'locale', 'logging', 'lzma', 'mailbox', 'mailcap', 'marshal', 'math',
                  'mimetypes', 'mmap', 'modulefinder', 'msilib', 'msvcrt', 'multiprocessing', 'netrc',
                  'nis', 'nntplib', 'numbers', 'operator', 'optparse', 'os', 'ossaudiodev', 'parser',
                  'pathlib', 'pdb', 'pickle', 'pickletools', 'pipes', 'pkgutil', 'platform', 'plistlib',
                  'poplib', 'posix', 'pprint', 'profile', 'pstats', 'pty', 'pwd', 'py_compile', 'pyclbr',
                  'pydoc', 'queue', 'quopri', 'random', 're', 'readline', 'reprlib', 'resource',
                  'rlcompleter', 'runpy', 'sched', 'secrets', 'select', 'selectors', 'shelve', 'shlex',
                  'shutil', 'signal', 'site', 'smtpd', 'smtplib', 'sndhdr', 'socket', 'socketserver',
                  'spwd', 'sqlite3', 'ssl', 'stat', 'statistics', 'string', 'stringprep', 'struct',
                  'subprocess', 'sunau', 'symbol', 'symtable', 'sys', 'sysconfig', 'syslog', 'tabnanny',
                  'tarfile', 'telnetlib', 'tempfile', 'termios', 'test', 'textwrap', 'threading', 'time',
                  'timeit', 'tkinter', 'token', 'tokenize', 'trace', 'traceback', 'tracemalloc', 'tty',
                  'turtle', 'turtledemo', 'types', 'typing', 'unicodedata', 'unittest', 'urllib', 'uu',
                  'uuid', 'venv', 'warnings', 'wave', 'weakref', 'webbrowser', 'winreg', 'winsound',
                  'wsgiref', 'xdrlib', 'xml', 'xmlrpc', 'zipapp', 'zipfile', 'zipimport', 'zlib',
                  'zoneinfo'])

# find modules that are defined inside the NEST code base
pn_modules = set(os.path.splitext(os.path.split(fn)[-1])[0]
                 for fn in glob.glob(f'{source_dir}/**/*.py', recursive=True))

# exclude modules from the Python Standard Library and modules defined within NEST
exclude_modules = py_modules.union(pn_modules)

# packages forming the base part of NEST
base_dirs = {f'{source_dir}/pynest/nest', f'{source_dir}/pynest/examples',
             f'{source_dir}/pynest/tests', f'{source_dir}/testsuite'}

# server package
server_dir = f'{source_dir}/pynest/nest/server'

# Match for import lines
#   - Picks up only top-level package and ignores relative imports
#   - Will only pick up the first module of comma-separated imports (import modA, modB).
#     As such imports violate PEP8 and do currently not occur in NEST, this is acceptable.
#   - We need \s* at the start of the regex because some imports are nested in try-blocks.
#     This can lead to false positives if a comment line or multiline string line begins with
#     "import" or "from".
import_re = re.compile('\s*(import|from)\s+(\w+)')

imports = defaultdict(set)
for dirpath, _, fnames in os.walk(source_dir):

    if any([exclude_dir in dirpath for exclude_dir in exclude_dirs]):
        continue

    for fname in fnames:
        if not fname.endswith('.py'):
            continue

        ffname = os.path.join(dirpath, fname)
        with open(ffname, encoding='utf-8') as pyfile:
            for line in pyfile:
                match = import_re.match(line)
                if match:
                    modname = match.group(2)
                    if modname not in exclude_modules:
                        if (any(dirpath.startswith(bd) for bd in base_dirs) and not dirpath.startswith(server_dir)):
                            imports[modname].add('BASE')
                        else:
                            imports[modname].add(dirpath)

print('Modules required by NEST base (may include false positives)')
for module, requester in sorted(imports.items()):
    if 'BASE' in requester:
        print(f'    {module}')

print('\n\nModules required outside NEST base (may include false positives)')
for module, requester in sorted(imports.items()):
    if 'BASE' not in requester:
        print(f'    {module}')
        for r in requester:
            print(f'        {r}')
