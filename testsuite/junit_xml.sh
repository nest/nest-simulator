
# junit_xml.sh
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
    t_start=$( date +%s%N )
    $1
    t_end=$( date +%s%N )
    
    # On macOS, `date +%s%N` returns time in seconds followed by N.
    # The following distinguishes which date version was used.
    if test "x${t_start: -1}" != xN ; then     # space before -1 required!
        echo $(( ( ${t_end} - ${t_start} ) / 1000000000 ))
    else
        echo $(( ${t_end%N} - ${t_start%N} ))
    fi
}

#
# bail_out message
#
bail_out ()
{
    echo "$1"
    exit 1
}

JUNIT_FILE=
JUNIT_TESTS=
JUNIT_SKIPS=
JUNIT_FAILURES=

# Gather some information about the host
INFO_ARCH="$(uname -m)"
INFO_OS="$(uname -s)"
INFO_VER="$(uname -r)"

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

    JUNIT_FILE="${REPORTDIR}/$1.xml"
    JUNIT_TESTS=0
    JUNIT_SKIPS=0
    JUNIT_FAILURES=0

    TIME_TOTAL=0

    # Be compatible with BSD date; no --rfc-3339 and :z modifier
    timestamp="$( date -u '+%FT%T+00:00' )"

    echo '<?xml version="1.0" encoding="UTF-8" ?>' > "${JUNIT_FILE}"

    echo "<testsuite errors=\"0\" failures=XXX name=\"$1\" tests=XXX skipped=XXX time=XXX timestamp=\"${timestamp}\">" >> "${JUNIT_FILE}"
    echo '  <properties>' >> "${JUNIT_FILE}"
    echo "    <property name=\"os.arch\" value=\"${INFO_ARCH}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"os.name\" value=\"${INFO_OS}\" />" >> "${JUNIT_FILE}"
    echo "    <property name=\"os.version\" value=\"${INFO_VER}\" />" >> "${JUNIT_FILE}"
    echo '  </properties>' >> "${JUNIT_FILE}"
}

#
# junit_write classname testname status [fail_message fail_trace]
#
junit_write ()
{
    if test "x${JUNIT_FILE}" = x ; then
        bail_out 'junit_write: report file not open, call junit_open first!'
    fi

    if test "x$1" = x || test "x$2" = x ; then
        bail_out 'junit_write: classname and testname arguments are mandatory!'
    fi

    printf '%s' "  <testcase classname=\"$1\" name=\"$2\" time=\"${TIME_ELAPSED}\">" >> "${JUNIT_FILE}"

    if test "x$3" = xskipped ; then
        echo "    <skipped message=\"$4\" type=\"\"></skipped>" >> "${JUNIT_FILE}"
    fi

    if test "x$3" = xfailure ; then
        echo "    <failure message=\"$4\" type=\"\"><![CDATA[" >> "${JUNIT_FILE}"
        echo "$5" | sed 's/]]>/]]>]]\&gt;<![CDATA[/' >> "${JUNIT_FILE}"
        echo "]]></failure>" >> "${JUNIT_FILE}"
    fi
    echo "  </testcase>" >> "${JUNIT_FILE}"
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
    portable_inplace_sed "${JUNIT_FILE}" "s/skipped=XXX/skipped=\"${JUNIT_SKIPS}\"/"
    portable_inplace_sed "${JUNIT_FILE}" "s/failures=XXX/failures=\"${JUNIT_FAILURES}\"/"

    echo '  <system-out><![CDATA[]]></system-out>' >> "${JUNIT_FILE}"
    echo '  <system-err><![CDATA[]]></system-err>' >> "${JUNIT_FILE}"
    echo '</testsuite>' >> "${JUNIT_FILE}"

    JUNIT_FILE=
}
