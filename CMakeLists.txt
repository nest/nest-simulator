cmake_minimum_required(VERSION 2.8.12)
# juqueen 2.8.12.2

# add cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Platform)

project(nest CXX C)
set(NEST_USER_EMAIL "nest_user@nest-initiative.org")

# VERSION exported to libnestutil/config.h, extras/create_release.sh
set(NEST_VERSION_MAJOR 2)
set(NEST_VERSION_MINOR 8)
set(NEST_VERSION_PATCHLEVEL 0-git)

set(NEST_VERSION_VERSION "${NEST_VERSION_MAJOR}.${NEST_VERSION_MINOR}.${NEST_VERSION_PATCHLEVEL}")
set(NEST_VERSION_PRGNAME "${PROJECT_NAME}-${NEST_VERSION_VERSION}")

# some default directories
# install prefix is set with -DCMAKE_INSTALL_PREFIX:PATH=...
set(INSTALL_BIN_DIR bin)
set(INSTALL_LIB_DIR lib)
set(INSTALL_DATA_DIR share/nest)
set(INSTALL_DOC_DIR share/doc/nest)

set(PKGSRCDIR ${PROJECT_SOURCE_DIR})
set(PKGBUILDDIR ${PROJECT_BINARY_DIR})
set(PKGDATADIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR})
set(PKGDOCDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR})
set(TESTSUITE_BASEDIR ${PKGDOCDIR})

# on OS X
set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

################################################################################
#################    Check for includes/ functions /symbols    #################
################################################################################


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

# Check types exist
include(CheckTypeSize)
CHECK_TYPE_SIZE("long long" LONG_LONG_SIZE) # also sets HAVE_LONG_LONG
if( LONG_LONG_SIZE GREATER 0 )
  set(HAVE_LONG_LONG ON)
endif()

# Check symbols / defines exist
include(CheckSymbolExists)
check_symbol_exists(NAN "math.h" HAVE_NAN)
check_symbol_exists(isnan "math.h" HAVE_ISNAN)

include(CheckCXXSymbolExists)
check_cxx_symbol_exists(M_E "cmath" HAVE_M_E)
check_cxx_symbol_exists(M_PI "cmath" HAVE_M_PI)

# Check functions exist
include(CheckFunctionExists)
check_function_exists(expm1 "math.h" HAVE_EXPM1)
check_function_exists(fork "unistd.h" HAVE_FORK)
check_function_exists(execvp "unistd.h" HAVE_EXECVP)
check_function_exists(opendir "dirent.h" HAVE_OPENDIR)

################################################################################
##################         All User Defined options           ##################
################################################################################

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
option(WITH_OPENMP "Enable OpenMP multithreading [default=ON]. Optional: set OMP flag." ON)
option(WITH_PYTHON "Find a Python interpreter [default=ON]. To ignore the Python set to OFF. To set a specific Python, set path." ON)
option(EXTERNAL_MODULES "External modules to be linked in. Devided by ; ." OFF)
option(STATIC_LIBRARIES "Build static libraries. [default=no]" OFF)
option(PS_ARRAYS "Use PS array construction semantics" ON)
SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)

# not defined yet
option(WITH_SCALI "Request compilation with Scali MPI; optionally give directory with Scali installation.[/opt/scali]" OFF)
option(WITH_MUSIC "Request the use of MUSIC; optionally give the directory, where MUSIC is installed." OFF)

################################################################################
##################           Find utility programs            ##################
################################################################################

# needed for target doc and fulldoc
find_package(Doxygen)
find_program(SED NAMES sed gsed)

################################################################################
##################          Process custom options            ##################
################################################################################

# add custom warnings and optimizations
if(WITH_OPTIMIZE)
  list(APPEND CMAKE_C_FLAGS "${WITH_OPTIMIZE}")
  list(APPEND CMAKE_CXX_FLAGS "${WITH_OPTIMIZE}")
endif(WITH_OPTIMIZE)

if(WITH_WARNING)
  list(APPEND CMAKE_C_FLAGS "${WITH_WARNING}")
  list(APPEND CMAKE_CXX_FLAGS "${WITH_WARNING}")
endif(WITH_WARNING)

if(K_COMPUTER)
  # TODO use the -DCMAKE_TOOLCHAIN_FILE= argument
  set(IS_K ON)
endif(K_COMPUTER)

