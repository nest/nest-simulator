
execute_process( COMMAND ${PROJECT_SOURCE_DIR}/extras/create_rcsinfo.sh ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} )
if ( EXISTS ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli )
  install( FILES ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli
      DESTINATION ${INSTALL_DATA_DIR}/sli )
endif ()

install( DIRECTORY bibliography logos
    DESTINATION ${INSTALL_DOC_DIR} )

install( FILES emacs/postscript-sli.el emacs/psvn.el ${CMAKE_CURRENT_BINARY_DIR}/emacs/sli.el
    DESTINATION ${INSTALL_DATA_DIR}/extras/emacs )

install( PROGRAMS
    ${PROJECT_BINARY_DIR}/extras/nest-config
    nest_indirect
    nest_serial
    ${PROJECT_BINARY_DIR}/extras/nest_vars.sh
    DESTINATION ${INSTALL_BIN_DIR} )

add_subdirectory( ConnPlotter )
