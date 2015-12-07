set( EXTRAS_DOC
  bibliography/nest.bib
  logos/nest.pdf
  logos/nest.png
  logos/nest.svg
  logos/nest-initiative.pdf
  logos/nest-initiative.png
  logos/nest-initiative.svg
  logos/nest-initiative-www.pdf
  logos/nest-initiative-www.png
  logos/nest-initiative-www.svg
  logos/nest-simulated.pdf
  logos/nest-simulated.png
  logos/nest-simulated.svg
  logos/nest-simulated-www.pdf
  logos/nest-simulated-www.png
  logos/nest-simulated-www.svg
  )

set( EXTRAS_EXTRA_DIST README.md )

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/extras/create_rcsinfo.sh ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
if(EXISTS ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli)
  install(FILES ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli
    DESTINATION ${INSTALL_DATA_DIR}/sli)
endif()

install(DIRECTORY bibliography logos
  DESTINATION ${INSTALL_DOC_DIR})

install(FILES emacs/postscript-sli.el emacs/psvn.el ${CMAKE_CURRENT_BINARY_DIR}/emacs/sli.el
  DESTINATION ${INSTALL_DATA_DIR}/extras/emacs)

install(PROGRAMS
  ${PROJECT_BINARY_DIR}/extras/nest-config
  nest_indirect
  nest_serial
  ${PROJECT_BINARY_DIR}/extras/nest_vars.sh
  DESTINATION ${INSTALL_BIN_DIR})

add_subdirectory(ConnPlotter)
