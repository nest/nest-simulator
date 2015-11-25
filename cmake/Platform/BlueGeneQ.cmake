# Based on the BlueGeneQ-static-XL-CXX platform file

include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME BlueGeneQ-static CACHE STRING "Cross-compiling for BlueGene/Q" FORCE)
# specify the cross compiler
cmake_force_c_compiler(bgxlc_r XL)
cmake_force_cxx_compiler(bgxlc++_r XL)

#
# Set ENABLE_BLUEGENE for main CMakeList.txt
#
set(ENABLE_BLUEGENE "Q" CACHE STRING "Configure for BlueGene." FORCE)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

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

#
# Indicate that this is a UNIX-like system
#
set(UNIX 1)

#
# Library prefixes, suffixes, extra libs.
#
set(CMAKE_LINK_LIBRARY_SUFFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")     # lib
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")      # .a
set(CMAKE_EXECUTABLE_SUFFIX "")            # .exe

#
# Library search prefix/suffix.
#
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_SYSTEM_IGNORE_PATH
  /lib             /lib64             /include
  /usr/lib         /usr/lib64         /usr/include
  /usr/local/lib   /usr/local/lib64   /usr/local/include
  /usr/X11/lib     /usr/X11/lib64     /usr/X11/include
  /usr/lib/X11     /usr/lib64/X11     /usr/include/X11
  /usr/X11R6/lib   /usr/X11R6/lib64   /usr/X11R6/include
  /usr/X11R7/lib   /usr/X11R7/lib64   /usr/X11R7/include
)

#
# For BGQ builds, we're cross compiling, but we don't want to re-root things
# (e.g. with CMAKE_FIND_ROOT_PATH) because users may have libraries anywhere on
# the shared filesystems, and this may lie outside the root.  Instead, we set the
# system directories so that the various system BG CNK library locations are
# searched first.  This is not the clearest thing in the world, given IBM's driver
# layout, but this should cover all the standard ones.
#
set(CMAKE_SYSTEM_LIBRARY_PATH
  /bgsys/drivers/ppcfloor/comm/lib
  /bgsys/drivers/ppcfloor/spi/lib
  /bgsys/drivers/ppcfloor/gnu-linux/lib
  /bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/lib
)

# create C/CXX objects
set(CMAKE_C_COMPILE_OBJECT   "<CMAKE_C_COMPILER>   <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")


# For static C executables, use default link settings.
set(BGQ_C_DEFAULT_EXE_FLAGS "<FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> -Wl,-relax ${BGQ_C_DEFAULT_EXE_FLAGS}")

# For static CXX executables, use default link settings.
set(BGQ_CXX_DEFAULT_EXE_FLAGS "<FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> -Wl,-relax ${BGQ_CXX_DEFAULT_EXE_FLAGS}")
