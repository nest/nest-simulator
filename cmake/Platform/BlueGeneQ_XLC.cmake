include(BlueGeneQ_Base)
# set the compiler
set(CMAKE_C_COMPILER bgxlc_r)
set(CMAKE_CXX_COMPILER bgxlc++_r)

#
# Compile flags for different build types
#
set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Compiler optimization flags")
set(CMAKE_C_FLAGS_RELEASE "-O3 -qstrict -qarch=qp -qtune=qp -DNDEBUG" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -qstrict -qarch=qp -qtune=qp -DNDEBUG" CACHE STRING "Compiler optimization flags")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -qarch=qp -qtune=qp -DNDEBUG" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -qarch=qp -qtune=qp -DNDEBUG" CACHE STRING "Compiler optimization flags")

set(OpenMP_C_FLAGS "-qsmp=omp" CACHE STRING "Compiler flag for OpenMP parallelization" FORCE)
set(OpenMP_CXX_FLAGS "-qsmp=omp" CACHE STRING "Compiler flag for OpenMP parallelization" FORCE)
