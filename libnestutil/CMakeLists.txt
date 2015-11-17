message("Hi from libnestutil...")

set(nestutil_sources
  stopwatch.h stopwatch.cpp
  numerics.h numerics.cpp
  propagator_stability.h propagator_stability.cpp
  lockptr.h
  sparseconfig.h
  template_util.h
  type_traits.h
  libc_allocator_with_realloc.h
  hashtable-common.h
  sparsetable.h
  compose.hpp
  logging.h
  logging_event.h logging_event.cpp
)

add_library(nestutil ${nestutil_sources})

target_link_libraries(nestutil)

target_include_directories(nestutil PRIVATE
  ${PROJECT_BINARY_DIR}/libnestutil
)

if(MPI_COMPILE_FLAGS)
  set_target_properties(nestutil PROPERTIES
    COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(nestutil PROPERTIES
    LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
endif()

install(TARGETS nestutil
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
