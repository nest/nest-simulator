# testsuite/selftests/CMakeLists.txt
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


# add selftests files
#file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)

foreach ( script test_pass.sli test_goodhandler.sli test_lazyhandler.sli )
  add_test( NAME selftests/${script}
      COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/nest
      ${CMAKE_INSTALL_FULL_DOCDIR}/selftests/${script} # use the installed version
      )
endforeach ()

foreach ( script test_fail.sli test_stop.sli test_badhandler.sli )
  add_test( NAME selftests/${script}
      COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/nest
      ${CMAKE_INSTALL_FULL_DOCDIR}/selftests/${script} # use the installed version
      )
  set_tests_properties( selftests/${script}
      PROPERTIES WILL_FAIL TRUE )
endforeach ()

foreach ( script test_pass_or_die.sli test_assert_or_die_b.sli test_assert_or_die_p.sli test_fail_or_die.sli test_crash_or_die.sli test_failbutnocrash_or_die_crash.sli test_failbutnocrash_or_die_pass.sli test_passorfailbutnocrash_or_die.sli )
  add_test( NAME selftests/${script}
      COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/nest
      ${CMAKE_INSTALL_FULL_DOCDIR}/selftests/${script} # use the installed version
      )
  set_tests_properties( selftests/${script}
      PROPERTIES WILL_FAIL TRUE )
endforeach ()
