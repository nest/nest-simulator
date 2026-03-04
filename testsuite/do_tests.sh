#!/usr/bin/env bash

# do_tests.sh
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

#
# Central entry point for the complete NEST test suite. The tests
# ensure a correctly working installation of NEST.
#
# The test suite consists of Python scripts that use the `pytest` library to assert certain
# invariants and thus ensure a correctly working installation of NEST.
#
set -euo pipefail

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
    if test "$1" = 1; then
        echo "Error: Unknown option '$2'"
    fi

    if test "$1" = 2; then
        echo "Error: Missing required option '$2'"
    fi

    cat <<EOF
Usage: $0 --prefix=<path> [options]

Required arguments:
    --prefix=<path>        The base installation path of NEST

Options:
    --with-python=<exe>    The Python executable to use
    --with-music=<exe>     The MUSIC executable to use
    --help                 Print program options and exit
EOF

    exit "$1"
}

PREFIX=""
PYTHON=""
MUSIC=""
while test $# -gt 0 ; do
    case "$1" in
        --help)
            usage 0
            ;;
        --prefix=*)
            PREFIX="${1/--prefix=/}"
            ;;
        --with-python=*)
            PYTHON="${1/--with-python=/}"
            ;;
        --with-music=*)
            MUSIC="${1/--with-music=/}"
            ;;
        *)
            usage 1 "$1"
            ;;
    esac
    shift
done

if test ! "${PREFIX:-}"; then
    usage 2 "--prefix";
fi

if test "${PYTHON}"; then
    TIME_LIMIT=120  # seconds, for each of the Python tests
    PYTEST_VERSION="$(${PYTHON} -m pytest --version --timeout ${TIME_LIMIT} --numprocesses=1 2>&1)" || {
        echo "Error: PyNEST testing requested, but 'pytest' cannot be run."
        echo "       Testing also requires the 'pytest-xdist' and 'pytest-timeout' extensions."
        exit 1
    }
    PYTEST_VERSION="$(echo "${PYTEST_VERSION}" | cut -d' ' -f2)"
fi

if ! ${PYTHON} -c "import junitparser" >/dev/null 2>&1; then
    echo "Error: Required Python package 'junitparser' not found."
    exit 1
fi

# Set PATH (for cpptests) and PYTHONPATH (for Python tests)
set -x
export PATH="${PREFIX}/bin${PATH:+:$PATH}"
echo "PATH: ${PATH}"

PYTHON_VERSION="$(python --version | cut -d' ' -f 2)"
NEST_PY_PATH="${PREFIX}/lib/python${PYTHON_VERSION%.*}/site-packages"
export PYTHONPATH="${NEST_PY_PATH}${PYTHONPATH:+:$PYTHONPATH}"

# source helpers to set environment variables and make functions available
# shellcheck source=testsuite/junit_xml.sh
. "$(dirname "$0")/junit_xml.sh"
# shellcheck source=testsuite/run_test.sh
. "$(dirname "$0")/run_test.sh"

# Directory containing installed tests
TEST_BASEDIR="${PREFIX}/share/nest/testsuite"

# Create the report directory in the directory in which make installcheck is called (do_tests.sh is run).
# Use absolute pathnames, this is necessary for MUSIC tests which otherwis will not find the logfiles
# while running in temporary directories.
REPORTDIR="${PWD}/$(mktemp -d test_report_XXX)"

TEST_LOGFILE="${REPORTDIR}/installcheck.log"
TEST_OUTFILE="${REPORTDIR}/output.log"
TEST_RETFILE="${REPORTDIR}/output.ret"
TEST_RUNFILE="${REPORTDIR}/runtest.sh"

get_build_info ()
{
  ${PYTHON} -c "import nest; print(nest.build_info['$1'])" --quiet
}

HAVE_MPI="$(get_build_info have_mpi)"
HAVE_OPENMP="$(get_build_info have_threads)"
HAVE_MUSIC=${MUSIC:+True}

