#!/bin/sh
#
# This script runs the NEST test suite.
#
# The test suite consists of SLI and Python scripts that use the
# language's native `unittest` library to assert certain invariants
# and thus ensure a correctly working installation of NEST.
#
# For commandline options, see the the function usage() below.
#

# from http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -eu
#set -o pipefail
#IFS=$' \n\t'
#set -x # or -o xtrace
#set -v # or -o verbose

#
# usage <exit_code> <argument>
#
# usage 1 <option>
#   Use this variant in case of an unknown option <opt>
# usage 2 <option>
#   Use this variant in case of a missing required option <opt>
# usage 3 <message>
#   Use this variant to print a custom error message <message>
#
usage ()
{
    if test "$1" = 1 ; then
        echo "Error: Unknown option \'$2\'"
    fi

    if test "$1" = 2 ; then
        echo "Error: Missing required option \'$2\'"
    fi

    if test "$1" = 3 ; then
        echo "$2"
    fi

    cat <<EOF
Usage: do_tests.sh --prefix=<path> --report-dir=<path> [options]"

    --prefix=<path>        The base installation path of NEST
    --report-dir=<path>    The directory to store the output in

Options:

    --with-python=<exe>    The Python executable to use
    --python-path=<path>   The PYTHONPATH for the NEST installation
    --with-music=<exe>     The MUSIC executable to use
    --help                 Print program options and exit
EOF

    exit "$1"
}


#
# bail_out message
#
bail_out ()
{
    echo "$1"
    exit 1
}


#
# read certain quantities from the 'testsuite' tags in a JUnit XML
# file FILE. KEY can be one of 'errors', 'failures', 'skips', 'tests',
# or 'time'. Use like this: read_junitxml FILE KEY
#
read_junitxml () {
    grep -o '<testsuite[^>]*' $1 \
        | grep -o "$2=\"[^ ]*\"" | cut -d= -f2 | tr -d \" | tr '\n' '+'
}


#
# sed has different syntax for extended regular expressions
# on different operating systems:
# BSD: -E
# other: -r
#
EXTENDED_REGEX_PARAM="r"
/bin/sh -c "echo 'hello' | sed -${EXTENDED_REGEX_PARAM} 's/[aeou]/_/g' "  >/dev/null 2>&1 || EXTENDED_REGEX_PARAM=E



RUN_TEST="$(dirname $0)/run_test"
MUSIC=""
PYTHON=""


while test $# -gt 0 ; do
    case "$1" in
        --help)
            usage 0
            ;;
        --prefix=*)
            PREFIX="$( echo "$1" | sed 's/^--prefix=//' )"
            if test ! "${PREFIX}"; then usage 2 "--prefix"; fi
            ;;
        --report-dir=*)
            REPORTDIR="$( echo "$1" | sed 's/^--report-dir=//' )"
            if test ! "${REPORTDIR}"; then usage 2 "--report-dir"; fi
            ;;
        --with-python=*)
            PYTHON="$( echo "$1" | sed 's/^--with-python=//' )"
            ;;
        --python-path=*)
            PYTHONPATH_="$( echo "$1" | sed 's/^--python-path=//' )"
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
if test ! "${PREFIX}"; then usage 2 "--prefix"; fi

export NEST="nest_serial"
export PATH="${PREFIX}/bin:$PATH"

unset NEST_INSTALL_DIR
unset NEST_DATA_DIR
unset NEST_DOC_DIR

if test "${PYTHON}"; then
    if test ! "${PYTHONPATH_}"; then
        usage 3 "Error: \'--with-python\' also requires \'--python-path\'"
    fi

    command -v pytest 2>&1 || {
        echo "Error: PyNEST testing requested but 'pytest' command not executable."
        exit 1
    }

    export PYTHONPATH="$PYTHONPATH_:${PYTHONPATH:-}"
fi

export TEST_BASEDIR="${PREFIX}/share/doc/nest"

# Gather some information about the host
INFO_ARCH="$(uname -m)"
INFO_HOME="$(/bin/sh -c 'echo ~')"
INFO_HOST="$(hostname -f)"
INFO_OS="$(uname -s)"
INFO_USER="$(whoami)"
INFO_VER="$(uname -r)"
NPROCS="$(cat /proc/cpuinfo | grep processor | wc -l)"
BASEDIR="$PWD"

