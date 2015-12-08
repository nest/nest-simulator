
# add selftests files
#file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)

foreach(script test_pass.sli test_goodhandler.sli test_lazyhandler.sli)
  add_test(NAME selftests/${script}
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/nest
    ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/selftests/${script} # use the installed version
  )
endforeach()

foreach(script test_fail.sli test_stop.sli test_badhandler.sli)
  add_test(NAME selftests/${script}
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/nest
    ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/selftests/${script} # use the installed version
  )
  set_tests_properties(selftests/${script}
    PROPERTIES WILL_FAIL TRUE)
endforeach()

foreach(script test_pass_or_die.sli test_assert_or_die_b.sli test_assert_or_die_p.sli test_fail_or_die.sli test_crash_or_die.sli test_failbutnocrash_or_die_crash.sli test_failbutnocrash_or_die_pass.sli test_passorfailbutnocrash_or_die.sli)
  add_test(NAME selftests/${script}
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/nest
    ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/selftests/${script} # use the installed version
  )
  set_tests_properties(selftests/${script}
    PROPERTIES WILL_FAIL TRUE)
endforeach()
