#!/bin/bash

# do_tests.sh.in
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


# This script runs the NEST testsuite.
#
# The test suite consists of SLI and Python scripts that use the
# language's native `unittest` library to assert certain invariants
# and thus ensure a correctly working installation of NEST.
#


#
# usage <exit_code> [argument]
#
# usage 0
#   Use this variant in case you want print plain usage information
# usage 1 <option>
#   Use this variant in case an unknown option <option> was encountered
# usage 2 <option>
#   Use this variant in case of a missing required option <option>
#
usage ()
{
    if test $1 = 1; then
        echo "Error: Unknown option \'$2\'"
    fi

    if test $1 = 2; then
        echo "Error: Missing required option \'$2\'"
    fi

    cat <<EOF
Usage: $0 --prefix=<path> --report-dir=<path> [options]

Required arguments:
    --prefix=<path>        The base installation path of NEST
    --report-dir=<path>    The directory to store the output to

Options:
    --with-python=<exe>    The Python executable to use
    --with-music=<exe>     The MUSIC executable to use
    --help                 Print program options and exit
EOF

    exit $1
}

while test $# -gt 0 ; do
    case "$1" in
        --help)
            usage 0
            ;;
        --prefix=*)
            PREFIX="$( echo "$1" | sed 's/^--prefix=//' )"
            ;;
        --report-dir=*)
            REPORTDIR="$( echo "$1" | sed 's/^--report-dir=//' )"
            ;;
        --with-python=*)
            PYTHON="$( echo "$1" | sed 's/^--with-python=//' )"
            ;;
        --with-music=*)
            MUSIC="$( echo "$1" | sed 's/^--with-music=//' )"
            ;;
        *)
            usage 1 "$1"
            ;;
    esac
    shift
done

if test ! "${PREFIX}"; then
    usage 2 "--prefix";
fi

if test ! "${REPORTDIR}"; then
    usage 2 "--report-dir";
fi

if test "${PYTHON}"; then
    command -v nosetests >/dev/null 2>&1 || {
        echo "Error: PyNEST testing requested, but command 'nosetests' cannot be executed."
        exit 1
    }
    NOSE="$(command -v nosetests)"
fi

python3 -c "import junitparser" >/dev/null 2>&1
if test $? != 0; then
    echo "Error: Required Python package 'junitparser' not found."
    exit 1
fi

# source helpers to set environment variables and make functions available
. "${PREFIX}/bin/nest_vars.sh"
. "$(dirname $0)/junit_xml.sh"
. "$(dirname $0)/run_test.sh"

if test -d "${REPORTDIR}"; then
    rm -rf "${REPORTDIR}"
fi
mkdir "${REPORTDIR}"

TEST_BASEDIR="${PREFIX}/share/doc/nest"
TEST_LOGFILE="${REPORTDIR}/installcheck.log"
TEST_OUTFILE="${REPORTDIR}/output.log"
TEST_RETFILE="${REPORTDIR}/output.ret"
TEST_RUNFILE="${REPORTDIR}/runtest.sh"

NEST="nest_serial"

# Under Mac OS X, suppress crash reporter dialogs. Restore old state at end.
if test "$(uname -s)" = "Darwin"; then
    TEST_CRSTATE="$( defaults read com.apple.CrashReporter DialogType )"
    defaults write com.apple.CrashReporter DialogType server
fi

print_paths () {
    indent="`printf '%23s'`"
    echo "$1" | sed "s/:/\n$indent/g" | sed '/^\s*$/d'
}

echo "================================================================================"
echo
echo "  NEST testsuite"
echo "  Date: $(date -u)"
echo "  Sysinfo: $(uname -s -r -m)"
echo
NEST_VERSION="$(sli -c "statusdict/version :: =only")"
echo "  NEST executable .... $NEST (version $NEST_VERSION)"
echo "  PREFIX ............. $PREFIX"
if test "${PYTHON}"; then
    PYTHON_VERSION="$("${PYTHON}" --version | cut -d' ' -f2)"
    echo "  Python executable .. $PYTHON (version $PYTHON_VERSION)"
    NOSE_VERSION="$("${NOSE}" --version | cut -d' ' -f3)"
    echo "  Nose executable .... $NOSE (version $NOSE_VERSION)"
    echo "  PYTHONPATH ......... `print_paths ${PYTHONPATH:-}`"
