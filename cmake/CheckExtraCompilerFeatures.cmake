# cmake/CheckExtraCompilerFeatures.cmake
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

# For checking abort and segfault exitcodes
# you cannot simply use `try_run`, as it handles
# these errors special and does not give correct
# return values. Also, you cannot just call execute_process
# on the compiled program, as this will halt the cmake
# configuration. Hence, let a shell script execute the
# program and return the exit code.

####### NEST_EXITCODE_ABORT ########
function( NEST_CHECK_EXITCODE_ABORT )
  message( STATUS "Check the abort exitcode." )
  set( ABORT_ERR "" )
  try_compile( COMPILE_VAR
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/assert_value.c
      COPY_FILE "${CMAKE_BINARY_DIR}/assert_value"
      COPY_FILE_ERROR CP_ERR
      )

  if ( COMPILE_VAR AND NOT CMAKE_CROSSCOMPILING )
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/check_return_val.sh "${CMAKE_BINARY_DIR}/assert_value"
        RESULT_VARIABLE RETURN_VALUE
        ERROR_QUIET OUTPUT_QUIET
    )
    if ( NOT RETURN_VALUE EQUAL 0 )
      set( ABORT_ERR ${RETURN_VALUE} )
    endif ()
    if ( EXISTS "${CMAKE_BINARY_DIR}/assert_value" )
      file( REMOVE "${CMAKE_BINARY_DIR}/assert_value" )
    endif ()
  endif ()
  message( STATUS "Check the abort exitcode. ${ABORT_ERR}" )
  set( NEST_EXITCODE_ABORT ${ABORT_ERR} PARENT_SCOPE )
endfunction()

####### NEST_EXITCODE_SEGFAULT ########
function( NEST_CHECK_EXITCODE_SEGFAULT )
  message( STATUS "Check the segmentation fault exitcode." )
  set( SEG_ERR "" )
  try_compile( COMPILE_VAR
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/segfault_value.c
      COPY_FILE "${CMAKE_BINARY_DIR}/segfault_value"
      COPY_FILE_ERROR CP_ERR
      )

  if ( COMPILE_VAR AND NOT CMAKE_CROSSCOMPILING )
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/check_return_val.sh "${CMAKE_BINARY_DIR}/segfault_value"
        RESULT_VARIABLE RETURN_VALUE
        ERROR_QUIET OUTPUT_QUIET
    )
    if ( NOT RETURN_VALUE EQUAL 0 )
      set( SEG_ERR ${RETURN_VALUE} )
    endif ()
    if ( EXISTS "${CMAKE_BINARY_DIR}/segfault_value" )
      file( REMOVE "${CMAKE_BINARY_DIR}/segfault_value" )
    endif ()
  endif ()
  message( STATUS "Check the segmentation fault exitcode. ${SEG_ERR}" )
  set( NEST_EXITCODE_SEGFAULT ${SEG_ERR} PARENT_SCOPE )
endfunction()

####### HAVE_CMATH_MAKROS_IGNORED ########
function( NEST_CHECK_HAVE_CMATH_MAKROS_IGNORED )
  message( STATUS "Check whether the compiler ignores cmath makros." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/CMathMacros.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_CMATH_MAKROS_IGNORED OFF )
  else ()
    set( HAVE_CMATH_MAKROS_IGNORED ON )
  endif ()
  message( STATUS "Check whether the compiler ignores cmath makros. ${HAVE_CMATH_MAKROS_IGNORED}" )
  set( HAVE_CMATH_MAKROS_IGNORED ${HAVE_CMATH_MAKROS_IGNORED} PARENT_SCOPE)
endfunction()

####### HAVE_ALPHA_CXX_STD_BUG ########
function( NEST_CHECK_HAVE_ALPHA_CXX_STD_BUG )
  message( STATUS "Check whether the compiler does NOT include <*.h> headers ISO conformant." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/AlphaCXXBug.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_ALPHA_CXX_STD_BUG OFF )
  else ()
    set( HAVE_ALPHA_CXX_STD_BUG ON )
  endif ()
  set( HAVE_ALPHA_CXX_STD_BUG ${HAVE_ALPHA_CXX_STD_BUG} PARENT_SCOPE )
  message( STATUS "Check whether the compiler does NOT include <*.h> headers ISO conformant. ${HAVE_ALPHA_CXX_STD_BUG}" )
endfunction()

####### HAVE_SIGUSR_IGNORED ########
function( NEST_CHECK_HAVE_SIGUSR_IGNORED )
  message( STATUS "Check whether the compiler respects symbolic signal names in signal.h." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/SigUsrIgnored.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_SIGUSR_IGNORED OFF )
  else ()
    set( HAVE_SIGUSR_IGNORED ON )
  endif ()
  set( HAVE_SIGUSR_IGNORED ${HAVE_SIGUSR_IGNORED} PARENT_SCOPE )
  message( STATUS "Check whether the compiler respects symbolic signal names in signal.h. ${HAVE_SIGUSR_IGNORED}" )
endfunction()

####### HAVE_STATIC_TEMPLATE_DECLARATION_FAIL ########
function( NEST_CHECK_HAVE_STATIC_TEMPLATE_DECLARATION_FAIL )
  message( STATUS "Check static template member declaration." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/StaticTemplateDeclaration.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_STATIC_TEMPLATE_DECLARATION_FAIL OFF )
  else ()
    set( HAVE_STATIC_TEMPLATE_DECLARATION_FAIL ON )
  endif ()
  set( HAVE_STATIC_TEMPLATE_DECLARATION_FAIL ${HAVE_STATIC_TEMPLATE_DECLARATION_FAIL} PARENT_SCOPE )
  message( STATUS "Check static template member declaration. ${HAVE_STATIC_TEMPLATE_DECLARATION_FAIL}" )