print_paths () {
    indent="`printf '%23s'`"
    echo "$1" | sed "s/:/\n$indent/g" | sed '/^\s*$/d'
}

echo "================================================================================"
echo
echo "  NEST testsuite"
echo "  Date: $(date -u)"
echo "  Sysinfo: $(uname -s -r -m -o)"
echo "  Running ${NPROCS} tests in parallel where possible"
echo
echo "  NEST executable .... $NEST"
echo "  Python executable .. $PYTHON"
echo "  PYTHONPATH ......... `print_paths ${PYTHONPATH:-}`"
echo "  TEST_BASEDIR ....... $TEST_BASEDIR"
echo "  PREFIX ............. $PREFIX"
echo "  REPORTDIR .......... $REPORTDIR"
echo "  PATH ............... `print_paths $PATH`"
echo
echo "================================================================================"

# logfiles and log-prefixes for test specific logs
export TEST_LOGFILE="${REPORTDIR}/installcheck.log"
export TEST_OUTFILE="${REPORTDIR}/output.log"
export TEST_RETFILE="${REPORTDIR}/output.ret"
export TEST_RUNFILE="${REPORTDIR}/runtest.sh"

# parallel execution line-counting files
export TEST_TOTAL="${REPORTDIR}/TOTAL"
export TEST_PASSED="${REPORTDIR}/PASSED"
export TEST_SKIPPED="${REPORTDIR}/SKIPPED"
export TEST_FAILED="${REPORTDIR}/FAILED"

# integer counting variables for serial tests
CPP_TEST_TOTAL=0
CPP_TEST_PASSED=0
CPP_TEST_SKIPPED=0
CPP_TEST_FAILED=0
PYNEST_TEST_TOTAL=0
PYNEST_TEST_PASSED=0
PYNEST_TEST_SKIPPED=0
PYNEST_TEST_FAILED=0

if test -d "${REPORTDIR}" ; then
    rm -rf "${REPORTDIR}"
fi

mkdir "${REPORTDIR}"
TEST_TMPDIR="$(mktemp -d ${REPORTDIR}/tmp.XXX)"
NEST_DATA_PATH="${TEST_TMPDIR}"
export NEST_DATA_PATH
export TEST_BASEDIR

# touch output files in case they are never written (e.g. no tests fail)
touch "$TEST_TOTAL" "$TEST_PASSED" "$TEST_SKIPPED" "$TEST_FAILED"

# Check for old version of the /mpirun command, which had the NEST executable hardcoded
if test "x$(sli -c '/mpirun load cva_t Flatten length 3 eq =')" = xtrue ; then
    echo "  Unable to run tests because you compiled with MPI and ~/.nestrc contains"
    echo "  an old definition of /mpirun. If you were using the standard definition,"
    echo "  please replace it by"
    echo
    echo "  /mpirun"
    echo "  [/integertype /stringtype /stringtype]"
    echo "  [/numproc     /executable /scriptfile]"
    echo "  {"
    echo "   () ["
    echo "    (mpirun -np ) numproc cvs ( ) executable ( ) scriptfile"
    echo "   ] {join} Fold"
    echo "  } Function def"
    echo
    echo "  If you used a custom definition, please adapt it so that the signature"
    echo "  of your version matches the one above (i.e. taking number of processes,"
    echo "  executable and scriptfile as arguments; the old one just took number of"
    echo "  processes and slifile, the executable \"nest\" was hard-coded)."
    echo
    echo
    exit 1
fi

# Under Mac OS X, suppress crash reporter dialogs. Restore old state at end.
if test "x${INFO_OS}" = xDarwin ; then
    TEST_CRSTATE="$( defaults read com.apple.CrashReporter DialogType )"
    defaults write com.apple.CrashReporter DialogType server
fi

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
' 205 Skipped (MUSIC required),'

phase_one() {
    echo
    echo 'Phase 1: Testing if SLI can execute scripts and report errors'
    echo '-------------------------------------------------------------'

    CODES_SUCCESS=' 0 Success'
    CODES_FAILURE=
    xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "${TEST_BASEDIR}/selftests/{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}" <<TESTS
        test_pass.sli
        test_goodhandler.sli
        test_lazyhandler.sli
TESTS

    CODES_SUCCESS=' 126 Success'
    CODES_FAILURE=
    xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "${TEST_BASEDIR}/selftests/{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}" <<TESTS
        test_fail.sli
        test_stop.sli
        test_badhandler.sli
TESTS
}

phase_two() {
    # At this point, we are sure that
    #
    #  * NEST will return 0 after finishing a script
    #  * NEST will return 126 when a script raises an unhandled error
    #  * Error handling in stopped contexts works

    echo
    echo "Phase 2: Testing SLI's unittest library"
    echo "---------------------------------------"

    # assert_or_die uses pass_or_die, so pass_or_die should be tested first.

    CODES_SUCCESS=' 2 Success'
    CODES_FAILURE=' 126 Failed: error in test script'
    $RUN_TEST ${TEST_BASEDIR}/selftests/test_pass_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

    CODES_SUCCESS=' 1 Success'
    CODES_FAILURE=\
'     2 Failed: error in tested code block,'\
'     126 Failed: error in test script,'
    xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "${TEST_BASEDIR}/selftests/{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}" <<TESTS
        test_assert_or_die_b.sli
        test_assert_or_die_p.sli
TESTS

    CODES_SUCCESS=' 3 Success'
    CODES_FAILURE=\
'     1 Failed: missed assertion,'\
'     2 Failed: error in tested code block,'\
'     126 Failed: error in test script,'
    $RUN_TEST ${TEST_BASEDIR}/selftests/test_fail_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

    CODES_SUCCESS=' 3 Success'
    CODES_FAILURE=\
'     1 Failed: missed assertion,'\
'     2 Failed: error in tested code block,'\
'     126 Failed: error in test script,'
    $RUN_TEST ${TEST_BASEDIR}/selftests/test_crash_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"

    CODES_SUCCESS=' 3 Success'
    CODES_FAILURE=' 1 Failed: missed assertion,'\
'     2 Failed: error in tested code block,'\
'     126 Failed: error in test script,'
    xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "${TEST_BASEDIR}/selftests/{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}" <<TESTS
        test_failbutnocrash_or_die_crash.sli
        test_failbutnocrash_or_die_pass.sli
TESTS

    CODES_SUCCESS=' 3 Success'
    CODES_FAILURE=\
'     1 Failed: missed assertion,'\
'     2 Failed: error in tested code block,'\
'     126 Failed: error in test script,'

    $RUN_TEST ${TEST_BASEDIR}/selftests/test_passorfailbutnocrash_or_die.sli "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"


    # At this point, we are sure that
    #
    #  * unittest::pass_or_die works
    #  * unittest::assert_or_die works
    #  * unittest::fail_or_die works
    #  * unittest::crash_or_die works

    # These are the default exit codes and their explanations
    CODES_SUCCESS=' 0 Success'
    CODES_FAILURE=\
'     1 Failed: missed SLI assertion,'\
'     2 Failed: error in tested code block,'\
'     3 Failed: tested code block failed to fail,'\
'     4 Failed: re-run serial,'\
'     10 Failed: unknown error,'\
'     20 Failed: inconsistent copyright header(s),'\
'     30 Failed: inconsistent Name definition(s)/declaration(s),'\
'     31 Failed: unused Name definition(s),'\
'     125 Failed: unknown C++ exception,'\
'     126 Failed: error in test script,'\
'     127 Failed: fatal error,'\
'     134 Failed: missed C++ assertion,'\
'     139 Failed: segmentation fault,'
}

phase_three() {
    echo
    echo "Phase 3: Integration  tests"
    echo "---------------------------"

    echo "TEST_BASEDIR $TEST_BASEDIR"
    echo "PWD $(pwd)"
    find "${TEST_BASEDIR}/unittests" -name "*.py" -o -name "*.sli" |\
        xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
}

phase_four() {
    echo
    echo "Phase 4: Regression tests"
    echo "-------------------------"

    find "${TEST_BASEDIR}/regressiontests/" -name "*.sli" -o -name "*.py" |\
        xargs -L1 -P${NPROCS} -I'{}' $RUN_TEST "{}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
}

