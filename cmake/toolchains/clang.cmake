set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-prog-name=llvm-ar
    OUTPUT_VARIABLE LLVM_AR_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-prog-name=llvm-ranlib
    OUTPUT_VARIABLE LLVM_RANLIB_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# clang with LTO requires "llvm-ar" to be used instead of the standard "ar"
find_program(LLVM_AR NAMES ${LLVM_AR_PATH} llvm-ar)
find_program(LLVM_RANLIB NAMES ${LLVM_RANLIB_PATH} llvm-ranlib)

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")

if(LLVM_AR)
    set(CMAKE_AR ${LLVM_AR} CACHE FILEPATH "Archiver" FORCE)
else()
    message( SEND_ERROR "Clang with LTO requires llvm-ar" )
endif()

if(LLVM_RANLIB)
    set(CMAKE_RANLIB ${LLVM_RANLIB} CACHE FILEPATH "Ranlib" FORCE)
else()
    message( SEND_ERROR "Clang with LTO requires llvm-ranlib" )
endif()
