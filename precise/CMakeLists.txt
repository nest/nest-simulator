message("Hi from precise...")

set(precise_sources
  slice_ring_buffer.cpp   slice_ring_buffer.h
  iaf_psc_delta_canon.cpp iaf_psc_delta_canon.h
  iaf_psc_alpha_canon.cpp iaf_psc_alpha_canon.h
  iaf_psc_alpha_presc.cpp iaf_psc_alpha_presc.h
  iaf_psc_exp_ps.cpp      iaf_psc_exp_ps.h
  poisson_generator_ps.cpp poisson_generator_ps.h
  parrot_neuron_ps.cpp parrot_neuron_ps.h
  precisemodule.cpp precisemodule.h
)

add_library(precise ${precise_sources})
target_link_libraries(precise 
  nestutil sli_lib nestkernel)

target_include_directories(precise PRIVATE
  ${PROJECT_SOURCE_DIR}/libnestutil
  ${PROJECT_BINARY_DIR}/libnestutil
  ${PROJECT_SOURCE_DIR}/librandom
  ${PROJECT_SOURCE_DIR}/sli
  ${PROJECT_SOURCE_DIR}/nestkernel
)

if(MPI_COMPILE_FLAGS)
  set_target_properties(precise PROPERTIES
    COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(precise PROPERTIES
    LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
endif()

install(TARGETS precise
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