phase_five() {
    echo
    echo "Phase 5: MPI tests"
    echo "------------------"

    if test "x$(sli -c 'statusdict/have_mpi :: =')" = xtrue ; then

        NEST=${PREFIX}/bin/nest_indirect
        for test_name in $(ls "${TEST_BASEDIR}/mpi_selftests/pass" | grep '.*\.sli$') ; do
            $RUN_TEST "${TEST_BASEDIR}/mpi_selftests/pass/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
        done

        # tests meant to fail
        SAVE_CODES_SUCCESS=${CODES_SUCCESS}
        SAVE_CODES_FAILURE=${CODES_FAILURE}
        CODES_SUCCESS=' 1 Success (expected failure)'
        CODES_FAILURE=' 0 Failed: Unittest failed to detect error.'
        for test_name in $(ls "${TEST_BASEDIR}/mpi_selftests/fail" | grep '.*\.sli$') ; do
            $RUN_TEST "${TEST_BASEDIR}/mpi_selftests/fail/${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
        done
        CODES_SUCCESS=${SAVE_CODES_SUCCESS}
        CODES_FAILURE=${SAVE_CODES_FAILURE}

        for test_name in $(ls "${TEST_BASEDIR}/mpitests/" | grep '.*\.sli$') ; do
            $RUN_TEST "${TEST_BASEDIR}/${test_dir}${test_name}" "${CODES_SUCCESS}" "${CODES_SKIPPED}" "${CODES_FAILURE}"
        done

    else
      echo "  Not running MPI tests because NEST was compiled without support"
      echo "  for distributed computing. See the file README.md for details."
    fi
}

phase_six() {
    echo
    echo "Phase 6: MUSIC tests"
    echo "--------------------"

    if test ${MUSIC}; then

        TESTDIR="${TEST_BASEDIR}/musictests/"

        for test_name in $(ls "${TESTDIR}" | grep '.*\.music$') ; do
            tmpdir="$(mktemp -d)"
            music_file="${TESTDIR}/${test_name}"

            # Collect the list of SLI files from the .music file.
            sli_files="$(grep '\.sli' "${music_file}" | sed -e "s#args=#${TESTDIR}#g")"
            sli_files="$(for f in ${sli_files}; do if test -f ${f}; then echo ${f}; fi; done)"

            # Check if there is an accompanying shell script for the test.
            sh_file="${TESTDIR}/$(basename ${music_file} .music).sh"
            if test ! -f "${sh_file}"; then sh_file=""; fi

            # Calculate the total number of processes in the .music file.
            np="$(($(sed -n 's/np=//p' ${music_file} | paste -sd'+' -)))"
            command="$(sli -c "${np} (${MUSIC}) (${test_name}) mpirun =")"

            proc_txt="processes"
            if test "$np" -eq 1; then proc_txt="process"; fi
            echo          "Running test 'musictests/${test_name}' with $np $proc_txt... " >> "${TEST_LOGFILE}"
            printf '%s' "  Running test 'musictests/${test_name}' with $np $proc_txt... "

            # Copy everything to the tmpdir.
            cp "${music_file}" ${sh_file} ${sli_files} "${tmpdir}"
            cd "${tmpdir}"

            # Create the runner script
            SHFILE=""
            if test -n "${sh_file}"; then
                chmod 755 "$(basename ${sh_file})"
                SHFILE="./$(basename ${sh_file})"
            fi
            cat >runner.sh <<EOT
#!/bin/sh
set +e
export NEST_DATA_PATH="${tmpdir}"
${command} > output.log 2>&1
$SHFILE
echo \$? > exit_code ; exit 0
EOT

            # Run the script and copy all output to the logfile.
            chmod 755 runner.sh
            ./runner.sh
            sed -e 's/^/   > /g' output.log >> "${TEST_LOGFILE}"

            # Retrieve the exit code. This is either the one of the mpirun
            # call or of the accompanying shell script if present.
            exit_code=$(cat exit_code)

            echo "CHECK ${tmpdir}" >>"${TEST_LOGFILE}"
            cd "${BASEDIR}"

            # If the name of the test contains 'failure', we expect it to
            # fail and the test logic is inverted.
            echo "${test_name}" >> "${TEST_TOTAL}"
            if test -z $(echo "${test_name}" | grep failure); then
                if test $exit_code -eq 0 ; then
                    echo "Success"
                    echo "${test_name}" >> "${TEST_PASSED}"
                    rm -rf $tmpdir
                elif test $exit_code -ge 200 && $exit_code -le 215; then
                    echo "Skipped"
                    echo "${test_name}" >> "${TEST_SKIPPED}"
                else
                    echo "Failure"
                    echo "${test_name}" >> "${TEST_FAILED}"
                fi
            else
                if test $exit_code -ne 0 ; then
                    echo "Success (expected failure)"
                    echo "${test_name}" >> "${TEST_PASSED}"
                    rm -rf $tmpdir
                elif test $exit_code -ge 200 && $exit_code -le 215; then
                    echo "Skipped"
                    echo "${test_name}" >> "${TEST_SKIPPED}"
                else
                    echo "Failure (test failed to fail)"
                    echo "${test_name}" >> "${TEST_FAILED}"
                fi
            fi
        done


    else
      echo "  Not running MUSIC tests because NEST was compiled without support"
      echo "  for it. See the file README.md for details."
    fi
}

