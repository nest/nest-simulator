set(random_sources
  librandom_exceptions.h
  exp_randomdev.h
  knuthlfg.h knuthlfg.cpp
  mt19937.h mt19937.cpp
  random_datums.h
  random_numbers.h random_numbers.cpp
  randomgen.h randomgen.cpp
  binomial_randomdev.h binomial_randomdev.cpp
  exp_randomdev.cpp
  gamma_randomdev.h gamma_randomdev.cpp
  normal_randomdev.h normal_randomdev.cpp
  lognormal_randomdev.h lognormal_randomdev.cpp
  poisson_randomdev.h poisson_randomdev.cpp
  randomdev.h randomdev.cpp
  clipped_randomdev.h
  uniform_randomdev.h uniform_randomdev.cpp
  uniformint_randomdev.h uniformint_randomdev.cpp
  gslrandomgen.h gslrandomgen.cpp
  gsl_binomial_randomdev.h gsl_binomial_randomdev.cpp
  random.h random.cpp
)

add_library(random ${random_sources} )
target_link_libraries(random sli_lib nestutil)

target_include_directories(random PRIVATE
  ${PROJECT_SOURCE_DIR}/libnestutil
  ${PROJECT_BINARY_DIR}/libnestutil
  ${PROJECT_SOURCE_DIR}/sli
)

if(MPI_COMPILE_FLAGS)
  set_target_properties(random PROPERTIES
    COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(random PROPERTIES
    LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
endif()

install(TARGETS random
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
