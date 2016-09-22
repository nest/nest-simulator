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
to the output of ./build.sh, make installcheck,
and cmake -DCMAKE_INSTALL_PREFIX=... . Changing any of those, requires
adapting this script.
"""

INDENT = "  "


def process_from_to(f, start, end):
    """
    Run through the file f.
    If only `start` is encountered, return `False`.
    If `start` and the `end` is encountered, return `True`.
    If none of those is encountered, return `None`.
    """
    result = None
    for line in open(f, "r"):
        if result is None and line.strip() == start:
            result = False
        if result is False and line.strip() == end:
            result = True

    return result


def process_installcheck(f):
    """
    Parse the `make installcheck` output and return the number of tests
    and failed tests. Especially, it parses the output after
    'NEST Testsuite Summary'.
    """
    in_test = False
    in_results = False
    total = None
    failed = None
    for line in open(f, "r"):
        if line.strip() == "======= Test NEST start =======":
            in_test = True
            total = 0
            failed = -1
        elif line.strip() == "======= Test NEST end =======":
            return total, failed
        elif in_test and line.strip() == "NEST Testsuite Summary":
            in_results = True
        elif in_results and "Total number of tests:" in line:
            total = int(line.split(' ')[-1])
        elif in_results and "Failed" in line:
            failed = [int(s) for s in line.split() if s.isdigit()][0]
    return total, failed


def process_changed_files(f):
    """
    Run through the file f until a line starts with 'file_names='.
    The equal sign is followed by a space separated list of changed files
    in the PR. Returns that list.
    """
    in_changed_files_section = False
    for line in open(f, "r"):
        if (not in_changed_files_section and
                line.strip() == "======= Extract changed files start ======="):
            in_changed_files_section = True
        if in_changed_files_section and line.strip().startswith('file_names='):
            return filter(lambda x: x != '',
                          line.strip().split('=')[1].split(' '))
        if line.strip() == "======= Extract changed files end =======":
            return []
    return []


def process_vera(f):
    """
    Process vera++ output for a certain file.
    """
    res = None
    in_vera = False
    for line in open(f, "r"):
        if line.strip() == "======= - vera++ end =======":
            in_vera = False
        elif line.strip().startswith("======= - vera++ for"):
            in_vera = True
            # ======= - vera++ for path/to/python/file.py =======
            filename = line.split(' ')[-2].strip()
            d = {}
            if res is None:
                res = {}
            res.update({filename: d})
        elif in_vera and line.strip().startswith(filename):
            key = line.split(":")[-1].strip()
            if key not in d:
                d[key] = 0
            d[key] += 1
    return res


def process_cppcheck(f):
    """
    Process cppcheck output for a certain file.
    """
    res = None
    in_cppcheck = False
    for line in open(f, "r"):
        if line.strip() == "======= - cppcheck end =======":
            in_cppcheck = False
        elif line.strip().startswith("======= - cppcheck for"):
            in_cppcheck = True
            # ======= - cppcheck path/to/cpp/file.cpp =======
            filename = line.split(' ')[-2].strip()
            d = {}
            if res is None:
                res = {}
            res.update({filename: d})
        elif in_cppcheck and line.strip().startswith('[' + filename):
            key = line[line.find('('):].strip()
            # ignore 'is never used' items
            if 'is never used' in key:
                continue
            # ignore '(information)' items
            if '(information)' in key:
                continue
            if key not in d:
                d[key] = 0   # first seen here
            d[key] += 1
    return res


def process_clang_format(f):
    """
    Process clang-format output for a certain file.
    """
    res = None
    in_clang_format = False
    for line in open(f, "r"):
        if line.strip() == "======= - clang-format end =======":
            in_clang_format = False
            if diff.strip() == "":
                d["Ok?"] = True
            else:
                d['diff'] = ('\n##############################\n' + diff +
                             '##############################')
        elif line.strip().startswith("======= - clang-format for"):
            in_clang_format = True
            # ======= - clang-format for path/to/cpp/file.cpp =======
            filename = line.split(' ')[-2].strip()
            d = {'Ok?': False}
            if res is None:
                res = {}
            res.update({filename: d})
            diff = ""

        elif in_clang_format:
            diff += line
    return res


def process_pep8(f):
    """
    Process PEP8 output.
    """
    res = None
    in_pep8 = False
    for line in open(f, "r"):
        if line.strip() == "======= Check PEP8 end =======":
            in_pep8 = False
        elif line.strip().startswith("======= Check PEP8 for"):
            in_pep8 = True
            # ======= Check PEP8 on file path/to/python/file.py =======
            filename = line.split(' ')[-2].strip()
            d = set()
            if res is None:
                res = {}
            res.update({filename: d})
        elif in_pep8:
            d.add(line.strip())
    return res


def process_s3_upload(f):
    """
    Check if log contains information about upload to s3.
    """
    uploading_results = True
    for line in open(f, "r"):
        if line.strip().startswith('WARNING: Not uploading results as this'):
            uploading_results = False

    return uploading_results


def print_static_analysis(clang_format, cppcheck, vera):
    """
    Print a well formatted version of the dict returned by
    process_static_analysis(f, line).
    """
    assert(clang_format.keys() == cppcheck.keys())
    assert(clang_format.keys() == vera.keys())

    for filename in clang_format.keys():
        print(INDENT + ' --' + '-' * len(filename))
        print(INDENT + ' | ' + filename + ':')
        print(INDENT + ' | ' + INDENT + ' *  vera++:')
        for k, v in vera[filename].iteritems():
            print(INDENT + ' | ' + 2 * INDENT +
                  ' - ' + k + ' : ' + str(v))
        print(INDENT + ' | ' + INDENT + ' * cppcheck:')
        for k, v in cppcheck[filename].iteritems():
            print(INDENT + ' | ' + 2 * INDENT +
                  ' - ' + k + ' : ' + str(v))
        print(INDENT + ' | ' + INDENT + ' * clang_format:')
        for k, v in clang_format[filename].iteritems():
            print(INDENT + ' | ' + 2 * INDENT +
                  ' - ' + k + ' : ' + str(v))


def count_warnings_errors(f):
    """
    Counts compiler warnings and errors. Stops when reading '+make install'.
    """
    warn = None
    error = None
    in_make = False
    for line in open(f, "r"):
        if line.strip() == "======= Make NEST start =======":
            in_make = True
            warn = {}
            error = {}

        if in_make and ': warning:' in line:
            file_name = line.split(':')[0]
            if file_name not in warn:
                warn[file_name] = 0
            warn[file_name] += 1

        if in_make and ': error:' in line:
            file_name = line.split(':')[0]
            if file_name not in error:
                error[file_name] = 0
            error[file_name] += 1

        if line.strip() == "======= Make NEST end =======":
            return warn, error
    return warn, error


def print_pep8(d):
    """
    Print a well formatted version of the dict returned by
    process_pep8(f, line).
    """
    for k1, v1 in d.iteritems():
        print(INDENT + ' --' + '-' * len(k1))
        print(INDENT + ' | ' + k1 + ':')
        for v2 in v1:
            print(INDENT + ' | ' + INDENT + v2)


def print_test_result(r):
    """
    Print result for multiple tests.
    """
    if r is None:
        return "Skipped."
    elif r is True:
        return "Ok."
    elif r is False:
        return "Failed."
    else:
        return str(r)


def print_make(actual_warnings, sum_of_errors, sum_of_warnings):
    """
    Print parse results from building NEST.
    """
    if actual_warnings is None:
        return "Skipped."
    elif sum_of_errors == 0:
        res = "Ok"
    else:
        res = "Error(" + str(sum_of_errors) + ")"
    res += " ( " + str(sum_of_warnings) + " warnings )."
    return res


def print_installcheck(make_check_all, make_check_failed):
    """
    Print parse results from `installcheck`ing NEST.
    """
    if make_check_all is None and make_check_failed is None:
        return "Skipped."
    elif make_check_failed == 0:
        res = "Ok ("
    else:
        res = "Error ("
    res += str(make_check_failed) + " / " + str(make_check_all) + ")."
    return res

if __name__ == '__main__':
    from sys import argv, exit

    script, filename = argv

    vera_init = process_from_to(filename,
                                "======= VERA++ init start =======",
                                "======= VERA++ init end =======")
    cppcheck_init = process_from_to(filename,
                                    "======= CPPCHECK init start =======",
                                    "======= CPPCHECK init end =======")
    clang_format_init = process_from_to(
        filename,
        "======= CLANG-FORMAT init start =======",
        "======= CLANG-FORMAT init end =======")

    changed_files = process_changed_files(filename)
    pep8_analysis = process_pep8(filename)
    vera_analysis = process_vera(filename)
    cppcheck_analysis = process_cppcheck(filename)
    clang_format_analysis = process_clang_format(filename)

    configure_ok = process_from_to(filename,
                                   "======= CLANG-FORMAT init start =======",
                                   "======= CLANG-FORMAT init end =======")
    warnings, errors = count_warnings_errors(filename)
    make_install_ok = process_from_to(filename,
                                      "======= Install NEST start =======",
                                      "======= Install NEST end =======")
    make_check_all, make_check_failed = process_installcheck(filename)

    uploading_results = process_s3_upload(filename)

    # post process values
    if not (warnings is None and errors is None):
        actual_warnings = {k.split("nest-simulator/")[1]: v
                           for k, v in warnings.iteritems()
                           if k.strip().startswith('/home/travis/build')}
        sum_of_warnings = sum([v for k, v in actual_warnings.iteritems()])

        sum_of_errors = sum([v for k, v in errors.iteritems()])
    else:
        actual_warnings, sum_of_errors, sum_of_warnings = None, None, None

    format_ok = (None if clang_format_analysis is None
                 else all([i["Ok?"]
                          for i in clang_format_analysis.itervalues()]))
    pep8_ok = (None if pep8_analysis is None
               else all([len(v) == 0 for v in pep8_analysis.values()]))

    print("\n--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")
    print("Vera init:           " + print_test_result(vera_init))
    print("Cppcheck init:       " + print_test_result(cppcheck_init))
    print("clang-format init:   " + print_test_result(clang_format_init))
    print("Changed files:       " + print_test_result(str(changed_files)))
    print("Formatting:          " + print_test_result(format_ok))
    print("PEP8:                " + print_test_result(pep8_ok))
    print("Configure:           " + print_test_result(configure_ok))
    print("Make:                " + print_make(actual_warnings,
                                               sum_of_errors,
                                               sum_of_warnings))
    print("Make install:        " + print_test_result(make_install_ok))
    print("Make installcheck:   " + print_installcheck(make_check_all,
                                                       make_check_failed))
    print("Logs uploaded to S3: " + ("Yes" if uploading_results else "No"))

    if clang_format_analysis and cppcheck_analysis and vera_analysis:
        print("\nStatic analysis:")
        print_static_analysis(clang_format_analysis,
                              cppcheck_analysis,
                              vera_analysis)

    if actual_warnings:
        print("\nWarnings:")
        for k, v in actual_warnings.iteritems():
            print(" - {}: {}".format(k, v))

    # Print PEP8 analysis only if files have been checked and problems found
    if pep8_analysis and max(len(res) for res in pep8_analysis.values()) > 0:
        print("\nPEP8 analysis: (only first occurrence)")
        print_pep8(pep8_analysis)

    print("\n--------<<<<<<<< Summary of TravisCI >>>>>>>>--------")

    if not ((vera_init is None or vera_init) and
            (cppcheck_init is None or cppcheck_init) and
            (configure_ok is None or configure_ok) and
            (sum_of_errors is None or sum_of_errors == 0) and
            (make_install_ok is None or make_install_ok) and
            (make_check_failed is None or
                make_check_failed == 0) and
            (format_ok is None or format_ok) and
            (pep8_ok is None or pep8_ok)):
        exit(1)
