set(topo_sources
  topologymodule.h
  topologymodule.cpp
  topology_names.h
  topology_names.cpp
  connection_creator.h
  connection_creator_impl.h
  connection_creator.cpp
  generic_factory.h
  position.h
  layer.h
  layer_impl.h
  layer.cpp
  free_layer.h
  grid_layer.h
  mask.h
  mask_impl.h
  grid_mask.h
  ntree.h
  ntree_impl.h
  vose.h
  vose.cpp
  parameter.h
  parameter.cpp
  selector.h
  selector.cpp
  topology.h topology.cpp
)

add_library(topology ${topo_sources})
target_link_libraries(topology nestutil sli_lib random nestkernel)

target_include_directories(topology PRIVATE
  ${PROJECT_SOURCE_DIR}/libnestutil
  ${PROJECT_BINARY_DIR}/libnestutil
  ${PROJECT_SOURCE_DIR}/librandom
  ${PROJECT_SOURCE_DIR}/sli
  ${PROJECT_SOURCE_DIR}/nestkernel
  )

install(TARGETS topology
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

install(FILES sli/topology-interface.sli
  DESTINATION ${INSTALL_DATA_DIR}/sli)

add_subdirectory(doc)
