set( nestutil_sources
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
    string_utils.h
    logging.h
    logging_event.h logging_event.cpp
    )

add_library( nestutil ${nestutil_sources} )

target_link_libraries( nestutil )

target_include_directories( nestutil PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    )

install( TARGETS nestutil
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )
