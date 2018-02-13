#.rst:
# FindGSL
# --------
#
# Find the native GSL includes and libraries.
#
# The GNU Scientific Library (GSL) is a numerical library for C and C++
# programmers. It is free software under the GNU General Public
# License.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# If GSL is found, this module defines the following :prop_tgt:`IMPORTED`
# targets::
#
#  GSL::gsl      - The main GSL library.
#  GSL::gslcblas - The CBLAS support library used by GSL.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project::
#
#  GSL_FOUND          - True if GSL found on the local system
#  GSL_INCLUDE_DIRS   - Location of GSL header files.
#  GSL_LIBRARIES      - The GSL libraries.
#  GSL_VERSION        - The version of the discovered GSL install.
#
# Hints
# ^^^^^
#
# Set ``GSL_ROOT_DIR`` to a directory that contains a GSL installation.
#
# This script expects to find libraries at ``$GSL_ROOT_DIR/lib`` and the GSL
# headers at ``$GSL_ROOT_DIR/include/gsl``.  The library directory may
# optionally provide Release and Debug folders.  For Unix-like systems, this
# script will use ``$GSL_ROOT_DIR/bin/gsl-config`` (if found) to aid in the
# discovery GSL.
#
# Cache Variables
# ^^^^^^^^^^^^^^^
#
# This module may set the following variables depending on platform and type
# of GSL installation discovered.  These variables may optionally be set to
# help this module find the correct files::
#
#  GSL_CLBAS_LIBRARY       - Location of the GSL CBLAS library.
#  GSL_CBLAS_LIBRARY_DEBUG - Location of the debug GSL CBLAS library (if any).
#  GSL_CONFIG_EXECUTABLE   - Location of the ``gsl-config`` script (if any).
#  GSL_LIBRARY             - Location of the GSL library.
#  GSL_LIBRARY_DEBUG       - Location of the debug GSL library (if any).
#

#=============================================================================
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
#=============================================================================

# Modified for NEST by Tammo Ippen <t.ippen@fz-juelich.de> 20 Jan 2016.

# Include these modules to handle the QUIETLY and REQUIRED arguments.
# include(Modules/FindPackageHandleStandardArgs.cmake)

#=============================================================================
# If the user has provided ``GSL_ROOT_DIR``, use it!  Choose items found
# at this location over system locations.
if ( EXISTS "$ENV{GSL_ROOT_DIR}" )
  file( TO_CMAKE_PATH "$ENV{GSL_ROOT_DIR}" GSL_ROOT_DIR )
  set( GSL_ROOT_DIR "${GSL_ROOT_DIR}" CACHE PATH "Prefix for GSL installation." )
endif ()
if ( NOT EXISTS "${GSL_ROOT_DIR}" )
  set( GSL_USE_PKGCONFIG ON )
endif ()

#=============================================================================
# As a first try, use the PkgConfig module.  This will work on many
# *NIX systems.  See :module:`findpkgconfig`
# This will return ``GSL_INCLUDEDIR`` and ``GSL_LIBDIR`` used below.
if ( GSL_USE_PKGCONFIG )
  find_package( PkgConfig )
  pkg_check_modules( GSL QUIET gsl )

  if ( EXISTS "${GSL_INCLUDEDIR}" )
    get_filename_component( GSL_ROOT_DIR "${GSL_INCLUDEDIR}" DIRECTORY CACHE )
  endif ()
endif ()

if ( NOT GSL_ROOT_DIR AND NOT GSL_INCLUDEDIR AND NOT GSL_LIBDIR )
  # not found yet... try gsl-config and set GSL_ROOT_DIR
  # 1. If gsl-config exists, query for the version.
  find_program( GSL_CONFIG_EXECUTABLE
      NAMES gsl-config
      )
  if ( EXISTS "${GSL_CONFIG_EXECUTABLE}" )
    execute_process(
        COMMAND "${GSL_CONFIG_EXECUTABLE}" --prefix
        OUTPUT_VARIABLE GSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE )
  endif ()
