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
# JURON base platform file.
#

# Set enable-juron for main CMakeList.txt
set( enable-juron "JURON" CACHE STRING "Configure for JURON." FORCE )
# no readline support on JURON
#set( with-readline OFF CACHE BOOL "Find a readline library [default=ON]. To set a specific readline, set install path." FORCE )

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
# JURON supports dynamic libraries regardless of whether we're building
# static or dynamic *executables*.
#
set_property( GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE )
# NEST: can irritate cmake 3.4 and before
#       if this is not set, TARGET_SUPPORTS_SHARED_LIBS will be set to
#       FALSE later
set( CMAKE_C_COMPILER_LINKS_STATICALLY OFF )
set( CMAKE_CXX_COMPILER_LINKS_STATICALLY OFF )

set( CMAKE_FIND_LIBRARY_PREFIXES "lib" )

macro( __juron_common_setup compiler_id lang )

  # Need to use the version of the comm lib compiled with the right compiler.
  set( __JURON_commlib_dir gcc )
  if ( ${compiler_id} STREQUAL XL )
    set( __JURON_commlib_dir xl )
  endif ()

  # Ensure that the system directories are included with the regular compilers, as users will expect this
  # to do the same thing as the MPI compilers, which add these flags.
  set( JURON_SYSTEM_INCLUDES "" )
  foreach ( dir ${CMAKE_SYSTEM_INCLUDE_PATH} )
    set( JURON_SYSTEM_INCLUDES "-I${dir}" )
  endforeach ()

  if ( CMAKE_VERSION VERSION_LESS 3 )
    # remove <INCLUDES> as cmake 2.8.12 is confused by it
    set( CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER>   <DEFINES> ${JURON_SYSTEM_INCLUDES} <FLAGS> -o <OBJECT> -c <SOURCE>" )
    set( CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> ${JURON_SYSTEM_INCLUDES} <FLAGS> -o <OBJECT> -c <SOURCE>" )
  else ()
    # later versions need <INCLUDES> for the includes
    set( CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER>   <DEFINES> ${JURON_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>" )
    set( CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> ${JURON_SYSTEM_INCLUDES} <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>" )
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
  elseif ( ${compiler_id} STREQUAL LLVM )
    set( CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-std=c++11" )
  else ()
    # Assume flags for GNU compilers (if the ID is GNU *or* anything else).
    set( CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-fPIC" )
    set( CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared" )
  endif ()

  # Both toolchains use the GNU linker on JURON, so these options are shared.
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
macro( __juron_setup_dynamic compiler_id lang )
  __juron_common_setup( ${compiler_id} ${lang} )

  set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a" )

  if ( ${compiler_id} STREQUAL XL )
    set( JURON_${lang}_DYNAMIC_EXE_FLAGS "-qnostaticlink -qnostaticlink=libgcc" )
  else ()
    set( JURON_${lang}_DYNAMIC_EXE_FLAGS "-dynamic" )
  endif ()

  # For dynamic executables, need to provide special JURON arguments.
  set( JURON_${lang}_DEFAULT_EXE_FLAGS
      "<FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>" )
  set( CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> -Wl,-relax ${JURON_${lang}_DYNAMIC_EXE_FLAGS} ${JURON_${lang}_DEFAULT_EXE_FLAGS}" )
endmacro()

#
# This macro needs to be called for static builds.  Right now it just adds -Wl,-relax
# to the link line.
#
macro( __juron_setup_static compiler_id lang )
  __juron_common_setup( ${compiler_id} ${lang} )

  set( CMAKE_FIND_LIBRARY_SUFFIXES ".a" )

  # For static executables, use default link settings.
  set( JURON_${lang}_DEFAULT_EXE_FLAGS
      "<FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>" )
  set( CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> -Wl,-relax ${JURON_${lang}_DEFAULT_EXE_FLAGS}" )
endmacro()
