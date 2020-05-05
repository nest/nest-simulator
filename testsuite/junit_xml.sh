

#
# portable_inplace_sed file_name expression
#
portable_inplace_sed ()
{
    cp -f "$1" "$1.XXX"
    sed -e "$2" "$1.XXX" > "$1"
    rm -f "$1.XXX"
}

#
# measure runtime of command
#
time_cmd()
{
    start=`date +%s%N`
    $1
    end=`date +%s%N`
    echo `awk "BEGIN {print ($end - $start) / 1000000000}"`
}

JUNIT_FILE=
JUNIT_TESTS=
JUNIT_FAILURES=
JUNIT_CLASSNAME='core'

TIME_TOTAL=0
TIME_ELAPSED=0

#
# junit_open file_name
#
junit_open ()
{
    if test "x$1" = x ; then
        bail_out 'junit_open: file_name not given!'
    fi

    JUNIT_FILE="${TEST_OUTDIR}/TEST-$1.xml"
    JUNIT_TESTS=0
    JUNIT_FAILURES=0

    TIME_TOTAL=0

    # Be compatible with BSD date; no --rfc-3339 and :z modifier
    timestamp="$( date -u '+%FT%T+00:00' )"

    echo '<?xml version="1.0" encoding="UTF-8" ?>' > "${JUNIT_FILE}"

    echo "<testsuite errors=\"0\" failures=XXX hostname=\"${INFO_HOST}\" name=\"$1\" tests=XXX time=XXX timestamp=\"${timestamp}\">" >> "${JUNIT_FILE}"
    echo '  <properties>' >> "${JUNIT_FILE}"
    echo "    <property name=\"os.arch\" value=\"${INFO_ARCH}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"os.name\" value=\"${INFO_OS}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"os.version\" value=\"${INFO_VER}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"user.home\" value=\"${INFO_HOME}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"user.name\" value=\"${INFO_USER}\" />" >> "${JUNIT_FILE}"
    echo '  </properties>' >> "${JUNIT_FILE}"
}

#
# junit_write classname testname [fail_message fail_trace]
#
junit_write ()
{
    if test "x${JUNIT_FILE}" = x ; then
        bail_out 'junit_write: report file not open, call junit_open first!'
    fi

    if test "x$1" = x || test "x$2" = x ; then
        bail_out 'junit_write: classname and testname arguments are mandatory!'
    fi

    JUNIT_TESTS=$(( ${JUNIT_TESTS} + 1 ))

    printf '%s' "  <testcase classname=\"$1\" name=\"$2\" time=\"${TIME_ELAPSED}\"" >> "${JUNIT_FILE}"

    if test "x$3" != x ; then
        echo '>' >> "${JUNIT_FILE}"
        echo "    <failure message=\"$3\" type=\"\"><![CDATA[" >> "${JUNIT_FILE}"
        echo "$4" | sed 's/]]>/]]>]]\&gt;<![CDATA[/' >> "${JUNIT_FILE}"
        echo "]]></failure>" >> "${JUNIT_FILE}"
        echo "  </testcase>" >> "${JUNIT_FILE}"
    else
        echo ' />' >> "${JUNIT_FILE}"
    fi
}

#
# junit_close
#
junit_close ()
{
    if test "x${JUNIT_FILE}" = x ; then
        bail_out 'junit_close: report file not open, call junit_open first!'
    fi

    portable_inplace_sed "${JUNIT_FILE}" "s/time=XXX/time=\"${TIME_TOTAL}\"/"
    portable_inplace_sed "${JUNIT_FILE}" "s/tests=XXX/tests=\"${JUNIT_TESTS}\"/"
    portable_inplace_sed "${JUNIT_FILE}" "s/failures=XXX/failures=\"${JUNIT_FAILURES}\"/"

    echo '  <system-out><![CDATA[]]></system-out>' >> "${JUNIT_FILE}"
    echo '  <system-err><![CDATA[]]></system-err>' >> "${JUNIT_FILE}"
    echo '</testsuite>' >> "${JUNIT_FILE}"

    JUNIT_FILE=
}
