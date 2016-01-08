set( sli_sources
    allocator.h allocator.cpp
    aggregatedatum.h
    arraydatum.cc arraydatum.h
    booldatum.cc booldatum.h
    callbackdatum.h
    charcode.cc charcode.h
    datum.cc datum.h
    dict.cc dict.h
    dictdatum.cc dictdatum.h
    dictstack.cc dictstack.h
    dictutils.cc dictutils.h
    doubledatum.cc doubledatum.h
    fdstream.cc fdstream.h
    filesystem.cc filesystem.h
    functional.h
    functiondatum.cc functiondatum.h
    genericdatum.h
    get_mem.c
    gnureadline.cc gnureadline.h
    integerdatum.cc integerdatum.h
    interpret.cc interpret.h
    iostreamdatum.cc iostreamdatum.h
    iteratordatum.h iteratordatum.cc
    literaldatum.cc
    lockptrdatum.h lockptrdatum_impl.h
    name.cc name.h
    slinames.h slinames.cpp
    namedatum.cc namedatum.h
    numericdatum.h numericdatum_impl.h
    oosupport.cc oosupport.h
    parser.cc parser.h
    parserdatum.h
    processes.cc processes.h
    psignal.c psignal.h
    scanner.cc scanner.h
    sli_io.cc sli_io.h
    sliactions.cc sliactions.h
    sliarray.cc sliarray.h
    slibuiltins.cc slibuiltins.h
    slicontrol.cc slicontrol.h
    slidata.cc slidata.h
    slidict.cc slidict.h
    sliexceptions.cc sliexceptions.h
    slifunction.h
    sligraphics.cc sligraphics.h
    slimath.cc slimath.h
    slimodule.cc slimodule.h
    sliregexp.cc sliregexp.h
    slistack.cc slistack.h
    slistartup.cc slistartup.h
    slitype.cc slitype.h
    slitypecheck.cc slitypecheck.h
    specialfunctionsmodule.cc specialfunctionsmodule.h
    stringdatum.cc stringdatum.h
    symboldatum.cc symboldatum.h
    tarrayobj.cc tarrayobj.h
    token.cc token.h
    tokenarray.cc tokenarray.h
    tokenstack.cc tokenstack.h
    tokenutils.cc tokenutils.h
    triedatum.cc triedatum.h
    typearray.h
    typechk.cc typechk.h
    utils.cc utils.h
    )

add_library( sli_lib ${sli_sources} )
target_link_libraries( sli_lib nestutil )

# add the executable
add_executable( sli puresli.cc )
target_link_libraries( sli sli_lib )

target_include_directories( sli PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    )

target_include_directories( sli_lib PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    )

target_compile_definitions( sli_lib PRIVATE
    -DPKGDATADIR=\"${PKGDATADIR}\"
    -DPKGDOCDIR=\"${PKGDOCDIR}\"
    -DPKGSOURCEDIR=\"${PROJECT_SOURCE_DIR}\"
    )

install( TARGETS sli_lib sli
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )

# DIRS for help
# TODO limit to used source folders
# base dirs + dirs/sli
file( GLOB children ${PROJECT_SOURCE_DIR}/* )
set( HELPDIRS "" )
foreach ( child ${children} )
  if ( IS_DIRECTORY ${child} )
    set( HELPDIRS "${HELPDIRS}${child}:" )
    # Automatically include sli directory if it exists
    if ( IS_DIRECTORY ${child}/sli )
      set( HELPDIRS "${HELPDIRS}${child}/sli:" )
    endif ()
  endif ()
endforeach ()

# testsuite dirs
file( GLOB children ${PROJECT_SOURCE_DIR}/testsuite/*/ )
foreach ( child ${children} )
  if ( IS_DIRECTORY ${child} )
    set( HELPDIRS "${HELPDIRS}${child}:" )
  endif ()
  if ( IS_DIRECTORY ${child}/pass )
    set( HELPDIRS "${HELPDIRS}${child}/pass:" )
  endif ()
  if ( IS_DIRECTORY ${child}/fail )
    set( HELPDIRS "${HELPDIRS}${child}/fail:" )
  endif ()
endforeach ()

# install help (depends on sli and lib/sli/* installed)
install( CODE
    "execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PKGDOCDIR}/help/sli
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PKGDOCDIR}/help/cc
      COMMAND ${CMAKE_COMMAND} -E remove ${PKGDOCDIR}/help/helpindex.html
      COMMAND ${CMAKE_COMMAND} -E remove ${PKGDOCDIR}/help/helpindex.hlp

      COMMAND ${CMAKE_COMMAND} -E make_directory ${PKGDOCDIR}/help
      COMMAND ${CMAKE_COMMAND}
          -DDOC_DIR='${PKGDOCDIR}'
          -DDATA_DIR='${PKGDATADIR}'
          -DHELPDIRS='${HELPDIRS}'
          -DINSTALL_DIR='${CMAKE_INSTALL_PREFIX}'
          -P ${PROJECT_SOURCE_DIR}/cmake/generate_help.cmake
        WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\"
      )"
    )

install( FILES ${PROJECT_SOURCE_DIR}/cmake/generate_help.cmake
    DESTINATION ${INSTALL_DOC_DIR} )