if test "${HAVE_MPI}" = "True"; then
    MPI_LAUNCHER="$(get_build_info mpiexec)"
    MPI_LAUNCHER_VERSION="$($MPI_LAUNCHER --version | head -n1)"
    # TODO PyNEST-NG The two PREFLAGS variables are double up, as is some code further down relating to it. Sort out.
    MPI_LAUNCHER_PREFLAGS="$(get_build_info mpiexec_preflags) --prefix $(python -c 'import sys; print(sys.prefix)')"
    # OpenMPI requires --oversubscribe to allow more processes than available cores
    #
    # ShellCheck warns about "SC2076 (warning): Remove quotes from right-hand side of =~ to match as a regex rather than literally.",
    # but we want to match literally, therefore:
    # shellcheck disable=SC2076
    if [[ "${MPI_LAUNCHER_VERSION}" =~ "(OpenRTE)" ]] ||  [[ "${MPI_LAUNCHER_VERSION}" =~ "(Open MPI)" ]]; then
	if [[ ! "$(get_build_info mpiexec_preflags)" =~ "--oversubscribe" ]]; then
	    MPI_LAUNCHER_PREFLAGS="${MPI_LAUNCHER_PREFLAGS} --oversubscribe"
	fi
    fi
    MPI_LAUNCHER_NUMPROC_FLAG="$(get_build_info mpiexec_numproc_flag)"
    MPI_LAUNCHER_CMDLINE="${MPI_LAUNCHER} ${MPI_LAUNCHER_PREFLAGS} ${MPI_LAUNCHER_NUMPROC_FLAG}"
fi

# Under Mac OS X, suppress crash reporter dialogs. Restore old state at end.
echo "INFO_OS=${INFO_OS:-}"
if test "${INFO_OS:-}" = "Darwin"; then
    TEST_CRSTATE="$( defaults read com.apple.CrashReporter DialogType )" || true
    echo "TEST_CRSTATE=$TEST_CRSTATE"
    defaults write com.apple.CrashReporter DialogType server || echo "WARNING: Could not set CrashReporter DialogType!"
fi

print_paths () {
    indent="$(printf '%23s' "")"
    echo "$1" | sed "s/:/\n$indent/g" | sed '/^\s*$/d'
}

echo "================================================================================"
echo
echo "  NEST testsuite"
echo "  Date: $(date -u)"
echo "  Sysinfo: $(uname -s -r -m)"
echo
echo "  NEST version ....... $(get_build_info version)"
echo "  PREFIX ............. $PREFIX"
if test "${HAVE_MUSIC}" = "True"; then
    MUSIC_VERSION="$("${MUSIC}" --version | head -n1 | cut -d' ' -f2)"
    echo "  MUSIC executable ... ${MUSIC} (version ${MUSIC_VERSION})"
fi
if test -n "${PYTHON}"; then
    PYTHON_VERSION="$("${PYTHON}" --version | cut -d' ' -f2)"
    echo "  Python executable .. ${PYTHON} (version ${PYTHON_VERSION})"
    echo "  PYTHONPATH ......... $(print_paths "${PYTHONPATH:-}")"
    echo "  Pytest version ..... ${PYTEST_VERSION}"
    echo "         timeout ..... ${TIME_LIMIT} s"
fi
if test "${HAVE_MPI}" = "True"; then
    echo "  Running MPI tests .. yes"
    echo "         launcher .... ${MPI_LAUNCHER}"
    echo "         version ..... ${MPI_LAUNCHER_VERSION}"
    echo "         cmdline ..... ${MPI_LAUNCHER_CMDLINE}"
else
    echo "  Running MPI tests .. no (compiled without MPI support)"
fi
echo "  TEST_BASEDIR ....... ${TEST_BASEDIR}"
echo "  REPORTDIR .......... ${REPORTDIR}"
echo "  PATH ............... $(print_paths "${PATH}")"
echo
echo "================================================================================"

