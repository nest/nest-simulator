# CMake - Cross Platform Makefile Generator
# Copyright 2000-2016 Kitware, Inc.
# Copyright 2000-2011 Insight Software Consortium
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
# ------------------------------------------------------------------------------
# 
# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.
# 
# ------------------------------------------------------------------------------
# 
# CMake was initially developed by Kitware with the following sponsorship:
# 
#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).
# 
#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.
# 
#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.
# 
#  * Kitware, Inc.

#
# Blue Gene/Q base platform file.
#
# NOTE: Do not set your platform to "BlueGeneQ-base".  This file is
# included by the real platform files.  Use one of these two platforms
# instead:
#
#     BlueGeneQ-dynamic  For dynamically linked executables
#     BlueGeneQ-static   For statically linked executables
#
# The platform you choose doesn't affect whether or not you can build
# shared or static libraries -- it ONLY changs whether exeuatbles are linked
# statically or dynamically.
#
# This platform file tries its best to adhere to the behavior of the MPI
# compiler wrappers included with the latest BG/P drivers.
#

# Based on the BlueGeneQ-base platform file
set( CMAKE_SYSTEM_NAME Linux CACHE STRING "Cross-compiling for BlueGene/Q" FORCE )
set( CMAKE_SYSTEM_PROCESSOR ppc64 )
set( TRIPLET_VENDOR ibm )

# Set enable-bluegene for main CMakeList.txt
set( enable-bluegene "Q" CACHE STRING "Configure for BlueGene." FORCE )
# no readline support on bluegene
set( with-readline OFF CACHE BOOL "Find a readline library [default=ON]. To set a specific readline, set install path." FORCE )
# we obviously want to do mpi on bluegene
set( with-mpi ON CACHE BOOL "Request compilation with MPI; optionally give directory with MPI installation." FORCE )

#
# This adds directories that find commands should specifically ignore
# for cross compiles.  Most of these directories are the includeand
# lib directories for the frontend on BG/P systems.  Not ignoring
# these can cause things like FindX11 to find a frontend PPC version
# mistakenly.  We use this on BG instead of re-rooting because backend
# libraries are typically strewn about the filesystem, and we can't
# re-root ALL backend libraries to a single place.
#
set( CMAKE_SYSTEM_IGNORE_PATH
    /lib /lib64 /include
    /usr/lib /usr/lib64 /usr/include
    /usr/local/lib /usr/local/lib64 /usr/local/include
    /usr/X11/lib /usr/X11/lib64 /usr/X11/include
    /usr/lib/X11 /usr/lib64/X11 /usr/include/X11
    /usr/X11R6/lib /usr/X11R6/lib64 /usr/X11R6/include
    /usr/X11R7/lib /usr/X11R7/lib64 /usr/X11R7/include
    )

#
# Indicate that this is a unix-like system
#
set( UNIX 1 )

#
# Library prefixes, suffixes, extra libs.
#
set( CMAKE_LINK_LIBRARY_SUFFIX "" )
set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )     # lib
set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )      # .a

set( CMAKE_SHARED_LIBRARY_PREFIX "lib" )     # lib
set( CMAKE_SHARED_LIBRARY_SUFFIX ".so" )     # .so
set( CMAKE_EXECUTABLE_SUFFIX "" )            # .exe

set( CMAKE_DL_LIBS "dl" )

#
# BG/Q supports dynamic libraries regardless of whether we're building
# static or dynamic *executables*.
#
set_property( GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE )
# NEST: can irritate cmake 3.4 and before
#       if this is not set, TARGET_SUPPORTS_SHARED_LIBS will be set to
#       FALSE later
set( CMAKE_C_COMPILER_LINKS_STATICALLY OFF )
set( CMAKE_CXX_COMPILER_LINKS_STATICALLY OFF )

set( CMAKE_FIND_LIBRARY_PREFIXES "lib" )