endif ()

#=============================================================================
# Set GSL_INCLUDE_DIRS and GSL_LIBRARIES. If we skipped the PkgConfig step, try
# to find the libraries at $GSL_ROOT_DIR (if provided) or in standard system
# locations.  These find_library and find_path calls will prefer custom
# locations over standard locations (HINTS).  If the requested file is not found
# at the HINTS location, standard system locations will be still be searched
# (/usr/lib64 (Redhat), lib/i386-linux-gnu (Debian)).

find_path( GSL_INCLUDE_DIR
    NAMES gsl/gsl_sf.h
    HINTS ${GSL_ROOT_DIR}/include ${GSL_INCLUDEDIR}
    )
find_library( GSL_LIBRARY
    NAMES gsl
    HINTS ${GSL_ROOT_DIR}/lib ${GSL_LIBDIR}
    PATH_SUFFIXES Release Debug
    )
find_library( GSL_CBLAS_LIBRARY
    NAMES gslcblas cblas
    HINTS ${GSL_ROOT_DIR}/lib ${GSL_LIBDIR}
    PATH_SUFFIXES Release Debug
    )
# Do we also have debug versions?
find_library( GSL_LIBRARY_DEBUG
    NAMES gsl
    HINTS ${GSL_ROOT_DIR}/lib ${GSL_LIBDIR}
    PATH_SUFFIXES Debug
    )
find_library( GSL_CBLAS_LIBRARY_DEBUG
    NAMES gslcblas cblas
    HINTS ${GSL_ROOT_DIR}/lib ${GSL_LIBDIR}
    PATH_SUFFIXES Debug
    )
set( GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR} )
set( GSL_LIBRARIES ${GSL_LIBRARY} ${GSL_CBLAS_LIBRARY} )

# If we didn't use PkgConfig, try to find the version via gsl-config or by
# reading gsl_version.h.
if ( NOT GSL_VERSION )
  # 1. If gsl-config exists, query for the version.
  find_program( GSL_CONFIG_EXECUTABLE
      NAMES gsl-config
      HINTS "${GSL_ROOT_DIR}/bin"
      )
  if ( EXISTS "${GSL_CONFIG_EXECUTABLE}" )
    execute_process(
        COMMAND "${GSL_CONFIG_EXECUTABLE}" --version
        OUTPUT_VARIABLE GSL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE )
  endif ()

  # 2. If gsl-config is not available, try looking in gsl/gsl_version.h
  if ( NOT GSL_VERSION AND EXISTS "${GSL_INCLUDE_DIRS}/gsl/gsl_version.h" )
    file( STRINGS "${GSL_INCLUDE_DIRS}/gsl/gsl_version.h" gsl_version_h_contents REGEX "define GSL_VERSION" )
    string( REGEX REPLACE ".*([0-9]\\.[0-9][0-9]).*" "\\1" GSL_VERSION ${gsl_version_h_contents} )
  endif ()

  # might also try scraping the directory name for a regex match "gsl-X.X"
endif ()

#=============================================================================
# handle the QUIETLY and REQUIRED arguments and set GSL_FOUND to TRUE if all
# listed variables are TRUE
find_package_handle_standard_args( GSL
  FOUND_VAR
    GSL_FOUND
  REQUIRED_VARS
    GSL_INCLUDE_DIR
    GSL_LIBRARY
    GSL_CBLAS_LIBRARY
  VERSION_VAR
    GSL_VERSION
    )

mark_as_advanced( GSL_ROOT_DIR GSL_VERSION GSL_LIBRARY GSL_INCLUDE_DIR
    GSL_CBLAS_LIBRARY GSL_LIBRARY_DEBUG GSL_CBLAS_LIBRARY_DEBUG
    GSL_USE_PKGCONFIG GSL_CONFIG )
