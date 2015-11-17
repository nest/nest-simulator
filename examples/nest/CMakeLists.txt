message("Hi from examples/nest...")

set(EXAMPLES_NEST_DATA_DIRS
  Brette_et_al_2007
  music
  Potjans_2014
  )

set(EXAMPLES_NEST_DATA_FILES
  balancedneuron.sli
  balancedneuron-2.sli
  brunel-2000.sli
  brunel_ps.sli
  hpc_benchmark.sli
  multimeter.sli
  )

install(DIRECTORY ${EXAMPLES_NEST_DATA_DIRS}
  DESTINATION ${INSTALL_DOC_DIR}/examples
  USE_SOURCE_PERMISSIONS)

install(FILES ${EXAMPLES_NEST_DATA_FILES}
  DESTINATION ${INSTALL_DOC_DIR}/examples
  )

