#!/bin/bash

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
# The test suite consists of SLI and Python scripts that use the
# respective language's native `unittest` library to assert certain
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
    if test $1 = 1; then
        echo "Error: Unknown option '$2'"
    fi

    if test $1 = 2; then
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

    exit $1
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
            PREFIX="$( echo "$1" | sed 's/^--prefix=//' )"
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

# source helpers to set environment variables and make functions available
. "${PREFIX}/bin/nest_vars.sh"
. "$(dirname $0)/junit_xml.sh"
. "$(dirname $0)/run_test.sh"

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

NEST="nest_serial"
HAVE_MPI="$(get_build_info have_mpi)"
if test "${HAVE_MPI}" = "True"; then
    MPI_LAUNCHER="$(get_build_info mpiexec)"
    MPI_LAUNCHER_VERSION="$($MPI_LAUNCHER --version | head -n1)"
    MPI_LAUNCHER_PREFLAGS="$(get_build_info mpiexec_preflags)"
    # OpenMPI requires --oversubscribe to allow more processes than available cores
    if [[ "${MPI_LAUNCHER_VERSION}" =~ "(OpenRTE)" ]] ||  [[ "${MPI_LAUNCHER_VERSION}" =~ "(Open MPI)" ]]; then
	if [[ ! "$(get_build_info mpiexec_preflags)" =~ "--oversubscribe" ]]; then
	    MPI_LAUNCHER_PREFLAGS="${MPI_LAUNCHER_PREFLAGS} --oversubscribe"
	fi
    fi
    MPI_LAUNCHER_NUMPROC_FLAG="$(get_build_info mpiexec_numproc_flag)"
    MPI_LAUNCHER_CMDLINE="${MPI_LAUNCHER} ${MPI_LAUNCHER_PREFLAGS} ${MPI_LAUNCHER_NUMPROC_FLAG}"
fi

# Under Mac OS X, suppress crash reporter dialogs. Restore old state at end.
if test "x${INFO_OS}" = "xDarwin"; then
    TEST_CRSTATE="$( defaults read com.apple.CrashReporter DialogType )" || true
    echo "TEST_CRSTATE=$TEST_CRSTATE"
    defaults write com.apple.CrashReporter DialogType server || echo "WARNING: Could not set CrashReporter DialogType!"
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
echo "  NEST version ....... $(get_build_info version)"
echo "  PREFIX ............. $PREFIX"
if test -n "${MUSIC}"; then
    MUSIC_VERSION="$("${MUSIC}" --version | head -n1 | cut -d' ' -f2)"
    echo "  MUSIC executable ... $MUSIC (version $MUSIC_VERSION)"
fi
if test -n "${PYTHON}"; then
    PYTHON_VERSION="$("${PYTHON}" --version | cut -d' ' -f2)"
    echo "  Python executable .. $PYTHON (version $PYTHON_VERSION)"
    echo "  PYTHONPATH ......... `print_paths ${PYTHONPATH:-}`"
    echo "  Pytest version ..... $PYTEST_VERSION"
    echo "         timeout ..... $TIME_LIMIT s"
fi
if test "${HAVE_MPI}" = "True"; then
    echo "  Running MPI tests .. yes"
    echo "         launcher .... $MPI_LAUNCHER"
    echo "         version ..... $MPI_LAUNCHER_VERSION"
else
    echo "  Running MPI tests .. no (compiled without MPI support)"
fi
echo "  TEST_BASEDIR ....... $TEST_BASEDIR"
echo "  REPORTDIR .......... $REPORTDIR"
echo "  PATH ............... `print_paths ${PATH}`"
echo
echo "================================================================================"

HEADLINE="NEST $(get_build_info version) testsuite log"
echo >  "${TEST_LOGFILE}" "$HEADLINE"
echo >> "${TEST_LOGFILE}" "$(printf '%0.s=' $(seq 1 ${#HEADLINE}))"
echo >> "${TEST_LOGFILE}" "Running tests from ${TEST_BASEDIR}"

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
    "${PYTHON}" -m pytest --verbose --timeout $TIME_LIMIT --junit-xml="${XUNIT_FILE}" --numprocesses=1 \
          --ignore="${PYNEST_TEST_DIR}/mpi" --ignore="${PYNEST_TEST_DIR}/sli2py_mpi" "${PYNEST_TEST_DIR}" 2>&1 | tee -a "${TEST_LOGFILE}"
    set -e

    # Run tests in the sli2py_mpi subdirectory. The must be run without loading conftest.py.
    if test "${HAVE_MPI}" = "True" && test "${HAVE_OPENMP}" = "true" ; then
	XUNIT_FILE="${REPORTDIR}/${XUNIT_NAME}_sli2py_mpi.xml"
	env
	set +e
	"${PYTHON}" -m pytest --noconftest --verbose --timeout $TIME_LIMIT --junit-xml="${XUNIT_FILE}" --numprocesses=1 \
		    "${PYNEST_TEST_DIR}/sli2py_mpi" 2>&1 | tee -a "${TEST_LOGFILE}"
	set -e
    fi

    # Run tests in the mpi/* subdirectories, with one subdirectory per number of processes to use
    if test "${HAVE_MPI}" = "True"; then
        if test "${MPI_LAUNCHER}"; then
	    # Loop over subdirectories whose names are the number of mpi procs to use
            for numproc in $(cd ${PYNEST_TEST_DIR}/mpi/; ls -d */ | tr -d '/'); do
                XUNIT_FILE="${REPORTDIR}/${XUNIT_NAME}_mpi_${numproc}.xml"
                PYTEST_ARGS="--verbose --timeout $TIME_LIMIT --junit-xml=${XUNIT_FILE} ${PYNEST_TEST_DIR}/mpi/${numproc}"

		if "${DO_TESTS_SKIP_TEST_REQUIRING_MANY_CORES:-false}"; then
		    PYTEST_ARGS="${PYTEST_ARGS} -m 'not requires_many_cores'"
		fi

		set +e
		echo "Running ${MPI_LAUNCHER_CMDLINE} ${numproc} "${PYTHON}" -m pytest ${PYTEST_ARGS}"
                $(${MPI_LAUNCHER_CMDLINE} ${numproc} "${PYTHON}" -m pytest ${PYTEST_ARGS}) 2>&1 | tee -a "${TEST_LOGFILE}"

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
  CPP_TEST_OUTPUT=$( run_all_cpptests --logger=JUNIT,error,"${REPORTDIR}/08_cpptests.xml":HRF,error,stdout 2>&1 )
  set -e
  echo "${CPP_TEST_OUTPUT}" | tail -2
else
  echo "  Not running C++ tests because NEST was compiled without Boost."
fi

# the following steps rely on `$?`, so breaking on error is not an option and we turn it off
set +e

# We use plain python3 here to collect results. This also works if
# PyNEST was not enabled and ${PYTHON} is consequently not set.
python3 "$(dirname $0)/summarize_tests.py" "${REPORTDIR}"
TESTSUITE_RESULT=$?

# Mac OS X: Restore old crash reporter state
if test "x${INFO_OS}" = xDarwin ; then
    defaults write com.apple.CrashReporter DialogType "${TEST_CRSTATE}" || echo "WARNING: Could not reset CrashReporter DialogType to '${TEST_CRSTATE}'!"
fi

exit $TESTSUITE_RESULT
