#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
# Copyright 2010 Todd Gamblin <tgamblin@llnl.gov>
# Copyright 2012 Julien Bigot <julien.bigot@cea.fr>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#=============================================================================

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
# Indicate that this is a unix-like system
#
set(UNIX 1)

#
# Library prefixes, suffixes, extra libs.
#
set(CMAKE_LINK_LIBRARY_SUFFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")     # lib
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")      # .a

set(CMAKE_SHARED_LIBRARY_PREFIX "lib")     # lib
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")     # .so
set(CMAKE_EXECUTABLE_SUFFIX "")            # .exe

set(CMAKE_DL_LIBS "dl")

#
# BG/Q supports dynamic libraries regardless of whether we're building
# static or dynamic *executables*.
#
#set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")

#
# For BGQ builds, we're cross compiling, but we don't want to re-root things
# (e.g. with CMAKE_FIND_ROOT_PATH) because users may have libraries anywhere on
# the shared filesystems, and this may lie outside the root.  Instead, we set the
# system directories so that the various system BG CNK library locations are
# searched first.  This is not the clearest thing in the world, given IBM's driver
# layout, but this should cover all the standard ones.
#
macro(__BlueGeneQ_common_setup compiler_id lang)
  # Need to use the version of the comm lib compiled with the right compiler.
  set(__BlueGeneQ_commlib_dir gcc)
  if (${compiler_id} STREQUAL XL)
    set(__BlueGeneQ_commlib_dir xl)
  endif()

  set(CMAKE_SYSTEM_LIBRARY_PATH
    /bgsys/drivers/ppcfloor/comm/default/lib                    # default comm layer (used by mpi compiler wrappers)
    /bgsys/drivers/ppcfloor/comm/${__BlueGeneQ_commlib_dir}/lib # PAMI, other lower-level comm libraries
    /bgsys/drivers/ppcfloor/gnu-linux/lib                       # CNK python installation directory
    /bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/lib   # CNK Linux image -- standard runtime libs, pthread, etc.
    )

  # Add all the system include paths.
  set(CMAKE_SYSTEM_INCLUDE_PATH
    /bgsys/drivers/ppcfloor/comm/sys/include
    /bgsys/drivers/ppcfloor/
    /bgsys/drivers/ppcfloor/spi/include
    /bgsys/drivers/ppcfloor/spi/include/kernel/cnk
    /bgsys/drivers/ppcfloor/comm/${__BlueGeneQ_commlib_dir}/include
    )

  # Ensure that the system directories are included with the regular compilers, as users will expect this
  # to do the same thing as the MPI compilers, which add these flags.
  set(BGQ_SYSTEM_INCLUDES "")
  foreach(dir ${CMAKE_SYSTEM_INCLUDE_PATH})
    set(BGQ_SYSTEM_INCLUDES "${BGQ_SYSTEM_INCLUDES} -I${dir}")
  endforeach()
  set(CMAKE_C_COMPILE_OBJECT   "<CMAKE_C_COMPILER>   <DEFINES> ${BGQ_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
  set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> ${BGQ_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")

endmacro()

#
# This macro needs to be called for static builds.  Right now it just adds -Wl,-relax
# to the link line.
#
macro(__BlueGeneQ_setup_static compiler_id lang)
  __BlueGeneQ_common_setup(${compiler_id} ${lang})

  # For static executables, use default link settings.
  set(BGQ_${lang}_DEFAULT_EXE_FLAGS
    "<FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> -Wl,-relax ${BGQ_${lang}_DEFAULT_EXE_FLAGS}")

  if(CMAKE_BUILD_TYPE MATCHES "Deb" AND ${compiler_id} STREQUAL "XL")
      # Work around an unknown compiler bug triggered in
      # compute_globals(). Using -O0 disables -qhot and this seems
      # to break the normal OpenMP flag -qsmp unless qualified with
      # noauto.
      set(OpenMP_C_FLAGS "-qsmp=noauto" CACHE STRING "Compiler flag for OpenMP parallelization")
      set(OpenMP_CXX_FLAGS "-qsmp=noauto" CACHE STRING "Compiler flag for OpenMP parallelization")
  endif()
endmacro()



set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

__BlueGeneQ_setup_static(XL C)
__BlueGeneQ_setup_static(XL CXX)

set(CMAKE_SYSTEM_NAME BlueGeneQ-static CACHE STRING "Cross-compiling for BlueGene/Q" FORCE)

set(CMAKE_C_FLAGS_DEBUG "-g" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "Compiler optimization flags")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "Compiler optimization flags")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Compiler optimization flags")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Compiler optimization flags")
