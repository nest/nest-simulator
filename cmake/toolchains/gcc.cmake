set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

get_filename_component(GCC_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

find_program(GCC_AR NAMES gcc-ar HINTS ${GCC_BIN_DIR})
find_program(GCC_RANLIB NAMES gcc-ranlib HINTS ${GCC_BIN_DIR})

if(GCC_AR)
    set(CMAKE_AR ${GCC_AR} CACHE FILEPATH "Archiver" FORCE)
endif()

if(GCC_RANLIB)
    set(CMAKE_RANLIB ${GCC_RANLIB} CACHE FILEPATH "Ranlib" FORCE)
endif()