fi
if test "${MUSIC}"; then
    MUSIC_VERSION="$("${MUSIC}" --version | head -n1 | cut -d' ' -f2)"
    echo "  MUSIC executable ... $MUSIC (version $MUSIC_VERSION)"
fi
echo "  TEST_BASEDIR ....... $TEST_BASEDIR"
echo "  REPORTDIR .......... $REPORTDIR"
echo "  PATH ............... `print_paths ${PATH}`"
echo
echo "================================================================================"

HEADLINE="$(nest -v) testsuite log"
echo >  "${TEST_LOGFILE}" "$HEADLINE"
echo >> "${TEST_LOGFILE}" "$(printf '%0.s=' $(seq 1 ${#HEADLINE}))"
echo >> "${TEST_LOGFILE}" "Running tests from ${TEST_BASEDIR}"

CODES_SKIPPED=\
' 200 Skipped,'\
' 201 Skipped (MPI required),'\
' 202 Skipped (build with-mpi=OFF required),'\
' 203 Skipped (Threading required),'\
' 204 Skipped (GSL required),'\
' 205 Skipped (MUSIC required),'\
' 206 Skipped (Recording backend Arbor required),'

echo
echo 'Phase 1: Testing if SLI can execute scripts and report errors'
echo '-------------------------------------------------------------'

junit_open '01_basetests'

CODES_SUCCESS=' 0 Success'
CODES_FAILURE=
for test_name in test_pass.sli test_goodhandler.sli test_lazyhandler.sli ; do
    run_test "selftests/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
done

CODES_SUCCESS=' 126 Success'
CODES_FAILURE=
for test_name in test_fail.sli test_stop.sli test_badhandler.sli ; do
    run_test "selftests/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
done

junit_close

# At this point, we are sure that
#
#  * NEST will return 0 after finishing a script
#  * NEST will return 126 when a script raises an unhandled error
#  * Error handling in stopped contexts works

echo
echo "Phase 2: Testing SLI's unittest library"
echo "---------------------------------------"

junit_open '02_selftests'

# assert_or_die uses pass_or_die, so pass_or_die should be tested first.

CODES_SUCCESS=' 2 Success'
CODES_FAILURE=' 126 Failed: error in test script'

run_test selftests/test_pass_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

CODES_SUCCESS=' 1 Success'
CODES_FAILURE=\
' 2 Failed: error in tested code block,'\
' 126 Failed: error in test script,'

run_test selftests/test_assert_or_die_b.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
run_test selftests/test_assert_or_die_p.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

CODES_SUCCESS=' 3 Success'
CODES_FAILURE=\
' 1 Failed: missed assertion,'\
' 2 Failed: error in tested code block,'\
' 126 Failed: error in test script,'

run_test selftests/test_fail_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

CODES_SUCCESS=' 3 Success'
CODES_FAILURE=\
' 1 Failed: missed assertion,'\
' 2 Failed: error in tested code block,'\
' 126 Failed: error in test script,'

run_test selftests/test_crash_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

CODES_SUCCESS=' 3 Success'
CODES_FAILURE=\
' 1 Failed: missed assertion,'\
' 2 Failed: error in tested code block,'\
' 126 Failed: error in test script,'

run_test selftests/test_failbutnocrash_or_die_crash.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
run_test selftests/test_failbutnocrash_or_die_pass.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

CODES_SUCCESS=' 3 Success'
CODES_FAILURE=\
' 1 Failed: missed assertion,'\
' 2 Failed: error in tested code block,'\
' 126 Failed: error in test script,'

run_test selftests/test_passorfailbutnocrash_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

junit_close

# At this point, we are sure that
#
#  * unittest::pass_or_die works
#  * unittest::assert_or_die works
#  * unittest::fail_or_die works
#  * unittest::crash_or_die works

# These are the default exit codes and their explanations
CODES_SUCCESS=' 0 Success'
CODES_FAILURE=\
' 1 Failed: missed SLI assertion,'\
' 2 Failed: error in tested code block,'\
' 3 Failed: tested code block failed to fail,'\
' 4 Failed: re-run serial,'\
' 10 Failed: unknown error,'\
' 20 Failed: inconsistent copyright header(s),'\
' 30 Failed: inconsistent Name definition(s)/declaration(s),'\
' 31 Failed: unused Name definition(s),'\
' 125 Failed: unknown C++ exception,'\
' 126 Failed: error in test script,'\
' 127 Failed: fatal error,'\
' 134 Failed: missed C++ assertion,'\
' 139 Failed: segmentation fault,'

