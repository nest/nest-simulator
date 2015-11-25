
# TODO set correctly
set(SLI_EXITCODE_ABORT 134 PARENT_SCOPE)
set(SLI_EXITCODE_SEGFAULT 139 PARENT_SCOPE)

#try_run(RUN_RESULT_VAR COMPILE_RESULT_VAR
#  ${CMAKE_BINARY_DIR}
#  ${CMAKE_CURRENT_SOURCE_DIR}/segfault_value.c
#  CMAKE_FLAGS -DCMAKE_C_FLAGS:String=-O3
#  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT_VAR
#  RUN_OUTPUT_VARIABLE RUN_OUTPUT_VAR)
#
#if(${COMPILE_RESULT_VAR})
#  message("Compile output:\n${COMPILE_OUTPUT_VAR}")
#else()
#  if(${RUN_RESULT_VAR} EQUAL "FAILED_TO_RUN")
#    message("RUN_OUTPUT_VAR: ${RUN_OUTPUT_VAR}")
#  else()
#    message("RUN_OUTPUT_VAR: ${RUN_OUTPUT_VAR}")
#    message("RUN_RESULT_VAR: ${RUN_RESULT_VAR}")
#  endif()
#endif()
#
#message("COMPILE_RESULT_VAR: ${COMPILE_RESULT_VAR}")
#message("COMPILE_OUTPUT_VAR: ${COMPILE_OUTPUT_VAR}")
#message("RUN_OUTPUT_VAR: ${RUN_OUTPUT_VAR}")
#message("RUN_RESULT_VAR: ${RUN_RESULT_VAR}")
#

message("Check whether the compiler ignores cmath makros.")
try_compile(COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/CMathMacros.cxx
  OUTPUT_VARIABLE OUTPUT
)
if(COMPILE_RESULT)
  set(HAVE_CMATH_MAKROS_IGNORED OFF CACHE BOOL "" FORCE)
else()
  set(HAVE_CMATH_MAKROS_IGNORED ON CACHE BOOL "" FORCE)
endif()
message("Check whether the compiler ignores cmath makros. ${HAVE_SIGUSR_IGNORED}")

message("Check whether the compiler does NOT include <*.h> headers ISO conformant.")
try_compile(COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/AlphaCXXBug.cxx
  OUTPUT_VARIABLE OUTPUT
)
if(COMPILE_RESULT)
  set(HAVE_ALPHA_CXX_STD_BUG OFF CACHE BOOL "" FORCE)
else()
  set(HAVE_ALPHA_CXX_STD_BUG ON CACHE BOOL "" FORCE)
endif()
message("Check whether the compiler does NOT include <*.h> headers ISO conformant. ${HAVE_ALPHA_CXX_STD_BUG}")

message("Check whether the compiler respects symbolic signal names in signal.h.")
try_compile(COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/SigUsrIgnored.cxx
  OUTPUT_VARIABLE OUTPUT
)
if(COMPILE_RESULT)
  set(HAVE_SIGUSR_IGNORED OFF CACHE BOOL "" FORCE)
else()
  set(HAVE_SIGUSR_IGNORED ON CACHE BOOL "" FORCE)
endif()
message("Check whether the compiler respects symbolic signal names in signal.h. ${HAVE_SIGUSR_IGNORED}")

message("Check whether static template member declaration succeeds.")
try_compile(COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/StaticTemplateDeclaration.cxx
  OUTPUT_VARIABLE OUTPUT
)
if(COMPILE_RESULT)
  set(HAVE_STATIC_TEMPLATE_DECLARATION_FAIL OFF CACHE BOOL "" FORCE)
else()
  set(HAVE_STATIC_TEMPLATE_DECLARATION_FAIL ON CACHE BOOL "" FORCE)
endif()
message("Check whether static template member declaration succeeds. ${HAVE_STATIC_TEMPLATE_DECLARATION_FAIL}")

message("Check for STL vector capacity base unity.")
set(RUN_RESULT 0)
set(RUN_RESULT__TRYRUN_OUTPUT "")
try_run(RUN_RESULT COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/VectorCapacity.cxx
  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT_VAR
  RUN_OUTPUT_VARIABLE RUN_OUTPUT_VAR)
if(RUN_RESULT EQUAL 0)
  set(HAVE_STL_VECTOR_CAPACITY_BASE_UNITY ON CACHE BOOL "" FORCE)
else()
  set(HAVE_STL_VECTOR_CAPACITY_BASE_UNITY OFF CACHE BOOL "" FORCE)
endif()
message("Check for STL vector capacity base unity. ${HAVE_STL_VECTOR_CAPACITY_BASE_UNITY}")

message("Check for STL vector capacity doubling strategy.")
try_run(RUN_RESULT COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/VectorCapacityDoubling.cxx
  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT_VAR
  RUN_OUTPUT_VARIABLE RUN_OUTPUT_VAR)
if(RUN_RESULT EQUAL 0)
  set(HAVE_STL_VECTOR_CAPACITY_DOUBLING ON CACHE BOOL "" FORCE)
else()
  set(HAVE_STL_VECTOR_CAPACITY_DOUBLING OFF CACHE BOOL "" FORCE)
endif()
message("Check for STL vector capacity doubling strategy. ${HAVE_STL_VECTOR_CAPACITY_DOUBLING}")


# Tests for a an internal compiler error observed in IBM xlC.
# If bug the ICE is detected, defines
# HAVE_XLC_ICE_ON_USING
#
# @author Hans E. Plesser

message("Check whether the compiler fails with ICE.")
try_compile(COMPILE_RESULT
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CheckFiles/ICE.cxx
  OUTPUT_VARIABLE OUTPUT
)
if(COMPILE_RESULT)
  set(HAVE_XLC_ICE_ON_USING OFF CACHE BOOL "" FORCE)
else()
  set(HAVE_XLC_ICE_ON_USING ON CACHE BOOL "" FORCE)
endif()
message("Check whether the compiler fails with ICE. ${HAVE_XLC_ICE_ON_USING}")
