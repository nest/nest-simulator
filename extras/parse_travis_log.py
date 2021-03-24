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
This Python script is part of the NEST Travis CI build and test environment.
It parses the Travis CI build log file 'travis_build.sh.log' (The name is
hard-wired in '.travis.yml'.) and creates the 'NEST Travis CI Build Summary'.

NOTE: Please note that the parsing process is coupled to shell script
      'travis_build.sh' and relies on the message numbers "MSGBLDnnnn'.
      It does not rely on the messages texts itself except for file names.
"""


def is_message_pair_in_logfile(log_filename, msg_start_of_section,
                               msg_end_of_section):
    """Read the NEST Travis CI build log file and return 'True' in case both
    messages are found in correct order. Return 'False' if only the first
    message was found. Return 'None' in case the first or both messages
    are not contained in the file.

    Parameters
    ----------
    log_filename:         NEST Travis CI build log file name.
    msg_start_of_section: Message number string, e.g. "MSGBLD1234".
    msg_end_of_section:   Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    True, False or None.
    """

    pair_found = None
    with open(log_filename) as fh:
        for line in fh:
            if pair_found is None and is_message(line, msg_start_of_section):
                pair_found = False
            if not pair_found and is_message(line, msg_end_of_section):
                pair_found = True

    return pair_found


def is_message_in_logfile(log_filename, msg_number):
    """Read the NEST Travis CI build log file. Return 'True' if the message is
    contained in the log file.

    Parameters
    ----------
    log_filename: NEST Travis CI build log file name.
    msg_number:   Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    True or False.
    """

    with open(log_filename) as fh:
        for line in fh:
            if is_message(line, msg_number):
                return True

    return False


def is_message(line, msg_number):
    """Return 'True' if 'line' contains the message identified by 'msg_number'.

    Parameters
    ----------
    line:       A single line from the NEST CI build log file.
    msg_number: Message number string.

    Returns
    -------
    True or False
    """

    if msg_number in line:
        return True

    return False


def list_of_changed_files(log_filename, msg_changed_files_section_start,
                          msg_changed_files_section_end, msg_changed_files):
    """Read the NEST Travis CI build log file, find the 'changed files' section
    and return a list of the changed files or an empty list, respectively.

    Parameters
    ----------
    log_filename:                    NEST Travis CI build log file name.
    msg_changed_files_section_start: Message number string, e.g. "MSGBLD1234".
    msg_changed_files_section_end:   Message number string, e.g. "MSGBLD1234".
    msg_changed_files:               Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    List of changed files.
    """

    changed_files = []
    if not is_message_pair_in_logfile(log_filename,
                                      msg_changed_files_section_start,
                                      msg_changed_files_section_end):
        return changed_files

    in_changed_files_section = False
    with open(log_filename) as fh:
        for line in fh:
            if not in_changed_files_section and \
                    is_message(line, msg_changed_files_section_start):
                in_changed_files_section = True
                continue

            if in_changed_files_section:
                if is_message(line, msg_changed_files):
                    changed_files.append(line.split(' ')[-1].strip())
                    continue

                if is_message(line, msg_changed_files_section_end):
                    # The log file contains only one 'changed-files-section'.
                    # Stop reading the log file.
                    return changed_files

    return changed_files


def msg_summary_vera(log_filename, msg_vera_section_start,
                     msg_vera_section_end, msg_vera):
    """Read the NEST Travis CI build log file, find the VERA++ sections,
    extract the VERA messages per file and return a dictionary containing an
    overall summary of the VERA++ code analysis.

    Parameters
    ----------
    log_filename:           NEST Travis CI build log file name.
    msg_vera_section_start: Message number string, e.g. "MSGBLD1234".
    msg_vera_section_end:   Message number string, e.g. "MSGBLD1234".
    msg_vera:               Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    None or a dictionary of dictionaries of VERA++ messages per file.
    """

    vera_msgs = None
    in_a_vera_section = False
    with open(log_filename) as fh:
        for line in fh:
            if not in_a_vera_section and is_message(line,
                                                    msg_vera_section_start):
                in_a_vera_section = True
                source_filename = line.split(' ')[-1].strip()
                single_file_vera_msgs = {}
                if vera_msgs is None:
                    vera_msgs = {}
                vera_msgs.update({source_filename: single_file_vera_msgs})
                continue

            if in_a_vera_section:
                if is_message(line, msg_vera):
                    message = line.split(":")[-1].strip()
                    if message not in single_file_vera_msgs:
                        single_file_vera_msgs[message] = 0
                    single_file_vera_msgs[message] += 1
                    continue

                if is_message(line, msg_vera_section_end):
                    in_a_vera_section = False

    return vera_msgs


def msg_summary_cppcheck(log_filename, msg_cppcheck_section_start,
                         msg_cppcheck_section_end, msg_cppcheck):
    """Read the NEST Travis CI build log file, find the cppcheck sections,
    extract the cppcheck messages per file and return a dictionary containing
    an overall summary of the cppcheck code analysis.

    Parameters
    ---------
    log_filename:               NEST Travis CI build log file name.
    msg_cppcheck_section_start: Message number string, e.g. "MSGBLD1234".
    msg_cppcheck_section_end:   Message number string, e.g. "MSGBLD1234".
    msg_cppcheck:               Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    None or a dictionary of dictionaries of cppcheck messages per file.
    """

    cppcheck_msgs = None
    in_a_cppcheck_section = False
    with open(log_filename) as fh:
        for line in fh:
            if not in_a_cppcheck_section and \
                    is_message(line, msg_cppcheck_section_start):
                in_a_cppcheck_section = True
                source_filename = line.split(' ')[-1].strip()
                single_file_cppcheck_msgs = {}
                if cppcheck_msgs is None:
                    cppcheck_msgs = {}
                cppcheck_msgs.update({source_filename:
                                      single_file_cppcheck_msgs})
                continue

            if in_a_cppcheck_section:
                if is_message(line, msg_cppcheck):
                    if 'Checking' in line:
                        continue
                    message = line[line.find('('):].strip()
                    if 'is never used' in message:
                        continue
                    if '(information)' in message:
                        continue
                    if message not in single_file_cppcheck_msgs:
                        single_file_cppcheck_msgs[message] = 0
                    single_file_cppcheck_msgs[message] += 1
                    continue

                if is_message(line, msg_cppcheck_section_end):
                    in_a_cppcheck_section = False

    return cppcheck_msgs


def msg_summary_format(log_filename, msg_format_section_start,
                       msg_format_section_end, msg_format):
    """Read the NEST Travis CI build log file, find the clang-format sections,
    extract the 'diff-messages' per file and return a dictionary containing an
    overall summary of the clang-format code analysis.

    Parameters
    ----------
    log_filename:             NEST Travis CI build log file name.
    msg_format_section_start: Message number string, e.g. "MSGBLD1234".
    msg_format_section_end:   Message number string, e.g. "MSGBLD1234".
    msg_format:               Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    None or a dictionary of dictionaries of clang-format diff-messages per
    file.
    """

    format_msgs = None
    in_a_format_section = False
    with open(log_filename) as fh:
        for line in fh:
            if not in_a_format_section and is_message(line, msg_format_section_start):  # noqa
                in_a_format_section = True
                source_filename = line.split(' ')[-1].strip()
                single_file_format_msgs = {}
                if format_msgs is None:
                    format_msgs = {}
                format_msgs.update({source_filename: single_file_format_msgs})      # noqa
                continue

            if in_a_format_section:
                if is_message(line, msg_format):
                    diffline = line.split(":")[-1].strip()
                    if diffline not in single_file_format_msgs:
                        single_file_format_msgs[diffline] = 0
                    single_file_format_msgs[diffline] += 1
                    continue

                if is_message(line, msg_format_section_end):
                    in_a_format_section = False

    return format_msgs


def msg_summary_pep8(log_filename, msg_pep8_section_start,
                     msg_pep8_section_end, msg_pep8):
    """Read the NEST Travis CI build log file, find the PEP8 sections, extract
    the PEP8 messages per file and return a dictionary containing an overall
    summary.

    Parameters:
    ----------
    log_filename:           NEST Travis CI build log file name.
    msg_pep8_section_start: Message number string, e.g. "MSGBLD1234".
    msg_pep8_section_end:   Message number string, e.g. "MSGBLD1234".
    msg_pep8:               Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    None or a dictionary of dictionaries of PEP8 messages per file.
    """

    pep8_msgs = None
    in_a_pep8_section = False
    with open(log_filename) as fh:
        for line in fh:
            if not in_a_pep8_section and is_message(line, msg_pep8_section_start):      # noqa
                in_a_pep8_section = True
                source_filename = line.split(' ')[-1].strip()
                single_file_pep8_msgs = {}
                if pep8_msgs is None:
                    pep8_msgs = {}
                pep8_msgs.update({source_filename: single_file_pep8_msgs})
                continue

            if in_a_pep8_section:
                if is_message(line, msg_pep8):
                    message = line.split(":")[-1].strip()
                    if message not in single_file_pep8_msgs:
                        single_file_pep8_msgs[message] = 0
                    single_file_pep8_msgs[message] += 1
                    continue

                if is_message(line, msg_pep8_section_end):
                    in_a_pep8_section = False

    return pep8_msgs


def makebuild_summary(log_filename, msg_make_section_start,
                      msg_make_section_end):
    """Read the NEST Travis CI build log file and return the number of build
    error and warning messages as well as dictionaries summarizing their
    occurrences.

    Parameters
    ----------
    log_filename:           NEST Travis CI build log file name.
    msg_make_section_start: Message number string, e.g. "MSGBLD1234".
    msg_make_section_end:   Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    True or False depending on the number of error messages.
    Number of error messages.
    Dictionary of file names and the number of errors within these files.
    Number of warning messages.
    Dictionary of file names and the number of warnings within these file.
    """

    error_summary = None
    warning_summary = None
    number_of_error_msgs = 0
    number_of_warning_msgs = 0
    in_make_section = False

    expected_warnings = 0

    # If a certain build_type (e.g. 'MINIMAL' or 'FULL') only builds
    # with some warnings, this would be a good point to re-set the
    # expected_warnings variable conditionally for that build_type.

    nest_warning_re = re.compile(f'.* ({build_dir}.*: warning:.*)')
    known_warnings = [
        f'{build_dir}/sli/scanner.cc:642:13: warning: this statement may fall through [-Wimplicit-fallthrough=]',
        f'{build_dir}/sli/scanner.cc:674:19: warning: this statement may fall through [-Wimplicit-fallthrough=]',
        f'{build_dir}/sli/scanner.cc:716:13: warning: this statement may fall through [-Wimplicit-fallthrough=]',
        f'{build_dir}/sli/scanner.cc:744:24: warning: this statement may fall through [-Wimplicit-fallthrough=]',
    ]

    with open(log_filename) as fh:
        for line in fh:
            if is_message(line, msg_make_section_start):
                in_make_section = True
                error_summary = {}
                warning_summary = {}

            if in_make_section:
                if ': error:' in line:
                    file_name = line.split(':')[0]
                    if file_name not in error_summary:
                        error_summary[file_name] = 0
                    error_summary[file_name] += 1
                    number_of_error_msgs += 1

                # Only count warnings originating in NEST source files
                warning_match = nest_warning_re.match(line)
                if warning_match is not None:
                    warning = warning_match.group(1)
                    if warning not in known_warnings:
                        file_name = warning.split(':')[0]
                        if file_name not in warning_summary:
                            warning_summary[file_name] = 0
                        warning_summary[file_name] += 1
                        number_of_warning_msgs += 1

                if is_message(line, msg_make_section_end):
                    # The log file contains only one 'make' section, return.
                    if number_of_error_msgs == 0 and number_of_warning_msgs == expected_warnings:
                        return(True, number_of_error_msgs, error_summary,
                               number_of_warning_msgs, expected_warnings, warning_summary)
                    else:
                        return(False, number_of_error_msgs, error_summary,
                               number_of_warning_msgs, expected_warnings, warning_summary)

    if in_make_section:
        # 'make' was not completed.
        return False, None, None, None, None
    else:
        # There is no 'make' section at all.
        return None, None, None, None, None


def testsuite_results(log_filename, msg_testsuite_section_start,
                      msg_testsuite_end_message):
    """Read the NEST Travis CI build log file, find the 'make-installcheck'
    section which runs the NEST test suite. Extract the total number of tests
    and the number of tests failed. Return True if all tests passed
    successfully and False in case one or more tests failed. Additionally the
    total number of tests performed and the number of tests failed are
    returned.

    Parameters
    ----------
    log_filename:                NEST Travis CI build log file name.
    msg_testsuite_section_start: Message number string, e.g. "MSGBLD1234".
    msg_testsuite_end_message:   Message number string, e.g. "MSGBLD1234".

    Returns
    -------
    True or False.
    Total number of tests.
    Number of tests failed.
    Number of tests skipped.
    List of failed tests.
    Time used to run tests.
    """

    in_installcheck_section = False
    in_results_section = False
    total_number_of_tests = None
    number_of_tests_failed = None
    number_of_tests_skipped = None
    failed_tests = []
    status_tests = None
    with open(log_filename) as fh:
        for line in fh:
            if is_message(line, msg_testsuite_section_start):
                in_installcheck_section = True

            if in_installcheck_section:
                if line.strip() == "NEST Testsuite Results":
                    in_results_section = True

                if in_results_section:
                    if line.startswith("Total"):
                        parts = line.split()
                        total_number_of_tests = int(parts[1])
                        number_of_tests_skipped = int(parts[2])
                        num_failures = int(parts[3])
                        num_errors = int(parts[4])
                        test_time = float(parts[5])
                        number_of_tests_failed = num_failures + num_errors

                    if line.strip().startswith('|'):
                        failed_tests.append(line.strip()[2:])

                if is_message(line, msg_testsuite_end_message):
                    if number_of_tests_failed == 0:
                        status_tests = True
                    else:
                        status_tests = False
                    # The log file contains only one 'make-installcheck'
                    # section. Stop reading the log file.
                    break

    return (status_tests, total_number_of_tests, number_of_tests_failed,
            number_of_tests_skipped, failed_tests, test_time)


def convert_bool_value_to_status_string(value):
    """Convert a boolean value, e.g. the value returned by
    is_message_pair_in_logfile(), into a meaningful string representation.

    Parameters
    ----------
    value:  Boolean value: True, None or False

    Returns
    -------
    String "Passed Successfully", "Skipped" or "Failed" (default).
    """
    if value:
        return "Passed successfully"
    if value is None:
        return "Skipped"

    return "Failed"


def convert_bool_value_to_yes_no_string(value):
    """Convert a boolean value into a 'Yes' or 'No' string representation.

    Parameters
    ----------
    value:  Boolean value.


    Returns
    -------
    String "YES", "NO" (default).
    """

    if value:
        return "Yes"

    return "No"


def convert_summary_to_status_string(summary, ignore):
    """Determine the status of any performed static code analysis and
    return a string representation of that status. By setting the ignore
    flag, "Ignored" instead of "Failed" is returned.

    Parameters
    ----------
    summary: A dictionary containing per file dictionaries of static code
             analysis messages.
    ignore:  Boolean value, True or False

    Returns
    -------
    String "Passed Successfully", "Skipped", "Failed" or "Ignored".
    """

    if summary is None:
        value = None
    else:
        num_msgs = get_num_msgs(summary)
        if num_msgs == 0:
            value = True
        else:
            value = False

    if ignore and value is False:  # 'not value' doesn't work here.
        return "Ignored"           # value can also be None.

    return convert_bool_value_to_status_string(value)


def get_num_msgs(summary):
    """Return the number of messages of any static code analysis.

    Parameters
    ----------
    summary: A dictionary containing per file dictionaries of static code
             analysis messages.

    Returns
    -------
    The total number of messages contained in the dictionary.
    """

    num_msgs = 0
    if summary is not None:
        for file_name in summary.keys():
            num_msgs += \
                get_num_msgs_for_file(file_name, summary)

    return num_msgs


def get_num_msgs_for_file(file_name, summary):
    """Return the number of messages of any static code analysis for a
    particular source file.

    Parameters
    ----------
    file_name: Source file name.
    summary:   A dictionary containing per file dictionaries of static code
               analysis messages.

    Returns
    -------
    The number of messages for the source file contained in the dictionary.
    """

    num_msgs = 0
    if summary is not None:
        for message, occurrences in summary[file_name].items():
            num_msgs += occurrences

    return num_msgs


def code_analysis_per_file_tables(summary_vera, summary_cppcheck,
                                  summary_format, summary_pep8):
    """Create formatted per-file-tables of VERA++, Cppcheck, clang-format and
    PEP8 messages. Concatenate and return them.

    Parameters
    ----------
    summary_vera:     Dictionary of dictionaries of VERA++ messages per file.
    summary_cppcheck: Dictionary of dictionaries of cppcheck messages per file.
    summary_format:   Dictionary of dictionaries of clang-format messages per
                      file.
    summary_pep8:     Dictionary of dictionaries of PEP8 messages per file.

    Returns
    -------
    Formatted tables string.
    """

    all_tables = ''

    # VERA++, cppcheck, clang-format
    if summary_vera is not None and summary_cppcheck is not None and \
       summary_format is not None:

        # Keys, i.e. file names, are identical in these dictionaries.
        # If this assertion raises an exception, please check travis_build.sh
        # which runs the Travis CI build.
        assert (summary_format.keys() == summary_cppcheck.keys())
        assert (summary_format.keys() == summary_vera.keys())

        # Again: Identical keys for clang-format, cppcheck and VERA++.
        for file in summary_format.keys():
            file_table = ''

            num_msgs_vera = get_num_msgs_for_file(file, summary_vera)
            num_msgs_cppcheck = get_num_msgs_for_file(file, summary_cppcheck)
            num_msgs_format = get_num_msgs_for_file(file, summary_format)

            if num_msgs_vera > 0 or \
               num_msgs_cppcheck > 0 or \
               num_msgs_format > 0:

                file_table = [['+ + + ' + file + ' + + +', '']]

                if num_msgs_vera > 0:
                    file_table.append(['VERA++ (MSGBLD0135):', 'Count'])
                    for message, count in summary_vera[file].items():
                        file_table.append([str(message), str(count)])

                if num_msgs_cppcheck > 0:
                    file_table.append(['Cppcheck (MSGBLD0155):', 'Count'])
                    for message, count in summary_cppcheck[file].items():
                        file_table.append([str(message), str(count)])

                if num_msgs_format > 0:
                    file_table.append(['clang-format (MSGBLD0175):', 'Count'])
                    for message, count in summary_format[file].items():
                        file_table.append([str(message), str(count)])

                table = AsciiTable(file_table)
                table.inner_row_border = True
                file_table = table.table + '\n'

            all_tables += file_table

    # PEP8
    if summary_pep8 is not None:
        for file in summary_pep8.keys():
            file_table = ''

            if get_num_msgs_for_file(file, summary_pep8) > 0:

                file_table = [['+ + + ' + file + ' + + +', '']]

                file_table.append(['PEP8 (MSGBLD0195):', 'Count'])
                for message, count in summary_pep8[file].items():
                    file_table.append([str(message), str(count)])

                table = AsciiTable(file_table)
                table.inner_row_border = True
                file_table = table.table + '\n'

            all_tables += file_table

    return all_tables


def warnings_table(summary):
    """Create a formatted table of source file names and the number of build
    warnings reported for that file.

    Parameters
    ----------
    summary: Dictionary of source file names and number of build warnings.

    Returns
    -------
    Formatted table string.
    """

    file_table = [['Warnings in file:', 'Count']]

    for file in summary.keys():
        file_table.append([file, summary[file]])

    table = AsciiTable(file_table)
    table.inner_row_border = True

    return table.table + '\n'


def errors_table(summary):
    """Create a formatted table of source file names and the number of build
    errors reported for that file.

    Parameters
    ----------
    summary: Dictionary of source file names and number of build errors.

    Returns
    -------
    Formatted table string.
    """

    file_table = [['Errors in file:', 'Count']]

    for file in summary.keys():
        file_table.append([file, summary[file]])

    table = AsciiTable(file_table)
    table.inner_row_border = True

    return table.table + '\n'


def report_failed_tests(failed_tests):
    """Create a table of tests that have failed.

    Parameters
    ----------
    List of failed tests.

    Returns
    -------
    Formatted string, empty if no tests failed.
    """

    if failed_tests:
        return ('\n\nFailed tests:' +
                ''.join('\n  ' + ft for ft in failed_tests))
    else:
        return ''


def printable_summary(list_of_changed_files,
                      status_cmake_configure,
                      status_make,
                      status_make_install,
                      status_amazon_s3_upload,
                      status_tests,
                      summary_vera,
                      summary_cppcheck,
                      summary_format,
                      summary_pep8,
                      summary_errors,
                      summary_warnings,
                      number_of_errors,
                      number_of_warnings,
                      expected_warnings,
                      number_of_tests_total,
                      number_of_tests_failed,
                      number_of_tests_skipped,
                      failed_tests,
                      test_time,
                      ignore_vera,
                      ignore_cppcheck,
                      ignore_format,
                      ignore_pep8,
                      exit_code):
    """Create an overall build summary in a printable format.

    Parameters
    ----------
    list_of_changed_files:   List of changed source files.
    status_cmake_configure:  Status of the 'CMake configure': True, False or
                             None
    status_make:             Status of the 'make': True, False or None
    status_make_install:     Status of the 'make install': True, False or None
    status_amazon_s3_upload: Status of the Amazon S3 upload: True, False
    status_tests:            Status of the test suite run: True, False or None
    summary_vera:            Dictionary of dictionaries of VERA++ messages per
                             file.
    summary_cppcheck:        Dictionary of dictionaries of cppcheck messages
                             per file.
    summary_format:          Dictionary of dictionaries of clang-format
                             messages per file.
    summary_pep8:            Dictionary of dictionaries of PEP8 messages per
                             file.
    summary_errors:          Dictionary of build error messages.
    summary_warnings:        Dictionary of build warning messages.
    number_of_errors:        Number of errors.
    number_of_warnings:      Number of warnings.
    expected_warnings:       Number of warnings expected.
    number_of_tests_total:   Number of tests total.
    number_of_tests_failed:  Number of tests failed.
    number_of_tests_skipped: Number of tests skipped.
    failed_tests:            List of failed tests.
    test_time:               Time required to run testsuite.
    exit_code:               Build exit code: 0 or 1.

    Returns
    -------
    Formatted build summary string.
    """

    header = """
    + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
    +                                                                         +
    +        N E S T   T r a v i s   C I   B u i l d   S u m m a r y          +
    +                                                                         +
    + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
    \n\n"""

    build_summary = header

    if get_num_msgs(summary_vera) > 0 or \
       get_num_msgs(summary_cppcheck) > 0 or \
       get_num_msgs(summary_format) > 0 or \
       get_num_msgs(summary_pep8) > 0:

        build_summary += '  S T A T I C   C O D E   A N A L Y S I S\n'

        # Create formatted per-file-tables of VERA++, Cppcheck, clang-format
        # and PEP8 messages.
        build_summary += code_analysis_per_file_tables(summary_vera,
                                                       summary_cppcheck,
                                                       summary_format,
                                                       summary_pep8)

    if number_of_warnings > 0:
        build_summary += '\n  W A R N I N G S\n'
        build_summary += warnings_table(summary_warnings)

    if number_of_errors > 0:
        build_summary += '\n  E R R O R S\n'
        build_summary += errors_table(summary_errors)

    build_summary += '\n\n  B U I L D   R E P O R T\n'

    summary_table = [
        ['Changed Files :', ''],
        ['', 'No files have been changed.'],
        ['Static Code Analysis :', ''],
        ['VERA++',
         convert_summary_to_status_string(summary_vera, ignore_vera) +
         '\n' + '\nNumber of messages (MSGBLD0135): ' +
         str(get_num_msgs(summary_vera))],
        ['Cppcheck',
         convert_summary_to_status_string(summary_cppcheck, ignore_cppcheck) +
         '\n' + '\nNumber of messages (MSGBLD0155): ' +
         str(get_num_msgs(summary_cppcheck))],
        ['clang-format',
         convert_summary_to_status_string(summary_format, ignore_format) +
         '\n' + '\nNumber of messages (MSGBLD0175): ' +
         str(get_num_msgs(summary_format))],
        ['PEP8',
         convert_summary_to_status_string(summary_pep8, ignore_pep8) +
         '\n' + '\nNumber of messages (MSGBLD0195): ' +
         str(get_num_msgs(summary_pep8))],
        ['NEST Build :', ''],
        ['CMake configure',
         convert_bool_value_to_status_string(status_cmake_configure)],
        ['Make', convert_bool_value_to_status_string(status_make) + '\n' +
         '\nErrors  : ' + str(number_of_errors) +
         '\nWarnings: ' + str(number_of_warnings) + f' ({expected_warnings} expected)'],
        ['Make install',
         convert_bool_value_to_status_string(status_make_install)],
        ['Make installcheck',
         convert_bool_value_to_status_string(status_tests) + '\n' +
         '\nTestsuite runtime      : {:d}s'.format(int(test_time)) +
         '\nTotal number of tests  : ' + str(number_of_tests_total) +
         '\nNumber of tests skipped: ' + str(number_of_tests_skipped) +
         '\nNumber of tests failed : ' + str(number_of_tests_failed) +
         report_failed_tests(failed_tests)
         ],
        ['Artifacts :', ''],
        ['Amazon S3 upload',
         convert_bool_value_to_yes_no_string(status_amazon_s3_upload)]
    ]
    table = AsciiTable(summary_table)
    table.inner_row_border = True
    max_width = table.column_max_width(1)

    # Bypass Travis issue:  ValueError: invalid width -29 (must be > 0)
    #                       (in the wrap() below max_width must be > 0)
    # The calculation of column_max_width is based on the returned terminal
    # width which sometimes seems to be zero resulting in a negative value.
    if max_width < 0:
        max_width = 70

    table.table_data[1][1] = '\n'.join(wrap(', '.join(list_of_changed_files),
                                            max_width))

    build_summary += table.table + '\n'

    if exit_code == 0:
        build_summary += '\nBUILD TERMINATED SUCCESSFULLY'
    else:
        build_summary += '\nBUILD FAILED'

    return build_summary


def build_return_code(status_cmake_configure,
                      status_make,
                      status_make_install,
                      status_tests,
                      summary_vera,
                      summary_cppcheck,
                      summary_format,
                      summary_pep8,
                      ignore_vera,
                      ignore_cppcheck,
                      ignore_format,
                      ignore_pep8,
                      skip_code_analysis,
                      skip_installcheck):
    """Depending in the build results, create a return code.

    Parameters
    ----------
    status_cmake_configure: Status of the 'CMake configure': True, False
                            or None
    status_make:            Status of the 'make': True, False or None
    status_make_install:    Status of the 'make install': True, False or None
    status_tests:           Status of the test suite run: True, False or None
    summary_vera:           Dictionary of dictionaries of VERA++ messages per
                            file.
    summary_cppcheck:       Dictionary of dictionaries of cppcheck messages per
                            file.
    summary_format:         Dictionary of dictionaries of clang-format messages
                            per file.
    summary_pep8:           Dictionary of dictionaries of PEP8 messages per
                            file.
    ignore_vera:            VERA++ messages will not cause the build to
                            fail: True, False
    ignore_cppcheck:        CPPCHECK messages will not cause the build to
                            fail: True, False
    ignore_format:          CLANG-FORMAT messages will not cause the build to
                            fail: True, False
    ignore_pep8:            PEP8 messages will not cause the build to
                            fail: True, False
    skip_code_analysis:     build ran w/o static code analysis: True, False
    skip_installcheck:      build ran w/o executing the test suite: True, False

    Returns
    -------
    0 (success) or 1.
    """
    if ((status_cmake_configure) and                                                           # noqa
        (status_make) and                                                                      # noqa
        (status_make_install) and                                                              # noqa
        (skip_installcheck or status_tests) and                                                # noqa
        (skip_code_analysis or ((ignore_vera or get_num_msgs(summary_vera) == 0) and           # noqa
                                (ignore_cppcheck or get_num_msgs(summary_cppcheck) == 0) and   # noqa
                                (ignore_format or get_num_msgs(summary_format) == 0) and       # noqa
                                (ignore_pep8 or get_num_msgs(summary_pep8) == 0))              # noqa
        )                                                                                      # noqa
       ):                                                                                      # noqa

        return 0
    else:
        return 1


if __name__ == '__main__':
    import re
    from sys import argv, exit
    from terminaltables import AsciiTable
    from textwrap import wrap

    this_script_filename, log_filename, build_type, build_dir = argv

    changed_files = \
        list_of_changed_files(log_filename, "MSGBLD0070",
                              "MSGBLD0100", "MSGBLD0095")

    skip_code_analysis = is_message_in_logfile(log_filename, "MSGBLD0225")
    skip_installcheck = is_message_in_logfile(log_filename, "MSGBLD0305")

    # The NEST Travis CI build consists of several steps and sections.
    # Each section is enclosed in a start- and an end-message.
    # By checking these message-pairs it can be verified whether a section
    # passed through successfully, failed or was skipped.
    status_cmake_configure = \
        is_message_pair_in_logfile(log_filename, "MSGBLD0230", "MSGBLD0240")

    status_make_install = \
        is_message_pair_in_logfile(log_filename, "MSGBLD0270", "MSGBLD0280")

    ignore_vera = is_message_in_logfile(log_filename, "MSGBLD1010")
    ignore_cppcheck = is_message_in_logfile(log_filename, "MSGBLD1020")
    ignore_format = is_message_in_logfile(log_filename, "MSGBLD1030")
    ignore_pep8 = is_message_in_logfile(log_filename, "MSGBLD1040")

    # Summarize the per file results from the static code analysis.
    summary_vera = msg_summary_vera(log_filename, "MSGBLD0130",
                                    "MSGBLD0140", "MSGBLD0135")

    summary_cppcheck = msg_summary_cppcheck(log_filename, "MSGBLD0150",
                                            "MSGBLD0160", "MSGBLD0155")

    summary_format = msg_summary_format(log_filename, "MSGBLD0170",
                                        "MSGBLD0180", "MSGBLD0175")

    summary_pep8 = msg_summary_pep8(log_filename, "MSGBLD0190",
                                    "MSGBLD0200", "MSGBLD0195")

    # Summarize the per file build error messages and warnings.
    status_make, number_of_errors, summary_errors, number_of_warnings, expected_warnings, \
        summary_warnings = makebuild_summary(log_filename, "MSGBLD0250",
                                             "MSGBLD0260")

    # Summarize the NEST test suite results.
    (status_tests, number_of_tests_total, number_of_tests_failed,
     number_of_tests_skipped, failed_tests, test_time) = \
        testsuite_results(log_filename, "MSGBLD0290", "MSGBLD0300")

    # Determine the build result to tell Travis CI whether the build was
    # successful or not.
    exit_code = build_return_code(status_cmake_configure,
                                  status_make,
                                  status_make_install,
                                  status_tests,
                                  summary_vera,
                                  summary_cppcheck,
                                  summary_format,
                                  summary_pep8,
                                  ignore_vera,
                                  ignore_cppcheck,
                                  ignore_format,
                                  ignore_pep8,
                                  skip_code_analysis,
                                  skip_installcheck)

    # Only after a successful build, Travis CI will upload the build artifacts
    # to Amazon S3.
    status_amazon_s3_upload = \
        (not is_message_in_logfile(log_filename, "MSGBLD0330") and
         is_message_in_logfile(log_filename, "MSGBLD0340") and
         exit_code == 0)

    print(printable_summary(changed_files,
                            status_cmake_configure,
                            status_make,
                            status_make_install,
                            status_amazon_s3_upload,
                            status_tests,
                            summary_vera,
                            summary_cppcheck,
                            summary_format,
                            summary_pep8,
                            summary_errors,
                            summary_warnings,
                            number_of_errors,
                            number_of_warnings,
                            expected_warnings,
                            number_of_tests_total,
                            number_of_tests_failed,
                            number_of_tests_skipped,
                            failed_tests,
                            test_time,
                            ignore_vera,
                            ignore_cppcheck,
                            ignore_format,
                            ignore_pep8,
                            exit_code))

    exit(exit_code)