phase_seven() {
    echo
    echo "Phase 7: PyNEST tests."
    echo "----------------------"

    if test ${PYTHON}; then

        PYTEST_OPTS="-m pytest --tb=auto --disable-warnings"
        cd "${PREFIX}/share/nest/tests/python"

        JUNITFILE="${REPORTDIR}"/pytest-serial.log
        COLUMNS=110 ${PYTHON} -u $PYTEST_OPTS --junitxml="${JUNITFILE}" --ignore=/mpi_tests.py . 2>&1 \
            | grep --line-buffered -v "========" | grep --line-buffered -v "generated xml file" \
            | grep --line-buffered -v '^$' | grep --line-buffered -v '^platform\|^rootdir\|^collected' \
            | sed -u 's/^/  /' | tee -a "${TEST_LOGFILE}"

        # The trailing 0 is needed because read_junitxml prints a trailing '+'.
        # Without having it, the calculation would end in that '+'.
        PYNEST_TEST_TOTAL=$(($(read_junitxml ${JUNITFILE} "tests") 0))
        PYNEST_TEST_SKIPPED=$(($(read_junitxml ${JUNITFILE} "skips") 0))
        PYNEST_TEST_FAILURES=$(($(read_junitxml ${JUNITFILE} "failures") 0))
        PYNEST_TEST_ERRORS=$(($(read_junitxml ${JUNITFILE} "errors") 0))

        echo

        if test "x$(sli -c 'statusdict/have_mpi :: =')" = xtrue ; then
            echo "  Running PyNEST MPI tests"
            echo "  ------------------------"
            JUNITFILE="${REPORTDIR}"/pytest-parallel.log
            COLUMNS=110 ${PYTHON} -u mpi_tests.py "${JUNITFILE}" "${PYTEST_OPTS}" 2>&1 \
                | sed -u 's/\x1b\[[0-9;]*[a-zA-Z]//g' \
                | grep --line-buffered -v "========" | grep --line-buffered -v "generated xml file" \
                | grep --line-buffered -v '^$' \
                | sed -u 's/^/  /' | tee -a "${TEST_LOGFILE}"

            PYNEST_TEST_TOTAL=$(($(read_junitxml ${JUNITFILE} "tests") ${PYNEST_TEST_TOTAL}))
            PYNEST_TEST_SKIPPED=$(($(read_junitxml ${JUNITFILE} "skips") ${PYNEST_TEST_SKIPPED}))
            PYNEST_TEST_FAILURES=$(($(read_junitxml ${JUNITFILE} "failures") ${PYNEST_TEST_FAILURES}))
            PYNEST_TEST_ERRORS=$(($(read_junitxml ${JUNITFILE} "errors") ${PYNEST_TEST_ERRORS}))
        fi

        cd - >/dev/null

        PYNEST_TEST_FAILED=$(($PYNEST_TEST_ERRORS + $PYNEST_TEST_FAILURES))
        PYNEST_TEST_PASSED=$(($PYNEST_TEST_TOTAL - $PYNEST_TEST_SKIPPED - $PYNEST_TEST_FAILED))

        PYNEST_TEST_SKIPPED_TEXT="(${PYNEST_TEST_SKIPPED} PyNEST)"
        PYNEST_TEST_FAILED_TEXT="(${PYNEST_TEST_FAILED} PyNEST)"

    else
        echo "  Not running PyNEST tests because NEST was compiled without support"
        echo "  for Python. See the file README.md for details."
    fi
}

