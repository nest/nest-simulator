cmake_minimum_required(VERSION 3.4)

# add cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Platform)

project(nest CXX C)

# install directories
set(INSTALL_BIN_DIR bin)
set(INSTALL_LIB_DIR lib)
set(INSTALL_DATA_DIR share/nest)
set(INSTALL_DOC_DIR share/doc/nest)

set(PACKAGE nest)

# VERSION exported to libnestutil/sliconfig.h
set(SLI_MAJOR 2)
set(SLI_MINOR 8)
set(SLI_PATCHLEVEL 0-git)

set(SLI_VERSION "${SLI_MAJOR}.${SLI_MINOR}.${SLI_PATCHLEVEL}")
set(SLI_PRGNAME "${PACKAGE}-${SLI_VERSION}")

set(SLI_PREFIX ${CMAKE_INSTALL_PREFIX})

# on OS X
set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

set(PKGSRCDIR ${PROJECT_SOURCE_DIR})
set(PKGBUILDDIR ${PROJECT_BINARY_DIR})
set(PKGDATADIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR})
set(PKGDOCDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR})
set(TESTSUITE_BASEDIR ${PKGDOCDIR})


include(CheckIncludeFiles)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(mach/mach.h HAVE_MACH_MACH_H)
check_include_files(mach-o/dyld.h HAVE_MACH_O_DYLD_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(signal.h HAVE_MEMORY_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)

include(CheckIncludeFileCXX)
check_include_file_cxx(sstream HAVE_SSTREAM)
check_include_file_cxx(istream HAVE_ISTREAM)
check_include_file_cxx(ostream HAVE_OSTREAM)

include(CheckTypeSize)
CHECK_TYPE_SIZE("long long" LONG_LONG_SIZE) # also sets HAVE_LONG_LONG
if( LONG_LONG_SIZE GREATER 0 )
  set(HAVE_LONG_LONG ON)
endif()

include(CheckSymbolExists)
check_symbol_exists(NAN "math.h" HAVE_NAN)

include(CheckCXXSymbolExists)
check_cxx_symbol_exists(M_E "cmath" HAVE_M_E)
check_cxx_symbol_exists(M_PI "cmath" HAVE_M_PI)

include(CheckFunctionExists)
check_function_exists(isnan "math.h" HAVE_ISNAN)
check_function_exists(expm1 "math.h" HAVE_EXPM1)
check_function_exists(fork "unistd.h" HAVE_FORK)
check_function_exists(execvp "unistd.h" HAVE_EXECVP)
check_function_exists(opendir "dirent.h" HAVE_OPENDIR)


set(ENABLE_BLUEGENE OFF CACHE STRING "Configure for Blue Gene; the specific BG model must be given as argument (=L/P/Q)." FORCE)
option(K_COMPUTER "Enable K computer." OFF)
set(TICS_PER_MS "1000.0" CACHE STRING "Specify elementary unit of time." FORCE)
set(TICS_PER_STEP "100" CACHE STRING "Specify resolution." FORCE)
option(WITH_MPI "Request compilation with MPI; optionally give directory with MPI installation." OFF)
option(LIBNEUROSIM "Request the use of libneurosim; optionally give the directory, where libneurosim is installed." OFF)
set(WITH_OPTIMIZE OFF CACHE STRING "Enable user defined optimizations." FORCE)
set(WITH_WARNING OFF CACHE STRING "Enable user defined warnings." FORCE)
option(GSL_PREFIX "Prefix where GSL is installed. (optional)" OFF)
option(WITH_READLINE "Find a readline library [default=ON]. To ignore readline set to OFF. To set a specific readline, set install path." ON)
option(WITHOUT_OPENMP "Disable OpenMP multithreading." OFF)
option(WITH_PYTHON "Find a Python interpreter [default=ON]. To ignore the Python set to OFF. To set a specific Python, set path." ON)
option(EXTERNAL_MODULES "External modules to be linked in. Devided by ; ." OFF)
option(STATIC_LIBRARIES "Build static libraries. [default=no]" OFF)
option(PS_ARRAYS "Use PS array construction semantics" ON)

# not defined yet
option(WITH_SCALI "Request compilation with Scali MPI; optionally give directory with Scali installation.[/opt/scali]" OFF)
option(WITH_MUSIC "Request the use of MUSIC; optionally give the directory, where MUSIC is installed." OFF)

if(WITH_OPTIMIZE)
  list(APPEND CMAKE_C_FLAGS ${WITH_OPTIMIZE})
  list(APPEND CMAKE_CXX_FLAGS ${WITH_OPTIMIZE})
endif(WITH_OPTIMIZE)

if(WITH_WARNING)
  list(APPEND CMAKE_C_FLAGS ${WITH_WARNING})
  list(APPEND CMAKE_CXX_FLAGS ${WITH_WARNING})
endif(WITH_WARNING)

if(K_COMPUTER)
  set(IS_K ON)
endif(K_COMPUTER)

if(ENABLE_BLUEGENE)
  # use the -DCMAKE_TOOLCHAIN_FILE= argument
  if(${ENABLE_BLUEGENE} STREQUAL "L")
    set(IS_BLUEGENE_L ON)
    #include(Platform/BlueGeneL-static)
  elseif(${ENABLE_BLUEGENE} STREQUAL "P")
    set(IS_BLUEGENE_P ON)
    #include(Platform/BlueGeneP-static)
  elseif(${ENABLE_BLUEGENE} STREQUAL "Q")
    set(IS_BLUEGENE_Q ON)
    #include(Platform/BlueGeneQ-static)
  else()
    message(SEND_ERROR "Only L/P/Q is allowed for ENABLE_BLUEGENE.")
  endif()
  set(IS_BLUEGENE ON)
  set(STATIC_LIBRARIES ON)
  
  # not sure if this is working
  set(WITH_READLINE OFF)
  set(WITH_PYTHON OFF)
endif(ENABLE_BLUEGENE)

find_package(LTDL)
if(LTDL_FOUND)
  set(HAVE_LIBLTDL ON)
  message("-- LTDL_INCLUDE_DIR: ${LTDL_INCLUDE_DIR}")
  message("-- LTDL_LIBRARIES: ${LTDL_LIBRARIES}")
  include_directories(${LTDL_INCLUDE_DIR})
  link_libraries(${LTDL_LIBRARIES})
endif(LTDL_FOUND)


if( STATIC_LIBRARIES )
  set(BUILD_SHARED_LIBS OFF)
else( STATIC_LIBRARIES )
  set(BUILD_SHARED_LIBS ON)
endif( STATIC_LIBRARIES)

# Static modules
set(SLI_MODULES models precise topology)
if( LIBNEUROSIM )
  set(HAVE_LIBNEUROSIM ON)
  set(SLI_MODULES ${SLI_MODULES} conngen)
endif( LIBNEUROSIM )

set(STATIC_MODULES_INDCLUDE_DIRS)
foreach(mod ${SLI_MODULES})
  set(STATIC_MODULES_INDCLUDE_DIRS ${STATIC_MODULES_INDCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/${mod} )
endforeach(mod)

if( STATIC_LIBRARIES )
  # TODO include path?
  # TODO lib path?
  if( EXTERNAL_MODULES )
    foreach(mod ${EXTERNAL_MODULES})
      set(SLI_MODULES ${SLI_MODULES} ${mod})
    endforeach(mod)
  endif( EXTERNAL_MODULES )
else( STATIC_LIBRARIES )
  # TODO dynamic linking stuff
endif( STATIC_LIBRARIES )

file(WRITE ${PROJECT_BINARY_DIR}/nest/static_modules.h "#ifndef STATIC_MODULES_H\n")
file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#define STATIC_MODULES_H\n\n")
foreach(mod ${SLI_MODULES})
  file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#include \"${mod}module.h\"\n")
endforeach(mod)
file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "\n#include \"interpret.h\"\n\n")
file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "void add_static_modules(SLIInterpreter& engine)\n{\n")
foreach(mod ${SLI_MODULES})
  #mod is the lower-case string
  string(SUBSTRING ${mod} 0 1 FIRST_LETTER)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
  string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" Mod "${mod}")
  file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "  engine.addmodule(new nest::${Mod}Module());\n")