echo
echo "Phase 3: Running NEST unit tests"
echo "--------------------------------"

junit_open '03_unittests'

for test_ext in sli py ; do
      for test_name in $(ls "${TEST_BASEDIR}/unittests/" | grep ".*\.${test_ext}\$") ; do
          run_test "unittests/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
      done
done

junit_close

echo
echo "Phase 4: Running regression tests"
echo "---------------------------------"

junit_open '04_regressiontests'

for test_ext in sli py ; do
    for test_name in $(ls "${TEST_BASEDIR}/regressiontests/" | grep ".*\.${test_ext}$") ; do
        run_test "regressiontests/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
    done
done

junit_close

echo
echo "Phase 5: Running MPI tests"
echo "--------------------------"
if test "x$(sli -c 'statusdict/have_mpi :: =')" = xtrue; then
    junit_open '05_mpitests'

    NEST="nest_indirect"
    for test_name in $(ls "${TEST_BASEDIR}/mpi_selftests/pass" | grep '.*\.sli$'); do
        run_test "mpi_selftests/pass/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
    done

    # tests meant to fail
    SAVE_CODES_SUCCESS=${CODES_SUCCESS}
    SAVE_CODES_FAILURE=${CODES_FAILURE}
    CODES_SUCCESS=' 1 Success (expected failure)'
    CODES_FAILURE=' 0 Failed: Unittest failed to detect error.'
    for test_name in $(ls "${TEST_BASEDIR}/mpi_selftests/fail" | grep '.*\.sli$'); do
        run_test "mpi_selftests/fail/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
    done
    CODES_SUCCESS=${SAVE_CODES_SUCCESS}
    CODES_FAILURE=${SAVE_CODES_FAILURE}

    for test_name in $(ls "${TEST_BASEDIR}/mpitests/" | grep '.*\.sli$'); do
        run_test "mpitests/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
    done

    junit_close
else
  echo "  Not running MPI tests because NEST was compiled without support"
  echo "  for distributed computing. See the file README.md for details."
fi

