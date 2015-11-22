set(DOC_DATA 
  ../LICENSE
)
set(DOC_EXTRA_DIST
  doc_header.txt README.md
)
set(nobase_data_DATA
  conngen/conngen.txt
  index.html
  quickref.html
  doc_header.txt
  model_details/noise_generator.ipynb
)

if(DOXYGEN_FOUND)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/set_rcsinfo.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/set_rcsinfo.cmake" @ONLY
  )

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/normaldoc.conf.in"
    "${CMAKE_CURRENT_BINARY_DIR}/normaldoc.conf" @ONLY
  )

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/fulldoc.conf.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf" @ONLY
  )

  add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/set_rcsinfo.cmake
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/normaldoc.conf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  if(DOXYGEN_DOT_FOUND)
    add_custom_target(fulldoc
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/set_rcsinfo.cmake
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif(DOXYGEN_DOT_FOUND)
endif(DOXYGEN_FOUND)
