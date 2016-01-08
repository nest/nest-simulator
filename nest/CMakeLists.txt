set( nest_sources
    neststartup.cpp neststartup.h
    sli_neuron.h sli_neuron.cpp
    )

add_executable( nest main.cpp ${nest_sources} )

add_library( nest_lib ${nest_sources} )

target_link_libraries( nest
    nestutil nestkernel random sli_lib ${SLI_MODULES} ${EXTERNAL_MODULE_LIBRARIES} )

target_link_libraries( nest_lib
    nestutil nestkernel random sli_lib ${SLI_MODULES} ${EXTERNAL_MODULE_LIBRARIES} )

target_include_directories( nest PRIVATE
    ${PROJECT_BINARY_DIR}/nest
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${SLI_MODULE_INCLUDE_DIRS}
    )

target_include_directories( nest_lib PRIVATE
    ${PROJECT_BINARY_DIR}/nest
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${SLI_MODULE_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    )

if ( HAVE_PYTHON )
  target_compile_definitions( nest_lib PRIVATE
      -D_IS_PYNEST
      )
endif ( HAVE_PYTHON )

install( TARGETS nest nest_lib
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )
