# run_test.sh
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


# This file only contains helper functions for the NEST testsuite.
# It is meant to be sourced from do_tests.sh rather than being run
# directly.


#
# run_test script_name codes_success codes_skipped codes_failure
#
# script_name: name of a .sli / .py script in $TEST_BASEDIR
#
# codes_success: variable that contains an explanation string for
#                all exit codes that are to be regarded as a success
# codes_skipped: variable that contains an explanation string for
#                all exit codes that mean the test was skipped
# codes_failure: variable that contains an explanation string for
#                all exit codes that are to be regarded as a failure
# Examples:
#
#   codes_success=' 0 Success'
#   codes_skipped=\
#   ' 200 Skipped,'\
#   ' 201 Skipped (MPI required),'\
#   ' 202 Skipped (build with-mpi=OFF required),'\
#   ' 203 Skipped (Threading required),'\
#   ' 204 Skipped (GSL required),'\
#   ' 205 Skipped (MUSIC required),'
#   ' 206 Skipped (Recording backend Arbor required),'
#   codes_failure=\
#   ' 1 Failed: missed assertion,'\
#   ' 2 Failed: error in tested code block,'\
#   ' 3 Failed: tested code block failed to fail,'\
#   ' 126 Failed: error in test script,'
#
# Description:
#
#   The function runs the NEST binary with the SLI script script_name.
#   The exit code is then transformed into a human readable string
#   (if possible) using the global variables CODES_SUCCESS, CODES_SKIPPED, and
#   CODES_FAILURE which contain a comma separated list of exit codes
#   and strings describing the exit code.
#
#   If any of the variables CODES_SUCCESS, CODES_SKIPPED or CODES_FAILURE
#   contain an entry for the exit code, the respective string is logged,
#   and the test is either counted as passed or failed.
#
#   If none of the variables CODES_SUCCESS, CODES_SKIPPED or CODES_FAILURE
#   contain an entry != "" for the returned exit code, the pass is counted as
#   failed, too, and unexpected exit code is logged).
#
run_test ()
{
    TEST_TOTAL=$(( ${TEST_TOTAL} + 1 ))

    param_script="$1"
    param_success="$2"
    param_skipped="$3"
    param_failure="$4"

    msg_error=

    junit_class="$( echo "${param_script}" | sed 's/.[^.]\+$//' | sed 's/\/[^/]\+$//' | sed 's/\//./' )"
    junit_name="$( echo "${param_script}" | sed 's/^.*\/\([^/]\+\)$/\1/' )"

    echo          "Running test '${param_script}'... " >> "${TEST_LOGFILE}"
    printf '%s' "  Running test '${param_script}'... "

    # Very unfortunately, it is cheaper to generate a test runner on fly
    # rather than trying to fight with sh, dash, bash, etc. variable
    # expansion algorithms depending on whether the command is a built-in
    # or not, how many subshells have been forked and so on.

    echo "#!/bin/sh" >  "${TEST_RUNFILE}"
    echo "set +e"   >> "${TEST_RUNFILE}"

    echo "${param_script}" | grep -q '\.sli'
    if test $? -eq 0 ; then
      command="'${NEST}' '${TEST_BASEDIR}/${param_script}' > '${TEST_OUTFILE}' 2>&1"
    else
      # Use plain python3 if the PYTHON variable is unset (i.e. PyNEST
      # was not enabled)
      PYTHON_CMD="${PYTHON:-python3}"	
      command="'${PYTHON_CMD}' '${TEST_BASEDIR}/${param_script}' > '${TEST_OUTFILE}' 2>&1"
    fi

    echo "${command}" >> "${TEST_RUNFILE}"
    echo "echo \$? > '${TEST_RETFILE}' ; exit 0" >> "${TEST_RUNFILE}"

    chmod 700 "${TEST_RUNFILE}"

    TIME_ELAPSED=$( time_cmd "${TEST_RUNFILE}" )
    TIME_TOTAL=$(( ${TIME_TOTAL} + ${TIME_ELAPSED} ))
    JUNIT_TESTS=$(( ${JUNIT_TESTS} + 1 ))
    
    rm -f "${TEST_RUNFILE}"

    exit_code="$(cat "${TEST_RETFILE}")"

    sed 's/^/   > /g' "${TEST_OUTFILE}" >> "${TEST_LOGFILE}"

    msg_dirty=${param_success##* ${exit_code} }
    msg_dirty_skip=${param_skipped##* ${exit_code} }
    msg_clean=${msg_dirty%%,*}
    if test "${msg_dirty}" != "${param_success}" ; then
        explanation="${msg_clean}"
        junit_status=pass
        junit_failure=
    elif test "${msg_dirty_skip}" != "${param_skipped}" ; then
        JUNIT_SKIPS=$(( ${JUNIT_SKIPS} + 1 ))
        msg_dirty=${msg_dirty_skip}
        msg_clean=${msg_dirty%%,*}
        explanation="${msg_clean}"
        junit_status=skipped
        junit_failure="${explanation}"
    else
        JUNIT_FAILURES=$(( ${JUNIT_FAILURES} + 1 ))

        msg_dirty=${param_failure##* ${exit_code} }
        msg_clean=${msg_dirty%%,*}
        msg_error="$( cat "${TEST_OUTFILE}" )"
        if test "${msg_dirty}" != "${param_failure}" ; then
            explanation="${msg_clean}"
        else
            explanation="Failed: unexpected exit code ${exit_code}"
            unexpected_exitcode=true
        fi

        junit_status=failure
        junit_failure="${exit_code} (${explanation})"
    fi

    echo "${explanation}"

    if test "x${msg_error}" != x ; then
        echo ==================================================
        echo "Following is the full output of the test:"
        echo ==================================================
        echo "${msg_error}"
        echo ==================================================
    fi

    echo >> "${TEST_LOGFILE}" "-> ${exit_code} (${explanation})"
    echo >> "${TEST_LOGFILE}" "----------------------------------------"

    junit_write "${junit_class}" "${junit_name}" "${junit_status}" "${junit_failure}" "$(cat "${TEST_OUTFILE}")"

    # Panic on "unexpected" exit code
    if test "x${unexpected_exitcode}" != x ; then
        echo "***"
        echo "*** An unexpected exit code usually hints at a bug in the test suite!"
        ask_results
        exit 2
    fi

    rm -f "${TEST_OUTFILE}" "${TEST_RETFILE}"

}