#
# For BGQ builds, we're cross compiling, but we don't want to re-root things
# (e.g. with CMAKE_FIND_ROOT_PATH) because users may have libraries anywhere on
# the shared filesystems, and this may lie outside the root.  Instead, we set the
# system directories so that the various system BG CNK library locations are
# searched first.  This is not the clearest thing in the world, given IBM's driver
# layout, but this should cover all the standard ones.
#
macro( __bluegeneq_common_setup compiler_id lang )

  # Need to use the version of the comm lib compiled with the right compiler.
  set( __BlueGeneQ_commlib_dir gcc )
  if ( ${compiler_id} STREQUAL XL )
    set( __BlueGeneQ_commlib_dir xl )
  endif ()

  set( CMAKE_SYSTEM_LIBRARY_PATH
      /bgsys/drivers/ppcfloor/comm/lib                            # default comm layer (used by mpi compiler wrappers)
      /bgsys/drivers/ppcfloor/comm/sys/lib/
      /bgsys/drivers/ppcfloor/comm/${__BlueGeneQ_commlib_dir}/lib # PAMI, other lower-level comm libraries
      /bgsys/drivers/ppcfloor/spi/lib
      /bgsys/drivers/ppcfloor/gnu-linux/lib                       # CNK python installation directory
      /bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/lib   # CNK Linux image -- standard runtime libs, pthread, etc.
      )

  # Add all the system include paths.
  set( CMAKE_SYSTEM_INCLUDE_PATH
      /bgsys/drivers/ppcfloor/comm/include
      /bgsys/drivers/ppcfloor/comm/sys/include
      /bgsys/drivers/ppcfloor/
      /bgsys/drivers/ppcfloor/spi/include
      /bgsys/drivers/ppcfloor/spi/include/kernel/cnk
      /bgsys/drivers/ppcfloor/comm/${__BlueGeneQ_commlib_dir}/include
      )

  # Ensure that the system directories are included with the regular compilers, as users will expect this
  # to do the same thing as the MPI compilers, which add these flags.
  set( BGQ_SYSTEM_INCLUDES "" )
  foreach ( dir ${CMAKE_SYSTEM_INCLUDE_PATH} )
    set( BGQ_SYSTEM_INCLUDES "${BGQ_SYSTEM_INCLUDES} -I${dir}" )
  endforeach ()

  if ( CMAKE_VERSION VERSION_LESS 3 )
    # remove <INCLUDES> as cmake 2.8.12 is confused by it
    set( CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER>   <DEFINES> ${BGQ_SYSTEM_INCLUDES} <FLAGS> -o <OBJECT> -c <SOURCE>" )
    set( CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> ${BGQ_SYSTEM_INCLUDES} <FLAGS> -o <OBJECT> -c <SOURCE>" )
  else ()
    # later versions need <INCLUDES> for the includes
    set( CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER>   <DEFINES> ${BGQ_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>" )
    set( CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> ${BGQ_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>" )
  endif ()

  #
  # Code below does setup for shared libraries.  That this is done
  # regardless of whether the platform is static or dynamic -- you can make
  # shared libraries even if you intend to make static executables, you just
  # can't make a dynamic executable if you use the static platform file.
  #
  if ( ${compiler_id} STREQUAL XL )
    # Flags for XL compilers if we explicitly detected XL
    set( CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-qpic" )
    set( CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-qmkshrobj -qnostaticlink" )
  else ()
    # Assume flags for GNU compilers (if the ID is GNU *or* anything else).
    set( CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-fPIC" )
    set( CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared" )
  endif ()

  # Both toolchains use the GNU linker on BG/P, so these options are shared.
  set( CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,-rpath," )
  set( CMAKE_SHARED_LIBRARY_RPATH_LINK_${lang}_FLAG "-Wl,-rpath-link," )
  set( CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,-soname," )
  set( CMAKE_EXE_EXPORTS_${lang}_FLAG "-Wl,--export-dynamic" )
  set( CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "" )  # +s, flag for exe link to use shared lib
  set( CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":" ) # : or empty

endmacro()

#
# This macro needs to be called for dynamic library support.  Unfortunately on BG,
# We can't support both static and dynamic links in the same platform file.  The
# dynamic link platform file needs to call this explicitly to set up dynamic linking.
#
macro( __bluegeneq_setup_dynamic compiler_id lang )
  __bluegeneq_common_setup( ${compiler_id} ${lang} )

  set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a" )

  if ( ${compiler_id} STREQUAL XL )
    set( BGQ_${lang}_DYNAMIC_EXE_FLAGS "-qnostaticlink -qnostaticlink=libgcc" )
  else ()
    set( BGQ_${lang}_DYNAMIC_EXE_FLAGS "-dynamic" )
  endif ()

  # For dynamic executables, need to provide special BG/Q arguments.
  set( BGQ_${lang}_DEFAULT_EXE_FLAGS
      "<FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>" )
  set( CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> -Wl,-relax ${BGQ_${lang}_DYNAMIC_EXE_FLAGS} ${BGQ_${lang}_DEFAULT_EXE_FLAGS}" )
endmacro()

#
# This macro needs to be called for static builds.  Right now it just adds -Wl,-relax
# to the link line.
#
macro( __bluegeneq_setup_static compiler_id lang )
  __bluegeneq_common_setup( ${compiler_id} ${lang} )

  set( CMAKE_FIND_LIBRARY_SUFFIXES ".a" )

  # For static executables, use default link settings.
  set( BGQ_${lang}_DEFAULT_EXE_FLAGS
      "<FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>" )
  set( CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> -Wl,-relax ${BGQ_${lang}_DEFAULT_EXE_FLAGS}" )
endmacro()