if(ENABLE_BLUEGENE)
  # TODO use the -DCMAKE_TOOLCHAIN_FILE= argument
  if(${ENABLE_BLUEGENE} STREQUAL "L")
    set(IS_BLUEGENE_L ON)
  elseif(${ENABLE_BLUEGENE} STREQUAL "P")
    set(IS_BLUEGENE_P ON)
  elseif(${ENABLE_BLUEGENE} STREQUAL "Q")
    set(IS_BLUEGENE_Q ON)
  else()
    message(SEND_ERROR "Only L/P/Q is allowed for ENABLE_BLUEGENE.")
  endif()
  set(IS_BLUEGENE ON)

  # Override options for JUQUEEN
  option(STATIC_LIBRARIES ON)
  option(WITH_READLINE OFF)
endif(ENABLE_BLUEGENE)

# build static or shared libraries
if( STATIC_LIBRARIES )
  set(BUILD_SHARED_LIBS OFF)
  if (UNIX OR APPLE)
    # On Linux .a is the static library suffix, on Mac OS X .lib can also
    # be used, so we'll add both to the preference list.
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
    # -static GCC
    set(CMAKE_EXE_LINKER_FLAGS "-static")
  endif()
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

# write static_modules.h
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
# write static_modules.h

# Set tics per ms / step
if( TICS_PER_MS )
  set(HAVE_TICS_PER_MS ON)
endif( TICS_PER_MS )

if( TICS_PER_STEP )
  set(HAVE_TICS_PER_STEP ON)
endif( TICS_PER_STEP )

################################################################################
##################            Find Libraries here             ##################
################################################################################

# Only find libLTDL if we link dynamically
if( NOT STATIC_LIBRARIES )
  find_package(LTDL)
  if(LTDL_FOUND)
    set(HAVE_LIBLTDL ON)
  
    include_directories(${LTDL_INCLUDE_DIR})
    link_libraries(${LTDL_LIBRARIES})
  else()
    set(HAVE_LIBLTDL OFF)
  endif(LTDL_FOUND)
else()
  set(HAVE_LIBLTDL OFF)
endif()

# GSL finding
if( GSL_PREFIX )
  # if set, use this prefix
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

# OPENMP finding
if(WITH_OPENMP)
  if (NOT ${WITH_OPENMP} STREQUAL "ON")
    # a path is set
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${WITH_OPENMP}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WITH_OPENMP}")
  else()
    find_package(OpenMP)
    if(OPENMP_FOUND)
      set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif(OPENMP_FOUND)
  endif()
endif(WITH_OPENMP)