endfunction()

####### HAVE_STL_VECTOR_CAPACITY_BASE_UNITY ########
function( NEST_CHECK_HAVE_STL_VECTOR_CAPACITY_BASE_UNITY )
  message( STATUS "Check for STL vector capacity base unity." )
  set( RUN_RESULT 0 )
  set( RUN_RESULT__TRYRUN_OUTPUT "" )
  try_run( RUN_RESULT COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/VectorCapacity.cxx
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT_VAR
      RUN_OUTPUT_VARIABLE RUN_OUTPUT_VAR
      )
  if ( RUN_RESULT EQUAL 0 )
    set( HAVE_STL_VECTOR_CAPACITY_BASE_UNITY ON )
  else ()
    set( HAVE_STL_VECTOR_CAPACITY_BASE_UNITY OFF )
  endif ()
  set( HAVE_STL_VECTOR_CAPACITY_BASE_UNITY ${HAVE_STL_VECTOR_CAPACITY_BASE_UNITY} PARENT_SCOPE )
  message( STATUS "Check for STL vector capacity base unity. ${HAVE_STL_VECTOR_CAPACITY_BASE_UNITY}" )
endfunction()

####### HAVE_STL_VECTOR_CAPACITY_DOUBLING ########
function( NEST_CHECK_HAVE_STL_VECTOR_CAPACITY_DOUBLING )
  message( STATUS "Check for STL vector capacity doubling strategy." )
  set( RUN_RESULT 0 )
  set( RUN_RESULT__TRYRUN_OUTPUT "" )
  try_run( RUN_RESULT COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/VectorCapacityDoubling.cxx
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT_VAR
      RUN_OUTPUT_VARIABLE RUN_OUTPUT_VAR
      )
  if ( RUN_RESULT EQUAL 0 )
    set( HAVE_STL_VECTOR_CAPACITY_DOUBLING ON )
  else ()
    set( HAVE_STL_VECTOR_CAPACITY_DOUBLING OFF )
  endif ()
  set( HAVE_STL_VECTOR_CAPACITY_DOUBLING ${HAVE_STL_VECTOR_CAPACITY_DOUBLING} PARENT_SCOPE )
  message( STATUS "Check for STL vector capacity doubling strategy. ${HAVE_STL_VECTOR_CAPACITY_DOUBLING}" )
endfunction()

function( NEST_CHECK_HAVE_XLC_ICE_ON_USING )
  # Tests for a an internal compiler error observed in IBM xlC.
  # If bug the ICE is detected, defines
  # HAVE_XLC_ICE_ON_USING
  #
  # @author Hans E. Plesser

  message( STATUS "Check whether the compiler fails with ICE." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/ICE.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_XLC_ICE_ON_USING OFF )
  else ()
    set( HAVE_XLC_ICE_ON_USING ON )
  endif ()
  set( HAVE_XLC_ICE_ON_USING ${HAVE_XLC_ICE_ON_USING} PARENT_SCOPE )
  message( STATUS "Check whether the compiler fails with ICE. ${HAVE_XLC_ICE_ON_USING}" )
endfunction()

####### Test if ::nan(...) is defined #######
function( NEST_CHECK_HAVE_STD_NAN )
  message( STATUS "Check if ::nan is available from cmath." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/std_nan.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_STD_NAN ON )
  else ()
    set( HAVE_STD_NAN OFF )
  endif ()
  set( HAVE_STD_NAN ${HAVE_STD_NAN} PARENT_SCOPE )
  message( STATUS "Check if ::nan is available from cmath. ${HAVE_STD_NAN}" )
endfunction()

####### Test if ::isnan(...) is defined #######
function( NEST_CHECK_HAVE_STD_ISNAN )
  message( STATUS "Check if ::isnan is available from cmath." )
  try_compile( COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFiles/std_isnan.cxx
      OUTPUT_VARIABLE OUTPUT
      )
  if ( COMPILE_RESULT )
    set( HAVE_STD_ISNAN ON )
  else ()
    set( HAVE_STD_ISNAN OFF )
  endif ()
  set( HAVE_STD_ISNAN ${HAVE_STD_ISNAN} PARENT_SCOPE )
  message( STATUS "Check if ::isnan is available from cmath. ${HAVE_STD_ISNAN}" )
endfunction()

####### Test if Random123 generators work #######
function( NEST_CHECK_RANDOM123 )
  message( STATUS "Check if Random123 generators work." )
  try_run( RUN_RESULT COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Random123/tests/kat_cpp.cpp
      CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
      RUN_OUTPUT_VARIABLE RUN_OUTPUT
      ARGS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Random123/tests/kat_vectors
      )
  if ( ${COMPILE_RESULT} AND ${RUN_RESULT} EQUAL 0 )
    set( HAVE_RANDOM123 ON )
  else ()
    message( ${RUN_OUTPUT} )
    set( HAVE_RANDOM123 OFF )
  endif ()
  set( HAVE_RANDOM123 ${HAVE_RANDOM123} PARENT_SCOPE )
  message( STATUS "Check if Random123 generators work. ${HAVE_RANDOM123}" )
endfunction()
