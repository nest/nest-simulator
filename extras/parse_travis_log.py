# -*- coding: utf-8 -*-
#
# parse_travis_log.py
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
This script parses the TravisCI output as it is generated
by ./build.sh and outputs a shorter summary. It is hard-coded
to the output of ./build.sh, ./bootstrap.sh, make installcheck,
and ../configure --prefix=... . Changing any of those, requires
adapting this script.
"""

INDENT = "  "

def process_until(f, final):
    """
    Run through the file f until a stripped line is equal to final.
    """
    while True:
        line = f.readline()
        if not line:
            return False, line

        if line.strip() == final:
            return True, line



def process_installcheck(f):
    """
    Parse the `make installcheck` output and return the number of tests
    and failed tests. Especially, it parses the output after 'NEST Testsuite Summary'.
    """
    res, line = process_until(f, 'NEST Testsuite Summary')
    if res:
        f.readline()  # -------
        f.readline()  # NEST Executable: ...
        f.readline()  # SLI Executable: ...
        line = f.readline()  # Total number of tests: <int>
        total = int(line.split(' ')[-1])
        f.readline()  # Passed: <int>
        line = f.readline()  # Failed: <int> (<int> PyNEST)
        failed = [int(s) for s in line.split() if s.isdigit()][0]
        return total, failed, line


def process_changed_files(f):
    """
    Run through the file f until a line starts with '+file_names='. After the
    equal sign is a space separated list of changed files in the PR. Returns that
    list.
    """
    while True:
        line = f.readline()
        if not line:
            return False, line

        # +file_names=extras/scan_travis_log.py nestkernel/connector_model.h nestkernel/nestmodule.cpp nestkernel/network.cpp 
        if line.startswith('+file_names='):
            return filter(lambda x: x != '', line.strip().split('=')[1].split(' ')), line


def process_vera(f, filename):
    """
    Process vera++ output for a certain file.
    """
    d = {}
    res = {'vera++': d}
    while True:
        line = f.readline()
        if not line:
            return res

        if line.startswith('+cppcheck '):
            return res

        if line.startswith(filename):
            # "nestkernel/network.cpp:107: full block {} expected in the control structure"
            key = line.split(":")[-1].strip()
            if not d.has_key(key):
                d[key] = 0
            d[key] += 1


def process_cppcheck(f, filename):
    """
    Process cppcheck output for a certain file.
    """
    d = {}
    res = {'cppcheck': d}
    while True:
        line = f.readline()
        if not line:
            return res

        if line.startswith('+clang-format-'):
            return res

        if line.startswith('[' + filename):
            key = line[line.find('('):].strip()
            if 'is never used' in key:
                continue
            if '(information)' in key:
                continue
            if not d.has_key(key):
                d[key] = 0
            d[key] += 1


def process_clang_format(f, filename):
    """
    Process clang-format output for a certain file.
    """
    d = {'Ok?': False}
    res = {'clang-format': d}
    while True:
        line = f.readline()
        if not line:
            return res

        if line.startswith('+cat '):
            line = f.readline()
            diff = ''
            while not line.startswith("+rm"):
                diff += line
                line = f.readline()
            if diff == '':
                d['Ok?'] = True
            else:
                d['diff'] = '\n##############################\n' + diff + '##############################'

            return res


def process_static_analysis(f, line):
    """
    Process static analysis output for a certain file.
    """
    # Static analysis on file nestkernel/archiving_node.cpp:
    filename = line.split(' ')[-1].strip()[0:-1]
    d = {}
    res = {filename: d}
    while True:
        line = f.readline()
        if not line:
            return res

        if line.startswith('+echo Static analysis on file '):
            return res

        if line.startswith('+rm -rf ./cppcheck'):
            return res

        if line.startswith(' - vera++ for '):
            d.update(process_vera(f, filename))

        if line.startswith(' - cppcheck for '):
            d.update(process_cppcheck(f, filename))

        if line.startswith(' - clang-format for '):
            d.update(process_clang_format(f, filename))


def print_static_analysis(d):
    """
    Print a well formatted version of the dict returned by process_static_analysis(f, line).
    """
    for k1, v1 in d.iteritems():
        print(INDENT + ' --' + '-' * len(k1))
        print(INDENT + ' | ' + k1 + ':')
        for k2, v2 in v1.iteritems():
            print(INDENT + ' | ' + INDENT + ' * ' + k2 + ':')
            for k3, v3 in v2.iteritems():
                print(INDENT + ' | ' + 2 * INDENT +
                      ' - ' + k3 + ' : ' + str(v3))

if __name__ == '__main__':
    from sys import argv, exit

    script, filename = argv

    bootstrapping_ok = False
    configure_ok = False
    make_ok = False
    make_install_ok = False
    make_installcheck_all = 0
    make_installcheck_failed = -1
    vera_init = False
    cppcheck_init = False
    changed_files = False
    static_analysis = {}
    uploading_results = True

    with open(filename, 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break

            if not bootstrapping_ok and line.startswith('+./bootstrap.sh'):
                bootstrapping_ok, line = process_until(f, 'Done.')

            if not vera_init and line.startswith('+mkdir -p vera_home'):
                vera_init, line = process_until(f, '+cat')

            if not cppcheck_init and line.startswith('+git clone https://github.com/danmar/cppcheck.git'):
                cppcheck_init, line = process_until(f, 'Cppcheck 1.69')

            if not changed_files and line.strip() == 'Extract changed files...':
                changed_files, line = process_changed_files(f)

            if line.startswith('Static analysis on file '):
                static_analysis.update(process_static_analysis(f, line))

            if not configure_ok and line.startswith('+../configure --prefix='):
                configure_ok, line = process_until(
                    f, 'You can now build and install NEST with')

            if not make_ok and line.strip() == '+make':
                make_ok, line = process_until(f, '+make install')

            if not make_install_ok and line.startswith('+make install'):
                make_install_ok, line = process_until(f, '+make installcheck')

            if make_installcheck_failed == -1 and line.startswith('+make installcheck'):
                make_installcheck_all, make_installcheck_failed, line = process_installcheck(
                    f)
            if line.strip() == 'WARNING: Not uploading results as this is a pull request':
                uploading_results = False

    print("\n--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")
    print("Bootstrapping:       " + ("Ok" if bootstrapping_ok else "Error"))
    print("Vera init:           " + ("Ok" if vera_init else "Error"))
    print("Cppcheck init:       " + ("Ok" if cppcheck_init else "Error"))
    print("Changed files:       " + str(changed_files))
    print("Formatting:          " + ("Ok" if all([ i['clang-format']['Ok?'] for i in static_analysis.itervalues()]) else "Error"))
    print("Configure:           " + ("Ok" if configure_ok else "Error"))
    print("Make:                " + ("Ok" if make_ok else "Error"))
    print("Make install:        " + ("Ok" if make_install_ok else "Error"))
    print("Make installcheck:   " + ("Ok (" if make_installcheck_failed == 0 else "Error (") +
          str(make_installcheck_failed) + " / " + str(make_installcheck_all) + ")")
    print("Logs uploaded to S3: " + ("Yes" if uploading_results else "No"))

    print("\nStatic analysis:" )
    print_static_analysis(static_analysis)
    print("--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")

    if not (bootstrapping_ok and 
            vera_init and 
            cppcheck_init and 
            configure_ok and 
            make_ok and 
            make_install_ok and
            make_installcheck_failed == 0 and
            all([ i['clang-format']['Ok?'] for i in static_analysis.itervalues()])):
        exit(1)
