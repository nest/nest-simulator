set(conngen_sources
  conngenmodule.h
  conngenmodule.cpp
  cg_connect.h
  cg_connect.cpp
  conngendatum.h conngendatum.cpp
  conngen.h conngen.cpp
)

add_library(conngen ${conngen_sources})

target_link_libraries(conngen nestutil sli_lib nestkernel)

target_include_directories(conngen PRIVATE
  ${PROJECT_BINARY_DIR}/libnestutil
  ${PROJECT_SOURCE_DIR}/libnestutil
  ${PROJECT_SOURCE_DIR}/nestkernel
  ${PROJECT_SOURCE_DIR}/sli
  ${PROJECT_SOURCE_DIR}/librandom
)

install(TARGETS conngen
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

install(DIRECTORY sli
  DESTINATION ${INSTALL_DATA_DIR})