HEADLINE="NEST $(get_build_info version) testsuite log"
{
    echo "${HEADLINE}"
    printf '%0.s=' $(seq 1 ${#HEADLINE})
    echo "Running tests from ${TEST_BASEDIR}"
} >"${TEST_LOGFILE}"


echo
echo "Phase 6: Running MUSIC tests"
echo "----------------------------"
if test "${MUSIC}"; then
    junit_open '06_musictests'

    # Create a temporary directory with a unique name.
    BASEDIR="$PWD"
    TMPDIR_MUSIC="$(mktemp -d)"

    TESTDIR="${TEST_BASEDIR}/sli2py_music/"

    # Initialize tracking variables to avoid 'unbound variable' errors under set -u
    JUNIT_TESTS=0
    JUNIT_SKIPS=0
    JUNIT_FAILURES=0
    TIME_TOTAL=0

    # shellcheck disable=SC2044
    for test_name in $(find "${TESTDIR}" -maxdepth 1 -name '*.music' -printf '%f\n'); do
        music_file="${TESTDIR}/${test_name}"

        # Collect the list of Python files from the '.music' file.
        py_files=()

        # Use sed to strip leading spaces and 'binary=', leaving only the file path.
        # We pipe to grep to ensure we only capture lines containing '.py'.
        readarray -t raw_py_files < <(sed -n 's/^[[:space:]]*binary=\(.*\)/\1/p' "${music_file}" | grep '\.py' || true)
        if [ ${#raw_py_files[@]} -gt 0 ]; then
            for raw_file in "${raw_py_files[@]}"; do
                # Construct the full path.
                # ${raw_file#./} removes any leading './' so we don't end up with messy paths like '/dir/./file.py'
                f="${TESTDIR}${raw_file#./}"

                # Check if the file exists and append it to our final array
                if test -f "${f}"; then
                    py_files+=("${f}")
                fi
            done
        else
            echo "No python files found in music file ${music_file}"
        fi

        # Check if there is an accompanying shell script for the test.
        sh_file="${TESTDIR}/$(basename "${music_file}" ".music").sh"
        if test ! -f "${sh_file}"; then sh_file=""; fi

        # Check if there is an accompanying input data file
        input_file="${TESTDIR}/$(basename "${music_file}" ".music")0.dat"
        if test ! -f "${input_file}"; then input_file=""; fi

        # Calculate the total number of processes from the '.music' file.
        np="$(($(sed -n 's/np=//p' "${music_file}" | paste -sd'+' -)))"
        test_command="${MPI_LAUNCHER_CMDLINE} ${np} ${MUSIC} ${test_name}"

        proc_txt="processes"
        if test $np -eq 1; then proc_txt="process"; fi
        echo          "Running test '${test_name}' with $np $proc_txt... " >> "${TEST_LOGFILE}"
        printf '%s' "  Running test '${test_name}' with $np $proc_txt... "

        # Copy everything to TMPDIR_MUSIC.
        # Note that variables might also be empty, so test for file existence first.
        # We double-quote "${py_files[@]}" so bash expands it to individual, safe arguments.
        for filename in "${music_file}" "${sh_file}" "${input_file}" "${py_files[@]}"; do
            if test -n "${filename}" && test -e "${filename}"; then
                cp "${filename}" "${TMPDIR_MUSIC}"
            fi
        done

        # Create the runner script in TMPDIR_MUSIC.
        cd "${TMPDIR_MUSIC}"
        {
            echo "#!/usr/bin/env sh"
            echo "set +e"
            echo "NEST_DATA_PATH=\"${TMPDIR_MUSIC}\""
            echo "timeout 5m ${test_command} > ${TEST_OUTFILE} 2>&1 < /dev/null"
            echo "RET=\$?"
            echo "if [ \$RET -eq 124 ]; then echo 'TIMEOUT_ERROR' >> ${TEST_OUTFILE}; fi"
            if test -n "${sh_file}"; then
                chmod 755 "$(basename "${sh_file}")"
                echo "./$(basename "${sh_file}")"
            fi
            echo "echo \$RET > exit_code"
        } >"runner.sh"

        # Run the script and measure execution time. Copy the output to the logfile.
        music_path="$(dirname "${MUSIC}")"
        chmod 755 "runner.sh"
        TIME_ELAPSED="$(PATH="$PATH:${music_path}" time_cmd ./runner.sh )"
        TIME_TOTAL="$(( TIME_TOTAL + TIME_ELAPSED ))"
        sed -e 's/^/   > /g' "${TEST_OUTFILE}" >> "${TEST_LOGFILE}"

        # Retrieve the exit code. This is either the one of the mpirun call
        # or of the accompanying shell script if present.
        exit_code="$(cat exit_code)"

        # Count the total number of tests, the tests skipped, and the tests with error.
        # The values will be stored in the XML report at 'junit_close'.
        # Test failures and diagnostic information are also stored in the xml-report file
        # with 'unit_write'.
        JUNIT_TESTS="$(( JUNIT_TESTS + 1 ))"
        if test -z "$(echo "${test_name}" | grep failure)"; then
            if test "$exit_code" -eq 0; then
                echo "Success"
            elif [ "$exit_code" -ge 200 ] && [ "$exit_code" -le 215 ]; then
                echo "Skipped"
                JUNIT_SKIPS="$(( JUNIT_SKIPS + 1 ))"
            else
                echo "Failure"
                JUNIT_FAILURES="$(( JUNIT_FAILURES + 1 ))"
                junit_write "musictests" "${test_name}" "failure" "$(cat "${TEST_OUTFILE}")"
            fi
        else
            if test "$exit_code" -ne 0; then
                echo "Success (expected failure)"
            elif [ "$exit_code" -ge 200 ] && [ "$exit_code" -le 215 ]; then
                echo "Skipped"
                JUNIT_SKIPS="$(( JUNIT_SKIPS + 1 ))"
            else
                echo "Failure (test failed to fail)"
                JUNIT_FAILURES="$(( JUNIT_FAILURES + 1 ))"
                junit_write "musictests" "${test_name}" "failure" "$(cat "${TEST_OUTFILE}")"
            fi
        fi

        cd "${BASEDIR}"
    done

    junit_close
else
    echo "  Not running MUSIC tests because NEST was compiled without support"
    echo "  for it."
fi



echo
echo "Phase 7: Running PyNEST tests"
echo "-----------------------------"

if test "${PYTHON}"; then
    PYNEST_TEST_DIR="${TEST_BASEDIR}/pytests"
    XUNIT_NAME="07_pynesttests"

    # Run all tests except those in the mpi* and sli2py_mpi subdirectories because they cannot be run concurrently
    XUNIT_FILE="${REPORTDIR}/${XUNIT_NAME}.xml"
    env
    set +e
    ${PYTHON} -m pytest --verbose --timeout "${TIME_LIMIT}" --junit-xml="${XUNIT_FILE}" \
	                --ignore="${PYNEST_TEST_DIR}/mpi" --ignore="${PYNEST_TEST_DIR}/sli2py_mpi" "${PYNEST_TEST_DIR}" 2>&1 | tee -a "${TEST_LOGFILE}"

    set -e

    # Run tests in the sli2py_mpi subdirectory. The must be run without loading conftest.py.
    if test "${HAVE_MPI}" = "True" && test "${HAVE_OPENMP}" = "True" ; then
        XUNIT_FILE="${REPORTDIR}/${XUNIT_NAME}_sli2py_mpi.xml"
        env
        set +e
        "${PYTHON}" -m pytest --verbose --timeout "${TIME_LIMIT}" --junit-xml="${XUNIT_FILE}" --numprocesses=1 \
            "${PYNEST_TEST_DIR}/sli2py_mpi" 2>&1 | tee -a "${TEST_LOGFILE}"
        set -e
    fi

    # Run tests in the mpi/* subdirectories, with one subdirectory per number of processes to use
    if test "${HAVE_MPI}" = "True"; then
        if test "${MPI_LAUNCHER}"; then

      if test "${INFO_OS:-}" = "Darwin"; then
   # ref https://stackoverflow.com/a/752893
   # Note that on GNU systems an additional '-r' would be needed for
   # xargs, which is not available here.
                proc_nums=$(cd "${PYNEST_TEST_DIR}/mpi/"; find ./* -maxdepth 0 -type d -print0 | xargs -0 -n1 basename)
      else
                proc_nums=$(cd "${PYNEST_TEST_DIR}/mpi/"; find ./* -maxdepth 0 -type d -printf "%f\n")
      fi

        # Loop over subdirectories whose names are the number of mpi procs to use
        for numproc in ${proc_nums}; do
            XUNIT_FILE="${REPORTDIR}/${XUNIT_NAME}_mpi_${numproc}.xml"
            PYTEST_ARGS="--verbose --timeout ${TIME_LIMIT} --junit-xml=${XUNIT_FILE} ${PYNEST_TEST_DIR}/mpi/${numproc}"

		set +e
		# Some doubling up of code here because trying to add the -m 'not requires...' to PYTEST_ARGS
		# loses the essential quotes.
		if test "${DO_TESTS_SKIP_TEST_REQUIRING_MANY_CORES:-False}" != "False"; then
		    echo "Running ${MPI_LAUNCHER_CMDLINE} ${numproc} ${PYTHON} -m pytest ${PYTEST_ARGS} -m 'not requires_many_cores'"
            # Double-quoting PYTEST_ARGS here does not work
            # shellcheck disable=SC2086
            ${MPI_LAUNCHER_CMDLINE} "${numproc}" "${PYTHON}" -m pytest ${PYTEST_ARGS} -m 'not requires_many_cores' 2>&1 | tee -a "${TEST_LOGFILE}"
		else
		    echo "Running ${MPI_LAUNCHER_CMDLINE} ${numproc} ${PYTHON} -m pytest ${PYTEST_ARGS}"
            # Double-quoting PYTEST_ARGS here does not work
            # shellcheck disable=SC2086
            ${MPI_LAUNCHER_CMDLINE} "${numproc}" "${PYTHON}" -m pytest ${PYTEST_ARGS} 2>&1 | tee -a "${TEST_LOGFILE}"
		fi
		set -e
            done
        fi
    fi
else
    echo
    echo "  Not running PyNEST tests because NEST was compiled without Python support."
    echo
fi

echo
echo "Phase 8: Running C++ tests (experimental)"
echo "-----------------------------------------"

if command -v run_all_cpptests >/dev/null 2>&1; then
    set +e
    CPP_TEST_OUTPUT="$( run_all_cpptests --logger=JUNIT,error,"${REPORTDIR}/08_cpptests.xml":HRF,error,stdout 2>&1 )"
    set -e
    echo "${CPP_TEST_OUTPUT}" | tail -2
else
    echo "  Not running C++ tests because NEST was compiled without Boost."
fi

# the following steps rely on `$?`, so breaking on error is not an option and we turn it off
set +e

# We use plain python3 here to collect results. This also works if
# PyNEST was not enabled and ${PYTHON} is consequently not set.
SUMMARY_OPTS=()
if test "${DO_TESTS_SKIP_TEST_REQUIRING_MANY_CORES:-False}" != "False"; then
   SUMMARY_OPTS+=("--no-manycore-tests")
fi
if test "${HAVE_MPI}" = "True"; then
   SUMMARY_OPTS+=("--have-mpi")
fi
if test "${HAVE_OPENMP}" = "True"; then
   SUMMARY_OPTS+=("--have-openmp")
fi
if test "${HAVE_MUSIC}" = "True"; then
   SUMMARY_OPTS+=("--have-music")
fi
python3 "$(dirname "$0")/summarize_tests.py" "${SUMMARY_OPTS[@]}" "${REPORTDIR}"
TESTSUITE_RESULT="$?"

# Mac OS X: Restore old crash reporter state
if test "${INFO_OS:-}" = "Darwin" ; then
    defaults write com.apple.CrashReporter DialogType "${TEST_CRSTATE}" || echo "WARNING: Could not reset CrashReporter DialogType to '${TEST_CRSTATE}'!"
fi

exit $TESTSUITE_RESULT