endforeach(mod)
file(APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "}\n\n#endif\n")
# Static modules

if( TICS_PER_MS )
  set(HAVE_TICS_PER_MS ON)
endif( TICS_PER_MS )

if( TICS_PER_STEP )
  set(HAVE_TICS_PER_STEP ON)
endif( TICS_PER_STEP )

# !!!GSL finding!!!

if( GSL_PREFIX )
  # use this prefix
  set(GSL_ROOT_DIR ${GSL_PREFIX})
endif( GSL_PREFIX )

find_package(GSL)

if( GSL_FOUND )
  set(HAVE_GSL ON)

  if("${GSL_VERSION}" VERSION_GREATER "1.2")
    set(HAVE_GSL_1_2 ON)
  endif()

  if("${GSL_VERSION}" VERSION_GREATER "1.11")
    set(HAVE_GSL_1_11 ON)
  endif()
  include_directories(${GSL_INCLUDE_DIRS})
  link_libraries(${GSL_LIBRARIES})
endif( GSL_FOUND )

# !!!GSL finding!!!

# Python executable finding

if(WITH_PYTHON)
  if (NOT ${WITH_PYTHON} STREQUAL "ON")
    # a path is set
    set(PYTHON_EXECUTABLE ${WITH_PYTHON})
  endif()

  find_package(PythonInterp)
  if(PYTHONINTERP_FOUND)
    set(HAVE_PYTHON ON)
    set(PYTHON ${PYTHON_EXECUTABLE})
    set(PYTHON_VERSION ${PYTHON_VERSION_STRING})
  endif()

  find_package(Cython)

  set(PYEXECDIR lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
else(WITH_PYTHON)
  set(HAVE_PYTHON OFF)
endif(WITH_PYTHON)

# Python executable finding

# Find readline

if(WITH_READLINE)
  if (NOT ${WITH_READLINE} STREQUAL "ON")
    # a path is set
    set(READLINE_ROOT_DIR ${WITH_READLINE})
  endif()

  find_package(Readline)
  if(READLINE_FOUND)
    set(HAVE_READLINE ON)
  endif(READLINE_FOUND)

  include_directories(${READLINE_INCLUDE_DIRS})
  link_libraries(${READLINE_LIBRARIES})
else(WITH_READLINE)
  set(HAVE_READLINE OFF)
endif(WITH_READLINE)

# Find readline

# OPENMP finding
if(NOT WITHOUT_OPENMP)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    list(APPEND CMAKE_C_FLAGS ${OpenMP_C_FLAGS})
    list(APPEND CMAKE_CXX_FLAGS ${OpenMP_CXX_FLAGS})
  endif(OPENMP_FOUND)
endif(NOT WITHOUT_OPENMP)

# MPI finding
if(WITH_MPI)
  find_package(MPI)
  if(MPI_CXX_FOUND)
    set(HAVE_MPI ON)
  endif()

  list(APPEND CMAKE_C_FLAGS ${MPI_C_COMPILE_FLAGS})
  list(APPEND CMAKE_CXX_FLAGS ${MPI_CXX_COMPILE_FLAGS})
  
  list(APPEND CMAKE_EXE_LINKER_FLAGS ${MPI_CXX_LINK_FLAGS})
  include_directories(${MPI_CXX_INCLUDE_PATH})
  link_libraries(${MPI_CXX_LIBRARIES})
endif(WITH_MPI)


add_subdirectory(cmake)
add_subdirectory(conngen)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(extras)
add_subdirectory(lib)
add_subdirectory(libnestutil)
add_subdirectory(librandom)
add_subdirectory(models)
add_subdirectory(sli)
add_subdirectory(nest)
add_subdirectory(nestkernel)
add_subdirectory(precise)
add_subdirectory(pynest)
add_subdirectory(testsuite)
add_subdirectory(topology)
#add_subdirectory($SLI_MODULES)
#add_subdirectory(libltdl)

configure_file(
  "${PROJECT_SOURCE_DIR}/libnestutil/config.h.in"
  "${PROJECT_BINARY_DIR}/libnestutil/config.h"
)

configure_file(
  "${PROJECT_SOURCE_DIR}/libnestutil/sliconfig.h.in"
  "${PROJECT_BINARY_DIR}/libnestutil/sliconfig.h"
)

configure_file(
  "${PROJECT_SOURCE_DIR}/pynest/setup.py.in"
  "${PROJECT_BINARY_DIR}/pynest/setup.py"
)

configure_file(
  "${PROJECT_SOURCE_DIR}/pynest/do_tests.py.in"
  "${PROJECT_BINARY_DIR}/pynest/do_tests.py"
)

add_custom_target(installcheck
  COMMAND cmake --build ${PROJECT_BINARY_DIR} --target install
  COMMAND export PATH="${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}"
  COMMAND export PYTHON="${PYTHON}"
  COMMAND export PYTHONPATH="${CMAKE_INSTALL_PREFIX}${PYEXECDIR}:$(PYTHONPATH)"
  COMMAND export exec_prefix="${CMAKE_INSTALL_PREFIX}"
  COMMAND
    ${PKGDATADIR}/extras/do_tests.sh --test-pynest --source-dir=${PROJECT_SOURCE_DIR}
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Execute NEST's testsuite....")

message("C_FLAGS: ${CMAKE_C_FLAGS}")
message("CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message("INCLUDE_DIRECTORIES: ${INCLUDE_DIRECTORIES}")
message("LINK_LIBRARIES: ${LINK_LIBRARIES}")