echo
echo "Phase 6: Running MUSIC tests"
echo "----------------------------"
if test "${MUSIC}"; then
    junit_open '06_musictests'

    # Create a temporary directory with a unique name.
    BASEDIR="$PWD"
    tmpdir="$(mktemp -d)"

    TESTDIR="${TEST_BASEDIR}/musictests/"

    for test_name in $(ls ${TESTDIR} | grep '.*\.music$') ; do
        music_file="${TESTDIR}/${test_name}"

        # Collect the list of SLI files from the '.music' file.
        sli_files=$(grep '\.sli' ${music_file} | sed -e "s#args=#${TESTDIR}#g")
        sli_files=$(for f in ${sli_files}; do if test -f ${f}; then echo ${f}; fi; done)
        sli_files=${sli_files//$'\n'/ }

        # Check if there is an accompanying shell script for the test.
        sh_file="${TESTDIR}/$(basename ${music_file} .music).sh"
        if test ! -f "${sh_file}"; then unset sh_file; fi

        # Calculate the total number of processes from the '.music' file.
        np=$(($(sed -n 's/np=//p' ${music_file} | paste -sd'+' -)))
        command="$(sli -c "${np} (${MUSIC}) (${test_name}) mpirun =")"

        proc_txt="processes"
        if test $np -eq 1; then proc_txt="process"; fi
        echo          "Running test '${test_name}' with $np $proc_txt... " >> "${TEST_LOGFILE}"
        printf '%s' "  Running test '${test_name}' with $np $proc_txt... "

        # Copy everything to 'tmpdir'.
        # Variables might also be empty. To prevent 'cp' from terminating in such a case,
        # the exit code is suppressed.
        cp ${music_file} ${sh_file} ${sli_files} ${tmpdir} 2>/dev/null || :

        # Create the runner script in 'tmpdir'.
        cd "${tmpdir}"
        echo "#!/bin/sh" >  runner.sh
        echo "set +e" >> runner.sh
        echo "NEST_DATA_PATH=\"${tmpdir}\"" >> runner.sh
        echo "${command} > ${TEST_OUTFILE} 2>&1" >> runner.sh
        if test -n "${sh_file}"; then
            chmod 755 "$(basename "${sh_file}")"
            echo "./$(basename "${sh_file}")" >> runner.sh
        fi
        echo "echo \$? > exit_code ; exit 0" >> runner.sh

        # Run the script and measure execution time. Copy the output to the logfile.
        chmod 755 runner.sh
        TIME_ELAPSED=$( time_cmd ./runner.sh )
        TIME_TOTAL=$(( ${TIME_TOTAL} + ${TIME_ELAPSED} ))
        sed -e 's/^/   > /g' ${TEST_OUTFILE} >> "${TEST_LOGFILE}"

        # Retrieve the exit code. This is either the one of the mpirun call
        # or of the accompanying shell script if present.
        exit_code=$(cat exit_code)

        # Count the total number of tests, the tests skipped, and the tests with error.
        # The values will be stored in the XML report at 'junit_close'.
        # Test failures and diagnostic information are also stored in the xml-report file
        # with 'unit_write'.
        JUNIT_TESTS=$(( ${JUNIT_TESTS} + 1 ))
        if test -z $(echo ${test_name} | grep failure); then
            if test $exit_code -eq 0; then
                echo "Success"
            elif test $exit_code -ge 200 && $exit_code -le 215; then
                echo "Skipped"
                JUNIT_SKIPS=$(( ${JUNIT_SKIPS} + 1 ))
            else
                echo "Failure"
                JUNIT_FAILURES=$(( ${JUNIT_FAILURES} + 1 ))
                junit_write "musictests" "${test_name}" "failure" "$(cat "${TEST_OUTFILE}")"
            fi
        else
            if test $exit_code -ne 0; then
                echo "Success (expected failure)"
            elif test $exit_code -ge 200 && $exit_code -le 215; then
                echo "Skipped"
                JUNIT_SKIPS=$(( ${JUNIT_SKIPS} + 1 ))
            else
                echo "Failure (test failed to fail)"
                JUNIT_FAILURES=$(( ${JUNIT_FAILURES} + 1 ))
                junit_write "musictests" "${test_name}" "failure" "$(cat "${TEST_OUTFILE}")"
            fi
        fi

        cd "${BASEDIR}"
    done

    rm -rf "$tmpdir"

    junit_close
else
  echo "  Not running MUSIC tests because NEST was compiled without support"
  echo "  for it. See the file README.md for details."
fi

echo
echo "Phase 7: Running PyNEST tests"
echo "-----------------------------"
if test "${PYTHON}"; then
    # Find the path to PyNEST without actually importing it
    PYNEST_TEST_DIR="$("${PYTHON}" -c "import importlib.util; print(importlib.util.find_spec('nest').submodule_search_locations[0])")/tests"
    XUNIT_FILE="${REPORTDIR}/07_pynesttests.xml"
    "${PYTHON}" "${NOSE}" -v --with-xunit --xunit-testsuite-name="07_pynesttests" --xunit-file="${XUNIT_FILE}" "${PYNEST_TEST_DIR}" 2>&1 \
        | tee -a "${TEST_LOGFILE}" | grep -i --line-buffered "\.\.\. ok\|fail\|skip\|error" | sed 's/^/  /'
else
    echo
    echo "  Not running PyNEST tests because NEST was compiled without Python support."
    echo
fi

echo
echo "Phase 8: Running C++ tests (experimental)"
echo "-----------------------------------------"

if command -v run_all_cpptests >/dev/null 2>&1; then
  CPP_TEST_OUTPUT=$( run_all_cpptests --logger=JUNIT,error,"${REPORTDIR}/08_cpptests.xml":HRF,error,stdout 2>&1 )
  echo "${CPP_TEST_OUTPUT}" | tail -2
else
  echo "  Not running C++ tests because NEST was compiled without Boost."
fi

# We use plain python3 here to collect results. This also works if
# PyNEST was not enabled and ${PYTHON} is consequently not set.
python3 "$(dirname $0)/summarize_tests.py" "${REPORTDIR}"
TESTSUITE_RESULT=$?

# Mac OS X: Restore old crash reporter state
if test "x${INFO_OS}" = xDarwin ; then
    defaults write com.apple.CrashReporter DialogType "${TEST_CRSTATE}"
fi

exit $TESTSUITE_RESULT