phase_eight() {
    echo
    echo "Phase 8: C++ tests (experimental)"
    echo "---------------------------------"

    if command -v ${TEST_BASEDIR}/testsuite/cpptests/run_all_cpptests > /dev/null 2>&1; then

        CPP_TEST_OUTPUT="$(${TEST_BASEDIR}/testsuite/cpptests/run_all_cpptests 2>&1)"
        # TODO:
        # We should check the return code ($?) of run_all_cpptests here to see
        # if a fatal crash occured. We cannot simply test $? != 0, since
        # run_all_cpptests will return a non-zero code if tests fail.

        # TODO:
        # The regex for the total number seems fine according to
        # https://www.boost.org/doc/libs/1_65_0/libs/test/doc/html/boost_test/test_output/log_formats/log_human_readable_format.html
        # but does the check for failures work as it should?
        # At some point, we should also count skipped tests.
        CPP_TEST_TOTAL=$(echo "$CPP_TEST_OUTPUT" | sed -${EXTENDED_REGEX_PARAM} -n 's/.*Running ([0-9]+).*/\1/p')
        CPP_TEST_FAILED=$(echo "$CPP_TEST_OUTPUT" | sed -${EXTENDED_REGEX_PARAM} -n 's/.*([0-9]+) failure.*/\1/p')

        # replace empty strings by zero so arithmetic expressions below work
        CPP_TEST_TOTAL=${CPP_TEST_TOTAL:-0}
        CPP_TEST_FAILED=${CPP_TEST_FAILED:-0}
        CPP_TEST_PASSED=$(( $CPP_TEST_TOTAL - $CPP_TEST_FAILED ))
        CPP_TEST_SKIPPED=0

        echo "  C++ tests : ${CPP_TEST_TOTAL}"
        echo "     Passed : ${CPP_TEST_PASSED}"
        echo "     Failed : ${CPP_TEST_FAILED}"
    else
        echo "  Not running C++ tests because NEST was compiled without Boost."
    fi
}

phase_one
phase_two
phase_three
phase_four
phase_five
phase_six
phase_seven
phase_eight

echo
echo "NEST Testsuite Summary"
echo "----------------------"
echo "  NEST Executable: $(which nest)"
echo "  SLI Executable : $(which sli)"
echo
echo "  Total number of tests: $(( $(cat "${TEST_TOTAL}" | wc -l) + $CPP_TEST_TOTAL + $PYNEST_TEST_TOTAL))"
echo "     Passed: $(( $(cat "${TEST_PASSED}" | wc -l) + $CPP_TEST_PASSED + $PYNEST_TEST_PASSED ))"
echo "     Skipped: $(( $(cat "${TEST_SKIPPED}" | wc -l) + $CPP_TEST_SKIPPED + $PYNEST_TEST_SKIPPED )) ${PYNEST_TEST_SKIPPED_TEXT:-}"
echo "     Failed: $(( $(cat "${TEST_FAILED}" | wc -l) + $CPP_TEST_FAILED + $PYNEST_TEST_FAILED )) ${PYNEST_TEST_FAILED_TEXT:-}"
echo

if test "$(cat "${TEST_FAILED}" | wc -l)" -gt 0 || test $CPP_TEST_FAILED -gt 0 || test $PYNEST_TEST_FAILED -gt 0; then
    echo "***"
    echo "*** There were errors detected during the run of the NEST test suite!"
    echo "***"
    echo "*** Please report the problem at"
    echo "***     https://github.com/nest/nest-simulator/issues"
    echo "***"
    echo "*** To help us diagnose the problem, please attach the archived content"
    echo "*** of these directories to the issue:"
    echo "***     - '${REPORTDIR}'"
    echo "***"
    echo
else
    rm -rf "${TEST_TMPDIR}"
fi

# Mac OS X: Restore old crash reporter state
if test "x${INFO_OS}" = xDarwin ; then
    defaults write com.apple.CrashReporter DialogType "${TEST_CRSTATE}"
fi

if test "$(cat "${TEST_FAILED}" | wc -l)" -gt 0 ; then
    exit 1
else
    exit 0
fi