# MPI finding
if(WITH_MPI)
  find_package(MPI)
  if(MPI_CXX_FOUND)
    set(HAVE_MPI ON)
  endif()

  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${MPI_C_COMPILE_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
  
  list(APPEND CMAKE_EXE_LINKER_FLAGS "${MPI_CXX_LINK_FLAGS}")
  include_directories(${MPI_CXX_INCLUDE_PATH})
  link_libraries(${MPI_CXX_LIBRARIES})
endif(WITH_MPI)

if(WITH_MUSIC)
  # TODO Music
else(WITH_MUSIC)
  set(HAVE_MUSIC OFF)
endif(WITH_MUSIC)

################################################################################
##################        Define Subdirectories here          ##################
################################################################################

add_subdirectory(cmake)
if(LIBNEUROSIM)
  add_subdirectory(conngen)
endif(LIBNEUROSIM)
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
if(HAVE_PYTHON)
  add_subdirectory(pynest)
endif(HAVE_PYTHON)
add_subdirectory(testsuite)
add_subdirectory(topology)

################################################################################
##################           File generation here             ##################
################################################################################

configure_file(
  "${PROJECT_SOURCE_DIR}/libnestutil/config.h.in"
  "${PROJECT_BINARY_DIR}/libnestutil/config.h" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/pynest/setup.py.in"
  "${PROJECT_BINARY_DIR}/pynest/setup.py" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/pynest/do_tests.py.in"
  "${PROJECT_BINARY_DIR}/pynest/do_tests.py" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/testsuite/do_tests.sh.in"
  "${PROJECT_BINARY_DIR}/testsuite/do_tests.sh" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/extras/emacs/sli.el.in"
  "${PROJECT_BINARY_DIR}/extras/emacs/sli.el" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/extras/nest-config.in"
  "${PROJECT_BINARY_DIR}/extras/nest-config" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/extras/create_release.sh.in"
  "${PROJECT_BINARY_DIR}/extras/create_release.sh" @ONLY
)

configure_file(
  "${PROJECT_SOURCE_DIR}/extras/nest_vars.sh.in"
  "${PROJECT_BINARY_DIR}/extras/nest_vars.sh" @ONLY
)

################################################################################
##################           Custom Global Targets            ##################
################################################################################

add_custom_target(installcheck
  COMMAND cmake --build ${PROJECT_BINARY_DIR} --target install
  COMMAND export PATH="${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}"
  COMMAND export PYTHON="${PYTHON}"
  COMMAND export PYTHONPATH="${CMAKE_INSTALL_PREFIX}${PYEXECDIR}:$(PYTHONPATH)"
  COMMAND export exec_prefix="${CMAKE_INSTALL_PREFIX}"
  COMMAND
    ${PKGDATADIR}/extras/do_tests.sh --test-pynest --source-dir=${PROJECT_SOURCE_DIR}
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Execute NEST's testsuite...."
)

################################################################################
##################           Configuration Summary            ##################
################################################################################

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(ALL_CFLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_DEBUG}")
  set(ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set(ALL_CFLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELEASE}")
  set(ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  set(ALL_CFLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  set(ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
  set(ALL_CFLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_MINSIZEREL}")
  set(ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "None")
  set(ALL_CFLAGS   "${CMAKE_C_FLAGS}")
  set(ALL_CXXFLAGS "${CMAKE_CXX_FLAGS}")
else()
  message(SEND_ERROR "Unknown build type: '${CMAKE_BUILD_TYPE}'")
endif()

message("")
message("--------------------------------------------------------------------------------")
message("NEST Configuration Summary")
message("--------------------------------------------------------------------------------")
message("")
message("C compiler          : ${CMAKE_C_COMPILER}")
message("C compiler flags    : ${ALL_CFLAGS}")
message("C++ compiler        : ${CMAKE_CXX_COMPILER}")
message("C++ compiler flags  : ${ALL_CXXFLAGS}")
message("Build dynamic       : ${BUILD_SHARED_LIBS}")
message("")
message("Built-in modules    : ${SLI_MODULES}")
if(EXTERNAL_MODULES)
  message("User modules        : ${EXTERNAL_MODULES}")
else(EXTERNAL_MODULES)
  message("User modules        : None")
endif(EXTERNAL_MODULES)

if(HAVE_PYTHON)
  message("Python bindings     : Yes (Python ${PYTHON_VERSION}: ${PYTHON})")
  message("Cython bindings     : Yes (Cython: ${CYTHON_EXECUTABLE})")
else(HAVE_PYTHON)
  message("Python bindings     : No")
endif(HAVE_PYTHON)

if(OPENMP_FOUND)
  message("Use threading       : Yes (OpenMP: ${OpenMP_CXX_FLAGS})")
else(OPENMP_FOUND)
  message("Use threading       : No")
endif(OPENMP_FOUND)

if(HAVE_GSL)
  message("Use GSL             : Yes (GSL ${GSL_VERSION})")
  message("    Includes        : ${GSL_INCLUDE_DIRS}")
  message("    Libraries       : ${GSL_LIBRARIES}")
  message("")
else(HAVE_GSL)
  message("Use GSL             : No")
endif(HAVE_GSL)

if(HAVE_READLINE)
  message("Use Readline        : Yes")
  message("    Includes        : ${READLINE_INCLUDE_DIRS}")
  message("    Libraries       : ${READLINE_LIBRARIES}")
  message("")
else(HAVE_READLINE)
  message("Use Readline        : No")
endif(HAVE_READLINE)

if(HAVE_LIBLTDL)
  message("Use libltdl         : Yes")
  message("    Includes        : ${LTDL_INCLUDE_DIR}")
  message("    Libraries       : ${LTDL_LIBRARIES}")
  message("")
else(HAVE_LIBLTDL)
  message("Use libltdl         : No")
endif(HAVE_LIBLTDL)

if(DOXYGEN_FOUND)
  message("Use doxygen         : Yes (${DOXYGEN_EXECUTABLE})")
  message("                    : target `doc` available")
  if(DOXYGEN_DOT_FOUND)
    message("    `dot` available : Yes (${DOXYGEN_DOT_EXECUTABLE})")
    message("                    : target `fulldoc` available")
  endif(DOXYGEN_DOT_FOUND)
else(DOXYGEN_FOUND)
  message("Use doxygen         : No")
endif(DOXYGEN_FOUND)

if(HAVE_MPI)
  message("Use MPI             : Yes (MPI: ${MPI_CXX_COMPILER})")
  message("    FLAGS           : ${MPI_CXX_COMPILE_FLAGS}")
  message("    Includes        : ${MPI_CXX_INCLUDE_PATH}")
  message("    Link Flags      : ${MPI_CXX_LINK_FLAGS}")
  message("    Libraries       : ${MPI_CXX_LIBRARIES}")
  message("")
else(HAVE_MPI)
  message("Use MPI             : No")
endif(HAVE_MPI)

if(HAVE_MUSIC)
  message("Use MUSIC           : Yes")
  message("    Includes        : ${MUSIC_INCLUDE_DIRS}")
  message("    Libraries       : ${MUSIC_LIBRARIES})")
  message("")
else(HAVE_MUSIC)
  message("Use MUSIC           : No")
endif(HAVE_MUSIC)

if(HAVE_LIBNEUROSIM)
  message("Use libneurosim     : Yes")
  message("    Includes        : ${LIBNEUROSIM_INCLUDE_DIRS})")
  message("    Libraries       : ${LIBNEUROSIM_LIBRARIES})")
  message("")
else(HAVE_LIBNEUROSIM)
  message("Use libneurosim     : No")
endif(HAVE_LIBNEUROSIM)

message("")
message("--------------------------------------------------------------------------------")
message("")
message("The NEST executable will be installed to:")
message("  ${SLI_PREFIX}/${INSTALL_BIN_DIR}/")
message("")
message("Documentation and examples will be installed to:")
message( "  ${PKGDOCDIR}/")
message("")
if( HAVE_PYTHON )
    message("PyNEST will be installed to:")
    message("    ${SLI_PREFIX}/${PYEXECDIR}")
    message("")
endif( HAVE_PYTHON )
message("To set necessary environment variables, add the following line")
message("to your ~/.bashrc :")
message("  source ${SLI_PREFIX}/bin/nest_vars.sh")
message("")
message("--------------------------------------------------------------------------------")
message("")

if( WITH_SCALI )
  message("")
  message("WARNING: We advise against using the SCALI MPI Library for the time being,")
  message("WARNING: because of instabilities observed when simulating with 28 or more")
  message("WARNING: processes. We recommend using OpenMPI (www.open-mpi.org) instead!")
  message("")
  message("--------------------------------------------------------------------------------")
  message("")
endif( WITH_SCALI )

if(HAVE_GSL)
  if(NOT HAVE_GSL_1_2)
    message("")
    message("Your GNU Scientific Library is earlier than GSL v 1.2.")
    message("GSL Random Number Generators will not be available.")
    message("See http://www.gnu.org/software/gsl for newer versions.")
    message("")
    message("--------------------------------------------------------------------------------")
    message("")
  endif(NOT HAVE_GSL_1_2)
  if (NOT HAVE_GSL_1_11)
    message("")
    message("Your GNU Scientific Library is earlier than GSL v 1.11.")
    message("The aeif_cond_alpha neuron will not be available.")
    message("See http://www.gnu.org/software/gsl for newer versions.")
    message("")
    message("--------------------------------------------------------------------------------")
    message("")
  endif(NOT HAVE_GSL_1_11)
else()
  message("")
  message("ATTENTION!")
  message("You are about to compile NEST without the GNU Scientific")
  message("Library. This means that conductance-based neuron models")
  message("and some random number generators will not be available.")
  message("")
  message("--------------------------------------------------------------------------------")
  message("")
endif(HAVE_GSL)

message("You can now build and install NEST with")
message("  make")
message("  make install")
message("  make installcheck")
message("")

if(HAVE_MPI)
  message("You have configured NEST with support for distributed computing.")
  message("After running make install, please tell NEST how to start a")
  message("distributed job on your system before running make installcheck.")
  message("You can do this by modifying the template for command /mpirun in")
  message("~/.nestrc, which is created by the first start of NEST.")
  message("")
endif(HAVE_MPI)

message("If you experience problems with the installation or the use of NEST,")
message("please see http://www.nest-simulator.org/frequently_asked_questions")
message("or go to http://www.nest-simulator.org/community to find out how to")
message("join the user mailing list.")
message("")
