function( NEST_PROCESS_WITH_PYTHON )
  # Find Python
  set( HAVE_PYTHON OFF PARENT_SCOPE )
  if ( ${with-python} STREQUAL "2" )
    message( FATAL_ERROR "Python 2 is not supported anymore, please use Python 3 by setting with-python=ON" )
  elseif ( ${with-python} STREQUAL "ON" )

    # Localize the Python interpreter and lib/header files
    find_package( Python 3.8 REQUIRED Interpreter Development )

    if ( Python_FOUND )
      if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
        execute_process( COMMAND "${Python_EXECUTABLE}" "-c"
          "import sys, os; print(int(bool(os.environ.get('CONDA_DEFAULT_ENV', False)) or (sys.prefix != sys.base_prefix)))"
          OUTPUT_VARIABLE Python_InVirtualEnv
          OUTPUT_STRIP_TRAILING_WHITESPACE )
        if ( NOT Python_InVirtualEnv AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
          message( FATAL_ERROR "Can't target system Python installations without a CMAKE_INSTALL_PREFIX. Please install NEST only to virtual environments or specify CMAKE_INSTALL_PREFIX.")
        endif()
        set ( Python_EnvRoot "${Python_SITELIB}/../../..")
        set ( CMAKE_INSTALL_PREFIX "${Python_EnvRoot}" CACHE PATH "Default install prefix for the active Python interpreter" FORCE )
      endif ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

      # Create package metadata
      execute_process( COMMAND "${Python_EXECUTABLE} setup.py install_egg_info --install-dir .")
      # TODO: Register output to CMake so that it's installed alongside Python module

      set( HAVE_PYTHON ON PARENT_SCOPE )

      # export found variables to parent scope
      set( Python_FOUND "${Python_FOUND}" PARENT_SCOPE )
      set( Python_EXECUTABLE ${Python_EXECUTABLE} PARENT_SCOPE )
      set( PYTHON ${Python_EXECUTABLE} PARENT_SCOPE )
      set( Python_VERSION ${Python_VERSION} PARENT_SCOPE )

      set( Python_INCLUDE_DIRS "${Python_INCLUDE_DIRS}" PARENT_SCOPE )
      set( Python_LIBRARIES "${Python_LIBRARIES}" PARENT_SCOPE )

      if ( cythonize-pynest )
        # Need updated Cython because of a change in the C api in Python 3.7
        find_package( Cython 0.28.3 REQUIRED )
        if ( CYTHON_FOUND )
          # export found variables to parent scope
          set( CYTHON_FOUND "${CYTHON_FOUND}" PARENT_SCOPE )
          set( CYTHON_EXECUTABLE "${CYTHON_EXECUTABLE}" PARENT_SCOPE )
          set( CYTHON_VERSION "${CYTHON_VERSION}" PARENT_SCOPE )
        endif ()
      endif ()
      set( PYEXECDIR "lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages" PARENT_SCOPE )
    endif ()
  elseif ( ${with-python} STREQUAL "OFF" )
  else ()
    message( FATAL_ERROR "Invalid option: -Dwith-python=" ${with-python} )
  endif ()
endfunction()
