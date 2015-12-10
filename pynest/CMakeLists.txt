if(HAVE_PYTHON)
  include( UseCython )
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/pynestkernel.pyx
    PROPERTIES CYTHON_IS_CXX TRUE )
  
  cython_add_module( pynestkernel pynestkernel.pyx )
  
  target_link_libraries(pynestkernel
    nest_lib nestutil nestkernel random sli_lib ${SLI_MODULES})
  
  target_include_directories(pynestkernel PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/nest
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${STATIC_MODULES_INDCLUDE_DIRS}
    ${LIBNEUROSIM_INCLUDE_DIR}
  )
  
  target_compile_definitions(pynestkernel PRIVATE 
    -D_IS_PYNEST
  )
  
  install(CODE "execute_process(
    COMMAND ${PYTHON} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
                               install --prefix=${CMAKE_INSTALL_PREFIX}
                                       --install-lib=${PYEXECDIR}
                                       --install-scripts=${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}
                                       --install-data=${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})"
  )
  install(TARGETS pynestkernel DESTINATION ${PYEXECDIR}/nest/)
  install(FILES ${PROJECT_BINARY_DIR}/pynest/do_tests.py
    DESTINATION ${INSTALL_DATA_DIR}/extras)

  install(DIRECTORY examples/
    DESTINATION ${INSTALL_DOC_DIR}/examples/pynest)

  find_program (NOSETESTS nosetests)
  if(NOT NOSETESTS STREQUAL "NOSETESTS-NOTFOUND")
    add_test(NAME PyNEST
      COMMAND ${NOSETESTS} -v --with-xunit 
                              --xunit-file=${PROJECT_BINARY_DIR}/reports/pynest_tests.xml 
                              ${PYEXECDIR}/nest/tests)
  endif()
endif(HAVE_PYTHON)
